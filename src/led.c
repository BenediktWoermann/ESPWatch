//_____________________ INCLUDES _____________________//

#include "led.h"


//_____________________ GLOBAL VARIABLES _____________________//

ledc_timer_config_t ledc_timer;

const ledc_channel_config_t ledc_channel[LED_COUNT] = {
        {
            .channel    = LED_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = LED_0,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = LED_1,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = LED_2,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_3,
            .duty       = 0,
            .gpio_num   = LED_3,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_4,
            .duty       = 0,
            .gpio_num   = LED_4,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_5,
            .duty       = 0,
            .gpio_num   = LED_5,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_6,
            .duty       = 0,
            .gpio_num   = LED_6,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
        {
            .channel    = LED_CHANNEL_7,
            .duty       = 0,
            .gpio_num   = LED_7,
            .speed_mode = LED_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LED_TIMER
        },
    };


//_____________________ SETUP FUNCTIONS _____________________//

static void setupLedcTimer(){

    // build configureation of ledc driver
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
    ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
    ledc_timer.speed_mode = LED_SPEED_MODE;           // timer mode
    ledc_timer.timer_num = LED_TIMER;            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;              // Auto select the source clock

    // activate configuration
    ledc_timer_config(&ledc_timer);
}

static void setupLedcChannel(){
   
    for(int channel = 0; channel < LED_COUNT; channel++){
        // set LED channel with previously prepared configuration
        ledc_channel_config(&ledc_channel[channel]);
    }

}

void setupLedc(){
    setupLedcTimer();
    setupLedcChannel();
}


//_____________________ UTILITIES AND OTHER FUNCTIONS _____________________//

void setLedBrightness(uint8_t ledIndex, uint8_t brightness){
    ledc_set_duty(ledc_channel[ledIndex].speed_mode, ledc_channel[ledIndex].channel, ((float)brightness/100.0f) * LED_MAX_DUTY_CYCLE);
    ledc_update_duty(ledc_channel[ledIndex].speed_mode, ledc_channel[ledIndex].channel);
}