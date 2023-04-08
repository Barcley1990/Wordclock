/**
 * @file led.cpp
 * @author Tobias Nuß
 * @brief 
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include "led.h"
#include "mcal.h"
#include <Adafruit_NeoPixel.h>

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

#define LED_WS2812_ENABLE_PIN       MCAL_LED_EN_PIN
#define LED_WS2812_DATA_PIN         MCAL_LED_DIN_PIN
#define LED_ROWs                    (10)
#define LED_COLUMNS                 (11)
#define LED_MINUTE_LEDS             (4)
#define LED_AMBILIGHT_LEDS          (0)
#define LED_WS2812_MAX_LEDS         ((LED_COLUMNS * LED_ROWs) + LED_MINUTE_LEDS + LED_AMBILIGHT_LEDS)
#define LED_NUM_OF_WORDS            (21)
#define LED_PIXEL_S                 (56)

#define LED_ELEMENTS_IN_TEXT_TABLE sizeof(LED_Text) / sizeof(LED_Text[0])

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
typedef struct
{
  String Text;
  uint8_t FirstPixel;
  uint8_t NumPixels;
  uint32_t Color;
} LED_Text_Type;

const LED_Text_Type LED_Text[LED_NUM_OF_WORDS] = 
{ 
  {"ES",      0,  0, 0}, //-->
  {"IST",     3,  0, 0}, //-->
  {"UHR",     99, 0, 0}, //<--
  {"VIRTEL",  26, 0, 0}, //-->
  {"HALB",    41, 0, 0}, //-->
  {"NACH",    38, 0, 0}, //-->
  {"VOR",     45, 0, 0}, //<--
  {"EIN",     61, 0, 0}, //<--
  {"EINS",    60, 0, 0}, //<--
  {"ZWEI",    62, 0, 0}, //<--
  {"DREI",    67, 0, 0}, //-->
  {"VIER",    77, 0, 0}, //<--
  {"FÜNF",    73, 0, 0}, //-->
  {"SECHS",   104,0, 0}, //<--
  {"SIEBEN",  55, 0, 0}, //<--
  {"ACHT",    89, 0, 0}, //-->
  {"NEUN",    81, 0, 0}, //<--
  {"ZEHN",    94, 0, 0}, //-->
  {"ELF",     85, 0, 0}, //<--
  {"ZWÖLF",   49, 0, 0}, //-->
  {"ZWANZIG", 11, 0, 0}, //<--
};

Adafruit_NeoPixel LED_Pixels = Adafruit_NeoPixel(LED_WS2812_MAX_LEDS, LED_WS2812_DATA_PIN, NEO_GRB + NEO_KHZ800);
static uint32_t LED_Color;
static bool LED_IsInitialised = false;

/***********************************************************************************************************************
 * Local function declarations
 ***********************************************************************************************************************/
void LED_SetWord(const String s);
void LED_SetMinute(uint8_t min);
String LED_NumToString(const uint8_t num);
void LED_SetColor(uint32_t c);
void LED_SetColorBrighness( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) ;

/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/

/**
 * @brief LED_Init
 * 
 * @param c Color to initialize
 * @param brightness Brightness to initialized in percentage
 */
void LED_Init(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
  pinMode(MCAL_LED_EN_PIN, OUTPUT);
  LED_Pixels.begin();
  LED_Pixels.clear();
  LED_Pixels.show();
  LED_Pixels.setBrightness(map(brightness, 0, 255, 0, 100));
  LED_Color = LED_Pixels.Color(r, g, b);
  LED_IsInitialised = true;
  LED_TurnPwrOn();
}

/**
 * @brief LED_ShowTime
 * 
 * @param hour 
 * @param minute 
 * @param second 
 */
void LED_ShowTime(uint8_t hour, uint8_t minute, uint8_t second)
{
  bool rand = random(0,1);

  LED_Pixels.clear();
  LED_SetWord("ES");
  LED_SetWord("IST");

  // Es ist 
  if((minute >= 0u) && (minute < 5u))
  {
    LED_SetMinute(minute);
    LED_SetWord(LED_NumToString(hour));
  }
  //Es ist 5 nach
  else if((minute >= 5u) && (minute < 10u))
  {
    LED_SetMinute(minute-5);
    LED_SetWord("FÜNF");
    LED_SetWord("NACH");
    LED_SetWord(LED_NumToString(hour));
  }
  // Es ist 10 nach
  else if((minute >= 10u) && (minute < 15u))
  {
    LED_SetMinute(minute-10);
    LED_SetWord("ZEHN");
    LED_SetWord("NACH");
    LED_SetWord(LED_NumToString(hour));
  }
  //Es ist virtel nach
  else if((minute >= 15u) && (minute < 20u))
  {
    LED_SetMinute(minute-15u);
    LED_SetWord("VIRTEL");
    LED_SetWord("NACH");
    LED_SetWord(LED_NumToString(hour));
  }
  // Es ist 10 vor halb / Es ist zwanzig nach
  else if((minute >= 20) && (minute < 25))
  {
    LED_SetMinute(minute-20);
    if(rand){
      LED_SetWord("ZWANZIG");
      LED_SetWord("NACH");
      LED_SetWord(LED_NumToString(hour));
    }
    else {
      LED_SetWord("ZEHN");
      LED_SetWord("VOR");
      LED_SetWord("HALB");
      LED_SetWord(LED_NumToString(hour+1));
    }
  }
  // Es ist 5 vor halb
  else if((minute >= 25) && (minute < 30))
  {
    LED_SetMinute(minute-25);
    LED_SetWord("FÜNF");
    LED_SetWord("VOR");
    LED_SetWord("HALB");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist halb
  else if((minute >= 30) && (minute < 35))
  {
    LED_SetMinute(minute-25);
    LED_SetWord("HALB");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist 5 nach halb
  else if((minute >= 35) && (minute < 40))
  {
    LED_SetMinute(minute-35);
    LED_SetWord("FÜNF");
    LED_SetWord("NACH");
    LED_SetWord("HALB");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist 10 nach halb
  else if((minute >= 40) && (minute < 45))
  {
    LED_SetMinute(minute-40);
    LED_SetWord("ZEHN");
    LED_SetWord("NACH");
    LED_SetWord("HALB");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist dreivirtel
  else if((minute >= 45) && (minute < 50))
  {
    LED_SetMinute(minute-45);
    LED_SetWord("DREI");
    LED_SetWord("VIRTEL");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist 10 vor
  else if((minute >= 50) && (minute < 55))
  {
    LED_SetMinute(minute-50);
    LED_SetWord("ZEHN");
    LED_SetWord("VOR");
    LED_SetWord(LED_NumToString(hour+1));
  }
  // Es ist 5 vor
  else if((minute >= 55) && (minute < 60))
  {
    LED_SetMinute(minute-55);
    LED_SetWord("FÜNF");
    LED_SetWord("VOR");
    LED_SetWord(LED_NumToString(hour+1));
  }
  else
  {
    Serial.println("Something went wrong...");
  }

  LED_SetWord("UHR");

  if(LED_IsInitialised == true)
    LED_Pixels.show();
  else
    Serial.println("Error: Neopixels not yet initialized!");
}

/**
 * @brief LED_SetWord
 * 
 * @param s 
 */
void LED_SetWord(const String s)
{
  uint8_t listelement;
  uint8_t firstPixel;
  uint8_t lastPixel;

  Serial.print("SetWord; "); Serial.println(s);

  // Check that string is not empty
  if(s == "")
    return;

  // Find word in list
  for(listelement=0; listelement<LED_ELEMENTS_IN_TEXT_TABLE; listelement++)
  {
    if(LED_Text[listelement].Text == s)
      break;
    
    if(listelement == LED_ELEMENTS_IN_TEXT_TABLE)
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
    LED_Pixels.setPixelColor(i, LED_Color);
  }
}

/**
 * @brief LED_SetMinute
 * 
 * @param min 
 */
void LED_SetMinute(uint8_t min)
{
  if(min < 5 && min != 0)
  {
    LED_Pixels.setPixelColor((LED_COLUMNS * LED_ROWs) + min, LED_Color);
  }
}

/**
 * @brief LED_NumToString
 * 
 * @param num 
 * @return String 
 */
String LED_NumToString(const uint8_t num)
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
 * @brief LED_SetColor
 * 
 * @param c 
 */
void LED_SetColor(uint32_t c)
{
  LED_Color = c;
}

/**
 * @brief Turn LED power off to safe energy
 * 
 */
void LED_TurnPwrOff()
{
  digitalWrite(MCAL_LED_EN_PIN, LOW);
  LED_Pixels.clear();
}

/**
 * @brief Turn LED power on
 * 
 */
void LED_TurnPwrOn()
{
  LED_Pixels.clear();
  digitalWrite(MCAL_LED_EN_PIN, HIGH);
  LED_Pixels.show();
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
void LED_SetColorBrighness( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) 
{
	LED_Pixels.setPixelColor(n, (brightness*r/255) , (brightness*g/255), (brightness*b/255));
}














