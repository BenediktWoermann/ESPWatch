//_____________________ INCLUDES _____________________//

//library includes
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
//header file
#include "main.h"
//includes for freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
//espidf includes
#include "driver/gpio.h"
#include "esp_system.h"
#include "sdkconfig.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_sntp.h"
#include "esp_event.h"
#include "esp_attr.h"
//local includes
#include "led.h"
#include "button.h"


//_____________________ DEFINES _____________________//

#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1

#define VDD_BAT 33
#define CHARG 26

#define SSID "BKA Drogendezanat Mobil #37"
#define PASSPHARSE "86916382869679997015"


//_____________________ FUNCTION PROTOTYPES _____________________//

static void initialize_sntp(void);
static void obtain_time(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);
static void initialise_wifi(void);
void wifi_connect();
void time_sync_notification_cb(struct timeval *tv);
void DispOff();
void DispOn();


//_____________________ TASKS _____________________//

//naming scheme: taskname = "task" + taskNr + function;
TaskHandle_t task1Hndlr = NULL;
static void task1DispOut(void * pvParameters);

TaskHandle_t task2Hndlr = NULL;
static void task2ButtonPoll(void* pvParameters);

TaskHandle_t task3Hndlr = NULL;
static void task3TimeUpdate(void* pvParameters);


//_____________________ GLOBAL VARIABLES _____________________//

static const char* TAG = "Main";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
time_t now;
struct tm timeinfo;

//_____________________ SETUP FUNCTIONS _____________________//

void setupWifi(){
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    initialise_wifi();
}

void setupSntp(){
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        initialize_sntp();
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
    tzset();
}

void setup(void) {
    ESP_LOGI(TAG, "Setup started!");

    setupLedc();
    setupButton();
    setupWifi();
    setupSntp();

    ESP_LOGI(TAG, "Setup finished!");
}

//_____________________ TASK- AND MAINFUNCTIONS _____________________//

void task1DispOut(void * pvParameters){
    TickType_t xLastWakeTime;
    TickType_t xFrequency = 10;
    xLastWakeTime = xTaskGetTickCount();
    uint8_t brightness = 100;

    for(;;){
        uint8_t hour = timeinfo.tm_hour;
        hour = hour > 12 ? hour - 12 : hour;
        setLedBrightness(0, brightness * ((hour >> 3) & 1));
        setLedBrightness(1, brightness * ((hour >> 2) & 1));
        setLedBrightness(2, brightness * ((hour >> 1) & 1));
        setLedBrightness(3, brightness * ((hour >> 0) & 1));

        uint8_t minute = timeinfo.tm_min;
        uint16_t second = timeinfo.tm_sec + 60 * minute;
        setLedBrightness(4, brightness * (second >= 1800));
        setLedBrightness(5, brightness * ((second%1800) >= 900));
        setLedBrightness(6, brightness * ((second%900) >= 450));
        setLedBrightness(7, brightness * ((second%450) >= 225));

        ESP_LOGI(TAG, "hour: %d, minute: %d, leds: %d %d %d %d %d %d %d %d", hour, minute, 
        brightness * ((hour >> 3) & 1), brightness * ((hour >> 2) & 1), brightness * ((hour >> 1) & 1),
        brightness * ((hour >> 0) & 1), brightness * (second >= 1800), brightness * ((second%1800) >= 900), 
        brightness * ((second%900) >= 450), brightness * ((second%450) >= 225));

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void task2ButtonPoll(void* pvParameters){
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 5;
    // duration, in which press of two buttons are considered as one input
    const TickType_t multiPressDuration = MULTI_PRESS_DURATION;
    // factor to determine, how many extra task cycles (polling cycles) shall be waited, before button gets activated again after successfull click (debouncing)
    const int debounceFactor = 4;
    int debounceCounter = 0;
    uint8_t debounceFlagReg = 0U;
    uint8_t swStatusReg = 0U;
    uint8_t swStatusRegPrev = 0U;

    xLastWakeTime = xTaskGetTickCount();
    for(;;){

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        if(debounceFlagReg){
            if(debounceCounter >= debounceFactor){
                // debouncing has finished
                // reset all variables
                debounceFlagReg = 0U;
                debounceCounter = 0;
                swStatusReg = 0U;
                swStatusRegPrev = 0U;
                portENTER_CRITICAL(&swMux);
                swPressReg = 0U;
                portEXIT_CRITICAL(&swMux);
            }else{
                // debouncing is ongoing
                debounceCounter++;
                continue;
            }
        }else{
            //activate debouncing, if swStatusReg != 0
            if(swStatusReg){
                debounceFlagReg = swStatusReg;
                continue;
            }
        }
        
        
pollButtonStatus:
        // check if any event gets triggered by a button
        portENTER_CRITICAL(&swMux);
            swStatusReg = swPressReg; // copy data to local variable
        portEXIT_CRITICAL(&swMux);
        

        if(swStatusReg > swStatusRegPrev){
            swStatusRegPrev = swStatusReg;
            vTaskDelayUntil(&xLastWakeTime, multiPressDuration);
            goto pollButtonStatus;
        }

        // work throuh all the triggered events and execute their code
        if((swStatusReg & (1U<<2)) && (swStatusReg & (1U<<3))){
            // code to run, whenn buttons 1 & 4 get pressed simultaneously
            setLedBrightness(0, 100);
            continue;
        }

        if(swStatusReg & (1U<<1)){
            // code to run, when button 1 got clicked
            setLedBrightness(1, 100);

        }
        if(swStatusReg & (1U<<2)){
            // code to run, when button 2 got clicked
            DispOff();
        }
        if(swStatusReg & (1U<<3)){
            // code to run, when button 3 got clicked
            DispOn();
        }
        if(swStatusReg & (1U<<4)){
            // code to run, when button 4 got clicked
            setLedBrightness(2, 100);
        }

        
    }
}

void task3TimeUpdate(void* pvParameters){
    TickType_t xLastWakeTime;
    TickType_t xFrequency = 5000;
    xLastWakeTime = xTaskGetTickCount();

    for(;;){

        char strftime_buf[64];
        time(&now);
        localtime_r(&now, &timeinfo);
        
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Germany is: %s", strftime_buf);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


void app_main(void)
{   
    setup();
    xTaskCreatePinnedToCore(task1DispOut, "DisplayOutput", 4096, NULL, 2, &task1Hndlr, 0);
    xTaskCreatePinnedToCore(task2ButtonPoll, "ButtonPoll", 1024, NULL, 3, &task2Hndlr, 0);
    xTaskCreatePinnedToCore(task3TimeUpdate, "TimeUpdate", 4096, NULL, 1, &task3Hndlr, 0);
}





static void obtain_time(void)
{

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,"icircuit");
    if(ret != ESP_OK ){
      ESP_LOGE(TAG,"failed to set hostname:%d",ret);  
    }
}

void wifi_connect(){
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHARSE,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void DispOff(){
    vTaskSuspend(task1Hndlr);
    for(uint8_t i = 0; i<8; i++){
        setLedBrightness(i, 0);
    }
}

void DispOn(){
    vTaskResume(task1Hndlr);
}