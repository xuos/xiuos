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
* @file drv_usb.c
* @brief register usb drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_usb.h>
#include <dev_usb.h>

static DoubleLinklistType usbdrv_linklist;

/*Create the driver linklist*/
static void UsbDrvLinkInit()
{
    InitDoubleLinkList(&usbdrv_linklist);
}

/*Find the regiter driver*/
DriverType UsbDriverFind(const char *drv_name, enum DriverType drv_type)
{
    NULL_PARAM_CHECK(drv_name);
    
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &usbdrv_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if ((!strcmp(driver->drv_name, drv_name)) && (drv_type == driver->driver_type)) {
            return driver;
        }
    }

    KPrintf("UsbDriverFind cannot find the %s driver.return NULL\n", drv_name);
    return NONE;
}

/*Register the Driver, manage with the double linklist*/
int UsbDriverRegister(struct Driver *driver)
{
    NULL_PARAM_CHECK(driver);

    x_err_t ret = EOK;
    static x_bool driver_link_flag = RET_FALSE;

    if (!driver_link_flag) {
        UsbDrvLinkInit();
        driver_link_flag = RET_TRUE;
    }

    DoubleLinkListInsertNodeAfter(&usbdrv_linklist, &(driver->driver_link));

    return ret;
}
