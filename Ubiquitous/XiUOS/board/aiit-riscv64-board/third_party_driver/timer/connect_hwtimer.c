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
* @file connect_hwtimer.c
* @brief support aiit-riscv64-board hwtimer function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <board.h>
#include <connect_hwtimer.h>
#include <fpioa.h>
#include <plic.h>
#include <stdio.h>
#include <sysctl.h>
#include <syslog.h>

static struct HwtimerCallBackInfo *ptim2_cb_info = NULL;

int timer_callback(void *ctx) 
{
    if (ptim2_cb_info) {
        if (ptim2_cb_info->timeout_callback) {
            ptim2_cb_info->timeout_callback(ptim2_cb_info->param);
        }
    }
    return 0;
}

uint32 HwtimerOpen(void *dev)
{
    struct HwtimerHardwareDevice *hwtimer_dev = dev;

    ptim2_cb_info = &hwtimer_dev->hwtimer_param.cb_info;

    plic_init();
    sysctl_enable_irq();
    timer_init(TIMER_DEVICE_1);

    size_t real_time =  timer_set_interval(TIMER_DEVICE_1, TIMER_CHANNEL_1, hwtimer_dev->hwtimer_param.period_millisecond *1000);
    KPrintf("timer_set_interval --  real_time : %ld\n", real_time);
    timer_irq_register(TIMER_DEVICE_1, TIMER_CHANNEL_1, !hwtimer_dev->hwtimer_param.repeat, 1, timer_callback, NULL);

    timer_set_enable(TIMER_DEVICE_1, TIMER_CHANNEL_1, 1);

    return EOK;
}

uint32 HwtimerClose(void *dev)
{
    timer_set_enable(TIMER_DEVICE_1, TIMER_CHANNEL_1, 0);

    return EOK;
}

/*manage the hwtimer device operations*/
static const struct HwtimerDevDone dev_done =
{
    .open = HwtimerOpen,
    .close = HwtimerClose,
    .write = NONE,
    .read = NONE,
};

/*Init hwtimer bus*/
static int BoardHwtimerBusInit(struct HwtimerBus *hwtimer_bus, struct HwtimerDriver *hwtimer_driver)
{
    x_err_t ret = EOK;

    /*Init the hwtimer bus */
    ret = HwtimerBusInit(hwtimer_bus, HWTIMER_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("board_hwtimer_init HwtimerBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the hwtimer driver*/
    hwtimer_driver->configure = NONE;
    ret = HwtimerDriverInit(hwtimer_driver, HWTIMER_DRIVER_NAME_1);
    if (EOK != ret) {
        KPrintf("board_hwtimer_init HwtimerDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the hwtimer driver to the hwtimer bus*/
    ret = HwtimerDriverAttachToBus(HWTIMER_DRIVER_NAME_1, HWTIMER_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("board_hwtimer_init USEDriverAttachToBus error %d\n", ret);
        return ERROR;
    }

    return ret;
}

/*Attach the hwtimer device to the hwtimer bus*/
static int BoardHwtimerDevBend(void)
{
    x_err_t ret = EOK;
    static struct HwtimerHardwareDevice hwtimer_device_0;
    memset(&hwtimer_device_0, 0, sizeof(struct HwtimerHardwareDevice));

    hwtimer_device_0.dev_done = &dev_done;

    ret = HwtimerDeviceRegister(&hwtimer_device_0, NONE, HWTIMER_1_DEVICE_NAME_1);
    if (EOK != ret) {
        KPrintf("BoardHwtimerDevBend HwtimerDeviceRegister device %s error %d\n", HWTIMER_1_DEVICE_NAME_1, ret);
        return ERROR;
    }

    ret = HwtimerDeviceAttachToBus(HWTIMER_1_DEVICE_NAME_1, HWTIMER_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("BoardHwtimerDevBend HwtimerDeviceAttachToBus device %s error %d\n", HWTIMER_1_DEVICE_NAME_1, ret);
        return ERROR;
    }

    return ret;
}

/*K210 BOARD HWTIMER INIT*/
int HwTimerInit(void)
{
    x_err_t ret = EOK;
    static struct HwtimerBus hwtimer_bus;
    memset(&hwtimer_bus, 0, sizeof(struct HwtimerBus));

    static struct HwtimerDriver hwtimer_driver;
    memset(&hwtimer_driver, 0, sizeof(struct HwtimerDriver));

    ret = BoardHwtimerBusInit(&hwtimer_bus, &hwtimer_driver);
    if (EOK != ret) {
        KPrintf("board_hwtimer_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardHwtimerDevBend();
    if (EOK != ret) {
        KPrintf("board_hwtimer_Init error ret %u\n", ret);
        return ERROR;
    }

    return ret;
}
