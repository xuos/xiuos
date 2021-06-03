/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-18     ZYH          first version
 */

/**
* @file connect_spi.c
* @brief support aiit-riscv64-board spi function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_spi.c
Description: support aiit-riscv64-board spi configure and spi bus register function
Others: take RT-Thread v4.0.2/bsp/k210/driver/drv_spi.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-riscv64-board spi configure, write and read
2. support aiit-riscv64-board spi bus device and driver register
*************************************************/

#include <connect_spi.h>
#include <dmac.h>
#include <drv_io_config.h>
#include <gpiohs.h>
#include <hardware_spi.h>
#include <string.h>
#include <sysctl.h>
#include <utils.h>

#ifdef  BSP_SPI1_USING_SS0
#define SPI_DEVICE_SLAVE_ID_0 0
#endif

#ifdef BSP_SPI1_USING_SS1
#define SPI_DEVICE_SLAVE_ID_1 1
#endif

#ifdef BSP_SPI1_USING_SS2
#define SPI_DEVICE_SLAVE_ID_2 2
#endif

#ifdef BSP_SPI1_USING_SS3
#define SPI_DEVICE_SLAVE_ID_3 3
#endif

static volatile spi_t *const spi_instance[4] =
{
    (volatile spi_t *)SPI0_BASE_ADDR,
    (volatile spi_t *)SPI1_BASE_ADDR,
    (volatile spi_t *)SPI_SLAVE_BASE_ADDR,
    (volatile spi_t *)SPI3_BASE_ADDR
};

void __spi_set_tmod(uint8_t spi_num, uint32_t tmod)
{
    CHECK(spi_num < SPI_DEVICE_MAX);
    volatile spi_t *spi_handle = spi[spi_num];
    uint8_t tmod_offset = 0;
    switch(spi_num)
    {
        case 0:
        case 1:
        case 2:
            tmod_offset = 8;
            break;
        case 3:
        default:
            tmod_offset = 10;
            break;
    }
    set_bit(&spi_handle->ctrlr0, 3 << tmod_offset, tmod << tmod_offset);
}

/*Init the spi sdk intetface */
static uint32 SpiSdkInit(struct SpiDriver *spi_drv)
{
    NULL_PARAM_CHECK(spi_drv);    
    uint8 cs_gpio_pin, cs_select_id;
    uint32 max_frequency;

    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_drv->driver.private_data);

    cs_gpio_pin = dev_param->spi_slave_param->spi_cs_gpio_pin;
    cs_select_id = dev_param->spi_slave_param->spi_cs_select_id;

    gpiohs_set_drive_mode(cs_select_id, GPIO_DM_OUTPUT);//Set the cs pin as output
    gpiohs_set_pin(cs_gpio_pin, GPIO_PV_HIGH);//set the cs gpio high

    spi_init(dev_param->spi_dma_param->spi_master_id, 
                        dev_param->spi_master_param->spi_work_mode & SPI_MODE_3, 
                        dev_param->spi_master_param->spi_frame_format, 
                        dev_param->spi_master_param->spi_data_bit_width,
                        dev_param->spi_master_param->spi_data_endian);

    max_frequency = (dev_param->spi_master_param->spi_maxfrequency < SPI_MAX_CLOCK) ? dev_param->spi_master_param->spi_maxfrequency : SPI_MAX_CLOCK;

    uint32 real_freq = spi_set_clk_rate(dev_param->spi_dma_param->spi_master_id, max_frequency);

    return EOK;
}

static uint32 SpiSdkCfg(struct SpiDriver *spi_drv, struct SpiMasterParam *spi_param)
{
    NULL_PARAM_CHECK(spi_drv);
    NULL_PARAM_CHECK(spi_param);  

    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_drv->driver.private_data);

    dev_param->spi_master_param = spi_param;
    dev_param->spi_master_param->spi_work_mode = dev_param->spi_master_param->spi_work_mode  & SPI_MODE_MASK;
    dev_param->spi_master_param->spi_frame_format = SPI_FF_STANDARD;

    return EOK;
}

/*Configure the spi device param, make sure struct (configure_info->private_data) = (SpiMasterParam)*/
static uint32 SpiDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    struct SpiDriver *spi_drv = (struct SpiDriver *)drv;
    struct SpiMasterParam *spi_param;

    switch (configure_info->configure_cmd)
    {
        case OPE_INT:
            ret = SpiSdkInit(spi_drv);
            break;
        case OPE_CFG:
            spi_param = (struct SpiMasterParam *)configure_info->private_data;
            ret = SpiSdkCfg(spi_drv, spi_param);
            break;
        default:
            break;
    }

    return ret;
}

static uint32 SpiWriteData(struct SpiHardwareDevice *spi_dev, struct SpiDataStandard *spi_datacfg)
{
    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_dev->haldev.private_data);

    uint8 device_id = dev_param->spi_slave_param->spi_slave_id;
    uint8 device_master_id = dev_param->spi_dma_param->spi_master_id;
    uint8 cs_gpio_pin = dev_param->spi_slave_param->spi_cs_gpio_pin;

    while (NONE != spi_datacfg)
    {
        uint32_t * tx_buff = NONE;
        int i;
        x_ubase dummy = 0xFFFFFFFFU;

        __spi_set_tmod(device_master_id, SPI_TMOD_TRANS_RECV);

        if (spi_datacfg->spi_chip_select) {
            gpiohs_set_pin(cs_gpio_pin, GPIO_PV_LOW);
        }

        if (spi_datacfg->length) {
            spi_instance[device_master_id]->dmacr = 0x3;
            spi_instance[device_master_id]->ssienr = 0x01;

            sysctl_dma_select(dev_param->spi_dma_param->spi_dmac_txchannel, SYSCTL_DMA_SELECT_SSI0_TX_REQ + device_master_id * 2);
            sysctl_dma_select(dev_param->spi_dma_param->spi_dmac_rxchannel, SYSCTL_DMA_SELECT_SSI0_RX_REQ + device_master_id* 2);
            
            dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_rxchannel, (void *)(&spi_instance[device_master_id]->dr[0]), &dummy, DMAC_ADDR_NOCHANGE, DMAC_ADDR_NOCHANGE,
                DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, spi_datacfg->length);

            if (!spi_datacfg->tx_buff) {
                dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_txchannel, &dummy, (void *)(&spi_instance[device_master_id]->dr[0]), DMAC_ADDR_NOCHANGE, DMAC_ADDR_NOCHANGE,
                            DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, spi_datacfg->length);
            } else {
                tx_buff = x_malloc(spi_datacfg->length * 4);
                if (!tx_buff) {
                    goto transfer_done;
                }
                for (i = 0; i < spi_datacfg->length; i++) {
                    tx_buff[i] = ((uint8_t *)spi_datacfg->tx_buff)[i];
                }
                dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_txchannel, tx_buff, (void *)(&spi_instance[device_master_id]->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                            DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, spi_datacfg->length);
            }
            
            spi_instance[device_master_id]->ser = 1U << dev_param->spi_slave_param->spi_cs_select_id;
            dmac_wait_done(dev_param->spi_dma_param->spi_dmac_txchannel);
            dmac_wait_done(dev_param->spi_dma_param->spi_dmac_rxchannel);
            spi_instance[device_master_id]->ser = 0x00;
            spi_instance[device_master_id]->ssienr = 0x00;

    transfer_done:
            if (tx_buff) {
                x_free(tx_buff);
            }
        }

        if (spi_datacfg->spi_cs_release) {
            gpiohs_set_pin(cs_gpio_pin, GPIO_PV_HIGH);
        }

        spi_datacfg = spi_datacfg->next;
    }

    return spi_datacfg->length;    
}

static uint32 SpiReadData(struct SpiHardwareDevice *spi_dev, struct SpiDataStandard *spi_datacfg)
{
    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_dev->haldev.private_data);

    uint8 device_id = dev_param->spi_slave_param->spi_slave_id;
    uint8 device_master_id = dev_param->spi_dma_param->spi_master_id;
    uint8 cs_gpio_pin = dev_param->spi_slave_param->spi_cs_gpio_pin;

    while (NONE != spi_datacfg)
    {
        uint32_t * rx_buff = NONE;
        int i;
        x_ubase dummy = 0xFFFFFFFFU;

        __spi_set_tmod(device_master_id, SPI_TMOD_TRANS_RECV);

        if (spi_datacfg->spi_chip_select) {
            gpiohs_set_pin(cs_gpio_pin, GPIO_PV_LOW);
        }
        if (spi_datacfg->length) {
            spi_instance[device_master_id]->dmacr = 0x3;
            spi_instance[device_master_id]->ssienr = 0x01;

            sysctl_dma_select(dev_param->spi_dma_param->spi_dmac_txchannel, SYSCTL_DMA_SELECT_SSI0_TX_REQ + device_master_id * 2);
            sysctl_dma_select(dev_param->spi_dma_param->spi_dmac_rxchannel, SYSCTL_DMA_SELECT_SSI0_RX_REQ + device_master_id* 2);

            dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_txchannel, &dummy, (void *)(&spi_instance[device_master_id]->dr[0]), DMAC_ADDR_NOCHANGE, DMAC_ADDR_NOCHANGE,
                DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, spi_datacfg->length);

            if (!spi_datacfg->rx_buff) {
                dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_rxchannel, (void *)(&spi_instance[device_master_id]->dr[0]), &dummy, DMAC_ADDR_NOCHANGE, DMAC_ADDR_NOCHANGE,
                            DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, spi_datacfg->length);
            } else {
                rx_buff = x_calloc(spi_datacfg->length * 4, 1);
                if(!rx_buff)
                {
                    goto transfer_done;
                }
                
                dmac_set_single_mode(dev_param->spi_dma_param->spi_dmac_rxchannel, (void *)(&spi_instance[device_master_id]->dr[0]), rx_buff, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
                            DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, spi_datacfg->length);
            }
            
            spi_instance[device_master_id]->ser = 1U << dev_param->spi_slave_param->spi_cs_select_id;
            dmac_wait_done(dev_param->spi_dma_param->spi_dmac_txchannel);
            dmac_wait_done(dev_param->spi_dma_param->spi_dmac_rxchannel);
            spi_instance[device_master_id]->ser = 0x00;
            spi_instance[device_master_id]->ssienr = 0x00;

            if (spi_datacfg->rx_buff) {
                for (i = 0; i < spi_datacfg->length; i++) {           
                    ((uint8_t *)spi_datacfg->rx_buff)[i] = (uint8_t)rx_buff[i];
                }
            }

    transfer_done:
            if (rx_buff) {
                x_free(rx_buff);
            }
        }

        if (spi_datacfg->spi_cs_release) {
            gpiohs_set_pin(cs_gpio_pin, GPIO_PV_HIGH);
        }

        spi_datacfg = spi_datacfg->next;
    }

    return spi_datacfg->length;    
}

/*manage the spi device operations*/
static const struct SpiDevDone spi_dev_done =
{
    .dev_open = NONE,
    .dev_close = NONE,
    .dev_write = SpiWriteData,
    .dev_read = SpiReadData,
};

static int BoardSpiBusInit(struct SpiBus *spi_bus, struct SpiDriver *spi_driver)
{
    x_err_t ret = EOK;

    /*Init the spi bus */
    ret = SpiBusInit(spi_bus, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the spi driver*/
    ret = SpiDriverInit(spi_driver, SPI_1_DRV_NAME);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the spi driver to the spi bus*/
    ret = SpiDriverAttachToBus(SPI_1_DRV_NAME, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the spi device to the spi bus*/
static int BoardSpiDevBend(struct SpiDmaParam *spi_initparam)
{
    x_err_t ret = EOK;

#ifdef BSP_SPI1_USING_SS0
    static struct SpiHardwareDevice spi_device0;
    memset(&spi_device0, 0, sizeof(struct SpiHardwareDevice));

    static struct SpiSlaveParam spi_slaveparam0;
    memset(&spi_slaveparam0, 0, sizeof(struct SpiSlaveParam));

    spi_slaveparam0.spi_slave_id = SPI_DEVICE_SLAVE_ID_0;
    spi_slaveparam0.spi_cs_gpio_pin = SPI1_CS0_PIN;
    spi_slaveparam0.spi_cs_select_id = SPI_CHIP_SELECT_0;

    spi_device0.spi_param.spi_dma_param = spi_initparam;
    spi_device0.spi_param.spi_slave_param = &spi_slaveparam0;

    spi_device0.spi_dev_done = &(spi_dev_done);

    ret = SpiDeviceRegister(&spi_device0, (void *)(&spi_device0.spi_param), SPI_1_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceInit device %s error %d\n", SPI_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    ret = SpiDeviceAttachToBus(SPI_1_DEVICE_NAME_0, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceAttachToBus device %s error %d\n", SPI_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  
#endif

#ifdef BSP_SPI1_USING_SS1
    static struct SpiHardwareDevice spi_device1;
    memset(&spi_device1, 0, sizeof(struct SpiHardwareDevice));

    static struct SpiSlaveParam spi_slaveparam1;
    memset(&spi_slaveparam1, 0, sizeof(struct SpiSlaveParam));

    spi_slaveparam1.spi_slave_id = SPI_DEVICE_SLAVE_ID_1;
    spi_slaveparam1.spi_cs_gpio_pin = SPI1_CS1_PIN;
    spi_slaveparam1.spi_cs_select_id = SPI_CHIP_SELECT_1;

    spi_device1.spi_param.spi_dma_param = spi_initparam;
    spi_device1.spi_param.spi_slave_param = &spi_slaveparam1;    
    
    spi_device1.spi_dev_done = &(spi_dev_done);

    ret = SpiDeviceRegister(&spi_device1, (void *)(&spi_device1.spi_param), SPI_1_DEVICE_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceInit device %s error %d\n", SPI_1_DEVICE_NAME_1, ret);
        return ERROR;
    } 

    ret = SpiDeviceAttachToBus(SPI_1_DEVICE_NAME_1, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceAttachToBus device %s error %d\n", SPI_1_DEVICE_NAME_1, ret);
        return ERROR;
    }  
#endif

#ifdef BSP_SPI1_USING_SS2
    static struct SpiHardwareDevice spi_device2;
    memset(&spi_device2, 0, sizeof(struct SpiHardwareDevice));

    spi_initparam->spi_slave_id[SPI_DEVICE_SLAVE_ID_2] = SPI_DEVICE_SLAVE_ID_2;
    spi_initparam->spi_cs_gpio_pin[SPI_DEVICE_SLAVE_ID_2] = SPI1_CS2_PIN;
    spi_initparam->spi_cs_select_id[SPI_DEVICE_SLAVE_ID_2] = SPI_CHIP_SELECT_2;

    spi_device2.spi_dev_done = &(spi_dev_done);

    ret = SpiDeviceRegister(&spi_device2, (void *)(&spi_device2.spi_param), SPI_1_DEVICE_NAME_2);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceInit device %s error %d\n", SPI_1_DEVICE_NAME_2, ret);
        return ERROR;
    } 

    ret = SpiDeviceAttachToBus(SPI_1_DEVICE_NAME_2, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceAttachToBus device %s error %d\n", SPI_1_DEVICE_NAME_2, ret);
        return ERROR;
    }  
#endif

#ifdef BSP_SPI1_USING_SS3
    static struct SpiHardwareDevice spi_device3;
    memset(&spi_device3, 0, sizeof(struct SpiHardwareDevice));

    spi_initparam->spi_slave_id[SPI_DEVICE_SLAVE_ID_3] = SPI_DEVICE_SLAVE_ID_3;
    spi_initparam->spi_cs_gpio_pin[SPI_DEVICE_SLAVE_ID_3] = SPI1_CS3_PIN;
    spi_initparam->spi_cs_select_id[SPI_DEVICE_SLAVE_ID_3] = SPI_CHIP_SELECT_3;

    spi_device3.spi_dev_done = &(spi_dev_done);

    ret = SpiDeviceRegister(&spi_device3, (void *)(&spi_device3.spi_param), SPI_1_DEVICE_NAME_3);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceInit device %s error %d\n", SPI_1_DEVICE_NAME_3, ret);
        return ERROR;
    } 

    ret = SpiDeviceAttachToBus(SPI_1_DEVICE_NAME_3, SPI_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDeviceAttachToBus device %s error %d\n", SPI_1_DEVICE_NAME_3, ret);
        return ERROR;
    }  
#endif
    return  ret;
}

/*RISC-V 64 BOARD SPI INIT*/
int HwSpiInit(void)
{
    x_err_t ret = EOK;
    static struct SpiDmaParam spi_initparam;
    memset(&spi_initparam, 0, sizeof(struct SpiDmaParam));

#ifdef  BSP_USING_SPI1

    static struct SpiBus spi_bus;
    memset(&spi_bus, 0, sizeof(struct SpiBus));

    static struct SpiDriver spi_driver;
    memset(&spi_driver, 0, sizeof(struct SpiDriver));

    spi_initparam.spi_master_id = SPI_DEVICE_1;
    spi_initparam.spi_dmac_txchannel = DMAC_CHANNEL1;
    spi_initparam.spi_dmac_rxchannel = DMAC_CHANNEL2;
    
    spi_driver.configure = &(SpiDrvConfigure);

    ret = BoardSpiBusInit(&spi_bus, &spi_driver);
    if (EOK != ret) {
        KPrintf("Board_Spi_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSpiDevBend(&spi_initparam);
    if (EOK != ret) {
        KPrintf("Board_Spi_Init error ret %u\n", ret);
        return ERROR;
    }    
#endif
    return ret;
}
