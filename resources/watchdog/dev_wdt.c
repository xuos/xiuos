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
* @file dev_wdt.c
* @brief register wdt dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_wdt.h>
#include <dev_wdt.h>

static DoubleLinklistType wdtdev_linklist;

/*Create the wdt device linklist*/
static void WdtDeviceLinkInit()
{
    InitDoubleLinkList(&wdtdev_linklist);
}

HardwareDevType WdtDeviceFind(const char *dev_name)
{
    NULL_PARAM_CHECK(dev_name);

    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &wdtdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if (!strcmp(device->dev_name, dev_name)) {
            return device;
        }
    }

    KPrintf("WdtDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int WdtDeviceRegister(struct WdtHardwareDevice *wdt_device, const char *device_name)
{
    NULL_PARAM_CHECK(wdt_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;
    static x_bool DevLinkFlag = RET_FALSE;

    if (DevLinkFlag) {
        WdtDeviceLinkInit();
        DevLinkFlag = RET_TRUE;
    }

    if (DEV_INSTALL != wdt_device->haldev.dev_state) {
        strncpy(wdt_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        wdt_device->haldev.dev_type = TYPE_WDT_DEV;
        wdt_device->haldev.dev_state = DEV_INSTALL;

        wdt_device->haldev.dev_done = (struct HalDevDone *)wdt_device->dev_done;

        DoubleLinkListInsertNodeAfter(&wdtdev_linklist, &(wdt_device->haldev.dev_link));
    } else {
        KPrintf("WdtDeviceRegister device has been register state%u\n", wdt_device->haldev.dev_state);        
    }

    return ret;
}

int WdtDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("WdtDeviceAttachToBus find wdt bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_WDT_BUS == bus->bus_type) {
        device = WdtDeviceFind(dev_name);
        if (NONE == device) {
            KPrintf("WdtDeviceAttachToBus find wdt device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_WDT_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("WdtDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
