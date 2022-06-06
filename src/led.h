#ifndef _LED_H_
#define _LED_H_


#include "driver/ledc.h"

#define LED_COUNT 8
#define LED_TIMER LEDC_TIMER_1
#define LED_SPEED_MODE LEDC_LOW_SPEED_MODE 
#define LED_MAX_DUTY_CYCLE 8191
#define LED_0 25
#define LED_CHANNEL_0 LEDC_CHANNEL_0
#define LED_1 32
#define LED_CHANNEL_1 LEDC_CHANNEL_1
#define LED_2 19 
#define LED_CHANNEL_2 LEDC_CHANNEL_2
#define LED_3 21
#define LED_CHANNEL_3 LEDC_CHANNEL_3
#define LED_4 23
#define LED_CHANNEL_4 LEDC_CHANNEL_4
#define LED_5 18
#define LED_CHANNEL_5 LEDC_CHANNEL_5
#define LED_6 16
#define LED_CHANNEL_6 LEDC_CHANNEL_6
#define LED_7 17
#define LED_CHANNEL_7 LEDC_CHANNEL_7

extern ledc_timer_config_t ledc_timer;
extern const ledc_channel_config_t ledc_channel[LED_COUNT];

void setupLedc();
void setLedBrightness(uint8_t ledIndex, uint8_t brightness);

#endif