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
* @file bus_sdio.c
* @brief register sdio bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_sdio.h>
#include <dev_sdio.h>

int SdioBusInit(struct SdioBus *sdio_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(sdio_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != sdio_bus->bus.bus_state) {
        strncpy(sdio_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        sdio_bus->bus.bus_type = TYPE_SDIO_BUS;
        sdio_bus->bus.bus_state = BUS_INSTALL;
        sdio_bus->bus.private_data = sdio_bus->private_data;

        ret = BusRegister(&sdio_bus->bus);
        if (EOK != ret) {
            KPrintf("SdioBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SdioBusInit BusRegister bus has been register state%u\n", sdio_bus->bus.bus_state);        
    }

    return ret;
}

int SdioDriverInit(struct SdioDriver *sdio_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(sdio_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != sdio_driver->driver.driver_state) {
        sdio_driver->driver.driver_type = TYPE_SDIO_DRV;
        sdio_driver->driver.driver_state = DRV_INSTALL;

        strncpy(sdio_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        sdio_driver->driver.configure = sdio_driver->configure;

        ret = SdioDriverRegister(&sdio_driver->driver);
        if (EOK != ret) {
            KPrintf("SdioDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SdioDriverInit DriverRegister driver has been register state%u\n", sdio_driver->driver.driver_state);
    }

    return ret;
}

int SdioReleaseBus(struct SdioBus *sdio_bus)
{
    NULL_PARAM_CHECK(sdio_bus);

    return BusRelease(&sdio_bus->bus);
}

int SdioDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SdioDriverAttachToBus find sdio bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_SDIO_BUS == bus->bus_type) {
        driver = SdioDriverFind(drv_name);
        if (NONE == driver) {
            KPrintf("SdioDriverAttachToBus find sdio driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_SDIO_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("SdioDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
