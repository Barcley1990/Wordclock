#include "recovery.h"
#include "recovery_state.h"

#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

extern "C" {
#include <user_interface.h>
}

namespace Recovery {
namespace {
// eboot owns the first 128 bytes of RTC user memory during OTA. Offset is in
// words: reserve our record at byte 256, outside the bootloader command.
constexpr uint32_t RtcOffset = 64;
constexpr uint32_t CrashRtcOffset = 72;
constexpr uint32_t CrashMagic = 0x57434331;
struct CrashRecord {
    uint32_t magic;
    uint32_t image;
    uint32_t reason;
    uint32_t cause;
    uint32_t epc1;
    uint32_t address;
    uint32_t count;
    uint32_t code[45];
    uint32_t check;
};
static_assert(CrashRtcOffset * 4 >= RtcOffset * 4 + sizeof(RecoveryState::Record), "RTC records overlap");
static_assert(CrashRtcOffset * 4 + sizeof(CrashRecord) <= 512, "Crash record exceeds RTC user memory");
CrashRecord lastCrash{};
uint32_t imageTag = 0;
bool crashAvailable = false;

uint32_t crashCheck(const CrashRecord& record) {
    uint32_t hash = CrashMagic ^ record.image ^ record.reason ^ record.cause ^
                    record.epc1 ^ record.address ^ record.count;
    for (uint32_t value : record.code) hash = (hash ^ value) * 16777619u;
    return hash;
}

void captureCrash(const rst_info* info, uint32_t stack, uint32_t end) {
    // No allocations, filesystem or network operations in the panic callback.
    lastCrash = {};
    lastCrash.magic = CrashMagic;
    lastCrash.image = imageTag;
    if (info) {
        lastCrash.reason = info->reason;
        lastCrash.cause = info->exccause;
        lastCrash.epc1 = info->epc1;
        lastCrash.address = info->excvaddr;
    }
    if ((stack & 3u) == 0 && stack >= 0x3ffe8000u && end <= 0x40000000u && end > stack) {
        if (end - stack > 8192u) end = stack + 8192u;
        for (uint32_t pos = stack; pos + 4 <= end && lastCrash.count < 45; pos += 4) {
            const uint32_t value = *reinterpret_cast<const uint32_t*>(pos);
            if (value >= 0x40000000u && value < 0x40300000u) {
                lastCrash.code[lastCrash.count++] = value;
            }
        }
    }
    lastCrash.check = crashCheck(lastCrash);
    ESP.rtcUserMemoryWrite(CrashRtcOffset, reinterpret_cast<uint32_t*>(&lastCrash), sizeof(lastCrash));
}

String crashJson() {
    if (!crashAvailable) return "null";
    String result;
    result.reserve(900);
    result = String("{\"image\":\"0x") + String(lastCrash.image, HEX) +
        "\",\"reason\":" + lastCrash.reason + ",\"cause\":" + lastCrash.cause +
        ",\"epc1\":\"0x" + String(lastCrash.epc1, HEX) +
        "\",\"address\":\"0x" + String(lastCrash.address, HEX) + "\",\"code\":[";
    for (uint32_t i = 0; i < lastCrash.count; ++i) {
        if (i) result += ',';
        result += String("\"0x") + String(lastCrash.code[i], HEX) + '"';
    }
    return result + "]}";
}

constexpr uint32_t BootWindowMs = 10000;
constexpr uint32_t HealthyAfterMs = 60000;
ESP8266WebServer web(80);
ESP8266HTTPUpdateServer updater;
DNSServer dns;
bool maintenance = false;
bool webRunning = false;
bool appStarted = false;
bool healthy = false;
bool apActive = false;
bool restartPending = false;
uint32_t restartAt = 0;
uint32_t windowStarted = 0;
uint32_t applicationStarted = 0;
uint32_t disconnectedAt = 0;
uint32_t resetReason = 0;
rst_info bootResetInfo{};
String sketchMd5;
void (*suspendApp)() = nullptr;

const char RecoveryPage[] PROGMEM = R"html(<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Wordclock recovery</title><body>
<h1>Wordclock recovery</h1>
<p>The updater works without the clock application, sensors or filesystem.</p>
<p><a href="/update">Upload firmware or filesystem</a> | <a href="/api/recovery/status">Device status</a></p>
<form method="post" action="/api/recovery/enter"><button>Stay in recovery / stop clock</button></form>
<form method="post" action="/api/recovery/resume"><button>Restart clock application</button></form>
<h2>WiFi setup</h2><form method="post" action="/wifi">
<label>SSID <input name="ssid" required maxlength="32"></label>
<label>Password <input name="pass" type="password" maxlength="64"></label>
<button>Save WiFi and restart in recovery</button></form>
<p>Fallback access point: wordclock-recovery, address 192.168.4.1.</p>
</body></html>)html";

bool store(RecoveryState::Phase phase) {
    auto record = RecoveryState::make(phase);
    if (!ESP.rtcUserMemoryWrite(RtcOffset, reinterpret_cast<uint32_t*>(&record), sizeof(record))) return false;
    RecoveryState::Record readback{};
    return ESP.rtcUserMemoryRead(RtcOffset, reinterpret_cast<uint32_t*>(&readback), sizeof(readback)) &&
           RecoveryState::valid(readback) && readback.phase == record.phase;
}

void startAp() {
    if (apActive) return;
    WiFi.mode(WIFI_AP_STA); // Keep retrying the stored WiFi credentials.
    apActive = WiFi.softAP("wordclock-recovery");
    if (apActive) dns.start(53, "*", WiFi.softAPIP());
}

void scheduleRestart() {
    restartPending = true;
    restartAt = millis();
}
} // namespace

const char* page() { return RecoveryPage; }

String statusJson() {
    const char* mode = maintenance ? "recovery" : appStarted ? "normal" : "startup";
    return String("{\"mode\":\"") + mode + "\",\"recovery_version\":3,\"reset_reason\":" + resetReason +
        ",\"reset_epc1\":\"0x" + String(bootResetInfo.epc1, HEX) +
        "\",\"reset_exccause\":" + bootResetInfo.exccause +
        ",\"reset_excvaddr\":\"0x" + String(bootResetInfo.excvaddr, HEX) +
        "\",\"last_crash\":" + crashJson() +
        ",\"free_heap\":" + ESP.getFreeHeap() +
        ",\"max_free_block\":" + ESP.getMaxFreeBlockSize() +
        ",\"application_healthy\":" + (healthy ? "true" : "false") +
        ",\"uptime_ms\":" + millis() + ",\"sketch_size\":" + ESP.getSketchSize() +
        ",\"free_sketch_space\":" + ESP.getFreeSketchSpace() +
        ",\"flash_size\":" + ESP.getFlashChipSize() +
        ",\"flash_real_size\":" + ESP.getFlashChipRealSize() +
        ",\"sketch_md5\":\"" + sketchMd5 +
        "\",\"wifi_ip\":\"" + WiFi.localIP().toString() +
        "\",\"fallback_ap\":" + (apActive ? "true" : "false") + "}";
}

bool enter() {
    if (!store(RecoveryState::Phase::Latched)) return false;
    scheduleRestart();
    return true;
}

void begin(void (*suspendApplication)()) {
    suspendApp = suspendApplication;
    crashAvailable = ESP.rtcUserMemoryRead(CrashRtcOffset, reinterpret_cast<uint32_t*>(&lastCrash), sizeof(lastCrash)) &&
        lastCrash.magic == CrashMagic && lastCrash.count <= 45 && lastCrash.check == crashCheck(lastCrash);
    const rst_info* info = ESP.getResetInfoPtr();
    if (info) bootResetInfo = *info;
    resetReason = info ? info->reason : REASON_DEFAULT_RST;
    const bool crashed = resetReason == REASON_EXCEPTION_RST || resetReason == REASON_SOFT_WDT_RST ||
                         resetReason == REASON_WDT_RST;
    RecoveryState::Record record{};
    const bool readOk = ESP.rtcUserMemoryRead(RtcOffset, reinterpret_cast<uint32_t*>(&record), sizeof(record));
    // An OTA from older firmware has no record yet. Keep its first software
    // restart in recovery until the new image has been checked remotely.
    maintenance = RecoveryState::mustRecover(record, readOk, crashed) ||
                  (resetReason == REASON_SOFT_RESTART && (!readOk || !RecoveryState::valid(record)));
    if (!store(maintenance ? RecoveryState::Phase::Latched : RecoveryState::Phase::Starting)) {
        maintenance = true; // Fail closed if reset tracking cannot be stored.
    }

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.hostname("wordclock");
    WiFi.setAutoReconnect(true);
    WiFi.begin();
    const uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && uint32_t(millis() - started) < 15000) delay(10);
    if (WiFi.status() != WL_CONNECTED) startAp();
    disconnectedAt = millis();
    // Cache this flash scan before any async HTTP callbacks can request status.
    sketchMd5 = ESP.getSketchMD5();
    imageTag = strtoul(sketchMd5.substring(0, 8).c_str(), nullptr, 16);

    // One shared Update object serves ArduinoOTA and the core HTTP updater.
    // Every attempt latches recovery, even if the transfer is interrupted.
    Update.onStart([]() {
        maintenance = true;
        store(RecoveryState::Phase::Latched);
        if (suspendApp) suspendApp();
    });
    ArduinoOTA.setHostname("wordclock");
    ArduinoOTA.begin(); // Also active on the fallback AP.
    MDNS.addService("http", "tcp", 80);

    web.on("/", HTTP_GET, []() { web.send_P(200, "text/html", RecoveryPage); });
    web.on("/recovery", HTTP_GET, []() { web.send_P(200, "text/html", RecoveryPage); });
    web.on("/api/recovery/status", HTTP_GET, []() { web.send(200, "application/json", statusJson()); });
    web.on("/api/recovery/enter", HTTP_POST, []() {
        maintenance = true;
        const bool ok = store(RecoveryState::Phase::Latched);
        web.send(ok ? 200 : 500, "text/plain", ok ? "Recovery mode active" : "RTC state write failed");
    });
    web.on("/api/recovery/resume", HTTP_POST, []() {
        if (!store(RecoveryState::Phase::Healthy)) {
            web.send(500, "text/plain", "RTC state write failed");
            return;
        }
        web.send(200, "text/plain", "Restarting clock; recovery returns on a crash.");
        scheduleRestart();
    });
    web.on("/wifi", HTTP_POST, []() {
        if (!web.hasArg("ssid") || web.arg("ssid").length() == 0 || web.arg("ssid").length() > 32 ||
            web.arg("pass").length() > 64 || !store(RecoveryState::Phase::Latched)) {
            web.send(400, "text/plain", "Invalid credentials or RTC state unavailable");
            return;
        }
        WiFi.persistent(true);
        WiFi.begin(web.arg("ssid").c_str(), web.arg("pass").c_str());
        WiFi.persistent(false);
        web.send(200, "text/plain", "WiFi saved. Restarting in recovery.");
        scheduleRestart();
    });
    updater.setup(&web, "/update"); // HTML lives in firmware, never in LittleFS.
    web.begin();
    webRunning = true;
    windowStarted = millis();
}

void handle() {
    ArduinoOTA.handle();
    if (maintenance && !webRunning) {
        web.begin();
        webRunning = true;
    }
    if (webRunning) web.handleClient();
    if (apActive) dns.processNextRequest();
    if (WiFi.status() == WL_CONNECTED) disconnectedAt = millis();
    else if (uint32_t(millis() - disconnectedAt) >= 30000) startAp();
    if (restartPending && uint32_t(millis() - restartAt) >= 500) ESP.restart();
}

bool applicationAllowed() {
    return !maintenance && !restartPending && !Update.isRunning() &&
           (appStarted || uint32_t(millis() - windowStarted) >= BootWindowMs);
}

void applicationStarting() {
    web.stop();
    webRunning = false;
    appStarted = true;
    applicationStarted = millis();
}

void applicationHealthy() {
    if (!healthy && !maintenance && uint32_t(millis() - applicationStarted) >= HealthyAfterMs) {
        healthy = store(RecoveryState::Phase::Healthy);
    }
}
} // namespace Recovery

extern "C" void custom_crash_callback(rst_info* info, uint32_t stack, uint32_t end) {
    Recovery::captureCrash(info, stack, end);
}
