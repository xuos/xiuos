/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-10     zylx         first version
 * 2020-06-16     thread-liu   Porting for stm32mp1
 * 2020-08-25     linyongkang  Fix the timer clock frequency doubling problem
 * 2020-10-14     Dozingfiretruck   Porting for stm32wbxx
 */

/**
* @file connect_hwtimer.c
* @brief support aiit-arm32-board hwtimer function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_hwtimer.c
Description: support aiit-arm32-board hwtimer configure and spi bus register function
Others: take RT-Thread v4.0.2/bsp/stm32/libraries/HAL_Drivers/drv_hwtimer.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board hwtimer configure
2. support aiit-arm32-board hwtimer bus device and driver register
*************************************************/

#include <board.h>
#include <connect_hwtimer.h>
#include <hardware_rcc.h>
#include <hardware_tim.h>
#include <misc.h>

static struct HwtimerCallBackInfo *ptim2_cb_info = NULL;


#ifdef ENABLE_TIM2
void TIM2_IRQHandler(int irq_num, void *arg)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    KPrintf("hwtimer 2 ... come ...\n");

    if (ptim2_cb_info)
    {
        if (ptim2_cb_info->TimeoutCb){
            ptim2_cb_info->TimeoutCb(ptim2_cb_info->param);
        }
    }
}
DECLARE_HW_IRQ(TIM2_IRQn, TIM2_IRQHandler, NONE);
#endif

uint32 HwtimerOpen(void *dev)
{
    struct HwtimerHardwareDevice *hwtimer_dev = dev;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_DeInit(TIM2);

    TIM_TimeBaseInitTypeDef timer_def;
    timer_def.TIM_Period = (hwtimer_dev->hwtimer_param.period_millisecond) * 10 - 1;
    timer_def.TIM_Prescaler = 8400 - 1;
    timer_def.TIM_CounterMode = TIM_CounterMode_Up;
    timer_def.TIM_ClockDivision = TIM_CKD_DIV1;
    timer_def.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timer_def);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;       
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             
    NVIC_Init(&NVIC_InitStructure);

    ptim2_cb_info = &hwtimer_dev->hwtimer_param.cb_info;

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    return EOK;
}

uint32 HwtimerClose(void *dev)
{
    TIM_Cmd(TIM2, DISABLE);
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
    ret = HwtimerBusInit(hwtimer_bus, HWTIMER_BUS_NAME_2);
    if (EOK != ret){
        KPrintf("board_hwtimer_init HwtimerBusInit error %d\n", ret);
        return ERROR;
    }

    #ifdef ENABLE_TIM2
    /*Init the hwtimer driver*/
    hwtimer_driver->configure = NONE;
    ret = HwtimerDriverInit(hwtimer_driver, HWTIMER_DRIVER_NAME_2);
    if (EOK != ret){
        KPrintf("board_hwtimer_init HwtimerDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the hwtimer driver to the hwtimer bus*/
    ret = HwtimerDriverAttachToBus(HWTIMER_DRIVER_NAME_2, HWTIMER_BUS_NAME_2);
    if (EOK != ret) {
        KPrintf("board_hwtimer_init USEDriverAttachToBus error %d\n", ret);
        return ERROR;
    }
    #endif

    return ret;
}

/*Attach the hwtimer device to the hwtimer bus*/
static int BoardHwtimerDevBend(void)
{
    #ifdef ENABLE_TIM2
    x_err_t ret = EOK;
    static struct HwtimerHardwareDevice hwtimer_device_2;
    memset(&hwtimer_device_2, 0, sizeof(struct HwtimerHardwareDevice));

    hwtimer_device_2.dev_done = &dev_done;

    ret = HwtimerDeviceRegister(&hwtimer_device_2, NONE, HWTIMER_2_DEVICE_NAME_2);
    if (EOK != ret){
        KPrintf("board_hwtimer_init HWTIMERDeviceInit device %s error %d\n", HWTIMER_2_DEVICE_NAME_2, ret);
        return ERROR;
    }

    ret = HwtimerDeviceAttachToBus(HWTIMER_2_DEVICE_NAME_2, HWTIMER_BUS_NAME_2);
    if (EOK != ret) {
        KPrintf("board_hwtimer_init HwtimerDeviceAttachToBus device %s error %d\n", HWTIMER_2_DEVICE_NAME_2, ret);
        return ERROR;
    }

    return ret;
    #endif
}

/*ARM-32 BOARD HWTIMER INIT*/
int Stm32HwTimerInit(void)
{
    x_err_t ret = EOK;
    static struct HwtimerBus hwtimer_bus;
    memset(&hwtimer_bus, 0, sizeof(struct HwtimerBus));

    static struct HwtimerDriver hwtimer_driver;
    memset(&hwtimer_driver, 0, sizeof(struct HwtimerDriver));


    ret = BoardHwtimerBusInit(&hwtimer_bus, &hwtimer_driver);
    if (EOK != ret){
        KPrintf("board_hwtimer_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardHwtimerDevBend();
    if (EOK != ret){
        KPrintf("board_hwtimer_Init error ret %u\n", ret);
        return ERROR;
    }

    return ret;
}
