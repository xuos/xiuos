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
* @file bus_pin.h
* @brief define pin bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_PIN_H
#define BUS_PIN_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PinDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *ConfigureInfo);
};

struct PinBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the Pin bus*/
int PinBusInit(struct PinBus *pin_bus, const char *bus_name);

/*Register the Pin driver*/
int PinDriverInit(struct PinDriver *pin_driver, const char *driver_name, void *data);

/*Release the Pin device*/
int PinReleaseBus(struct PinBus *pin_bus);

/*Register the Pin driver to the Pin bus*/
int PinDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int PinDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType PinDriverFind(const char *drv_name, enum DriverType drv_type);

/*Get the initialized Pin bus*/
BusType PinBusInitGet(void);

#ifdef __cplusplus
}
#endif

#endif
