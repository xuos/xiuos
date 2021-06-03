/*
 * Copyright (c) Guangzhou Xingyi Electronic  Technology Co., Ltd
 *
 * Change Logs:
 * Date               Author       Notes
 * 2014-7-4      alientek   first version
 */

/**
* @file connect_touch.c
* @brief support aiit-arm32-board touch function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_touch.c
Description: support aiit-arm32-board touch configure and touch bus register function
Others: take hardware/touch/touch.c for references
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board touch configure, write and read
2. support aiit-arm32-board touch bus device and driver register
*************************************************/

#include <board.h>
#include <connect_touch.h> 
#include <hardware_gpio.h>
#include <hardware_rcc.h>
#include <misc.h>

touch_device_info tp_dev =
{
    0,
    0, 
    0,
    0,
    0,
    0,	  	 		
    0,
    0,	  	 		
};	

unsigned  char  CMD_RDX=0XD0;
unsigned  char CMD_RDY=0X90;

TP_modify_save  modify_save =
{
    0,0,0,0,0,0
};

 static int Stm32Udelay(uint32 us)
{
    uint32 ticks;
    uint32 told, tnow, tcnt = 0;
    uint32 reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / TICK_PER_SECOND);
    told = SysTick->VAL;
    while (1) {
        tnow = SysTick->VAL;
        if (tnow != told) {
            if (tnow < told) {
                tcnt += told - tnow;
            } else {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) {
                return 0;
                break;
            }
        }
    }
}	 			    					   

void TouchWriteByte(unsigned char num)    
{  
	u8 count=0;   
	for(count = 0;count < 8;count ++) { 	  
		if (num & 0x80)
            T_MOSI=1;  
		else 
            T_MOSI=0;   
		num <<= 1;    
		T_CLK = 0; 
		Stm32Udelay(1);
		T_CLK = 1;		 
	}		 			    
} 		 

u16 TpReadAd(u8 cmd)	  
{ 	 
	u8 count = 0; 	  
	u16 Num = 0; 

	T_CLK =0;		 
	T_MOSI = 0; 	
	TCS = 0; 	

	TouchWriteByte(cmd);
	Stm32Udelay(6);
	T_CLK = 0; 	     	   

	Stm32Udelay(1);  	   
	T_CLK = 1;	

	Stm32Udelay(1);   
	T_CLK = 0; 	     	    

	for(count = 0;count < 16;count ++) { 				  
		Num <<= 1; 	 
		T_CLK = 0;	
		Stm32Udelay(1); 
 		T_CLK =1;
 		if (T_MISO == 1)
            Num++; 		 
	}  	
	Num >>= 4;   	
	TCS = 1;	

	return(Num);   
}

#define READ_TIMES 5 	
#define LOST_VAL 1	  	

u16 TpReadXoy(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i = 0;i < READ_TIMES;i ++)
        buf[i]=TpReadAd(xy);		 		    
	for(i = 0;i < READ_TIMES-1; i ++) {
		for(j = i+1;j < READ_TIMES;j ++) {
			if (buf[i] > buf[j]) {
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i = LOST_VAL;i < READ_TIMES - LOST_VAL;i ++)
        sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

u8 TpReadXy(u16 *x, u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TpReadXoy(CMD_RDX);
	ytemp=TpReadXoy(CMD_RDY);	  												   

	*x=xtemp;
	*y=ytemp;
	return 1;
}

static uint32 TouchRead(void *dev, struct BusBlockReadParam *read_param)
{
    uint32   ret  = EOK;
    NULL_PARAM_CHECK(read_param);
    struct TouchDataStandard   * data   =  ( struct TouchDataStandard*)read_param->buffer;
    TpReadXy(&data->x,&data->y);
    return ret;
}
			
static uint32 TouchConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    GPIO_InitTypeDef  gpio_initstructure;	

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOD, ENABLE);

    gpio_initstructure.GPIO_Pin =GPIO_Pin_4;                      
    gpio_initstructure.GPIO_Mode = GPIO_Mode_IN;         
    gpio_initstructure.GPIO_OType = GPIO_OType_PP;     
    gpio_initstructure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_initstructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio_initstructure);

    gpio_initstructure.GPIO_Pin = GPIO_Pin_3;
    gpio_initstructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_initstructure);

    gpio_initstructure.GPIO_Pin = GPIO_Pin_13;
    gpio_initstructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOG, &gpio_initstructure);	

    gpio_initstructure.GPIO_Pin = GPIO_Pin_5;
    gpio_initstructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_initstructure);

    gpio_initstructure.GPIO_Pin = GPIO_Pin_6;                       
    gpio_initstructure.GPIO_Mode = GPIO_Mode_OUT;     
    GPIO_Init(GPIOD, &gpio_initstructure);                            

    return 0;
}

struct TouchDevDone touch_dev_done  =
{
    .open  = NONE,
    .close  = NONE,
    .write  =  NONE,
    .read  =  TouchRead
};

struct Stm32Touch
{
    char *bus_name;
    struct TouchBus touch_bus;
};

struct Stm32Touch touch;

static int BoardTouchBusInit(struct Stm32Touch *stm32touch_bus, struct TouchDriver *touch_driver)
{
    x_err_t ret = EOK;

    /*Init the touch bus */
    ret = TouchBusInit(&stm32touch_bus->touch_bus, stm32touch_bus->bus_name);
    if (EOK != ret) {
        KPrintf("Board_touch_init touchBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the touch driver*/
    ret = TouchDriverInit(touch_driver, TOUCH_DRV_NAME_1);
    if (EOK != ret){
        KPrintf("Board_touch_init touchDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the touch driver to the touch bus*/
    ret = TouchDriverAttachToBus(TOUCH_DRV_NAME_1, stm32touch_bus->bus_name);
    if (EOK != ret){
        KPrintf("Board_touch_init TouchDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the touch device to the touch bus*/
static int BoardTouchDevBend(void)
{
    x_err_t ret = EOK;
    static struct TouchHardwareDevice touch_device;
    memset(&touch_device, 0, sizeof(struct TouchHardwareDevice));

    touch_device.dev_done = &touch_dev_done;

    ret = TouchDeviceRegister(&touch_device, NONE, TOUCH_1_DEVICE_NAME_0);
    if (EOK != ret){
        KPrintf("TouchDeviceRegister  device %s error %d\n", TOUCH_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    ret = TouchDeviceAttachToBus(TOUCH_1_DEVICE_NAME_0, TOUCH_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("TouchDeviceAttachToBus  device %s error %d\n", TOUCH_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    return ret;
}

int Stm32HwTouchBusInit(void)
{
    x_err_t ret = EOK;
    struct Stm32Touch *Stmtouch;  

    static struct TouchDriver    touch_driver;
    memset(&touch_driver, 0, sizeof(struct TouchDriver));

    touch_driver.configure = TouchConfigure;

    Stmtouch = &touch;
    Stmtouch->bus_name = TOUCH_BUS_NAME_1;
    Stmtouch->touch_bus.private_data = &touch;

    ret = BoardTouchBusInit(Stmtouch, &touch_driver);
    if (EOK != ret) {
      return ERROR;
    }

    ret = BoardTouchDevBend();
    if (EOK != ret) {
        KPrintf("board_touch_Init error ret %u\n", ret);
        return ERROR;
    } 
    return EOK;
}
