#ifndef WORDCLOCK_H
#define WORDCLOCK_H
#include <Arduino.h>

//=================================================
#define DEBUG_MODE          true
#define SOFTWARE_VERSION_MAYOR 0
#define SOFTWARE_VERSION_MINOR 1
#define SOFTWARE_VERSION_PATCH 0
#define EXEPTION while(1){;} 


//=================================================
extern void WordClock_Init();
extern void WordClock_Runnable_1s();
#ifdef DEBUG_MODE
extern uint8_t DEBUG_HOUR;
extern uint8_t DEBUG_MINUTE;
#endif

#endif