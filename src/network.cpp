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
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif


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
std::unique_ptr<ESP8266WebServer> Server;


//===============================================
//LOCAL FUNCTION DECLARATIONS
//===============================================
void handleRoot();
void handleNotFound();
#ifdef USEOTA
void OTA_Init();
#endif

//===============================================
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
  } 
  else 
  {
    Serial.println("connected...");

    Server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
    Server->on("/", handleRoot);
    //Server->on("/inline", []() {
    //  Server->send(200, "text/plain", "this works as well");
    //});
    Server->onNotFound(handleNotFound);
    Server->begin();
    Serial.println("HTTP server started");
    Serial.println(WiFi.localIP());
    Wifi_Mode = Network_ServerMode;
  }

#ifdef USEOTA
  OTA_Init();
#endif
}

//===============================================
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

//===============================================
void handleRoot() 
{
  Server->send(200, "text/plain", "hello from esp8266!");

  // tell if a client has connected
  String addy = Server->client().remoteIP().toString();
  Serial.println("Client connected on: " + addy);
}

//===============================================
void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += Server->uri();
  message += "\nMethod: ";
  message += (Server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += Server->args();
  message += "\n";
  for (uint8_t i = 0; i < Server->args(); i++) {
    message += " " + Server->argName(i) + ": " + Server->arg(i) + "\n";
  }
  Server->send(404, "text/plain", message);
}

//===============================================
#ifdef USEOTA
void OTA_Init()
{
  ArduinoOTA.setHostname("WordClock_OTA");
  ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready!");
}
#endif

//===============================================
void Wifi_ServerExec()
{
  if(Wifi_Mode != Network_Offline)
  {
    Server->handleClient();
  }

#ifdef USEOTA
  ArduinoOTA.handle();
#endif
}
