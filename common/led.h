/*
 * led.c - H303 Nucleo LED setup
 */

#ifndef __led__
#define __led__

#include "stm32f3xx_hal.h"

void LEDInit(void);
void LEDOn(void);
void LEDOff(void);
void LEDToggle(void);

#endif
