/**
 * @file neo_matrix.cpp
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
#include "neo_matrix.h"




/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/


/**
 * @brief Turn LED power off to safe energy
 * 
 */
void NeoMatrix::PowerOff()
{
  digitalWrite(MCAL_LED_EN_PIN, LOW);
  led_handle.clear();
}

/**
 * @brief Turn LED power on
 * 
 */
void NeoMatrix::PowerOn()
{
  led_handle.clear();
  digitalWrite(MCAL_LED_EN_PIN, HIGH);
  led_handle.show();
}

/**
 * @brief StartupLEDsTest
 * 
 */
void NeoMatrix::StartupLEDsTest(void) 
{

}

/**
 * @brief LED_ShowTime
 * 
 * @param hour 
 * @param minute 
 * @param second 
 */
void NeoMatrix::ShowTime(uint8_t hour, uint8_t minute)
{
  bool rand = random(0,1);

  led_handle.clear();
  DrawWord("ES");
  DrawWord("IST");

  // Es ist 
  if((minute >= 0u) && (minute < 5u))
  {
    SetMinute(minute);
    DrawWord(NumToString(hour));
  }
  //Es ist 5 nach
  else if((minute >= 5u) && (minute < 10u))
  {
    SetMinute(minute-5);
    DrawWord("FÜNF");
    DrawWord("NACH");
    DrawWord(NumToString(hour));
  }
  // Es ist 10 nach
  else if((minute >= 10u) && (minute < 15u))
  {
    SetMinute(minute-10);
    DrawWord("ZEHN");
    DrawWord("NACH");
    DrawWord(NumToString(hour));
  }
  //Es ist virtel nach
  else if((minute >= 15u) && (minute < 20u))
  {
    SetMinute(minute-15u);
    DrawWord("VIRTEL");
    DrawWord("NACH");
    DrawWord(NumToString(hour));
  }
  // Es ist 10 vor halb / Es ist zwanzig nach
  else if((minute >= 20) && (minute < 25))
  {
    SetMinute(minute-20);
    if(rand){
      DrawWord("ZWANZIG");
      DrawWord("NACH");
      DrawWord(NumToString(hour));
    }
    else {
      DrawWord("ZEHN");
      DrawWord("VOR");
      DrawWord("HALB");
      DrawWord(NumToString(hour+1));
    }
  }
  // Es ist 5 vor halb
  else if((minute >= 25) && (minute < 30))
  {
    SetMinute(minute-25);
    DrawWord("FÜNF");
    DrawWord("VOR");
    DrawWord("HALB");
    DrawWord(NumToString(hour+1));
  }
  // Es ist halb
  else if((minute >= 30) && (minute < 35))
  {
    SetMinute(minute-25);
    DrawWord("HALB");
    DrawWord(NumToString(hour+1));
  }
  // Es ist 5 nach halb
  else if((minute >= 35) && (minute < 40))
  {
    SetMinute(minute-35);
    DrawWord("FÜNF");
    DrawWord("NACH");
    DrawWord("HALB");
    DrawWord(NumToString(hour+1));
  }
  // Es ist 10 nach halb
  else if((minute >= 40) && (minute < 45))
  {
    SetMinute(minute-40);
    DrawWord("ZEHN");
    DrawWord("NACH");
    DrawWord("HALB");
    DrawWord(NumToString(hour+1));
  }
  // Es ist dreivirtel
  else if((minute >= 45) && (minute < 50))
  {
    SetMinute(minute-45);
    DrawWord("DREI");
    DrawWord("VIRTEL");
    DrawWord(NumToString(hour+1));
  }
  // Es ist 10 vor
  else if((minute >= 50) && (minute < 55))
  {
    SetMinute(minute-50);
    DrawWord("ZEHN");
    DrawWord("VOR");
    DrawWord(NumToString(hour+1));
  }
  // Es ist 5 vor
  else if((minute >= 55) && (minute < 60))
  {
    SetMinute(minute-55);
    DrawWord("FÜNF");
    DrawWord("VOR");
    DrawWord(NumToString(hour+1));
  }
  else
  {
    Serial.println("Something went wrong...");
  }

    DrawWord("UHR");
    led_handle.show();
}

/**
 * @brief 
 * 
 * @param s 
 */
void NeoMatrix::DrawWord(const String s)
{
  uint8_t listelement;
  uint8_t firstPixel;
  uint8_t lastPixel;

  Serial.print("SetWord; "); Serial.println(s);

  // Check that string is not empty
  if(s == "")
    return;

  // Find word in list
  for(listelement=0; listelement<ELEMENTS_IN_TEXT_TABLE; listelement++)
  {
    if(LED_Text[listelement].Text == s)
      break;
    
    if(listelement == ELEMENTS_IN_TEXT_TABLE)
      Serial.println("Word is not within list!!!");
  }

  firstPixel = LED_Text[listelement].FirstPixel;
  lastPixel = (LED_Text[listelement].FirstPixel + LED_Text[listelement].Text.length());

  Serial.print("listEl; "); Serial.println(listelement);
  Serial.print("firstPix; "); Serial.println(firstPixel);
  Serial.print("LastPix; "); Serial.println(lastPixel);

  // Set corresponding LED pixels
  for(uint8_t i=firstPixel; i<lastPixel; i++)
  {
    led_handle.setPixelColor(i, Color);
  }
}

/**
 * @brief 
 * 
 */
void NeoMatrix::SetMinute(uint8_t min)
{
  if(min < 5 && min != 0)
  {
    led_handle.setPixelColor((COLUMNS * ROWs) + min, Color);
  }
}

/**
 * @brief 
 * 
 * @param num 
 * @return String 
 */
String NeoMatrix::NumToString(const uint8_t num)
{
  String s = "";
  uint8_t temp = num;

  if(num>12)
    temp = 1;

  switch (temp)
  {
    case 1:  s = "EIN";    break;
    case 2:  s = "ZWEI";   break;
    case 3:  s = "DREI";   break;
    case 4:  s = "VIER";   break;
    case 5:  s = "FÜNF";   break;
    case 6:  s = "SECHS";  break;
    case 7:  s = "SIEBEN"; break;
    case 8:  s = "ACHT";   break;
    case 9:  s = "NEUN";   break;
    case 10: s = "ZEHN";   break;
    case 11: s = "ELF";    break;
    case 12: s = "ZWÖLF";  break;
    default: Serial.println("Unknown Number!");
  }

  return s;
}

/**
 * @brief Set the Color Brightness object
 * 
 * @param n 
 * @param r 
 * @param g 
 * @param b 
 * @param brightness 
 */
void NeoMatrix::SetColorBrighness( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) 
{
	led_handle.setPixelColor(n, (brightness*r/255) , (brightness*g/255), (brightness*b/255));
}