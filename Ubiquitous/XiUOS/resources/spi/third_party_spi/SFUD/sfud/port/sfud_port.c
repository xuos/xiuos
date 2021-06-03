/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 * 
 */

/**
* @file sfud_port.c
* @brief support sfud function register based on spi bus driver
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

/*************************************************
File name: sfud_port.c
Description: support spi bus driver function, then register to sfud lib
Others: add sfud_port.c to support SPI Flash function based on SFUD LIB
                https://github.com/armink/SFUD
History: 
1. Date: 2021-04-24
Author: AIIT XUOS Lab
Modification: 
1、implement sfud_port APIs with XiUOS driver framework APIs
2、add flash dev write、read and control functions
*************************************************/

#include <sfud_port.h>
#include <stdarg.h>

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

/**
 * This function supports to obtain the spi bus.
 *
 * @param spi_flash_dev flash dev descriptor
 */
static uint32 spi_flash_obtain_bus(SpiFlashDeviceType spi_flash_dev)
{
    NULL_PARAM_CHECK(spi_flash_dev);
    x_err_t ret;
    
    struct Driver *spi_drv = spi_flash_dev->spi_dev->haldev.owner_bus->owner_driver;

    struct BusConfigureInfo configure_info;
    struct SpiMasterParam spi_master_param;
    spi_master_param.spi_data_bit_width = 8;
    spi_master_param.spi_work_mode = SPI_MODE_0 | SPI_MSB;
    spi_master_param.spi_maxfrequency = SPI_FLASH_FREQUENCY;
    spi_master_param.spi_data_endian = 0;

    configure_info.private_data = (void *)&spi_master_param;
    ret = DeviceObtainBus(spi_flash_dev->spi_dev->haldev.owner_bus, &spi_flash_dev->spi_dev->haldev, spi_drv->drv_name, &configure_info);
    if (EOK != ret) {
        KPrintf("spi_flash_obtain_bus  error!\n");
        KMutexAbandon(spi_flash_dev->spi_dev->haldev.owner_bus->bus_lock);
        return ERROR;
    }

    return ret;
}

/**
 * This function supports to read and write data by the spi bus.
 *
 * @param spi_flash_dev flash dev descriptor
 * @param write_buf write buffer descriptor
 * @param write_size write data length
 * @param read_buf read buffer descriptor
 * @param read_size read data length
 */
static void spi_bus_read_write(SpiFlashDeviceType spi_flash_dev, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf, size_t read_size)
{
    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;

    BusDevOpen(&spi_flash_dev->spi_dev->haldev);

    write_param.buffer = (void *)write_buf;
    write_param.size = write_size;
    BusDevWriteData(&spi_flash_dev->spi_dev->haldev, &write_param);

    read_param.buffer = (void *)read_buf;
    read_param.size = read_size;
    BusDevReadData(&spi_flash_dev->spi_dev->haldev, &read_param);

    BusDevClose(&spi_flash_dev->spi_dev->haldev);
}

/**
 * This function supports to read data by the spi bus.
 *
 * @param spi_flash_dev flash dev descriptor
 * @param read_buf read buffer descriptor
 * @param read_size read data length
 */
static void spi_bus_read(SpiFlashDeviceType spi_flash_dev, uint8_t *read_buf, size_t read_size)
{
    struct BusBlockReadParam read_param;

    BusDevOpen(&spi_flash_dev->spi_dev->haldev);

    read_param.buffer = (void *)read_buf;
    read_param.size = read_size;
    BusDevReadData(&spi_flash_dev->spi_dev->haldev, &read_param);

    BusDevClose(&spi_flash_dev->spi_dev->haldev);
}

/**
 * This function supports to write data by the spi bus.
 *
 * @param spi_flash_dev flash dev descriptor
 * @param write_buf write buffer descriptor
 * @param write_size write data length
 */
static void spi_bus_write(SpiFlashDeviceType spi_flash_dev, const uint8_t *write_buf, size_t write_size)
{
    struct BusBlockWriteParam write_param;

    BusDevOpen(&spi_flash_dev->spi_dev->haldev);

    write_param.buffer = (void *)write_buf;
    write_param.size = write_size;
    BusDevWriteData(&spi_flash_dev->spi_dev->haldev, &write_param);

    BusDevClose(&spi_flash_dev->spi_dev->haldev);
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your spi write and read code
     */
    x_err_t ret;
    SpiFlashDeviceType spi_flash_dev;
    sfud_flash *sfud_flash_dev = (sfud_flash *) (spi->user_data);
    spi_flash_dev = (SpiFlashDeviceType)sfud_flash_dev->user_data;

    NULL_PARAM_CHECK(sfud_flash_dev);
    NULL_PARAM_CHECK(spi_flash_dev);

    if (write_size && read_size) {
        NULL_PARAM_CHECK(write_buf);
        NULL_PARAM_CHECK(read_buf);

        ret = spi_flash_obtain_bus(spi_flash_dev);
        if (EOK != ret) {
            return SFUD_ERR_TIMEOUT;
        }

        spi_bus_read_write(spi_flash_dev, write_buf, write_size, read_buf, read_size);
    } else if (read_size) {
        NULL_PARAM_CHECK(read_buf);

        ret = spi_flash_obtain_bus(spi_flash_dev);
        if (EOK != ret) {
            return SFUD_ERR_TIMEOUT;
        }

        spi_bus_read(spi_flash_dev, read_buf, read_size);
    } else if (write_size) {
        NULL_PARAM_CHECK(write_buf);

        ret = spi_flash_obtain_bus(spi_flash_dev);
        if (EOK != ret) {
            return SFUD_ERR_TIMEOUT;
        }

        spi_bus_write(spi_flash_dev, write_buf, write_size);
    } else {
        KPrintf("spi_write_read write or read size error\n");
        return SFUD_ERR_NOT_FOUND;
    }

    KMutexAbandon(spi_flash_dev->spi_dev->haldev.owner_bus->bus_lock);
    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

static void spi_lock(const sfud_spi *spi) 
{
    SpiFlashDeviceType spi_flash_dev;
    sfud_flash *sfud_flash_dev = (sfud_flash *) (spi->user_data);
    spi_flash_dev = (SpiFlashDeviceType)sfud_flash_dev->user_data;

    CHECK(sfud_flash_dev);
    CHECK(spi_flash_dev);

    KMutexObtain(spi_flash_dev->flash_lock, WAITING_FOREVER);
}

static void spi_unlock(const sfud_spi *spi)
{
    SpiFlashDeviceType spi_flash_dev;
    sfud_flash *sfud_flash_dev = (sfud_flash *) (spi->user_data);
    spi_flash_dev = (SpiFlashDeviceType)sfud_flash_dev->user_data;

    CHECK(sfud_flash_dev);
    CHECK(spi_flash_dev);

    KMutexAbandon(spi_flash_dev->flash_lock);
}

sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
    CHECK(flash);

    flash->spi.wr = spi_write_read;
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    flash->spi.user_data = flash;
    flash->retry.delay = NONE;
    flash->retry.times = 10000 * 60;

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    KPrintf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    KPrintf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    KPrintf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    KPrintf("%s\n", log_buf);
    va_end(args);
}
