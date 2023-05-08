/**
 * @file wordclock.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include "wordclock.h"

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))


/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
/**
 * @brief Turn LED power off to safe energy
 * 
 */
void Wordclock::powerOff()
{
  digitalWrite(MCAL_LED_EN_PIN, LOW);
  clear();
}

/**
 * @brief Turn LED power on
 * 
 */
void Wordclock::powerOn()
{
  clear();
  digitalWrite(MCAL_LED_EN_PIN, HIGH);
  show();
}

/**
 * @brief Set LED on XY-Coordinate
 * 
 * @param x Matrix X-Position
 * @param y Matrix Y-Position
 * @param color LED color
 */
void Wordclock::setPixelColorXY(uint8_t x, uint8_t y, uint32_t c)
{
  uint16_t i;
  
  if(_matrixSerpentineLayout == false) 
  {
    if (_matrixVertical == false) 
    {
      i = (y * COLUMNS) + x;
    } 
    else 
    {
      i = ROWs * (COLUMNS - (x+1))+y;
    }
  }

  if( _matrixSerpentineLayout == true) 
  {
    if (_matrixVertical == false) 
    {
      if(y & 0x01) 
      {
        // Odd rows run backwards
        uint8_t reverseX = (COLUMNS - 1) - x;
        i = (y * COLUMNS) + reverseX;
      } 
      else 
      {
        // Even rows run forwards
        i = (y * COLUMNS) + x;
      }
    } 
    // vertical positioning
    else 
    { 
      if (x & 0x01) 
      {
        i = ROWs * (COLUMNS - (x+1))+y;
      } 
      else 
      {
        i = ROWs * (COLUMNS - x) - (y+1);
      }
    }
  }
 
  setPixelColor(i, c);
}

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