/**
 * @file mylcd_draw.h
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/13
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/13: v0.0.1 Initial Version
 *  - ----/--/--: --
 */

#ifndef __UG_DRAW_ELEMENTS_H__
#define __UG_DRAW_ELEMENTS_H__
#include <stdint.h>
#include "mylcd.h"

typedef enum {
    UG_CIRCLE_PART_LOWRIGHT = 0,
    UG_CIRCLE_PART_LOWLEFT,
    UG_CIRCLE_PART_UPRIGHT,
    UG_CIRCLE_PART_UPLEFT,
}ug_circle_part_t;




/* Line */
void lcd_draw_line(LcdDevice *dev, int16_t x0, int16_t y0, int16_t x1, int16_t y1, lcd_color_t color);
void lcd_draw_wideLine(LcdDevice *dev, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t with, lcd_color_t color);

/* Rectangle */
void lcd_draw_rectangle(LcdDevice *dev, int16_t x, int16_t y, int16_t w, int16_t h, lcd_color_t color);
void lcd_draw_fillRectangle(LcdDevice *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,lcd_color_t color);

/* RoundRectangle */
void lcd_draw_roundRect(LcdDevice *dev, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,lcd_color_t color) ;

/* Circle */
void lcd_draw_circle(LcdDevice *dev, int16_t x0, int16_t y0, int16_t r,lcd_color_t color) ;
void lcd_draw_fillCircle(LcdDevice *dev, int16_t x0, int16_t y0, int16_t r,lcd_color_t color);
void lcd_draw_quarterCircle(LcdDevice *dev, int16_t x0, int16_t y0, int16_t r, ug_circle_part_t part,lcd_color_t color) ;

/* Tirangle */
void lcd_draw_triangle(LcdDevice *dev, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,lcd_color_t color)  ;



#endif // !__UG_DRAW_ELEMENTS_H__
