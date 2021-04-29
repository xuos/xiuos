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
* @file sd_spi.c
* @brief support spi-sd dev function using spi bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <sd_spi.h>

static struct SpiSdDevice g_spi_sd_dev;
static struct SdCmdParam g_sd_cmd_param;

static uint32 SdReady(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    uint8 data = 0xFF;
    uint8 read;
    uint32 start_time;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    write_param.buffer = (void *)&data;
    write_param.size = 1;
    read_param.buffer = (void *)&read;
    read_param.size = 1;

    start_time = 0;
    do
    {
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

        BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

        SD_TIMEOUT(start_time, 10 * SPI_SD_TIMEOUT_NUM);
    }while(0xFF != read);

    return EOK;
}

static uint32 SdSendCmdByte(SpiSdDeviceType spi_sd_dev, struct SdCmdParam *sd_cmd_param)
{
    NULL_PARAM_CHECK(spi_sd_dev);
    NULL_PARAM_CHECK(&spi_sd_dev->spi_dev->haldev);
    NULL_PARAM_CHECK(sd_cmd_param);

    x_err_t ret = EOK;
    uint32 start_time;
    uint8 i;
    uint8 write[8], read[8];
    uint8 write_byte, read_byte;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    ret = SdReady(spi_sd_dev);
    if (EOK != ret) {
        return ret;
    }

    /*8 Bytes CMD : \0xFF\(CMD \ 0x40)\CMD ARG0\CMD ARG1\CMD ARG2\CMD ARG3\(CRC << 1 \ 0x1)\0xFF\*/
    write[0] = 0xFF;
    write[1] = sd_cmd_param->sd_cmd_type | 0x40;
    for (i = 2; i < 6; i ++) {
        write[i] = (uint8)(sd_cmd_param->sd_cmd_args >> ((5- i) * 8) );
    }
    write[6] = (uint8)sd_cmd_param->sd_cmd_crc;//CRC is not valid using SPI SD

    write[7] = 0xFF;

    write_param.buffer = (void *)write;
    write_param.size = 8;
    BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

    start_time = 0;
    do
    {
        read_param.buffer = (void *)&read;
        read_param.size = 1;
        BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

        if ((SD_CMD_17  == sd_cmd_param->sd_cmd_type) || (SD_CMD_18  == sd_cmd_param->sd_cmd_type)) {
            MdelayKTask(100);
        }

        SD_TIMEOUT(start_time, 2 * SPI_SD_TIMEOUT_NUM);
    }while((0 != (read[0] & 0x80)));

    switch (sd_cmd_param->sd_respone_type)
    {
        case SD_RESPONE_1 :
            sd_cmd_param->sd_respone_data[0] = read[0];
            return EOK;
        case SD_RESPONE_1B :
            sd_cmd_param->sd_respone_data[0] = read[0];

            start_time = 0;
            do
            {
                read_param.buffer = (void *)&read_byte;
                read_param.size = 1;
                BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

                SD_TIMEOUT(start_time, 2 * SPI_SD_TIMEOUT_NUM);
            }while(0xFF != read_byte);

            return EOK;
        case SD_RESPONE_2 :
            sd_cmd_param->sd_respone_data[0] = read[0];

            read_param.buffer = (void *)&read_byte;
            read_param.size = 1;
            BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

            sd_cmd_param->sd_respone_data[1] = read_byte;

            return EOK;
        case SD_RESPONE_3 :
        case SD_RESPONE_7 :
            sd_cmd_param->sd_respone_data[0] = read[0];

            read_param.buffer = (void *)&read;
            read_param.size = 4;
            BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

            sd_cmd_param->sd_respone_data[1] = read[0];
            sd_cmd_param->sd_respone_data[2] = read[1];
            sd_cmd_param->sd_respone_data[3] = read[2];
            sd_cmd_param->sd_respone_data[4] = read[3];

            return EOK;
        default:
            break;
    }

    KPrintf("SdSendCmdByte not support respone type %u\n", sd_cmd_param->sd_respone_type);
    return ERROR;
}

static uint32 SdSpiConfigure(struct Driver *spi_drv, struct SpiMasterParam *spi_master_param)
{
    NULL_PARAM_CHECK(spi_drv);
    NULL_PARAM_CHECK(spi_master_param);
    x_err_t ret = EOK;

    struct BusConfigureInfo configure_info;

    configure_info.configure_cmd = OPE_CFG;
    configure_info.private_data = (void *)spi_master_param;
    ret = BusDrvConfigure(spi_drv, &configure_info);
    if (ret) {
        KPrintf("spi drv OPE_CFG error drv %8p cfg %8p\n", spi_drv, spi_master_param);
        return ERROR;
    }

    configure_info.configure_cmd = OPE_INT;
    ret = BusDrvConfigure(spi_drv, &configure_info);
    if (ret) {
        KPrintf("spi drv OPE_INT error drv %8p\n", spi_drv);
        return ERROR;
    }
    
    return ret;
}

static void SdHwInitCs(SpiSdDeviceType spi_sd_dev)
{
    uint8 i;
    uint8 data = 0xFF;

    struct BusBlockWriteParam write_param;

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    for (i = 0 ; i < 10 ; i ++) {
        write_param.buffer = (void *)&data;
        write_param.size = 1;
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);
    }
}

static uint32 SdHwInit(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint32 start_time;

    g_sd_cmd_param.sd_cmd_type = SD_CMD_0;
    g_sd_cmd_param.sd_cmd_args = 0x00;
    g_sd_cmd_param.sd_cmd_crc = 0x95;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    start_time = 0;

    do
    {
        /*pull down the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

        ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
        if (EOK != ret) {
            KPrintf("SdHwInit send CMD0 error %d\n", ret);
        }

        /*pull up the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

        SD_TIMEOUT(start_time, 3 * SPI_SD_TIMEOUT_NUM);
    }
    while(0x01 != g_sd_cmd_param.sd_respone_data[0]);

    return EOK;
}

static uint32 SdConfirmType(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint32 start_time;

    g_sd_cmd_param.sd_cmd_type = SD_CMD_8;
    g_sd_cmd_param.sd_cmd_args = 0x01AA;
    g_sd_cmd_param.sd_cmd_crc = 0x87;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_7;

    start_time = 0;

    do
    {
        /*pull down the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

        ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
        if (EOK != ret) {
            KPrintf("SdConfirmType send CMD8 error %d\n", ret);
        }

        /*pull up the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

        /*Only SD2.0 support CMD8, return 0x01*/
        if (0x01 == g_sd_cmd_param.sd_respone_data[0]) {
            spi_sd_dev->sd_param.sd_type = SD_TYPE_V2;

            if (0x01 != g_sd_cmd_param.sd_respone_data[3]) {
                KPrintf("SdConfirmType CMD8 working voltage is not 2.0V\n");
                return ERROR;
            }
        } else {
            spi_sd_dev->sd_param.sd_type = SD_TYPE_V1;
        }

        SD_TIMEOUT(start_time, 3 * SPI_SD_TIMEOUT_NUM);
    }
    while(0xAA != g_sd_cmd_param.sd_respone_data[4]);

    return EOK;
}

static uint32 SdHwReset(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint8 sd_mmc_flag = 0;
    uint32 start_time;

    if (SD_TYPE_V2 == spi_sd_dev->sd_param.sd_type) {
        start_time = 0;
        do
        {
            /*Step1 : CMD55, return 0x01*/
            g_sd_cmd_param.sd_cmd_type = SD_CMD_55;
            g_sd_cmd_param.sd_cmd_args = 0x00;
            g_sd_cmd_param.sd_cmd_crc = 0x65;
            g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;
            
            /*pull down the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

            ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
            if (EOK != ret) {
                KPrintf("SdHwReset type 2.0 send CMD55 error %d\n", ret);
            }

            if (0x01 != g_sd_cmd_param.sd_respone_data[0]) {
                KPrintf("SdHwReset type 2.0 send CMD55 respone error 0x%x\n", g_sd_cmd_param.sd_respone_data[0]);
            }

            /*Step2 : ACMD41,return 0x00*/
            g_sd_cmd_param.sd_cmd_type = SD_ACMD_41;
            g_sd_cmd_param.sd_cmd_args = 0x40000000;
            g_sd_cmd_param.sd_cmd_crc = 0x77;
            g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

            ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
            if (EOK != ret) {
                KPrintf("SdHwReset type 2.0 send ACMD41 error %d\n", ret);
            }

            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

            SD_TIMEOUT(start_time, SPI_SD_TIMEOUT_NUM);
        }while(0x00 != g_sd_cmd_param.sd_respone_data[0]);

        /*Step3 : CMD58, read OCR to verify whether sd 2.0 is SDHC or not*/
        g_sd_cmd_param.sd_cmd_type = SD_CMD_58;
        g_sd_cmd_param.sd_cmd_args = 0x00;
        g_sd_cmd_param.sd_cmd_crc = 0x00;
        g_sd_cmd_param.sd_respone_type = SD_RESPONE_3;
            
        /*pull down the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

        ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
        if (EOK != ret) {
            KPrintf("SdHwReset type 2.0 send CMD58 error %d\n", ret);

            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);  
            return ERROR;
        }

        /*pull up the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);      

        if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
            KPrintf("SdHwReset type 2.0 send CMD58 respone error 0x%x\n", g_sd_cmd_param.sd_respone_data[0]);
            return ERROR;
        }

        if (g_sd_cmd_param.sd_respone_data[1] >> 2) {
            KPrintf("SdHwReset SD 2.0 is SDHC!\n");
            spi_sd_dev->sd_param.sd_type = SD_TYPE_SDHC;
        }

        KPrintf("SdHwReset SD 2.0 done!\n");
        return EOK;
    } else if (SD_TYPE_V1 == spi_sd_dev->sd_param.sd_type) {
        start_time = 0;
        do
        {
            /*Step1 : CMD55, return 0x01*/
            g_sd_cmd_param.sd_cmd_type = SD_CMD_55;
            g_sd_cmd_param.sd_cmd_args = 0x00;
            g_sd_cmd_param.sd_cmd_crc = 0x65;
            g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;
            
            /*pull down the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

            ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
            if (EOK != ret) {
                KPrintf("SdHwReset type 1.0 send CMD55 error %d\n", ret);
            }

            if (0x01 != g_sd_cmd_param.sd_respone_data[0]) {
                KPrintf("SdHwReset type 1.0 send CMD55 respone 0x%x, might be MMC\n", g_sd_cmd_param.sd_respone_data[0]);
                break;
            }

            /*Step2 : ACMD41,return 0x00*/
            g_sd_cmd_param.sd_cmd_type = SD_ACMD_41;
            g_sd_cmd_param.sd_cmd_args = 0x40000000;
            g_sd_cmd_param.sd_cmd_crc = 0x77;
            g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

            ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
            if (EOK != ret) {
                KPrintf("SdHwReset type 1.0 send ACMD41 error %d\n", ret);
            }

            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

            if ((0x00 != g_sd_cmd_param.sd_respone_data[0]) || (0x01 != g_sd_cmd_param.sd_respone_data[0])) {
                KPrintf("SdHwReset type 1.0 send ACMD41 respone 0x%x, might be MMC\n", g_sd_cmd_param.sd_respone_data[0]);
                sd_mmc_flag = 1;
                break;
            }

            SD_TIMEOUT(start_time, SPI_SD_TIMEOUT_NUM);
        }while(0x00 != g_sd_cmd_param.sd_respone_data[0]);

        if (sd_mmc_flag) {
            SdHwInitCs(spi_sd_dev);
            
            /*Step1 : CMD0, enter IDLE, return 0x01*/
            ret = SdHwInit(spi_sd_dev);
            if (EOK != ret) {
                KPrintf("SdHwReset MMC make sd card enter idle error %d using CMD0\n", ret);
            }

            start_time = 0;

            /*Step2 : CMD1,return 0x00*/
            do
            {
                g_sd_cmd_param.sd_cmd_type = SD_CMD_1;
                g_sd_cmd_param.sd_cmd_args = 0x00;
                g_sd_cmd_param.sd_cmd_crc = 0x00;
                g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

                /*pull down the cs pin*/
                SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

                ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
                if (EOK != ret) {
                    KPrintf("SdHwReset type MMC send CMD1 error %d\n", ret);
                }

                /*pull up the cs pin*/
                SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

                SD_TIMEOUT(start_time, SPI_SD_TIMEOUT_NUM);
            }while(0x00 != g_sd_cmd_param.sd_respone_data[0]);

            KPrintf("SdHwReset SD 1.0 is MMC!\n");
            spi_sd_dev->sd_param.sd_type = SD_TYPE_MMC;
            return EOK;
        }
    } else {
        KPrintf("SdHwReset do not support the sd type %d\n", spi_sd_dev->sd_param.sd_type);
        return ERROR;
    }
}

static uint32 SdHwSetBlockLength(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;

    g_sd_cmd_param.sd_cmd_type = SD_CMD_16;
    g_sd_cmd_param.sd_cmd_args = SD_BLOCK_LENGTH;
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdHwSetBlockLength send CMD16 error %d\n", ret);
    }

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdHwSetBlockLength CMD16 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    spi_sd_dev->sd_param.block_param.block_size = SD_BLOCK_LENGTH;
    spi_sd_dev->sd_param.block_param.sector_bytes = SD_BLOCK_LENGTH;

    return EOK;
}

static void SdGetCSDv1(SpiSdDeviceType spi_sd_dev)
{
    /*CSD reg bit[103 : 96]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_transpeed = spi_sd_dev->sd_param.csd_data.sd_csd_data[3];

    /*CSD reg bit[83 : 80]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_readbl_len = spi_sd_dev->sd_param.csd_data.sd_csd_data[5] & 0x0F;

    /*CSD reg bit[73 : 62]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_csize = 
        ((uint16)(spi_sd_dev->sd_param.csd_data.sd_csd_data[6] & 0x03) << 10) + 
        (uint16)(spi_sd_dev->sd_param.csd_data.sd_csd_data[7] << 2) + 
        (spi_sd_dev->sd_param.csd_data.sd_csd_data[8] >> 6);

    /*CSD reg bit[49 : 47]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_csize_multi = 
        ((spi_sd_dev->sd_param.csd_data.sd_csd_data[9] & 0x03) << 1) + 
        ((spi_sd_dev->sd_param.csd_data.sd_csd_data[10] & 0x80) >> 7);

    /*memory capacity = BLOCKNR*BLOCK_LEN Bytes*/
    /*memory capacity = (C_SIZE+1)*2^(C_SIZE_MULT+2+READ_BL_LEN) Bytes*/
    spi_sd_dev->sd_param.csd_data.sd_csd_capacity = 
        (spi_sd_dev->sd_param.csd_data.sd_csd_csize + 1) *
        (1 << (spi_sd_dev->sd_param.csd_data.sd_csd_csize_multi + 2 + spi_sd_dev->sd_param.csd_data.sd_csd_readbl_len));

    spi_sd_dev->sd_param.block_param.sector_num = 
        spi_sd_dev->sd_param.csd_data.sd_csd_capacity / spi_sd_dev->sd_param.block_param.sector_bytes;

    KPrintf("SdGetCSDv1 transpeed %d read_bl_size %d c_size %d c_size_mult %d capacity %d MB\n",
        spi_sd_dev->sd_param.csd_data.sd_csd_transpeed,
        spi_sd_dev->sd_param.csd_data.sd_csd_readbl_len,
        spi_sd_dev->sd_param.csd_data.sd_csd_csize,
        spi_sd_dev->sd_param.csd_data.sd_csd_csize_multi,
        spi_sd_dev->sd_param.csd_data.sd_csd_capacity / (1024 * 1024));
}

static void SdGetCSDv2(SpiSdDeviceType spi_sd_dev)
{
    /*CSD reg bit[103 : 96]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_transpeed = spi_sd_dev->sd_param.csd_data.sd_csd_data[3];

    /*CSD reg bit[69 : 48]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_csize = 
        (uint32)((spi_sd_dev->sd_param.csd_data.sd_csd_data[7] & 0x3F) << 16) +
        (uint32)(spi_sd_dev->sd_param.csd_data.sd_csd_data[8] << 8) +
        spi_sd_dev->sd_param.csd_data.sd_csd_data[9];

    /*memory capacity = (C_SIZE+1)*512 KBytes*/
    spi_sd_dev->sd_param.csd_data.sd_csd_capacity = 
        (spi_sd_dev->sd_param.csd_data.sd_csd_csize + 1) * 512;

    spi_sd_dev->sd_param.block_param.sector_num = 
        spi_sd_dev->sd_param.csd_data.sd_csd_capacity / spi_sd_dev->sd_param.block_param.sector_bytes;

    KPrintf("SdGetCSDv1 transpeed %d c_size %d capacity %d.%d GB\n",
        spi_sd_dev->sd_param.csd_data.sd_csd_transpeed,
        spi_sd_dev->sd_param.csd_data.sd_csd_csize,
        spi_sd_dev->sd_param.csd_data.sd_csd_capacity / (1024 * 1024),
        ((spi_sd_dev->sd_param.csd_data.sd_csd_capacity / 1024 ) % 1024) * 100 / 1024);
}

static uint32 SdGetMMCCSDParam(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    if (spi_sd_dev->sd_param.csd_data.sd_csd_structure > 2) {
        KPrintf("SdGetMMCCSDParam csd structure error %d\n", spi_sd_dev->sd_param.csd_data.sd_csd_structure);
        return ERROR;
    }

    SdGetCSDv1(spi_sd_dev);

    switch (spi_sd_dev->sd_param.csd_data.sd_csd_transpeed & 0x03)
    {
    case 0 :
        spi_sd_dev->sd_param.sd_spi_maxfreq = 100 * 1000;
        break;
    case 1 :
        spi_sd_dev->sd_param.sd_spi_maxfreq = 1 * 1000 * 1000;
        break;
    case 2 :
        spi_sd_dev->sd_param.sd_spi_maxfreq = 10 * 1000 * 1000;
        break;
    case 3 :
        spi_sd_dev->sd_param.sd_spi_maxfreq = 100 * 1000 * 1000;
        break;
    default:
        KPrintf("SdGetMMCCSDParam sd_csd_transpeed error %d\n", spi_sd_dev->sd_param.csd_data.sd_csd_transpeed);
        spi_sd_dev->sd_param.sd_spi_maxfreq = 100 * 1000;
        break;
    }

    return EOK;
}

static uint32 SdGetCSDParam(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    if (spi_sd_dev->sd_param.csd_data.sd_csd_structure > 1) {
        KPrintf("SdGetCSDParam csd structure error %d\n", spi_sd_dev->sd_param.csd_data.sd_csd_structure);
        return ERROR;
    }

    /*CSD v2.0*/
    if (1 == spi_sd_dev->sd_param.csd_data.sd_csd_structure) {
        SdGetCSDv2(spi_sd_dev);

        switch (spi_sd_dev->sd_param.csd_data.sd_csd_transpeed)
        {
        case 0x32 :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 10 * 1000 * 1000;
            break;
        case 0x5A :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 50 * 1000 * 1000;
            break;
        case 0x0B :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 100 * 1000 * 1000;
            break;
        case 0x2B :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 200 * 1000 * 1000;
            break;
        default:
            KPrintf("SdGetCSDParam 2.0 sd_csd_transpeed %d\n", spi_sd_dev->sd_param.csd_data.sd_csd_transpeed);
            spi_sd_dev->sd_param.sd_spi_maxfreq = 1 * 1000 * 1000;
            break;
        }
    }
    /*CSD v1.0*/
    else if (0 == spi_sd_dev->sd_param.csd_data.sd_csd_structure) {
        SdGetCSDv1(spi_sd_dev);

        switch (spi_sd_dev->sd_param.csd_data.sd_csd_transpeed)
        {
        case 0x32 :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 10 * 1000 * 1000;
            break;
        case 0x5A :
            spi_sd_dev->sd_param.sd_spi_maxfreq = 50 * 1000 * 1000;
            break;
        default:
            KPrintf("SdGetCSDParam 1.0 sd_csd_transpeed %d\n", spi_sd_dev->sd_param.csd_data.sd_csd_transpeed);
            spi_sd_dev->sd_param.sd_spi_maxfreq = 1 * 1000 * 1000;
            break;
        }
    }

    return EOK;
}

static uint32 SdVerifyCSD(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;

    /*CSD reg bit[127 : 126]*/
    spi_sd_dev->sd_param.csd_data.sd_csd_structure = spi_sd_dev->sd_param.csd_data.sd_csd_data[0] >> 6;
    KPrintf("SdVerifyCSD csd structure %d type %d\n", 
        spi_sd_dev->sd_param.csd_data.sd_csd_structure,
        spi_sd_dev->sd_param.sd_type);

    switch (spi_sd_dev->sd_param.sd_type)
    {
        case SD_TYPE_UNKNOW :
            ret = ERROR;
            break;
        case SD_TYPE_MMC :
            ret = SdGetMMCCSDParam(spi_sd_dev);
            break;
        case SD_TYPE_V1 :
        case SD_TYPE_V2 :
        case SD_TYPE_SDHC :
        case SD_TYPE_SDXC :
            ret = SdGetCSDParam(spi_sd_dev);
            break;
        default:
            break;
    }

    return ret;
}

static uint32 SdHwReadCSD(SpiSdDeviceType spi_sd_dev)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint8 i;
    uint8 read[SD_CMD_CSD_LENGTH];

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    /*Step1 : CMD9, return 0x00*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_9;
    g_sd_cmd_param.sd_cmd_args = 0x00;
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_2;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdHwReadCSD send CMD9 error %d\n", ret);
        return ERROR;
    }

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdHwReadCSD CMD9 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    if (0xFE != g_sd_cmd_param.sd_respone_data[1]) {
        /*Step2 : SPI write data 0xFF until read 0xFE*/
        uint8 data = 0xFF;
        uint8 read_spi; 
        uint32 start_time;

        write_param.buffer = (void *)&data;
        write_param.size = 1;
        read_param.buffer = (void *)&read_spi;
        read_param.size = 1;

        start_time = 0;

        do
        {
            BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);
            BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

            SD_TIMEOUT(start_time, 10 * SPI_SD_TIMEOUT_NUM);
        }while(0xFE != read_spi);
    }

    /*Step3 : SPI read 16 bytes CSD register data*/
    read_param.buffer = (void *)read;
    read_param.size = SD_CMD_CSD_LENGTH;
    BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

    memcpy(spi_sd_dev->sd_param.csd_data.sd_csd_data, read, SD_CMD_CSD_LENGTH);

    /*Step4 : SPI read 2 bytes CRC*/
    read_param.buffer = (void *)read;
    read_param.size = 2;
    BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

    /*Step5 : verify CSD data*/
    ret = SdVerifyCSD(spi_sd_dev);
    if (EOK != ret) {
        KPrintf("SdHwReadCSD verify data error %d\n", ret);
        return ERROR;
    }

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    return EOK;
}

static uint32 SdReadSingleBlock(SpiSdDeviceType spi_sd_dev, uint32 id, uint8 *read_buffer)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    /*Step1 : CMD17, return 0x00*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_17;
    if (SD_TYPE_SDHC == spi_sd_dev->sd_param.sd_type) {
        g_sd_cmd_param.sd_cmd_args = id;
    } else {
        g_sd_cmd_param.sd_cmd_args = id * spi_sd_dev->sd_param.block_param.block_size;
    }
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdReadSingleBlock send CMD17 error %d\n", ret);
        return ERROR;
    }

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdReadSingleBlock CMD17 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    /*Step2 : SPI write data 0xFF until read 0xFE*/
    uint8 data = 0xFF;
    uint8 read[2];
    uint32 start_time;

    write_param.buffer = (void *)&data;
    write_param.size = 1;
    read_param.buffer = (void *)read;
    read_param.size = 1;

    start_time = 0;

    do
    {
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);
        BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

        SD_TIMEOUT(start_time, 100 * SPI_SD_TIMEOUT_NUM);
    }while(0xFE != read[0]);

    /*Step3 : SPI read single block size data*/
    read_param.buffer = (void *)read_buffer;
    read_param.size = spi_sd_dev->sd_param.block_param.block_size;
    BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

    /*Step4 : SPI read 2 bytes CRC*/
    read_param.buffer = (void *)read;
    read_param.size = 2;
    BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    return EOK;
}

static uint32 SdReadMultiBlock(SpiSdDeviceType spi_sd_dev, uint32 id, const uint8 *read_buffer, uint32 block_num)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    /*Step1 : CMD18, return 0x00*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_18;
    if (SD_TYPE_SDHC == spi_sd_dev->sd_param.sd_type) {
        g_sd_cmd_param.sd_cmd_args = id;
    } else {
        g_sd_cmd_param.sd_cmd_args = id * spi_sd_dev->sd_param.block_param.block_size;
    }
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdReadMultiBlock send CMD18 error %d\n", ret);
        return ERROR;
    }

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdReadMultiBlock CMD18 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    /*Step2 : SPI write data 0xFF until read 0xFE*/
    uint32 i = 0;
    uint8 data = 0xFF;
    uint8 read[2];
    uint32 start_time;

    for (i = 0 ; i < block_num ; i ++) {
        write_param.buffer = (void *)&data;
        write_param.size = 1;
        read_param.buffer = (void *)read;
        read_param.size = 1;

        start_time = 0;

        do
        {
            BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);
            BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

            SD_TIMEOUT(start_time, 10 * SPI_SD_TIMEOUT_NUM);
        }while(0xFE != read[0]);

        /*Step3 : SPI read multi block size data*/
        read_param.buffer = (void *)((uint8 *)read_buffer + i * spi_sd_dev->sd_param.block_param.block_size);
        read_param.size = spi_sd_dev->sd_param.block_param.block_size;
        BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);
    }

    /*Step4 : SPI read 2 bytes CRC*/
    read_param.buffer = (void *)read;
    read_param.size = 2;
    BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

    /*Step5 : CMD12 stop read*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_12;
    g_sd_cmd_param.sd_cmd_args = 0x00;
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1B;

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdReadMultiBlock send CMD12 error %d\n", ret);
        return ERROR;
    }

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    return EOK;
}

static uint32 SdWriteSingleBlock(SpiSdDeviceType spi_sd_dev, uint32 id, const uint8 *write_buffer)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint32 start_time;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    /*Step1 : CMD24, return 0x00*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_24;
    if (SD_TYPE_SDHC == spi_sd_dev->sd_param.sd_type) {
        g_sd_cmd_param.sd_cmd_args = id;
    } else {
        g_sd_cmd_param.sd_cmd_args = id * spi_sd_dev->sd_param.block_param.block_size;
    }
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    start_time = 0;
    do
    {
        ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
        if (EOK != ret) {
            KPrintf("SdWriteSingleBlock send CMD24 error %d\n", ret);
            return ERROR;
        }

        SD_TIMEOUT(start_time, SPI_SD_TIMEOUT_NUM);
    }while(0x00 != g_sd_cmd_param.sd_respone_data[0]);

    /*Step2 : SPI write data 0xFE*/
    uint8 data = 0xFE;
    uint8 read;
    uint8 write[2] = {0xFF, 0xFF};

    write_param.buffer = (void *)&data;
    write_param.size = 1;
    BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

    /*Step3 : SPI write single block size data*/
    write_param.buffer = (void *)write_buffer;
    write_param.size = spi_sd_dev->sd_param.block_param.block_size;
    BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

    /*Step4 : SPI write 2 bytes CRC*/
    write_param.buffer = (void *)write;
    write_param.size = 2;
    BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

    /*Step5 : SPI read respone xxx0 0101, write data done*/
    read_param.buffer = (void *)&read;
    read_param.size = 1;

    start_time = 0;
    do
    {
        BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

        SD_TIMEOUT(start_time, 10 * SPI_SD_TIMEOUT_NUM);
    }while(0x05 != (read & 0x1F));
    
    /*Step6 : SPI read respone 0xFF, write done*/
    ret = SdReady(spi_sd_dev);
    if (EOK != ret) {
        KPrintf("SdWriteSingleBlock SdReady error %d\n", ret);
        return ERROR;
    }

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    return EOK;
}

static uint32 SdWriteMultiBlock(SpiSdDeviceType spi_sd_dev, uint32 id, const void *write_buffer, uint32 block_num)
{
    NULL_PARAM_CHECK(spi_sd_dev);

    x_err_t ret = EOK;
    uint32 start_time;

    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    /*pull down the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 1, 0);

    /*Step1 : CMD55 + ACMD23 erase block*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_55;
    g_sd_cmd_param.sd_cmd_args = 0x00;
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdWriteMultiBlock send CMD55 error %d\n", ret);
        return ERROR;
    }

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdWriteMultiBlock CMD55 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    g_sd_cmd_param.sd_cmd_type = SD_ACMD_23;
    g_sd_cmd_param.sd_cmd_args = block_num;
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
    if (EOK != ret) {
        KPrintf("SdWriteMultiBlock send CMD55 error %d\n", ret);
        return ERROR;
    }

    if (0x00 != g_sd_cmd_param.sd_respone_data[0]) {
        KPrintf("SdWriteMultiBlock ACMD23 respone error %d\n", g_sd_cmd_param.sd_respone_data[0]);
        return ERROR;
    }

    /*Step2 : CMD25, return 0x00*/
    g_sd_cmd_param.sd_cmd_type = SD_CMD_25;
    if (SD_TYPE_SDHC == spi_sd_dev->sd_param.sd_type) {
        g_sd_cmd_param.sd_cmd_args = id;
    } else {
        g_sd_cmd_param.sd_cmd_args = id * spi_sd_dev->sd_param.block_param.block_size;
    }
    g_sd_cmd_param.sd_cmd_crc = 0x00;
    g_sd_cmd_param.sd_respone_type = SD_RESPONE_1;

    start_time = 0;
    do
    {
        ret = SdSendCmdByte(spi_sd_dev, &g_sd_cmd_param);
        if (EOK != ret) {
            KPrintf("SdWriteMultiBlock send CMD25 error %d\n", ret);
            return ERROR;
        }

        SD_TIMEOUT(start_time, SPI_SD_TIMEOUT_NUM);
    }while(0x00 != g_sd_cmd_param.sd_respone_data[0]);

    /*Step3 : SPI write data 0xFC*/
    uint32 i;
    uint8 data = 0xFC;
    uint8 read;
    uint8 write[2] = {0xFF, 0xFF};

    for (i = 0 ; i < block_num; i ++) {
        write_param.buffer = (void *)&data;
        write_param.size = 1;
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

        /*Step4 : SPI write multi block size data*/
        write_param.buffer = (void *)((uint8 *)write_buffer + i * spi_sd_dev->sd_param.block_param.block_size);
        write_param.size = spi_sd_dev->sd_param.block_param.block_size;
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

        /*Step5 : SPI write 2 bytes CRC*/
        write_param.buffer = (void *)write;
        write_param.size = 2;
        BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

        /*Step6 : SPI read respone xxx0 0101, single block write data done*/
        read_param.buffer = (void *)&read;
        read_param.size = 1;

        start_time = 0;
        do
        {
            BusDevReadData(&spi_sd_dev->spi_dev->haldev, &read_param);

            SD_TIMEOUT(start_time, 10 * SPI_SD_TIMEOUT_NUM);
        }while(0x05 != (read & 0x1F));
    
        /*Step7 : SPI read respone 0xFF, single block write done*/
        ret = SdReady(spi_sd_dev);
        if (EOK != ret) {
            KPrintf("SdWriteMultiBlock i %d SdReady error %d\n", i, ret);
            return ERROR;
        }
    }

    /*Step8 : SPI write 0xFD, multi block write data done*/
    write_param.buffer = (void *)&data;
    write_param.size = 1;
    BusDevWriteData(&spi_sd_dev->spi_dev->haldev, &write_param);

    /*Step9 : SPI read respone 0xFF, multi block write done*/
    ret = SdReady(spi_sd_dev);
    if (EOK != ret) {
        KPrintf("SdWriteMultiBlock final SdReady error %d\n", ret);
        return ERROR;
    }

    /*pull up the cs pin*/
    SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);

    return EOK;
}

uint32 SdOpen(void *dev)
{
    NULL_PARAM_CHECK(dev);

    SpiSdDeviceType spi_sd_dev;
    HardwareDevType haldev = (struct HardwareDev *)dev;
    struct SpiHardwareDevice *sd_dev;

    sd_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    spi_sd_dev = CONTAINER_OF(sd_dev, struct SpiSdDevice, sd_dev);

    x_err_t ret = EOK;
    struct Driver *spi_drv = spi_sd_dev->spi_dev->haldev.owner_bus->owner_driver;

    /*Step1 : Configure SPI Low speed*/
    struct BusConfigureInfo configure_info;
    struct SpiMasterParam spi_master_param;
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = SPI_SD_FREQUENCY;//ensure the initialization successfully
    spi_master_param.spi_data_endian = 0;

    ret = spi_sd_dev->spi_dev->haldev.owner_bus->match(spi_drv, &spi_sd_dev->spi_dev->haldev);
    if (EOK != ret) {
        KPrintf("SD SPI dev match spi drv error!\n");
        return ret;
    }

    configure_info.private_data = (void *)&spi_master_param;
    ret = DeviceObtainBus(spi_sd_dev->spi_dev->haldev.owner_bus, &spi_sd_dev->spi_dev->haldev, spi_drv->drv_name, &configure_info);
    if (EOK != ret) {
        KPrintf("SD SPI obtain spi bus error!\n");
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    ret = SdSpiConfigure(spi_drv, &spi_master_param);
    if (EOK != ret) {
        KPrintf("SD SPI configure error!\n");
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    /*Step2 : Init the SD Card, pull up the cs pin for 74 clock*/
    SdHwInitCs(spi_sd_dev);

    /*Step3 : Init the SD Card using CMD0*/
    ret = SdHwInit(spi_sd_dev);
    if (EOK != ret) {
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    /*Step4 : Confirm the SD Card type using CMD8*/
    ret = SdConfirmType(spi_sd_dev);
    if (EOK != ret) {
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    /*Step5 : Reset the SD Card based on the card type*/
    ret = SdHwReset(spi_sd_dev);
    if (EOK != ret) {
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    /*Step6 : Set the block length*/
    ret = SdHwSetBlockLength(spi_sd_dev);
    if (EOK != ret) {
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ret;
    }

    /*Step7 : Read the CSD register data*/
    ret = SdHwReadCSD(spi_sd_dev);
    if (EOK != ret) {
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        /*pull up the cs pin*/
        SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);
        return ret;
    }

    /*Step8 : Configure SPI High speed*/
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = spi_sd_dev->sd_param.sd_spi_maxfreq;
    spi_master_param.spi_data_endian = 0;
    if (SdSpiConfigure(spi_drv, &spi_master_param)) {
        KPrintf("SD SPI configure error!\n");
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ERROR;
    }

    KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
    return EOK;
}

uint32 SdClose(void *dev)
{
    return EOK;
}

uint32 SdRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);

    SpiSdDeviceType spi_sd_dev;
    HardwareDevType haldev = (struct HardwareDev *)dev;
    struct SpiHardwareDevice *sd_dev;

    sd_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    spi_sd_dev = CONTAINER_OF(sd_dev, struct SpiSdDevice, sd_dev);

    uint32 id = read_param->pos;
    uint32 block_num = read_param->size;
    uint8 *read_buffer = (uint8 *)read_param->buffer;

    if (0 == block_num) {
        KPrintf("SdRead block_num is 0 just return\n");
        return block_num;
    }

    x_err_t ret = EOK;
    struct Driver *spi_drv = spi_sd_dev->spi_dev->haldev.owner_bus->owner_driver;

    /*Step1 : SD Card Obtain SPI  BUS*/
    struct BusConfigureInfo configure_info;
    struct SpiMasterParam spi_master_param;
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = spi_sd_dev->sd_param.sd_spi_maxfreq;
    spi_master_param.spi_data_endian = 0;

    configure_info.private_data = (void *)&spi_master_param;
    ret = DeviceObtainBus(spi_sd_dev->spi_dev->haldev.owner_bus, &spi_sd_dev->spi_dev->haldev, spi_drv->drv_name, &configure_info);
    if (EOK != ret) {
        KPrintf("SdRead SD SPI obtain spi bus error!\n");
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ERROR;
    }

    /*Step2 : read SD block*/
    if (1 == block_num) {
        ret = SdReadSingleBlock(spi_sd_dev, id, read_buffer);
        if (EOK != ret) {
            KPrintf("SdRead SingleBlock error %d\n", ret);
            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);
            KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
            return 0;
        }
    } else {
        ret = SdReadMultiBlock(spi_sd_dev, id, read_buffer, block_num);
        if (EOK != ret) {
            KPrintf("SdRead MultiBlock error %d\n", ret);
            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);
            KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
            return 0;
        }
    }

    KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);

    return block_num;
}

uint32 SdWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);

    SpiSdDeviceType spi_sd_dev;
    HardwareDevType haldev = (struct HardwareDev *)dev;
    struct SpiHardwareDevice *sd_dev;

    sd_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    spi_sd_dev = CONTAINER_OF(sd_dev, struct SpiSdDevice, sd_dev);

    uint32 id = write_param->pos;
    uint32 block_num = write_param->size;
    const uint8 *write_buffer = (uint8 *)write_param->buffer;

    if (0 == block_num) {
        KPrintf("SdWrite block_num is 0 just return\n");
        return block_num;
    }

    x_err_t ret = EOK;
    struct Driver *spi_drv = spi_sd_dev->spi_dev->haldev.owner_bus->owner_driver;

    /*Step1 : SD Card Obtain SPI  BUS*/
    struct BusConfigureInfo configure_info;
    struct SpiMasterParam spi_master_param;
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = spi_sd_dev->sd_param.sd_spi_maxfreq;
    spi_master_param.spi_data_endian = 0;

    configure_info.private_data = (void *)&spi_master_param;
    ret = DeviceObtainBus(spi_sd_dev->spi_dev->haldev.owner_bus, &spi_sd_dev->spi_dev->haldev, spi_drv->drv_name, &configure_info);
    if (EOK != ret) {
        KPrintf("SdWrite SD SPI obtain spi bus error!\n");
        KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ERROR;
    }

    /*Step2 : write SD block*/
    if (1 == block_num) {
        ret = SdWriteSingleBlock(spi_sd_dev, id, write_buffer);
        if (EOK != ret) {
            KPrintf("SdWrite SingleBlock error %d\n", ret);
            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);
            KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
            return 0;
        }
    } else {
        ret = SdWriteMultiBlock(spi_sd_dev, id, write_buffer, block_num);
        if (EOK != ret) {
            KPrintf("SdWrite MultiBlock error %d\n", ret);
            /*pull up the cs pin*/
            SpiDevConfigureCs(&spi_sd_dev->spi_dev->haldev, 0, 1);
            KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
            return 0;
        }
    }

    KMutexAbandon(spi_sd_dev->spi_dev->haldev.owner_bus->bus_lock);
    return block_num;
}

static int SdControl(struct HardwareDev *dev, struct HalDevBlockParam *block_param)
{
    NULL_PARAM_CHECK(dev);

    if (OPER_BLK_GETGEOME == block_param->cmd) {
        block_param->dev_block.size_perbank = g_spi_sd_dev.sd_param.block_param.sector_bytes;
        block_param->dev_block.block_size = g_spi_sd_dev.sd_param.block_param.block_size;
        block_param->dev_block.bank_num = g_spi_sd_dev.sd_param.block_param.sector_num;
    }

    return EOK;
}

static const struct HalDevDone sd_done =
{
    .open = SdOpen,
    .close = SdClose,
    .write = SdWrite,
    .read = SdRead,
};

SpiSdDeviceType SpiSdInit(struct Bus *bus, const char *dev_name, const char *drv_name, const char *sd_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(sd_name);
    NULL_PARAM_CHECK(bus);

    x_err_t ret = EOK;
    SpiSdDeviceType spi_sd_dev = &g_spi_sd_dev;
    HardwareDevType haldev;

    memset(spi_sd_dev, 0, sizeof(struct SpiSdDevice));

    haldev = SpiDeviceFind(dev_name, TYPE_SPI_DEV);
    if (NONE == haldev) {
        KPrintf("SpiSdInit find spi haldev %s error! \n", dev_name);
        return NONE;
    }

    spi_sd_dev->spi_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    if (NONE == spi_sd_dev->spi_dev) {
        KPrintf("SpiSdInit find spi sd device %s error! \n", dev_name);
        return NONE;
    }

    spi_sd_dev->sd_dev.spi_dev_flag = RET_TRUE;
    spi_sd_dev->sd_dev.haldev.dev_done = &sd_done;
    spi_sd_dev->sd_dev.haldev.dev_block_control = SdControl;

    spi_sd_dev->spi_dev->haldev.owner_bus->owner_driver = SpiDriverFind(drv_name, TYPE_SPI_DRV);
    if (NONE == spi_sd_dev->spi_dev->haldev.owner_bus->owner_driver) {
        KPrintf("SpiSdInit find spi driver %s error! \n", drv_name);
        return NONE;
    }

    ret = SpiDeviceRegister(&spi_sd_dev->sd_dev, NONE, sd_name);
    if (EOK != ret) {
        KPrintf("SpiSdInit SpiDeviceRegister device %s error %d\n", sd_name, ret);
        return NONE;
    }

    ret = SpiDeviceAttachToBus(sd_name, bus->bus_name);
    if (EOK != ret) {
        KPrintf("SpiSdInit SpiDeviceAttachToBus device %s error %d\n", sd_name, ret);
        return NONE;
    }

    return spi_sd_dev;
}
