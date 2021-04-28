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
* @file connect_touch.h
* @brief define aiit-riscv64-board touch function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef CONNECT_TOUCH_H
#define CONNECT_TOUCH_H

#include <device.h>
#include "fpioa.h"
#include "gpio.h" 
#include <sleep.h>

#define TP_CLK             0
#define TP_CS                1
#define TP_MISO           2
#define TP_PEN             3
#define TP_MOSI           4

#define TP_PRES_DOWN 0x80  
#define TP_CATH_PRES 0x40  


//touch screen control struct
typedef struct
{
	unsigned short x; 	
	unsigned short y;					
	unsigned char sta;								
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
	unsigned char touchtype;
}touch_device_info;

extern touch_device_info tp_dev;	 	

//save data struct
typedef struct {
    int ty_xfac;
	int ty_yfac;
    short x_pos;
    short y_pos;
    unsigned char iic_touchtype;        
	unsigned char iic_flag;              
}TP_modify_save;

int HwTouchBusInit(void);
 
#endif
