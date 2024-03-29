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
#include <NTPClient.h>


/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
Wordclock::Wordclock(ILayout* layout) :
  Adafruit_NeoPixel((layout->getMatrixCols()*layout->getMatrixRows()) + 4u , MCAL_LED_DIN_PIN)
{
  Serial.println("Initializing Wordclock");

  // Initialize color on startup (Apply gamma correction)
  _colorHSV = gamma32(ColorHSV(Color::MAGENTA, _saturation, _brightness));

  // Apply layout
  _layout = layout;

  // Create LDR object
  _lightMeter = new BH1750(BH1750_ADDR);

  // Create RTC object
  _myWire = new ThreeWire(MCAL_DAT_PIN, MCAL_CLK_PIN, MCAL_RST_PIN);
  _rtc = new RtcDS1302<ThreeWire>(*_myWire);

  // Start RTC and LDR
  Wire.begin(MCAL_SDA_PIN, MCAL_SCL_PIN);
  _lightMeter->begin(BH1750::CONTINUOUS_HIGH_RES_MODE) ?
      Serial.println(F("BH1750 initialised")) :
      Serial.println(F("Error initialising BH1750"));
  _rtc->Begin();
  checkRTCTime();
}

Wordclock::~Wordclock()
{
    powerOff();
    delete(_lightMeter);
    delete(_myWire);
    delete(_rtc);
    delete(_layout);
};

/**
 * @brief Turn LED power off to safe energy
 *
 */
void Wordclock::powerOff()
{
  digitalWrite(MCAL_LED_EN_PIN, LOW);
  clear();
  _ledPowerState = false;
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
  _ledPowerState = true;
}

/**
 * @brief
 *
 */
void Wordclock::setTime(uint8_t h, uint8_t m)
{
  uint8_t min = m % 10u;
  uint8_t rows = _layout->getMatrixRows();
  uint8_t cols = _layout->getMatrixCols();
  uint8_t numleds = rows * cols;
  bool isFullHour = false;

  //Set min pixels
  if(min > 5u) min -= 5u;
  switch (min)
  {
    case 4: setPixelColor(numleds+0u, _colorHSV);
    case 3: setPixelColor(numleds+1u, _colorHSV);
    case 2: setPixelColor(numleds+2u, _colorHSV);
    case 1: setPixelColor(numleds+3u, _colorHSV);
    default: break;
  }

  if(_layout == nullptr) return;
  // Update "matrix" depending on the loaded layout
  clearFrame();
  _layout->setMatrixTerm(Terms::ESIST);

  // Set Minutes
  switch (m)
  {
    case 0: isFullHour = true; break;
    case 1: isFullHour = true; break;
    case 2: isFullHour = true; break;
    case 3: isFullHour = true; break;
    case 4: isFullHour = true; break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      _layout->setMatrixTerm(Terms::FUENF_1);
      _layout->setMatrixTerm(Terms::NACH);
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      _layout->setMatrixTerm(Terms::ZEHN_1);
      _layout->setMatrixTerm(Terms::NACH);
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      _layout->setMatrixTerm(Terms::VIRTEL);
      _layout->setMatrixTerm(Terms::NACH);
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      _layout->setMatrixTerm(Terms::ZWANZIG);
      _layout->setMatrixTerm(Terms::NACH);
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      _layout->setMatrixTerm(Terms::FUENF_1);
      _layout->setMatrixTerm(Terms::VOR);
      _layout->setMatrixTerm(Terms::HALB);
      h += 1; //Offset Hour
      break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
      _layout->setMatrixTerm(Terms::HALB);
      h += 1; //Offset Hour
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      _layout->setMatrixTerm(Terms::FUENF_1);
      _layout->setMatrixTerm(Terms::NACH);
      _layout->setMatrixTerm(Terms::HALB);
      h += 1; //Offset Hour
      break;
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
      _layout->setMatrixTerm(Terms::ZEHN_1);
      _layout->setMatrixTerm(Terms::NACH);
      _layout->setMatrixTerm(Terms::HALB);
      h += 1; //Offset Hour
      break;
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
      if(random(0u,1u) == 1u)
      {
        _layout->setMatrixTerm(Terms::DREIVIRTEL);
      }
      else
      {
        _layout->setMatrixTerm(Terms::VIRTEL);
        _layout->setMatrixTerm(Terms::VOR);
      }
      h += 1; //Offset Hour
      break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
      _layout->setMatrixTerm(Terms::ZEHN_1);
      _layout->setMatrixTerm(Terms::VOR);
      h += 1; //Offset Hour
      break;
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
      _layout->setMatrixTerm(Terms::FUENF_1);
      _layout->setMatrixTerm(Terms::VOR);
      h += 1; //Offset Hour
      break;
    default: break;
  }

  // Set Hours
  switch (h % 12u)
  {
    case 0: _layout->setMatrixTerm(Terms::ZWOELF); break;
    case 1: _layout->setMatrixTerm(Terms::EIN); break;
    case 2: _layout->setMatrixTerm(Terms::ZWEI); break;
    case 3: _layout->setMatrixTerm(Terms::DREI_2); break;
    case 4: _layout->setMatrixTerm(Terms::VIER_2); break;
    case 5: _layout->setMatrixTerm(Terms::FUENF_2); break;
    case 6: _layout->setMatrixTerm(Terms::SECHS); break;
    case 7: _layout->setMatrixTerm(Terms::SIEBEN); break;
    case 8: _layout->setMatrixTerm(Terms::ACHT); break;
    case 9: _layout->setMatrixTerm(Terms::NEUN); break;
    case 10: _layout->setMatrixTerm(Terms::ZEHN_2); break;
    case 11: _layout->setMatrixTerm(Terms::ELF); break;
    case 12: _layout->setMatrixTerm(Terms::ZWOELF); break;
    default: break;
  }

  // Set only at full hour
  if(isFullHour)
  {
    _layout->setMatrixTerm(Terms::UHR);
  }

  //Get matrix and set correspondnding pixels
  updateColor(_colorHSV);
}

/**
 *
*/
void Wordclock::clearFrame()
{
  _layout->clearMatrix();
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
  uint8_t cols = _layout->getMatrixCols();
  uint8_t rows = _layout->getMatrixRows();
  uint16_t maxElements = (cols*rows)-1;
  uint16_t i = 0;
  //Serial.print(x);
  //Serial.print(":");
  //Serial.println(y);
  //Serial.println(c);

  if(_matrixSerpentineLayout == true)
  {
    if((_matrixVertical == true) && (_matrixHorizontal == true))
    {
      if(y & 0x01)
      {
        // Odd rows
        i = ((rows-1-y)*cols)+cols-1-x;
        //Serial.print("odd v: ");
        //Serial.print(i);
        //Serial.print("->odd h: ");
        //Serial.println(i);
      }
      else
      {
        // Even rows
        i = (maxElements - (x + (y * cols)));
        //Serial.print("even v: ");
        //Serial.print(i);
        i += -2*(5-x);
        //Serial.print("->even h: ");
        //Serial.println(i);
      }

    }
    //Serial.println("");
  }

  setPixelColor(i, c);
}

void Wordclock::updateColor(uint32_t color)
{
  uint8_t rows = _layout->getMatrixRows();
  uint8_t cols = _layout->getMatrixCols();
  uint8_t numleds = rows * cols;
  
  // Update HSV color in case function was called from outside
  _colorHSV = color;

  setPixelColor(numleds+0u, _colorHSV);
  setPixelColor(numleds+1u, _colorHSV);
  setPixelColor(numleds+2u, _colorHSV);
  setPixelColor(numleds+3u, _colorHSV);

  for(uint8_t y=0u; y<_layout->getMatrixRows(); y++)
  {
    for(uint8_t x=0u; x<_layout->getMatrixCols(); x++)
    {
      if(_layout->getMatrixPixel(x,y) == true)
      {
        setPixelColorXY(x, y, _colorHSV);
      }
      else
      {
        setPixelColorXY(x, y, 0u);
      }
    }
  }
}

/**
 * @brief updateColor
 * 
 * @param h Hue 
 * @param s Saturation
 * @param v Brightness
 */
void Wordclock::updateColor(uint16_t h, uint8_t s, uint8_t v)
{
  // Apply gamma correction 
  _colorHSV = gamma32(ColorHSV(h, s, v));

  for(uint8_t y=0u; y<_layout->getMatrixRows(); y++)
  {
    for(uint8_t x=0u; x<_layout->getMatrixCols(); x++)
    {
      if(_layout->getMatrixPixel(x,y) == true)
      {
        setPixelColorXY(x,y,_colorHSV);
      }
      else
      {
        setPixelColorXY(x,y,0u);
      }
    }
  }
}

/**
 * @brief Check RTC
 *
 * @return RtcDateTime
 */
void Wordclock::checkRTCTime()
{
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  RtcDateTime now;

  Serial.println("Checking RTC...");
  if(_rtc != nullptr)
  {
    if (!_rtc->IsDateTimeValid())
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        _rtc->SetDateTime(compiled);
    }

    if (_rtc->GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        _rtc->SetIsWriteProtected(false);
    }

    if (!_rtc->GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        _rtc->SetIsRunning(true);
    }

    now =  _rtc->GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time! (Updating DateTime)");
        _rtc->SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
    Serial.print("RTC time: ");
    printDateTime(now);
    Serial.print("Compile time: ");
    printDateTime(compiled);
  }
  else
  {
    Serial.println("Error");
  }
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
  if(_rtc == nullptr) {
    Serial.println("Retreiving time failed");
    return dummy;
  }
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
  if(_rtc == nullptr) {
    Serial.println("Setting time failed!");
    return;
  }
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