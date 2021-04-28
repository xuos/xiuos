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
* @file udisk.c
* @brief support usb udisk function , write and read
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: udisk.c
Description: support usb udisk function , write and read
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/class/udisk.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb udisk configure, function and struct
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>

#include <usb_host.h>
#include "mass.h"

#if defined(FS_VFS)
#include <iot-vfs.h>
#endif

#ifdef USBH_MSTORAGE
#include <bus.h>
#include <bus_usb.h>
#include <dev_usb.h>


#define UDISK_MAX_COUNT        8
static uint8 UdiskIdset = 0;

static int UdiskGetId(void)
{
    int i;

    for(i=0; i< UDISK_MAX_COUNT; i++)
    {
        if((UdiskIdset & (1 << i)) != 0) continue;
        else break;
    }

   
    if(i == UDISK_MAX_COUNT) CHECK(0);

    UdiskIdset |= (1 << i);
    return i;
}

static void UdiskFreeId(int id)
{
    CHECK(id < UDISK_MAX_COUNT);

    UdiskIdset &= ~(1 << id);
}

static x_size_t UdiskRead_new(struct UstorData* data, x_OffPos pos, void* buffer,
    x_size_t size)
{
    x_err_t ret;
    struct uhintf* intf;
    int timeout = USB_TIMEOUT_LONG;
    
    
    NULL_PARAM_CHECK(buffer);

    if(size > 4096) timeout *= 2;

    intf = data->intf;

    ret = UsbhStorageRead10(intf, (uint8*)buffer, pos, size, timeout);

    if (ret != EOK)
    {
        KPrintf("usb mass_storage read failed\n");
        return 0;
    }

    return size;
}

static x_size_t UdiskWrite_new (struct UstorData* data, x_OffPos pos, const void* buffer,
    x_size_t size)
{
    x_err_t ret;
    struct uhintf* intf;
    int timeout = USB_TIMEOUT_LONG;

    
    NULL_PARAM_CHECK(buffer);

    if(size * SECTOR_SIZE > 4096) timeout *= 2;

    intf = data->intf;

    ret = UsbhStorageWrite10(intf, (uint8*)buffer, pos, size, timeout);
    if (ret != EOK)
    {
        KPrintf("usb mass_storage write %d sector failed\n", size);
        return 0;
    }

    return size;
}

uint32 UdiskRead_new_api(void *dev, struct BusBlockReadParam *read_param)
{
    struct UsbHardwareDevice* usb_dev = (struct UsbHardwareDevice*)dev;

    return UdiskRead_new(usb_dev->private_data, read_param->pos, read_param->buffer, read_param->size);
}

uint32 UdiskWirte_new_api(void *dev, struct BusBlockWriteParam *write_param)
{
    struct UsbHardwareDevice* usb_dev = (struct UsbHardwareDevice*)dev;

    return UdiskWrite_new(usb_dev->private_data, write_param->pos, write_param->buffer, write_param->size);
}

x_err_t UdiskRun(struct uhintf* intf)
{
    int i = 0;
    x_err_t ret;
    char dname[8];
    char sname[8];
    uint8 MaxLun, *sector, sense[18], inquiry[36];
   
    UstorPointer stor;

 
    NULL_PARAM_CHECK(intf );


    ret = UsbhStorageReset(intf);
    if(ret != EOK) return ret;

    stor = (UstorPointer)intf->UserData;

    
    ret = UsbhStorageGetMaxLun(intf, &MaxLun);
    if(ret != EOK)
        UsbhClearFeature(intf->device, 0, USB_FEATURE_ENDPOINT_HALT);

  
    if(stor->pipe_in->status == UPIPE_STATUS_STALL)
    {
        ret = UsbhClearFeature(intf->device,
        stor->pipe_in->ep.bEndpointAddress, USB_FEATURE_ENDPOINT_HALT);
        if(ret != EOK) return ret;
    }


    
    if(stor->pipe_out->status == UPIPE_STATUS_STALL)
    {
        ret = UsbhClearFeature(intf->device,
        stor->pipe_out->ep.bEndpointAddress, USB_FEATURE_ENDPOINT_HALT);
        if(ret != EOK) return ret;
    }

    while((ret = UsbhStorageInquiry(intf, inquiry)) != EOK)
    {
        if(ret == -EPIO) return ret;

        DelayKTask(5);
        if(i++ < 10) continue;
        KPrintf("UsbhStorageInquiry error\n");
        return -ERROR;
    }

    i = 0;

 
    while((ret = UsbhStorageTestUnitReady(intf)) != EOK)
    {
        if(ret == -EPIO) return ret;

        ret = UsbhStorageRequestSense(intf, sense);
        if(ret == -EPIO) return ret;

        DelayKTask(10);
        if(i++ < 10) continue;

        KPrintf("UsbhStorageTestUnitReady error\n");
        return -ERROR;
    }

    i = 0;
    memset(stor->capicity, 0, sizeof(stor->capicity));

    
    while((ret = UsbhStorageGetCapacity(intf,
        (uint8*)stor->capicity)) != EOK)
    {
        if(ret == -EPIO) return ret;

        DelayKTask(50);
        if(i++ < 10) continue;

        stor->capicity[0] = 2880;
        stor->capicity[1] = 0x200;

        KPrintf("UsbhStorageGetCapacity error\n");
        break;
    }

    stor->capicity[0] = uswap_32(stor->capicity[0]);
    stor->capicity[1] = uswap_32(stor->capicity[1]);
    stor->capicity[0] += 1;

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("capicity %d, block size %d\n",
        stor->capicity[0], stor->capicity[1]));

   
    sector = (uint8*) x_malloc (SECTOR_SIZE);
    if (sector == NONE)
    {
        KPrintf("allocate partition sector buffer failed\n");
        return -ERROR;
    }

    memset(sector, 0, SECTOR_SIZE);

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("read partition table\n"));


    ret = UsbhStorageRead10(intf, sector, 0, 1, USB_TIMEOUT_LONG);
    if(ret != EOK)
    {
        KPrintf("read parition table error\n");

        x_free(sector);
        return -ERROR;
    }

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("finished reading partition\n"));

    struct UstorData* data = x_malloc(sizeof(struct UstorData));
    memset(data, 0, sizeof(struct UstorData));
    data->UdiskId = UdiskGetId();

  
    data->intf = intf;


    snprintf(dname, sizeof(dname), "udisk%d", data->UdiskId);

    struct Bus* usb_bus = BusFind(UDISK_USB_BUS_NAME);
    struct UsbHardwareDevice* usb_dev = (struct UsbHardwareDevice*)BusFindDevice(usb_bus, UDISK_USB_DEVICE_NAME);
    if (usb_dev)
    {
        KPrintf("udisk.c find usb dev seccess\n");
        usb_dev->private_data = (void*)data;
    }
    else
    {
        KPrintf("udisk.c find usb dev fail   \n");
    }
 
    data->intf = intf;

    NULL_PARAM_CHECK(data);
    NULL_PARAM_CHECK(intf );
    CHECK(data->intf == intf);

    stor->DevCnt++;
  
    if (MountFilesystem(UDISK_USB_BUS_NAME, UDISK_USB_DEVICE_NAME,USB_DRIVER_NAME, FSTYPE_FATFS, UDISK_MOUNTPOINT) == 0)
        KPrintf("Mount FAT on Udisk successful.\n");
    else
        KPrintf("Mount FAT on Udisk failed.\n");

//     for(i=0; i<MAX_PARTITION_COUNT; i++)
//     {
//         /* get the first partition */
//         ret = dfs_filesystem_get_partition(&part[i], sector, i);
//         if (ret == EOK)
//         {
//             struct ustor_data* data = x_malloc(sizeof(struct ustor_data));
//             memset(data, 0, sizeof(struct ustor_data));
//             data->intf = intf;
//             data->udisk_id = udisk_get_id();
//             snprintf(dname, 6, "ud%d-%d", data->udisk_id, i);
//             snprintf(sname, 8, "sem_ud%d",  i);
//             data->part.lock = KSemaphoreCreate( 1);

//             /* register sdcard device */
//             stor->dev[i].type    = Dev_TYPE_Block;

//             stor->dev[i].init    = udisk_init;
//             stor->dev[i].read    = udisk_read;
//             stor->dev[i].write   = udisk_write;
//             stor->dev[i].control = udisk_control;

//             stor->dev[i].UserData = (void*)data;

//             DeviceRegister(&stor->dev[i], dname, SIGN_OPER_RDWR |
//                SIGN_OPER_REMOVABLE |SIGN_OPER_STANDALONE);

//             stor->dev_cnt++;
//             if (dfs_mount(stor->dev[i].parent.name, UDISK_MOUNTPOINT, "elm",
//                 0, 0) == 0)
//             {
//                 SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("udisk part %d mount successfully\n", i));
//             }
//             else
//             {
//                 SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("udisk part %d mount failed\n", i));
//             }
//         }
//         else
//         {
//             if(i == 0)
//             {
//                 struct ustor_data* data = x_malloc(sizeof(struct ustor_data));
//                 memset(data, 0, sizeof(struct ustor_data));
//                 data->udisk_id = udisk_get_id();

//                 /* there is no partition table */
//                 data->part.offset = 0;
//                 data->part.size   = 0;
//                 data->intf = intf;
//                 data->part.lock = KSemaphoreCreate( 1);

//                 snprintf(dname, 7, "udisk%d", data->udisk_id);

//                 /* register sdcard device */
//                 stor->dev[0].type    = Dev_TYPE_Block;

//                 stor->dev[0].init    = udisk_init;
//                 stor->dev[0].read    = udisk_read;
//                 stor->dev[0].write   = udisk_write;
//                 stor->dev[0].control = udisk_control;

//                 stor->dev[0].UserData = (void*)data;

//                 DeviceRegister(&stor->dev[0], dname,
//                     SIGN_OPER_RDWR |SIGN_OPER_REMOVABLE
//                     |SIGN_OPER_STANDALONE);

//                 stor->dev_cnt++;
//                 if (dfs_mount(stor->dev[0].parent.name, UDISK_MOUNTPOINT,
//                     "elm", 0, 0) == 0)
//                 {
//                     KPrintf("Mount FAT on Udisk successful.\n");
//                 }
//                 else
//                 {
//                     KPrintf("Mount FAT on Udisk failed.\n");
//                 }
//             }

//             break;
//         }
//     }

    x_free(sector);

    return EOK;
}

x_err_t UdiskStop(struct uhintf* intf)
{
    int i;
    UstorPointer stor;
 
    NULL_PARAM_CHECK(intf);
    NULL_PARAM_CHECK(intf->device);

    stor = (UstorPointer)intf->UserData;
    NULL_PARAM_CHECK(stor);

    for(i=0; i<stor->DevCnt; i++)
    {
        UnmountFileSystem(UDISK_MOUNTPOINT);
    }

    return EOK;
}

#endif
