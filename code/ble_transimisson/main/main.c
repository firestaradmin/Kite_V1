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

// #include "uv_log.h"
// user
// #include "gui.h"
#include "mylcd.h"

#include "mybeep.h"
// #include "my_rgbled.h"
// #include "my_ble.h"
#include "mlog.h"

static const char* TAG = "Main";

// void app_main(void)
// {
//     uv_log(UVL_INFO, "Start\n");
//     ble_init();
//     uv_log(UVL_INFO, "init ok!\n");

//     while (1)
//     {
//         vTaskDelay(10 / portTICK_PERIOD_MS);
//     }
// }

static DRAM_ATTR uint16_t lcd_096_vram[80 * 160] = {0};

LcdDevice lcd_096;

void screenInit()
{
    lcd_color16_t forecolor = UG_COLOR_ORANGE;
    lcd_color16_t backcolor = UG_COLOR_DEEP_GRAY;

    // uv_log(UVL_INFO, "forecolor: 0x%04X\n", forecolor.full);
    // uv_log(UVL_INFO, "backcolor: 0x%04X\n", backcolor.full);

    LcdConfigure lcd_config_096 = LCD_DEFAULT_CONFIG_ST7735S_096;
    int ret = lcd_init(k_lcd_type_st7735s_096, &lcd_config_096, &lcd_096, &lcd_096_vram);

    lcd_draw_fillRectangle(&lcd_096, 0, 0, 79, 160, backcolor);

    lcd_draw_fillRectangle(&lcd_096, 10, 100, 70, 130, forecolor);

    uint16_t w = 60;
    uint16_t h = 30;
    uint16_t x = (lcd_096.res_w - w) / 2 - 1;
    uint16_t y = (lcd_096.res_h - h) / 2 - 1;
    lcd_draw_rectangle(&lcd_096, x, y, w, h, UG_COLOR_DEEP_ORANGE);

    ug_draw_str(&lcd_096, 10, 10, &ug_font_consolas_16px_15, UG_COLOR_DEEP_PURPLE, backcolor,
                "Hello");
    lcd_xfer_ram_swap_16bit(&lcd_096);

}

extern void bleServerInit(void);

void app_main(void)
{
    MLOGI("Start\n");
    screenInit();
    bleServerInit();
    lcd_draw_fillRectangle(&lcd_096, 0, 0, 80, 160, UG_COLOR_GREEN);
    ug_draw_str(&lcd_096, 10, 10, &ug_font_consolas_16px_15, UG_COLOR_DEEP_PURPLE, UG_COLOR_GREEN,
            "Init OK");
    lcd_xfer_ram_swap_16bit(&lcd_096);
    MLOGI("init ok!\n");

    while (1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}












