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
* @file bus_rtc.c
* @brief register rtc bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_rtc.h>
#include <dev_rtc.h>

int RtcBusInit(struct RtcBus *rtc_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(rtc_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != rtc_bus->bus.bus_state) {
        strncpy(rtc_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        rtc_bus->bus.bus_type = TYPE_RTC_BUS;
        rtc_bus->bus.bus_state = BUS_INSTALL;
        rtc_bus->bus.private_data = rtc_bus->private_data;

        ret = BusRegister(&rtc_bus->bus);
        if (EOK != ret) {
            KPrintf("RtcBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("RtcBusInit BusRegister bus has been register state%u\n", rtc_bus->bus.bus_state);        
    }

    return ret;
}

int RtcDriverInit(struct RtcDriver *rtc_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(rtc_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != rtc_driver->driver.driver_state) {
        rtc_driver->driver.driver_type = TYPE_RTC_DRV;
        rtc_driver->driver.driver_state = DRV_INSTALL;

        strncpy(rtc_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        rtc_driver->driver.configure = rtc_driver->configure;

        ret = RtcDriverRegister(&rtc_driver->driver);
        if (EOK != ret) {
            KPrintf("RtcDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("RtcDriverInit DriverRegister driver has been register state%u\n", rtc_driver->driver.driver_state);
    }

    return ret;
}

int RtcReleaseBus(struct RtcBus *rtc_bus)
{
    NULL_PARAM_CHECK(rtc_bus);

    return BusRelease(&rtc_bus->bus);
}

int RtcDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("RtcDriverAttachToBus find rtc bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_RTC_BUS == bus->bus_type) {
        driver = RtcDriverFind(drv_name, TYPE_RTC_DRV);
        if (NONE == driver) {
            KPrintf("RtcDriverAttachToBus find rtc driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_RTC_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("RtcDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
