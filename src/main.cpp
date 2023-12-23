
/**********************************************************************************************************************
 * @file main.cpp
 * @author Tobias Nuß
 * @brief
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Include Area
 ***********************************************************************************************************************/
#include "ascii.h"
#include "mcal.h"
#include "version.h"
#include "wordclock.h"
#include "debounce.h"
#include "layouts/ilayout.h"
#include "layouts/layout_de_11x10.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <RtcDateTime.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WORDCLOCK_BRIGHNESS_LIMIT (127u)
#define WORDCLOCK_USE_WIFI        (true)
#define WORDCLOCK_WIFI_TIMEOUT    (20u)
// Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
#define WORDCLOCK_TIME_OFFSET     (3600u)

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
static bool WiFi_Setup_Successful = false;

const char* mdns_name = "Wordclock";
const char* ssid      = "TARDIS"; 
const char* password  = "82uqFnUSjUn7YL";

const uint8_t WEBSOCKET_COMMAND_SET_HSV = 95;
const uint8_t WEBSOCKET_COMMAND_LEDPWR_OFF = 96;
const uint8_t WEBSOCKET_COMMAND_LEDPWR_ON = 97;
const uint8_t WEBSOCKET_COMMAND_ESP_RESET = 20;
const uint8_t WEBSOCKET_COMMAND_WEBSOCKET_RDY = 21;
const uint8_t WEBSOCKET_COMMAND_ADPTV_BRIGHNTESS = 98;

const char* JSON_KEY_COMMAND          = "CMD";
const char* JSON_KEY_AMBIENT          = "Light";
const char* JSON_KEY_TIME             = "Time";
const char* JSON_KEY_VERSION          = "Version";
const char* JSON_KEY_WIFI_STATUS      = "WiFiStatus";
const char* JSON_KEY_LED_PWR_STATE    = "PwrState";
const char* JSON_KEY_BRIGHTNESS       = "Brightness";
const char* JSON_KEY_HSV_COLOR        = "HSV";
const char* JSON_KEY_ADPTV_BRIGHT     = "AdptvBrightness";

/***********************************************************************************************************************
 * Local function declarations and objects
 ***********************************************************************************************************************/
void Runnable_100_ms();
void Runnable_1000_ms();
void WebSocketReceive(uint8_t* payload, uint8_t length);
//void WebSocketSend(String key, const void* data);
template <typename T> 
void WebSocketSend(const char *key, T value);
void WebSocketSend(const JsonDocument &doc);
void WebSocketSend(String s);
void BootPinCbk();
String GetVersion();

IPAddress local_IP(192, 168, 178, 90);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 255);
ESP8266WebServer HtmlServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer OTAServer;
ILayout* layout;
Wordclock* wordclock;
Debounce bootButton(MCAL_BOOT_PIN, 5000u, 100u, BootPinCbk);
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const size_t size = 512;
StaticJsonDocument<size> ClockSettings;
#if (DEBUG_MODE)
StaticJsonDocument<512> DebugMsg;
static inline void debugWebSocket(String msg);
#endif
  
/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
/**
 * @brief Main init funciton. Called only once from within main
*/
void setup()
{
  // Initialize GPIOs
  pinMode(MCAL_WIFI_EN_PIN, INPUT);
  pinMode(MCAL_BOOT_PIN, INPUT);
  pinMode(MCAL_LED_EN_PIN, OUTPUT);
  pinMode(MCAL_DAT_PIN,OUTPUT);
  pinMode(MCAL_CLK_PIN,OUTPUT);
  pinMode(MCAL_RST_PIN,OUTPUT);
  digitalWrite(MCAL_LED_EN_PIN, LOW);

  // Start serial inteface
  Serial.begin(115200);
  delay(100);

  // Print software version
  DEBUG_MSG_LN("");
  DEBUG_MSG_LN("");
  DEBUG_MSG_LN(header);
  DEBUG_MSG_LN("");
  DEBUG_MSG_LN("");
  DEBUG_MSG_LN("--------------------------------------");
  DEBUG_MSG("\nVersion: ");
  DEBUG_MSG_LN(GetVersion());
  Serial.printf("Chip ID         : %08X\n", ESP.getChipId());
  Serial.printf("Flash ID        : %08X\n\n", ESP.getFlashChipId());
  Serial.printf("CPU Speed       : %u MHz \n\n", ESP.getCpuFreqMHz());

  Serial.printf("Flash real Size : %u KByte\n",
                ESP.getFlashChipRealSize() / 1024);
  Serial.printf("Flash ide  Size : %u KByte\n",
                ESP.getFlashChipSize() / 1024);
  Serial.printf("Flash ide Speed : %u\n\n", ESP.getFlashChipSpeed());

  Serial.printf("Free Heap Size  : %u Byte\n", ESP.getFreeHeap());
  Serial.printf("Sketch Size     : %u Byte \n", ESP.getSketchSize());
  Serial.printf("Free Sketch Size: %u Byte \n\n", ESP.getFreeSketchSpace());

  Serial.printf("SDK Version     : %s\n", ESP.getSdkVersion());
  DEBUG_MSG("RESET Info      : ");
  DEBUG_MSG_LN(ESP.getResetInfo());
  DEBUG_MSG("COMPILED        : ");
  DEBUG_MSG(__DATE__);
  DEBUG_MSG(" ");
  DEBUG_MSG_LN(__TIME__);

  DEBUG_MSG_LN("--------------------------------------");
  DEBUG_MSG_LN("Ende Setup");
  DEBUG_MSG_LN("--------------------------------------");
  DEBUG_MSG_LN("");
  delay(100);


#if (WORDCLOCK_USE_WIFI == true)
  // Initialize LittleFS
  DEBUG_MSG("Try to initialize LittleFS... ");
  //LittleFS.format();
  if (!LittleFS.begin())
  {
    DEBUG_MSG_LN("-> An error has occurred while mounting LittleFS. End Setup!\n");
    return;
  }
  else
  {
    DEBUG_MSG_LN("-> LittleFS mounted successfully\n");
  }

  // Connect to the network
  WiFi.begin(ssid, password);             
  DEBUG_MSG("Connecting to ");
  DEBUG_MSG(ssid); DEBUG_MSG_LN(" ...");

#if DEBUG_MODE
  int i = 0;
  // Wait for the Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    DEBUG_MSG(++i); DEBUG_MSG(' ');
  }
#endif 

  // Send the IP address of the ESP8266 to the computer
  DEBUG_MSG_LN('\n');
  DEBUG_MSG_LN("Connection established!");  
  DEBUG_MSG("IP address:\t");
  DEBUG_MSG_LN(WiFi.localIP());         

  {
    // Successful connected to local wifi...
    DEBUG_MSG("Successfully connected! IP is: ");
    DEBUG_MSG_LN(WiFi.localIP());

    // Configure html server
    HtmlServer.onNotFound([](){
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += HtmlServer.uri();
        message += "\nMethod: ";
        message += (HtmlServer.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += HtmlServer.args();
        message += "\n";
        for (uint8_t i = 0; i < HtmlServer.args(); i++) {
          message += " " + HtmlServer.argName(i) + ": " + HtmlServer.arg(i) + "\n";
        }
        HtmlServer.send(404, "text/plain", message);
    });
    HtmlServer.serveStatic("/", LittleFS, "/index.html");
    HtmlServer.serveStatic("/index", LittleFS, "/index.html");
    HtmlServer.serveStatic("/update", LittleFS, "/update.html");
    HtmlServer.serveStatic("/settings", LittleFS, "/settings.html");
    HtmlServer.serveStatic("/css/style.css", LittleFS, "/css/style.css");
    HtmlServer.serveStatic("/css/color-picker.css", LittleFS, "/css/color-picker.css");
    HtmlServer.serveStatic("/js/index.js", LittleFS, "/js/index.js");
    HtmlServer.serveStatic("/js/settings.js", LittleFS, "/js/settings.js");
    HtmlServer.serveStatic("/js/color-picker.js", LittleFS, "/js/color-picker.js");
    HtmlServer.serveStatic("/js/websocket.js", LittleFS, "/js/websocket.js");
    HtmlServer.serveStatic("/js/jquery.js", LittleFS, "/js/jquery.js");
    HtmlServer.serveStatic("/images/picker_dark_theme.png", LittleFS, "/images/picker_dark_theme.png");


    // Configure WebSocket Server
    webSocket.onEvent([](char num, WStype_t type, uint8_t* payload, size_t length){
      String version = GetVersion();
      String settings;
      switch (type)
      {
      case WStype_DISCONNECTED:
        DEBUG_MSG_LN("Client Disconnected");
        break;
      case WStype_CONNECTED:
        DEBUG_MSG_LN("Client Connected");
        debugWebSocket("Websocket Connected!");
        break;
      case WStype_TEXT:
        DEBUG_MSG("Data Received: ");
        WebSocketReceive(payload, length);
        break;
        //-Do Nothing-
      case WStype_BIN:
      case WStype_ERROR:
      case WStype_FRAGMENT_TEXT_START:
      case WStype_FRAGMENT_BIN_START:
      case WStype_FRAGMENT:
      case WStype_FRAGMENT_FIN:
      default:
        break;
      }
    });

    // Initialize servers (HTML, WS, OTA, mDNS)
    OTAServer.setup(&HtmlServer);
    HtmlServer.begin();
    webSocket.begin();
    if (MDNS.begin(mdns_name)) {
      DEBUG_MSG("DNS started: ");
      DEBUG_MSG_LN("http://" + String(mdns_name) + ".local/");
    }
    MDNS.addService("http", "tcp", 80);

    DEBUG_MSG("HTTP server started on port 80: ");
    Serial.printf("HTTPUpdateServer (OTA) ready! Open http://%s.local/update in your browser\n", mdns_name);
    WiFi_Setup_Successful = true;
  }
#endif

#if (WORDCLOCK_USE_WIFI == true)
  // Initialize a NTPClient to get actual time
  timeClient.begin();
  timeClient.setTimeOffset(WORDCLOCK_TIME_OFFSET);
  timeClient.update();

  // Update RTC with server time
  RtcDateTime dt;
  dt.InitWithUnix32Time((uint32_t)(timeClient.getEpochTime()));
#endif

  // Initialize led strip
  layout = new Layout_De_11x10();
  wordclock = new Wordclock(layout);

  // Update RTC with server time
  wordclock->setRTCDateTime(dt);
  // Switch LED power on
  wordclock->powerOn();
  delay(100);
  wordclock->begin();
  wordclock->clear();
  wordclock->setBrightness(WORDCLOCK_BRIGHNESS_LIMIT);
  wordclock->show();

  // Update Clock Settings
  ClockSettings[JSON_KEY_LED_PWR_STATE] = wordclock->getPowerState();
  ClockSettings[JSON_KEY_BRIGHTNESS] = wordclock->getBrightness();
  ClockSettings[JSON_KEY_HSV_COLOR] = wordclock->getHSVColor();
  ClockSettings[JSON_KEY_VERSION] = GetVersion();
  ClockSettings[JSON_KEY_ADPTV_BRIGHT] = "false";
  ClockSettings[JSON_KEY_WIFI_STATUS] = WiFi.RSSI();
}

/**
 * @brief Main loop  function. Called from main()
 */
void loop()
{
  static uint32_t old_time_100 = 0u;
  static uint32_t old_time_1000 = 0u;
  uint32_t sys_time = millis();

  // check if 100ms passed
  if ((uint32_t)(sys_time - old_time_100) >= 100u) {
    old_time_100 = sys_time;
    Runnable_100_ms();
  }

  // check if 1000ms passed
  if ((uint32_t)(sys_time - old_time_1000) >= 1000u) {
    old_time_1000 = sys_time;
    Runnable_1000_ms();
  }

#if (WORDCLOCK_USE_WIFI == true)
  // Execute Server Objects
  if(WiFi_Setup_Successful == true)
  {
    HtmlServer.handleClient();
    webSocket.loop();
    MDNS.update();
  }
#endif
}

/**
 * @brief called every 100ms
 *
 */
void Runnable_100_ms()
{
  bootButton.poll();
}

/**
 * @brief called every 1000ms
 *
 */
void Runnable_1000_ms()
{
  // Get Ambient Brightness
  float lux = wordclock->getAmbBrightness();

  // Get RTC Time
  RtcDateTime dt = wordclock->getRTCDateTime();

  // Convert date-time to send to html server
  char datetimeBuffer[25u] = "";
  sprintf(datetimeBuffer, "%04d/%02d/%02d %02d:%02d:%02d",
    dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());

  // Send data to client via web socket
  String ambBrightness = (String)lux;
  WebSocketSend<String>(JSON_KEY_AMBIENT, ambBrightness);
  WebSocketSend<String>(JSON_KEY_TIME, datetimeBuffer);

  // Check if adaptive brightness is enabled
  const char *cb = ClockSettings[JSON_KEY_ADPTV_BRIGHT];
  if (strcmp(cb, "true") == 0) {
      wordclock->updateColor(0);
  }
  
  // Update Wordclock Time
  wordclock->clear();
  wordclock->setTime(dt.Hour(), dt.Minute());
  wordclock->show();
}

/**
 * @brief Receive Websocket Data
 *
 * @param payload
 * @param length
 */
void WebSocketReceive(uint8_t *payload, uint8_t length)
{
  StaticJsonDocument<512> json;
  String settings;

  // Deserialize payload
  json.clear();
  DeserializationError err = deserializeJson(json, payload);
  if (err) 
  {
#if (DEBUG_MODE == true)
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      debugWebSocket("deserializeJson() failed.");
#endif
      return;
  }
#if (DEBUG_MODE == true)
  else
  {
    String stringified;
    (void)serializeJson(json, stringified);
    Serial.println(stringified);
    debugWebSocket("Data received: ");
    debugWebSocket(stringified);
  }
#endif

  // Check for general commands
  if (json.containsKey(JSON_KEY_COMMAND) == true) {
    
    // Check which command was received
    uint8_t cmd = (uint8_t)json[JSON_KEY_COMMAND];
    switch(cmd)
    { 
      case WEBSOCKET_COMMAND_ESP_RESET: {
       ESP.reset(); 
      } 
      break;
      case WEBSOCKET_COMMAND_WEBSOCKET_RDY: {
        WebSocketSend(ClockSettings);
      }
      break;
      case WEBSOCKET_COMMAND_LEDPWR_OFF: {
        DEBUG_MSG_LN("Turn Leds off");
        wordclock->powerOff();
      }
      break;
      case WEBSOCKET_COMMAND_LEDPWR_ON: {
        DEBUG_MSG_LN("Turn Leds on");
        wordclock->powerOn();
      }
      break;
      default: {
        DEBUG_MSG_LN("Unknown command.");
      }
    }
  }
  // Check if KEY for adaptive brightness was received
  else if(json.containsKey(JSON_KEY_ADPTV_BRIGHT)) {
    //ClockSettings[JSON_KEY_ADPTV_BRIGHT] = (boolean)json[JSON_KEY_ADPTV_BRIGHT];
  }
  // Check if KEY for HSV color was received
  else if(json.containsKey(JSON_KEY_HSV_COLOR)) {
    uint16_t hue = (uint16_t)json[JSON_KEY_HSV_COLOR]["HUE"];
    uint8_t sat = (uint8_t)json[JSON_KEY_HSV_COLOR]["SAT"];
    uint8_t val = (uint8_t)json[JSON_KEY_HSV_COLOR]["VAL"];

    // Add new values to settings container
    ClockSettings["HUE"] = (uint16_t)map(hue, 0x0, 360, 0x0, 0xFFFF);
    ClockSettings["SAT"] = (uint8_t)map(sat, 0x0, 100, 0x0, 0xFF);
    ClockSettings["VAL"] = (uint8_t)map(val, 0x0, 100, 0x0, 0xFF);

    // Update color
    wordclock->updateColor(ClockSettings["HUE"], 
                           ClockSettings["SAT"], 
                           ClockSettings["VAL"]);
    wordclock->show();
  }
  else {
    DEBUG_MSG_LN("Unknown Key."); 
  }
}

/**
 * @brief Broadcast WebSocket Data
 *
 * @param obj JSON Object
 */
template <typename T>
void WebSocketSend(const char *key, T value)
{
  bool retVal;
  StaticJsonDocument<512> tempJDOC;
  String jsonString;

  tempJDOC[key] = value;
  (void)serializeJson(tempJDOC, jsonString);
  retVal = webSocket.broadcastTXT(jsonString);
  if(retVal == false) DEBUG_MSG_LN("Websocket broadcast failed!");
}

void WebSocketSend(const JsonDocument &doc)
{
  bool retVal;
  String jsonString;

  (void)serializeJson(doc, jsonString);
  retVal = webSocket.broadcastTXT(jsonString);
  if(retVal == false) DEBUG_MSG_LN("Websocket broadcast failed!");
}

/**
 * @brief Broadcast WebSocket Data
 *
 * @param obj JSON Object
 */
void WebSocketSend(String s)
{
  bool retVal;
  
  // Broadcast to all websocket clients
  retVal = webSocket.broadcastTXT(s);
  if(retVal == false) DEBUG_MSG_LN("Websocket broadcast failed!");
}

/**
 * @brief Callback for boot pin
 *
 */
void BootPinCbk()
{
  DEBUG_MSG_LN("-----Rebooting Wordclock-----");
  wordclock->powerOff();
  delay(100);
  //ESP.reset();
  ESP.rebootIntoUartDownloadMode();
}

/**
 * @brief Get the Version object
 *
 * @return String
 */
String GetVersion()
{
  return (String)(
      String(SOFTWARE_VERSION_MAYOR)+ "." +
      String(SOFTWARE_VERSION_MINOR)+ "." +
      String(SOFTWARE_VERSION_PATCH));
}

#if (DEBUG_MODE)
static inline void debugWebSocket(String msg)
{
  WebSocketSend<String>("DEBUG_MSG", msg);
}
#endif