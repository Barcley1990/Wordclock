
//=================================================
#include <Arduino.h>
#include "wordclock.h"

//=================================================
#ifdef DEBUG_MODE
  uint8_t DEBUG_HOUR = 0;
  uint8_t DEBUG_MINUTE = 0;
#endif
static uint32_t WordClock_SysTimerCnt;
bool WordClock_1000ms_Ev = false;

//=================================================


//=================================================
void setup() {
  WordClock_SysTimerCnt = 0;
}

//=================================================
void loop() {
  // Every second
  if(millis() > WordClock_SysTimerCnt)
  {
    WordClock_Runnable_1s();
    WordClock_SysTimerCnt += 1000;
  }
}


//=================================================
