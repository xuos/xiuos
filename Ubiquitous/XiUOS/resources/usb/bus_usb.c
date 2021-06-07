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
* @file bus_usb.c
* @brief register usb bus function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_usb.h>
#include <dev_usb.h>

/*Register the USB BUS*/
int UsbBusInit(struct UsbBus *usb_bus, const char *bus_name)
{
    NULL_PARAM_CHECK(usb_bus);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret = EOK;

    if (BUS_INSTALL != usb_bus->bus.bus_state) {
        strncpy(usb_bus->bus.bus_name, bus_name, NAME_NUM_MAX);

        usb_bus->bus.bus_type = TYPE_USB_BUS;
        usb_bus->bus.bus_state = BUS_INSTALL;
        usb_bus->bus.private_data = usb_bus->private_data;

        ret = BusRegister(&usb_bus->bus);
        if (EOK != ret) {
            KPrintf("UsbBusInit BusRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("UsbBusInit BusRegister bus has been register state%u\n", usb_bus->bus.bus_state);        
    }

    return ret;
}

/*Register the USB Driver*/
int UsbDriverInit(struct UsbDriver *usb_driver, const char *driver_name)
{
    NULL_PARAM_CHECK(usb_driver);
    NULL_PARAM_CHECK(driver_name);

    x_err_t ret = EOK;

    if (DRV_INSTALL != usb_driver->driver.driver_state) {
        usb_driver->driver.driver_type = TYPE_USB_DRV;
        usb_driver->driver.driver_state = DRV_INSTALL;

        strncpy(usb_driver->driver.drv_name, driver_name, NAME_NUM_MAX);

        usb_driver->driver.configure = usb_driver->configure;

        usb_driver->driver.private_data = usb_driver->private_data;

        ret = UsbDriverRegister(&usb_driver->driver);
        if (EOK != ret) {
            KPrintf("UsbDriverInit DriverRegister error %u\n", ret);
            return ret;
        }
    } else {
        KPrintf("UsbDriverInit DriverRegister driver has been register state%u\n", usb_driver->driver.driver_state);
    }

    return ret;
}

/*Release the USB device*/
int UsbReleaseBus(struct UsbBus *usb_bus)
{
    NULL_PARAM_CHECK(usb_bus);

    return BusRelease(&usb_bus->bus);
}

/*Register the USB Driver to the USB BUS*/
int UsbDriverAttachToBus(const char *drv_name, const char *bus_name)
{
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct Driver *driver;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("UsbDriverAttachToBus find usb bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_USB_BUS == bus->bus_type) {
        driver = UsbDriverFind(drv_name, TYPE_USB_DRV);
        if (NONE == driver) {
            KPrintf("UsbDriverAttachToBus find usb driver error!name %s\n", drv_name);
            return ERROR;
        }

        if (TYPE_USB_DRV == driver->driver_type) {
            ret = DriverRegisterToBus(bus, driver);
            if (EOK != ret) {
                KPrintf("UsbDriverAttachToBus DriverRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return ret;
}


