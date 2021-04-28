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
* @file bus_touch.h
* @brief define touch bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_TOUCH_H
#define BUS_TOUCH_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TouchDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);
};

struct TouchBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the touch bus*/
int TouchBusInit(struct TouchBus *touch_bus, const char *bus_name);

/*Register the touch driver*/
int TouchDriverInit(struct TouchDriver *touch_driver, const char *driver_name);

/*Release the touch device*/
int TouchReleaseBus(struct TouchBus *touch_bus);

/*Register the touch driver to the touch bus*/
int TouchDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int TouchDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType TouchDriverFind(const char *drv_name, enum DriverType drv_type);

#ifdef __cplusplus
}
#endif

#endif
