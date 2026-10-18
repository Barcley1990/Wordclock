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
#include "clock_animation.h"

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
#define BH1750_ADDR             (0x23u)

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))

class Color
{
public:
    static const uint16_t RED = 0;
    static const uint16_t YELLOW = 65536/6;
    static const uint16_t GREEN = 65536/3;
    static const uint16_t CYAN = 65536/2;
    static const uint16_t BLUE = 65536*2/3;
    static const uint16_t MAGENTA = 65536*5/6;
};
class Wordclock : public Adafruit_NeoPixel
{
private:
    const bool _matrixSerpentineLayout = true;
    const bool _matrixVertical = true;
    const bool _matrixHorizontal = true;
    const uint8_t _matrixAngle = 0u;
    bool _ledPowerState = false;;
    uint8_t _brightness = 0xFFu/2u;
    uint8_t _saturation = 0xFFu/2u;
    uint8_t _hue = 0u;
    uint32_t _colorHSV;
    BH1750* _lightMeter = nullptr;
    ThreeWire* _myWire = nullptr;
    RtcDS1302<ThreeWire>* _rtc = nullptr;
    ILayout* _layout = nullptr;
    ClockAnimation::Animator _animator;
    
    /**
     * @brief Print date/time to Serial.
     * @param dt Date/time to print.
     */
    void printDateTime(const RtcDateTime& dt);
    /**
     * @brief Validate RTC state and update if required.
     */
    void checkRTCTime();
    
public:
    /**
     * @brief Construct a new Wordclock.
     * @param layout Layout implementation for the matrix.
     */
    Wordclock(ILayout* layout);
    /**
     * @brief Destroy the Wordclock instance.
     */
    ~Wordclock();

    // Comon Functions
    /**
     * @brief Turn LED power on.
     */
    void powerOn();
    /**
     * @brief Turn LED power off.
     */
    void powerOff();
    /**
     * @brief Get LED power state.
     * @return true if powered on.
     */
    bool getPowerState() {
        return _ledPowerState;
    };
    /**
     * @brief Get current hue.
     * @return Hue value.
     */
    uint8_t getHue() {
        return _hue;
    };
    /**
     * @brief Get current saturation.
     * @return Saturation value.
     */
    uint8_t getSaturation() {
        return _saturation;
    };
    /**
     * @brief Get current brightness.
     * @return Brightness value.
     */
    uint8_t getBrightness() {
        return _brightness;
    };
    /**
     * @brief Get current HSV color.
     * @return HSV color value.
     */
    uint32_t getHSVColor() {
        return _colorHSV;
    }

    // Clock Functions
    /**
     * @brief Set displayed time.
     * @param hour Hour.
     * @param minute Minute.
     */
    void setTime(uint8_t hour, uint8_t minute);
    /** Render at most one animation frame; call frequently from loop(). */
    void animate(uint32_t now);
    void setAnimation(ClockAnimation::Effect effect) { _animator.setEffect(effect); }
    void setBrightness(uint8_t brightness) {
        _brightness = brightness;
        Adafruit_NeoPixel::setBrightness(brightness);
        _animator.requestRedraw();
    }
    /**
     * @brief Clear internal frame buffer.
     */
    void clearFrame();

    // LED Functions
    /**
     * @brief Set pixel color at XY coordinate.
     * @param x Matrix X position.
     * @param y Matrix Y position.
     * @param c Color value.
     */
    void setPixelColorXY(uint8_t x, uint8_t y, uint32_t c);
    /**
     * @brief Update active pixels with given color.
     * @param color HSV color value.
     */
    void updateColor(uint32_t color);
    /**
     * @brief Update active pixels with HSV values.
     * @param h Hue.
     * @param s Saturation.
     * @param v Brightness.
     */
    void updateColor(uint16_t h, uint8_t s, uint8_t v);
    
    // RTC Functions
    /**
     * @brief Set RTC date/time.
     * @param dt Date/time.
     */
    void setRTCDateTime(RtcDateTime dt);
    /**
     * @brief Get RTC date/time.
     * @return Current date/time.
     */
    RtcDateTime getRTCDateTime();

    // NTP
    /**
     * @brief Fetch date/time via NTP (if implemented).
     */
    void getNTPDateTime();
    
    // LDR Functions
    /**
     * @brief Get ambient brightness in lux.
     * @return Ambient brightness.
     */
    float getAmbBrightness();
};


#endif //_WORDCLOCK_H
