/* 
 * Kite_V1 core board demo
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/semphr.h"


#include "key.h"
#include "led.h"
#include "lcd_st7735s.h"
#include "ug_log.h"
/* LVGL */
#include "lvgl.h"
#include "lv_port_disp.h"

void vOtherFunction( void );
void create_ui();

SemaphoreHandle_t ugLog_mutex;

void ugLogLock()
{
    xSemaphoreTake(ugLog_mutex, portMAX_DELAY );
    
}
void ugLogUnlock()
{
    xSemaphoreGive(ugLog_mutex);
}

void ug_log_mutex_init( )
{
    ugLog_mutex = xSemaphoreCreateMutex();
    if( ugLog_mutex == NULL )
    {
        printf("ug log mutex err!\r\n");
    }
}
void app_main(void)
{
	printf("Kite_V1 core board demo!\n");

    ug_log_mutex_init();
    ug_log_init(ug_log_mutexWay_externel, printf, ugLogLock, ugLogUnlock);
    led_init();
	key_init();
    lv_init();
    lv_port_disp_init();

    create_ui();

    vOtherFunction();
    UGINFO("every thing willl be ok!\r\n");
    while(1)
	{
        lv_timer_handler();
    }

}






// Function that creates a task.
void vOtherFunction( void )
{

    TaskHandle_t xHandle = NULL;

	// xTaskCreate( vTask_ug_tick, "vTask_ug_tick", 1024, NULL, 9, &xHandle );
	// configASSERT( xHandle );

	// Use the handle to delete the task.
	// if( xHandle != NULL )
	// {
	// 	vTaskDelete( xHandle );
	// }
}



// Task to be created.
void vTask_ug_tick( void * pvParameters )
{
	for( ;; )
	{
		// Task code goes here.
        lv_tick_inc(10);
        // vTaskDelay(10 / portTICK_PERIOD_MS);
		vTaskDelay(pdMS_TO_TICKS(10));

    }
}


void btn_event_cb(lv_event_t * e)
{

}


void create_ui()
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());                   /*Add a button to the current screen*/
    lv_obj_set_pos(btn, 10, 10);                                    /*Set its position*/
    lv_obj_set_size(btn, 100, 50);                                  /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);                        /*Add a label to the button*/
    lv_label_set_text(label, "Button");                             /*Set the labels text*/
    lv_obj_center(label); 
}