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
* @file dev_usb.h
* @brief define usb dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_USB_H
#define DEV_USB_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UDISK_USB_BUS_NAME       USB_BUS_NAME
#define UDISK_USB_DEVICE_NAME USB_DEVICE_NAME

struct UsbDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct UsbHardwareDevice
{
    struct HardwareDev haldev;
    const struct UsbDevDone *dev_done;

    void *private_data;
};

/*Register the USB device*/
int USBDeviceRegister(struct UsbHardwareDevice *usb_device, void *usb_param, const char *device_name);

/*Register the USB device to the USB bus*/
int USBDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register USB device*/
HardwareDevType USBDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
