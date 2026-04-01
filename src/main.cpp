#include <Arduino.h>
#include <cstring>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <Ticker.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <algorithm>

extern "C" {
#include <user_interface.h>
}

#include "ascii.h"
#include "logger.h"
#include "mcal.h"
#include "wordclock.h"
#include "debounce.h"
#include "layouts/ilayout.h"
#include "layouts/layout_de_11x10.h"

/**
 * @brief Boot pin callback.
 */
void BootPinCbk();

ILayout* layout;
Wordclock* wordclock;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
Ticker restartTicker;
Debounce bootButton(MCAL_BOOT_PIN, 5000u, 100u, BootPinCbk);
bool portalActive = false;
Logger logger;
bool wifiConnected = false;

#define WORDCLOCK_BRIGHNESS_LIMIT (127u)
#define WORDCLOCK_USE_WIFI (true)
// OTA settings
#define WORDCLOCK_OTA_HOSTNAME "wordclock"
// Leave empty to disable OTA password
#define WORDCLOCK_OTA_PASSWORD ""
// mDNS hostname
#define WORDCLOCK_MDNS_HOSTNAME "wordclock"
// Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
#define WORDCLOCK_TIME_OFFSET (3600u)
#define WORDCLOCK_APPEARANCE_FILE "/appearance.cfg"
#define WORDCLOCK_ERROR_LOG_FILE "/error.log"

uint32_t gClockColor = 0u;
uint8_t gClockBrightness = WORDCLOCK_BRIGHNESS_LIMIT;
bool gAutoBrightnessEnabled = false;
float gAutoLuxLow = 5.0f;
float gAutoLuxHigh = 300.0f;
uint8_t gAutoBrightnessMin = 12u;
uint8_t gAutoBrightnessMax = WORDCLOCK_BRIGHNESS_LIMIT;
float gAmbientSamples[10] = {0.0f};
uint8_t gAmbientSampleIndex = 0u;
uint8_t gAmbientSampleCount = 0u;
float gAmbientFilteredLux = 0.0f;
uint8_t gEffectiveBrightness = WORDCLOCK_BRIGHNESS_LIMIT;
bool gLittleFsMounted = false;


/**
 * @brief WebSocket event handler.
 */
static void onWsEvent(AsyncWebSocket* serverPtr, AsyncWebSocketClient* client,
                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
  (void)serverPtr;
  (void)client;
  (void)arg;
  (void)data;
  (void)len;
  if (type == WS_EVT_CONNECT) {
    logger.info("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    logger.info("WebSocket client disconnected");
  }
}

/**
 * @brief Send current time to all WebSocket clients.
 * @param dt Current RTC date/time.
 */
static void sendTimeToWs(const RtcDateTime& dt) {
  char datetimeBuffer[25u] = "";
  snprintf(datetimeBuffer, sizeof(datetimeBuffer), "%04d/%02d/%02d %02d:%02d:%02d",
           dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
  ws.textAll(datetimeBuffer);
}

/**
 * @brief Build current timestamp string for persisted logger entries.
 * @return Timestamp string in YYYY/MM/DD HH:MM:SS format.
 */
static String getCurrentLogTimestamp() {
  if (wordclock != nullptr) {
    const RtcDateTime dt = wordclock->getRTCDateTime();
    if (dt.Year() >= 2024) {
      char datetimeBuffer[25u] = "";
      snprintf(datetimeBuffer, sizeof(datetimeBuffer), "%04d/%02d/%02d %02d:%02d:%02d",
               dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
      return String(datetimeBuffer);
    }
  }
  return String(millis()) + "ms";
}

/**
 * @brief Convert reset reason code into human-readable text.
 * @param reason Reset reason code from rst_info.
 * @return Reset reason text.
 */
static const char* resetReasonToString(uint8_t reason) {
  switch (reason) {
    case REASON_DEFAULT_RST: return "Power-On";
    case REASON_WDT_RST: return "Hardware WDT";
    case REASON_EXCEPTION_RST: return "Exception";
    case REASON_SOFT_WDT_RST: return "Software WDT";
    case REASON_SOFT_RESTART: return "Software Restart";
    case REASON_DEEP_SLEEP_AWAKE: return "Deep Sleep Wake";
    case REASON_EXT_SYS_RST: return "External Reset";
    default: return "Unknown";
  }
}

/**
 * @brief Detect and log reset cause on boot.
 */
static void logResetCause() {
  const rst_info* info = ESP.getResetInfoPtr();
  if (info == nullptr) {
    logger.error("Reset detected: reason unavailable");
    return;
  }

  String msg = "Reset detected: reason=" + String(resetReasonToString((uint8_t)info->reason)) +
               " (" + String((int)info->reason) + ")";

  if (info->reason == REASON_EXCEPTION_RST || info->reason == REASON_SOFT_WDT_RST || info->reason == REASON_WDT_RST) {
    msg += ", exccause=" + String((int)info->exccause) +
           ", epc1=0x" + String((unsigned long)info->epc1, HEX) +
           ", epc2=0x" + String((unsigned long)info->epc2, HEX) +
           ", epc3=0x" + String((unsigned long)info->epc3, HEX) +
           ", excvaddr=0x" + String((unsigned long)info->excvaddr, HEX) +
           ", depc=0x" + String((unsigned long)info->depc, HEX);
  }

  logger.error(msg);
}


/**
 * @brief Convert RGB color to hex string.
 * @param color 24-bit RGB value.
 * @return Hex string formatted as #RRGGBB.
 */
static String colorToHexString(uint32_t color) {
  char buf[8] = "";
  snprintf(buf, sizeof(buf), "#%06lX", (unsigned long)(color & 0xFFFFFFu));
  return String(buf);
}

/**
 * @brief Calculate the median from sampled ambient readings.
 * @return Median ambient lux.
 */
static float getAmbientMedianLux() {
  if (gAmbientSampleCount == 0u) {
    return 0.0f;
  }

  float sorted[10] = {0.0f};
  for (uint8_t i = 0u; i < gAmbientSampleCount; ++i) {
    sorted[i] = gAmbientSamples[i];
  }
  std::sort(sorted, sorted + gAmbientSampleCount);

  if ((gAmbientSampleCount & 0x01u) != 0u) {
    return sorted[gAmbientSampleCount / 2u];
  }

  const uint8_t idx = gAmbientSampleCount / 2u;
  return (sorted[idx - 1u] + sorted[idx]) * 0.5f;
}

/**
 * @brief Calculate active brightness based on current settings.
 * @return Brightness to apply.
 */
static uint8_t getTargetBrightness() {
  if (!gAutoBrightnessEnabled) {
    return gClockBrightness;
  }

  if (gAutoLuxHigh <= gAutoLuxLow) {
    return gAutoBrightnessMax;
  }

  float ratio = (gAmbientFilteredLux - gAutoLuxLow) / (gAutoLuxHigh - gAutoLuxLow);
  if (ratio < 0.0f) {
    ratio = 0.0f;
  } else if (ratio > 1.0f) {
    ratio = 1.0f;
  }

  const float scaled = (float)gAutoBrightnessMin +
                       ratio * ((float)gAutoBrightnessMax - (float)gAutoBrightnessMin);
  return (uint8_t)scaled;
}

/**
 * @brief Apply current color and brightness to the wordclock.
 */
static void applyClockAppearance() {
  gEffectiveBrightness = getTargetBrightness();
  wordclock->setBrightness(gEffectiveBrightness);
  wordclock->updateColor(gClockColor);
  wordclock->show();
}

/**
 * @brief Parse #RRGGBB string to 24-bit RGB.
 * @param hexColor Input color string.
 * @param outColor Parsed color.
 * @return true if parsing was successful.
 */
static bool parseHexColor(const String& hexColor, uint32_t* outColor) {
  if (outColor == nullptr) {
    return false;
  }

  if (hexColor.length() != 7 || hexColor.charAt(0) != '#') {
    return false;
  }

  char* end = nullptr;
  const unsigned long value = strtoul(hexColor.substring(1).c_str(), &end, 16);
  if (end == nullptr || *end != '\0' || value > 0xFFFFFFul) {
    return false;
  }

  *outColor = (uint32_t)value;
  return true;
}

/**
 * @brief Load persisted appearance settings from LittleFS.
 * @return true if at least one appearance field was loaded.
 */
static bool loadAppearanceFromFs() {
  File f;
  if (LittleFS.exists(WORDCLOCK_APPEARANCE_FILE)) {
    f = LittleFS.open(WORDCLOCK_APPEARANCE_FILE, "r");
  } else {
    return false;
  }
  if (!f) {
    logger.warn("Appearance file open failed");
    return false;
  }

  bool loadedAny = false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.startsWith("color=")) {
      uint32_t parsedColor = 0u;
      const String value = line.substring(6);
      if (parseHexColor(value, &parsedColor)) {
        gClockColor = parsedColor;
        loadedAny = true;
      }
    } else if (line.startsWith("#")) {
      // Legacy format: single #RRGGBB line in /color.txt
      uint32_t parsedColor = 0u;
      if (parseHexColor(line, &parsedColor)) {
        gClockColor = parsedColor;
        loadedAny = true;
      }
    } else if (line.startsWith("brightness=")) {
      const String value = line.substring(11);
      const long parsed = value.toInt();
      if (parsed >= 1 && parsed <= 255) {
        gClockBrightness = (uint8_t)parsed;
        loadedAny = true;
      }
    } else if (line.startsWith("auto_enabled=")) {
      const String value = line.substring(13);
      gAutoBrightnessEnabled = (value.toInt() != 0);
      loadedAny = true;
    } else if (line.startsWith("auto_lux_low=")) {
      const String value = line.substring(13);
      const float parsed = value.toFloat();
      if (parsed >= 0.0f) {
        gAutoLuxLow = parsed;
        loadedAny = true;
      }
    } else if (line.startsWith("auto_lux_high=")) {
      const String value = line.substring(14);
      const float parsed = value.toFloat();
      if (parsed >= 0.0f) {
        gAutoLuxHigh = parsed;
        loadedAny = true;
      }
    } else if (line.startsWith("auto_brightness_min=")) {
      const String value = line.substring(20);
      const long parsed = value.toInt();
      if (parsed >= 1 && parsed <= 255) {
        gAutoBrightnessMin = (uint8_t)parsed;
        loadedAny = true;
      }
    } else if (line.startsWith("auto_brightness_max=")) {
      const String value = line.substring(20);
      const long parsed = value.toInt();
      if (parsed >= 1 && parsed <= 255) {
        gAutoBrightnessMax = (uint8_t)parsed;
        loadedAny = true;
      }
    }
  }
  f.close();

  if (!loadedAny) {
    logger.warn("Appearance file content invalid");
    return false;
  }

  applyClockAppearance();
  logger.info("Loaded appearance: color=" + colorToHexString(gClockColor) +
              " brightness=" + String(gClockBrightness));
  return true;
}

/**
 * @brief Persist current appearance settings to LittleFS.
 * @return true if settings saved successfully.
 */
static bool saveAppearanceToFs() {
  File f = LittleFS.open(WORDCLOCK_APPEARANCE_FILE, "w");
  if (!f) {
    logger.warn("Appearance file write open failed");
    return false;
  }

  f.println(String("color=") + colorToHexString(gClockColor));
  f.println(String("brightness=") + String(gClockBrightness));
  f.println(String("auto_enabled=") + String(gAutoBrightnessEnabled ? 1 : 0));
  f.println(String("auto_lux_low=") + String(gAutoLuxLow, 2));
  f.println(String("auto_lux_high=") + String(gAutoLuxHigh, 2));
  f.println(String("auto_brightness_min=") + String(gAutoBrightnessMin));
  f.println(String("auto_brightness_max=") + String(gAutoBrightnessMax));
  f.close();
  logger.info("Saved appearance");
  return true;
}

/**
 * @brief Connect to WiFi using stored credentials or start captive portal.
 * @return true if connected to WiFi, false if portal started.
 */
static bool connectWifiWithPortal() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin();

  logger.info("WiFi connect: trying stored credentials");
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000u) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    portalActive = false;
    logger.info("WiFi connected: " + WiFi.localIP().toString());
    return true;
  }

  // Start AP + captive portal if no stored credentials or connection failed
  WiFi.mode(WIFI_AP);
  WiFi.softAP("wordclock-setup");
  dnsServer.start(53, "*", WiFi.softAPIP());
  portalActive = true;
  logger.warn("WiFi connect failed, started AP: wordclock-setup");
  return false;
}

/**
 * @brief Initialize Arduino OTA.
 */
static void setupOta() {
  ArduinoOTA.setHostname(WORDCLOCK_OTA_HOSTNAME);
  if (strlen(WORDCLOCK_OTA_PASSWORD) > 0) {
    ArduinoOTA.setPassword(WORDCLOCK_OTA_PASSWORD);
  }
  ArduinoOTA.begin();
  logger.info("OTA ready");
}

/**
 * @brief Initialize mDNS service.
 */
static void setupMdns() {
  if (!MDNS.begin(WORDCLOCK_MDNS_HOSTNAME)) {
    logger.warn("mDNS start failed");
    return;
  }
  MDNS.addService("http", "tcp", 80);
  logger.info(String("mDNS: http://") + WORDCLOCK_MDNS_HOSTNAME + ".local");
}

/**
 * @brief Initialize async web server and routes.
 */
static void setupWebServer() {
  if (!gLittleFsMounted) {
    logger.error("LittleFS not mounted, web files unavailable");
  }
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (portalActive) {
      request->send(LittleFS, "/config.html", "text/html");
    } else {
      request->send(LittleFS, "/index.html", "text/html");
    }
  });
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (!request->hasParam("ssid", true) || !request->hasParam("pass", true)) {
      request->send(400, "text/plain", "Missing ssid or pass");
      logger.warn("WiFi portal: missing ssid or pass");
      return;
    }
    const String ssid = request->getParam("ssid", true)->value();
    const String pass = request->getParam("pass", true)->value();
    request->send(200, "text/plain", "OK");
    logger.info("WiFi portal: credentials received, restarting");

    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    portalActive = false;
    restartTicker.once_ms(1000, []() { ESP.restart(); });
  });
  server.on("/api/color", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json",
                  String("{\"color\":\"") + colorToHexString(gClockColor) +
                  "\",\"brightness\":" + String(gClockBrightness) +
                  ",\"effective_brightness\":" + String(gEffectiveBrightness) + "}");
  });
  server.on("/api/color", HTTP_POST, [](AsyncWebServerRequest* request) {
    bool hasColor = request->hasParam("color", true);
    bool hasBrightness = request->hasParam("brightness", true);
    if (!hasColor && !hasBrightness) {
      request->send(400, "application/json", "{\"error\":\"missing params\"}");
      return;
    }

    if (hasColor) {
      const String colorParam = request->getParam("color", true)->value();
      uint32_t parsedColor = 0u;
      if (!parseHexColor(colorParam, &parsedColor)) {
        request->send(400, "application/json", "{\"error\":\"invalid color\"}");
        return;
      }
      gClockColor = parsedColor;
    }

    if (hasBrightness) {
      const String brightnessParam = request->getParam("brightness", true)->value();
      const long parsedBrightness = brightnessParam.toInt();
      if (parsedBrightness < 1 || parsedBrightness > 255) {
        request->send(400, "application/json", "{\"error\":\"invalid brightness\"}");
        return;
      }
      gClockBrightness = (uint8_t)parsedBrightness;
    }

    applyClockAppearance();
    if (!saveAppearanceToFs()) {
      request->send(500, "application/json", "{\"error\":\"save failed\"}");
      return;
    }

    request->send(200, "application/json",
                  String("{\"ok\":true,\"color\":\"") + colorToHexString(gClockColor) +
                  "\",\"brightness\":" + String(gClockBrightness) +
                  ",\"effective_brightness\":" + String(gEffectiveBrightness) + "}");
  });
  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json",
                  String("{\"auto_brightness_enabled\":") + String(gAutoBrightnessEnabled ? "true" : "false") +
                  ",\"auto_lux_low\":" + String(gAutoLuxLow, 2) +
                  ",\"auto_lux_high\":" + String(gAutoLuxHigh, 2) +
                  ",\"auto_brightness_min\":" + String(gAutoBrightnessMin) +
                  ",\"auto_brightness_max\":" + String(gAutoBrightnessMax) +
                  ",\"ambient_filtered_lux\":" + String(gAmbientFilteredLux, 2) + "}");
  });
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest* request) {
    bool changed = false;

    if (request->hasParam("auto_brightness_enabled", true)) {
      const String value = request->getParam("auto_brightness_enabled", true)->value();
      gAutoBrightnessEnabled = (value == "1" || value == "true" || value == "on");
      changed = true;
    }

    if (request->hasParam("auto_lux_low", true)) {
      const float value = request->getParam("auto_lux_low", true)->value().toFloat();
      if (value < 0.0f) {
        request->send(400, "application/json", "{\"error\":\"invalid auto_lux_low\"}");
        return;
      }
      gAutoLuxLow = value;
      changed = true;
    }

    if (request->hasParam("auto_lux_high", true)) {
      const float value = request->getParam("auto_lux_high", true)->value().toFloat();
      if (value < 0.0f) {
        request->send(400, "application/json", "{\"error\":\"invalid auto_lux_high\"}");
        return;
      }
      gAutoLuxHigh = value;
      changed = true;
    }

    if (request->hasParam("auto_brightness_min", true)) {
      const long value = request->getParam("auto_brightness_min", true)->value().toInt();
      if (value < 1 || value > 255) {
        request->send(400, "application/json", "{\"error\":\"invalid auto_brightness_min\"}");
        return;
      }
      gAutoBrightnessMin = (uint8_t)value;
      changed = true;
    }

    if (request->hasParam("auto_brightness_max", true)) {
      const long value = request->getParam("auto_brightness_max", true)->value().toInt();
      if (value < 1 || value > 255) {
        request->send(400, "application/json", "{\"error\":\"invalid auto_brightness_max\"}");
        return;
      }
      gAutoBrightnessMax = (uint8_t)value;
      changed = true;
    }

    if (gAutoBrightnessMin > gAutoBrightnessMax) {
      const uint8_t tmp = gAutoBrightnessMin;
      gAutoBrightnessMin = gAutoBrightnessMax;
      gAutoBrightnessMax = tmp;
    }

    if (gAutoLuxLow > gAutoLuxHigh) {
      const float tmp = gAutoLuxLow;
      gAutoLuxLow = gAutoLuxHigh;
      gAutoLuxHigh = tmp;
    }

    if (!changed) {
      request->send(400, "application/json", "{\"error\":\"no params\"}");
      return;
    }

    applyClockAppearance();
    if (!saveAppearanceToFs()) {
      request->send(500, "application/json", "{\"error\":\"save failed\"}");
      return;
    }

    request->send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"ok\":true}");
    logger.warn("Reset requested via web settings");
    restartTicker.once_ms(500, []() { ESP.restart(); });
  });
  server.on("/api/logs/errors", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!gLittleFsMounted) {
      request->send(500, "text/plain", "LittleFS not mounted");
      return;
    }
    if (!LittleFS.exists(WORDCLOCK_ERROR_LOG_FILE)) {
      request->send(404, "text/plain", "No error logs available");
      return;
    }
    request->send(LittleFS, WORDCLOCK_ERROR_LOG_FILE, "text/plain", true);
  });
  server.serveStatic("/", LittleFS, "/");
  server.begin();
  logger.info("Web server started");
}

/**
 * @brief Runnable called every 100ms (approx.).
 */
static void Runnable_100ms() {
  bootButton.poll();
  logger.handle();

  const float amb = wordclock->getAmbBrightness();
  gAmbientSamples[gAmbientSampleIndex] = amb;
  gAmbientSampleIndex = (uint8_t)((gAmbientSampleIndex + 1u) % 10u);
  if (gAmbientSampleCount < 10u) {
    gAmbientSampleCount++;
  }
  gAmbientFilteredLux = getAmbientMedianLux();

  if (gAutoBrightnessEnabled) {
    const uint8_t target = getTargetBrightness();
    if (target != gEffectiveBrightness) {
      gEffectiveBrightness = target;
      wordclock->setBrightness(gEffectiveBrightness);
    }
  }
}

/**
 * @brief Runnable called every 1000ms (approx.).
 */
static void Runnable_1000ms() {
  RtcDateTime dt = wordclock->getRTCDateTime();
  ws.cleanupClients();
  wordclock->clear();
  wordclock->setTime(dt.Hour(), dt.Minute());
  wordclock->show();
  logger.debug("Ambient median lux: " + String(gAmbientFilteredLux, 2));
  logger.debug("Wordclock time updated");
  sendTimeToWs(dt);
}

// TODO: Improve DST handling by adding a user option on the web interface to select the timezone and DST rules, and calculating the offset accordingly. The current implementation is a simplified example for Central Europe only.
static uint32_t calculateTimeOffset(const RtcDateTime& dt) {
  // For Central Europe: DST starts last Sunday of March, ends last Sunday of October
  int month = dt.Month();
  int day = dt.Day();
  int hour = dt.Hour();

  // Before March or after October = winter time
  if (month < 3 || month > 10) {
    return 3600u;  // GMT +1 (CET)
  }

  // After October = winter time
  if (month > 10) {
    return 3600u;  // GMT +1 (CET)
  }

  // March: DST starts last Sunday of March at 2:00 AM
  if (month == 3) {
    int lastSunday = 31 - ((dt.DayOfWeek() + (31 - day)) % 7);
    if (day < lastSunday || (day == lastSunday && hour < 2)) {
      return 3600u;  // GMT +1 (CET)
    }
    return 7200u;  // GMT +2 (CEST)
  }

  // October: DST ends last Sunday of October at 3:00 AM
  if (month == 10) {
    int lastSunday = 31 - ((dt.DayOfWeek() + (31 - day)) % 7);
    if (day > lastSunday || (day == lastSunday && hour >= 3)) {
      return 3600u;  // GMT +1 (CET)
    }
    return 7200u;  // GMT +2 (CEST)
  }

  // April to September = summer time
  return 7200u;  // GMT +2 (CEST)
}

/**
 * @brief Callback for boot pin
 *
 */
/**
 * @brief Handle boot pin press.
 */
void BootPinCbk()
{
  Serial.println("-----Rebooting Wordclock-----");
  wordclock->powerOff();
  delay(100);
  //ESP.reset();
  ESP.rebootIntoUartDownloadMode();
}

/**
 * @brief Arduino setup.
 */
void setup() {
  // Initialize GPIOs
  pinMode(MCAL_WIFI_EN_PIN, INPUT);
  pinMode(MCAL_BOOT_PIN, INPUT);
  pinMode(MCAL_LED_EN_PIN, OUTPUT);
  pinMode(MCAL_DAT_PIN,OUTPUT);
  pinMode(MCAL_CLK_PIN,OUTPUT);
  pinMode(MCAL_RST_PIN,OUTPUT);
  digitalWrite(MCAL_LED_EN_PIN, LOW);

  Serial.begin(115200);
  delay(50);
  gLittleFsMounted = LittleFS.begin();
  logger.begin(23);
  logger.setLevel(LOG_DEBUG);
  logger.setTimestampProvider(getCurrentLogTimestamp);
  if (gLittleFsMounted) {
    logger.enableErrorFileLogging(WORDCLOCK_ERROR_LOG_FILE);
  }
  logger.info("Booting");


  // Initialize led strip
  layout = new Layout_De_11x10();
  wordclock = new Wordclock(layout);

  logResetCause();

#if (WORDCLOCK_USE_WIFI == true)
  wifiConnected = connectWifiWithPortal();
  setupWebServer();
  if (wifiConnected) {
    setupOta();
    setupMdns();

    // Initialize a NTPClient to get actual time
    timeClient.begin();
    timeClient.update();
    logger.info("NTP time updated");

    // Update RTC with server time
    // TODO: Improve DST handling by adding a user option on the web interface to select the timezone and DST rules, and calculating the offset accordingly. The current implementation is a simplified example for Central Europe only.
    RtcDateTime dt;
    dt.InitWithUnix32Time((uint32_t)(timeClient.getEpochTime()));
    timeClient.setTimeOffset(calculateTimeOffset(dt));
    timeClient.update();
    dt.InitWithUnix32Time((uint32_t)(timeClient.getEpochTime()));
    wordclock->setRTCDateTime(dt);
  }
#endif

  // Switch LED power on
  wordclock->powerOn();
  delay(100);
  wordclock->begin();
  wordclock->clear();
  gClockColor = wordclock->Color(255u, 0u, 255u);
  gClockBrightness = WORDCLOCK_BRIGHNESS_LIMIT;
  applyClockAppearance();
  if (!loadAppearanceFromFs()) {
    saveAppearanceToFs();
  }
  wordclock->show();
}

/**
 * @brief Arduino main loop.
 */
void loop() {
  if (wifiConnected) {
    ArduinoOTA.handle();
    MDNS.update();
  }
  if (portalActive) {
    dnsServer.processNextRequest();
  }

  static uint32_t last100msMs = 0u;
  const uint32_t nowMs = millis();
  if (nowMs - last100msMs >= 100u) {
    last100msMs = nowMs;
    Runnable_100ms();
  }

  static uint32_t last1000msMs = 0u;
  if (nowMs - last1000msMs >= 1000u) {
    last1000msMs = nowMs;
    Runnable_1000ms();
  }

}
