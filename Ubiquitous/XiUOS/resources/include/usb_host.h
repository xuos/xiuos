/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-3-12     Yi Qiu      first version
 */

/**
* @file usb_host.h
* @brief define usb host function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: usb_host.h
Description: define usb host function and struct
Others: take RT-Thread v4.0.2/components/drivers/include/drivers/usb_host.h for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. define usb host configure, function and struct
2. support bus driver framework 
*************************************************/

#ifndef USB_HOST_H
#define USB_HOST_H

#include <xiuos.h>

#include <usb_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_MAX_DEVICE                       0x20
#define USB_MAX_INTERFACE               0x08
#define USB_HUB_PORT_NUM             0x04
#define SIZEOF_USB_REQUEST            0x08

#define DEV_STATUS_IDLE                    0x00
#define DEV_STATUS_BUSY                   0x01
#define DEV_STATUS_ERROR                0x02

#define UPIPE_STATUS_OK                      0x00
#define UPIPE_STATUS_STALL                0x01
#define UPIPE_STATUS_ERROR              0x02

#define USBH_PID_SETUP                  0x00
#define USBH_PID_DATA                      0x01

struct uhcd;
struct uhintf;
struct uhub;
struct upipe;

struct UclassDriver
{
    DoubleLinklistType list;
    int ClassCode;
    int subclass_code;
    
    x_err_t (*enable)(void* arg);
    x_err_t (*disable)(void* arg);
    
    void* UserData;
};
typedef struct UclassDriver* UcdPointer;

struct uprotocal
{
    DoubleLinklistType list;
    int ProId;
    
    x_err_t (*init)(void* arg);
    x_err_t (*callback)(void* arg);    
};
typedef struct uprotocal* UprotocalPointer;

struct uinstance
{
    struct UdeviceDescriptor DevDesc;
    UcfgDescPointer CfgDesc;
    struct uhcd *hcd;

    struct upipe * PipeEp0Out;
    struct upipe * PipeEp0In;
    DoubleLinklistType pipe;

    uint8 status;
    uint8 type;
    uint8 index;
    uint8 address;    
    uint8 speed;
    uint8 MaxPacketSize;    
    uint8 port;

    struct uhub* parent_hub;
    struct uhintf* intf[USB_MAX_INTERFACE];        
};
typedef struct uinstance* UinstPointer;

struct uhintf
{
    struct uinstance* device;
    UintfDescPointer IntfDesc;

    UcdPointer drv;
    void *UserData;
};

struct upipe
{
    DoubleLinklistType list;
    uint8 pipe_index;
    uint32 status;
    struct UendpointDescriptor ep;
    UinstPointer inst;
    FuncCallback callback;
    void* UserData;
};
typedef struct upipe* upipe_t;

struct uhub
{
    struct UhubDescriptor HubDesc;
    uint8 NumPorts;
    uint32 PortStatus[USB_HUB_PORT_NUM];
    struct uinstance* child[USB_HUB_PORT_NUM];        

    x_bool IsRoothub;

    uint8 buffer[8];    
    struct uinstance* self;
    struct uhcd *hcd;
};    
typedef struct uhub* UhubPointer;

struct uhcd_ops
{
    x_err_t    (*reset_port)   (uint8 port);
    int         (*pipe_xfer)    (upipe_t pipe, uint8 token, void* buffer, int nbytes, int timeout);
    x_err_t    (*open_pipe)    (upipe_t pipe);
    x_err_t    (*close_pipe)   (upipe_t pipe);  
};
typedef struct uhcd_ops* uhcd_ops_t;
struct uhcd
{
    uhcd_ops_t ops;
    uint8 NumPorts;
    UhubPointer roothub; 
};
typedef struct uhcd* UhcdPointer;

enum uhost_msg_type
{
    USB_MSG_CONNECT_CHANGE,
    USB_MSG_CALLBACK,
};
typedef enum uhost_msg_type uhost_msg_type;

struct UhostMsg
{
    uhost_msg_type type; 
    union
    {
        struct uhub* hub;
        struct 
        {
            FuncCallback function;
            void *context;
        }cb;
    }content;
};
typedef struct UhostMsg* uhost_msg_t;

/* usb host system interface */
x_err_t UsbHostInit(struct uhcd *hcd);
void UsbhHubInit(struct uhcd *hcd);

/* usb host core interface */
struct uinstance* UsbhAllocInstance(UhcdPointer uhcd);
x_err_t UsbhAttatchInstance(struct uinstance* device);
x_err_t UsbhDetachInstance(struct uinstance* device);
x_err_t UsbhGetDescriptor(struct uinstance* device, uint8 type, void* buffer, int nbytes);
x_err_t UsbhSetConfigure(struct uinstance* device, int config);
x_err_t UsbhSetAddress(struct uinstance* device);
x_err_t UsbhSetInterface(struct uinstance* device, int intf);
x_err_t UsbhClearFeature(struct uinstance* device, int endpoint, int feature);
x_err_t UsbhGetInterfaceDescriptor(UcfgDescPointer CfgDesc, int num, UintfDescPointer* IntfDesc);
x_err_t UsbhGetEndpointDescriptor(UintfDescPointer IntfDesc, int num, UepDescPointer* EpDesc);

/* usb class driver interface */
x_err_t UsbhClassDriverInit(void);
x_err_t UsbhClassDriverRegister(UcdPointer drv);
x_err_t UsbhClassDriverUnregister(UcdPointer drv);
x_err_t UsbhClassDriverEnable(UcdPointer drv, void* args);
x_err_t UsbhClassDriverDisable(UcdPointer drv, void* args);
UcdPointer UsbhClassDriverFind(int ClassCode, int subclass_code);

/* usb class driver implement */
UcdPointer UsbhClassDriverHub(void);
UcdPointer UsbhClassDriverStorage(void);

/* usb hub interface */
x_err_t UsbhHubGetDescriptor(struct uinstance* device, uint8 *buffer, 
    x_size_t size);
x_err_t UsbhHubGetStatus(struct uinstance* device, uint32* buffer);
x_err_t UsbhHubGetPortStatus(UhubPointer uhub, uint16 port, 
    uint32* buffer);
x_err_t UsbhHubClearPortFeature(UhubPointer uhub, uint16 port, 
    uint16 feature);
x_err_t UsbhHubSetPortFeature(UhubPointer uhub, uint16 port, 
    uint16 feature);
x_err_t UsbhHubResetPort(UhubPointer uhub, uint16 port);
x_err_t UsbhEventSignal(struct UhostMsg* msg);

void UsbhRootHubConnectHandler(struct uhcd *hcd, uint8 port, x_bool isHS);
void UsbhRootHubDisconnectHandler(struct uhcd *hcd, uint8 port);

/* usb host controller driver interface */
static __inline x_err_t usb_instance_add_pipe(UinstPointer inst, upipe_t pipe)
{
    NULL_PARAM_CHECK(inst);
    NULL_PARAM_CHECK(pipe);
    DoubleLinkListInsertNodeBefore(&inst->pipe, &pipe->list);
    return EOK;
}
static __inline upipe_t UsbInstanceFindPipe(UinstPointer inst,uint8 ep_address)
{
    DoubleLinklistType * l;
    for(l = inst->pipe.node_next;l != &inst->pipe;l = l->node_next)
    {
        if(SYS_DOUBLE_LINKLIST_ENTRY(l,struct upipe,list)->ep.bEndpointAddress == ep_address)
        {
            return SYS_DOUBLE_LINKLIST_ENTRY(l,struct upipe,list);
        }
    }
    return NONE;
}
static __inline x_err_t UsbHcdAllocPipe(UhcdPointer hcd, upipe_t* pipe, UinstPointer inst, UepDescPointer ep)
{
    *pipe = (upipe_t)x_malloc(sizeof(struct upipe));
    if(*pipe == NONE)
    {
        return ERROR;
    }
    memset(*pipe,0,sizeof(struct upipe));
    (*pipe)->inst = inst;
    memcpy(&(*pipe)->ep,ep,sizeof(struct UendpointDescriptor));
    return hcd->ops->open_pipe(*pipe);
}
static __inline void UsbPipeAddCallback(upipe_t pipe, FuncCallback callback)
{
    pipe->callback = callback;
}

static __inline x_err_t UsbHcdFreePipe(UhcdPointer hcd, upipe_t pipe)
{
    NULL_PARAM_CHECK(pipe);
    hcd->ops->close_pipe(pipe);
    x_free(pipe);
    return EOK;
}

int UsbHcdPipeXfer(UhcdPointer hcd, upipe_t pipe, void* buffer, int nbytes, int timeout);
static __inline int UsbHcdSetupXfer(UhcdPointer hcd, upipe_t pipe, UreqPointer setup, int timeout)
{
    return hcd->ops->pipe_xfer(pipe, USBH_PID_SETUP, (void *)setup, 8, timeout);
}

#ifdef __cplusplus
}
#endif

#endif