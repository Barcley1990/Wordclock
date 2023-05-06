/**
 * @file wordclock.h
 * @author Tobias Nuß
 * @brief 
 * @version 0.1
 * @date 2023-04-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef WORDCLOCK_H
#define WORDCLOCK_H
/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include <Arduino.h>

#define DEBUG_MODE          true

#define EXEPTION while(1){;} 
/***********************************************************************************************************************
 * Public variables
 ***********************************************************************************************************************/
#ifdef DEBUG_MODE
extern uint8_t DEBUG_HOUR;
extern uint8_t DEBUG_MINUTE;
#endif

/***********************************************************************************************************************
 * Global function declarations
 ***********************************************************************************************************************/
extern void WordClock_Init();
extern void WordClock_Runnable_1s();
void changeColor(uint8_t r, uint8_t g, uint8_t b);

#endif