/* perfmon (performance monitor) example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "led.h"
#include "mlog.h"


extern void bleServerInit(void);

// void app_main(void)
// {
//     MLOGI("Start\n");
//     ledInit();
//     buttonInit();
//     bleServerInit();
//     MLOGI("init ok!\n");

//     MLOGI("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());
        
//     while (1)
//     {
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         // MLOGI("LOOP!\n");
//     }
// }



void app_main(void)
{
    MLOGI("Start\n");
    bleServerInit();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        MLOGI("LOOP!\n");
    }
}










