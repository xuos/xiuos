/*
 * File      : spi_flash_sfud.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2016, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-09-28     armink       first version.
 */

/**
* @file test_spi_flash.c
* @brief support to test spi flash function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-17
*/

/*************************************************
File name: test_spi_flash.c
Description: support spi flash function test
Others: add spi flash test cmd from SFUD/blob/master/demo/stm32f2xx_rtt/RT-Thread-2.1.0/components/drivers/spi/spi_flash_sfud.c
                https://github.com/armink/SFUD/
History: 
1. Date: 2021-05-17
Author: AIIT XUOS Lab
Modification: 
1. support spi flash open, read and write function
*************************************************/

#include <xiuos.h>
#include <device.h>
#include <flash_spi.h>
#include <user_api.h>

#define SPI_FLASH_PATH "/dev/spi1_W25Q64"
#define FlashDataPrint(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

static int spi_flash_fd;

void FlashOpen(void)
{
    x_err_t ret = EOK;

    spi_flash_fd = open(SPI_FLASH_PATH, O_RDWR);
    if (spi_flash_fd < 0) {
        KPrintf("open spi flash fd error %d\n", spi_flash_fd);
    }

    KPrintf("Spi Flash init succeed\n");

    return;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                FlashOpen, FlashOpen,  open spi flash device);

void FlashRead(int argc, char *argv[])
{
    x_size_t i, j = 0;
    uint32 addr;
    uint32 size;
    uint8 data[16];

    struct BusBlockReadParam read_param;
    memset(&read_param, 0, sizeof(struct BusBlockReadParam));

    memset(data, 0, 16);

    if (3 != argc) {
        KPrintf("FlashRead cmd format: FlashRead addr size.\n");
        return;
    } else {
        addr = strtol(argv[1], NULL, 0);
        size = strtol(argv[2], NULL, 0);

        read_param.buffer = data;
        read_param.pos = addr;
        read_param.size = size;

        if (read_param.buffer) {
            read(spi_flash_fd, &read_param, size);
            if (size == read_param.read_length) {
                KPrintf("Read the %s flash data success. Start from 0x%08X, size is %ld. The data is:\n",
                        SPI_FLASH_PATH, addr, size);
                KPrintf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
                for (i = 0; i < size; i += 16) {
                    KPrintf("[%08X] ", addr + i);
                    /* dump hex */
                    for (j = 0; j < 16; j++) {
                        if (i + j < size) {
                            KPrintf("%02X ", data[i + j]);
                        } else {
                            KPrintf("   ");
                        }
                    }
                    /* dump char for hex */
                    for (j = 0; j < 16; j++) {
                        if (i + j < size) {
                            KPrintf("%c", FlashDataPrint(data[i + j]) ? data[i + j] : '.');
                        }
                    }
                    KPrintf("\n");
                }
                KPrintf("\n");
            }
        } else {
            KPrintf("SpiFlashRead alloc read buffer failed!\n");
        }
    }

    return;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                FlashRead, FlashRead,  read data from spi flash device);

void FlashWrite(int argc, char *argv[])
{
    x_err_t ret = EOK;
    x_size_t i, j = 0;
    uint32 addr;
    uint32 size;
    uint8 data[16];

    struct BusBlockWriteParam write_param;
    memset(&write_param, 0, sizeof(struct BusBlockWriteParam));

    memset(data, 0, 16);

    if (argc < 3) {
        KPrintf("FlashWrite cmd format: FlashWrite addr data.\n");
        return;
    } else {
        addr = strtol(argv[1], NULL, 0);
        size = argc - 2;

        write_param.buffer = data;
        write_param.pos = addr;
        write_param.size = size;

        if (data) {
            for (i = 0; i < size; i++) {
                data[i] = strtol(argv[2 + i], NULL, 0);
            }

            ret = write(spi_flash_fd, &write_param, size);
            if (EOK == ret) {
                KPrintf("Write the %s flash data success. Start from 0x%08X, size is %ld.\n",
                        SPI_FLASH_PATH, addr, size);
                KPrintf("Write data: ");
                for (i = 0; i < size; i++) {
                    KPrintf("%d ", data[i]);
                }
                KPrintf(".\n");
            }
        } else {
            KPrintf("SpiFlashWrite alloc write buffer failed!\n");
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                FlashWrite, FlashWrite,  write data to spi flash device);
