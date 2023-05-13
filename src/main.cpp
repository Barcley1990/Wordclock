
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

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>  
#include <WebSocketsServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>


/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
static bool WiFi_Setup_Successful = false;
static uint32_t old_time_100 = 0u;

const char* mdns_name = "Wordclock";

/***********************************************************************************************************************
 * Local function declarations and objects
 ***********************************************************************************************************************/
void Runnable_100_ms();
void WebSocketReceive(uint8_t* payload, uint8_t length);
void BootPinCbk();
String GetVersion();

IPAddress local_IP(192, 168, 178, 90);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 255);
ESP8266WebServer HtmlServer(80);
WebSocketsServer WebSocketServer = WebSocketsServer(81);
ESP8266HTTPUpdateServer OTAServer;
Wordclock wordclock;
Debounce bootButton(MCAL_BOOT_PIN, 5000u, 100u, BootPinCbk);

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
  digitalWrite(MCAL_LED_EN_PIN, LOW);

  // Start serial inteface
  Serial.begin(115200);
  delay(100);

  // Print software version
  Serial.println("");
  Serial.println("");
  Serial.println(header);
  Serial.print("\nVersion: ");
  Serial.println(GetVersion());
  Serial.println("");
  Serial.println("");
  delay(100);

  // Initialize LittleFS
  Serial.print("Try to initialize LittleFS ");
  //LittleFS.format();
  if (!LittleFS.begin())
  {
    Serial.println("->An error has occurred while mounting LittleFS. End Setup!");
    return;
  }
  else
  {
    Serial.println("->LittleFS mounted successfully");
  }

  // Setup Wifi manager
  WiFiManager wifiManager;
  //wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet);
  //wifiManager.setAPStaticIPConfig(local_IP, gateway, subnet);
  wifiManager.setConnectTimeout(5);
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
    HtmlServer.serveStatic("/update", LittleFS, "/index_color.html");

    // Configure WebSocket Server
    WebSocketServer.onEvent([](char num, WStype_t type, uint8_t* payload, size_t length){
      String text;
      JSONVar sliderValues;
      String jsonString;
      switch (type)
      {
      case WStype_DISCONNECTED:
        Serial.println("Client Disconnected");
        break;
      case WStype_CONNECTED:
        Serial.println("Client Connected");
        break;
      case WStype_TEXT:
        WebSocketReceive(payload, length);
        break;
      default:
        break;
      }
    });

    // Initialize servers (HTML, WS, OTA, mDNS)
    OTAServer.setup(&HtmlServer);
    HtmlServer.begin();
    WebSocketServer.begin();
    if (MDNS.begin(mdns_name)) {
      Serial.print("DNS started: ");
      Serial.println("http://" + String(mdns_name) + ".local/");
    }
    MDNS.addService("http", "tcp", 80);

    Serial.print("HTTP server started on port 80: ");
    Serial.printf("HTTPUpdateServer (OTA) ready! Open http://%s.local/update in your browser\n", mdns_name);
  }
  WiFi_Setup_Successful = true;
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

  // Execute Server
  if(WiFi_Setup_Successful == true)
  {
    HtmlServer.handleClient();
    WebSocketServer.loop();
    MDNS.update();
  }
}

/**
 * @brief called every 10ms
 * 
 */
void Runnable_100_ms()
{
  wordclock.rainbow(0, 10);
  wordclock.show();
  bootButton.poll();
}

/**
 * @brief 
 * 
 * @param payload 
 * @param length 
 */
void WebSocketReceive(uint8_t *payload, uint8_t length)
{

}

/**
 * @brief Callback for boot pin
 * 
 */
void BootPinCbk()
{
  Serial.println("-----Shut Down-----");
  wordclock.powerOff();
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