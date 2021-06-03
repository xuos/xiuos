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
* @file driver.c
* @brief support usb host driver function and register configure
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: driver.c
Description: support usb host driver function , init and register
Others: take RT-Thread v4.0.2/components/drivers/usb/usbhost/core/driver.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support usb host driver configure, function and struct
2. support usb bus driver framework 
*************************************************/

#include <xiuos.h>
#include <xs_klist.h>
#include <usb_host.h>

static DoubleLinklistType DriverList;


x_err_t UsbhClassDriverInit(void)
{
    InitDoubleLinkList(&DriverList);

    return EOK;    
}



x_err_t UsbhClassDriverRegister(UcdPointer drv)
{
    if (drv == NONE) return -ERROR;

 
    DoubleLinkListInsertNodeAfter(&DriverList, &(drv->list));
    
    return EOK;    
}


x_err_t UsbhClassDriverUnregister(UcdPointer drv)
{
    NULL_PARAM_CHECK(drv);

   
    DoubleLinkListRmNode(&(drv->list));

    return EOK;
}


x_err_t UsbhClassDriverEnable(UcdPointer drv, void* args)
{
    NULL_PARAM_CHECK(drv);

    if(drv->enable != NONE)
        drv->enable(args);

    return EOK;
}


x_err_t UsbhClassDriverDisable(UcdPointer drv, void* args)
{
    NULL_PARAM_CHECK(drv);

    if(drv->disable != NONE)
        drv->disable(args);

    return EOK;
}



UcdPointer UsbhClassDriverFind(int ClassCode, int subclass_code)
{
    struct SysDoubleLinklistNode *node;
    x_base lock;
    
    if (GetKTaskDescriptor() != NONE)
        lock = CriticalAreaLock();

    for (node = DriverList.node_next; node != &DriverList; node = node->node_next)
    {
        UcdPointer drv = 
            (UcdPointer)SYS_DOUBLE_LINKLIST_ENTRY(node, struct UclassDriver, list);
        if (drv->ClassCode == ClassCode)
        {
            if (GetKTaskDescriptor() != NONE)
                CriticalAreaUnLock(lock);

            return drv;
        }
    }

    if (GetKTaskDescriptor() != NONE)
        CriticalAreaUnLock(lock);

    return NONE;
}