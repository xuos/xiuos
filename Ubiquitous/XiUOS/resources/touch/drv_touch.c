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
* @file drv_touch.c
* @brief register touch drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_touch.h>
#include <dev_touch.h>

static DoubleLinklistType touchdrv_linklist;

/*Create the driver linklist*/
static void TouchDrvLinkInit()
{
    InitDoubleLinkList(&touchdrv_linklist);
}

DriverType TouchDriverFind(const char *drv_name, enum DriverType drv_type)
{
    NULL_PARAM_CHECK(drv_name);
    
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &touchdrv_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if ((!strcmp(driver->drv_name, drv_name)) && (drv_type == driver->driver_type)) {
            return driver;
        }
    }

    KPrintf("TouchDriverFind cannot find the %s driver.return NULL\n", drv_name);
    return NONE;
}

int TouchDriverRegister(struct Driver *driver)
{
    NULL_PARAM_CHECK(driver);

    x_err_t ret = EOK;
    static x_bool driver_link_flag = RET_FALSE;

    if (!driver_link_flag) {
        TouchDrvLinkInit();
        driver_link_flag = RET_TRUE;
    }

    DoubleLinkListInsertNodeAfter(&touchdrv_linklist, &(driver->driver_link));

    return ret;
}
