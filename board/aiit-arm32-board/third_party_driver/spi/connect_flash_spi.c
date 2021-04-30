/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-27     SummerGift   add spi flash port file
 */

/**
* @file connect_flash_spi.c
* @brief support aiit-arm32-board spi flash function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_flash_spi.c
Description: support aiit-arm32-board spi flash bus register function
Others: take RT-Thread v4.0.2/bsp/stm32/stm32f407-atk-explorer/board/ports/spi-flash-init.c
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board spi flash register to spi bus
2. support aiit-arm32-board spi flash init
*************************************************/

#include <connect_spi.h>
#include <flash_spi.h>
#include <hardware_gpio.h>

int FlashW25qxxSpiDeviceInit(void)
{
#ifdef BSP_USING_SPI1

    __IO uint32_t tmpreg = 0x00U;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    tmpreg = RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN;
    (void)tmpreg;

    if (EOK != HwSpiDeviceAttach(SPI_BUS_NAME_1, "spi1_dev0", GPIOB, GPIO_Pin_0)) {
        return ERROR;
    }

    if (NONE == SpiFlashInit(SPI_BUS_NAME_1, "spi1_dev0", SPI_1_DRV_NAME, "W25Q64")) {
        return ERROR;
    }

#endif

    return EOK;
}