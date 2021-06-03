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
* @brief support kd233-board watchdog function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include "wdt.h"
#include "connect_wdt.h"

static uint32 WdtOpen(void *dev)
{
    NULL_PARAM_CHECK(dev);
    
    wdt_device_number_t id;
    struct WdtHardwareDevice *wdt = (struct WdtHardwareDevice *)dev;
    id = *(wdt_device_number_t *)wdt->private_data;

    wdt_init(id, 4095, NONE, NONE);
    return EOK;
}

static uint32 WdtConfigure(void *drv, struct BusConfigureInfo *args)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(args);

    struct WdtDriver *wdt = (struct WdtDriver *)drv;
    wdt_device_number_t id = *(wdt_device_number_t *)wdt->private_data;

    switch (args->configure_cmd)
    {
    case OPER_WDT_SET_TIMEOUT:
        if (wdt_init(id, (uint64_t)*(int *)args->private_data, NONE, NONE) == 0)
        {
            return ERROR;
        }
        break;
    case OPER_WDT_KEEPALIVE:
        wdt_feed(id);
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

int HwWdtInit(void)
{
    wdt_device_number_t id;

    x_err_t ret = EOK;

#ifdef BSP_USING_WDT0
    {
        static struct WdtBus wdt0;

        ret = WdtBusInit(&wdt0, WDT_BUS_NAME_0);
        if(ret != EOK)
        {
            KPrintf("Watchdog bus init error %d\n", ret);
            return ERROR;
        }

        static struct WdtDriver drv0;
        drv0.configure = WdtConfigure;
        id = WDT_DEVICE_0;
        drv0.private_data = &id;
        
        ret = WdtDriverInit(&drv0, WDT_DRIVER_NAME_0);
        if(ret != EOK)
        {
            KPrintf("Watchdog driver init error %d\n", ret);
            return ERROR;
        }
        ret = WdtDriverAttachToBus(WDT_DRIVER_NAME_0, WDT_BUS_NAME_0);
        if(ret != EOK)
        {
            KPrintf("Watchdog driver attach error %d\n", ret);
            return ERROR;
        }

        static struct WdtHardwareDevice dev0;
        dev0.dev_done = &dev_done;
        dev0.private_data = &id;

        ret = WdtDeviceRegister(&dev0, WDT_0_DEVICE_NAME_0);
        if(ret != EOK)
        {
            KPrintf("Watchdog device register error %d\n", ret);
            return ERROR;
        }
        ret = WdtDeviceAttachToBus(WDT_0_DEVICE_NAME_0, WDT_BUS_NAME_0);
        if(ret != EOK)
        {
            KPrintf("Watchdog device register error %d\n", ret);
            return ERROR;
        }
    }
#endif

#ifdef BSP_USING_WDT1
    {
        static struct WdtBus wdt1;

        ret = WdtBusInit(&wdt1, WDT_BUS_NAME_1);
        if(ret != EOK)
        {
            KPrintf("Watchdog bus init error %d\n", ret);
            return ERROR;
        }

        static struct WdtDriver drv1;
        drv1.configure = WdtConfigure;
        id = WDT_DEVICE_1;
        drv1.private_data = &id;
        
        ret = WdtDriverInit(&drv1, WDT_DRIVER_NAME_1);
        if(ret != EOK)
        {
            KPrintf("Watchdog driver init error %d\n", ret);
            return ERROR;
        }
        ret = WdtDriverAttachToBus(WDT_DRIVER_NAME_1, WDT_BUS_NAME_1);
        if(ret != EOK)
        {
            KPrintf("Watchdog driver attach error %d\n", ret);
            return ERROR;
        }

        static struct WdtHardwareDevice dev1;
        dev1.dev_done = &dev_done;
        dev1.private_data = &id;

        ret = WdtDeviceRegister(&dev1, WDT_1_DEVICE_NAME_1);
        if(ret != EOK)
        {
            KPrintf("Watchdog device register error %d\n", ret);
            return ERROR;
        }
        ret = WdtDeviceAttachToBus(WDT_1_DEVICE_NAME_1, WDT_BUS_NAME_1);
        if(ret != EOK)
        {
            KPrintf("Watchdog device register error %d\n", ret);
            return ERROR;
        }
    }
#endif

    return ret;
}
