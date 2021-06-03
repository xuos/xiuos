/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-03-12     Yi Qiu      first version
 */

/**
* @file core.c
* @brief support usb host function and configure
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: core.c
Description: support usb host function and configure
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/core/usbhost_core.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb host configure, function and struct
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>
#include <usb_host.h>

static struct uinstance dev[USB_MAX_DEVICE];


/**
 * Allocate a usb host instance
 * 
 * @param uhcd pointer to a uhcd object
 * 
 * @return pointer to a uinstance object (NULL on failure)
 */
UinstPointer UsbhAllocInstance(UhcdPointer uhcd)
{
    int i;
    x_base lock;

    for(i=0; i<USB_MAX_DEVICE; i++)
    {
        lock = CriticalAreaLock();
        if(dev[i].status != DEV_STATUS_IDLE) continue;
        
        memset(&dev[i], 0, sizeof(struct uinstance));
        
        dev[i].status = DEV_STATUS_BUSY;
        dev[i].index = i + 1;
        dev[i].address = 0;
        dev[i].MaxPacketSize = 0x8;
        InitDoubleLinkList(&dev[i].pipe);
        dev[i].hcd = uhcd;
 
        CriticalAreaUnLock(lock);
        return &dev[i];
    }
     
    return NONE;
}


static struct UendpointDescriptor Ep0OutDesc = 
{
 
    USB_DESC_LENGTH_ENDPOINT,
    USB_DESC_TYPE_ENDPOINT,
    0x00 | USB_DIR_OUT,
    USB_EP_ATTR_CONTROL,
    0x00,
    0x00,
};
static struct UendpointDescriptor Ep0InDesc = 
{
  
    USB_DESC_LENGTH_ENDPOINT,
    USB_DESC_TYPE_ENDPOINT,
    0x00 | USB_DIR_IN,
    USB_EP_ATTR_CONTROL,
    0x00,
    0x00,
};

/**
 * Enumerate and invoke class driver when a usb device is connected
 * 
 * @param device pointer to a uinstance object
 */
x_err_t UsbhAttatchInstance(UinstPointer device)
{
    int i = 0;
    x_err_t ret = EOK;
    struct UconfigDescriptor CfgDesc;
    UdevDescPointer DevDesc;
    UintfDescPointer IntfDesc;
    UepDescPointer EpDesc;
    uint8 EpIndex;
    upipe_t pipe;
    UcdPointer drv;

    NULL_PARAM_CHECK(device);
    
    memset(&CfgDesc, 0, sizeof(struct UconfigDescriptor));
    DevDesc = &device->DevDesc;
    
   
    Ep0OutDesc.wMaxPacketSize = 8;
    Ep0InDesc.wMaxPacketSize = 8;
    UsbHcdAllocPipe(device->hcd, &device->PipeEp0Out, device, &Ep0OutDesc);
    UsbHcdAllocPipe(device->hcd, &device->PipeEp0In, device, &Ep0InDesc);

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("start enumnation\n"));
    
    
    ret = UsbhGetDescriptor(device, USB_DESC_TYPE_DEVICE, (void*)DevDesc, 8);
    if(ret != EOK)
    {
        KPrintf("get device descriptor head failed\n");
        return ret;
    }
    

    
    UsbhHubResetPort(device->parent_hub, device->port);
    DelayKTask(2);
    UsbhHubClearPortFeature(device->parent_hub, i + 1, PORT_FEAT_C_CONNECTION);
    
    ret = UsbhSetAddress(device);

    if(ret != EOK)
    {
        KPrintf("set device address failed\n");
        return ret;
    }
  
    
    UsbHcdFreePipe(device->hcd,device->PipeEp0Out);
    UsbHcdFreePipe(device->hcd,device->PipeEp0In);
    
   
    Ep0OutDesc.wMaxPacketSize = device->DevDesc.bMaxPacketSize0;
    Ep0InDesc.wMaxPacketSize = device->DevDesc.bMaxPacketSize0;
    
    
    UsbHcdAllocPipe(device->hcd, &device->PipeEp0Out, device, &Ep0OutDesc);
    UsbHcdAllocPipe(device->hcd, &device->PipeEp0In, device, &Ep0InDesc);
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("get device descriptor length %d\n",
                                DevDesc->bLength));
    
    
    ret = UsbhGetDescriptor(device, USB_DESC_TYPE_DEVICE, (void*)DevDesc, DevDesc->bLength);
    if(ret != EOK)
    {
        KPrintf("get full device descriptor failed\n");
        return ret;
    }

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("Vendor ID 0x%x\n", DevDesc->idVendor));
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("Product ID 0x%x\n", DevDesc->idProduct));

   
    ret = UsbhGetDescriptor(device, USB_DESC_TYPE_CONFIGURATION, &CfgDesc, 18);
    if(ret != EOK)
    {
        KPrintf("get configuration descriptor head failed\n");
        return ret;
    }

 
    device->CfgDesc = (UcfgDescPointer)x_malloc(CfgDesc.wTotalLength);
    memset(device->CfgDesc, 0, CfgDesc.wTotalLength);

  
    ret = UsbhGetDescriptor(device, USB_DESC_TYPE_CONFIGURATION, 
        device->CfgDesc, CfgDesc.wTotalLength);
    if(ret != EOK)
    {
        KPrintf("get full configuration descriptor failed\n");
        return ret;
    }

    
    ret = UsbhSetConfigure(device, 1);
    if(ret != EOK) 
    {
        return ret;
    }
    for(i=0; i<device->CfgDesc->bNumInterfaces; i++)
    {        
       
        ret = UsbhGetInterfaceDescriptor(device->CfgDesc, i, &IntfDesc);
        if(ret != EOK)
        {
            KPrintf("usb_get_interface_descriptor error\n");
            return -ERROR;
        }

        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("interface class 0x%x, subclass 0x%x\n", 
                                    IntfDesc->bInterfaceClass,
                                    IntfDesc->bInterfaceSubClass));
        
        for(EpIndex = 0; EpIndex < IntfDesc->bNumEndpoints; EpIndex++)
        {
            UsbhGetEndpointDescriptor(IntfDesc, EpIndex, &EpDesc);
            if(EpDesc != NONE)
            {
                if(UsbHcdAllocPipe(device->hcd, &pipe, device, EpDesc) != EOK)
                {
                    KPrintf("alloc pipe failed\n");
                    return ERROR;
                }
                usb_instance_add_pipe(device,pipe);
            }
            else
            {
                KPrintf("get endpoint desc failed\n");
                return ERROR;
            }
        }
        
        drv = UsbhClassDriverFind(IntfDesc->bInterfaceClass, 
            IntfDesc->bInterfaceSubClass);
        
        if(drv != NONE)
        {
           
            device->intf[i] = (struct uhintf*)x_malloc(sizeof(struct uhintf));
            device->intf[i]->drv = drv;
            device->intf[i]->device = device;
            device->intf[i]->IntfDesc = IntfDesc;
            device->intf[i]->UserData = NONE;

      
            ret = UsbhClassDriverEnable(drv, (void*)device->intf[i]);
            if(ret != EOK)
            {
                KPrintf("interface %d run class driver error\n", i);
            }
        }
        else
        {
            KPrintf("find usb device driver failed\n");
            continue;
        }
    }
    
    return EOK;
}


/**
 * Detach a connected device
 * 
 * @param device pointer to a uinstance object
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhDetachInstance(UinstPointer device)
{
    int i = 0;
    DoubleLinklistType * l;
    if(device == NONE) 
    {
        KPrintf("no usb instance to detach\n");
        return -ERROR;
    }
    
    
    if (device->CfgDesc) {
        for (i = 0; i < device->CfgDesc->bNumInterfaces; i++)
        {
            if (device->intf[i] == NONE) continue;
            if (device->intf[i]->drv == NONE) continue;

            CHECK(device->intf[i]->device == device);

            SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("free interface instance %d\n", i));
            UsbhClassDriverDisable(device->intf[i]->drv, (void*)device->intf[i]);
            x_free(device->intf[i]);
        }
        x_free(device->CfgDesc);
    }
    
    UsbHcdFreePipe(device->hcd,device->PipeEp0Out);
    UsbHcdFreePipe(device->hcd,device->PipeEp0In);
    
    while(device->pipe.node_next!= &device->pipe)
    {
        l = device->pipe.node_next;
        DoubleLinkListRmNode(l);
        UsbHcdFreePipe(device->hcd,SYS_DOUBLE_LINKLIST_ENTRY(l,struct upipe,list));
    }
    memset(device, 0, sizeof(struct uinstance));
    
    return EOK;
}


/**
 * Get device descriptor of a connected device
 * 
 * @param device pointer to a uinstance object
 * @param type device type
 * @param buffer buffer to store the descriptor
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhGetDescriptor(UinstPointer device, uint8 type, void* buffer, 
    int nbytes)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
    
    NULL_PARAM_CHECK(device);

    setup.RequestType = USB_REQ_TYPE_DIR_IN | USB_REQ_TYPE_STANDARD | 
        USB_REQ_TYPE_DEVICE;
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wIndex = 0;
    setup.wLength = nbytes;
    setup.wValue = type << 8;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) == 8)
    {
        if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, buffer, nbytes, timeout) == nbytes)
        {
            if(UsbHcdPipeXfer(device->hcd, device->PipeEp0Out, NONE, 0, timeout) == 0)
            {
                return EOK;
            }
        }
    }
    return ERROR;
}


/**
 * Set address of a connected device
 * 
 * @param device pointer to a uinstance object
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhSetAddress(UinstPointer device)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
    
    NULL_PARAM_CHECK(device);

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("usb_set_address\n"));

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_STANDARD | 
        USB_REQ_TYPE_DEVICE;
    setup.bRequest = USB_REQ_SET_ADDRESS;
    setup.wIndex = 0;
    setup.wLength = 0;
    setup.wValue = device->index;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return ERROR;
    }
    if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, NONE, 0, timeout) == 0)
    {
        device->address = device->index;
    }

    return EOK;
}


/**
 * Set configuration of a connected device
 * 
 * @param device pointer to a uinstance object
 * @param config value of configuration
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhSetConfigure(UinstPointer device, int config)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;

    
    NULL_PARAM_CHECK(device);

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_STANDARD | 
        USB_REQ_TYPE_DEVICE;
    setup.bRequest = USB_REQ_SET_CONFIGURATION;
    setup.wIndex = 0;
    setup.wLength = 0;
    setup.wValue = config;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return ERROR;
    }
    if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, NONE, 0, timeout) != 0)
    {
        return ERROR;
    }
    return EOK;    
}


/**
 * Set interface of a connected device
 * 
 * @param device pointer to a uinstance object
 * @param intf interface number
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhSetInterface(UinstPointer device, int intf)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;

  
    NULL_PARAM_CHECK(device);

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_STANDARD | 
        USB_REQ_TYPE_INTERFACE;
    setup.bRequest = USB_REQ_SET_INTERFACE;
    setup.wIndex = 0;
    setup.wLength = 0;
    setup.wValue = intf;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return ERROR;
    }
    
    return EOK;    
}


/**
 * Clear feature of a given endpoint
 * 
 * @param device pointer to a uinstance object
 * @param endpoint endpoint number
 * @param feature feature number
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhClearFeature(UinstPointer device, int endpoint, int feature)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;

    
    NULL_PARAM_CHECK(device);

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_STANDARD | 
        USB_REQ_TYPE_ENDPOINT;
    setup.bRequest = USB_REQ_CLEAR_FEATURE;
    setup.wIndex = endpoint;
    setup.wLength = 0;
    setup.wValue = feature;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) != 8)
    {
        return ERROR;
    }
    
    return EOK;    
}


/**
 * Get interface descriptor of a connected device
 * 
 * @param CfgDesc pointer to a UconfigDescriptor object
 * @param num interface number
 * @param IntfDesc pointer to a UinterfaceDescriptor object
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhGetInterfaceDescriptor(UcfgDescPointer CfgDesc, int num, 
    UintfDescPointer* IntfDesc)
{
    uint64 ptr, depth = 0;
    UdescPointer desc;

 
    NULL_PARAM_CHECK(CfgDesc);

    ptr = (uint64)CfgDesc + CfgDesc->bLength;
    while(ptr < (uint64)CfgDesc + CfgDesc->wTotalLength)
    {
        if(depth++ > 0x20) 
        {
            *IntfDesc = NONE;        
            return -EPIO;
        }
        desc = (UdescPointer)ptr;
        if(desc->type == USB_DESC_TYPE_INTERFACE)
        {
            if(((UintfDescPointer)desc)->bInterfaceNumber == num)
            {
                *IntfDesc = (UintfDescPointer)desc;

                SYS_KDEBUG_LOG(SYS_DEBUG_USB,
                             ("usb_get_interface_descriptor: %d\n", num));                
                return EOK;
            }
        }    
        ptr = (uint64)desc + desc->bLength;
    }

    KPrintf("usb_get_interface_descriptor %d failed\n", num);
    return -EPIO;
}


/**
 * Get descriptor of an interface endpoint
 * 
 * @param IntfDesc pointer to an UinterfaceDescriptor object
 * @param num endpoint number
 * @param EpDesc pointer to an UendpointDescriptor object pointer
 * 
 * @return 0 on success, error code on failure
 */
x_err_t UsbhGetEndpointDescriptor(UintfDescPointer IntfDesc, int num, 
    UepDescPointer* EpDesc)
{
    int count = 0, depth = 0;
    uint64 ptr;    
    UdescPointer desc;

   
    NULL_PARAM_CHECK(IntfDesc);
    CHECK(num < IntfDesc->bNumEndpoints);
    *EpDesc = NONE;

    ptr = (uint64)IntfDesc + IntfDesc->bLength;
    while(count < IntfDesc->bNumEndpoints)
    {
        if(depth++ > 0x20) 
        {
            *EpDesc = NONE;            
            return -EPIO;
        }
        desc = (UdescPointer)ptr;        
        if(desc->type == USB_DESC_TYPE_ENDPOINT)
        {
            if(num == count) 
            {
                *EpDesc = (UepDescPointer)desc;

                SYS_KDEBUG_LOG(SYS_DEBUG_USB,
                             ("usb_get_endpoint_descriptor: %d\n", num));
                return EOK;
            }
            else count++;
        }
        ptr = (uint64)desc + desc->bLength;
    }

    KPrintf("usb_get_endpoint_descriptor %d failed\n", num);
    return -EPIO;
}

/**
 * Transfer data on a specified endpoint (pipe)
 * 
 * @param hcd pointer to a uhcd object
 * @param pipe pointer to a upipe object
 * @param buffer buffer to store data to be sent/received
 * @param nbytes number of bytes to be sent/received
 * @param timeout maximum timeout
 * 
 * @return number of bytes sent/received
 */
int UsbHcdPipeXfer(UhcdPointer hcd, upipe_t pipe, void* buffer, int nbytes, int timeout)
{
    x_size_t RemainSize;
    x_size_t SendSize;
    RemainSize = nbytes;
    uint8 * pbuffer = (uint8 *)buffer;
    do
    {
        SYS_KDEBUG_LOG(SYS_DEBUG_USB,("pipe transform remain size,: %d\n", RemainSize));
        SendSize = (RemainSize > pipe->ep.wMaxPacketSize) ? pipe->ep.wMaxPacketSize : RemainSize;
        if(hcd->ops->pipe_xfer(pipe, USBH_PID_DATA, pbuffer, SendSize, timeout) == SendSize)
        {
            RemainSize -= SendSize;
            pbuffer += SendSize;
        }
        else
        {
            return 0;
        }
    }while(RemainSize > 0);
    return nbytes;
}