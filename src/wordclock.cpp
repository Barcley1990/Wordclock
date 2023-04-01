//=================================================
#include "wordclock.h"
#include "mcal.h"
#include "rtc.h"
#include "network.h"
#include "led.h"
#include "ascii.h"

//=================================================

// Function Macros:
#define WORDCLOCK_12H_FORMAT(h) (((h)>12) ? ((h)-12) : (h))

//=================================================

//=================================================
void WordClock_Init()
{
    struct tm ntp_time;

    pinMode(MCAL_LED_EN_PIN, OUTPUT);
    Serial.println("");
    Serial.println("");
    Serial.println(header);

    Wifi_Setup("TARDIS", "82uqFnUSjUn7YL");
    Wifi_Get_NtpTime(&ntp_time);
    Rtc_Setup(&ntp_time);
}

//=================================================
void WordClock_Runnable_1s()
{   static uint8 pin_state;

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
