#include "network.h"
#include "mcal.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


typedef enum NetworkMode
{
    Network_Offline = 0x0u,
    Network_AP_Mode = 0x1u,
    Network_ServerMode = 0x2u,
    Network_ClientMode = 0x3u
}NetworkModeType;

//Static variables
const char*           _ssid     = "";
const char*           _password = "";  
const char*           _ssid_AP     = "ESP8266_Wordclock";
const char*           _password_AP = "123456";  
const char*           NTP_Server = "pool.ntp.org";
const long            GTM_Offset_Sec = 3600; // +1h
const int             DayLight_Offset_Sec = 3600; // +0h winter time
ESP8266WebServer      Server(80);
NetworkModeType Wifi_Mode = Network_Offline;

//===============================================
//LOCAL FUNCTION DECLARATIONS
//===============================================
void Wifi_ClientStatus();



//===============================================
void Wifi_Setup(const char *ssid, const char *pwd)
{
    _ssid = ssid;
    _password = pwd;
    //pinMode(MCAL_WIFI_EN_PIN, INPUT);
}

//===============================================
void Wifi_Get_NtpTime(struct tm* timeinfo)
{ 
  uint8_t timeout = 0;

  WiFi.begin(_ssid, _password);
  Serial.println("Try to connect to " + *_ssid);
  delay(500);
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
  Serial.println("\nWiFi connected.");
  
  // Init and get the time
  Serial.println("Load NTP Server Time.");
  configTime( GTM_Offset_Sec, 
              DayLight_Offset_Sec, 
              NTP_Server );

  if(!getLocalTime(timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  else {Serial.println("Server time read successfully");}

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi disconnected.");
}

//===============================================
void handleRoot() {
  Server.send(200, "text/html", "<h1>Hello from ESP8266 AP!</h1>");
  String addy = Server.client().remoteIP().toString();
  Serial.println("Client connected on: " + addy);
}

void handleNotFound() {
  Server.send(200, "text/html", "<h1>Error 404 - Not Found</h1>");
}

//===============================================
void Wifi_StartAP()
{
  uint8 retVal = true;
  IPAddress local_IP(192,168,4,22);
  IPAddress gateway(192,168,4,9);
  IPAddress subnet(255,255,255,0);

  Serial.print("Setting soft-AP configuration ... ");
  retVal = !WiFi.softAPConfig(local_IP, gateway, subnet);
  Serial.println(retVal ? "Failed!" : "Ready!");
  Serial.print("Setting soft-AP ... ");
  retVal |= !WiFi.softAP(_ssid_AP);
  Serial.println(retVal ? "Failed!" : "Ready!");
  
  // false means no error has occured
  if(retVal == false)
  {
    Serial.print("Soft-AP IP address = ");
    Serial.print(WiFi.softAPIP());
    Server.on ( "/", handleRoot );
    Server.begin();
  	Serial.println("\nHTTP server started!");
    Wifi_Mode = Network_AP_Mode;
  }
  else
  {
    Serial.println("Starting AP failed...");
  }  
}

//===============================================
void Wifi_ServerExec()
{
  if(Wifi_Mode != Network_Offline)
  {
    Server.handleClient();
  }
}

void Wifi_ClientStatus()
{
    unsigned char number_client;
    struct station_info *stat_info;

    struct ip4_addr *IPaddress;
    IPAddress address;
    int i=1;

    number_client= wifi_softap_get_station_num();
    stat_info = wifi_softap_get_station_info();

    Serial.print(" Total connected_client are = ");
    Serial.println(number_client);

    while (stat_info != NULL) {
        IPaddress = &stat_info->ip;
        address = IPaddress->addr;

        Serial.print("client= ");

        Serial.print(i);
        Serial.print(" ip adress is = ");
        Serial.print((address));
        Serial.print(" with mac adress is = ");

        Serial.print(stat_info->bssid[0],HEX);
        Serial.print(stat_info->bssid[1],HEX);
        Serial.print(stat_info->bssid[2],HEX);
        Serial.print(stat_info->bssid[3],HEX);
        Serial.print(stat_info->bssid[4],HEX);
        Serial.print(stat_info->bssid[5],HEX);

        stat_info = STAILQ_NEXT(stat_info, next);
        i++;
        Serial.println();
    }
}
