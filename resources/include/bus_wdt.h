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
* @file bus_wdt.h
* @brief define wdt bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_WDT_H
#define BUS_WDT_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct WdtDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *ConfigureInfo);
    void *private_data;
};

struct WdtBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the wdt bus*/
int WdtBusInit(struct WdtBus *wdt_bus, const char *bus_name);

/*Register the wdt driver*/
int WdtDriverInit(struct WdtDriver *wdt_driver, const char *driver_name);

/*Release the wdt device*/
int WdtReleaseBus(struct WdtBus *wdt_bus);

/*Register the wdt driver to the wdt bus*/
int WdtDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int WdtDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType WdtDriverFind(const char *drv_name);

#ifdef __cplusplus
}
#endif

#endif
