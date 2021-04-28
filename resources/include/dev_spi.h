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
* @file dev_spi.h
* @brief define spi dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_SPI_H
#define DEV_SPI_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_MAX_CLOCK            40000000
#define spi_device_max_num  4

#define SPI_LINE_CPHA     (1<<0)                           
#define SPI_LINE_CPOL     (1<<1)                          

#define SPI_LSB       (0<<2)                             
#define SPI_MSB      (1<<2)                             

#define SPI_MASTER           (0<<3)                            
#define DEV_SPI_SLAVE    (1<<3)      

#define SPI_MODE_0             (0 | 0)                        
#define SPI_MODE_1             (0 | SPI_LINE_CPHA)              
#define SPI_MODE_2             (SPI_LINE_CPOL | 0)            
#define SPI_MODE_3             (SPI_LINE_CPOL | SPI_LINE_CPHA)    
#define SPI_MODE_MASK    (SPI_LINE_CPHA | SPI_LINE_CPOL | SPI_MSB)

#define SPI_CS_HIGH  (1<<4)                            
#define SPI_NO_CS      (1<<5)                           
#define SPI_3WIRE        (1<<6)                             
#define SPI_READY        (1<<7)   

struct SpiDataStandard
{
    uint8 spi_chip_select;
    uint8 spi_cs_release;

    const uint8 *tx_buff;
    uint32 tx_len;

    const uint8 *rx_buff;
    uint32 rx_len;

    uint32 length;
    struct SpiDataStandard *next;
};

struct SpiMasterParam
{
    uint8 spi_work_mode;//CPOL CPHA
    uint8 spi_frame_format;//frame format
    uint8 spi_data_bit_width;//bit width
    uint8 spi_data_endian;//little endian：0，big endian：1
    uint32 spi_maxfrequency;//work frequency
};

struct SpiDmaParam 
{
    uint8 spi_master_id;
    uint8 spi_dmac_txchannel;
    uint8 spi_dmac_rxchannel;
};

struct SpiSlaveParam
{
    uint8 spi_slave_id;
    uint8 spi_cs_gpio_pin;
    uint8 spi_cs_select_id;
};

typedef struct
{
    struct SpiDmaParam *spi_dma_param;

    struct SpiSlaveParam *spi_slave_param;

    struct SpiMasterParam *spi_master_param;

}SpiDeviceParam;

struct SpiHardwareDevice;

struct SpiDevDone
{
    uint32 (*open) (struct SpiHardwareDevice *dev);
    uint32 (*close) (struct SpiHardwareDevice *dev);
    uint32 (*write) (struct SpiHardwareDevice *dev, struct SpiDataStandard *msg);
    uint32 (*read) (struct SpiHardwareDevice *dev, struct SpiDataStandard *msg);
};

struct SpiHardwareDevice
{
    struct HardwareDev haldev;
    SpiDeviceParam spi_param;

    x_bool spi_dev_flag;

    const struct SpiDevDone *spi_dev_done;
    
    void *private_data;
};

/*Register the spi device*/
int SpiDeviceRegister(struct SpiHardwareDevice *spi_device, void *spi_param, const char *device_name);

/*Register the spi device to the spi bus*/
int SpiDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register spi device*/
HardwareDevType SpiDeviceFind(const char *dev_name, enum DevType dev_type);

/*Configure the cs pin of spi dev*/
int SpiDevConfigureCs(struct HardwareDev *dev, uint8 spi_chip_select, uint8 spi_cs_release);

#ifdef __cplusplus
}
#endif

#endif
