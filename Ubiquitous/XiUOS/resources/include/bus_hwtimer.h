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
* @file bus_hwtimer.h
* @brief define hwtimer bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_HWTIMER_H
#define BUS_HWTIMER_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HwtimerDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);
};

struct HwtimerBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the hwtimer bus*/
int HwtimerBusInit(struct HwtimerBus *hwtimer_bus, const char *bus_name);

/*Register the hwtimer driver*/
int HwtimerDriverInit(struct HwtimerDriver *hwtimer_driver, const char *driver_name);

/*Release the hwtimer device*/
int HwtimerReleaseBus(struct HwtimerBus *hwtimer_bus);

/*Register the hwtimer driver to the hwtimer bus*/
int HwtimerDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int HwtimerDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType HwtimerDriverFind(const char *drv_name);

#ifdef __cplusplus
}
#endif

#endif
