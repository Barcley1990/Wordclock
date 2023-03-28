#ifndef WORDCLOCK_H
#define WORDCLOCK_H
#include <Arduino.h>

//=================================================
#define DEBUG_MODE           true
#define EXEPTION while(1){;} 


//=================================================
extern void WordClock_Init();
extern void WordClock_Runnable_1s();
#ifdef DEBUG_MODE
extern uint8_t DEBUG_HOUR;
extern uint8_t DEBUG_MINUTE;
#endif

#endif