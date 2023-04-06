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
#include "html/root.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>  

#ifdef USEOTA
#include <ESP8266mDNS.h>
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
const long            GTM_Offset_Sec = 3600; // +1h
const int             DayLight_Offset_Sec = 3600; // +0h winter time
//ESP8266WebServer      Server(80);
NetworkModeType Wifi_Mode = Network_Offline;
ESP8266WebServer Server(80);
#ifdef USEOTA
ESP8266HTTPUpdateServer httpUpdater;
#endif

/***********************************************************************************************************************
 * Local function declarations
 ***********************************************************************************************************************/
void handleRoot();
void handleNotFound();
#ifdef USEOTA
void OTA_Init();
#endif

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

  WiFiManager wifiManager;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // then goes into a blocking loop awaiting configuration and will return success result
  retVal = wifiManager.autoConnect("ESP8266_Wordclock");
  if(!retVal) 
  {
    Serial.println("Failed to connect");
    // ESP.restart();
    while(1){;} 
  } 
  else 
  {
    Serial.println("connected...");

    Server.on("/", handleRoot);
    //Server->on("/inline", []() {
    //  Server->send(200, "text/plain", "this works as well");
    //});
    Server.onNotFound(handleNotFound);
#ifdef USEOTA
    httpUpdater.setup(&Server);
#endif
    Server.begin();
    Serial.print("HTTP server started on port 80: ");
    Serial.println(WiFi.localIP());
#ifdef USEOTA
    MDNS.begin("WordClock_Webupdate");
    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer (OTA) ready! Open http://%s.local/update in your browser\n", "WordClock_Webupdate");
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
  Server.send(200, "text/plain", "hello from esp8266!");

  // tell if a client has connected
  String addy = Server.client().remoteIP().toString();
  Serial.println("Client connected on: " + addy);
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



/**
 * @brief Execute http server handle
 * 
 */
void Wifi_ServerExec()
{
  if(Wifi_Mode != Network_Offline)
  {
    Server.handleClient();
    #ifdef USEOTA
      MDNS.update();
    #endif
  }
}
