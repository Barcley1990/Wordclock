//=================================================
#include "wordclock.h"
#include "mcal.h"
#include "rtc.h"
#include "network.h"
#include "led.h"
#include "ascii.h"
#include <BH1750.h>
#include <Wire.h>

//=================================================
#define WORDCLOCK_BH1750_ADDR 0x23u

// Function Macros:
#define WORDCLOCK_12H_FORMAT(h) (((h)>12) ? ((h)-12) : (h))

//=================================================
BH1750 lightMeter(WORDCLOCK_BH1750_ADDR);
static bool Wordclock_BH1750_Intitialised = false;

//=================================================
void WordClock_Init()
{
    struct tm ntp_time;

    pinMode(MCAL_LED_EN_PIN, OUTPUT);
    Serial.println("");
    Serial.println("");
    Serial.println(header);
    Serial.flush();

    // Setup Wifi
    Wifi_Setup("TARDIS", "82uqFnUSjUn7YL");

    // Get NTP Server Time
    Wifi_Get_NtpTime(&ntp_time);

    // Setup RTC
    Rtc_Setup(&ntp_time);

    // Start AP
    Wifi_StartAP();

    // Setup Light Meter
    Wire.begin(MCAL_SDA_PIN, MCAL_SCL_PIN);
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println(F("Initialise BH1750"));
        Wordclock_BH1750_Intitialised = true;
    } else {
        Serial.println(F("Error initialising BH1750"));
    }

    // Print Lux
    if ((Wordclock_BH1750_Intitialised == true) && 
        (lightMeter.measurementReady() == true)) 
    {
        float lux = lightMeter.readLightLevel();
        Serial.print("Light: ");
        Serial.print(lux);
        Serial.println(" lx");
    }
}

//=================================================
void WordClock_Runnable_1s()
{  
    

    //Serial.println("1000ms");
    //pin_state = !pin_state;
    //digitalWrite(MCAL_LED_EN_PIN, pin_state);  

#ifdef DEBUG_MODE
    static uint8_t temp_h, temp_min;
    if(temp_h != DEBUG_HOUR || temp_min != DEBUG_MINUTE)
    {
        Serial.print("--DEBUG MODE-- Time set to: ");
        Serial.print(DEBUG_HOUR);
        Serial.print(":");
        Serial.println(DEBUG_MINUTE);
        LED_ShowTime(DEBUG_HOUR, DEBUG_MINUTE, 0);
        temp_h = DEBUG_HOUR;
        temp_min = DEBUG_MINUTE;
    }
    
#else
    RtcDateTime now = Rtc.GetDateTime();
    LED_ShowTime(WORDCLOCK_12H_FORMAT(now.Hour()), now.Minute(), now.Second());
#endif
}
