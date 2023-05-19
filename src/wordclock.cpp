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



/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
Wordclock::Wordclock(ILayout* layout) : Adafruit_NeoPixel(layout->getMatrixCols()*layout->getMatrixRows(), MCAL_LED_DIN_PIN)
{
    Serial.println("Initializing Wordclock");
    _lightMeter = new BH1750(BH1750_ADDR);
    _myWire = new ThreeWire(MCAL_DAT_PIN, MCAL_CLK_PIN, MCAL_RST_PIN);
    _rtc = new RtcDS1302<ThreeWire>(*_myWire);

    Wire.begin(MCAL_SDA_PIN, MCAL_SCL_PIN);
    _lightMeter->begin(BH1750::CONTINUOUS_HIGH_RES_MODE) ? 
        Serial.println(F("BH1750 initialised")) :
        Serial.println(F("Error initialising BH1750"));
    _rtc->Begin();
    layout = layout;
}
Wordclock::~Wordclock()
{
    powerOff();
    delete(_lightMeter);
    delete(_myWire);
    delete(_rtc);
};

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
 * @brief 
 * 
 */
void Wordclock::setTime(uint8_t h, uint8_t m)
{
  // Update "matrix" depending on the loaded layout
  layout->setMatrixTime(ESIST);
  switch (h)
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12: 
  default:
    break;
  }
  
  //Get matrix and set correspondnding pixels
  for(uint8_t x=0; x<layout->getMatrixCols(); x++)
  {
    for(uint8_t y=0; y<layout->getMatrixRows(); y++)
    {
      if(layout->getMatrixPixel(x,y) == SET) 
      {
        setPixelColorXY(x,y,Color(100,100,100));
      }
      else 
      {
        setPixelColorXY(x,y,0);
      }
    }
  }
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

/**
 * @brief Get RTC Date Time
 * 
 * @return RtcDateTime 
 */
RtcDateTime Wordclock::getRTCDateTime()
{
  RtcDateTime dummy(0);
  // Check if object is valid
  if(_rtc == nullptr) return dummy;
  // return date time object
  return _rtc->GetDateTime();
}

/**
 * @brief Update RTC with date time
 * 
 * @param dt 
 */
void Wordclock::setRTCDateTime(RtcDateTime dt)
{
  // Check if object is valid
  if(_rtc == nullptr) return;
  // set date time object
  _rtc->SetDateTime(dt);
}

/**
 * @brief Returns light level in LUX
 * 
 * @return float light level
 */
float Wordclock::getAmbBrightness()
{
  float retVal = 0.0f;
  if(_lightMeter == nullptr) return retVal;

  if(_lightMeter->measurementReady())
  {
    retVal = _lightMeter->readLightLevel();
  }

  return retVal;
}


//******************************************************************
// PRIVATE FUNCTIONS
/**
 * @brief 
 * 
 * @param dt 
 */
void Wordclock::printDateTime(const RtcDateTime& dt)
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