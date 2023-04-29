/**
 * @file wordclock.cpp
 * @author Tobias Nuß
 * @brief 
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023
 * 
 * Microcontroller ESP8266
 * Module WROOM-02
 * Frequency 80MHz
 * Flash 2MB
 * RAM 80KB
 * Vendor Espressif
 * 
 */



/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include "wordclock.h"
#include "mcal.h"
#include "rtc.h"
#include "network.h"
#include "led.h"
#include "ascii.h"
#include <BH1750.h>
#include <Wire.h>

//=================================================
#define WORDCLOCK_BH1750_ADDR (0x23u)
#define WORDCLOCK_EEPROM_SIZE (512u)
#define WORDCLOCK_EEPROM_SSID_ADDR (0x0u);
#define WORDCLOCK_EEPROM_SSID_SIZE (0x8u);
#define WORDCLOCK_EEPROM_PWD_ADDR (0x8u);
#define WORDCLOCK_EEPROM_PWD_SIZE (0x8u);

// Function Macros:
#define WORDCLOCK_12H_FORMAT(h) (((h)>12) ? ((h)-12) : (h))

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
BH1750 lightMeter(WORDCLOCK_BH1750_ADDR);
static bool Wordclock_BH1750_Intitialised = false;

/***********************************************************************************************************************
 * Function declarations
 ***********************************************************************************************************************/
String WordClock_GetVersion();

/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/

/**
 * @brief WordClock_Init
 * 
 */
void WordClock_Init()
{
    // local variable for time data
    struct tm ntp_time;

    // Print software version
    Serial.println("");
    Serial.println("");
    Serial.println(header);
    Serial.print("\nVersion: ");
    Serial.println(WordClock_GetVersion());
    Serial.println("Start wordclock");
    delay(100);

    // Setup Wifi
    Wifi_Setup();

    // Get NTP Server Time
    Wifi_Get_NtpTime(&ntp_time);

    // Setup RTC
    Rtc_Setup(&ntp_time);

    // Setup Light Meter
    Wire.begin(MCAL_SDA_PIN, MCAL_SCL_PIN);
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println(F("Initialise BH1750"));
        Wordclock_BH1750_Intitialised = true;
    } else {
        Serial.println(F("Error initialising BH1750"));
    }

    // Print Lux
    delay(500);
    if ((Wordclock_BH1750_Intitialised == true) && 
        (lightMeter.measurementReady() == true)) 
    {
        float lux = lightMeter.readLightLevel();
        Serial.print("Light: ");
        Serial.print(lux);
        Serial.println(" lx");
    }
    else
    {
        Serial.println("Light meassurement failed");
    }

    // Initialize LEDs
    Serial.println("LEDS init");
    LED_Init(255, 160, 140, 20);
}

/**
 * @brief Called every second
 * 
 */
void WordClock_Runnable_1s()
{   

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

//===============================================
String WordClock_GetVersion()
{
    return (String)(
        String(SOFTWARE_VERSION_MAYOR)+ "." + 
        String(SOFTWARE_VERSION_MINOR)+ "." + 
        String(SOFTWARE_VERSION_PATCH));
}
