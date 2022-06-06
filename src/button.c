//_____________________ INCLUDES _____________________//

#include "button.h"

#define GPIO_INPUT_PIN_SEL  ((1ULL<<SW_1) | (1ULL<<SW_2) | (1ULL<<SW_3) | (1ULL<<SW_4))
#define ESP_INTR_FLAG_DEFAULT 0


//_____________________ FUNCTION PROTOTYPES _____________________//

static void IRAM_ATTR sw1IsrHandler(void* arg);
static void IRAM_ATTR sw2IsrHandler(void* arg);
static void IRAM_ATTR sw3IsrHandler(void* arg);
static void IRAM_ATTR sw4IsrHandler(void* arg);


//_____________________ GLOBAL VARIABLES _____________________//

portMUX_TYPE swMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t swPressReg = 0U;


//_____________________ SETUP FUNCTIONS _____________________//

void setupIoConfig(){
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void setupIsr(){
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pins
    gpio_isr_handler_add(SW_1, sw1IsrHandler, (void*) SW_1);
    gpio_isr_handler_add(SW_2, sw2IsrHandler, (void*) SW_2);
    gpio_isr_handler_add(SW_3, sw3IsrHandler, (void*) SW_3);
    gpio_isr_handler_add(SW_4, sw4IsrHandler, (void*) SW_4);
}

void setupButton(void){
    setupIoConfig();
    setupIsr();
}


//_____________________ ISR Handler _____________________//

static void IRAM_ATTR sw1IsrHandler(void* arg){
    portENTER_CRITICAL_ISR(&swMux);
    swPressReg |= (1U << 1); 
    portEXIT_CRITICAL_ISR(&swMux);
}

static void IRAM_ATTR sw2IsrHandler(void* arg){
    portENTER_CRITICAL_ISR(&swMux);
    swPressReg |= (1U << 2); 
    portEXIT_CRITICAL_ISR(&swMux);
}

static void IRAM_ATTR sw3IsrHandler(void* arg){
    portENTER_CRITICAL_ISR(&swMux);
    swPressReg |= (1U << 3); 
    portEXIT_CRITICAL_ISR(&swMux);
}

static void IRAM_ATTR sw4IsrHandler(void* arg){
    portENTER_CRITICAL_ISR(&swMux);
    swPressReg |= (1U << 4); 
    portEXIT_CRITICAL_ISR(&swMux);
}