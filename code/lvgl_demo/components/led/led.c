
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "led.h"



static void led_task(void* arg)
{
    uint32_t cnt = 0;
    while(1){
        cnt ++;
		gpio_set_level(GPIO_LED0, cnt % 2);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}



void led_init(void)
{
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //create led task for blink
    xTaskCreate(led_task, "led_task", 1024, NULL, 1, NULL);
}

