/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-01-27     tianchunyu        the first version
*/

#ifndef __DRV_DVP_H__
#define __DRV_DVP_H__
#include <dvp.h>
#include <fpioa.h>
#include <sysctl.h>
#include <plic.h>
#include <sysctl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rt_dvp_device
{
    struct rt_device parent;
};

struct kendryte_dvp
{
    struct rt_dvp_device dev;
};

extern void rt_dvp_start(uint32_t pData, uint32_t Length);
extern void rt_dvp_stop(void);
extern rt_err_t rt_set_irq_dvp_callback_hander(void (*p)(void));





#ifdef __cplusplus
}
#endif

#endif

