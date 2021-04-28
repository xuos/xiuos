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
* @file bus_lcd.c
* @brief register lcd bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_lcd.h>
#include <dev_lcd.h>

int LcdBusInit(struct LcdBus *lcd_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(lcd_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != lcd_bus->bus.bus_state) {
        strncpy(lcd_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        lcd_bus->bus.bus_type = TYPE_LCD_BUS;
        lcd_bus->bus.bus_state = BUS_INSTALL;
        lcd_bus->bus.private_data = lcd_bus->private_data;

        ret = BusRegister(&lcd_bus->bus);
        if (EOK != ret) {
            KPrintf("LCDBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("LCDBusInit BusRegister bus has been register state%u\n", lcd_bus->bus.bus_state);        
    }

    return ret;
}

int LcdDriverInit(struct LcdDriver *lcd_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(lcd_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != lcd_driver->driver.driver_state) {
        lcd_driver->driver.driver_type = TYPE_LCD_DRV;
        lcd_driver->driver.driver_state = DRV_INSTALL;

        strncpy(lcd_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        lcd_driver->driver.configure = lcd_driver->configure;

        ret = LcdDriverRegister(&lcd_driver->driver);
        if (EOK != ret) {
            KPrintf("LcdDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("LcdDriverInit DriverRegister driver has been register state%u\n", lcd_driver->driver.driver_state);
    }

    return ret;
}

int LcdReleaseBus(struct LcdBus *lcd_bus)
{
    NULL_PARAM_CHECK(lcd_bus);

    return BusRelease(&lcd_bus->bus);
}

int LcdDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;
    
    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("LcdDriverAttachToBus find lcd bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_LCD_BUS == bus->bus_type) {
        driver = LcdDriverFind(drv_name, TYPE_LCD_DRV);
        if (NONE == driver) {
            KPrintf("LcdDriverAttachToBus find lcd driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_LCD_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("LcdDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}

