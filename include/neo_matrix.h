/**
 * @file neo_matrix.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NEO_MATRIX_H
#define _NEO_MATRIX_H

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
#include "Arduino.h"

#include "mcal.h"

#define MATRIX_SERPENTINELAYOUT (true)
#define MATRIX_VERTIACAL        (false)
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

class NeoMatrix 
{
private:
    Adafruit_NeoPixel led_handle = Adafruit_NeoPixel(WS2812_MAX_LEDS, WS2812_DATA_PIN, NEO_GRB + NEO_KHZ800);
    const bool kMatrixSerpentineLayout = MATRIX_SERPENTINELAYOUT;
    const bool kMatrixVertical = MATRIX_VERTIACAL;
    typedef struct WordClock_Text_DE
    {
        String Text;
        uint8_t FirstPixel;
    } Text_Type;
    const Text_Type LED_Text[NUM_OF_WORDS] = 
        { 
            {"ES",      0, }, //-->
            {"IST",     3, }, //-->
            {"UHR",     99,}, //<--
            {"VIRTEL",  26,}, //-->
            {"HALB",    41,}, //-->
            {"NACH",    38,}, //-->
            {"VOR",     45,}, //<--
            {"EIN",     61,}, //<--
            {"EINS",    60,}, //<--
            {"ZWEI",    62,}, //<--
            {"DREI",    67,}, //-->
            {"VIER",    77,}, //<--
            {"FÜNF",    73,}, //-->
            {"SECHS",   104}, //<--
            {"SIEBEN",  55,}, //<--
            {"ACHT",    89,}, //-->
            {"NEUN",    81,}, //<--
            {"ZEHN",    94,}, //-->
            {"ELF",     85,}, //<--
            {"ZWÖLF",   49,}, //-->
            {"ZWANZIG", 11} //<--
        };
        uint32_t Color;

    void DrawWord(const String s);
    void SetMinute(uint8_t min);
    String NumToString(const uint8_t num);
    void SetColorBrighness( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) ;
    
    
public:
    NeoMatrix() {
        
    }
    ~NeoMatrix()
    {
        PowerOff();
    };

    void Init() {
        Serial.println("Start NeoMatrix");
        pinMode(MCAL_LED_EN_PIN, OUTPUT);
        PowerOn();
        delay(100);
        Color = led_handle.Color(255, 255, 255);
        led_handle.begin();
        led_handle.clear();
        led_handle.show();
        led_handle.setBrightness(map(20, 0, 255, 0, 100));
        
    }
    void PowerOn();
    void PowerOff();
    void StartupLEDsTest(void);
    void ShowTime(uint8_t hour, uint8_t minute);
    void SetColor(uint32_t c);
    
    void SetLedXY(uint8_t x, uint8_t y, uint32_t color);
    void SetLed(uint8_t i, uint32_t color);
    void SetPixelColor(uint16_t n, uint32_t c) {
        led_handle.setPixelColor(n, c);
    }
    
    void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
        led_handle.setPixelColor(n, r, g, b);
    }
    void Update() {
        led_handle.show();
    }
    void Clear() {
        led_handle.clear();
    }

};


#endif //_NEO_MATRIX_H