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
* @file bus_serial.c
* @brief register serial bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_serial.h>
#include <dev_serial.h>

int SerialBusInit(struct SerialBus *serial_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(serial_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != serial_bus->bus.bus_state) {
        strncpy(serial_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        serial_bus->bus.bus_type = TYPE_SERIAL_BUS;
        serial_bus->bus.bus_state = BUS_INSTALL;
        serial_bus->bus.private_data = serial_bus->private_data;

        ret = BusRegister(&serial_bus->bus);
        if (EOK != ret) {
            KPrintf("serial BusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SerialBusInit BusRegister bus has been register state%u\n", serial_bus->bus.bus_state);        
    }

    return ret;
}

int SerialDriverInit(struct SerialDriver *serial_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(serial_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != serial_driver->driver.driver_state) {
        serial_driver->driver.driver_type = TYPE_SERIAL_DRV;
        serial_driver->driver.driver_state = DRV_INSTALL;

        strncpy(serial_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        serial_driver->driver.configure = serial_driver->configure;

        ret = SerialDriverRegister(&serial_driver->driver);
        if (EOK != ret) {
            KPrintf("SerialDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SerialDriverInit DriverRegister driver has been register state%u\n", serial_driver->driver.driver_state);
    }

    return ret;
}

int SerialReleaseBus(struct SerialBus *serial_bus)
{
    NULL_PARAM_CHECK(serial_bus);

    return BusRelease(&serial_bus->bus);
}

int SerialDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SerialDriverAttachToBus find serial bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_SERIAL_BUS == bus->bus_type) {
        driver = SerialDriverFind(drv_name, TYPE_SERIAL_DRV);
        if (NONE == driver) {
            KPrintf("SerialDriverAttachToBus find serial driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_SERIAL_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("SerialDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
