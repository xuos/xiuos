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
* @file bus_lcd.h
* @brief define lcd bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_LCD_H
#define BUS_LCD_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LcdDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);
};

struct LcdBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the lcd bus*/
int LcdBusInit(struct LcdBus *lcd_bus , const char *bus_name);

/*Register the lcd driver*/
int LcdDriverInit(struct LcdDriver *lcd_driver, const char *driver_name);

/*Release the lcd device*/
int LcdReleaseBus(struct LcdBus *lcd_bus );

/*Register the lcd driver to the lcd bus*/
int LcdDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int LcdDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType LcdDriverFind(const char *drv_name, enum DriverType drv_type);

#ifdef __cplusplus
}
#endif

#endif
