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
* @file flash_spi.c
* @brief support spi-flash dev function using spi bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_spi.h>
#include <dev_spi.h>
#include <flash_spi.h>

/**
 * This function supports to write data to the flash.
 *
 * @param dev flash dev descriptor
 * @param write_param flash dev write datacfg param
 */
static uint32  SpiFlashWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);

    x_err_t ret = EOK;

    x_OffPos pos;
    x_size_t size;
    const uint8 *write_buffer;

    sfud_flash *sfud_flash_dev;

    SpiFlashDeviceType spi_flash_dev;
    HardwareDevType haldev = (struct HardwareDev *)dev;
    struct SpiHardwareDevice *flash_dev;

    struct BusBlockWriteParam *flash_write_param = (struct BusBlockWriteParam *)write_param->buffer;

    flash_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    spi_flash_dev = CONTAINER_OF(flash_dev, struct SpiFlashDevice, flash_dev);
    sfud_flash_dev = (sfud_flash *)spi_flash_dev->flash_param.flash_private_data;

    pos = flash_write_param->pos;// * spi_flash_dev->flash_param.flash_block_param.sector_bytes;
    size = flash_write_param->size;// * spi_flash_dev->flash_param.flash_block_param.sector_bytes;
    write_buffer = (uint8 *)flash_write_param->buffer;

    KPrintf("flash write pos %u sector_bytes %u size %u\n", 
        flash_write_param->pos, 
        spi_flash_dev->flash_param.flash_block_param.sector_bytes,
        flash_write_param->size);

    ret = sfud_erase_write(sfud_flash_dev, pos, size, write_buffer);
    if (SFUD_SUCCESS != ret) {
        KPrintf("SpiFlashWrite error %d pos %d size %d buffer %p\n", ret, pos, size, write_buffer);
        return ERROR;
    }

    haldev->owner_bus->owner_haldev = haldev;

    return ret;
}

/**
 * This function supports to read data from the flash.
 *
 * @param dev flash dev descriptor
 * @param read_param flash dev read datacfg param
 */
static uint32  SpiFlashRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);
    
    x_err_t ret = EOK;
    x_OffPos pos;
    x_size_t size;
    uint8 *read_buffer;

    sfud_flash *sfud_flash_dev;

    SpiFlashDeviceType spi_flash_dev;
    HardwareDevType haldev = (struct HardwareDev *)dev;
    struct SpiHardwareDevice *flash_dev;

    struct BusBlockReadParam *flash_read_param = (struct BusBlockReadParam *)read_param->buffer;

    flash_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);
    spi_flash_dev = CONTAINER_OF(flash_dev, struct SpiFlashDevice, flash_dev);
    sfud_flash_dev = (sfud_flash *)spi_flash_dev->flash_param.flash_private_data;

    pos = flash_read_param->pos;// * spi_flash_dev->flash_param.flash_block_param.sector_bytes;
    size = flash_read_param->size;// * spi_flash_dev->flash_param.flash_block_param.sector_bytes;
    read_buffer = (uint8 *)flash_read_param->buffer;

    ret = sfud_read(sfud_flash_dev, pos, size, read_buffer);
    if (SFUD_SUCCESS != ret) {
        KPrintf("SpiFlashRead error %d pos %d size %d buffer %p\n", ret, pos, size, read_buffer);
        return ERROR;
    }

    flash_read_param->read_length = flash_read_param->size;
    read_param->read_length = flash_read_param->size;

    haldev->owner_bus->owner_haldev = haldev;

    KPrintf("SpiFlashRead read buffer done\n");

    return ret;
}

/**
 * This function supports to get block data from the flash dev.
 *
 * @param dev flash dev descriptor
 * @param block_param flash dev read datacfg param
 */
static int  SpiFlashControl(struct HardwareDev *dev, struct HalDevBlockParam *block_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(block_param);
    
    x_err_t ret = EOK;
    x_size_t size;
    uint32_t addr;

    sfud_flash *sfud_flash_dev;

    SpiFlashDeviceType spi_flash_dev;
    struct SpiHardwareDevice *flash_dev;

    flash_dev = CONTAINER_OF(dev, struct SpiHardwareDevice, haldev);
    spi_flash_dev = CONTAINER_OF(flash_dev, struct SpiFlashDevice, flash_dev);
    sfud_flash_dev = (sfud_flash *)spi_flash_dev->flash_param.flash_private_data;

    if (OPER_BLK_GETGEOME == block_param->cmd) {
        block_param->dev_block.size_perbank = spi_flash_dev->flash_param.flash_block_param.sector_bytes;
        block_param->dev_block.block_size = spi_flash_dev->flash_param.flash_block_param.block_size;
        block_param->dev_block.bank_num = spi_flash_dev->flash_param.flash_block_param.sector_num;
    } else if (OPER_BLK_ERASE == block_param->cmd) {
        NULL_PARAM_CHECK(block_param->dev_addr);
        if (block_param->dev_addr->start > block_param->dev_addr->end) {
            KPrintf("SpiFlashControl erase start %u > end %u addr\n", block_param->dev_addr->start, block_param->dev_addr->end);
            return ERROR;
        } else if (block_param->dev_addr->start == block_param->dev_addr->end) {
            block_param->dev_addr->end++;
        }

        addr = block_param->dev_addr->start * spi_flash_dev->flash_param.flash_block_param.sector_bytes;
        size = (block_param->dev_addr->end - block_param->dev_addr->start)* spi_flash_dev->flash_param.flash_block_param.sector_bytes;

        ret = sfud_erase(sfud_flash_dev, addr, size);
        if (SFUD_SUCCESS != ret) {
            KPrintf("SpiFlashControl erase error %d addr %p size %d\n", ret, addr, size);
            return ERROR;
        }
    } else {
        KPrintf("SpiFlashControl do not suppot the cmd 0x%x\n", block_param->cmd);
    }

    return ret;
}

static uint32 FlashSpiConfigure(SpiFlashDeviceType spi_flash_dev)
{
    NULL_PARAM_CHECK(spi_flash_dev);

    x_err_t ret = EOK;

    struct Driver *spi_drv = spi_flash_dev->spi_dev->haldev.owner_bus->owner_driver;

    struct BusConfigureInfo configure_info;
    struct SpiMasterParam spi_master_param;
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = SPI_FLASH_FREQUENCY;
    spi_master_param.spi_data_endian = 0;

    ret = spi_flash_dev->spi_dev->haldev.owner_bus->match(spi_drv, &spi_flash_dev->spi_dev->haldev);
    if (EOK != ret) {
        KPrintf("FLASH SPI dev match spi drv error!\n");
        return ret;
    }

    configure_info.configure_cmd = OPE_CFG;
    configure_info.private_data = (void *)&spi_master_param;
    ret = BusDrvConfigure(spi_drv, &configure_info);
    if (ret) {
        KPrintf("spi drv OPE_CFG error drv %8p cfg %8p\n", spi_drv, &spi_master_param);
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

/**
 * This function supports to find sfud_flash descriptor by flash_name
 *
 * @param flash_name flash dev name
 */
sfud_flash_t SpiFlashFind(char *flash_name)
{
    NULL_PARAM_CHECK(flash_name);

    HardwareDevType haldev = SpiDeviceFind(flash_name, TYPE_SPI_DEV);
    if (NONE == haldev) {
        KPrintf("SpiFlashFind %s error! \n", flash_name);
        return NONE;
    }

    struct SpiHardwareDevice *flash_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);

    SpiFlashDeviceType spi_flash_dev = CONTAINER_OF(flash_dev, struct SpiFlashDevice, flash_dev);

    sfud_flash_t sfud_flash_dev = (sfud_flash_t)spi_flash_dev->flash_param.flash_private_data;

    return sfud_flash_dev;
}

static const struct FlashDevDone flash_done =
{
    .open = NONE,
    .close = NONE,
    .write = SpiFlashWrite,
    .read = SpiFlashRead,
};

/**
 * This function supports to init spi_flash_dev and sfud_flash descriptor
 *
 * @param bus_name spi bus name
 * @param dev_name spi dev name
 * @param drv_name spi drv name
 * @param flash_name flash dev name
 */
SpiFlashDeviceType SpiFlashInit(char *bus_name, char *dev_name, char *drv_name, char *flash_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(flash_name);
    NULL_PARAM_CHECK(bus_name);

    x_err_t ret;

    HardwareDevType haldev = SpiDeviceFind(dev_name, TYPE_SPI_DEV);
    if (NONE == haldev) {
        KPrintf("SpiFlashInit find spi haldev %s error! \n", dev_name);
        return NONE;
    }

    SpiFlashDeviceType spi_flash_dev = (SpiFlashDeviceType)malloc(sizeof(struct SpiFlashDevice));
    if (NONE == spi_flash_dev) {
        KPrintf("SpiFlashInit malloc spi_flash_dev failed\n");
        free(spi_flash_dev);
        return NONE;
    }

    sfud_flash *sfud_flash_dev = (sfud_flash_t)malloc(sizeof(sfud_flash));
    if (NONE == sfud_flash_dev) {
        KPrintf("SpiFlashInit malloc sfud_flash_dev failed\n");
        free(spi_flash_dev);
        free(sfud_flash_dev);
        return NONE;
    }

    memset(spi_flash_dev, 0, sizeof(struct SpiFlashDevice));
    memset(sfud_flash_dev, 0, sizeof(sfud_flash));

    spi_flash_dev->spi_dev = CONTAINER_OF(haldev, struct SpiHardwareDevice, haldev);

    spi_flash_dev->flash_dev.spi_dev_flag = RET_TRUE;
    spi_flash_dev->flash_param.flash_private_data = (void *)sfud_flash_dev;
    spi_flash_dev->flash_dev.haldev.dev_done = (struct HalDevDone *)&flash_done;
    spi_flash_dev->flash_dev.haldev.dev_block_control = SpiFlashControl;

    spi_flash_dev->spi_dev->haldev.owner_bus->owner_driver = SpiDriverFind(drv_name, TYPE_SPI_DRV);
    if (NONE == spi_flash_dev->spi_dev->haldev.owner_bus->owner_driver) {
        KPrintf("SpiFlashInit find spi driver %s error! \n", drv_name);
        free(spi_flash_dev);
        free(sfud_flash_dev);
        return NONE;
    }

    ret = FlashSpiConfigure(spi_flash_dev);
    if (EOK != ret) {
        KPrintf("SpiFlashInit configure failed\n");
        free(spi_flash_dev);
        free(sfud_flash_dev);
        return NONE;
    }

    sfud_flash_dev->name = flash_name;
    sfud_flash_dev->user_data = (void *)spi_flash_dev;

    sfud_flash_dev->spi.name = dev_name;
    sfud_flash_dev->spi.user_data = (void *)sfud_flash_dev;

    ret = sfud_device_init(sfud_flash_dev);
    if (SFUD_SUCCESS != ret) {
        KPrintf("sfud_device_init failed %d\n", ret);
        free(spi_flash_dev);
        free(sfud_flash_dev);
        return NONE;
    }

    spi_flash_dev->flash_param.flash_block_param.block_size = sfud_flash_dev->chip.erase_gran;
    spi_flash_dev->flash_param.flash_block_param.sector_bytes = sfud_flash_dev->chip.erase_gran;
    spi_flash_dev->flash_param.flash_block_param.sector_num = sfud_flash_dev->chip.capacity / sfud_flash_dev->chip.erase_gran;

    ret = SpiDeviceRegister(&spi_flash_dev->flash_dev, NONE, flash_name);
    if (EOK != ret) {
        KPrintf("SpiFlashInit SpiDeviceRegister device %s error %d\n", flash_name, ret);
        free(spi_flash_dev);
        free(sfud_flash_dev);        
        return NONE;
    }

    ret = SpiDeviceAttachToBus(flash_name, bus_name);
    if (EOK != ret) {
        KPrintf("SpiFlashInit SpiDeviceAttachToBus device %s error %d\n", flash_name, ret);
        free(spi_flash_dev);
        free(sfud_flash_dev);
        return NONE;
    }

    spi_flash_dev->flash_lock = KMutexCreate();

    return spi_flash_dev;
}

/**
 * This function supports to release spi_flash_dev and sfud_flash descriptor
 *
 * @param spi_flash_dev spi flash descriptor
 */
uint32 SpiFlashRelease(SpiFlashDeviceType spi_flash_dev)
{
    NULL_PARAM_CHECK(spi_flash_dev);

    x_err_t ret;
    sfud_flash *sfud_flash_dev = (sfud_flash_t)spi_flash_dev->flash_param.flash_private_data;

    DeviceDeleteFromBus(spi_flash_dev->flash_dev.haldev.owner_bus, &spi_flash_dev->flash_dev.haldev);

    KMutexDelete(spi_flash_dev->flash_lock);

    free(sfud_flash_dev);
    free(spi_flash_dev);

    return EOK;
}
