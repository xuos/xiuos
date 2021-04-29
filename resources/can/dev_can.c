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
* @file dev_can.c
* @brief register can dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_can.h>
#include <dev_can.h>

static DoubleLinklistType candev_linklist;

/*Create the CAN device linklist*/
static void CanDeviceLinkInit()
{
    InitDoubleLinkList(&candev_linklist);
}

/*Find the register CAN device*/
HardwareDevType CanDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &candev_linklist;
    for (node = head->node_next; node != head; node = node->node_next) {    
   
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("CanDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

/*Register the CAN device*/
int CanDeviceRegister(struct CanHardwareDevice *can_device, void *can_param, const char *device_name)
{
    NULL_PARAM_CHECK(can_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        CanDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != can_device->haldev.dev_state) {
        strncpy(can_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        can_device->haldev.dev_type = TYPE_CAN_DEV;
        can_device->haldev.dev_state = DEV_INSTALL;

        can_device->haldev.dev_done = (struct HalDevDone *)can_device->dev_done;

        can_device->haldev.private_data = can_param;

        DoubleLinkListInsertNodeAfter(&candev_linklist, &(can_device->haldev.dev_link));
    } else {
        KPrintf("CanDeviceRegister device has been register state%u\n", can_device->haldev.dev_state);        
    }

    return ret;
}

/*Register the CAN Device to the CAN BUS*/
int CanDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("CanDeviceAttachToBus find can bus error!name %s\n", bus_name);
        return ERROR;
    }

    if (TYPE_CAN_BUS == bus->bus_type) {
        device = CanDeviceFind(dev_name, TYPE_CAN_DEV);

        if (NONE == device) {
            KPrintf("CanDeviceAttachToBus find can device error!name %s\n", dev_name);
            return ERROR;
        }
    
        if (TYPE_CAN_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);

            if (EOK != ret) {
                KPrintf("CanDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
