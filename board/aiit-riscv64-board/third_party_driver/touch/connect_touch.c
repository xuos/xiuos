/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file connect_touch.c
* @brief support aiit-riscv64-board touch function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <connect_touch.h>

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

void TouchWriteByte(unsigned char num)    
{  
	unsigned char count=0;   
	for (count=0;count<8;count++) { 	  
		if (num&0x80) {
            gpio_set_pin(TP_MOSI, GPIO_PV_HIGH);
        } else {
            gpio_set_pin(TP_MOSI, GPIO_PV_LOW);
        }   
		num<<=1;    
		gpio_set_pin(TP_CLK, GPIO_PV_LOW);
		usleep(1);
		gpio_set_pin(TP_CLK, GPIO_PV_HIGH);  
	}		 			    
} 	

unsigned short TP_Read_AD(unsigned char CMD)	  
{ 	 
	unsigned char count=0; 	  
	unsigned short Num=0; 
    
    gpio_set_pin(TP_CLK, GPIO_PV_LOW);
    gpio_set_pin(TP_MOSI, GPIO_PV_LOW);
    gpio_set_pin(TP_CS, GPIO_PV_LOW);      
	
	TouchWriteByte(CMD);
	usleep(6);
     gpio_set_pin(TP_CLK, GPIO_PV_LOW);	   	    
	usleep(1);  	   
	gpio_set_pin(TP_CLK, GPIO_PV_HIGH); 	
	usleep(1);   
	  gpio_set_pin(TP_CLK, GPIO_PV_LOW);	      	    
	for (count = 0;count < 16;count++) { 				  
		Num<<=1; 	 
		gpio_set_pin(TP_CLK, GPIO_PV_LOW);		   
		usleep(1); 
 	    gpio_set_pin(TP_CLK, GPIO_PV_HIGH);	    	      
 		if(gpio_get_pin(TP_MISO))
            Num++; 		 
	}  	
	Num>>=4;   	
	gpio_set_pin(TP_CS, GPIO_PV_HIGH);     		 
	return(Num);   
}

#define READ_TIMES           5 	
#define LOST_VAL                 1	  
unsigned short TP_Read_XOY(unsigned  char     xy)
{
	unsigned short i, j;
	unsigned short buf[READ_TIMES];
	unsigned short sum=0;
	unsigned short temp;
	for (i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for (i=0;i<READ_TIMES-1; i++) {
		for (j=i+1;j<READ_TIMES;j++) {
			if (buf[i]>buf[j]) {
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

unsigned char TP_Read_XY(unsigned short *x, unsigned short *y)
{
	unsigned  short    xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   

	*x=xtemp;
	*y=ytemp;
	return 1;
}

uint32 TouchRead(void *dev, struct BusBlockReadParam *read_param)
{
    uint32   ret  = EOK;
    NULL_PARAM_CHECK(read_param);
    struct TouchDataStandard   * data   =  ( struct TouchDataStandard*)read_param->buffer;
	TP_Read_XY(&data->x,&data->y);
	return ret;
}

/*
*  read  the twice  
*  return 0  is error   1  :ok
*/
#define band_error 66 
unsigned char TP_Read_XY_TWICE(unsigned short *x, unsigned short *y) 
{
	unsigned short fir_x, fir_y, sec_x, sec_y;
 	unsigned char flag;    
    flag = TP_Read_XY(&fir_x, &fir_y);   
    if (flag == 0)
        return(0);
    flag = TP_Read_XY(&sec_x, &sec_y);	   
    if (flag == 0)
        return(0);   
    if (((sec_x <= fir_x && fir_x < sec_x + band_error) || (fir_x <= sec_x && sec_x < fir_x + band_error))
    && ((sec_y <= fir_y && fir_y < sec_y + band_error) || (fir_y <= sec_y && sec_y < fir_y + band_error)))  {
        *x = (fir_x + sec_x) / 2;
        *y = (fir_y + sec_y) / 2;
        return 1;
    } else 
        return 0;	  
}  
				  
uint32 TouchConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    FpioaSetFunction(42, FUNC_GPIO0);
    FpioaSetFunction(43, FUNC_GPIO1);
    FpioaSetFunction(44, FUNC_GPIO2);
    FpioaSetFunction(45, FUNC_GPIO3);
    FpioaSetFunction(46, FUNC_GPIO4);

    gpio_init();
    gpio_set_drive_mode(TP_CLK, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(TP_CS , GPIO_DM_OUTPUT);
    gpio_set_drive_mode(TP_MISO, GPIO_DM_INPUT);
    gpio_set_drive_mode(TP_PEN, GPIO_DM_INPUT);
    gpio_set_drive_mode(TP_MOSI, GPIO_DM_OUTPUT);

    return 0;
}

struct TouchDevDone touch_dev_done =
{
    .open  = NONE,
    .close  = NONE,
    .write  =  NONE,
    .read  =  TouchRead  
};
  
void TP_Init(void)
{
    TouchConfigure(NONE,NONE);
    while (1)
	{
	 	TP_Read_XY(&tp_dev.x, &tp_dev.y);
		KPrintf("tp_dev.x = %8d    ***  tp_dev.y= %8d \r\n", tp_dev.x, tp_dev.y);
		MdelayKTask(100);
	}			 
}

unsigned short my_abs(unsigned short x1, unsigned short x2)
{			 
	if(x1>x2)
        return x1-x2;
	else 
        return x2-x1;
}  

struct RiscTouch
{
    char *BusName;
    struct TouchBus touchbus;
};

struct RiscTouch touch;

static int BoardTouchBusInit(struct RiscTouch *risc_touch_bus, struct TouchDriver *touch_driver)
{
    x_err_t ret = EOK;

    /*Init the touch bus */
    ret = TouchBusInit(&risc_touch_bus->touchbus, risc_touch_bus->BusName);
    if (EOK != ret) {
        KPrintf("Board_touch_init touchBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the touch driver*/
    ret = TouchDriverInit(touch_driver, TOUCH_DRV_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_touch_init touchDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the touch driver to the touch bus*/
    ret = TouchDriverAttachToBus(TOUCH_DRV_NAME_1, risc_touch_bus->BusName);
    if (EOK != ret) {
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
    if (EOK != ret) {
        KPrintf("device %s error %d\n", TOUCH_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    ret = TouchDeviceAttachToBus(TOUCH_1_DEVICE_NAME_0, TOUCH_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("device %s error %d\n", TOUCH_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    return ret;
}

int HwTouchBusInit(void)
{
    x_err_t ret = EOK;
    struct RiscTouch *risc_touch;  

    static struct TouchDriver touch_driver;
    memset(&touch_driver, 0, sizeof(struct TouchDriver));

    touch_driver.configure = TouchConfigure;

    risc_touch = &touch;
    risc_touch->BusName = TOUCH_BUS_NAME_1;
    risc_touch->touchbus.private_data = &touch;

    TouchConfigure(NONE,NONE);

    ret = BoardTouchBusInit(risc_touch, &touch_driver);
    if (EOK != ret) {
      return ERROR;
    }

    ret =  BoardTouchDevBend();
    if (EOK != ret) {
        KPrintf("board_touch_Init error ret %u\n", ret);
        return ERROR;
    } 
    return EOK;
}
