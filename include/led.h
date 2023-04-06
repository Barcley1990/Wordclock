/**
 * @file led.h
 * @author Tobias Nuß
 * @brief 
 * @version 0.1
 * @date 2023-04-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef LED_H
#define LED_H

/***********************************************************************************************************************
 * Include area
 ***********************************************************************************************************************/
#include <Arduino.h>

/***********************************************************************************************************************
 * Public variables
 ***********************************************************************************************************************/



/***********************************************************************************************************************
 * Global function declarations
 ***********************************************************************************************************************/
void LED_Init();
void LED_ShowTime(uint8_t hour, uint8_t minute, uint8_t second);


#endif