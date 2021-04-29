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
* @file drv_sdio.c
* @brief register sdio drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_sdio.h>
#include <dev_sdio.h>

static DoubleLinklistType sdiodrv_linklist;

/*Create the driver linklist*/
static void SdioDrvLinkInit()
{
    InitDoubleLinkList(&sdiodrv_linklist);
}

DriverType SdioDriverFind(const char *drv_name)
{
    NULL_PARAM_CHECK(drv_name);
    
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &sdiodrv_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if (!strcmp(driver->drv_name, drv_name)) {
            return driver;
        }
    }

    KPrintf("SdioDriverFind cannot find the %s driver.return NULL\n", drv_name);
    return NONE;
}

int SdioDriverRegister(struct Driver *driver)
{
    NULL_PARAM_CHECK(driver);

    x_err_t ret = EOK;
    static x_bool driver_link_flag = RET_FALSE;

    if (!driver_link_flag) {
        SdioDrvLinkInit();
        driver_link_flag = RET_TRUE;
    }

    DoubleLinkListInsertNodeAfter(&sdiodrv_linklist, &(driver->driver_link));

    return ret;
}
