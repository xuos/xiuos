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
* @file bus_pin.c
* @brief register pin bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_pin.h>
#include <dev_pin.h>

int PinBusInit(struct PinBus *pin_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(pin_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != pin_bus->bus.bus_state) {
        strncpy(pin_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        pin_bus->bus.bus_type = TYPE_PIN_BUS;
        pin_bus->bus.bus_state = BUS_INSTALL;
        pin_bus->bus.private_data = pin_bus->private_data;

        ret = BusRegister(&pin_bus->bus);
        if (EOK != ret) {
            KPrintf("PinBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("PinBusInit BusRegister bus has been register state%u\n", pin_bus->bus.bus_state);        
    }

    return ret;
}

int PinDriverInit(struct PinDriver *pin_driver, const char *driver_name, void *data)
{
    NULL_PARAM_CHECK(pin_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != pin_driver->driver.driver_state) {
        pin_driver->driver.driver_type = TYPE_PIN_DRV;
        pin_driver->driver.driver_state = DRV_INSTALL;

        strncpy(pin_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        pin_driver->driver.configure = pin_driver->configure;
        pin_driver->driver.private_data = data;

        ret = PinDriverRegister(&pin_driver->driver);
        if (EOK != ret) {
            KPrintf("PinDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("PinDriverInit DriverRegister driver has been register state%u\n", pin_driver->driver.driver_state);
    }

    return ret;
}

int PinReleaseBus(struct PinBus *pin_bus)
{
    NULL_PARAM_CHECK(pin_bus);

    return BusRelease(&pin_bus->bus);
}

int PinDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("PinDriverAttachToBus find spi bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_PIN_BUS == bus->bus_type) {
        driver = PinDriverFind(drv_name, TYPE_PIN_DRV);
        if (NONE == driver) {
            KPrintf("PinDriverAttachToBus find spi driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_PIN_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("PinDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}

BusType PinBusInitGet(void)
{
    struct BusConfigureInfo configure_info;
    BusType pin;                                                                
    int ret = 0; 
                                                                 
    pin = BusFind(PIN_BUS_NAME);                                
    if (!pin) {                                                                         
        KPrintf("find %s failed!\n", PIN_BUS_NAME);                           
        return NONE;                                                              
    } 
                                                                          
    pin->owner_driver = BusFindDriver(pin, PIN_DRIVER_NAME);         
    pin->owner_haldev = BusFindDevice(pin, PIN_DEVICE_NAME); 

    configure_info.configure_cmd = OPE_INT;
    ret = BusDrvConfigure(pin->owner_driver, &configure_info);                                      
    if (ret != EOK) {                                                                         
        KPrintf("initialize %s failed!\n", PIN_BUS_NAME);                     
        return NONE;                                                              
    }   

    return pin;
}
