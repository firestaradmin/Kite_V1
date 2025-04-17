/**
 * @file ugcolor.h
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/27
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/27: v0.0.1 Initial Version
 *  - ----/--/--: --
 */


#ifndef __UG_COLOR_H__
#define __UG_COLOR_H__

#include "stdint.h"



#define UG_COLOR_DEPTH          16
// #define UG_COLOR_RGB_ORDER      1

#if UG_COLOR_DEPTH == 16

typedef union {
    struct {
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
    } ch;
    uint16_t full;
} lcd_color16_t;
typedef lcd_color16_t lcd_color_t;

#elif UG_COLOR_DEPTH == 32
typedef union lcd_color32_t
{
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } ch;
    uint32_t full;
} lcd_color32_t;
typedef lcd_color16_t lcd_color_t;
#endif

#if (0)
/* The most simple macro to create a color from R,G and B values */
#if UG_COLOR_DEPTH == 1
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){.full = (uint8_t)((r8 >> 7) | (g8 >> 7) | (b8 >> 7))})
#elif UG_COLOR_DEPTH == 8
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{(uint8_t)((r8 >> 5) & 0x7U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((b8 >> 6) & 0x3U)}})
#elif UG_COLOR_DEPTH == 16
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{(uint16_t)((r8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x3FU), (uint16_t)((b8 >> 3) & 0x1FU)}})
#elif UG_COLOR_DEPTH == 32
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{r8, g8, b8, 0xff}}) /*Fix 0xff alpha*/
#endif
#else
/* The most simple macro to create a color from R,G and B values */
#if UG_COLOR_DEPTH == 1
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){.full = (uint8_t)((b8 >> 7) | (g8 >> 7) | (r8 >> 7))})
#elif UG_COLOR_DEPTH == 8
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{(uint8_t)((b8 >> 6) & 0x3U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 5) & 0x7U)}})
#elif UG_COLOR_DEPTH == 16
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{(uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x3FU), (uint16_t)((r8 >> 3) & 0x1FU)}})
#elif UG_COLOR_DEPTH == 32
#define UG_COLOR_MAKE(r8, g8, b8) ((lcd_color_t){{b8, g8, r8, 0xff}}) /*Fix 0xff alpha*/
#endif
#endif

#define UG_COLOR(color) UG_COLOR_MAKE(((color >> 16) & 0xFFU), ((color >> 8) & 0xFFU), ((color) & 0xFFU))

#define UG_MATH_UDIV255(x) ((uint32_t)((uint32_t) (x) * 0x8081) >> 0x17)

#define UG_COLOR_SET_R16(c, v) 	(c).ch.red = (uint8_t)(v) & 0x1FU;  
#define UG_COLOR_SET_G16(c, v) 	(c).ch.green = (uint8_t)(v) & 0x3FU;
#define UG_COLOR_SET_B16(c, v) 	(c).ch.blue = (uint8_t)(v) & 0x1FU;
#define UG_COLOR_GET_R16(c) 	(c).ch.red
#define UG_COLOR_GET_G16(c) 	(c).ch.green
#define UG_COLOR_GET_B16(c) 	(c).ch.blue




/*---------------------------------------
    * Macros for the current color depth
    * to set/get values of the color channels
    *------------------------------------------*/
#if UG_COLOR_DEPTH == 1

#elif UG_COLOR_DEPTH == 8

#elif UG_COLOR_DEPTH == 16
# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R16(c,v)
# define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G16(c,v)
# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B16(c,v)
# define UG_COLOR_GET_R(c) UG_COLOR_GET_R16(c)
# define UG_COLOR_GET_G(c) UG_COLOR_GET_G16(c)
# define UG_COLOR_GET_B(c)   UG_COLOR_GET_B16(c)

#elif UG_COLOR_DEPTH == 32

#endif




/**
 * Mix two colors with a given ratio.
 * @param c1 the first color to mix (usually the foreground)
 * @param c2 the second color to mix (usually the background)
 * @param mix The ratio of the colors. 0: full `c2`, 255: full `c1`, 127: half `c1` and half`c2`
 * @return the mixed color
 */
static inline lcd_color_t ug_color_mix(lcd_color_t c1, lcd_color_t c2, uint8_t mix)
{
    lcd_color_t ret;
#if UG_COLOR_DEPTH != 1
    /*UG_COLOR_DEPTH == 8, 16 or 32*/
    UG_COLOR_SET_R(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_R(c1) * mix + UG_COLOR_GET_R(c2) *
                                        (255 - mix) ));
    UG_COLOR_SET_G(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_G(c1) * mix + UG_COLOR_GET_G(c2) *
                                        (255 - mix) ));
    UG_COLOR_SET_B(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_B(c1) * mix + UG_COLOR_GET_B(c2) *
                                        (255 - mix) ));
#else
    /*UG_COLOR_DEPTH == 1*/
    ret.full = mix > UG_OPA_50 ? c1.full : c2.full;
#endif

    return ret;
}



#define   UG_COLOR_RED                  UG_COLOR(0xF44336)
#define   UG_COLOR_PINK                 UG_COLOR(0xE91E63)
#define   UG_COLOR_PURPLE               UG_COLOR(0x9C27B0)
#define   UG_COLOR_DEEP_PURPLE          UG_COLOR(0x673AB7)
#define   UG_COLOR_INDIGO               UG_COLOR(0x3F51B5)
#define   UG_COLOR_BLUE                 UG_COLOR(0x2196F3)
#define   UG_COLOR_LIGHT_BLUE           UG_COLOR(0x03A9F4)
#define   UG_COLOR_CYAN                 UG_COLOR(0x00BCD4)
#define   UG_COLOR_TEAL                 UG_COLOR(0x009688)
#define   UG_COLOR_GREEN                UG_COLOR(0x4CAF50)
#define   UG_COLOR_LIGHT_GREEN          UG_COLOR(0x8BC34A)
#define   UG_COLOR_LIME                 UG_COLOR(0xCDDC39)
#define   UG_COLOR_YELLOW               UG_COLOR(0xFFEB3B)
#define   UG_COLOR_AMBER                UG_COLOR(0xFFC107)
#define   UG_COLOR_ORANGE               UG_COLOR(0xFF9800)
#define   UG_COLOR_DEEP_ORANGE          UG_COLOR(0xF4511E)
#define   UG_COLOR_BROWN                UG_COLOR(0x795548)
#define   UG_COLOR_BLUE_GRAY            UG_COLOR(0x607D8B)
#define   UG_COLOR_WHITE                UG_COLOR(0xFFFFFF)
#define   UG_COLOR_GRAY_50              UG_COLOR(0xFAFAFA)
#define   UG_COLOR_GRAY_100             UG_COLOR(0xF5F5F5)
#define   UG_COLOR_GRAY_200             UG_COLOR(0xEEEEEE)
#define   UG_COLOR_GRAY_300             UG_COLOR(0xE0E0E0)
#define   UG_COLOR_GRAY_400             UG_COLOR(0xBDBDBD)
#define   UG_COLOR_GRAY_500             UG_COLOR(0x9E9E9E)
#define   UG_COLOR_GRAY_600             UG_COLOR(0x757575)
#define   UG_COLOR_GRAY_700             UG_COLOR(0x616161)
#define   UG_COLOR_GRAY_800             UG_COLOR(0x424242)
#define   UG_COLOR_GRAY_900             UG_COLOR(0x212121)
#define   UG_COLOR_DEEP_GRAY            UG_COLOR(0x1f1f1f)
#define   UG_COLOR_BLACK                UG_COLOR(0x000000)



#endif // !__UG_COLOR_H__
