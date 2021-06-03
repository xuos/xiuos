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
* @file connect_spi_lora.h
* @brief define spi lora dev function and struct using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef CONNECT_SPI_LORA_H
#define CONNECT_SPI_LORA_H

#include <bus.h>
#include <connect_spi.h>
#include <device.h>
#include <radio.h>
#include <hardware_gpio.h>
#include <stm32f4xx.h>
#include <spi_lora_sx12xx.h>
#include <sx1276.h>
#include <sx1276-Hal.h>
#include <sx1276-LoRa.h>
#include <sx1276-LoRaMisc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_LORA_FREQUENCY 10000000
#define SPI_LORA_BUFFER_SIZE 256

typedef struct SpiLoraDevice *SpiLoraDeviceType;

struct LoraDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct SpiLoraDevice
{
    struct SpiHardwareDevice *spi_dev;
    struct SpiHardwareDevice lora_dev;
};

#ifdef __cplusplus
}
#endif

#endif
