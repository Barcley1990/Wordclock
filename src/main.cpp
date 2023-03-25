
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
  Os_SysTimerCnt = 0;
}

//=================================================
void loop() 
{
  uint32_t sys_time = millis();
  static uint32_t old_time = 0u;

  if((sys_time - old_time) >= OS_10MS_EVENT)
  {
    Os_SysTimerCnt++;
    old_time = sys_time;
  }

  if((Os_SysTimerCnt % OS_100MS_EVENT) >= OS_100MS_EVENT)
  {
    // Exec

  }

  if((Os_SysTimerCnt % OS_1000MS_EVENT) >= OS_1000MS_EVENT)
  {
    // Exec
    WordClock_Runnable_1s();
  }

  // Reset counter
  if(Os_SysTimerCnt >= 1000u)
    Os_SysTimerCnt = 0u;
}


//=================================================
