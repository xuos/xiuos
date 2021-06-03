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
* @file bus_wdt.c
* @brief register wdt bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_wdt.h>
#include <dev_wdt.h>

int WdtBusInit(struct WdtBus *wdt_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(wdt_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != wdt_bus->bus.bus_state) {
        strncpy(wdt_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        wdt_bus->bus.bus_type = TYPE_WDT_BUS;
        wdt_bus->bus.bus_state = BUS_INSTALL;
        wdt_bus->bus.private_data = wdt_bus->private_data;

        ret = BusRegister(&wdt_bus->bus);
        if (EOK != ret) {
            KPrintf("WdtBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("WdtBusInit BusRegister bus has been register state%u\n", wdt_bus->bus.bus_state);        
    }

    return ret;
}

int WdtDriverInit(struct WdtDriver *wdt_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(wdt_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != wdt_driver->driver.driver_state) {
        wdt_driver->driver.driver_type = TYPE_WDT_DRV;
        wdt_driver->driver.driver_state = DRV_INSTALL;

        strncpy(wdt_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        wdt_driver->driver.configure = wdt_driver->configure;

        ret = WdtDriverRegister(&wdt_driver->driver);
        if (EOK != ret) {
            KPrintf("WdtDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("WdtDriverInit DriverRegister driver has been register state%u\n", wdt_driver->driver.driver_state);
    }

    return ret;
}

int WdtReleaseBus(struct WdtBus *wdt_bus)
{
    NULL_PARAM_CHECK(wdt_bus);

    return BusRelease(&wdt_bus->bus);
}

int WdtDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("WdtDriverAttachToBus find watchdog bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_WDT_BUS == bus->bus_type) {
        driver = WdtDriverFind(drv_name);
        if (NONE == driver) {
            KPrintf("WdtDriverAttachToBus find watchdog driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_WDT_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("WdtDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
