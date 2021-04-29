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
* @file dev_rtc.c
* @brief register rtc dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_rtc.h>
#include <dev_rtc.h>

static DoubleLinklistType rtcdev_linklist;

/*Create the rtc device linklist*/
static void RtcDeviceLinkInit()
{
    InitDoubleLinkList(&rtcdev_linklist);
}

HardwareDevType RtcDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &rtcdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("RtcDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int RtcDeviceRegister(struct RtcHardwareDevice *rtc_device, void *rtc_param, const char *device_name)
{
    NULL_PARAM_CHECK(rtc_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        RtcDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != rtc_device->haldev.dev_state) {
        strncpy(rtc_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        rtc_device->haldev.dev_type = TYPE_RTC_DEV;
        rtc_device->haldev.dev_state = DEV_INSTALL;

        rtc_device->haldev.dev_done = (struct HalDevDone *)rtc_device->dev_done;

        rtc_device->haldev.private_data = rtc_param;

        DoubleLinkListInsertNodeAfter(&rtcdev_linklist, &(rtc_device->haldev.dev_link));
    } else {
        KPrintf("RtcDeviceRegister device has been register state%u\n", rtc_device->haldev.dev_state);        
    }

    return ret;
}

int RtcDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("RtcDeviceAttachToBus find rtc bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_RTC_BUS == bus->bus_type) {
        device = RtcDeviceFind(dev_name, TYPE_RTC_DEV);
        if (NONE == device) {
            KPrintf("RtcDeviceAttachToBus find rtc device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_RTC_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("RtcDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
