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
* @file dev_lcd.c
* @brief register lcd dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_lcd.h>
#include <dev_lcd.h>

static DoubleLinklistType lcddev_linklist;

/*Create the lcd device linklist*/
static void LcdDeviceLinkInit()
{
    InitDoubleLinkList(&lcddev_linklist);
}

HardwareDevType LcdDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &lcddev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("LcdDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int LcdDeviceRegister(struct LcdHardwareDevice *lcd_device, void *lcd_param, const char *device_name)
{
    NULL_PARAM_CHECK(lcd_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        LcdDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != lcd_device->haldev.dev_state) {
        strncpy(lcd_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        lcd_device->haldev.dev_type = TYPE_LCD_DEV;
        lcd_device->haldev.dev_state = DEV_INSTALL;

        lcd_device->haldev.dev_done = (struct HalDevDone *)lcd_device->dev_done;

        lcd_device->haldev.private_data = lcd_param;

        DoubleLinkListInsertNodeAfter(&lcddev_linklist, &(lcd_device->haldev.dev_link));
    } else {
        KPrintf("LcdDeviceRegister device has been register state%u\n", lcd_device->haldev.dev_state);        
    }

    return ret;
}

int LcdDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("LcdDeviceAttachToBus find lcd bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_LCD_BUS == bus->bus_type) {
        device = LcdDeviceFind(dev_name, TYPE_LCD_DEV);
        if (NONE == device) {
            KPrintf("LcdDeviceAttachToBus find lcd device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_LCD_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("LcdDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
