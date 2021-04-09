//_____________________ INCLUDES _____________________//

//library includes
#include <stdio.h>
//header file
#include "main.h"
//includes for freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//espidf includes
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "sdkconfig.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
//local includes


//_____________________ DEFINES _____________________//

#define BLINK_GPIO 23
#define SW_1 24
#define SW_2 4
#define SW_3 34
#define SW_4 35

#define LED_COUNT 8
#define LED_TIMER LEDC_TIMER_0
#define LED_SPEED_MODE LEDC_LOW_SPEED_MODE 
#define LED_MAX_DUTY_CYCLE 8192
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

#define VDD_BAT 33
#define CHARG 26


//_____________________ FUNCTION DECLARATION _____________________//


void setupLedcTimer(void);
void setupLedcChannel(void);

//_____________________ TASKS _____________________//

//naming scheme: taskname = "task" + taskNr + function;
TaskHandle_t task999Hndlr = NULL;
static void task999BlinkLed(void * pvParameters);

TaskHandle_t task1Hndlr = NULL;
static void task1DispOut(void * pvParameters);


//_____________________ GLOBAL VARIABLES _____________________//

static const char* TAG = "Main";

ledc_timer_config_t ledc_timer;
ledc_channel_config_t* ledc_channel;


//_____________________ SETUP FUNCTIONS _____________________//

void setup(void) {
    ESP_LOGI(TAG, "Setup started!");

    setupLedcTimer();
    setupLedcChannel();

    ESP_LOGI(TAG, "Setup finished!");
}

void setupLedcTimer(){

    // build configureation of ledc driver
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
    ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
    ledc_timer.speed_mode = LED_SPEED_MODE;           // timer mode
    ledc_timer.timer_num = LED_TIMER;            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;              // Auto select the source clock

    // activate configuration
    ledc_timer_config(&ledc_timer);
}

void setupLedcChannel(){

    uint8_t gpioPins [] = {LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7};
        
    for(int channel = 0; channel < LED_COUNT; channel++){
        // setup the channel
        ledc_channel[channel].channel = channel;
        ledc_channel[channel].duty = 0;
        ledc_channel[channel].gpio_num = gpioPins[channel];
        ledc_channel[channel].speed_mode = LED_SPEED_MODE;
        ledc_channel[channel].hpoint = 0;
        ledc_channel[channel].timer_sel = LED_TIMER;
        
        // set LED channel with previously prepared configuration
        ledc_channel_config(&ledc_channel[channel]);
    }

}


//_____________________ TASK- AND MAINFUNCTIONS _____________________//

void task999BlinkLed(void * pvParameters){
    gpio_pad_select_gpio(LED_0);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t) LED_0, GPIO_MODE_OUTPUT);
    for(;;){
        /* Blink off (output low) */
        ESP_LOGI(TAG, "Turn LED off");
        gpio_set_level((gpio_num_t) LED_0, 0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        /* Blink on (output high) */
        ESP_LOGI(TAG, "Turn LED on");
        gpio_set_level((gpio_num_t) LED_0, 1);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

void task1DispOut(void * pvParameters){
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 10;
  xLastWakeTime = xTaskGetTickCount();

  for(;;){
    
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void app_main(void)
{   
  setup();
  xTaskCreatePinnedToCore(task999BlinkLed, "BlinkLed", 4096, NULL, 2, &task999Hndlr, 1);
  xTaskCreatePinnedToCore(task1DispOut, "DisplayOutput", 1024, NULL, 2, &task1Hndlr, 0);

}


//_____________________ UTILITIES AND OTHER FUNCTIONS _____________________//

void setLedBrightness(uint8_t ledIndex, uint8_t brightness){
    ledc_set_duty(ledc_channel[ledIndex].speed_mode, ledc_channel[ledIndex].channel, ((float)brightness/100.0f) * LED_MAX_DUTY_CYCLE);
    ledc_update_duty(ledc_channel[ledIndex].speed_mode, ledc_channel[ledIndex].channel);
}