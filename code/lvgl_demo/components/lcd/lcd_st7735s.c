#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_st7735s.h"

static void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
static void VSPI_init(void);

DRAM_ATTR uint16_t lcd_ram[80][160];

esp_err_t ret;
static spi_device_handle_t spi;

// static spi_bus_config_t buscfg={
//     .miso_io_num=-1,
//     .mosi_io_num=PIN_NUM_MOSI,
//     .sclk_io_num=PIN_NUM_CLK,
//     .quadwp_io_num=-1,
//     .quadhd_io_num=-1,
//     .max_transfer_sz=80*160*2+8
// };
#define CONFIG_LCD_OVERCLOCK 
// static spi_device_interface_config_t devcfg={
// #ifdef CONFIG_LCD_OVERCLOCK
//     .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
// #else
//     .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
// #endif
//     .mode=0,                                //SPI mode 0
//     .spics_io_num=PIN_NUM_CS,               //CS pin
//     .queue_size=7,                          //We want to be able to queue 7 transactions at a time
//     .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
// };

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
    /* Sleep Out */
    {0x11, {0}, 1},
    /* Normal mode */
    {0xB1, {0x05, 0x3C, 0x3C}, 3},
    /* Idle mode */
    {0xB2, {0x05, 0x3C, 0x3C}, 3},
    /* Partial mode */
    {0xB3, {0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}, 6},
    /* Dot inversion */
    {0xB4, {0x03}, 1},
    /* AVDD GVDD */
    {0xC0, {0xAB, 0x0B, 0x04}, 3},
    /* VGH VGL */
    {0xC1, {0xC5}, 1},
    /* Normal mode */
    {0xC2, {0x0D, 0x00}, 2},
    /* Idle */
    {0xC3, {0x8D, 0x6A}, 2},
    /* Partial+Full */
    {0xC4, {0x8D, 0xEE}, 2},
    /* VCOM */
    {0xC5, {0x0F}, 1},
    /* positive gamma */
    {0xE0, {0x07, 0x0E, 0x08, 0x07, 0x10, 0x07, 0x02, 0x07, 0x09, 0x0F, 0x25, 0x36, 0x00, 0x08, 0x04, 0x10}, 16},
    /* negative gamma */
    {0xE1, {0x0A, 0x0D, 0x08, 0x07, 0x0F, 0x07, 0x02, 0x07, 0x09, 0x0F, 0x25, 0x35, 0x00, 0x09, 0x04, 0x10}, 16},

    {0x3A, {0x05}, 1},
    /* display dir */
    {0x36, {0x68}, 1},
    /* Display inversion */
    {0x21, {0}, 0},
    /* Display On */
    {0x29, {0}, 0},
    /* Set Column Address */
    {0x2A, {0x00, 0x1A, 0x00, 0x69}, 4},
    /* Set Page Address */
    {0x2B, {0x00, 0x01, 0x00, 0xA0}, 4},

    {0x2C, {0}, 0},
    {0, {0}, 0xff}

};





//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

// static void VSPI_init(void)
// {
//     //Initialize the SPI bus
//     ret=spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
//     ESP_ERROR_CHECK(ret);

//     //Attach the LCD to the SPI bus
//     ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
//     ESP_ERROR_CHECK(ret);

// }

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}
static void lcd_wr_reg(uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void lcd_wr_data_16bit(uint16_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t txdata[2];
    txdata[0] = data >> 8;
    txdata[1] = (uint8_t)data;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=2*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=txdata;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}




//Initialize the display
void lcd_init(void)
{
    //Initialize spi 
    // VSPI_init();
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=23,
        .sclk_io_num=18,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=80*160*2+8
    };
    spi_device_interface_config_t devcfg = {
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz = 60 * 1000 * 1000, // Clock out at 26 MHz
#else
        .clock_speed_hz = 10 * 1000 * 1000, // Clock out at 10 MHz
#endif
        .mode = 0,                               // SPI mode 0
        .spics_io_num = 5,                       // CS pin
        .queue_size = 7,                         // We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
        
    };
    devcfg.flags |= SPI_DEVICE_NO_DUMMY;
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);


    
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);



    int cmd=0;  
    //Send all the commands
    while (st_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, st_init_cmds[cmd].cmd);
        lcd_data(spi, st_init_cmds[cmd].data, st_init_cmds[cmd].databytes&0x1F);
        if (st_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
        // printf("cmd:%d\r\n", cmd);

    }
    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 1);

    
}



/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void lcd_address_set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
		lcd_wr_reg(0x2a);//列地址设置
		lcd_wr_data_16bit(x1+26);
		lcd_wr_data_16bit(x2+26);
		lcd_wr_reg(0x2b);//行地址设置
		lcd_wr_data_16bit(y1+1);
		lcd_wr_data_16bit(y2+1);
		lcd_wr_reg(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		lcd_wr_reg(0x2a);//列地址设置
		lcd_wr_data_16bit(x1+26);
		lcd_wr_data_16bit(x2+26);
		lcd_wr_reg(0x2b);//行地址设置
		lcd_wr_data_16bit(y1+1);
		lcd_wr_data_16bit(y2+1);
		lcd_wr_reg(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		lcd_wr_reg(0x2a);//列地址设置
		lcd_wr_data_16bit(x1+1);
		lcd_wr_data_16bit(x2+1);
		lcd_wr_reg(0x2b);//行地址设置
		lcd_wr_data_16bit(y1+26);
		lcd_wr_data_16bit(y2+26);
		lcd_wr_reg(0x2c);//储存器写
	}
	else
	{
		lcd_wr_reg(0x2a);//列地址设置
		lcd_wr_data_16bit(x1+1);
		lcd_wr_data_16bit(x2+1);
		lcd_wr_reg(0x2b);//行地址设置
		lcd_wr_data_16bit(y1+26);
		lcd_wr_data_16bit(y2+26);
		lcd_wr_reg(0x2c);//储存器写
	}
}


/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void lcd_drawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	lcd_address_set(x,y,x,y);//设置光标位置 
	lcd_wr_data_16bit(color);
} 

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void lcd_fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	lcd_address_set(xsta,ysta,xend,yend);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			lcd_wr_data_16bit(color);
		}
	} 					  	    
}

void lcd_fill_ram(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	lcd_address_set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			lcd_ram[j][i] = color;
		}
	} 					  	    
}

void lcd_xfer_ram(void)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=80*160*2*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=lcd_ram;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


void lcd_xfer(uint8_t *buf,uint32_t px_cnt)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = px_cnt*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer = buf;               //Data
    t.user = (void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

