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
* @file bus_can.h
* @brief define can bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_CAN_H
#define BUS_CAN_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CanDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);

    void *private_data;
};

struct CanBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the CAN bus*/
int CanBusInit(struct CanBus *can_bus, const char *bus_name);

/*Register the CAN driver*/
int CanDriverInit(struct CanDriver *can_driver, const char *driver_name);

/*Release the CAN device*/
int CanReleaseBus(struct CanBus *can_bus);

/*Register the CAN driver to the CAN bus*/
int CanDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int CanDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType CanDriverFind(const char *drv_name, enum DriverType drv_type);

#ifdef __cplusplus
}
#endif

#endif
