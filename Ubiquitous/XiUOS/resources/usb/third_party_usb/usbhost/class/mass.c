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
* @file mass.c
* @brief support usb storage function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: mass.c
Description: support usb storage function
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/class/mass.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb storage configure, function and struct
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>
#include <usb_host.h>
#include "mass.h"

#ifdef USBH_MSTORAGE

extern x_err_t UdiskRun(struct uhintf* intf);
extern x_err_t UdiskStop(struct uhintf* intf);

static struct UclassDriver StorageDriver;


static x_err_t PipeCheck(struct uhintf* intf, upipe_t pipe)
{
    struct uinstance* device;        
    x_err_t ret;
    UstorPointer stor;
    int size = 0; 
    struct UstorageCsw csw;

    if(intf == NONE || pipe == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    

  
    device = intf->device;    

 
    stor = (UstorPointer)intf->UserData;
    
 
    if(pipe->status == UPIPE_STATUS_OK) return EOK;

    if(pipe->status == UPIPE_STATUS_ERROR)
    {
        KPrintf("pipe status error\n");
        return -EPIO;
    }
    if(pipe->status == UPIPE_STATUS_STALL)
    {
       
        ret = UsbhClearFeature(device, pipe->ep.bEndpointAddress, 
            USB_FEATURE_ENDPOINT_HALT);
        if(ret != EOK) return ret;
    }
    

    DelayKTask(50);

    KPrintf("pipes1 0x%x, 0x%x\n", stor->pipe_in, stor->pipe_out);

    stor->pipe_in->status = UPIPE_STATUS_OK;

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("clean storage in pipe stall\n"));

    
    size = UsbHcdPipeXfer(stor->pipe_in->inst->hcd, 
        stor->pipe_in, &csw, SIZEOF_CSW, 100);
    if(size != SIZEOF_CSW) 
    {
        KPrintf("receive the csw after stall failed\n");
        return -EPIO;
    }
    
    return -ERROR;
}


static x_err_t UsbBulkOnlyXfer(struct uhintf* intf, 
    UstorageCbwPointer cmd, uint8* buffer, int timeout)
{
    x_size_t size;
    x_err_t ret;    
    upipe_t pipe;
    struct UstorageCsw csw;
    UstorPointer stor;

    NULL_PARAM_CHECK(cmd);    

    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }
    
 
    stor = (UstorPointer)intf->UserData;

    do
    {
        
        size = UsbHcdPipeXfer(stor->pipe_out->inst->hcd, stor->pipe_out, 
            cmd, SIZEOF_CBW, timeout);
        if(size != SIZEOF_CBW) 
        {
            KPrintf("CBW size error\n");
            return -EPIO;
        }
        if(cmd->XferLen != 0)
        {
            pipe = (cmd->dflags == CBWFLAGS_DIR_IN) ? stor->pipe_in :
                stor->pipe_out;
            size = UsbHcdPipeXfer(pipe->inst->hcd, pipe, (void*)buffer, 
                cmd->XferLen, timeout);
            if(size != cmd->XferLen)
            {
                KPrintf("request size %d, transfer size %d\n", 
                    cmd->XferLen, size);
                break;
            }    
        }
        
    
        size = UsbHcdPipeXfer(stor->pipe_in->inst->hcd, stor->pipe_in, 
            &csw, SIZEOF_CSW, timeout);
        if(size != SIZEOF_CSW) 
        {
            KPrintf("csw size error\n");
            return -EPIO;
        }
    }while(0);

    
    ret = PipeCheck(intf, stor->pipe_in);
    if(ret != EOK) 
    {
        KPrintf("in pipe error\n");
        return ret;
    }
    
 
    ret = PipeCheck(intf, stor->pipe_out);
    if(ret != EOK)
    {
        KPrintf("out pipe error\n");
        return ret;
    }
    
  
    if(csw.signature != CSW_SIGNATURE || csw.tag != CBW_TAG_VALUE)
    {
        KPrintf("csw signature error\n");
        return -EPIO;
    }
    
    if(csw.status != 0)
    {
     
        return -ERROR;
    }
    
    return EOK;
}


x_err_t UsbhStorageGetMaxLun(struct uhintf* intf, uint8* MaxLun)
{
    struct uinstance* device;    
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;

    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    

    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageGetMaxLun\n"));


    device = intf->device;    

 
    setup.RequestType = USB_REQ_TYPE_DIR_IN | USB_REQ_TYPE_CLASS | 
        USB_REQ_TYPE_INTERFACE;
    setup.bRequest = USBREQ_GET_MAX_LUN;
    setup.wValue = intf->IntfDesc->bInterfaceNumber;
    setup.wIndex = 0;
    setup.wLength = 1;

 
    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return -EPIO;
    }
    if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, MaxLun, 1, timeout) != 1)
    {
        return -EPIO;
    }
    if(UsbHcdPipeXfer(device->hcd, device->PipeEp0Out, NONE, 0, timeout) != 0)
    {
        return -EPIO;
    }
    return EOK;
}


x_err_t UsbhStorageReset(struct uhintf* intf)
{
    struct urequest setup;
    struct uinstance* device;    
    int timeout = USB_TIMEOUT_BASIC;

   
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    

    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageReset\n"));

    
    device = intf->device;    

 
    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_CLASS | 
        USB_REQ_TYPE_INTERFACE;
    setup.bRequest = USBREQ_MASS_STORAGE_RESET;
    setup.wIndex = intf->IntfDesc->bInterfaceNumber;
    setup.wLength = 0;
    setup.wValue = 0;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return -EPIO;
    }
    if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, NONE, 0, timeout) != 0)
    {
        return -EPIO;
    }
    return EOK;
}


x_err_t UsbhStorageRead10(struct uhintf* intf, uint8 *buffer, 
    uint32 sector, x_size_t count, int timeout)
{
    struct UstorageCbw cmd;

   
    if(intf == NONE)
    {
        KPrintf("interface is not available\n");
        return -EPIO;
    }    
    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageRead10\n"));

   
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = SECTOR_SIZE * count;
    cmd.dflags = CBWFLAGS_DIR_IN;
    cmd.lun = 0;
    cmd.CbLen = 10;
    cmd.cb[0] = SCSI_READ_10;
    cmd.cb[1] = 0;
    cmd.cb[2] = (uint8)(sector >> 24);
    cmd.cb[3] = (uint8)(sector >> 16);
    cmd.cb[4] = (uint8)(sector >> 8);
    cmd.cb[5] = (uint8)sector;
    cmd.cb[6] = 0;
    cmd.cb[7] = (count & 0xff00) >> 8;
    cmd.cb[8] = (uint8) count & 0xff;

    return UsbBulkOnlyXfer(intf, &cmd, buffer, timeout);
}


x_err_t UsbhStorageWrite10(struct uhintf* intf, uint8 *buffer, 
    uint32 sector, x_size_t count, int timeout)
{
    struct UstorageCbw cmd;

  
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    
    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageWrite10\n"));

  
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = SECTOR_SIZE * count;
    cmd.dflags = CBWFLAGS_DIR_OUT;
    cmd.lun = 0;
    cmd.CbLen = 10;
    cmd.cb[0] = SCSI_WRITE_10;
    cmd.cb[1] = 0;
    cmd.cb[2] = (uint8)(sector >> 24);
    cmd.cb[3] = (uint8)(sector >> 16);
    cmd.cb[4] = (uint8)(sector >> 8);
    cmd.cb[5] = (uint8)sector;
    cmd.cb[6] = 0;
    cmd.cb[7] = (count & 0xff00) >> 8;
    cmd.cb[8] = (uint8) count & 0xff;

    return UsbBulkOnlyXfer(intf, &cmd, buffer, timeout);
}


x_err_t UsbhStorageRequestSense(struct uhintf* intf, uint8* buffer)
{
    struct UstorageCbw cmd;
    int timeout = USB_TIMEOUT_LONG;

   
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    
    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageRequestSense\n"));

    
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = 18;
    cmd.dflags = CBWFLAGS_DIR_IN;
    cmd.lun = 0;
    cmd.CbLen = 6;
    cmd.cb[0] = SCSI_REQUEST_SENSE;
    cmd.cb[4] = 18;

    return UsbBulkOnlyXfer(intf, &cmd, buffer, timeout);
}


x_err_t UsbhStorageTestUnitReady(struct uhintf* intf)
{
    struct UstorageCbw cmd;
    int timeout = USB_TIMEOUT_LONG;

 
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    
    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageTestUnitReady\n"));

   
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = 0;
    cmd.dflags = CBWFLAGS_DIR_OUT;
    cmd.lun = 0;
    cmd.CbLen = 12;
    cmd.cb[0] = SCSI_TEST_UNIT_READY;
 
    return UsbBulkOnlyXfer(intf, &cmd, NONE, timeout);
}


x_err_t UsbhStorageInquiry(struct uhintf* intf, uint8* buffer)
{
    struct UstorageCbw cmd;
    int timeout = USB_TIMEOUT_LONG;
    
   
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    
    
    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageInquiry\n"));

    
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = 36;
    cmd.dflags = CBWFLAGS_DIR_IN;
    cmd.lun = 0;
    cmd.CbLen = 6;
    cmd.cb[0] = SCSI_INQUIRY_CMD;
    cmd.cb[4] = 36;

    return UsbBulkOnlyXfer(intf, &cmd, buffer, timeout);
}


x_err_t UsbhStorageGetCapacity(struct uhintf* intf, uint8* buffer)
{
    struct UstorageCbw cmd;
    int timeout = USB_TIMEOUT_LONG;

   
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }    

    NULL_PARAM_CHECK(intf->device);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("UsbhStorageGetCapacity\n"));

 
    memset(&cmd, 0, sizeof(struct UstorageCbw));
    cmd.signature = CBW_SIGNATURE;
    cmd.tag = CBW_TAG_VALUE;
    cmd.XferLen = 8;
    cmd.dflags = CBWFLAGS_DIR_IN;
    cmd.lun = 0;
    cmd.CbLen = 12;
    cmd.cb[0] = SCSI_READ_CAPACITY;

    return UsbBulkOnlyXfer(intf, &cmd, buffer, timeout);
}


static x_err_t UsbhStorageEnable(void* arg)
{
    int i = 0;
    x_err_t ret;    
    UstorPointer stor;
    struct uhintf* intf = (struct uhintf*)arg;

    
    if(intf == NONE)
    {
        KPrintf("the interface is not available\n");
        return -EPIO;
    }

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("subclass %d, protocal %d\n", 
        intf->IntfDesc->bInterfaceSubClass,
        intf->IntfDesc->bInterfaceProtocol));
        
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("usbh_storage_run\n"));

  
    
    stor = x_malloc(sizeof(struct ustor));
    NULL_PARAM_CHECK(stor);


    memset(stor, 0, sizeof(struct ustor));    
    intf->UserData = (void*)stor;

    for(i=0; i<intf->IntfDesc->bNumEndpoints; i++)
    {        
        UepDescPointer EpDesc;
        
        
        UsbhGetEndpointDescriptor(intf->IntfDesc, i, &EpDesc);
        if(EpDesc == NONE)
        {
            KPrintf("usb_get_endpoint_descriptor error\n");
            return -ERROR;
        }
        
          
        if((EpDesc->bmAttributes & USB_EP_ATTR_TYPE_MASK) != USB_EP_ATTR_BULK)
            continue;
        
    
        if(EpDesc->bEndpointAddress & USB_DIR_IN)
        {
          
            stor->pipe_in = UsbInstanceFindPipe(intf->device,EpDesc->bEndpointAddress);
        }
        else
        {        
       
            stor->pipe_out = UsbInstanceFindPipe(intf->device,EpDesc->bEndpointAddress);
        }
    }

  
    if(stor->pipe_in == NONE || stor->pipe_out == NONE)
    {
        KPrintf("pipe error, unsupported device\n");
        return -ERROR;
    }    
    
 
    ret = UdiskRun(intf);
    if(ret != EOK) return ret;

    return EOK;
}


static x_err_t UsbhStorageDisable(void* arg)
{
    UstorPointer stor;
    struct uhintf* intf = (struct uhintf*)arg;

   
    NULL_PARAM_CHECK(intf);
    NULL_PARAM_CHECK(intf->UserData);
    NULL_PARAM_CHECK(intf->device );

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("usbh_storage_stop\n"));

    
    stor = (UstorPointer)intf->UserData;

    UdiskStop(intf);

    
  
    if(stor != NONE) x_free(stor);
    return EOK;
}


UcdPointer UsbhClassDriverStorage(void)
{
    StorageDriver.ClassCode = USB_CLASS_MASS_STORAGE;
    
    StorageDriver.enable = UsbhStorageEnable;
    StorageDriver.disable = UsbhStorageDisable;

    return &StorageDriver;
}

#endif