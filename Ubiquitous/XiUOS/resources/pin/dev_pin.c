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
* @file dev_pin.c
* @brief register pin dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_pin.h>
#include <dev_pin.h>

static DoubleLinklistType pindev_linklist;

/*Create the Pin device linklist*/
static void PinDeviceLinkInit()
{
    InitDoubleLinkList(&pindev_linklist);
}

HardwareDevType PinDeviceFind(const char *dev_name, enum DevType DevType)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &pindev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (DevType == device->dev_type)) {
            return device;
        }
    }

    KPrintf("PinDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int PinDeviceRegister(struct PinHardwareDevice *pin_device, void *pin_param, const char *device_name)
{
    NULL_PARAM_CHECK(pin_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        PinDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != pin_device->haldev.dev_state) {
        strncpy(pin_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        pin_device->haldev.dev_type = TYPE_PIN_DEV;
        pin_device->haldev.dev_state = DEV_INSTALL;

        pin_device->haldev.dev_done = (struct HalDevDone *)pin_device->dev_done;

        pin_device->haldev.private_data = pin_param;

        DoubleLinkListInsertNodeAfter(&pindev_linklist, &(pin_device->haldev.dev_link));
    } else {
        KPrintf("PinDeviceRegister device has been register state%u\n", pin_device->haldev.dev_state);        
    }

    return ret;
}

int PinDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("PinDeviceAttachToBus find Pin bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_PIN_BUS == bus->bus_type) {
        device = PinDeviceFind(dev_name, TYPE_PIN_DEV);
        if (NONE == device) {
            KPrintf("PinDeviceAttachToBus find Pin device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_PIN_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("PinDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}

