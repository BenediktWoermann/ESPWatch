#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define SW_1 22
#define SW_2 4
#define SW_3 34
#define SW_4 35
#define MULTI_PRESS_DURATION 10
#define DEBOUNCE_TIME 10

extern portMUX_TYPE swMux;
extern uint8_t swPressReg;

void setupButton(void);

#endif