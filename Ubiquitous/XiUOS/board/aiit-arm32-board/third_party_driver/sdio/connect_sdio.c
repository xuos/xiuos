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
* @file connect_sdio.c
* @brief support sdio function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-22
*/

/*************************************************
File name: connect_sdio.c
Description: support sdio function using bus driver framework
Others: hardware/sdio/sdio.c for references
History: 
1. Date: 2021-04-22
Author: AIIT XUOS Lab
Modification: Support aiit-arm32-board sdio configure, write and read functions
*************************************************/

#include <connect_sdio.h>
#include "sdio_sd.h"

#ifndef SDCARD_SECTOR_SIZE
#define SDCARD_SECTOR_SIZE 512
#endif

static uint32 SdioConfigure(void *drv, struct BusConfigureInfo *ConfigureInfo)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(ConfigureInfo);

    if (ConfigureInfo->configure_cmd == OPER_BLK_GETGEOME) {
        NULL_PARAM_CHECK(ConfigureInfo->private_data);
        struct DeviceBlockArrange *args = (struct DeviceBlockArrange *)ConfigureInfo->private_data;
        SD_GetCardInfo(&SDCardInfo);

        args->size_perbank = SDCardInfo.CardBlockSize;
        args->block_size = SDCardInfo.CardBlockSize;
        if (SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
            args->bank_num = (SDCardInfo.SD_csd.DeviceSize + 1)  * 1024;
        else
            args->bank_num = SDCardInfo.CardCapacity;
    }

    return EOK;
}

static int SDLock = -1;
static uint32 SDBuffer[512/sizeof(uint32_t)];

static uint32 SdioOpen(void *dev)
{
    NULL_PARAM_CHECK(dev);

    if (SDLock >= 0) {
        KSemaphoreDelete(SDLock);
    }
    SDLock = KSemaphoreCreate(1);
    if (SDLock < 0) {
        return ERROR;
    }

    return EOK;
}

static uint32 SdioClose(void *dev)
{
    NULL_PARAM_CHECK(dev);

    KSemaphoreDelete(SDLock);

    return EOK;
}

static uint32 SdioRead(void *dev, struct BusBlockReadParam *read_param)
{
    uint8 ret = SD_OK;

    KSemaphoreObtain(SDLock, WAITING_FOREVER);

    if (((uint32)read_param->buffer & 0x03) != 0) {
        uint64_t sector;
        uint8_t* temp;

        sector = (uint64_t)read_param->pos * SDCARD_SECTOR_SIZE;
        temp = (uint8_t*)read_param->buffer;

        for (uint8 i = 0; i < read_param->size; i++) {
            ret = SD_ReadBlock((uint8_t *)SDBuffer, sector, 1);
            if (ret != SD_OK) {
                KPrintf("read failed: %d, buffer 0x%08x\n", ret, temp);
                return 0;
            }
#if defined (SD_DMA_MODE)
            ret = SD_WaitReadOperation();
            if (ret != SD_OK) {
                KPrintf("read failed: %d, buffer 0x%08x\n", ret, temp);
                return 0;
            }
#endif
            memcpy(temp, SDBuffer, SDCARD_SECTOR_SIZE);

            sector += SDCARD_SECTOR_SIZE;
            temp += SDCARD_SECTOR_SIZE;
        }
    } else {
        ret = SD_ReadBlock((uint8_t *)read_param->buffer, (uint64_t)read_param->pos * SDCARD_SECTOR_SIZE, read_param->size);
        if (ret != SD_OK) {
            KPrintf("read failed: %d, buffer 0x%08x\n", ret, (uint8_t *)read_param->buffer);
            return 0;
        }
#if defined (SD_DMA_MODE)
        ret = SD_WaitReadOperation();
        if (ret != SD_OK) {
            KPrintf("read failed: %d, buffer 0x%08x\n", ret, (uint8_t *)read_param->buffer);
            return 0;
        }
#endif
    }

    KSemaphoreAbandon(SDLock);

    return read_param->size;
}

static uint32 SdioWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    uint8 ret = SD_OK;

    KSemaphoreObtain(SDLock, WAITING_FOREVER);

    if (((uint32)write_param->buffer & 0x03) != 0) {
        uint64_t sector;
        uint8_t* temp;

        sector = (uint64_t)write_param->pos * SDCARD_SECTOR_SIZE;
        temp = (uint8_t*)write_param->buffer;

        for (uint8 i = 0; i < write_param->size; i++) {
            memcpy(SDBuffer, temp, SDCARD_SECTOR_SIZE);

            ret = SD_WriteBlock((uint8_t *)SDBuffer, sector, 1);
            if (ret != SD_OK) {
                KPrintf("write failed: %d, buffer 0x%08x\n", ret, temp);
                return 0;
            }
#if defined (SD_DMA_MODE)
            ret = SD_WaitWriteOperation();
            if (ret != SD_OK) {
                KPrintf("write failed: %d, buffer 0x%08x\n", ret, temp);
                return 0;
            }
#endif
            sector += SDCARD_SECTOR_SIZE;
            temp += SDCARD_SECTOR_SIZE;
        }
    } else {
        ret = SD_WriteBlock((uint8_t *)write_param->buffer, (uint64_t)write_param->pos * SDCARD_SECTOR_SIZE, write_param->size);
        if (ret != SD_OK) {
            KPrintf("write failed: %d, buffer 0x%08x\n", ret, (uint8_t *)write_param->buffer);
            return 0;
        }
#if defined (SD_DMA_MODE)
        ret = SD_WaitWriteOperation();
        if (ret != SD_OK) {
            KPrintf("write failed: %d, buffer 0x%08x\n", ret, (uint8_t *)write_param->buffer);
            return 0;
        }
#endif
    }

    KSemaphoreAbandon(SDLock);

    return write_param->size;
}

static struct SdioDevDone dev_done = 
{
    SdioOpen,
    SdioClose,
    SdioWrite,
    SdioRead,
};

int HwSdioInit(void)
{
    static struct SdioBus bus;
    static struct SdioDriver drv;
    static struct SdioHardwareDevice dev;

    x_err_t ret = EOK;
    ret = SD_Init();
    if (ret != SD_OK) {
        KPrintf("SD init failed!"); 
        return ERROR;
    }
    
    ret = SdioBusInit(&bus, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio bus init error %d\n", ret);
        return ERROR;
    }

    ret = SdioDriverInit(&drv, SDIO_DRIVER_NAME);
    if (ret != EOK) {
        KPrintf("Sdio driver init error %d\n", ret);
        return ERROR;
    }
    ret = SdioDriverAttachToBus(SDIO_DRIVER_NAME, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio driver attach error %d\n", ret);
        return ERROR;
    }

    dev.dev_done = &dev_done;
    ret = SdioDeviceRegister(&dev, SDIO_DEVICE_NAME);
    if (ret != EOK) {
        KPrintf("Sdio device register error %d\n", ret);
        return ERROR;
    }
    ret = SdioDeviceAttachToBus(SDIO_DEVICE_NAME, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio device register error %d\n", ret);
        return ERROR;
    }

    return ret;
}
