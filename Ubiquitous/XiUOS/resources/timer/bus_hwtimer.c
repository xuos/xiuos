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
* @file bus_hwtimer.c
* @brief register hwtimer bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_hwtimer.h>
#include <dev_hwtimer.h>

int HwtimerBusInit(struct HwtimerBus *hwtimer_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(hwtimer_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != hwtimer_bus->bus.bus_state) {
        strncpy(hwtimer_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        hwtimer_bus->bus.bus_type = TYPE_HWTIMER_BUS;
        hwtimer_bus->bus.bus_state = BUS_INSTALL;
        hwtimer_bus->bus.private_data = hwtimer_bus->private_data;

        ret = BusRegister(&hwtimer_bus->bus);
        if (EOK != ret) {
            KPrintf("HwtimerBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("HwtimerBusInit BusRegister bus has been register state%u\n", hwtimer_bus->bus.bus_state);        
    }

    return ret;
}

int HwtimerDriverInit(struct HwtimerDriver *hwtimer_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(hwtimer_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != hwtimer_driver->driver.driver_state) {
        hwtimer_driver->driver.driver_type = TYPE_HWTIMER_DRV;
        hwtimer_driver->driver.driver_state = DRV_INSTALL;

        strncpy(hwtimer_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        hwtimer_driver->driver.configure = hwtimer_driver->configure;

        ret = HwtimerDriverRegister(&hwtimer_driver->driver);
        if (EOK != ret) {
            KPrintf("HwtimerDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("HwtimerDriverInit DriverRegister driver has been register state%u\n", hwtimer_driver->driver.driver_state);
    }

    return ret;
}

int HwtimerReleaseBus(struct HwtimerBus *hwtimer_bus)
{
    NULL_PARAM_CHECK(hwtimer_bus);

    return BusRelease(&hwtimer_bus->bus);
}

int HwtimerDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("HwtimerDriverAttachToBus find hwtimer bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_HWTIMER_BUS == bus->bus_type) {
        driver = HwtimerDriverFind(drv_name);
        if (NONE == driver) {
            KPrintf("HwtimerDriverAttachToBus find hwtimer driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_HWTIMER_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("HwtimerDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
