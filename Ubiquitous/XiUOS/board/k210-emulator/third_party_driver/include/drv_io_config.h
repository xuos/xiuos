/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-19     ZYH          first version
 */

/**
* @file drv_io_config.h
* @brief define kd233-board io configure
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: drv_io_config.h
Description: define kd233-board io configure
Others: take RT-Thread v4.0.2/bsp/k210/driver/drv_io_config.h for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: add kd233-board io configure define
*************************************************/

#ifndef __DRV_IO_CONFIG_H__
#define __DRV_IO_CONFIG_H__

enum HS_GPIO_CONFIG
{
#ifdef BSP_USING_LCD
    LCD_DC_PIN = 0,     /* LCD DC PIN */
#endif
#ifdef BSP_SPI1_USING_SS0
    SPI1_CS0_PIN,
#endif
#ifdef BSP_SPI1_USING_SS1
    SPI1_CS1_PIN,
#endif
#ifdef BSP_SPI1_USING_SS2
    SPI1_CS2_PIN,
#endif
#ifdef BSP_SPI1_USING_SS3
    SPI1_CS3_PIN,
#endif
    GPIO_ALLOC_START /* index of gpio driver start */
};

extern int IoConfigInit(void);

#endif
