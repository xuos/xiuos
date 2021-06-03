/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file connect_lcd.c
* @brief support aiit-riscv64-board lcd function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_lcd.c
Description: support aiit-riscv64-board lcd configure and lcd bus register function
Others: https://canaan-creative.com/developer
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-riscv64-board lcd configure, write and read
2. support aiit-riscv64-board lcd bus device and driver register
*************************************************/

#include <connect_lcd.h>
#include <board.h>
#include <drv_io_config.h>
#include <font.h>
#include <fpioa.h>
#include <gpiohs.h>
#include <graphic.h>

#define NO_OPERATION                      0x00
#define SOFTWARE_RESET                0x01
#define READ_ID                                     0x04
#define READ_STATUS                         0x09
#define READ_POWER_MODE           0x0A
#define READ_MADCTL                        0x0B
#define READ_PIXEL_FORMAT          0x0C
#define READ_IMAGE_FORMAT         0x0D
#define READ_SIGNAL_MODE           0x0E
#define READ_SELT_DIAG_RESULT 0x0F
#define SLEEP_ON                                 0x10
#define SLEEP_OFF                               0x11
#define PARTIAL_DISPALY_ON          0x12
#define NORMAL_DISPALY_ON          0x13
#define INVERSION_DISPALY_OFF   0x20
#define INVERSION_DISPALY_ON     0x21
#define GAMMA_SET                              0x26
#define DISPALY_OFF                             0x28
#define DISPALY_ON                              0x29
#define HORIZONTAL_ADDRESS_SET          0x2A
#define VERTICAL_ADDRESS_SET                  0x2B
#define MEMORY_WRITE                                    0x2C
#define COLOR_SET                                              0x2D
#define MEMORY_READ                                       0x2E
#define PARTIAL_AREA                                        0x30
#define VERTICAL_SCROL_DEFINE                 0x33
#define TEAR_EFFECT_LINE_OFF                   0x34
#define TEAR_EFFECT_LINE_ON                     0x35
#define MEMORY_ACCESS_CTL                        0x36
#define VERTICAL_SCROL_S_ADD                  0x37
#define IDLE_MODE_OFF                                    0x38
#define IDLE_MODE_ON                                      0x39
#define PIXEL_FORMAT_SET                             0x3A
#define WRITE_MEMORY_CONTINUE            0x3C
#define READ_MEMORY_CONTINUE              0x3E
#define SET_TEAR_SCANLINE                          0x44
#define GET_SCANLINE                                       0x45
#define WRITE_BRIGHTNESS                            0x51
#define READ_BRIGHTNESS                              0x52
#define WRITE_CTRL_DISPALY                         0x53
#define READ_CTRL_DISPALY                           0x54
#define WRITE_BRIGHTNESS_CTL                  0x55
#define READ_BRIGHTNESS_CTL                    0x56
#define WRITE_MIN_BRIGHTNESS                  0x5E
#define READ_MIN_BRIGHTNESS                    0x5F
#define READ_ID1                                                   0xDA
#define READ_ID2                                                   0xDB
#define READ_ID3                                                   0xDC
#define RGB_IF_SIGNAL_CTL                            0xB0
#define NORMAL_FRAME_CTL                           0xB1
#define IDLE_FRAME_CTL                                   0xB2
#define PARTIAL_FRAME_CTL                           0xB3
#define INVERSION_CTL                                      0xB4
#define BLANK_PORCH_CTL                              0xB5
#define DISPALY_FUNCTION_CTL                    0xB6
#define ENTRY_MODE_SET                                 0xB7
#define BACKLIGHT_CTL1                                   0xB8
#define BACKLIGHT_CTL2                                   0xB9
#define BACKLIGHT_CTL3                                   0xBA
#define BACKLIGHT_CTL4                                   0xBB
#define BACKLIGHT_CTL5                                   0xBC
#define BACKLIGHT_CTL7                                   0xBE
#define BACKLIGHT_CTL8                                   0xBF
#define POWER_CTL1                                            0xC0
#define POWER_CTL2                                            0xC1
#define VCOM_CTL1                                                0xC5
#define VCOM_CTL2                                                0xC7
#define NV_MEMORY_WRITE                              0xD0
#define NV_MEMORY_PROTECT_KEY             0xD1
#define NV_MEMORY_STATUS_READ             0xD2
#define READ_ID4                                                    0xD3
#define POSITIVE_GAMMA_CORRECT             0xE0
#define NEGATIVE_GAMMA_CORRECT            0xE1
#define DIGITAL_GAMMA_CTL1                         0xE2
#define DIGITAL_GAMMA_CTL2                         0xE3
#define INTERFACE_CTL                                       0xF6

typedef enum _lcd_dir
{
    DIR_XY_RLUD = 0x00,
    DIR_YX_RLUD = 0x20,
    DIR_XY_LRUD = 0x40,
    DIR_YX_LRUD = 0x60,
    DIR_XY_RLDU = 0x80,
    DIR_YX_RLDU = 0xA0,
    DIR_XY_LRDU = 0xC0,
    DIR_YX_LRDU = 0xE0,
    DIR_XY_MASK = 0x20,
    DIR_MASK = 0xE0,
} lcd_dir_t;

#define LCD_SPI_CHANNEL                 SPI_DEVICE_0
#define LCD_SPI_CHIP_SELECT         SPI_CHIP_SELECT_0

typedef struct Lcd8080Device
{
     struct LcdBus lcd_bus;
    struct DeviceLcdInfo lcd_info;
    int spi_channel;
    int cs;
    int dc_pin;
    int dma_channel;
} * Lcd8080DeviceType;

Lcd8080DeviceType lcd ;

static void DrvLcdCmd(uint8 cmd)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_LOW);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(lcd->spi_channel/*spi num*/, 8 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, &cmd, 1, SPI_TRANS_CHAR);
}

static void DrvLcdDataByte(uint8 *data_buf, uint32 length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(lcd->spi_channel, 8 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, data_buf, length, SPI_TRANS_CHAR);
}

static void DrvLcdDataHalfWord(uint16 *data_buf, uint32 length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 16, 0);
    spi_init_non_standard(lcd->spi_channel, 16 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, data_buf, length, SPI_TRANS_SHORT);
}

static void DrvLcdDataWord(uint32 *data_buf, uint32 length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
 /*spi  num      Polarity and phase mode   Multi-line mode    Data bit width    little endian  */
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);

/*  spi num      instrction length    address length     wait cycles    spi address trans mode*/
    spi_init_non_standard(lcd->spi_channel, 0 , 32, 0 ,SPI_AITM_AS_FRAME_FORMAT );

    /*dma  channel     spi num   chip_selete   tx_buff   tx_len    spi_trans_data_width */
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, data_buf, length, SPI_TRANS_INT);
}

static void DrvLcdHwInit(Lcd8080DeviceType lcd)
{
    gpiohs_set_drive_mode(lcd->dc_pin, GPIO_DM_OUTPUT);
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_set_clk_rate(lcd->spi_channel, 25000000);
}

static void DrvLcdSetDirection(lcd_dir_t dir)
{
#if !BOARD_LICHEEDAN
    dir |= 0x08;
#endif
    if (dir & DIR_XY_MASK) {
        lcd->lcd_info.width = 320;
        lcd->lcd_info.height = 240;
    } else {
        lcd->lcd_info.width = 240;
        lcd->lcd_info.height = 320;
    }

    DrvLcdCmd(MEMORY_ACCESS_CTL);
    DrvLcdDataByte((uint8 *)&dir, 1);
}

static void DrvLcdSetArea(uint16 x1, uint16 y1, uint16 x2, uint16 y2)
{
    uint8 data[4] = {0};

    data[0] = (uint8)(x1 >> 8);
    data[1] = (uint8)(x1);
    data[2] = (uint8)(x2 >> 8);
    data[3] = (uint8)(x2);
    DrvLcdCmd(HORIZONTAL_ADDRESS_SET);
    DrvLcdDataByte(data, 4);

    data[0] = (uint8)(y1 >> 8);
    data[1] = (uint8)(y1);
    data[2] = (uint8)(y2 >> 8);
    data[3] = (uint8)(y2);
    DrvLcdCmd(VERTICAL_ADDRESS_SET);
    DrvLcdDataByte(data, 4);

    DrvLcdCmd(MEMORY_WRITE);
}

static void DrvLcdSetPixel(uint16_t x, uint16_t y, uint16_t color)
{
    DrvLcdSetArea(x, y, x, y);
    DrvLcdDataHalfWord(&color, 1);
}

void LcdShowChar(uint16 x,uint16 y,uint8 num,uint8 size,uint16 color,uint16 back_color)
{  							  
    uint8 temp,t1,t;
	uint16 y0=y;
	uint8 csize=(size/8+((size%8)?1:0))*(size/2);	
 	num=num-' ';
	for (t = 0;t < csize;t ++) {   
		if (size==12)
            temp=asc2_1206[num][t]; 	 	     //1206
		else if (size==16)
            temp=asc2_1608[num][t];	//1608
		else if (size==24)
            temp=asc2_2412[num][t];	//2412
		else if (size==32)
            temp=asc2_3216[num][t];	//3216
		else
            return;				

		for(t1 = 0;t1 < 8;t1 ++) {			    
			if (temp&0x80)
                DrvLcdSetPixel(x,y,color);
            else
                DrvLcdSetPixel(x,y,back_color);

			temp<<=1;
			y++;
			if(y>=lcd->lcd_info.height)
                return;	
			if ((y-y0) == size) {
				y=y0;
				x++;
				if(x>=lcd->lcd_info.width)
                    return;	
				break;
			}
		}  	 
	}  	    	   	 	  
}   

void LcdShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8 size,uint8 *p,uint16_t color,uint16_t back_color)
{         
	uint16_t x0 = x;
	width += x;
	height += y;
    while ((*p<='~')&&(*p>=' ')) {       
        if (x>=width) {
            x=x0;
            y+=size;
        }
        if(y>=height)
            break;
        LcdShowChar(x,y,*p,size,color,back_color);
        x += size/2;
        p++;
    }  
}

/*
*   clear lcd by  color
*   para     color
*   return  none
*/
void DrvLcdClear(uint16 color)
{
    uint32 data = ((uint32)color << 16) | (uint32)color;

    DrvLcdSetArea(0, 0, lcd->lcd_info.width - 1, lcd->lcd_info.height - 1);
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(lcd->spi_channel,
                                                          0                         /*instrction length*/, 
                                                          32                       /*address length   */, 
                                                          0                         /*wait cycles            */,
                                                         SPI_AITM_AS_FRAME_FORMAT );
    spi_fill_data_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, (const uint32_t *)&data, lcd->lcd_info.width * lcd->lcd_info.height / 2);
}

static void DrvLcdRectUpdate(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
{
    static uint16 * rect_buffer = NONE;
    if (!rect_buffer) {
        rect_buffer = x_malloc(lcd->lcd_info.height * lcd->lcd_info.width * (lcd->lcd_info.bits_per_pixel / 8));
        if (!rect_buffer) {
            return;
        }
    }
    if (x1 == 0 && y1 == 0 && width == lcd->lcd_info.width && height == lcd->lcd_info.height) {
        DrvLcdSetArea(x1, y1, x1 + width - 1, y1 + height - 1);
        DrvLcdDataWord((uint32 *)lcd->lcd_info.framebuffer, width * height / (lcd->lcd_info.bits_per_pixel / 8));
    } else {
        DrvLcdSetArea(x1, y1, x1 + width - 1, y1 + height - 1);
        DrvLcdDataWord((uint32 *)rect_buffer, width * height / 2);
    }
}

x_err_t DrvLcdInit(Lcd8080DeviceType dev)
{
    x_err_t ret = EOK;
    lcd = (Lcd8080DeviceType)dev;
    uint8 data = 0;

    if (!lcd) {
        return ERROR;
    }
    DrvLcdHwInit(lcd);
    /* reset LCD */
    DrvLcdCmd(SOFTWARE_RESET);
    MdelayKTask(100);

    /* Enter normal status */
    DrvLcdCmd(SLEEP_OFF);
    MdelayKTask(100);

    /* pixel format rgb565 */
    DrvLcdCmd(PIXEL_FORMAT_SET);
    data = 0x55;
    DrvLcdDataByte(&data, 1);

    /* set direction */
    DrvLcdSetDirection(DIR_YX_RLUD);

    lcd->lcd_info.framebuffer = x_malloc(lcd->lcd_info.height * lcd->lcd_info.width * (lcd->lcd_info.bits_per_pixel / 8));
    CHECK(lcd->lcd_info.framebuffer);

    /*display on*/
    DrvLcdCmd(DISPALY_ON);

    return ret;
}

static x_err_t drv_lcd_control(Lcd8080DeviceType dev, int cmd, void *args)
{
    x_err_t ret = EOK;
    Lcd8080DeviceType lcd = (Lcd8080DeviceType)dev;
    x_base level;
    struct DeviceRectInfo* rect_info = (struct DeviceRectInfo*)args;

    NULL_PARAM_CHECK(dev);

    switch (cmd)
    {
        case GRAPHIC_CTRL_RECT_UPDATE: 
            if(!rect_info)
            {
                SYS_ERR("GRAPHIC_CTRL_RECT_UPDATE error args");
                return -ERROR;
            }
            DrvLcdRectUpdate(rect_info->x, rect_info->y, rect_info->width, rect_info->height);
            break;

        case GRAPHIC_CTRL_POWERON:
            /* Todo: power on */
            ret = -ENONESYS;
            break;

        case GRAPHIC_CTRL_POWEROFF:
            /* Todo: power off */
            ret = -ENONESYS;
            break;

        case GRAPHIC_CTRL_GET_INFO:
            *(struct DeviceLcdInfo *)args = lcd->lcd_info;
            break;

        case GRAPHIC_CTRL_SET_MODE:
            ret = -ENONESYS;
            break;
        case GRAPHIC_CTRL_GET_EXT:
            ret = -ENONESYS;
            break;
        default:
            SYS_ERR("drv_lcd_control cmd: %d", cmd);
            break;
    }

    return ret;
}

void ClearHandwriting (void)
{
    //clear  the lcd    
    DrvLcdClear(WHITE);
       
    LcdShowString(10, 10, 100, 24, 24, "RST ", RED, WHITE);
}

#ifdef CONFIG_TOUCH
void HandTest(unsigned short *x_pos, unsigned short *y_pos)
{
    float x1,y1;
    TpReadXy(x_pos,y_pos);     //address
    float  a = 12.1875,b = 16.25;
    x1 = 320 -  (*x_pos)/a +10;
    y1 = (* y_pos)/b;

    if ((*x_pos> 500)&&(*y_pos<500)) {
        ClearHandwriting();
    } else {
        DrvLcdSetPixel(x1,  y1,  RED);
        DrvLcdSetPixel(x1+1,  y1,  RED);
        DrvLcdSetPixel(x1-1,  y1,  RED);

        DrvLcdSetPixel(x1,  y1+1,  RED);
        DrvLcdSetPixel(x1,  y1-1,  RED);
       
       DrvLcdSetPixel(x1+1,  y1+1,  RED);
       DrvLcdSetPixel(x1-1,  y1-1,  RED);

       DrvLcdSetPixel(x1+1,  y1-1,  RED);
       DrvLcdSetPixel(x1-1,  y1+1,  RED);
    }
}
#endif

static uint32 LcdWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    if (write_param  == NONE) {
         return  ERROR;
     }

    LcdStringParam  * show = (LcdStringParam *)write_param->buffer;
    
    if (0==write_param->pos) {         //output string
        LcdShowString(show->x_pos,show->y_pos,show->width,show->height,show->font_size,show->addr,show->font_color,show->back_color);
        return   EOK;   
    } else if (1==write_param->pos) {  //output dot
        DrvLcdSetPixel(show->x_pos, show->y_pos, show->font_color);
        return   EOK;
    } else {
        return ERROR;
    }
}

uint32 DrvLcdClearDone(void * dev, struct BusConfigureInfo *configure_info)
{
    uint16 color =   0;
    color =(uint16)(configure_info->configure_cmd |0x0000ffff );
    DrvLcdClear( color);
    return 0;
}

const struct LcdDevDone lcd_dev_done  = 
{
    .open = NONE,
    .close = NONE,
    .write = LcdWrite,
    .read  = NONE
};

static int BoardLcdBusInit(struct LcdBus * lcd_bus, struct LcdDriver * lcd_driver)
{
    x_err_t ret = EOK;

    /*Init the lcd bus */
    ret = LcdBusInit( lcd_bus, LCD_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_lcd_init LcdBusInit error %d\n", ret);
        return ERROR;
    }

    lcd_driver->configure = DrvLcdClearDone;
    /*Init the lcd driver*/
    ret = LcdDriverInit( lcd_driver, LCD_DRV_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_LCD_init LcdDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the lcd driver to the lcd bus*/
    ret = LcdDriverAttachToBus(LCD_DRV_NAME_1, LCD_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_LCD_init LcdDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the lcd device to the lcd bus*/
static int BoardLcdDevBend(void)
{
    x_err_t ret = EOK;

    static struct LcdHardwareDevice lcd_device;      
    memset(&lcd_device, 0, sizeof(struct LcdHardwareDevice));

    lcd_device.dev_done = &(lcd_dev_done);

    ret = LcdDeviceRegister(&lcd_device, NONE, LCD_1_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("Board_LCD_init LcdDeviceInit device %s error %d\n", LCD_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    ret = LcdDeviceAttachToBus(LCD_1_DEVICE_NAME_0, LCD_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_LCD_init LcdDeviceAttachToBus device %s error %d\n", LCD_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    return  ret;
}

int HwLcdInit(void)
{
    x_err_t ret = EOK;

    static struct LcdDriver lcd_driver;
    memset(&lcd_driver, 0, sizeof(struct LcdDriver));

    Lcd8080DeviceType  lcd_dev = (Lcd8080DeviceType  )malloc(sizeof(  struct Lcd8080Device));
    memset(lcd_dev, 0, sizeof(struct Lcd8080Device));

    if (!lcd_dev) {
        return -1;
    }

    FpioaSetFunction(41,FUNC_GPIOHS9);       //DC    order  /  data
    FpioaSetFunction(47,FUNC_GPIOHS10);   //BL
    FpioaSetFunction(40,FUNC_SPI0_SS0);      //chip  select
    FpioaSetFunction(38,FUNC_SPI0_SCLK);   //clk

    lcd_dev->cs                                               = SPI_CHIP_SELECT_0;
    lcd_dev->dc_pin                                     =  9;  
    lcd_dev->dma_channel                       = DMAC_CHANNEL0;
    lcd_dev->spi_channel                          = SPI_DEVICE_0;
    lcd_dev->lcd_info.bits_per_pixel    = 16;
    lcd_dev->lcd_info.pixel_format       =  PIXEL_FORMAT_BGR565;

    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);

    sysctl_set_spi0_dvp_data(1);                                                                 //open  the lcd  interface  with spi0
    ret = BoardLcdBusInit(&lcd_dev->lcd_bus, &lcd_driver);  //init  lcd bus
    if (EOK != ret) {
        KPrintf("Board_lcd_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardLcdDevBend();                                          //init  lcd device
    if (EOK != ret) {
        KPrintf("BoardLcdDevBend error ret %u\n", ret);
        return ERROR;
    }

    gpiohs_set_drive_mode(10, GPIO_DM_OUTPUT);
    gpiohs_set_pin(10, GPIO_PV_HIGH);                                        
    KPrintf("LCD driver inited ...\r\n");
    DrvLcdInit(lcd_dev);

    return ret;
}

//x1,y1:start
//x2,y2:end  
void LcdDrawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2,uint16 color)
{
	uint16 t; 
	int xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int incx,incy,uRow,uCol; 
	delta_x = x2 - x1;  
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1; 

	if(delta_x>0)
        incx = 1; 
	else if(delta_x==0)
        incx = 0;
	else {
        incx = -1;
        delta_x = -delta_x;
    } 

	if(delta_y>0)
        incy = 1; 
	else if(delta_y==0)
        incy = 0;
	else {
        incy= -1;
        delta_y = -delta_y;
    } 

	if (delta_x>delta_y)
        distance=delta_x; 
	else 
        distance=delta_y; 

	for(t = 0;t <= distance+1;t ++ ) {  
        DrvLcdSetPixel(uRow,uCol,color);
		xerr += delta_x ; 
		yerr += delta_y ; 
		if (xerr>distance) { 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if (yerr>distance) { 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    

void LcdDrawRectangle(uint16 x1, uint16 y1, uint16 x2, uint16 y2,uint16 color)
{
	LcdDrawLine(x1,y1,x2,y1,color);
	LcdDrawLine(x1,y1,x1,y2,color);
	LcdDrawLine(x1,y2,x2,y2,color);
	LcdDrawLine(x2,y1,x2,y2,color);
}

void LcdDrawCircle(uint16 x0,uint16 y0,uint8 r,uint16 color)
{
	int a,b;
	int di;
	a = 0;
    b = r;	  
	di = 3-(r<<1);            
	while(a <= b) {
		DrvLcdSetPixel(x0+a,y0-b,color);             //5
 		DrvLcdSetPixel(x0+b,y0-a,color);             //0           
		DrvLcdSetPixel(x0+b,y0+a,color);             //4               
		DrvLcdSetPixel(x0+a,y0+b,color);             //6 
		DrvLcdSetPixel(x0-a,y0+b,color);             //1       
 		DrvLcdSetPixel(x0-b,y0+a,color);             
		DrvLcdSetPixel(x0-a,y0-b,color);             //2             
  		DrvLcdSetPixel(x0-b,y0-a,color);             //7     	         
		a++;
		//Bresenham     
		if(di<0)
            di += 4*a+6;	  
		else {
			di += 10+4*(a-b);   
			b--;
		} 						    
	}
}								  
