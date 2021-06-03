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
* @file TestIwg.c
* @brief support to test watchdog function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <device.h>

static BusType wdt;

/**
 * @description: Feed watchdog task function
 */
static void FeedWatchdog(void)
{
    while (1)
    {
        /* keep watchdog alive in idle task */
        struct BusConfigureInfo cfg;
        cfg.configure_cmd = OPER_WDT_KEEPALIVE;
        cfg.private_data = NONE;
        BusDrvConfigure(wdt->owner_driver, &cfg);
        KPrintf("feed the dog!\n ");
    }
}

/**
 * @description: Watchdog test function
 * @return success: EOK, failure: other
 */
int TestIwg(void)
{
    x_err_t res = EOK;
    uint32 timeout = 1000;    /* timeout time */

    wdt = BusFind(WDT_BUS_NAME_0);
    wdt->owner_driver = BusFindDriver(wdt, WDT_DRIVER_NAME_0);
    wdt->owner_haldev = BusFindDevice(wdt, WDT_0_DEVICE_NAME_0);

    /* set watchdog timeout time */
    struct BusConfigureInfo cfg;
    cfg.configure_cmd = OPER_WDT_SET_TIMEOUT;
    cfg.private_data = &timeout;
    res = BusDrvConfigure(wdt->owner_driver, &cfg);
    KPrintf("feed the dog!\n");
    
    int32 WdtTask = KTaskCreate("WdtTask", (void *)FeedWatchdog, NONE, 2048, 20); 
	res = StartupKTask(WdtTask);

    return res;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),TestIwg, TestIwg,  TestIwg );
