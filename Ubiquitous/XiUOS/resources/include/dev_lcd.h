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
* @file dev_lcd.h
* @brief define lcd dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_LCD_H
#define DEV_LCD_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

/*set the pen color*/
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000  
#define BLUE         	   0x001F  
#define BRED               0xF81F
#define GRED 			   0xFFE0
#define GBLUE			 0x07FF
#define RED           	     0xF800
#define MAGENTA      0xF81F
#define GREEN            0x07E0
#define CYAN               0x7FFF
#define YELLOW         0xFFE0
#define BROWN 		   0xBC40    
#define BRRED 	   		 0xFC07      
#define GRAY               0x8430      

#define DARKBLUE      	 0x01CF//Navy blue 
#define LIGHTBLUE      	 0x7D7C//Light blue 
#define GRAYBLUE          0x5458//Gray blue 
#define LIGHTGREEN     0x841F 
#define LGRAY 			        0xC618
#define LGRAYBLUE        0xA651
#define LBBLUE                0x2B12

typedef struct 
{
    uint16 x_pos;
    uint16 y_pos;
    uint16 width;
    uint16 height;
    uint8 font_size;
    uint8 *addr;
    uint16 font_color;
    uint16 back_color;
}LcdStringParam;

struct  LcdDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct  LcdHardwareDevice
{
    struct HardwareDev haldev;

    const struct  LcdDevDone *dev_done;
    
    void *private_data;
};

/*Register the lcd device*/
int LcdDeviceRegister(struct  LcdHardwareDevice *lcd_device, void *lcd_param, const char *device_name);

/*Register the lcd device to the lcd bus*/
int LcdDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register lcd device*/
HardwareDevType LcdDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif


#endif