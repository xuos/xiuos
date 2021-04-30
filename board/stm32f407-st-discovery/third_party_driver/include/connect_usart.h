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
* @file connect_usart.h
* @brief define stm32f407-st-discovery-board usart function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef CONNECT_USART_H
#define CONNECT_USART_H

#include <device.h>
#include "hardware_usart.h"
#include "hardware_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_CONSOLE_BUS_NAME       SERIAL_BUS_NAME_1
#define KERNEL_CONSOLE_DRV_NAME       SERIAL_DRV_NAME_1
#define KERNEL_CONSOLE_DEVICE_NAME SERIAL_1_DEVICE_NAME_0

struct UsartHwCfg
{
    USART_TypeDef *uart_device;
    IRQn_Type irq;
};

struct Stm32Usart
{
    struct Stm32UsartDma
    {
        DMA_Stream_TypeDef *RxStream;
        uint32 RxCh;
        uint32 RxFlag;
        uint8 RxIrqCh;
        x_size_t SettingRecvLen;
        x_size_t LastRecvIndex;
    } dma;

    struct SerialBus serial_bus;
};

int Stm32HwUsartInit(void);

#ifdef __cplusplus
}
#endif

#endif
