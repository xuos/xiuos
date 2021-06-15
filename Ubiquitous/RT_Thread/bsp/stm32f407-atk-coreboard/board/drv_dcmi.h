/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2020-07-27     thread-liu        the first version
 */

#ifndef __DRV_DCMI_H__
#define __DRV_DCMI_H__
#include "board.h"
#ifdef __cplusplus
extern "C" {
#endif
struct rt_dcmi_device
{
    struct rt_device parent;
};
struct stm32_dcmi
{
    DCMI_HandleTypeDef DCMI_Handle;
    struct rt_dcmi_device dev;
};

extern DMA_HandleTypeDef hdma_dcmi;
extern void  DCMI_IRQHandler(void);
extern void rt_hw_dcmi_dma_config(rt_uint32_t dst_addr1, rt_uint32_t dst_addr2, rt_uint32_t len);
extern void rt_dcmi_start(uint32_t pData, uint32_t Length);
extern void rt_dcmi_stop(void);
extern rt_err_t rt_set_irq_dcmi_callback_hander(void (*p)(void));
#ifdef __cplusplus
}
#endif

#endif
