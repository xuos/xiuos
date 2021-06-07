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
* @file bus_i2c.c
* @brief register i2c bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_i2c.h>
#include <dev_i2c.h>

/*Register the I2C BUS*/
int I2cBusInit(struct I2cBus *i2c_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(i2c_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != i2c_bus->bus.bus_state) {
        strncpy(i2c_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        i2c_bus->bus.bus_type = TYPE_I2C_BUS;
        i2c_bus->bus.bus_state = BUS_INSTALL;
        i2c_bus->bus.private_data = i2c_bus->private_data;

        ret = BusRegister(&i2c_bus->bus);
        if (EOK != ret) {
            KPrintf("I2cBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("I2cBusInit BusRegister bus has been register state%u\n", i2c_bus->bus.bus_state);        
    }

    return ret;
}

/*Register the I2C Driver*/
int I2cDriverInit(struct I2cDriver *i2c_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(i2c_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != i2c_driver->driver.driver_state) {
        i2c_driver->driver.driver_type = TYPE_I2C_DRV;
        i2c_driver->driver.driver_state = DRV_INSTALL;

        strncpy(i2c_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        i2c_driver->driver.private_data = i2c_driver->private_data;

        ret = I2cDriverRegister(&i2c_driver->driver);
        if (EOK != ret) {
            KPrintf("I2cDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("I2cDriverInit DriverRegister driver has been register state%u\n", i2c_driver->driver.driver_state);
    }

    return ret;
}

/*Release the I2C device*/
int I2cReleaseBus(struct I2cBus *i2c_bus)
{
    NULL_PARAM_CHECK(i2c_bus);

    return BusRelease(&i2c_bus->bus);
}

/*Register the I2C Driver to the I2C BUS*/
int I2cDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("I2cDriverAttachToBus find i2c bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_I2C_BUS == bus->bus_type) {
        driver = I2cDriverFind(drv_name, TYPE_I2C_DRV);
        if (NONE == driver) {
            KPrintf("I2cDriverAttachToBus find i2c driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_I2C_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("I2cDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}
