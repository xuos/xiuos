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
* @file bus_spi.c
* @brief register spi bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_spi.h>
#include <dev_spi.h>

int SpiBusInit(struct SpiBus *spi_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(spi_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != spi_bus->bus.bus_state) {
        strncpy(spi_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        spi_bus->bus.bus_type = TYPE_SPI_BUS;
        spi_bus->bus.bus_state = BUS_INSTALL;
        spi_bus->bus.private_data = spi_bus->private_data;

        ret = BusRegister(&spi_bus->bus);
        if (EOK != ret) {
            KPrintf("SpiBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SpiBusInit BusRegister bus has been register state%u\n", spi_bus->bus.bus_state);        
    }

    return ret;
}

int SpiDriverInit(struct SpiDriver *spi_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(spi_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != spi_driver->driver.driver_state) {
        spi_driver->driver.driver_type = TYPE_SPI_DRV;
        spi_driver->driver.driver_state = DRV_INSTALL;

        strncpy(spi_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        spi_driver->driver.configure = spi_driver->configure;

        ret = SpiDriverRegister(&spi_driver->driver);
        if (EOK != ret) {
            KPrintf("SpiDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("SpiDriverInit DriverRegister driver has been register state%u\n", spi_driver->driver.driver_state);
    }

    return ret;
}

int SpiReleaseBus(struct SpiBus *spi_bus)
{
    NULL_PARAM_CHECK(spi_bus);

    return BusRelease(&spi_bus->bus);
}

int SpiDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SpiDriverAttachToBus find spi bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_SPI_BUS == bus->bus_type) {
        driver = SpiDriverFind(drv_name, TYPE_SPI_DRV);
        if (NONE == driver) {
            KPrintf("SpiDriverAttachToBus find spi driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_SPI_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("SpiDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
