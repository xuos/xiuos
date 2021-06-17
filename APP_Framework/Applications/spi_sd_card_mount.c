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
 * @file spi_sd_card_mount.c
 * @brief Mount SD card when opened SPI SD card
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.01
 */

#include "user_api/switch_api/user_api.h"
#include <stdio.h>

#if defined(FS_VFS)
#include <iot-vfs.h>

/**
 * @description: Mount SD card
 * @return 0
 */
int MountSDCard(void)
{
    struct Bus *spi_bus;
    spi_bus = BusFind(SPI_BUS_NAME_1);

    if (NONE == SpiSdInit(spi_bus, SPI_1_DEVICE_NAME_0, SPI_1_DRV_NAME, SPI_SD_NAME)) {
        KPrintf("MountSDCard SpiSdInit error!\n");
        return 0;
    }
    
    if (EOK == MountFilesystem(SPI_BUS_NAME_1, SPI_SD_NAME, SPI_1_DRV_NAME, FSTYPE_FATFS, "/"))
        KPrintf("SPI SD card fatfs mounted\n");

    return 0;
}
#endif
