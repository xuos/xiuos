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
* @file drv_can.c
* @brief register can drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_can.h>
#include <dev_can.h>

static DoubleLinklistType candrv_linklist;

/*Create the driver linklist*/
static void CanDrvLinkInit()
{
    InitDoubleLinkList(&candrv_linklist);
}

/*Find the regiter driver*/
DriverType CanDriverFind(const char *drv_name, enum DriverType drv_type)
{
    NULL_PARAM_CHECK(drv_name);
    
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &candrv_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if ((!strcmp(driver->drv_name, drv_name)) && (drv_type == driver->driver_type)) {
            return driver;
        }
    }

    KPrintf("CanDriverFind cannot find the %s driver.return NULL\n", drv_name);
    return NONE;
}

/*Register the Driver, manage with the double linklist*/
int CanDriverRegister(struct Driver *driver)
{
    NULL_PARAM_CHECK(driver);

    x_err_t ret = EOK;
    static x_bool driver_link_flag = RET_FALSE;

    if (!driver_link_flag) {
        CanDrvLinkInit();
        driver_link_flag = RET_TRUE;
    }

    DoubleLinkListInsertNodeAfter(&candrv_linklist, &(driver->driver_link));

    return ret;
}
