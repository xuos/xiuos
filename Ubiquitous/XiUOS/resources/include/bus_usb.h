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
* @file bus_usb.h
* @brief define usb bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_USB_H
#define BUS_USB_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct UsbDriver
{
    struct Driver driver;

    uint32 (*configure)(void *drv, struct BusConfigureInfo *configure_info);

    void *private_data;
};

struct UsbBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the USB bus*/
int UsbBusInit(struct UsbBus *usb_bus, const char *bus_name);

/*Register the USB driver*/
int UsbDriverInit(struct UsbDriver *usb_driver, const char *driver_name);

/*Release the USB device*/
int UsbReleaseBus(struct UsbBus *usb_bus);

/*Register the USB driver to the USB bus*/
int UsbDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int UsbDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType UsbDriverFind(const char *drv_name, enum DriverType drv_type);

#ifdef __cplusplus
}
#endif

#endif
