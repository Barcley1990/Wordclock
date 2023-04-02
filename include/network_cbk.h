#include <Arduino.h>
#include <WiFiManager.h>  
//callbacks
  // called after AP mode and config portal has started
  //  setAPCallback( std::function<void(WiFiManager*)> func );
  // called after webserver has started
  //  setWebServerCallback( std::function<void()> func );
  // called when settings reset have been triggered
  //  setConfigResetCallback( std::function<void()> func );
  // called when wifi settings have been changed and connection was successful ( or setBreakAfterConfig(true) )
  //  setSaveConfigCallback( std::function<void()> func );
  // called when saving either params-in-wifi or params page
  //  setSaveParamsCallback( std::function<void()> func );
  // called when saving params-in-wifi or params before anything else happens (eg wifi)
  //  setPreSaveConfigCallback( std::function<void()> func );
  // called just before doing OTA update
  //  setPreOtaUpdateCallback( std::function<void()> func );

void saveWifiCallback(){
  Serial.println("[CALLBACK] saveCallback fired");
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("[CALLBACK] configModeCallback fired");
  // myWiFiManager->setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 
  // Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  // Serial.println(myWiFiManager->getConfigPortalSSID());
  // 
  // esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  // wm.stopConfigPortal();
}

void handlePreOtaUpdateCallback(){
  Update.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("CUSTOM Progress: %u%%\r", (progress / (total / 100)));
  });
}