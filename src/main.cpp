
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

/***********************************************************************************************************************
 * Local function declarations
 ***********************************************************************************************************************/



/***********************************************************************************************************************
 * Function definitions
 ***********************************************************************************************************************/
/**
 * @brief Main init funciton. Called only once from within main
*/
void setup() 
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif
  WordClock_Init();
  Os_SysTimerCnt = 0u;
  
}

/**
 * @brief Main loop  function. Called from main()
 */
void loop() 
{
  uint32_t sys_time = millis();
  static uint32_t old_time = 0u;

if (sys_time - old_time >= OS_1000MS_EVENT) {    // check if 1000ms passed
  old_time = sys_time;   // save the last time you blinked the LED
  WordClock_Runnable_1s();
}

// Execute Server Handle
Wifi_ServerExec();


#ifdef DEBUG_MODE
  int temp_int1 =0;
  int temp_int2 =0;
  // if there's any serial available, read it:
  while (Serial.available() > 0) {
    temp_int1 = Serial.parseInt();
    temp_int2 = Serial.parseInt();
  }


  if(temp_int1 != 0 && temp_int2 != 0)
  {
    DEBUG_HOUR = temp_int1;
    DEBUG_MINUTE = temp_int2;
    temp_int1 = 0;
    temp_int2 = 0;
  }
  
#endif

}


