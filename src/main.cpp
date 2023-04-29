
/**********************************************************************************************************************
 * @file main.cpp
 * @author Tobias Nuß
 * @brief 
 * @version 0.1
 * @date 2023-04-06
 * 
 * @copyright Copyright (c) 2023
 * 
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Include Area
 ***********************************************************************************************************************/
#
#include <Arduino.h>
#include "wordclock.h"
#include "network.h"
#include "mcal.h"

#define OS_1MS_EVENT    1
#define OS_10MS_EVENT   10
#define OS_100MS_EVENT  100
#define OS_1000MS_EVENT 1000
#define SERIAL_BUFF_SIZE 100

/***********************************************************************************************************************
 * Private Variables
 ***********************************************************************************************************************/
#ifdef DEBUG_MODE
  uint8_t DEBUG_HOUR = 0;
  uint8_t DEBUG_MINUTE = 0;
#endif
static uint16_t Os_SysTimerCnt;
static uint16_t WiFiEnDebounceCounterHi;
static uint16_t WiFiEnDebounceCounterLo;
static String Serial_Buffer;

/***********************************************************************************************************************
 * Local function declarations
 ***********************************************************************************************************************/
bool gpio_debounce(uint16_t* debCnt, const uint16_t debTime);


/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
/**
 * @brief Main init funciton. Called only once from within main
*/
void setup() 
{
  Os_SysTimerCnt = 0u;

  // Initialize GPIOs
  pinMode(MCAL_WIFI_EN_PIN, INPUT);
  pinMode(MCAL_BOOT_PIN, INPUT);
  pinMode(MCAL_LED_EN_PIN, OUTPUT);
  digitalWrite(MCAL_LED_EN_PIN, LOW);

  // Check for debugging mode
#ifdef DEBUG_MODE
  Serial.begin(115200);
  delay(100);
  static String SerialBuffer;
#endif

  // Initialize wordclock
  WordClock_Init();
}

/**
 * @brief Main loop  function. Called from main()
 */
void loop() 
{
  uint32_t sys_time = millis();
  static uint32_t old_time_1000 = 0u;
  static uint32_t old_time_100 = 0u;
  static uint32_t old_time_10 = 0u;
  static uint16_t boot_pin_state_cnt = 0;
  static bool wifi_pin_state_flag_Hi = false;
  static bool wifi_pin_state_flag_Lo = false;

  // check if 1000ms passed
  if (sys_time - old_time_1000 >= OS_1000MS_EVENT) {    
    old_time_1000 = sys_time;
    WordClock_Runnable_1s();
  }

  // check if 100ms passed
  if (sys_time - old_time_100 >= OS_100MS_EVENT) {    
    old_time_100 = sys_time;
  }

  // check if 10ms passed
  if (sys_time - old_time_10 >= OS_10MS_EVENT) {   
    old_time_10 = sys_time;
    // poll Boot-Pin
    if(digitalRead(MCAL_BOOT_PIN) == false)
    {
      boot_pin_state_cnt++;
      // Reboot after 5s
      if(boot_pin_state_cnt >= 500u)
      {
        Serial.println("Rebooting...");
        digitalWrite(MCAL_LED_EN_PIN, LOW);
        delay(100);
        ESP.rebootIntoUartDownloadMode();
      }
    }
    // Reset counter to debounce
    else
    {
      boot_pin_state_cnt = 0u;
    }

    // poll WifiEn-Pin 
    // HIGH
    if(digitalRead(MCAL_WIFI_EN_PIN))
    {
      WiFiEnDebounceCounterLo = 0u;
      wifi_pin_state_flag_Lo = false;
      if(wifi_pin_state_flag_Hi == false && gpio_debounce(&WiFiEnDebounceCounterHi, 4u))
      {
        Serial.println("WiFi EN Pin is HIGH");
        wifi_pin_state_flag_Hi = true;
      }
    }
    else
    {
      WiFiEnDebounceCounterHi = 0u;
      wifi_pin_state_flag_Hi = false;
      if(wifi_pin_state_flag_Lo == false && gpio_debounce(&WiFiEnDebounceCounterLo, 4u))
      {
        Serial.println("WiFi EN Pin is LOW");
        wifi_pin_state_flag_Lo = true;
      }
    }
  }

  // Execute Server Handle
  Wifi_ServerExec();


#ifdef DEBUG_MODE
  static int temp_int1 = 0u;
  static int temp_int2 = 0u;
  // if there's any serial available, read it (max. 2s):
  if(Serial.available() && temp_int1 == 0){
      String SerialData = Serial.readString();
      temp_int1 = atol(SerialData.c_str());
      Serial.println(temp_int1);
  }
  if(Serial.available() && temp_int1 != 0){
      String SerialData2 = Serial.readString();
      temp_int2 = atol(SerialData2.c_str());
      Serial.println(temp_int2);
  }
  
  if(temp_int1 != 0 && temp_int2 != 0)
  {
    DEBUG_HOUR = temp_int1;
    DEBUG_MINUTE = temp_int2;
    temp_int1 = 0;
    temp_int2 = 0;
    Serial_Buffer.clear();
  }
  

  
#endif

}

/**
 * @brief 
 * 
 * @param debCnt 
 * @param debTime 
 * @return true 
 * @return false 
 */
bool gpio_debounce(uint16_t* debCnt, const uint16_t debTime)
{
  bool retVal = false;

  (*debCnt)++;
  if(*debCnt >= debTime)
  {
    retVal = true;
    *debCnt = 0u;
  }

  return retVal;
}
