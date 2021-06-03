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
* @file sfud_port.h
* @brief define spi flash function and struct using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef SFUD_PORT_H
#define SFUD_PORT_H

#include <device.h>
#include <sfud.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_FLASH_FREQUENCY 10000000

typedef struct SpiFlashDevice *SpiFlashDeviceType;

struct FlashDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct FlashBlockParam
{
    uint32 sector_num;                      
    uint32 sector_bytes;                 
    uint32 block_size;                         
};

struct SpiFlashParam
{
    struct FlashBlockParam flash_block_param;
    void *flash_private_data;
};

struct SpiFlashDevice
{
    struct SpiHardwareDevice *spi_dev;
    
    struct SpiFlashParam flash_param;
    struct SpiHardwareDevice flash_dev;

    int flash_lock;
};

#ifdef __cplusplus
}
#endif

#endif


