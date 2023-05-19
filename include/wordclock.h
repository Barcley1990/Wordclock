/**
 * @file wordclock.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _WORDCLOCK_H
#define _WORDCLOCK_H

/*************************************************************
E S K I S T L F Ü N F    ==> ES IST FÜNF        0------------->10
Z E H N Z W A N Z I G    ==> ZEHN ZWANZIG       21<------------11
D R E I V I E R T E L    ==> DREI|VIERTEL       22------------>32
T G N A C H V O R J M    ==> NACH VOR           43<------------33
H A L B Q Z W Ö L F P    ==> HALB ZWÖLF         44------------>54
Z W E I N S I E B E N    ==> ZW|EI|N|S|IEBEN    65<------------55   
K D R E I R H F Ü N F    ==> DREI FÜNF          66------------>76
E L F N E U N V I E R    ==> ELF NEUN VIER      87<------------77
W A C H T Z E H N R S    ==> ACHT ZEHN          88------------>98 
B S E C H S F M U H R    ==> SECHS UHRx        109<------------99
**************************************************************/

/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Wire.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <Arduino.h>

#include "mcal.h"
#include "layouts/ilayout.h"

#define MATRIX_SERPENTINELAYOUT (true)
#define MATRIX_VERTIACAL        (false)
#define MATRIX_ROTATION         (2u) //0=0° 1=90° 2=180° 3=270°
// Set 'kMatrixSerpentineLayout' to false if your pixels are 
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'kMatrixSerpentineLayout' to true if your pixels are 
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15

#define WS2812_ENABLE_PIN       (MCAL_LED_EN_PIN)
#define WS2812_DATA_PIN         (MCAL_LED_DIN_PIN)
#define ROWs                    (10u)
#define COLUMNS                 (11u)
#define MINUTE_LEDS             (4u)
#define AMBILIGHT_LEDS          (0u)
#define WS2812_MAX_LEDS         ((COLUMNS * ROWs) + MINUTE_LEDS + AMBILIGHT_LEDS)
#define NUM_OF_WORDS            (21u)
#define PIXEL_S                 (56u)
#define ELEMENTS_IN_TEXT_TABLE  (sizeof(LED_Text) / sizeof(LED_Text[0]))
#define BH1750_ADDR             (0x23u)

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))
class Wordclock : public Adafruit_NeoPixel
{
private:
    const bool _matrixSerpentineLayout = true;
    const bool _matrixVertical = false;
    const uint8_t _matrixAngle = 0u;
    uint8_t _brightness = 30u;
    BH1750* _lightMeter = NULL;
    ThreeWire* _myWire = NULL;
    RtcDS1302<ThreeWire>* _rtc = NULL;
    ILayout* layout = nullptr;
    
    void printDateTime(const RtcDateTime& dt);
    
public:
    Wordclock(ILayout* layout);
    ~Wordclock();

    // Comon Functions
    void powerOn();
    void powerOff();

    // Clock Functions
    void setTime(uint8_t hour, uint8_t minute);

    // LED Functions
    void setPixelColorXY(uint8_t x, uint8_t y, uint32_t c);
    
    // RTC Functions
    void setRTCDateTime(RtcDateTime dt);
    RtcDateTime getRTCDateTime();
    
    // LDR Functions
    float getAmbBrightness();
};


#endif //_WORDCLOCK_H