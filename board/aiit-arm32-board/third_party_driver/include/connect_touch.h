/*
 * Copyright (c) Guangzhou Xingyi Electronic  Technology Co., Ltd
 *
 * Change Logs:
 * Date                     Author              Notes
 * 2014-5-7        alientek team   first version
 */

/**
* @file connect_touch.c
* @brief support aiit-arm32-board touch function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/
#ifndef CONNECT_TOUCH_H
#define CONNECT_TOUCH_H

#include <device.h>
#include <stm32f4xx.h>

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  (*((volatile unsigned long  *)(addr))) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14 
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014 
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414    
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814   
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14    
#define GPIOI_ODR_Addr      (GPIOI_BASE+20) //0x40022014     

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10 
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010 
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410 
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810 
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10 
#define GPIOI_IDR_Addr      (GPIOI_BASE+16) //0x40022010 
 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //output 
#define PAin(n)      BIT_ADDR(GPIOA_IDR_Addr,n)  //input 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //output 
#define PBin(n)      BIT_ADDR(GPIOB_IDR_Addr,n)  //input 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //output 
#define PCin(n)      BIT_ADDR(GPIOC_IDR_Addr,n)  //input 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //output 
#define PDin(n)      BIT_ADDR(GPIOD_IDR_Addr,n)  //input 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //output 
#define PEin(n)      BIT_ADDR(GPIOE_IDR_Addr,n)  //input

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //output 
#define PFin(n)      BIT_ADDR(GPIOF_IDR_Addr,n)  //input

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //output 
#define PGin(n)      BIT_ADDR(GPIOG_IDR_Addr,n)  //input

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //output 
#define PHin(n)      BIT_ADDR(GPIOH_IDR_Addr,n)  //input

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //output 
#define PIin(n)      BIT_ADDR(GPIOI_IDR_Addr,n)  //input

#define TP_PRES_DOWN 0x80  
#define TP_CATH_PRES   0x40  

//touch screen control struct
typedef struct
{
	u16 x; 	
	u16 y;					
	u8  sta;								
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
	u8 touchtype;
}touch_device_info;

extern touch_device_info tp_dev;	 	

//save data struct
typedef struct 
{
	s32  ty_xfac;
	s32  ty_yfac;
	short  x_pos;
	short  y_pos;
	u8  iic_touchtype;        
	u8  iic_flag;              
}TP_modify_save;

//io pin define
#define PEN  	    	          PDin(6)  	
#define  T_MISO	              PBin(4)   	
#define T_MOSI 	        	  PBout(5)  
#define T_CLK 	         	    PBout(3)  	
#define TCS  	 	               PGout(13)  	
   
int Stm32HwTouchBusInit(void);

#endif
