
//=================================================
#include <Arduino.h>
#include "wordclock.h"

//=================================================
#define OS_1MS_EVENT    1
#define OS_10MS_EVENT   10
#define OS_100MS_EVENT  100
#define OS_1000MS_EVENT 1000

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

if (sys_time - old_time >= OS_1000MS_EVENT) {    // check if 1000ms passed
  old_time = sys_time;   // save the last time you blinked the LED
  WordClock_Runnable_1s();
}


#ifdef DEBUG_MODE
  // if there's any serial available, read it:
  while (Serial.available() > 0) {
    DEBUG_HOUR = Serial.parseInt();
    DEBUG_MINUTE = Serial.parseInt();

    // look for the newline. That's the end of your sentence:
    if (Serial.read() == '\n') {
      Serial.print("--DEBUG MODE-- Time set to: ");
      Serial.print(DEBUG_HOUR);
      Serial.print(":");
      Serial.println(DEBUG_MINUTE);
    }
  }
#endif

}


