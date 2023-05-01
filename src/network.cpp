/**********************************************************************************************************************
 * @file network.cpp
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
#include "network.h"
#include "network_cbk.h"
#include "mcal.h"
#include "version.h"
#include "html/root.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>  
#include <WebSocketsServer.h>
#ifdef USEOTA
#include "html/OTAWeb.h"
#include <LittleFS.h>
#include <ESP8266HTTPUpdateServer.h>
#endif

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/

typedef enum NetworkMode
{
    Network_Offline = 0x0u,
    Network_AP_Mode = 0x1u,
    Network_ServerMode = 0x2u,
    Network_ClientMode = 0x3u
}NetworkModeType;

//Static variables
const char* modes[] = { "NULL", "STA", "AP", "STA+AP" };
const char*           NTP_Server = "pool.ntp.org";
const char* mdns_name = "Wordclock";
const long            GTM_Offset_Sec = 3600; // +1h
const int             DayLight_Offset_Sec = 3600; // +0h winter time
//ESP8266WebServer      Server(80);
NetworkModeType Wifi_Mode = Network_Offline;
ESP8266WebServer Server(80);
WebSocketsServer ws = WebSocketsServer(81);
#ifdef USEOTA
ESP8266HTTPUpdateServer OTAServer;
#endif

/***********************************************************************************************************************
 * Local function declarations
 ***********************************************************************************************************************/
void handleRoot();
void handleNotFound();
#ifdef USEOTA
void OTA_Init();
#endif
void WiFiSettingsChanged();

/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/

/**
 * @brief Setup function
 * 
 */
void Wifi_Setup()
{
  bool retVal;
  // Set your Static IP address
  IPAddress local_IP(192, 168, 178, 90);
  IPAddress gateway(192, 168, 178, 1);
  IPAddress subnet(255, 255, 255, 255);

  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet);
  wifiManager.setAPStaticIPConfig(local_IP, gateway, subnet);
  wifiManager.setConnectTimeout(30);
  wifiManager.setTimeout(30);
  wifiManager.setSaveConfigCallback(&WiFiSettingsChanged);

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // then goes into a blocking loop awaiting configuration and will return success result
  retVal = wifiManager.autoConnect("ESP8266_Wordclock");
  if(!retVal) 
  {
    Serial.println("Failed to connect!");
    // ESP.restart();
  } 
  else 
  {
    // Successful connected to local wifi...
    Serial.println("connected!");

    // Configure html server
    Server.onNotFound(handleNotFound);
    Server.on("/", handleRoot);
    Server.on("/update", HTTP_GET, [&](){
      Server.sendHeader(F("Content-Encoding"), F("gzip"));
      Server.send_P(200, "text/html", (const char*)index_color_html_gz, (int)index_color_html_gz_len);
    });
    
    ws.onEvent([](char num, WStype_t type, uint8_t* payload, size_t length){
      String text;
      switch (type)
      {
      case WStype_DISCONNECTED:
        Serial.println("Client Disconnected");
        break;
      case WStype_CONNECTED:
        Serial.println("Client Connected");
        break;
      case WStype_TEXT:
        
        for(uint8_t i=0; i<length; i++)
        {
          text += (char)payload[i];
        }
        if(text=="btn_reset")
          ESP.restart();
        break;
      default:
        break;
      }
    });

    // Setup OTA server using HTML server
#ifdef USEOTA
    OTAServer.setup(&Server);
#endif

    // Starting HTML server
    Server.begin();
    ws.begin();
    Serial.print("HTTP server started on port 80: ");
    Serial.println(WiFi.localIP());
    if (MDNS.begin(mdns_name)) {
      Serial.print("DNS started: ");
      Serial.println("http://" + String(mdns_name) + ".local/");
    }
    MDNS.addService("http", "tcp", 80);

#ifdef USEOTA  
    Serial.printf("HTTPUpdateServer (OTA) ready! Open http://%s.local/update in your browser\n", mdns_name);
#endif
    Wifi_Mode = Network_ServerMode;
  }
}

/**
 * @brief Receive the current NTP time
 * 
 * @param timeinfo 
 */
void Wifi_Get_NtpTime(struct tm* timeinfo)
{ 
  // Init and get the time
  Serial.println("Load NTP Server Time.");
  configTime( GTM_Offset_Sec, 
              DayLight_Offset_Sec, 
              NTP_Server );

  if(!getLocalTime(timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  else 
  {
    Serial.println("Server time read successfully");
  }
}

/**
 * @brief Handle for http server
 * 
 */
void handleRoot() 
{
  String s = index_html;
  Server.send(200, "text/html", s);

  // tell if a client has connected
  Serial.println("Client connected on: " + Server.client().remoteIP().toString());
}

/**
 * @brief Handle for http server
 * 
 */
void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += Server.uri();
  message += "\nMethod: ";
  message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += Server.args();
  message += "\n";
  for (uint8_t i = 0; i < Server.args(); i++) {
    message += " " + Server.argName(i) + ": " + Server.arg(i) + "\n";
  }
  Server.send(404, "text/plain", message);
}

//===============================================
bool loadFromFS(String path, String dataType) {
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (LittleFS.exists(path)){
      File dataFile = LittleFS.open(path, "r");
      if (!dataFile) {
          handleNotFound();
          return false;
      }

      if (Server.streamFile(dataFile, dataType) != dataFile.size()) {
        Serial.println("Sent less data than expected!");
      }else{
          Serial.println("Page served!");
      }

      dataFile.close();
  }else{
      handleNotFound();
      return false;
  }
  return true;
}

void WiFiSettingsChanged()
{
  Serial.println("WiIi Settings have been changed!");
}

/**
 * @brief Execute http server handle
 * 
 */
void Wifi_ServerExec()
{
  if(Wifi_Mode != Network_Offline)
  {
    Server.handleClient();
    ws.loop();
    MDNS.update();

  }
}
