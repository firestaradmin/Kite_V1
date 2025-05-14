
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include <inttypes.h>


#include "button.h"
#include "mlog.h"
/**
 * Brief:
 * This test code is about key interrupt.
 *
 * GPIO17:  input, pulled up, interrupt from rising edge.
 * 
 */


#define GPIO_KEY_L   17
#define GPIO_KEY_L_PIN_SEL  (1ULL << GPIO_KEY_L) 

#define ESP_INTR_FLAG_DEFAULT 0


// key_struct_t key_L ={
//     .gpio_num = GPIO_KEY_L,
//     .state = KEY_RELEASED,
//     .event = {{0}},
//     .lastPressedTime = 0
// };


static QueueHandle_t gpio_evt_queue = NULL;

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
            // MLOGI("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            int level = gpio_get_level(io_num);
            MLOGI("GPIO[%d] intr, val: %d\n", io_num, level);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void buttonInit(void)
{
    gpio_config_t io_conf;

    /* init key L */
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO17 here
    io_conf.pin_bit_mask = GPIO_KEY_L_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);


    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 9, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_KEY_L, key_isr_handler, (void*) GPIO_KEY_L);

}

