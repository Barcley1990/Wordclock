//=================================================
#include "wordclock.h"
#include <Arduino.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

//=================================================
#define WORDCLOCK_BOOT_PIN      0u
#define WORDCLOCK_WIFI_EN_PIN   4u
#define WORDCLOCK_LED_DIN_PIN   5u
#define WORDCLOCK_DAT_PIN       12u       
#define WORDCLOCK_CLK_PIN       13u
#define WORDCLOCK_SCL_PIN       14u
#define WORDCLOCK_RST_PIN       15u
#define WORDCLOCK_LED_EN_PIN    16u
#define WORDCLOCK_ADC_PIN       17u // Arduino A0


//=================================================
static bool           pin_state = 0;
const char*           ssid     = "TARDIS";
const char*           password = "82uqFnUSjUn7YL";
const char*           WordClock_NTP_Server = "pool.ntp.org";
const long            WordClock_GTM_Offset_Sec = 3600; // +1h
const int             WordClock_DayLight_Offset_Sec = 0; // +0h winter time
ThreeWire             myWire(WORDCLOCK_DAT_PIN, WORDCLOCK_CLK_PIN, WORDCLOCK_RST_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire>  Rtc(myWire);

//=================================================
void WordClock_Init()
{
    pinMode(WORDCLOCK_LED_EN_PIN, OUTPUT);
    Serial.println("12h Wordclock");
}

//=================================================
void WordClock_Runnable_1s()
{   
    digitalWrite(WORDCLOCK_LED_EN_PIN, !pin_state);  
}