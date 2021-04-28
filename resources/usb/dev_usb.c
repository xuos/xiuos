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
* @file dev_usb.c
* @brief register usb dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_usb.h>
#include <dev_usb.h>

static DoubleLinklistType usbdev_linklist;

/*Create the usb device linklist*/
static void USBDeviceLinkInit()
{
    InitDoubleLinkList(&usbdev_linklist);
}

/*Find the register USB device*/
HardwareDevType USBDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &usbdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("USBDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

/*Register the USB device*/
int USBDeviceRegister(struct UsbHardwareDevice *usb_device, void *usb_param, const char *device_name)
{
    NULL_PARAM_CHECK(usb_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (dev_link_flag) {
        USBDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != usb_device->haldev.dev_state) {
        strncpy(usb_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        usb_device->haldev.dev_type = TYPE_USB_DEV;
        usb_device->haldev.dev_state = DEV_INSTALL;

        usb_device->haldev.dev_done = (struct HalDevDone *)usb_device->dev_done;

        usb_device->haldev.private_data = usb_param;

        DoubleLinkListInsertNodeAfter(&usbdev_linklist, &(usb_device->haldev.dev_link));
    } else {
        KPrintf("USBDeviceRegister device has been register state%u\n", usb_device->haldev.dev_state);        
    }

    return ret;
}

/*Register the USB Device to the USB BUS*/
int USBDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("USBDeviceAttachToBus find usb bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_USB_BUS == bus->bus_type) {
        device = USBDeviceFind(dev_name, TYPE_USB_DEV);
        if (NONE == device) {
            KPrintf("USBDeviceAttachToBus find usb device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_USB_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("usbDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
