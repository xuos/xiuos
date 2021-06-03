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
* @file mass.h
* @brief define usb storage function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: mass.h
Description: define usb storage function and struct
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/class/mass.h for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support define usb storage configure, function and struct
2. support usb bus driver framework 
*************************************************/

#ifndef __MASS_H__
#define __MASS_H__

#include <xiuos.h>


#define MAX_PARTITION_COUNT        4
#define SECTOR_SIZE                512

struct UstorData
{
   
    struct uhintf* intf;
    int UdiskId;
    const char path;
};

struct ustor
{
    upipe_t pipe_in;
    upipe_t pipe_out;
    uint32 capicity[2];
    
    uint8 DevCnt;
};    
typedef struct ustor* UstorPointer;

x_err_t UsbhStorageGetMaxLun(struct uhintf* intf, uint8* MaxLun);
x_err_t UsbhStorageReset(struct uhintf* intf);
x_err_t UsbhStorageRead10(struct uhintf* intf, uint8 *buffer, 
    uint32 sector, x_size_t count, int timeout);
x_err_t UsbhStorageWrite10(struct uhintf* intf, uint8 *buffer, 
    uint32 sector, x_size_t count, int timeout);
x_err_t UsbhStorageRequestSense(struct uhintf* intf, uint8* buffer);
x_err_t UsbhStorageTestUnitReady(struct uhintf* intf);
x_err_t UsbhStorageInquiry(struct uhintf* intf, uint8* buffer);
x_err_t UsbhStorageGetCapacity(struct uhintf* intf, uint8* buffer);

#endif