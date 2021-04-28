/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file graphic.h
* @brief define aiit-riscv64-board lcd operation
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: graphic.h
Description: define aiit-riscv64-board lcd operation
Others: take RT-Thread v4.0.2/include/rtdef.h for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: add aiit-riscv64-board lcd configure and operation function
*************************************************/

#ifndef GRAPHIC_H
#define GRAPIHC_H

#include <xiuos.h>

#define GRAPHIC_CTRL_RECT_UPDATE                      0
#define GRAPHIC_CTRL_POWERON                              1
#define GRAPHIC_CTRL_POWEROFF                            2
#define GRAPHIC_CTRL_GET_INFO                               3
#define GRAPHIC_CTRL_SET_MODE                             4
#define GRAPHIC_CTRL_GET_EXT                                  5

enum
{
    PIXEL_FORMAT_MONO = 0,
    PIXEL_FORMAT_GRAY4,
    PIXEL_FORMAT_GRAY16,
    PIXEL_FORMAT_RGB332,
    PIXEL_FORMAT_RGB444,
    PIXEL_FORMAT_RGB565 ,
    PIXEL_FORMAT_RGB565P,
    PIXEL_FORMAT_BGR565 = PIXEL_FORMAT_RGB565P,
    PIXEL_FORMAT_RGB666,
    PIXEL_FORMAT_RGB888,
    PIXEL_FORMAT_ARGB888,
    PIXEL_FORMAT_ABGR888,
    PIXEL_FORMAT_ARGB565,
    PIXEL_FORMAT_ALPHA,
    PIXEL_FORMAT_COLOR,
};

struct DeviceLcdInfo
{
    uint8 pixel_format;                           
    uint8 bits_per_pixel;                       
    uint16 reserved;                               

    uint16 width;                                 
    uint16 height;                                

    uint8 *framebuffer;                            
};

struct DeviceRectInfo
{
    uint16 x;                                    
    uint16 y;                                  
    uint16 width;                              
    uint16 height;                             
};

struct DeviceGraphicDone
{
    void (*set_pixel) (const char *pixel, int x, int y);
    void (*get_pixel) (char *pixel, int x, int y);

    void (*draw_hline) (const char *pixel, int x1, int x2, int y);
    void (*draw_vline) (const char *pixel, int x, int y1, int y2);

    void (*blit_line) (const char *pixel, int x, int y, x_size_t size);
};

#define GraphixDone(device) ((struct DeviceGraphicDone *)(device->UserData))

#endif
