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
* @file sd_spi.h
* @brief define spi-sd dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef SD_SPI_H
#define SD_SPI_H

#include <bus_spi.h>
#include <dev_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_SD_FREQUENCY                 400000
#define SPI_SD_TIMEOUT_NUM                  100
#define SD_CMD_RESPONE_LENGTH            5
#define SD_CMD_CSD_LENGTH                     16
#define SD_BLOCK_LENGTH                         512

#define SD_TIMEOUT(cnt, time)                      \
do \
{                                                                                 \
    cnt++;                                           \
    if(cnt >= time)                  \
    {                                                              \
        KPrintf("SD_TMEOUT %s %d\n", __FUNCTION__, __LINE__);                                                  \
        return ETIMEOUT;                                \
    }                                                      \
}while(0)

typedef struct SpiSdDevice *SpiSdDeviceType;

typedef enum
{
    SD_CMD_0 = 0,
    SD_CMD_1,
    SD_CMD_8 = 8,
    SD_CMD_9,
    SD_CMD_12 = 12,
    SD_CMD_16 = 16,
    SD_CMD_17,
    SD_CMD_18,
    SD_ACMD_23 = 23,
    SD_CMD_24,
    SD_CMD_25,
    SD_ACMD_41 = 41,
    SD_CMD_55 = 55,
    SD_CMD_58 = 58,
}SdCmd;

typedef enum
{
    SD_RESPONE_1 = 1,
    SD_RESPONE_1B,
    SD_RESPONE_2,
    SD_RESPONE_3,
    SD_RESPONE_7,
}SdRespone;

struct SdCmdParam
{
    SdCmd sd_cmd_type;
    uint8 sd_cmd_crc;
    uint32 sd_cmd_args;
    SdRespone sd_respone_type;
    uint8 sd_respone_data[SD_CMD_RESPONE_LENGTH];
};

struct SdRespone1
{
    uint8 sd_respone_r1;
};

struct SdRespone2
{
    uint16 sd_respone_r2;
};

typedef enum
{
    SD_TYPE_UNKNOW = 0,
    SD_TYPE_MMC,
    SD_TYPE_V1,
    SD_TYPE_V2,
    SD_TYPE_SDHC,
    SD_TYPE_SDXC,
}SdType;

struct BlockDevParam
{
    uint32 sector_num;                      
    uint32 sector_bytes;                 
    uint32 block_size;                         
};

struct CSDRegData
{
    uint8 sd_csd_data[SD_CMD_CSD_LENGTH];
    uint8 sd_csd_structure;
    uint32 sd_csd_csize;
    uint8 sd_csd_csize_multi;
    uint8 sd_csd_readbl_len;
    uint8 sd_csd_transpeed;
    uint8 sd_csd_sectorsize;
    uint32 sd_csd_capacity;
};

struct SdDevParam
{
    SdType sd_type;
    uint32 sd_spi_maxfreq;

    struct BlockDevParam block_param;
    struct CSDRegData csd_data;
};

struct SdDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct SpiSdDevice
{
    struct SpiHardwareDevice *spi_dev;
    struct SdDevParam sd_param;
    struct SpiHardwareDevice sd_dev;
};

SpiSdDeviceType SpiSdInit(struct Bus *bus, const char *dev_name, const char *drv_name, const char *sd_name);

#ifdef __cplusplus
}
#endif

#endif
