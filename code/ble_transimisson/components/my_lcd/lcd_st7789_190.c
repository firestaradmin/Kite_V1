/**
 * @file lcd_st7789.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "stdint.h"
#include "mylcd.h"

#include "esp_log.h"
static const char* TAG = "LCD_ST7789_190";

#if 1
#define _VENDOR "ZhongJingYuan"
#define _DESC "1.90 inch 170x320 IPS"


#define _RESOLUTION_SHORT   170
#define _RESOLUTION_LONG    320



// static void _lcd_init(LcdConfigure *config, LcdDevice *dev_dest);
static void _lcd_led_switch(LcdDevice *dev, uint8_t v);
static void _lcd_wr_cmd(LcdDevice *dev, uint8_t cmd);
static void _lcd_wr_data(LcdDevice *dev, const uint8_t *data, uint32_t len);
static void _lcd_wr_data_queue(LcdDevice *dev, uint8_t *buf, uint32_t len);
static void _lcd_wr_data_16bit(LcdDevice *dev, uint16_t data);
static void _lcd_wr_reg(LcdDevice *dev, uint8_t reg);
static void _lcd_address_set(LcdDevice *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


// Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static LcdInitCmdSeq _lcd_init_cmds[] = {
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    // {0x36, {(1 << 5) | (1 << 6)}, 1},
    {0x11, {0}, 0},
    {0x36, {0x00}, 1}, // 占位，屏幕内存刷新方向配置和颜色顺序配置，在init函数根据屏幕方向会修改此值

    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x05}, 1},
    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x35}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x1A}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01}, 1},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x0B}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0f}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 2},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xF0,0x00,0x04,0x04,0x04,0x05,0x29,0x33,0x3E,0x38,0x12,0x12,0x28,0x30}, 14},

    /* Negative Voltage Gamma Control */
    {0xE1, {0xF0,0x07,0x0A,0x0D,0x0B,0x07,0x28,0x33,0x3E,0x36,0x14,0x14,0x29,0x32}, 14},
    /* Sleep Out */
    {0x21, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}};

// Initialize the display
void lcd_init_st7789_190(LcdConfigure *config, LcdDevice *dev_dest)
{

    dev_dest->vendor = _VENDOR;
    dev_dest->desc = _DESC;
    ESP_LOGI(TAG, "_lcd_init [%s] start", dev_dest->desc);
    
    dev_dest->pfunc_wr_cmd = _lcd_wr_cmd;
    dev_dest->pfunc_wr_data = _lcd_wr_data;
    dev_dest->pfunc_wr_data16 = _lcd_wr_data_16bit;
    dev_dest->pfunc_wr_reg = _lcd_wr_reg;
    dev_dest->pfunc_set_win = _lcd_address_set;
    dev_dest->pfunc_led_switch = _lcd_led_switch;

    if ((uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_0) ||
        (uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_180))
    {
        dev_dest->res_w = _RESOLUTION_SHORT;
        dev_dest->res_h = _RESOLUTION_LONG;
    }
    else{
        dev_dest->res_w = _RESOLUTION_LONG;
        dev_dest->res_h = _RESOLUTION_SHORT;
    }

    
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = config->gpio_config.io_num_mosi,
        .sclk_io_num = config->gpio_config.io_num_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = _RESOLUTION_SHORT * _RESOLUTION_LONG * 2 + 8,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = config->protocol_config.freq,          // 没有TE脚和读数据脚，只能慢慢测试速度，尽量保证和屏幕刷新速度一致来减少 撕裂
        .mode = 0,                                // SPI mode 0
        .spics_io_num = config->gpio_config.io_num_cs,              // CS pin
        .queue_size = 7,                          // We want to be able to queue 7 transactions at a time
        .pre_cb = NULL, // Specify pre-transfer callback to handle D/C line
        // .pre_cb = _lcd_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line

    };
    // devcfg.flags |= SPI_DEVICE_3WIRE;
    devcfg.flags |= SPI_DEVICE_NO_DUMMY;
    // Initialize the SPI bus
    ret = spi_bus_initialize(config->protocol_config.spi_dev, &buscfg, config->protocol_config.dma_ch);
    // ESP_ERROR_CHECK(ret);
    if (ret == ESP_ERR_INVALID_STATE){
        ESP_LOGW(TAG, "SPI already initialized.");
    }

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(config->protocol_config.spi_dev, &devcfg, &dev_dest->_spi_dev);
    ESP_ERROR_CHECK(ret);


    // Initialize non-SPI GPIOs
    gpio_set_direction(config->gpio_config.io_num_dc, GPIO_MODE_OUTPUT);
    if (config->gpio_config.io_num_reset >= 0)
        gpio_set_direction(config->gpio_config.io_num_reset, GPIO_MODE_OUTPUT);
    if (config->gpio_config.io_num_backlight >= 0)
        gpio_set_direction(config->gpio_config.io_num_backlight, GPIO_MODE_OUTPUT);

    // Reset the display
    if (config->gpio_config.io_num_reset >= 0)
    {
        gpio_set_level(config->gpio_config.io_num_reset, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(config->gpio_config.io_num_reset, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }


    int i = 0;
    // Send all the commands
    while (_lcd_init_cmds[i].databytes != 0xff)
    {
        if (_lcd_init_cmds[i].cmd == 0x36){ // change screen direction
            if ((uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_0))
                _lcd_init_cmds[i].data[0] = 0x00;
            else if ((uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_180))
                _lcd_init_cmds[i].data[0] = 0xC0;
            else if ((uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_90))
                _lcd_init_cmds[i].data[0] = 0x70;
            else if ((uint8_t)(config->direction) == (uint8_t)(k_direction_clockwise_270))
                _lcd_init_cmds[i].data[0] = 0xA0;
        }
        _lcd_wr_cmd(dev_dest, _lcd_init_cmds[i].cmd);
        _lcd_wr_data(dev_dest, _lcd_init_cmds[i].data, _lcd_init_cmds[i].databytes & 0x1F);
        if (_lcd_init_cmds[i].databytes & 0x80)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        i++;
    }

    /// Enable backlight
    _lcd_led_switch(dev_dest, 1);
    // if (config->gpio_config.io_num_backlight >= 0)
    //     gpio_set_level(config->gpio_config.io_num_backlight, config->gpio_config.backlight_io_active_high);

    ESP_LOGI(TAG, "_lcd_init [%s] end", dev_dest->desc);
}


/**
 * @brief switch backlight
 * 
 * @param dev 
 * @param v 0:off , 1: on
 */
static void _lcd_led_switch(LcdDevice *dev, uint8_t v)
{
    int _off = 0;
    int _on = 1;
    if (dev->config.gpio_config.backlight_io_active_high == 0) {
        _off = 1;
        _on = 0;
    }
    if (dev->config.gpio_config.io_num_backlight >= 0)
    {
        if (v != 0){
            gpio_set_level(dev->config.gpio_config.io_num_backlight, _on);
        }
        else {
            gpio_set_level(dev->config.gpio_config.io_num_backlight, _off);
        }
    }

}


// // This function is called (in irq context!) just before a transmission starts. It will
// // set the D/C line to the value indicated in the user field.
// static void _lcd_spi_pre_transfer_callback(spi_transaction_t *t)
// {
//     dc = t->user. 1: data, 0:cmd
//     int dc = (int)t->user;
//     if (_dev->config.gpio_config.io_num_dc >= 0)
//         gpio_set_level(_dev->config.gpio_config.io_num_dc, dc);
// }


/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
static void _lcd_wr_cmd(LcdDevice *dev, uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = 8;                                    // Command is 8 bits
    t.tx_buffer = &cmd;                              // The data is the cmd itself
    t.user = (void *)0;                              // D/C needs to be set to 0
    if (dev->config.gpio_config.io_num_dc >= 0)
        gpio_set_level(dev->config.gpio_config.io_num_dc, t.user);
    ret = spi_device_polling_transmit(dev->_spi_dev, &t); // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.
}

static void _lcd_wr_data(LcdDevice *dev, const uint8_t *data, uint32_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0)
        return;                                      // no need to send anything
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = len * 8;                              // Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;                              // Data
    t.user = (void *)1;                              // D/C needs to be set to 1
    if (dev->config.gpio_config.io_num_dc >= 0)
        gpio_set_level(dev->config.gpio_config.io_num_dc, t.user);
    ret = spi_device_polling_transmit(dev->_spi_dev, &t); // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.
}

/* DMA 适合整屏刷新，因为只是传递指针 */
static void _lcd_wr_data_queue(LcdDevice *dev, uint8_t *buf, uint32_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Zero out the transaction
    t.length = len * 8;       // Len is in bytes, transaction length is in bits.
    t.tx_buffer = buf;        // Data
    t.user = (void *)1;       // D/C needs to be set to 1
    if (dev->config.gpio_config.io_num_dc >= 0)
        gpio_set_level(dev->config.gpio_config.io_num_dc, t.user);
    ret = spi_device_queue_trans(dev->_spi_dev, &t, portMAX_DELAY);
    assert(ret == ESP_OK);
}

static void _lcd_wr_data_16bit(LcdDevice *dev, uint16_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t txdata[2];
    txdata[0] = data >> 8;
    txdata[1] = (uint8_t)data;
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = 2 * 8;                                // Len is in bytes, transaction length is in bits.
    t.tx_buffer = txdata;                            // Data
    t.user = (void *)1;                              // D/C needs to be set to 1
    if (dev->config.gpio_config.io_num_dc >= 0)
        gpio_set_level(dev->config.gpio_config.io_num_dc, t.user);
    ret = spi_device_polling_transmit(dev->_spi_dev, &t); // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.
}

static void _lcd_wr_reg(LcdDevice *dev, uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = 8;                                    // Command is 8 bits
    t.tx_buffer = &cmd;                              // The data is the cmd itself
    t.user = (void *)0;                              // D/C needs to be set to 0
    if (dev->config.gpio_config.io_num_dc >= 0)
        gpio_set_level(dev->config.gpio_config.io_num_dc, t.user);
    ret = spi_device_polling_transmit(dev->_spi_dev, &t); // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
static void _lcd_address_set(LcdDevice *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{

    if (dev->config.direction == k_direction_clockwise_0 || 
        dev->config.direction == k_direction_clockwise_180 )
    {
        _lcd_wr_reg(dev, 0x2a); // 列地址设置
        _lcd_wr_data_16bit(dev, x1 + 35);
        _lcd_wr_data_16bit(dev, x2 + 35);
        _lcd_wr_reg(dev, 0x2b); // 行地址设置
        _lcd_wr_data_16bit(dev, y1);
        _lcd_wr_data_16bit(dev, y2);
        _lcd_wr_reg(dev, 0x2c); // 储存器写
    }
    else if (dev->config.direction == k_direction_clockwise_90 ||
            dev->config.direction == k_direction_clockwise_270)
    {
        _lcd_wr_reg(dev, 0x2a); // 列地址设置
        _lcd_wr_data_16bit(dev, x1);
        _lcd_wr_data_16bit(dev, x2);
        _lcd_wr_reg(dev, 0x2b); // 行地址设置
        _lcd_wr_data_16bit(dev, y1 + 35);
        _lcd_wr_data_16bit(dev, y2 + 35);
        _lcd_wr_reg(dev, 0x2c); // 储存器写
    }
    else
    {
        ESP_LOGE(TAG, "lcd addr set: unsupport lcd screen direction.");
    }
}



#endif