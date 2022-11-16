
#ifndef __LCD_ST7735S_H__
#define __LCD_ST7735S_H__
/* 0.96inch 80x160 pixel */
#include "stdint.h"
/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;


#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define PIN_NUM_DC   21
#define PIN_NUM_RST  0
#define PIN_NUM_BCKL 22

/*
 The screen display direction.
 0: Horizontal
 1: 180 degrees horizontal
*/
#define USE_HORIZONTAL 2

//画笔颜色
#define WHITE         	    0xFFFF
#define BLACK         	    0x0000	  
#define BLUE           	    0x001F  
#define BRED                0XF81F
#define GRED 		        0XFFE0
#define GBLUE			    0X07FF
#define RED           	    0xF800
#define MAGENTA       	    0xF81F
#define GREEN         	    0x07E0
#define CYAN          	    0x7FFF
#define YELLOW        	    0xFFE0
#define BROWN 			    0XBC40  //棕色
#define BRRED 			    0XFC07  //棕红色
#define GRAY  			    0X8430  //灰色
#define DARKBLUE      	    0X01CF  //深蓝色
#define LIGHTBLUE      	    0X7D7C	//浅蓝色  
#define GRAYBLUE       	    0X5458  //灰蓝色
#define LIGHTGREEN     	    0X841F  //浅绿色
#define LGRAY 			    0XC618  //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE           0XA651  //浅灰蓝色(中间层颜色)
#define LBBLUE              0X2B12  //浅棕蓝色(选择条目的反色)


void lcd_init(void);
uint32_t lcd_get_id(void);
void lcd_fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
void lcd_drawPoint(uint16_t x,uint16_t y,uint16_t color);
void lcd_fill_ram(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
void lcd_xfer_ram(void);

void lcd_xfer(uint8_t *buf,uint32_t px_cnt);
/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_wr_data_16bit(uint16_t data);
#endif

