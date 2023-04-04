
//=================================================
#include <Arduino.h>
#include "wordclock.h"

//=================================================
#define OS_1MS_EVENT    1
#define OS_10MS_EVENT   10
#define OS_100MS_EVENT  100
#define OS_1000MS_EVENT 1000
#define SERIAL_BUFF_SIZE 100

//=================================================
#ifdef DEBUG_MODE
  uint8_t DEBUG_HOUR = 0;
  uint8_t DEBUG_MINUTE = 0;
#endif
static uint16_t Os_SysTimerCnt;

//=================================================


//=================================================
void setup() 
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif
  WordClock_Init();
  Os_SysTimerCnt = 0u;
  
}

//=================================================
void loop() 
{
  uint32_t sys_time = millis();
  static uint32_t old_time = 0u;
  int temp_int1 =0;
  int temp_int2 =0;

if (sys_time - old_time >= OS_1000MS_EVENT) {    // check if 1000ms passed
  old_time = sys_time;   // save the last time you blinked the LED
  WordClock_Runnable_1s();
}


#ifdef DEBUG_MODE
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


