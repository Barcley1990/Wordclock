//=================================================
#include "wordclock.h"
#include "network.h"
#include "led.h"
#include "ascii.h"
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

// Function Macros:
#define WORDCLOCK_COUNTOF(a) (sizeof(a) / sizeof(a[0]))
#define WORDCLOCK_12H_FORMAT(h) (((h)>12) ? ((h)-12) : (h))

void WordClock_SetUpRTC(struct tm* timeinfo);
void printDateTime(const RtcDateTime& dt);

//=================================================
static bool           pin_state = 0;
ThreeWire             myWire(WORDCLOCK_DAT_PIN, WORDCLOCK_CLK_PIN, WORDCLOCK_RST_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire>  Rtc(myWire);
Network               NTPTime("TARDIS", "82uqFnUSjUn7YL");

//=================================================
void WordClock_Init()
{
    struct tm ntp_time;

    pinMode(WORDCLOCK_LED_EN_PIN, OUTPUT);
    Serial.println("");
    Serial.println("");
    Serial.println(header);

    NTPTime.GetNTPTime(&ntp_time);
    WordClock_SetUpRTC(&ntp_time);

}

//=================================================
void WordClock_Runnable_1s()
{   
    Serial.println("1000ms");
    pin_state = !pin_state;
    digitalWrite(WORDCLOCK_LED_EN_PIN, pin_state);  

#ifdef DEBUG_MODE
    LED_ShowTime(DEBUG_HOUR, DEBUG_MINUTE, 0);
#else
    RtcDateTime now = Rtc.GetDateTime();
    LED_ShowTime(WORDCLOCK_12H_FORMAT(now.Hour()), now.Minute(), now.Second());
#endif
}

//=================================================
void WordClock_SetUpRTC(struct tm* timeinfo)
{
    Rtc.Begin();

    // Convert NTP Time structure into RtcDateTime obect
    RtcDateTime ntp_time = RtcDateTime(
      timeinfo->tm_year + 1900,
      timeinfo->tm_mon + 1,
      timeinfo->tm_mday,
      timeinfo->tm_hour,
      timeinfo->tm_min,
      timeinfo->tm_sec
    );

    Serial.print("Network time is: ");
    printDateTime(ntp_time);

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(ntp_time);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    // Get the stored date and time
    RtcDateTime now = Rtc.GetDateTime();
    Serial.print("RTC time is: ");
    printDateTime(now);

    if (now < ntp_time) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(ntp_time);
    }
    else if (now > ntp_time) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == ntp_time) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

//=================================================
void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            WORDCLOCK_COUNTOF(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}