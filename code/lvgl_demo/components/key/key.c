
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "key.h"
/**
 * Brief:
 * This test code is about key interrupt.
 *
 * GPIO17:  input, pulled up, interrupt from rising edge.
 * 
 */


#define GPIO_KEY_L   17
#define GPIO_KEY_L_PIN_SEL  (1ULL << GPIO_KEY_L) 
#define GPIO_KEY_MID   18
#define GPIO_KEY_MID_PIN_SEL  (1ULL << GPIO_KEY_MID) 
#define GPIO_KEY_R   19
#define GPIO_KEY_R_PIN_SEL  (1ULL << GPIO_KEY_R)

key_struct_t key_L = {
    .gpio_num = GPIO_KEY_L,
    .state = KEY_RELEASED,
    .event = 0,
    .lastPressedTime = 0
};

key_struct_t key_R = {
    .gpio_num = GPIO_KEY_R,
    .state = KEY_RELEASED,
    .event = 0,
    .lastPressedTime = 0
};

key_struct_t key_Mid = {
    .gpio_num = GPIO_KEY_MID,
    .state = KEY_RELEASED,
    .event = 0,
    .lastPressedTime = 0
};

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR key_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void key_init(void)
{
    gpio_config_t io_conf;


    /* init key L */
    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO17 here
    io_conf.pin_bit_mask = GPIO_KEY_L_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    /* init key R */
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_KEY_R_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    /* init key Mid */
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_KEY_MID_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);




    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 9, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_KEY_L, key_isr_handler, (void*) GPIO_KEY_L);
    gpio_isr_handler_add(GPIO_KEY_R, key_isr_handler, (void*) GPIO_KEY_R);
    gpio_isr_handler_add(GPIO_KEY_MID, key_isr_handler, (void*) GPIO_KEY_MID);

}

