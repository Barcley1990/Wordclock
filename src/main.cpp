
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
#include <Arduino_JSON.h>
#include <RtcDateTime.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WORDCLOCK_USE_WIFI (true)
// Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
#define WORDCLOCK_TIME_OFFSET (3600u)

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
static bool WiFi_Setup_Successful = false;
static uint32_t old_time_100 = 0u;
static uint32_t old_time_1000 = 0u;

const char* mdns_name = "Wordclock";

/***********************************************************************************************************************
 * Local function declarations and objects
 ***********************************************************************************************************************/
void Runnable_100_ms();
void Runnable_1000_ms();
void WebSocketReceive(uint8_t* payload, uint8_t length);
void WebSocketSend(String key, const void* data);
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

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


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
  Serial.println("");
  Serial.println("");
  Serial.println(header);
  Serial.println("");
  Serial.println("");
  Serial.println("--------------------------------------");
  Serial.print("\nVersion: ");
  Serial.println(GetVersion());
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
  Serial.print("RESET Info      : ");
  Serial.println(ESP.getResetInfo());
  Serial.print("COMPILED        : ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);

  Serial.println("--------------------------------------");
  Serial.println("Ende Setup");
  Serial.println("--------------------------------------");
  Serial.println("");
  delay(100);


#if (WORDCLOCK_USE_WIFI == true)
  // Initialize LittleFS
  Serial.print("Try to initialize LittleFS... ");
  //LittleFS.format();
  if (!LittleFS.begin())
  {
    Serial.println("-> An error has occurred while mounting LittleFS. End Setup!\n");
    return;
  }
  else
  {
    Serial.println("-> LittleFS mounted successfully\n");
  }


  // Setup Wifi manager
  WiFiManager wifiManager;
  //wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet);
  //wifiManager.setAPStaticIPConfig(local_IP, gateway, subnet);
  wifiManager.setConnectTimeout(10u);
  wifiManager.setSaveConfigCallback([](){
    Serial.println("WiIi Settings have been changed!");
  });

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name,
  // then goes into a blocking loop awaiting configuration and will return success result
  if(!wifiManager.autoConnect("ESP8266_Wordclock"))
  {
    Serial.println("Failed to connect!");
    return;
  }
  else
  {
    // Successful connected to local wifi...
    Serial.print("Successfully connected! IP is: ");
    Serial.println(WiFi.localIP());

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
    HtmlServer.serveStatic("/style.css", LittleFS, "/style.css");
    HtmlServer.serveStatic("/index.js", LittleFS, "/index.js");
    HtmlServer.serveStatic("/jquery.js", LittleFS, "/jquery.js");
    HtmlServer.serveStatic("/update", LittleFS, "/update.html");
    HtmlServer.serveStatic("/images/colorwheel5.png", LittleFS, "/images/colorwheel5.png");


    // Configure WebSocket Server
    webSocket.onEvent([](char num, WStype_t type, uint8_t* payload, size_t length){
      String version = GetVersion();
      switch (type)
      {
      case WStype_DISCONNECTED:
        Serial.println("Client Disconnected");
        break;
      case WStype_CONNECTED:
        Serial.println("Client Connected");
        WebSocketSend("version", &version);
        break;
      case WStype_TEXT:
        Serial.print("Data Received: ");
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
      Serial.print("DNS started: ");
      Serial.println("http://" + String(mdns_name) + ".local/");
    }
    MDNS.addService("http", "tcp", 80);

    Serial.print("HTTP server started on port 80: ");
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
  time_t epochTime = timeClient.getEpochTime();

  RtcDateTime dt;
  dt.InitWithUnix32Time((uint32_t)epochTime);

  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  // -->DEBUG
  Serial.print("Time ");
  Serial.print(dt.Hour());
  Serial.print(":");
  Serial.print(dt.Minute());
  Serial.print(":");
  Serial.print(dt.Second());
  Serial.print("\n");
#endif

  // Initialize led strip
  layout = new Layout_De_11x10();
  wordclock = new Wordclock(layout);

  wordclock->setRTCDateTime(dt);
  wordclock->powerOn();
  delay(100);
  wordclock->begin();
  wordclock->clear();
  wordclock->setBrightness(map(50, 0, 100, 0, 255));
  wordclock->show();
}

/**
 * @brief Main loop  function. Called from main()
 */
void loop()
{
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
  static uint16_t hue = 0u;
  static uint8_t value = 0u;
  float lux = wordclock->getAmbBrightness();
  if(lux != 0.0f) {
    //value = (uint8_t)map((uint8_t)lux,50,400,100,255);
  }
  value = 0xFFu;

  // Read RTC time
  RtcDateTime dt = wordclock->getRTCDateTime();

  // Convert date-time to send to html server
  char datetimeBuffer[25u] = "";
  sprintf(datetimeBuffer, "%04d/%02d/%02d %02d:%02d:%02d",
    dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());

  // Send data to html server
  String ambBrightness = (String)lux;
  WebSocketSend("Light", &ambBrightness);
  WebSocketSend("Time", &datetimeBuffer);

  // -->DEBUG
  Serial.print("Time ");
  Serial.print(dt.Hour());
  Serial.print(":");
  Serial.print(dt.Minute());
  Serial.print(":");
  Serial.print(dt.Second());
  Serial.print("\n");

  // change color over time
  hue += 100;
  wordclock->updateColor(hue, 150, value);
  wordclock->clear();
  wordclock->setTime(dt.Hour(), dt.Minute());
  wordclock->show();
}

/**
 * @brief
 *
 * @param payload
 * @param length
 */
void WebSocketReceive(uint8_t *payload, uint8_t length)
{
  String data;

  for(uint8_t i=0; i<length; i++) {
    data += *(char*)payload++;
  }
  Serial.println(data);

  if(data == "#esp_reset") {
    ESP.reset();
  }
  else if(data == "pwr_on_off") {
    Serial.println("Turn Leds on/off");
    wordclock->powerOff();
  }
  else if(data == "color_reset") {
    Serial.println("Reset Color"); 
  }
}

void WebSocketSend(String key, const void* data)
{
  JSONVar objects;
  String jsonString;

  // Build JSON object
  objects[key] = *(String*)data;;
  jsonString = JSON.stringify(objects);

  // Broadcast to all websocket clients
  webSocket.broadcastTXT(jsonString);
}

/**
 * @brief Callback for boot pin
 *
 */
void BootPinCbk()
{
  Serial.println("-----Rebooting Wordclock-----");
  wordclock->powerOff();
  delay(100);
  ESP.reset();
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