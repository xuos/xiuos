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
* @file dev_hwtimer.c
* @brief register hwtimer dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_hwtimer.h>
#include <dev_hwtimer.h>

static DoubleLinklistType hwtimerdev_linklist;

/*Create the hwtimer device linklist*/
static void HwtimerDeviceLinkInit()
{
    InitDoubleLinkList(&hwtimerdev_linklist);
}

HardwareDevType HwtimerDeviceFind(const char *dev_name)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &hwtimerdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if (!strcmp(device->dev_name, dev_name)) {
            return device;
        }
    }

    KPrintf("HwtimerDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int HwtimerDeviceRegister(struct HwtimerHardwareDevice *hwtimer_device, void *hwtimer_param, const char *device_name)
{
    NULL_PARAM_CHECK(hwtimer_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        HwtimerDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != hwtimer_device->haldev.dev_state) {
        strncpy(hwtimer_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        hwtimer_device->haldev.dev_type = TYPE_HWTIMER_DEV;
        hwtimer_device->haldev.dev_state = DEV_INSTALL;

        hwtimer_device->haldev.dev_done = (struct HalDevDone *)hwtimer_device->dev_done;

        hwtimer_device->haldev.private_data = hwtimer_param;

        DoubleLinkListInsertNodeAfter(&hwtimerdev_linklist, &(hwtimer_device->haldev.dev_link));
    } else {
        KPrintf("HwtimerDeviceRegister device has been register state%u\n", hwtimer_device->haldev.dev_state);        
    }

    return ret;
}

int HwtimerDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("HwtimerDeviceAttachToBus find hwtimer bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_HWTIMER_BUS == bus->bus_type) {
        device = HwtimerDeviceFind(dev_name);
        if (NONE == device) {
            KPrintf("HwtimerDeviceAttachToBus find hwtimer device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_HWTIMER_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("HwtimerDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
