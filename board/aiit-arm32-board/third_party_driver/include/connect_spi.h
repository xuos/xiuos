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
* @file connect_spi.h
* @brief define aiit-arm32-board spi function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef CONNECT_SPI_H
#define CONNECT_SPI_H

#include <device.h>
#include <hardware_spi.h>
#include <hardware_dma.h>
#include <stm32f4xx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_USING_RX_DMA_FLAG   (1<<0)
#define SPI_USING_TX_DMA_FLAG   (1<<1)

struct Stm32HwSpiCs
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

struct Stm32SpiDma
{
    uint32_t dma_rcc;
    DMA_Stream_TypeDef *instance;
    uint32 channel;
    IRQn_Type dma_irq;
    DMA_InitTypeDef init;
    x_size_t setting_len;
    x_size_t last_index;
};

struct Stm32Spi
{
    SPI_TypeDef *instance;

    char *bus_name;

    SPI_InitTypeDef init;

    struct
    {
        struct Stm32SpiDma dma_rx;
        struct Stm32SpiDma dma_tx;
    }dma;

    uint8 spi_dma_flag;
    struct SpiBus spi_bus;
};

int Stm32HwSpiInit(void);
x_err_t HwSpiDeviceAttach(const char *bus_name, const char *device_name, GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin);

#ifdef __cplusplus
}
#endif

#endif
