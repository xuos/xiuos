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
* @file bus_can.c
* @brief register can bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_can.h>
#include <dev_can.h>

/*Register the CAN BUS*/
int CanBusInit(struct CanBus *can_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(can_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != can_bus->bus.bus_state) {
        strncpy(can_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        can_bus->bus.bus_type = TYPE_CAN_BUS;
        can_bus->bus.bus_state = BUS_INSTALL;
        can_bus->bus.private_data = can_bus->private_data;

        ret = BusRegister(&can_bus->bus);
        if (EOK != ret) {
            KPrintf("CanBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("CanBusInit BusRegister bus has been register state%u\n", can_bus->bus.bus_state);        
    }

    return ret;
}

/*Register the CAN Driver*/
int CanDriverInit(struct CanDriver *can_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(can_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != can_driver->driver.driver_state) {
        can_driver->driver.driver_type = TYPE_CAN_DRV;
        can_driver->driver.driver_state = DRV_INSTALL;

        strncpy(can_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        can_driver->driver.configure = can_driver->configure;
        can_driver->driver.private_data = can_driver->private_data;

        ret = CanDriverRegister(&can_driver->driver);
        if (EOK != ret) {
            KPrintf("CanDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("CanDriverInit DriverRegister driver has been register state%u\n", can_driver->driver.driver_state);
    }

    return ret;
}

/*Release the CAN device*/
int CanReleaseBus(struct CanBus *can_bus)
{
    NULL_PARAM_CHECK(can_bus);

    return BusRelease(&can_bus->bus);
}

/*Register the CAN Driver to the CAN BUS*/
int CanDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);    

    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("CanDriverAttachToBus find can bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_CAN_BUS == bus->bus_type) {
        driver = CanDriverFind(drv_name, TYPE_CAN_DRV);
        if (NONE == driver) {
            KPrintf("CanDriverAttachToBus find can driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_CAN_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("CanDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
