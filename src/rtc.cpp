#include "rtc.h"
#include "mcal.h"
#include <Arduino.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))

ThreeWire             myWire(MCAL_DAT_PIN, MCAL_CLK_PIN, MCAL_RST_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire>  Rtc(myWire);

void printDateTime(const RtcDateTime& dt);

void Rtc_Setup(struct tm* timeinfo)
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
            COUNTOF(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}