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
* @file dev_touch.c
* @brief register touch dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_touch.h>
#include <dev_touch.h>

static DoubleLinklistType touchdev_linklist;

/*Create the touch device linklist*/
static void TouchDeviceLinkInit()
{
    InitDoubleLinkList(&touchdev_linklist);
}

HardwareDevType TouchDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &touchdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next){
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("TouchDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int TouchDeviceRegister(struct TouchHardwareDevice *touch_device, void *touch_param, const char *device_name)
{
    NULL_PARAM_CHECK(touch_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    

    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        TouchDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != touch_device->haldev.dev_state) {
        strncpy(touch_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        touch_device->haldev.dev_type = TYPE_TOUCH_DEV;
        touch_device->haldev.dev_state = DEV_INSTALL;

        touch_device->haldev.dev_done = (struct HalDevDone *)touch_device->dev_done;

        touch_device->haldev.private_data = touch_param;

        DoubleLinkListInsertNodeAfter(&touchdev_linklist, &(touch_device->haldev.dev_link));
    } else {
        KPrintf("TouchDeviceRegister device has been register state%u\n", touch_device->haldev.dev_state);        
    }

    return ret;
}

int TouchDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("TouchDeviceAttachToBus find touch bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_TOUCH_BUS == bus->bus_type) {
        device = TouchDeviceFind(dev_name, TYPE_TOUCH_DEV);
        if (NONE == device) {
            KPrintf("TouchDeviceAttachToBus find TOUCH device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_TOUCH_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("TouchDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
