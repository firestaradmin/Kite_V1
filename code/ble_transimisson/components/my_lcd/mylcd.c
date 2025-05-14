/**
 * @file mylcd.c
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

#include "mylcd.h"

#include "esp_log.h"
static const char* TAG = "MYLCD";

extern void lcd_init_st7735s_096(LcdConfigure *config, LcdDevice *dev_dest);
extern void lcd_init_st7789_114(LcdConfigure *config, LcdDevice *dev_dest);
extern void lcd_init_st7789_190(LcdConfigure *config, LcdDevice *dev_dest);
extern void lcd_init_st7789_190_wlk(LcdConfigure *config, LcdDevice *dev_dest);

int lcd_init(LcdScreenType lcdType, LcdConfigure *config, LcdDevice *dev_dest, void* vram)
{
    dev_dest->lcd_type = lcdType;
    dev_dest->vram = vram;
    memcpy(&dev_dest->config, config, sizeof(LcdConfigure));
    switch (lcdType)
    {
        case k_lcd_type_st7735s_096:
            lcd_init_st7735s_096(config, dev_dest);
            break;
        case k_lcd_type_st7789_114:
            lcd_init_st7789_114(config, dev_dest);
            break;
        case k_lcd_type_st7789_190:
            lcd_init_st7789_190(config, dev_dest);
            break;
        case k_lcd_type_st7789_190_wlk:
            lcd_init_st7789_190_wlk(config, dev_dest);
            break;

        default:
            ESP_LOGE(TAG, "unsupport screen type.");
            return -1;
            break;
    }
    dev_dest->initialized = 1;

    return 0;
}


void lcd_led_switch(LcdDevice *dev, uint8_t v)
{
    if(dev->pfunc_led_switch != NULL)
        dev->pfunc_led_switch(dev, v);
}




void lcd_drawPoint(LcdDevice *dev, uint16_t x,uint16_t y,uint16_t color)
{
	dev->pfunc_set_win(dev, x,y,x,y);//设置光标位置 
	dev->pfunc_wr_data16(dev, color);
} 



/**
 * @brief transfer internal lcd ram to lcd
*/
void lcd_xfer_buf(LcdDevice *dev, uint16_t xsta,uint16_t ysta,uint16_t w,uint16_t h,uint8_t *buf, uint32_t bytes)
{

	dev->pfunc_set_win(dev, xsta,ysta,xsta+w-1,ysta+h-1);

    dev->pfunc_wr_data(dev, buf, bytes);

}


/**
 * @brief fill area
*/
void lcd_fill(LcdDevice *dev, uint16_t xsta,uint16_t ysta, uint16_t w, uint16_t h,uint16_t color)
{          
	uint16_t i,j; 
	dev->pfunc_set_win(dev, xsta,ysta,xsta+w-1,ysta+h-1);//设置范围
	for(i=0;i<h;i++)
	{													   	 	
		for(j=0;j<w;j++)
		{
	        dev->pfunc_wr_data16(dev, color);
		}
	} 					  	    
}

void lcd_fill_fullscreen(LcdDevice *dev, uint16_t color)
{          
	uint16_t i,j; 
	dev->pfunc_set_win(dev, 0,0,dev->res_w-1,dev->res_h-1);//设置范围
	for(i=0;i<dev->res_h;i++)
	{													   	 	
		for(j=0;j<dev->res_w;j++)
		{
	        dev->pfunc_wr_data16(dev, color);
		}
	} 					  	    
}


/**
 * @brief draw point in internal lcd ram only, will not refresh to lcd, 
 *              need call func lcd_xfer_ram() to refresh.
*/
void lcd_ram_drawPoint(LcdDevice *dev, uint16_t x,uint16_t y,uint16_t color)
{
    if (dev->vram == NULL){
        return;
    }
    uint16_t *pvram = (uint16_t *)dev->vram;
    if (x >= dev->res_w | y >= dev->res_h)
        return;
    pvram[y*dev->res_w+x] = color;
} 



/**
 * @brief fill internal lcd ram only, will not refresh to lcd, 
 *              need call func lcd_xfer_ram() to refresh.
*/
void lcd_ram_fill(LcdDevice *dev, uint16_t xsta,uint16_t ysta,uint16_t w,uint16_t h,uint16_t color)
{          
    if (dev->vram == NULL){
        return;
    }
    uint16_t *pvram = (uint16_t *)dev->vram;
    uint16_t i,j; 
	for(i=0;i<h;i++)
	{													   	 	
		for(j=0;j<w;j++)
		{
			pvram[i+xsta*(dev->res_w+(j+ysta))] = color;
		}
	} 		
    
    
}

// /**  with BUGS
//  * @brief fill internal lcd ram only, will not refresh to lcd, 
//  *              need call func lcd_xfer_ram() to refresh.           
// */
// void lcd_ram_fill_fullscreen(LcdDevice *dev, uint16_t color)
// {          
//     if (dev->vram == NULL){
//         return;
//     }
//     uint16_t *pvram = (uint16_t *)dev->vram;
//     uint16_t i,j; 
// 	for(i=0;i<dev->res_h;i++)
// 	{													   	 	
// 		for(j=0;j<j<dev->res_w;j++)
// 		{
// 			pvram[i+j*dev->res_w] = color;
// 		}
// 	} 		

// }




/**
 * @brief fill internal lcd ram only, will not refresh to lcd, 
 *              need call func lcd_xfer_ram() to refresh.
*/
void lcd_ram_fill_bitmap(LcdDevice *dev, uint16_t xsta,uint16_t ysta,uint16_t w,uint16_t h,uint16_t *buf)
{          
    if (dev->vram == NULL){
        return;
    }
    uint16_t *pvram = (uint16_t *)dev->vram;
    uint16_t i,j;
    int cnt = 0;
    // xsta = UG_MATH_MIN(xsta, dev->res_w);
    // ysta = UG_MATH_MIN(xsta, dev->res_h);
    // if(xsta + w > dev->res_w)
    //     w = dev->res_w - xsta;
    // if(ysta + h > dev->res_h)
    //     h = dev->res_h - ysta;

    uint16_t x, y;
    // uint16_t startpx = ysta * dev->res_w + xsta;
    for(i=0;i<h;i++)
	{													   	 	
		for(j=0;j<w;j++)
		{
            x = xsta + j;
            y = ysta + i;
            if (x >= dev->res_w | y >= dev->res_h)
                return;
            pvram[y * dev->res_w + x] = buf[cnt++];
		}
	} 					  	    
}

/**
 * @brief transfer internal lcd ram to lcd
*/
void lcd_xfer_ram(LcdDevice *dev)
{
    if (dev->vram == NULL){
        return;
    }
	dev->pfunc_set_win(dev, 0,0,dev->res_w-1,dev->res_h-1);//设置显示范围

    dev->pfunc_wr_data(dev, dev->vram, dev->res_h*dev->res_w*2);

}
/**
 * @brief transfer internal lcd ram to lcd
*/
void lcd_xfer_ram_swap_16bit(LcdDevice *dev)
{
    if (dev->vram == NULL){
        return;
    }
    lcd_draw_sw_rgb565_swap(dev->vram, dev->res_h * dev->res_w);
    dev->pfunc_set_win(dev, 0,0,dev->res_w-1,dev->res_h-1);//设置显示范围

    dev->pfunc_wr_data(dev, dev->vram, dev->res_h*dev->res_w*2);

}





// uint32_t lcd_get_id(void)
// {
//     // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
//     spi_device_acquire_bus(_spi_dev, portMAX_DELAY);

//     //get_id cmd
//     lcd_cmd(_spi_dev, 0x04);

//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));
//     t.length = 8 * 3;
//     t.flags = SPI_TRANS_USE_RXDATA|SPI_TRANS_CS_KEEP_ACTIVE;
//     t.user = (void*)1;

//     esp_err_t ret = spi_device_polling_transmit(_spi_dev, &t);
//     assert(ret == ESP_OK);

//     // Release bus
//     spi_device_release_bus(_spi_dev);

//     return *(uint32_t*)t.rx_data;
// }



/******************** DEMO ***************************************************************************/

void my_lcd_test_demo(void)
{
    int ret;

    LcdDevice lcd_190;
    LcdConfigure lcd_config_190 = LCD_DEFAULT_CONFIG_ST7789_190;
    ret = lcd_init(k_lcd_type_st7789_190,&lcd_config_190, &lcd_190,NULL);


    LcdDevice lcd_114;
    LcdConfigure lcd_config_114 = LCD_DEFAULT_CONFIG_ST7789_114;
    ret = lcd_init(k_lcd_type_st7789_114,&lcd_config_114, &lcd_114,NULL);

    lcd_color16_t forecolor = UG_COLOR_RED;
    lcd_color16_t backcolor = UG_COLOR_WHITE;

    lcd_fill_fullscreen(&lcd_114, backcolor.full);
    uint16_t w = 60;
    uint16_t h = 30;
    uint16_t x = (lcd_114.res_w - w) / 2 - 1;
    uint16_t y = (lcd_114.res_h - h) / 2 - 1;
    lcd_fill(&lcd_114, x, y, w, h, forecolor.full);
    ug_draw_str(&lcd_114, 0, 0, &ug_font_consolas_16px_15, forecolor, backcolor,
                "@firestaradmin");

    lcd_fill_fullscreen(&lcd_190, backcolor.full);
    uint16_t w1 = 60;
    uint16_t h1 = 30;
    uint16_t x1 = (lcd_190.res_w - w) / 2 - 1;
    uint16_t y1 = (lcd_190.res_h - h) / 2 - 1;
    lcd_fill(&lcd_190, x1, y1, w1, h1, forecolor.full);


    ug_draw_str(&lcd_190, 0, 0, &ug_font_consolas_16px_15, forecolor, backcolor,
                "@firestaradmin");



}
/******************** DEMO - END *********************************************************************/





/**
 * @brief 反转16位深度颜色顺序，如rgb -> bgr
 * 
 * @param buf 
 * @param px_cnt 
 */
void lcd_draw_sw_rgb565_swap(void * buf, uint32_t px_cnt)
{
    // if(LV_DRAW_SW_RGB565_SWAP(buf, buf_size_px) == LV_RESULT_OK) return;

    uint32_t u32_cnt = px_cnt / 2;
    uint16_t * buf16 = buf;
    uint32_t * buf32 = buf;

    while(u32_cnt >= 8) {
        buf32[0] = ((buf32[0] & 0xff00ff00) >> 8) | ((buf32[0] & 0x00ff00ff) << 8);
        buf32[1] = ((buf32[1] & 0xff00ff00) >> 8) | ((buf32[1] & 0x00ff00ff) << 8);
        buf32[2] = ((buf32[2] & 0xff00ff00) >> 8) | ((buf32[2] & 0x00ff00ff) << 8);
        buf32[3] = ((buf32[3] & 0xff00ff00) >> 8) | ((buf32[3] & 0x00ff00ff) << 8);
        buf32[4] = ((buf32[4] & 0xff00ff00) >> 8) | ((buf32[4] & 0x00ff00ff) << 8);
        buf32[5] = ((buf32[5] & 0xff00ff00) >> 8) | ((buf32[5] & 0x00ff00ff) << 8);
        buf32[6] = ((buf32[6] & 0xff00ff00) >> 8) | ((buf32[6] & 0x00ff00ff) << 8);
        buf32[7] = ((buf32[7] & 0xff00ff00) >> 8) | ((buf32[7] & 0x00ff00ff) << 8);
        buf32 += 8;
        u32_cnt -= 8;
    }

    while(u32_cnt) {
        *buf32 = ((*buf32 & 0xff00ff00) >> 8) | ((*buf32 & 0x00ff00ff) << 8);
        buf32++;
        u32_cnt--;
    }

    if(px_cnt & 0x1) {
        uint32_t e = px_cnt - 1;
        buf16[e] = ((buf16[e] & 0xff00) >> 8) | ((buf16[e] & 0x00ff) << 8);
    }

}

















/************************
 *   test
 ************************/

void _test_screen(void)
{
    LcdDevice lcd_190;
    LcdConfigure lcd_config_190 = LCD_DEFAULT_CONFIG_ST7789_190_WLK;
    lcd_config_190.direction = k_direction_clockwise_270;
    int ret = lcd_init(k_lcd_type_st7789_190_wlk,&lcd_config_190, &lcd_190,NULL);

    lcd_color16_t forecolor = UG_COLOR_RED;
    lcd_color16_t backcolor = UG_COLOR_WHITE;

    lcd_fill_fullscreen(&lcd_190, backcolor.full);
    uint16_t w = 60;
    uint16_t h = 30;
    uint16_t x = (lcd_190.res_w - w) / 2 - 1;
    uint16_t y = (lcd_190.res_h - h) / 2 - 1;
    lcd_fill(&lcd_190, x, y, w, h, forecolor.full);
    ug_draw_str(&lcd_190, 0, 0, &ug_font_consolas_16px_15, forecolor, backcolor,
                "@firestaradmin");

}

