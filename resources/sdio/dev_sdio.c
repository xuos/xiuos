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
* @file dev_sdio.c
* @brief register sdio dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_sdio.h>
#include <dev_sdio.h>

static DoubleLinklistType sdiodev_linklist;

/*Create the sdio device linklist*/
static void SdioDeviceLinkInit()
{
    InitDoubleLinkList(&sdiodev_linklist);
}

HardwareDevType SdioDeviceFind(const char *dev_name)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &sdiodev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if (!strcmp(device->dev_name, dev_name)) {
            return device;
        }
    }

    KPrintf("SdioDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int SdioDeviceRegister(struct SdioHardwareDevice *sdio_device, const char *device_name)
{
    NULL_PARAM_CHECK(sdio_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        SdioDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != sdio_device->haldev.dev_state) {
        strncpy(sdio_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        sdio_device->haldev.dev_type = TYPE_SDIO_DEV;
        sdio_device->haldev.dev_state = DEV_INSTALL;

        sdio_device->haldev.dev_done = (struct HalDevDone *)sdio_device->dev_done;
        sdio_device->haldev.private_data = sdio_device->private_data;

        DoubleLinkListInsertNodeAfter(&sdiodev_linklist, &(sdio_device->haldev.dev_link));
    } else {
        KPrintf("SdioDeviceRegister device has been register state%u\n", sdio_device->haldev.dev_state);        
    }

    return ret;
}

int SdioDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SdioDeviceAttachToBus find sdio bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_SDIO_BUS == bus->bus_type) {
        device = SdioDeviceFind(dev_name);
        if (NONE == device) {
            KPrintf("SdioDeviceAttachToBus find sdio device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_SDIO_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("SdioDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
