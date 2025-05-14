/**
 * @file my_lcd_type.h
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/23
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/23: v0.0.1 Initial Version
 *  - ----/--/--: --
 */




#pragma once

#include "stdint.h"
#include "ugcolor.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"



/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
 字节长度参数，最高位代表延时100ms，低7位代表数据长度，0xFF代表初始化参数结束
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} LcdInitCmdSeq;




/* define screen direction */
#define LCD_DIRECTION_CLOCKWISE_0       0
#define LCD_DIRECTION_CLOCKWISE_90      1
#define LCD_DIRECTION_CLOCKWISE_180     2
#define LCD_DIRECTION_CLOCKWISE_270     3
typedef enum LcdScreenDirection
{
    k_direction_clockwise_0 = LCD_DIRECTION_CLOCKWISE_0,
    k_direction_clockwise_90 = LCD_DIRECTION_CLOCKWISE_90,
    k_direction_clockwise_180 = LCD_DIRECTION_CLOCKWISE_180,
    k_direction_clockwise_270 = LCD_DIRECTION_CLOCKWISE_270
} LcdScreenDirection;


typedef enum LcdScreenType
{
    k_lcd_type_st7735s_096 = 0,
    k_lcd_type_st7789_114,
    k_lcd_type_st7789_190,
    k_lcd_type_st7789_190_wlk,
} LcdScreenType;


#define LCD_DEFAULT_CONFIG_ST7735S_096 {\
    .protocol_config = {\
        .spi_dev = HSPI_HOST,\
        .freq = 20000000,\
        .dma_ch = SPI_DMA_CH_AUTO\
    },\
    .gpio_config = {\
        .backlight_io_active_high = 1,\
        .io_num_mosi = 23,\
        .io_num_clk = 18,\
        .io_num_cs = 5,\
        .io_num_dc = 21,\
        .io_num_reset = 0,\
        .io_num_backlight = 22,\
        .io_num_tear_effect = -1\
    },\
    .direction = k_direction_clockwise_0}


#define LCD_DEFAULT_CONFIG_ST7789_114 {\
    .protocol_config = {\
        .spi_dev = SPI2_HOST,\
        .freq = 20000000,\
        .dma_ch = SPI_DMA_CH_AUTO\
    },\
    .gpio_config = {\
        .backlight_io_active_high = 0,\
        .io_num_mosi = 11,\
        .io_num_clk = 12,\
        .io_num_cs = 10,\
        .io_num_dc = 14,\
        .io_num_reset = 15,\
        .io_num_backlight = 9,\
        .io_num_tear_effect = -1\
    },\
    .direction = k_direction_clockwise_0}

#define LCD_DEFAULT_CONFIG_ST7789_190 {\
    .protocol_config = {\
        .spi_dev = SPI2_HOST,\
        .freq = 20000000,\
        .dma_ch = SPI_DMA_CH_AUTO\
    },\
    .gpio_config = {\
        .backlight_io_active_high = 0,\
        .io_num_mosi = 11,\
        .io_num_clk = 12,\
        .io_num_cs = 36,\
        .io_num_dc = 34,\
        .io_num_reset = 33,\
        .io_num_backlight = 35,\
        .io_num_tear_effect = -1\
    },\
    .direction = k_direction_clockwise_0}

#define LCD_DEFAULT_CONFIG_ST7789_190_WLK {\
    .protocol_config = {\
        .spi_dev = SPI2_HOST,\
        .freq = 20000000,\
        .dma_ch = SPI_DMA_CH_AUTO\
    },\
    .gpio_config = {\
        .backlight_io_active_high = 0,\
        .io_num_mosi = 11,\
        .io_num_clk = 12,\
        .io_num_cs = 10,\
        .io_num_dc = 13,\
        .io_num_reset = 14,\
        .io_num_backlight = 9,\
        .io_num_tear_effect = 8\
    },\
    .direction = k_direction_clockwise_0}







/* screen device gpio config */
typedef struct LcdGpioConfig{
    int io_num_mosi;
    int io_num_clk;
    int io_num_cs;
    int io_num_dc;
    int io_num_reset;
    int io_num_backlight;
    int io_num_tear_effect;

    uint8_t backlight_io_active_high;   // 0: active low level, 1: active high level
} LcdGpioConfig;


/* screen device protocol config */
typedef struct LcdProtoclConfig{
    spi_host_device_t spi_dev;
    int freq;
    spi_common_dma_t dma_ch;
} LcdProtoclConfig;


/* screen device struct */
typedef struct LcdConfigure{
    LcdScreenDirection direction;
    LcdGpioConfig gpio_config;
    LcdProtoclConfig protocol_config;
} LcdConfigure;


/* screen device struct */
struct LcdDevice;
typedef struct LcdDevice{
    LcdScreenType lcd_type;
    const char *desc;
    const char *vendor;
    LcdConfigure config;
    uint16_t res_w;
    uint16_t res_h;
    void *vram;

    spi_device_handle_t _spi_dev;
    // void (*pfunc_init)(LcdConfigure *config, struct LcdDevice *dev_dest);
    void (*pfunc_wr_cmd)(struct LcdDevice *dev, uint8_t cmd);
    void (*pfunc_wr_data)(struct LcdDevice *dev, const uint8_t *data, uint32_t bytes);
    void (*pfunc_wr_data16)(struct LcdDevice *dev, uint16_t data16);
    void (*pfunc_wr_reg)(struct LcdDevice *dev, uint8_t reg);
    void (*pfunc_set_win)(struct LcdDevice *dev, uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
    void (*pfunc_led_switch)(struct LcdDevice *dev, uint8_t v);

    uint32_t initialized : 1;
} LcdDevice;






