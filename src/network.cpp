#include "network.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//Static variables
const char*           _ssid     = "";
const char*           _password = "";  
const char*           _ssid_AP     = "ESP8266_Wordclock";
const char*           _password_AP = "123456";  
const char*           NTP_Server = "pool.ntp.org";
const long            GTM_Offset_Sec = 3600; // +1h
const int             DayLight_Offset_Sec = 3600; // +0h winter time

//===============================================
void Wifi_Setup(const char *ssid, const char *pwd)
{
    _ssid = ssid;
    _password = pwd;
}

//===============================================
void Wifi_Get_NtpTime(struct tm* timeinfo)
{ 
  uint8_t timeout = 0;
  WiFi.begin(_ssid, _password);
  Serial.println("Try to connect to " + *_ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout++;
    if(timeout >= 40)
    {
      Serial.println("Wifi Timeout..");
      return;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime( GTM_Offset_Sec, 
              DayLight_Offset_Sec, 
              NTP_Server );

  if(!getLocalTime(timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


