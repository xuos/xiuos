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
* @file hub.c
* @brief support usb hub driver function and configure
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: hub.c
Description: support usb hub driver function , init and irq configure
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/core/hub.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb hub driver configure, function and struct
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>
#include <usb_host.h>

#define USB_THREAD_STACK_SIZE    4096

static int32 UsbMq;
static struct UclassDriver HubDriver;
static struct uhub RootHub;

static x_err_t RootHubCtrl(struct uhcd *hcd, uint16 port, uint8 cmd, void *args)
{
    switch(cmd)
    {
    case RH_GET_PORT_STATUS:
        (*(uint32 *)args) = hcd->roothub->PortStatus[port-1];
        break;
    case RH_SET_PORT_STATUS:
        hcd->roothub->PortStatus[port-1] = (*(uint32 *)args);
        break;
    case RH_CLEAR_PORT_FEATURE:
        switch(((uint64)args))
        {
        case PORT_FEAT_C_CONNECTION:
            hcd->roothub->PortStatus[port-1] &= ~PORT_CCSC;
            break;
        case PORT_FEAT_C_ENABLE:
            hcd->roothub->PortStatus[port-1] &= ~PORT_PESC;
            break;
        case PORT_FEAT_C_SUSPEND:
            hcd->roothub->PortStatus[port-1] &= ~PORT_PSSC;
            break;
        case PORT_FEAT_C_OVER_CURRENT:
            hcd->roothub->PortStatus[port-1] &= ~PORT_POCIC;
            break;
        case PORT_FEAT_C_RESET:
            hcd->roothub->PortStatus[port-1] &= ~PORT_PRSC;
            break;
        }
        break;
    case RH_SET_PORT_FEATURE:
        switch((uint64)args)
        {
        case PORT_FEAT_CONNECTION:
            hcd->roothub->PortStatus[port-1] |= PORT_CCSC;
            break;
        case PORT_FEAT_ENABLE:
            hcd->roothub->PortStatus[port-1] |= PORT_PESC;
            break;
        case PORT_FEAT_SUSPEND:
            hcd->roothub->PortStatus[port-1] |= PORT_PSSC;
            break;
        case PORT_FEAT_OVER_CURRENT:
            hcd->roothub->PortStatus[port-1] |= PORT_POCIC;
            break;
        case PORT_FEAT_RESET:
            hcd->ops->reset_port(port);
            break;
        case PORT_FEAT_POWER:
            break;
        case PORT_FEAT_LOWSPEED:
            break;
        case PORT_FEAT_HIGHSPEED:
            break;
        }
        break;
    default:
        return ERROR;
    }
    return EOK;
} 
void UsbhRootHubConnectHandler(struct uhcd *hcd, uint8 port, x_bool isHS)
{
    struct UhostMsg msg;
    msg.type = USB_MSG_CONNECT_CHANGE;
    msg.content.hub = hcd->roothub;
    hcd->roothub->PortStatus[port - 1] |= PORT_CCS | PORT_CCSC;
    if(isHS)
    {
        hcd->roothub->PortStatus[port - 1] &= ~PORT_LSDA;
    }
    else
    {
        hcd->roothub->PortStatus[port - 1] |= PORT_LSDA;
    }
    UsbhEventSignal(&msg);
}

void UsbhRootHubDisconnectHandler(struct uhcd *hcd, uint8 port)
{
    struct UhostMsg msg;
    msg.type = USB_MSG_CONNECT_CHANGE;
    msg.content.hub = hcd->roothub;
    hcd->roothub->PortStatus[port - 1] |= PORT_CCSC;
    hcd->roothub->PortStatus[port - 1] &= ~PORT_CCS;
    UsbhEventSignal(&msg);
}


x_err_t UsbhHubGetDescriptor(struct uinstance* device, uint8 *buffer, x_size_t nbytes)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
        
   
    NULL_PARAM_CHECK(device);
    
    setup.RequestType = USB_REQ_TYPE_DIR_IN | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DEVICE;
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wIndex = 0;
    setup.wLength = nbytes;
    setup.wValue = USB_DESC_TYPE_HUB << 8;

    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) == 8)
    {
        if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, buffer, nbytes, timeout) == nbytes)
        {
            return EOK;
        }
    } 
    return -RET_FALSE;    
}


x_err_t UsbhHubGetStatus(struct uinstance* device, uint32* buffer)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
    
  
    NULL_PARAM_CHECK(device);

    setup.RequestType = USB_REQ_TYPE_DIR_IN | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DEVICE;
    setup.bRequest = USB_REQ_GET_STATUS;
    setup.wIndex = 0;
    setup.wLength = 4;
    setup.wValue = 0;
    if(UsbHcdSetupXfer(device->hcd, device->PipeEp0Out, &setup, timeout) == 8)
    {
        if(UsbHcdPipeXfer(device->hcd, device->PipeEp0In, buffer, 4, timeout) == 4)
        {
            return EOK;
        }
    }
    return -RET_FALSE;    
}


x_err_t UsbhHubGetPortStatus(UhubPointer hub, uint16 port, uint32* buffer)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
    
 
    NULL_PARAM_CHECK(hub);

 
    if(hub->IsRoothub)
    {
        RootHubCtrl(hub->hcd, port, RH_GET_PORT_STATUS, 
            (void*)buffer);
        return EOK;
    }

    setup.RequestType = USB_REQ_TYPE_DIR_IN | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_OTHER;
    setup.bRequest = USB_REQ_GET_STATUS;
    setup.wIndex = port;
    setup.wLength = 4;
    setup.wValue = 0;

    if(UsbHcdSetupXfer(hub->hcd, hub->self->PipeEp0Out, &setup, timeout) == 8)
    {
        if(UsbHcdPipeXfer(hub->hcd, hub->self->PipeEp0In, buffer, 4, timeout) == 4)
        {
            return EOK;
        }
    }
    return -RET_FALSE;        
}


x_err_t UsbhHubClearPortFeature(UhubPointer hub, uint16 port, uint16 feature)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
        
    
    NULL_PARAM_CHECK(hub);
    


    if(hub->IsRoothub)
    {
        RootHubCtrl(hub->hcd, port, RH_CLEAR_PORT_FEATURE, 
            (void*)(uint64)feature);
        return EOK;
    }

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_CLASS | 
        USB_REQ_TYPE_OTHER;
    setup.bRequest = USB_REQ_CLEAR_FEATURE;
    setup.wIndex = port;
    setup.wLength = 0;
    setup.wValue = feature;

    if(UsbHcdSetupXfer(hub->hcd, hub->self->PipeEp0Out, &setup, timeout) == 8)
    {
        return EOK;
    }
    return -RET_FALSE;    
}


x_err_t UsbhHubSetPortFeature(UhubPointer hub, uint16 port, 
    uint16 feature)
{
    struct urequest setup;
    int timeout = USB_TIMEOUT_BASIC;
        
    
    NULL_PARAM_CHECK(hub);

   
    if(hub->IsRoothub)
    {
        RootHubCtrl(hub->hcd, port, RH_SET_PORT_FEATURE, 
            (void*)(uint64)feature);
        return EOK;
    }

    setup.RequestType = USB_REQ_TYPE_DIR_OUT | USB_REQ_TYPE_CLASS | 
        USB_REQ_TYPE_OTHER;
    setup.bRequest = USB_REQ_SET_FEATURE;
    setup.wIndex = port;
    setup.wLength = 0;
    setup.wValue = feature;

    if(UsbHcdSetupXfer(hub->hcd, hub->self->PipeEp0Out, &setup, timeout) == 8)
    {
        return EOK;
    }
    else return -RET_FALSE;        
}


x_err_t UsbhHubResetPort(UhubPointer hub, uint16 port)
{
    x_err_t ret;
    uint32 pstatus;
    
   
    NULL_PARAM_CHECK(hub);
    
    DelayKTask(50);

   
    ret = UsbhHubSetPortFeature(hub, port, PORT_FEAT_RESET);
    if(ret != EOK) return ret;

    while(RET_TRUE)
    {
        ret = UsbhHubGetPortStatus(hub, port, &pstatus);
        if(!(pstatus & PORT_PRS)) break;
    }
    
   
    ret = UsbhHubClearPortFeature(hub, port, PORT_FEAT_C_RESET);    
    if(ret != EOK) return ret;

    DelayKTask(50);    

    return EOK;
}


x_err_t UsbhHubPortDebounce(UhubPointer hub, uint16 port)
{
    x_err_t ret;
    int i = 0, times = 20;
    uint32 pstatus;
    x_bool connect = RET_TRUE;
    int delayticks = USB_DEBOUNCE_TIME / times;
    if (delayticks < 1)
        delayticks = 1;

   
    NULL_PARAM_CHECK(hub);

    for(i=0; i<times; i++)
    {
        ret = UsbhHubGetPortStatus(hub, port, &pstatus);
        if(ret != EOK) return ret;
            
        if(!(pstatus & PORT_CCS)) 
        {
            connect = RET_FALSE;
            break;
        }
        
        DelayKTask(delayticks);
    }        

    if(connect) return EOK;
    else return -ERROR;
}


static x_err_t UsbhHubPortChange(UhubPointer hub)
{
    int i;
    x_bool reconnect;

   
    NULL_PARAM_CHECK(hub);

 
    for (i = 0; i < hub->NumPorts; i++)
    {
        x_err_t ret;
        struct uinstance* device;
        uint32 pstatus = 0;

        reconnect = RET_FALSE;
        
      
        ret = UsbhHubGetPortStatus(hub, i + 1, &pstatus);
        if(ret != EOK) continue;

        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("port %d status 0x%x\n", i + 1, pstatus));

        
        if (pstatus & PORT_CCSC) 
        {        
           
            UsbhHubClearPortFeature(hub, i + 1, PORT_FEAT_C_CONNECTION);
            reconnect = RET_TRUE;
        }

        if(pstatus & PORT_PESC)
        {
            UsbhHubClearPortFeature(hub, i + 1, PORT_FEAT_C_ENABLE);            
            reconnect = RET_TRUE;
        }
        
        if(reconnect)
        {            
            if(hub->child[i] != NONE && hub->child[i]->status != DEV_STATUS_IDLE) 
                UsbhDetachInstance(hub->child[i]);
            
            ret = UsbhHubPortDebounce(hub, i + 1);
            if(ret != EOK) continue;
            
            
            device = UsbhAllocInstance(hub->hcd);
            if(device == NONE) break;
            
            
            device->speed = (pstatus & PORT_LSDA) ? 1 : 0;
            device->parent_hub = hub;    
            device->hcd = hub->hcd;
            device->port = i + 1;
            hub->child[i] = device;

            
            UsbhHubResetPort(hub, i + 1);
            
           
            UsbhAttatchInstance(device); 
        }
    }

    return EOK;
}


static void UsbhHubIrq(void* context)
{
    upipe_t pipe;
    UhubPointer hub;
    int timeout = USB_TIMEOUT_BASIC;
    
    if(NONE == context) {
		KPrintf("PARAM CHECK FAILED ...%s %d contex is NULL.\n",__FUNCTION__,__LINE__); 
		return;
	}

    pipe = (upipe_t)context;
    hub = (UhubPointer)pipe->UserData;

    if(pipe->status != UPIPE_STATUS_OK)
    {
        SYS_KDEBUG_LOG(SYS_DEBUG_USB,("hub irq error\n"));
        return;
    }
    
    UsbhHubPortChange(hub);

    SYS_KDEBUG_LOG(SYS_DEBUG_USB,("hub int xfer...\n"));

    
    CHECK(pipe->inst->hcd);
    
    UsbHcdPipeXfer(hub->self->hcd, pipe, hub->buffer, pipe->ep.wMaxPacketSize, timeout);
}



static x_err_t UsbhHubEnable(void *arg)
{
    int i = 0;
    x_err_t ret = EOK;
    UepDescPointer EpDesc = NONE;
    UhubPointer hub;
    struct uinstance* device;
    struct uhintf* intf = (struct uhintf*)arg;
    upipe_t pipe_in = NONE;
    int timeout = USB_TIMEOUT_LONG;
    
    NULL_PARAM_CHECK(intf);
    
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("usbh_hub_run\n"));

    
    device = intf->device;

  
    hub = x_malloc(sizeof(struct uhub));
    memset(hub, 0, sizeof(struct uhub));
    
  
    intf->UserData = (void*)hub;

 
    ret = UsbhHubGetDescriptor(device, (uint8*)&hub->HubDesc, 8);
    if(ret != EOK)
    {
        KPrintf("get hub descriptor failed\n");
        return -ERROR;        
    }

  
    ret = UsbhHubGetDescriptor(device, (uint8*)&hub->HubDesc, 
        hub->HubDesc.length);
    if(ret != EOK)
    {
        KPrintf("get hub descriptor again failed\n");
        return -ERROR;        
    }    

   
    hub->NumPorts = hub->HubDesc.NumPorts;
    hub->hcd = device->hcd;
    hub->self = device;


    for (i = 0; i < hub->NumPorts; i++)
    {

        UsbhHubSetPortFeature(hub, i + 1, PORT_FEAT_POWER);
        DelayKTask(hub->HubDesc.PwronToGood

            * 2 * TICK_PER_SECOND / 1000 );
    }

    if(intf->IntfDesc->bNumEndpoints != 1) 
        return -ERROR;

   
    UsbhGetEndpointDescriptor(intf->IntfDesc, 0, &EpDesc);
    if(EpDesc == NONE)
    {
        KPrintf("usb_get_endpoint_descriptor error\n");
        return -ERROR;
    }

      
    if( USB_EP_ATTR(EpDesc->bmAttributes) == USB_EP_ATTR_INT)
    {
 
        if(EpDesc->bEndpointAddress & USB_DIR_IN)
        {    
   
            pipe_in = UsbInstanceFindPipe(device,EpDesc->bEndpointAddress);
            if(pipe_in == NONE)
            {
                return ERROR;
            }
            UsbPipeAddCallback(pipe_in,UsbhHubIrq);
        }
        else return -ERROR;
    }


    NULL_PARAM_CHECK(device->hcd);
    pipe_in->UserData = hub;
    UsbHcdPipeXfer(hub->hcd, pipe_in, hub->buffer, 
        pipe_in->ep.wMaxPacketSize, timeout);
    return EOK;
}


static x_err_t UsbhHubDisable(void* arg)
{
    int i;
    UhubPointer hub;
    struct uhintf* intf = (struct uhintf*)arg;

  
    NULL_PARAM_CHECK(intf);

    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("usbh_hub_stop\n"));
    hub = (UhubPointer)intf->UserData;

    for(i=0; i<hub->NumPorts; i++)
    {
        if(hub->child[i] != NONE)
            UsbhDetachInstance(hub->child[i]);
    }
    
    if(hub != NONE) x_free(hub);
    if(intf != NONE) x_free(intf);

    return EOK;
}


UcdPointer UsbhClassDriverHub(void)
{    
    HubDriver.ClassCode = USB_CLASS_HUB;
    
    HubDriver.enable = UsbhHubEnable;
    HubDriver.disable = UsbhHubDisable;

    return &HubDriver;
}


static void UsbhHubThreadEntry(void* parameter)
{    
    while(RET_TRUE)
    {    
        struct UhostMsg msg;
        

      
        if(KMsgQueueRecv(UsbMq, &msg, sizeof(struct UhostMsg), WAITING_FOREVER) 

            != EOK ) continue;

        
        switch (msg.type)
        {        
        case USB_MSG_CONNECT_CHANGE:
            UsbhHubPortChange(msg.content.hub);
            break;
        case USB_MSG_CALLBACK:
            
            msg.content.cb.function(msg.content.cb.context);
            break;
        default:
            break;
        }            
    }
}


x_err_t UsbhEventSignal(struct UhostMsg* msg)
{
    NULL_PARAM_CHECK(msg);


   
    KMsgQueueSend(UsbMq, (void*)msg, sizeof(struct UhostMsg));


    return EOK;
}


void UsbhHubInit(UhcdPointer hcd)
{

    int32 thread;
 
    RootHub.IsRoothub = RET_TRUE;
    hcd->roothub = &RootHub;
    RootHub.hcd = hcd;
    RootHub.NumPorts = hcd->NumPorts;
    
    UsbMq = KCreateMsgQueue( 32, 16);
    
   
    thread = KTaskCreate("usbh", UsbhHubThreadEntry, NONE, 
        USB_THREAD_STACK_SIZE, 8);
    if(thread >= 0)
    {
        
        StartupKTask(thread);

    }
}
