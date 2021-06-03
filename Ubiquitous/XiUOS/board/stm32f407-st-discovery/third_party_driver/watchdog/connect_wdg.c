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
* @file connect_wdt.c
* @brief support stm32f407-st-discovery-board watchdog function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <device.h>
#include "hardware_iwdg.h"
#include "connect_wdg.h"

/**
 * This function                            Watchdog configuration function
 *
 * @param arg                             Watchdog device Parameters
 * 
 * @return                                       EOK
 */
static int WdgSet(uint16_t arg)
{
    IWDG_Enable();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_16);
    IWDG_SetReload(arg);
    while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) != RESET);
    IWDG_ReloadCounter();
    return 0;
}
/**
 * This function                             Watchdog initialization
 *
 * @param dev                              Watchdog driver structure  handle
 * 
 * @return                                       EOK
 */
static uint32 WdtOpen(void *dev)
{
    WdgSet(4000);
    return EOK;
}
/**
 * This function                              Watchdog control function
 *
 * @param drv                               Watchdog driver structure  handle
 * 
 * @param args                              Watchdog driver Parameters
 * 
 * @return                                       EOK
 */
static uint32 WdtConfigure(void *drv, struct BusConfigureInfo *args)
{
    switch (args->configure_cmd)
    {
        case OPER_WDT_SET_TIMEOUT:
            if (WdgSet((uint16_t)*(int *)args->private_data) != 0) {
                return ERROR;
            }
            break;
        case OPER_WDT_KEEPALIVE:
            IWDG_ReloadCounter();
            break;
        default:
            return ERROR;
    }
    return EOK;
}

static const struct WdtDevDone dev_done =
{
    WdtOpen,
    NONE,
    NONE,
    NONE,
};

/**
 * This function                            Watchdog initialization
 *
 * @return                                      EOK
 */
int HwWdtInit(void)
{
    x_err_t ret = EOK;

    static struct WdtBus wdt;
    static struct WdtDriver drv;
    static struct WdtHardwareDevice dev;

    ret = WdtBusInit(&wdt, WDT_BUS_NAME);
    if(ret != EOK){
        KPrintf("Watchdog bus init error %d\n", ret);
        return ERROR;
    }

    drv.configure = WdtConfigure;
    ret = WdtDriverInit(&drv, WDT_DRIVER_NAME);
    if (ret != EOK) {
        KPrintf("Watchdog driver init error %d\n", ret);
        return ERROR;
    }
    ret = WdtDriverAttachToBus(WDT_DRIVER_NAME, WDT_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Watchdog driver attach error %d\n", ret);
        return ERROR;
    }

    dev.dev_done = &dev_done;

    ret = WdtDeviceRegister(&dev, WDT_DEVICE_NAME);
    if (ret != EOK) {
        KPrintf("Watchdog device register error %d\n", ret);
        return ERROR;
    }
    ret = WdtDeviceAttachToBus(WDT_DEVICE_NAME, WDT_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Watchdog device register error %d\n", ret);
        return ERROR;
    }
}
