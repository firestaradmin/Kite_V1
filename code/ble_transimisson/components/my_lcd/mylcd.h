
/**
 * @file mylcd.h
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/06
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/06: v0.0.1 Initial Version
 *  - ----/--/--: --
 */


#pragma once
// #include <stdint.h>
#include "stdint.h"
#include "mylcd_math.h"
#include "ugfont.h"
#include "my_lcd_define.h"
#include "mylcd_draw.h"

int lcd_init(LcdScreenType lcdType, LcdConfigure *config, LcdDevice *dev_dest, void *vram);
void lcd_led_switch(LcdDevice *dev, uint8_t v);
void lcd_drawPoint(LcdDevice *dev, uint16_t x,uint16_t y,uint16_t color);
void lcd_ram_drawPoint(LcdDevice *dev, uint16_t x, uint16_t y, uint16_t color);
void lcd_fill(LcdDevice *dev, uint16_t xsta,uint16_t ysta, uint16_t w, uint16_t h,uint16_t color);
void lcd_fill_fullscreen(LcdDevice *dev, uint16_t color);
void lcd_ram_fill(LcdDevice *dev, uint16_t xsta, uint16_t ysta, uint16_t w, uint16_t h, uint16_t color);
void lcd_ram_fill_bitmap(LcdDevice *dev, uint16_t xsta, uint16_t ysta, uint16_t w, uint16_t h, uint16_t *buf);
void lcd_xfer_ram(LcdDevice *dev);
void lcd_xfer_ram_swap_16bit(LcdDevice *dev);
void lcd_xfer_buf(LcdDevice *dev, uint16_t xsta, uint16_t ysta, uint16_t w, uint16_t h, uint8_t *buf, uint32_t bytes);

void my_lcd_test_demo(void);



void lcd_draw_sw_rgb565_swap(void *buf, uint32_t px_cnt);