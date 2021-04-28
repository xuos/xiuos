/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-12-12     Yi Qiu      first version
 */

/**
* @file usbhost.c
* @brief support usb host function init
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: usbhost.c
Description: support usb host function init
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/core/usbhost.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb host driver init
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>
#include <usb_host.h>

#define USB_HOST_CONTROLLER_NAME      "usbh"

x_err_t UsbHostInit(struct uhcd *hcd)
{
    UcdPointer drv;
   
    UsbhHubInit((UhcdPointer)hcd);

    UsbhClassDriverInit();

#ifdef USBH_MSTORAGE

    drv = UsbhClassDriverStorage();
    UsbhClassDriverRegister(drv);
#endif

 
    drv = UsbhClassDriverHub();
    UsbhClassDriverRegister(drv);

    return EOK;
}