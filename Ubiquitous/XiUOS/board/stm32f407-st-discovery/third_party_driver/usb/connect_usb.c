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
* @file connect_usb.c
* @brief support stm32f407-st-discovery-board usb function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <board.h>
#include "core_cm4.h"
#include "connect_usb.h"
#include "bus_usb.h"
#include "dev_usb.h"

uint32 UdiskRead_new_api(void *dev, struct BusBlockReadParam *read_param);
uint32 UdiskWirte_new_api(void *dev, struct BusBlockWriteParam *write_param);

static uint32 UdiskOpenNewApi(void *dev)
{
    return EOK;
}

static uint32 UdiskCloseNewApi(void *dev)
{
    return EOK;
}

/*manage the usb device operations*/
static const struct UsbDevDone dev_done =
{
    .open = UdiskOpenNewApi,
    .close = UdiskCloseNewApi,
    .write = UdiskWirte_new_api,
    .read = UdiskRead_new_api,
};

/*Init usb bus*/
static int BoardUsbBusInit(struct UsbBus *usb_bus, struct UsbDriver *usb_driver)
{
    x_err_t ret = EOK;

    /*Init the usb bus */
    ret = UsbBusInit(usb_bus, USB_BUS_NAME);
    if (EOK != ret) {
        KPrintf("board_usb_init UsbBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the usb driver*/
    ret = UsbDriverInit(usb_driver, USB_DRIVER_NAME);
    if (EOK != ret) {
        KPrintf("board_usb_init UsbDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the usb driver to the usb bus*/
    ret = UsbDriverAttachToBus(USB_DRIVER_NAME, USB_BUS_NAME);
    if (EOK != ret) {
        KPrintf("board_usb_init USEDriverAttachToBus error %d\n", ret);
        return ERROR;
    }

    return ret;
}

/*Attach the usb device to the usb bus*/
static int BoardUsbDevBend(void)
{
    x_err_t ret = EOK;
    static struct UsbHardwareDevice usb_device1;
    memset(&usb_device1, 0, sizeof(struct UsbHardwareDevice));

    usb_device1.dev_done = &dev_done;

    ret = USBDeviceRegister(&usb_device1, NONE, USB_DEVICE_NAME);
    if (EOK != ret) {
        KPrintf("board_usb_init USBDeviceInit device %s error %d\n", USB_DEVICE_NAME, ret);
        return ERROR;
    }

    ret = USBDeviceAttachToBus(USB_DEVICE_NAME, USB_BUS_NAME);
    if (EOK != ret) {
        KPrintf("board_usb_init USBDeviceAttachToBus device %s error %d\n", USB_DEVICE_NAME, ret);
        return ERROR;
    }

    return ret;
}

/*ARM-32 BOARD USB INIT*/
int Stm32HwUsbInit(void)
{
    x_err_t ret = EOK;
    static struct UsbBus usb_bus;
    memset(&usb_bus, 0, sizeof(struct UsbBus));

    static struct UsbDriver usb_driver;
    memset(&usb_driver, 0, sizeof(struct UsbDriver));

    ret = BoardUsbBusInit(&usb_bus, &usb_driver);
    if (EOK != ret) {
        KPrintf("board_usb_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardUsbDevBend();
    if (EOK != ret) {
        KPrintf("board_usb_Init error ret %u\n", ret);
        return ERROR;
    }

    return ret;
}
