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
* @file bus_touch.c
* @brief register touch bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_touch.h>
#include <dev_touch.h>

int TouchBusInit(struct TouchBus *touch_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(touch_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != touch_bus->bus.bus_state) {
        strncpy(touch_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        touch_bus->bus.bus_type = TYPE_TOUCH_BUS;
        touch_bus->bus.bus_state = BUS_INSTALL;
        touch_bus->bus.private_data = touch_bus->private_data;

        ret = BusRegister(&touch_bus->bus);
        if (EOK != ret) {
            KPrintf("touchBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("touchBusInit BusRegister bus has been register state%u\n", touch_bus->bus.bus_state);        
    }

    return ret;
}

int TouchDriverInit(struct TouchDriver *touch_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(touch_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != touch_driver->driver.driver_state) {
        touch_driver->driver.driver_type = TYPE_TOUCH_DRV;
        touch_driver->driver.driver_state = DRV_INSTALL;

        strncpy(touch_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        touch_driver->driver.configure =touch_driver->configure;

        ret = TouchDriverRegister(&touch_driver->driver);
        if (EOK != ret) {
            KPrintf("TouchDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("TouchDriverInit DriverRegister driver has been register state%u\n", touch_driver->driver.driver_state);
    }

    return ret;
}

int TouchReleaseBus(struct TouchBus *touch_bus)
{
    NULL_PARAM_CHECK(touch_bus);

    return BusRelease(&touch_bus->bus);
}

int TouchDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("TouchDriverAttachToBus find touch bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_TOUCH_BUS == bus->bus_type) {
        driver = TouchDriverFind(drv_name, TYPE_TOUCH_DRV);
        if (NONE == driver) {
            KPrintf("TouchDriverAttachToBus find touch driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_TOUCH_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("TouchDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
