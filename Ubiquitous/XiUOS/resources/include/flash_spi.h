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
* @file flash_spi.h
* @brief define spi-flash dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef FLASH_SPI_H
#define FLASH_SPI_H

#include <sfud_port.h>

#ifdef __cplusplus
extern "C" {
#endif

SpiFlashDeviceType SpiFlashInit(char *bus_name, char *dev_name, char *drv_name, char *flash_name);

uint32 SpiFlashRelease(SpiFlashDeviceType spi_flash_dev);

sfud_flash_t SpiFlashFind(char *flash_name);

#ifdef __cplusplus
}
#endif

#endif
