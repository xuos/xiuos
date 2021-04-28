/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file connect_usart.c
* @brief support aiit-arm32-board usart function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_uart.c
Description: support aiit-arm32-board usart configure and uart bus register function
Others: take RT-Thread v4.0.2/bsp/stm32/libraries/HAL_Drivers/drv_usart.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board usart configure, write and read
2. support aiit-arm32-board usart bus device and driver register
*************************************************/

#include <board.h>
#include <connect_usart.h>
#include <hardware_gpio.h>
#include <hardware_rcc.h>
#include <misc.h>

/* UART GPIO define. */
#define UART1_GPIO_TX       GPIO_Pin_9
#define UART1_TX_PIN_SOURCE GPIO_PinSource9
#define UART1_GPIO_RX       GPIO_Pin_10
#define UART1_RX_PIN_SOURCE GPIO_PinSource10
#define UART1_GPIO          GPIOA
#define UART1_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART1 RCC_APB2Periph_USART1

#define UART2_GPIO_TX       GPIO_Pin_2
#define UART2_TX_PIN_SOURCE GPIO_PinSource2
#define UART2_GPIO_RX       GPIO_Pin_3
#define UART2_RX_PIN_SOURCE GPIO_PinSource3
#define UART2_GPIO          GPIOA
#define UART2_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART2 RCC_APB1Periph_USART2

#define UART3_GPIO_TX       GPIO_Pin_10
#define UART3_TX_PIN_SOURCE GPIO_PinSource10
#define UART3_GPIO_RX       GPIO_Pin_11
#define UART3_RX_PIN_SOURCE GPIO_PinSource11
#define UART3_GPIO          GPIOB
#define UART3_GPIO_RCC      RCC_AHB1Periph_GPIOB
#define RCC_APBPeriph_UART3 RCC_APB1Periph_USART3

#define UART4_GPIO_TX       GPIO_Pin_10
#define UART4_TX_PIN_SOURCE GPIO_PinSource10
#define UART4_GPIO_RX       GPIO_Pin_11
#define UART4_RX_PIN_SOURCE GPIO_PinSource11
#define UART4_GPIO          GPIOC
#define UART4_GPIO_RCC      RCC_AHB1Periph_GPIOC
#define RCC_APBPeriph_UART4 RCC_APB1Periph_UART4

#define UART5_GPIO_TX       GPIO_Pin_12
#define UART5_TX_PIN_SOURCE GPIO_PinSource12
#define UART5_GPIO_RX       GPIO_Pin_2
#define UART5_RX_PIN_SOURCE GPIO_PinSource2
#define UART5_TX            GPIOC
#define UART5_RX            GPIOD
#define UART5_GPIO_RCC_TX   RCC_AHB1Periph_GPIOC
#define UART5_GPIO_RCC_RX   RCC_AHB1Periph_GPIOD
#define RCC_APBPeriph_UART5 RCC_APB1Periph_UART5

#define UART_ENABLE_IRQ(n)            NVIC_EnableIRQ((n))
#define UART_DISABLE_IRQ(n)           NVIC_DisableIRQ((n))

static void RCCConfiguration(void)
{
#ifdef BSP_USING_USART1
    RCC_AHB1PeriphClockCmd(UART1_GPIO_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APBPeriph_UART1, ENABLE);
#endif

#ifdef BSP_USING_USART2
    RCC_AHB1PeriphClockCmd(UART2_GPIO_RCC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APBPeriph_UART2, ENABLE);
#endif

#ifdef BSP_USING_USART3
    RCC_AHB1PeriphClockCmd(UART3_GPIO_RCC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APBPeriph_UART3, ENABLE);
#endif

#ifdef BSP_USING_UART4
    RCC_AHB1PeriphClockCmd(UART4_GPIO_RCC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APBPeriph_UART4, ENABLE);
#endif

#ifdef BSP_USING_UART5
    RCC_AHB1PeriphClockCmd(UART5_GPIO_RCC_TX | UART5_GPIO_RCC_RX, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APBPeriph_UART5, ENABLE);
#endif
}

static void GPIOConfiguration(void)
{
    GPIO_InitTypeDef gpio_initstructure;

    gpio_initstructure.GPIO_Mode  = GPIO_Mode_AF;
    gpio_initstructure.GPIO_OType = GPIO_OType_PP;
    gpio_initstructure.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_2MHz;

#ifdef BSP_USING_USART1
    gpio_initstructure.GPIO_Pin = UART1_GPIO_RX | UART1_GPIO_TX;
    GPIO_PinAFConfig(UART1_GPIO, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);
    GPIO_PinAFConfig(UART1_GPIO, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);    
    
    GPIO_Init(UART1_GPIO, &gpio_initstructure);
#endif

#ifdef BSP_USING_USART2
    gpio_initstructure.GPIO_Pin = UART2_GPIO_RX | UART2_GPIO_TX;
    GPIO_PinAFConfig(UART2_GPIO, UART2_TX_PIN_SOURCE, GPIO_AF_USART2);
    GPIO_PinAFConfig(UART2_GPIO, UART2_RX_PIN_SOURCE, GPIO_AF_USART2);
    
    GPIO_Init(UART2_GPIO, &gpio_initstructure);
#endif

#ifdef BSP_USING_USART3
    gpio_initstructure.GPIO_Pin = UART3_GPIO_TX | UART3_GPIO_RX;
    GPIO_PinAFConfig(UART3_GPIO, UART3_TX_PIN_SOURCE, GPIO_AF_USART3);
    GPIO_PinAFConfig(UART3_GPIO, UART3_RX_PIN_SOURCE, GPIO_AF_USART3);
    
    GPIO_Init(UART3_GPIO, &gpio_initstructure);
#endif

#ifdef BSP_USING_UART4
    gpio_initstructure.GPIO_Pin = UART4_GPIO_TX | UART4_GPIO_RX;
    GPIO_PinAFConfig(UART4_GPIO, UART4_TX_PIN_SOURCE, GPIO_AF_UART4);
    GPIO_PinAFConfig(UART4_GPIO, UART4_RX_PIN_SOURCE, GPIO_AF_UART4);
    
    GPIO_Init(UART4_GPIO, &gpio_initstructure);
#endif

#ifdef BSP_USING_UART5
    gpio_initstructure.GPIO_Pin = UART5_GPIO_TX;
    GPIO_PinAFConfig(UART5_TX, UART5_TX_PIN_SOURCE, GPIO_AF_UART5); 
    GPIO_Init(UART5_TX, &gpio_initstructure);
    
    gpio_initstructure.GPIO_Pin = UART5_GPIO_RX;
    GPIO_PinAFConfig(UART5_RX, UART5_RX_PIN_SOURCE, GPIO_AF_UART5);
    GPIO_Init(UART5_RX, &gpio_initstructure);
#endif
}

static void NVIC_Configuration(IRQn_Type irq)
{
    NVIC_InitTypeDef nvic_initstructure;

    nvic_initstructure.NVIC_IRQChannel = irq;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_initstructure.NVIC_IRQChannelSubPriority = 1;
    nvic_initstructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_initstructure);
}

static void DmaUartConfig(struct Stm32UsartDma *dma, USART_TypeDef *uart_device, uint32_t SettingRecvLen, void *mem_base_addr)
{   
    DMA_InitTypeDef DMA_InitStructure;

    dma->SettingRecvLen = SettingRecvLen;
    DMA_DeInit(dma->RxStream);
    while (DMA_GetCmdStatus(dma->RxStream) != DISABLE);
    DMA_InitStructure.DMA_Channel = dma->RxCh;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(uart_device->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)mem_base_addr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = dma->SettingRecvLen;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(dma->RxStream, &DMA_InitStructure);
}

static void DMAConfiguration(struct SerialHardwareDevice *serial_dev, USART_TypeDef *uart_device)
{
    struct Stm32Usart *serial = CONTAINER_OF(serial_dev->haldev.owner_bus, struct Stm32Usart, SerialBus);
    struct Stm32UsartDma *dma = &serial->dma; 
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;

    NVIC_InitTypeDef nvic_initstructure;

    USART_ITConfig(uart_device, USART_IT_IDLE , ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DmaUartConfig(dma, uart_device, serial_cfg->data_cfg.serial_buffer_size, serial_dev->serial_fifo.serial_rx->serial_rx_buffer);

    DMA_ClearFlag(dma->RxStream, dma->RxFlag);
    DMA_ITConfig(dma->RxStream, DMA_IT_TC, ENABLE);
    USART_DMACmd(uart_device, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(dma->RxStream, ENABLE);

    nvic_initstructure.NVIC_IRQChannel = dma->RxIrqCh;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_initstructure.NVIC_IRQChannelSubPriority = 0;
    nvic_initstructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_initstructure);
}

static void SerialCfgParamCheck(struct SerialCfgParam *serial_cfg_default, struct SerialCfgParam *serial_cfg_new)
{    
    struct SerialDataCfg *data_cfg_default = &serial_cfg_default->data_cfg;
    struct SerialDataCfg *data_cfg_new = &serial_cfg_new->data_cfg;

    if((data_cfg_default->serial_baud_rate != data_cfg_new->serial_baud_rate) && (data_cfg_new->serial_baud_rate)){
        data_cfg_default->serial_baud_rate = data_cfg_new->serial_baud_rate;
    }

    if((data_cfg_default->serial_bit_order != data_cfg_new->serial_bit_order) && (data_cfg_new->serial_bit_order)){
        data_cfg_default->serial_bit_order = data_cfg_new->serial_bit_order;
    }

    if((data_cfg_default->serial_buffer_size != data_cfg_new->serial_buffer_size) && (data_cfg_new->serial_buffer_size)){
        data_cfg_default->serial_buffer_size = data_cfg_new->serial_buffer_size;
    }

    if((data_cfg_default->serial_data_bits != data_cfg_new->serial_data_bits) && (data_cfg_new->serial_data_bits)){
        data_cfg_default->serial_data_bits = data_cfg_new->serial_data_bits;
    }

    if((data_cfg_default->serial_invert_mode != data_cfg_new->serial_invert_mode) && (data_cfg_new->serial_invert_mode)) {
        data_cfg_default->serial_invert_mode = data_cfg_new->serial_invert_mode;
    }

    if((data_cfg_default->serial_parity_mode != data_cfg_new->serial_parity_mode) && (data_cfg_new->serial_parity_mode)) {
        data_cfg_default->serial_parity_mode = data_cfg_new->serial_parity_mode;
    }

    if((data_cfg_default->serial_stop_bits != data_cfg_new->serial_stop_bits) && (data_cfg_new->serial_stop_bits)) {
        data_cfg_default->serial_stop_bits = data_cfg_new->serial_stop_bits;
    }
}

static uint32 Stm32SerialInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;

    if(configure_info->private_data){
        struct SerialCfgParam *serial_cfg_new = (struct SerialCfgParam *)configure_info->private_data;
        SerialCfgParamCheck(serial_cfg, serial_cfg_new);
    }

    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = serial_cfg->data_cfg.serial_baud_rate;

    if (serial_cfg->data_cfg.serial_data_bits == DATA_BITS_8) {
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    }
    else if (serial_cfg->data_cfg.serial_data_bits == DATA_BITS_9){
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    }

    if (serial_cfg->data_cfg.serial_stop_bits == STOP_BITS_1) {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    else if (serial_cfg->data_cfg.serial_stop_bits == STOP_BITS_2) {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    }

    if (serial_cfg->data_cfg.serial_parity_mode == PARITY_NONE){
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }
    else if (serial_cfg->data_cfg.serial_parity_mode == PARITY_ODD){
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
    }
    else if (serial_cfg->data_cfg.serial_parity_mode == PARITY_EVEN){
        USART_InitStructure.USART_Parity = USART_Parity_Even;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(serial_hw_cfg->uart_device, &USART_InitStructure);

    USART_Cmd(serial_hw_cfg->uart_device, ENABLE);

    return EOK;
}

static uint32 Stm32SerialConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)serial_drv->driver.owner_bus->owner_haldev;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;

    switch (serial_operation_cmd)
    {
        case OPER_CLR_INT:
            UART_DISABLE_IRQ(serial_hw_cfg->irq);
            USART_ITConfig(serial_hw_cfg->uart_device, USART_IT_RXNE, DISABLE);
            break;
        case OPER_SET_INT:
            UART_ENABLE_IRQ(serial_hw_cfg->irq);
            USART_ITConfig(serial_hw_cfg->uart_device, USART_IT_RXNE, ENABLE);
            break;
        case OPER_CONFIG :
            if (SIGN_OPER_DMA_RX == serial_dev_param->serial_set_mode)
            {
                DMAConfiguration(serial_dev, serial_hw_cfg->uart_device);
            }
    }

    return EOK;
}

static int Stm32SerialPutchar(struct SerialHardwareDevice *serial_dev, char c)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;

    while (!(serial_hw_cfg->uart_device->SR & USART_FLAG_TXE));
    serial_hw_cfg->uart_device->DR = c;

    return EOK;
}

static int Stm32SerialGetchar(struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;

    int ch = -1;
    if (serial_hw_cfg->uart_device->SR & USART_FLAG_RXNE) {
        ch = serial_hw_cfg->uart_device->DR & 0xff;
    }

    return ch;
}

static void DmaUartRxIdleIsr(struct SerialHardwareDevice *serial_dev, struct Stm32UsartDma *dma, USART_TypeDef *uart_device)
{
    x_base level = CriticalAreaLock();

    x_size_t recv_total_index = dma->SettingRecvLen - DMA_GetCurrDataCounter(dma->RxStream);
    x_size_t recv_len = recv_total_index - dma->LastRecvIndex;
    dma->LastRecvIndex = recv_total_index;
    CriticalAreaUnLock(level);

    if (recv_len) SerialSetIsr(serial_dev, SERIAL_EVENT_RX_DMADONE | (recv_len << 8));

    USART_ReceiveData(uart_device);
}

static void DmaRxDoneIsr(struct Stm32Usart *serial, struct SerialDriver *serial_drv, struct SerialHardwareDevice *serial_dev)
{
    struct Stm32UsartDma *dma = &serial->dma; 
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;

    if (DMA_GetFlagStatus(dma->RxStream, dma->RxFlag) != RESET){
        x_base level = CriticalAreaLock();

        x_size_t recv_len = dma->SettingRecvLen - dma->LastRecvIndex;
        dma->LastRecvIndex = 0;
        CriticalAreaUnLock(level);

        if (recv_len) SerialSetIsr(serial_dev, SERIAL_EVENT_RX_DMADONE | (recv_len << 8));

        DMA_ClearFlag(dma->RxStream, dma->RxFlag);
    }
}

static void UartIsr(struct Stm32Usart *serial, struct SerialDriver *serial_drv, struct SerialHardwareDevice *serial_dev)
{
    struct Stm32UsartDma *dma = &serial->dma; 
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct UsartHwCfg *serial_hw_cfg = (struct UsartHwCfg *)serial_cfg->hw_cfg.private_data;

    if(USART_GetITStatus(serial_hw_cfg->uart_device, USART_IT_RXNE) != RESET){
        SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
        USART_ClearITPendingBit(serial_hw_cfg->uart_device, USART_IT_RXNE);
    }
    if(USART_GetITStatus(serial_hw_cfg->uart_device, USART_IT_IDLE) != RESET){
        DmaUartRxIdleIsr(serial_dev, dma, serial_hw_cfg->uart_device);
    }
    if (USART_GetITStatus(serial_hw_cfg->uart_device, USART_IT_TC) != RESET){
        USART_ClearITPendingBit(serial_hw_cfg->uart_device, USART_IT_TC);
    }
    if (USART_GetFlagStatus(serial_hw_cfg->uart_device, USART_FLAG_ORE) == SET){
        USART_ReceiveData(serial_hw_cfg->uart_device);
    }
}

#ifdef BSP_USING_USART1
struct Stm32Usart serial_1;
struct SerialDriver serial_driver_1;
struct SerialHardwareDevice serial_device_1;

static const struct Stm32UsartDma usart_dma_1 =
{
    DMA2_Stream5,
    DMA_Channel_4,
    DMA_FLAG_TCIF5,
    DMA2_Stream5_IRQn,
    0,
    0,
};

void USART1_IRQHandler(int irq_num, void *arg)
{
    UartIsr(&serial_1, &serial_driver_1, &serial_device_1);
}
DECLARE_HW_IRQ(USART1_IRQn, USART1_IRQHandler, NONE);

void DMA2_Stream5_IRQHandler(int irq_num, void *arg) {
    DmaRxDoneIsr(&serial_1, &serial_driver_1, &serial_device_1);
}
DECLARE_HW_IRQ(DMA2_Stream5_IRQn, DMA2_Stream5_IRQHandler, NONE);
#endif

#ifdef BSP_USING_USART2
struct Stm32Usart serial_2;
struct SerialDriver serial_driver_2;
struct SerialHardwareDevice serial_device_2;

static const struct Stm32UsartDma usart_dma_2 =
{
    DMA1_Stream5,
    DMA_Channel_4,
    DMA_FLAG_TCIF5,
    DMA1_Stream5_IRQn,
    0,
    0,
};

void USART2_IRQHandler(int irq_num, void *arg)
{
    UartIsr(&serial_2, &serial_driver_2, &serial_device_2);
}
DECLARE_HW_IRQ(USART2_IRQn, USART2_IRQHandler, NONE);

void DMA1_Stream5_IRQHandler(int irq_num, void *arg) {
    DmaRxDoneIsr(&serial_2, &serial_driver_2, &serial_device_2);
}
DECLARE_HW_IRQ(DMA1_Stream5_IRQn, DMA1_Stream5_IRQHandler, NONE);
#endif

#ifdef BSP_USING_USART3
struct Stm32Usart serial_3;
struct SerialDriver serial_driver_3;
struct SerialHardwareDevice serial_device_3;

static const struct Stm32UsartDma usart_dma_3 =
{
    DMA1_Stream1,
    DMA_Channel_4,
    DMA_FLAG_TCIF1,
    DMA1_Stream1_IRQn,
    0,
    0,
};

void USART3_IRQHandler(int irq_num, void *arg)
{
    UartIsr(&serial_3, &serial_driver_3, &serial_device_3);
}
DECLARE_HW_IRQ(USART3_IRQn, USART3_IRQHandler, NONE);

void DMA1_Stream1_IRQHandler(int irq_num, void *arg) {
    DmaRxDoneIsr(&serial_3, &serial_driver_3, &serial_device_3);
}
DECLARE_HW_IRQ(DMA1_Stream1_IRQn, DMA1_Stream1_IRQHandler, NONE);
#endif

#ifdef BSP_USING_UART4
struct Stm32Usart serial_4;
struct SerialDriver serial_driver_4;
struct SerialHardwareDevice serial_device_4;

static const struct Stm32UsartDma uart_dma_4 =
{
    DMA1_Stream2,
    DMA_Channel_4,
    DMA_FLAG_TCIF2,
    DMA1_Stream2_IRQn,
    0,
    0,
};

void UART4_IRQHandler(int irq_num, void *arg)
{
    UartIsr(&serial_4, &serial_driver_4, &serial_device_4);
}
DECLARE_HW_IRQ(UART4_IRQn, UART4_IRQHandler, NONE);

void DMA1_Stream2_IRQHandler(int irq_num, void *arg) {
    DmaRxDoneIsr(&serial_4, &serial_driver_4, &serial_device_4);
}
DECLARE_HW_IRQ(DMA1_Stream2_IRQn, DMA1_Stream2_IRQHandler, NONE);
#endif

#ifdef BSP_USING_UART5
struct Stm32Usart serial_5;
struct SerialDriver serial_driver_5;
struct SerialHardwareDevice serial_device_5;

static const struct Stm32UsartDma uart_dma_5 =
{
    DMA1_Stream0,
    DMA_Channel_4,
    DMA_FLAG_TCIF0,
    DMA1_Stream0_IRQn,
    0,
    0,
};

void UART5_IRQHandler(int irq_num, void *arg)
{
    UartIsr(&serial_5, &serial_driver_5, &serial_device_5);
}
DECLARE_HW_IRQ(UART5_IRQn, UART5_IRQHandler, NONE);

void DMA1_Stream0_IRQHandler(int irq_num, void *arg) {
    DmaRxDoneIsr(&serial_5, &serial_driver_5, &serial_device_5);
}
DECLARE_HW_IRQ(DMA1_Stream0_IRQn, DMA1_Stream0_IRQHandler, NONE);
#endif

static uint32 Stm32SerialDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    int serial_operation_cmd;
    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;

    switch (configure_info->configure_cmd)
    {
    case OPE_INT:
        ret = Stm32SerialInit(serial_drv, configure_info);
        break;
    case OPE_CFG:
        serial_operation_cmd = *(int *)configure_info->private_data;
        ret = Stm32SerialConfigure(serial_drv, serial_operation_cmd);
        break;
    default:
        break;
    }

    return ret;
}

static const struct SerialDataCfg data_cfg_init = 
{
    .serial_baud_rate = BAUD_RATE_115200,
    .serial_data_bits = DATA_BITS_8,
    .serial_stop_bits = STOP_BITS_1,
    .serial_parity_mode = PARITY_NONE,
    .serial_bit_order = BIT_ORDER_LSB,
    .serial_invert_mode = NRZ_NORMAL,
    .serial_buffer_size = SERIAL_RB_BUFSZ,
};

/*manage the serial device operations*/
static const struct SerialDrvDone drv_done =
{
    .init = Stm32SerialInit,
    .configure = Stm32SerialConfigure,
};

/*manage the serial device hal operations*/
static struct SerialHwDevDone hwdev_done =
{
    .put_char = Stm32SerialPutchar,
    .get_char = Stm32SerialGetchar,
};

static int BoardSerialBusInit(struct SerialBus *serial_bus, struct SerialDriver *serial_driver, const char *bus_name, const char *drv_name)
{
    x_err_t ret = EOK;

    /*Init the serial bus */
    ret = SerialBusInit(serial_bus, bus_name);
    if(EOK != ret){
        KPrintf("hw_serial_init SerialBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the serial driver*/
    ret = SerialDriverInit(serial_driver, drv_name);
    if(EOK != ret){
        KPrintf("hw_serial_init SerialDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the serial driver to the serial bus*/
    ret = SerialDriverAttachToBus(drv_name, bus_name);
    if(EOK != ret){
        KPrintf("hw_serial_init SerialDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the serial device to the serial bus*/
static int BoardSerialDevBend(struct SerialHardwareDevice *serial_device, void *serial_param, const char *bus_name, const char *dev_name)
{
    x_err_t ret = EOK;

    ret = SerialDeviceRegister(serial_device, serial_param, dev_name);
    if(EOK != ret){
        KPrintf("hw_serial_init SerialDeviceInit device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    ret = SerialDeviceAttachToBus(dev_name, bus_name);
    if(EOK != ret){
        KPrintf("hw_serial_init SerialDeviceAttachToBus device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    return  ret;
}

int Stm32HwUsartInit(void)
{
    x_err_t ret = EOK;

    RCCConfiguration();
    GPIOConfiguration();

#ifdef BSP_USING_USART1
    static struct SerialCfgParam serial_cfg_1;
    memset(&serial_cfg_1, 0, sizeof(struct SerialCfgParam));

    static struct UsartHwCfg serial_hw_cfg_1;
    memset(&serial_hw_cfg_1, 0, sizeof(struct UsartHwCfg));

    static struct SerialDevParam serial_dev_param_1;
    memset(&serial_dev_param_1, 0, sizeof(struct SerialDevParam));

    serial_1.dma = usart_dma_1;
    
    serial_driver_1.drv_done = &drv_done;
    serial_driver_1.configure = &Stm32SerialDrvConfigure;
    serial_device_1.hwdev_done = &hwdev_done;

    serial_cfg_1.data_cfg = data_cfg_init;

    serial_hw_cfg_1.uart_device = USART1;
    serial_hw_cfg_1.irq = USART1_IRQn;
    serial_cfg_1.hw_cfg.private_data = (void *)&serial_hw_cfg_1;
    serial_driver_1.private_data = (void *)&serial_cfg_1;

    serial_dev_param_1.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_1.haldev.private_data = (void *)&serial_dev_param_1;

    NVIC_Configuration(serial_hw_cfg_1.irq);

    ret = BoardSerialBusInit(&serial_1.SerialBus, &serial_driver_1, SERIAL_BUS_NAME_1, SERIAL_DRV_NAME_1);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart1 error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_1, (void *)&serial_cfg_1, SERIAL_BUS_NAME_1, SERIAL_1_DEVICE_NAME_0);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart1 error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_USART2
    static struct SerialCfgParam serial_cfg_2;
    memset(&serial_cfg_2, 0, sizeof(struct SerialCfgParam));

    static struct UsartHwCfg serial_hw_cfg_2;
    memset(&serial_hw_cfg_2, 0, sizeof(struct UsartHwCfg));

    static struct SerialDevParam serial_dev_param_2;
    memset(&serial_dev_param_2, 0, sizeof(struct SerialDevParam));

    serial_2.dma = usart_dma_2;
    
    serial_driver_2.drv_done = &drv_done;
    serial_driver_2.configure = &Stm32SerialDrvConfigure;
    serial_device_2.hwdev_done = &hwdev_done;

    serial_cfg_2.data_cfg = data_cfg_init;
    serial_cfg_2.data_cfg.serial_baud_rate=BAUD_RATE_115200;

    serial_hw_cfg_2.uart_device = USART2;
    serial_hw_cfg_2.irq = USART2_IRQn;
    serial_cfg_2.hw_cfg.private_data = (void *)&serial_hw_cfg_2;
    serial_driver_2.private_data = (void *)&serial_cfg_2;

    serial_dev_param_2.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_2.haldev.private_data = (void *)&serial_dev_param_2;

    NVIC_Configuration(serial_hw_cfg_2.irq);

    ret = BoardSerialBusInit(&serial_2.SerialBus, &serial_driver_2, SERIAL_BUS_NAME_2, SERIAL_DRV_NAME_2);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart2 error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_2, (void *)&serial_cfg_2, SERIAL_BUS_NAME_2, SERIAL_2_DEVICE_NAME_0);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart2 error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_USART3
    static struct SerialCfgParam serial_cfg_3;
    memset(&serial_cfg_3, 0, sizeof(struct SerialCfgParam));

    static struct UsartHwCfg serial_hw_cfg_3;
    memset(&serial_hw_cfg_3, 0, sizeof(struct UsartHwCfg));

    static struct SerialDevParam serial_dev_param_3;
    memset(&serial_dev_param_3, 0, sizeof(struct SerialDevParam));

    serial_3.dma = usart_dma_3;
    
    serial_driver_3.drv_done = &drv_done;
    serial_driver_3.configure = &Stm32SerialDrvConfigure;
    serial_device_3.hwdev_done = &hwdev_done;

    serial_cfg_3.data_cfg = data_cfg_init;
    serial_cfg_3.data_cfg.serial_baud_rate=BAUD_RATE_57600;

    serial_hw_cfg_3.uart_device = USART3;
    serial_hw_cfg_3.irq = USART3_IRQn;
    serial_cfg_3.hw_cfg.private_data = (void *)&serial_hw_cfg_3;
    serial_driver_3.private_data = (void *)&serial_cfg_3;

    serial_dev_param_3.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_3.haldev.private_data = (void *)&serial_dev_param_3;

    NVIC_Configuration(serial_hw_cfg_3.irq);

    ret = BoardSerialBusInit(&serial_3.SerialBus, &serial_driver_3, SERIAL_BUS_NAME_3, SERIAL_DRV_NAME_3);
    if(EOK != ret) {
        KPrintf("Stm32HwUsartInit usart3 error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_3, (void *)&serial_cfg_3, SERIAL_BUS_NAME_3, SERIAL_3_DEVICE_NAME_0);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart3 error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_UART4
    static struct SerialCfgParam serial_cfg_4;
    memset(&serial_cfg_4, 0, sizeof(struct SerialCfgParam));

    static struct UsartHwCfg serial_hw_cfg_4;
    memset(&serial_hw_cfg_4, 0, sizeof(struct UsartHwCfg));

    static struct SerialDevParam serial_dev_param_4;
    memset(&serial_dev_param_4, 0, sizeof(struct SerialDevParam));

    serial_4.dma = uart_dma_4;
    
    serial_driver_4.drv_done = &drv_done;
    serial_driver_4.configure = &Stm32SerialDrvConfigure;
    serial_device_4.hwdev_done = &hwdev_done;

    serial_cfg_4.data_cfg = data_cfg_init;

    serial_hw_cfg_4.uart_device = UART4;
    serial_hw_cfg_4.irq = UART4_IRQn;
    serial_cfg_4.hw_cfg.private_data = (void *)&serial_hw_cfg_4;
    serial_driver_4.private_data = (void *)&serial_cfg_4;

    serial_dev_param_4.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_4.haldev.private_data = (void *)&serial_dev_param_4;

    NVIC_Configuration(serial_hw_cfg_4.irq);

    ret = BoardSerialBusInit(&serial_4.SerialBus, &serial_driver_4, SERIAL_BUS_NAME_4, SERIAL_DRV_NAME_4);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart4 error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_4, (void *)&serial_cfg_4, SERIAL_BUS_NAME_4, SERIAL_4_DEVICE_NAME_0);
    if(EOK != ret) {
        KPrintf("Stm32HwUsartInit usart4 error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_UART5
    static struct SerialCfgParam serial_cfg_5;
    memset(&serial_cfg_5, 0, sizeof(struct SerialCfgParam));

    static struct UsartHwCfg serial_hw_cfg_5;
    memset(&serial_hw_cfg_5, 0, sizeof(struct UsartHwCfg));

    static struct SerialDevParam serial_dev_param_5;
    memset(&serial_dev_param_5, 0, sizeof(struct SerialDevParam));

    serial_5.dma = uart_dma_5;
    
    serial_driver_5.drv_done = &drv_done;
    serial_driver_5.configure = &Stm32SerialDrvConfigure;
    serial_device_5.hwdev_done = &hwdev_done;

    serial_cfg_5.data_cfg = data_cfg_init;

    serial_hw_cfg_5.uart_device = UART5;
    serial_hw_cfg_5.irq = UART5_IRQn;
    serial_cfg_5.hw_cfg.private_data = (void *)&serial_hw_cfg_5;
    serial_driver_5.private_data = (void *)&serial_cfg_5;

    serial_dev_param_5.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_5.haldev.private_data = (void *)&serial_dev_param_5;

    NVIC_Configuration(serial_hw_cfg_5.irq);

    ret = BoardSerialBusInit(&serial_5.SerialBus, &serial_driver_5, SERIAL_BUS_NAME_5, SERIAL_DRV_NAME_5);
    if(EOK != ret) {
        KPrintf("Stm32HwUsartInit usart5 error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_5, (void *)&serial_cfg_5, SERIAL_BUS_NAME_5, SERIAL_5_DEVICE_NAME_0);
    if(EOK != ret){
        KPrintf("Stm32HwUsartInit usart5 error ret %u\n", ret);
        return ERROR;
    }    
#endif

    return ret;
}
