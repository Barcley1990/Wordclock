//=================================================
#include "wordclock.h"
#include "network.h"
#include "ascii.h"
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
ThreeWire             myWire(WORDCLOCK_DAT_PIN, WORDCLOCK_CLK_PIN, WORDCLOCK_RST_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire>  Rtc(myWire);
Network               network();

//=================================================
void WordClock_Init()
{
    pinMode(WORDCLOCK_LED_EN_PIN, OUTPUT);
    Serial.println("");
    Serial.println("");
    Serial.println(header);
}

//=================================================
void WordClock_Runnable_1s()
{   
    Serial.println("1000ms");
    pin_state = !pin_state;
    digitalWrite(WORDCLOCK_LED_EN_PIN, pin_state);  
}