/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 * 2018-12-11     greedyhao    Porting for stm32f7xx
 * 2019-01-03     zylx         modify DMA initialization and spixfer function
 * 2020-01-15     whj4674672   Porting for stm32h7xx
 * 2020-06-18     thread-liu   Porting for stm32mp1xx
 * 2020-10-14     Dozingfiretruck   Porting for stm32wbxx
 */

/**
* @file connect_spi.c
* @brief support aiit-arm32-board spi function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_spi.c
Description: support aiit-arm32-board spi configure and spi bus register function
Others: take RT-Thread v4.0.2/bsp/stm32/libraries/HAL_Drivers/drv_spi.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board spi configure, write and read
2. support aiit-arm32-board spi bus device and driver register
*************************************************/

#include <board.h>
#include <connect_spi.h>
#include <hardware_spi.h>
#include <hardware_dma.h>
#include <hardware_gpio.h>
#include <hardware_rcc.h>
#include <misc.h>
#include <stm32f4xx.h>

/* SPI GPIO define */
#ifdef BSP_USING_SPI1
#define SPI1_GPIO_NSS       GPIO_Pin_4
#define SPI1_NSS_PIN_SOURCE GPIO_PinSource4
#define SPI1_GPIO_SCK       GPIO_Pin_5
#define SPI1_SCK_PIN_SOURCE GPIO_PinSource5
#define SPI1_GPIO_MISO       GPIO_Pin_6
#define SPI1_MISO_PIN_SOURCE GPIO_PinSource6
#define SPI1_GPIO_MOSI       GPIO_Pin_7
#define SPI1_MOSI_PIN_SOURCE GPIO_PinSource7
#define SPI1_GPIO          GPIOA
#define SPI1_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_SPI1 RCC_APB2Periph_SPI1
#endif

#ifdef BSP_USING_SPI2
#define SPI2_GPIO_NSS       GPIO_Pin_6
#define SPI2_NSS_PIN_SOURCE GPIO_PinSource6
#define SPI2_GPIO_SCK       GPIO_Pin_13
#define SPI2_SCK_PIN_SOURCE GPIO_PinSource13
#define SPI2_GPIO_MISO       GPIO_Pin_2
#define SPI2_MISO_PIN_SOURCE GPIO_PinSource2
#define SPI2_GPIO_MOSI       GPIO_Pin_3
#define SPI2_MOSI_PIN_SOURCE GPIO_PinSource3
#define SPI2_GPIO          GPIOC
#define SPI2_SCK            GPIOB
#define SPI2_GPIO_RCC      RCC_AHB1Periph_GPIOC
#define SPI2_GPIO_RCC_SCK   RCC_AHB1Periph_GPIOB
#define RCC_APBPeriph_SPI2 RCC_APB1Periph_SPI2
#endif

#ifdef BSP_USING_SPI3
#define SPI3_GPIO_NSS       GPIO_Pin_15
#define SPI3_NSS_PIN_SOURCE GPIO_PinSource15
#define SPI3_GPIO_SCK       GPIO_Pin_10
#define SPI3_SCK_PIN_SOURCE GPIO_PinSource10
#define SPI3_GPIO_MISO       GPIO_Pin_11
#define SPI3_MISO_PIN_SOURCE GPIO_PinSource11
#define SPI3_GPIO_MOSI       GPIO_Pin_12
#define SPI3_MOSI_PIN_SOURCE GPIO_PinSource12
#define SPI3_GPIO          GPIOC
#define SPI3_NSS            GPIOA
#define SPI3_GPIO_RCC      RCC_AHB1Periph_GPIOC
#define SPI3_GPIO_RCC_NSS   RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_SPI3 RCC_APB2Periph_SPI3
#endif

/**
 * This function                        SPI device initialization
 *
 * @param SpiDrv                  SPI device structure   pointer
 * 
 *  @param cfg                        SPI device operating mode configuration structure   pointer
 * 
 *  @return                                 if   successful   return  EOK
 */

static x_err_t Stm32SpiInit(struct Stm32Spi *SpiDrv, struct SpiMasterParam *cfg)
{
    NULL_PARAM_CHECK(SpiDrv);
    NULL_PARAM_CHECK(cfg);

    SPI_InitTypeDef *SpiInit = &SpiDrv->init;

    if (cfg->spi_work_mode & DEV_SPI_SLAVE)
    {
        SpiInit->SPI_Mode = SPI_Mode_Slave;
    }
    else
    {
        SpiInit->SPI_Mode = SPI_Mode_Master;
    }

    if (cfg->spi_work_mode & SPI_3WIRE)
    {
        SpiInit->SPI_Direction = SPI_Direction_1Line_Rx;
    }
    else
    {
        SpiInit->SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    }

    if (cfg->spi_data_bit_width == 8)
    {
        SpiInit->SPI_DataSize = SPI_DataSize_8b;
    }
    else if (cfg->spi_data_bit_width == 16)
    {
        SpiInit->SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        return EPIO;
    }

    if (cfg->spi_work_mode & SPI_LINE_CPHA)
    {
        SpiInit->SPI_CPHA = SPI_CPHA_2Edge;
    }
    else
    {
        SpiInit->SPI_CPHA = SPI_CPHA_1Edge;
    }

    if (cfg->spi_work_mode & SPI_LINE_CPOL)
    {
        SpiInit->SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        SpiInit->SPI_CPOL = SPI_CPOL_Low;
    }

    if (cfg->spi_work_mode & SPI_NO_CS)
    {
        SpiInit->SPI_NSS = SPI_NSS_Soft;
    }
    else
    {
        SpiInit->SPI_NSS = SPI_NSS_Soft;
    }

    uint32_t SPI_APB_CLOCK;

    SPI_APB_CLOCK = system_core_clock>>apb_presc_table[(RCC->CFGR & RCC_CFGR_PPRE2)>> 13U];

    if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 2)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 4)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 8)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 16)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 32)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 64)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    }
    else if (cfg->spi_maxfrequency >= SPI_APB_CLOCK / 128)
    {
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    }
    else
    {
        /* min prescaler 256 */
        SpiInit->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    }
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);

    if (cfg->spi_work_mode & SPI_MSB)
    {
        SpiInit->SPI_FirstBit = SPI_FirstBit_MSB;       /*Highest bit transmit first*/
    }
    else
    {
        SpiInit->SPI_FirstBit = SPI_FirstBit_LSB;          /*lowest bit transmit first*/
    }

    SpiInit->SPI_CRCPolynomial = 7;

    SPI_Init(SpiDrv->instance, SpiInit);

    SPI_Cmd(SpiDrv->instance, ENABLE);

    uint32_t prioritygroup = 0x00U;
    /* DMA configuration */
    if (SpiDrv->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
    {
        DMA_Init(SpiDrv->dma.dma_rx.instance, &SpiDrv->dma.dma_rx.init);
        
        prioritygroup = NVIC_GetPriorityGrouping();

        NVIC_SetPriority(SpiDrv->dma.dma_rx.dma_irq, NVIC_EncodePriority(prioritygroup, 0, 0));

        NVIC_EnableIRQ(SpiDrv->dma.dma_rx.dma_irq);
    }

    if (SpiDrv->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
    {
        DMA_Init(SpiDrv->dma.dma_tx.instance, &SpiDrv->dma.dma_tx.init);

        prioritygroup = NVIC_GetPriorityGrouping();

        NVIC_SetPriority(SpiDrv->dma.dma_tx.dma_irq, NVIC_EncodePriority(prioritygroup, 0, 1));

        NVIC_EnableIRQ(SpiDrv->dma.dma_tx.dma_irq);
    }

    SpiDrv->instance->CR1 |= SPI_CR1_SPE;

    return EOK;
}
/**
 * This function                                 Use DMA during spi transfer
 *
 * @param spi_bus                          SPI bus handle
 * 
 *  @param SettingLen                   Set data length
 * 
 * @param RxBaseAddr                 Base address for receiving data
 * 
 * @param TxBaseAddr                 Base address for sending data
 * 
 * @return                                          none
 */

static void DmaSpiConfig(struct SpiBus *spi_bus, uint32_t setting_len, void *rx_base_addr, void *tx_base_addr)
{
    struct Stm32Spi *spi = (struct Stm32Spi *)spi_bus->private_data;
    uint32 tmpreg = 0x00U;
    NVIC_InitTypeDef NVIC_InitStructure;

    if(spi->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
    {
      spi->dma.dma_rx.setting_len = setting_len;
      DMA_DeInit(spi->dma.dma_rx.instance);  
      while(DMA_GetCmdStatus(spi->dma.dma_rx.instance) != DISABLE);
      spi->dma.dma_rx.init.DMA_Channel = spi->dma.dma_rx.channel;
      spi->dma.dma_rx.init.DMA_PeripheralBaseAddr = (uint32_t)&(spi->instance->DR);
      spi->dma.dma_rx.init.DMA_Memory0BaseAddr = (uint32_t)rx_base_addr;
      spi->dma.dma_rx.init.DMA_DIR = DMA_DIR_PeripheralToMemory;
      spi->dma.dma_rx.init.DMA_BufferSize = spi->dma.dma_rx.setting_len;
      spi->dma.dma_rx.init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
      spi->dma.dma_rx.init.DMA_MemoryInc = DMA_MemoryInc_Enable;
      spi->dma.dma_rx.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
      spi->dma.dma_rx.init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
      spi->dma.dma_rx.init.DMA_Mode = DMA_Mode_Normal;
      spi->dma.dma_rx.init.DMA_Priority = DMA_Priority_High;
      spi->dma.dma_rx.init.DMA_FIFOMode = DMA_FIFOMode_Disable;
      spi->dma.dma_rx.init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
      spi->dma.dma_rx.init.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
      spi->dma.dma_rx.init.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
      DMA_Init(spi->dma.dma_rx.instance, &spi->dma.dma_rx.init);
      DMA_ITConfig(spi->dma.dma_rx.instance, DMA_IT_TC, ENABLE);

      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

      DMA_Cmd(spi->dma.dma_rx.instance, ENABLE);

      NVIC_InitStructure.NVIC_IRQChannel = spi->dma.dma_rx.dma_irq;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);

      tmpreg = 0x00U;
      RCC->AHB1ENR |= spi->dma.dma_rx.dma_rcc;
      tmpreg = RCC->AHB1ENR & spi->dma.dma_rx.dma_rcc;
      (void)tmpreg;
    }

    if(spi->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
    {
      spi->dma.dma_tx.setting_len = setting_len;
      DMA_DeInit(spi->dma.dma_tx.instance);  
      while(DMA_GetCmdStatus(spi->dma.dma_tx.instance) != DISABLE);
      spi->dma.dma_tx.init.DMA_PeripheralBaseAddr = (uint32_t)&(spi->instance->DR);
      spi->dma.dma_tx.init.DMA_Memory0BaseAddr = (uint32_t)tx_base_addr;
      spi->dma.dma_tx.init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
      spi->dma.dma_tx.init.DMA_BufferSize = spi->dma.dma_tx.setting_len;
      spi->dma.dma_tx.init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
      spi->dma.dma_tx.init.DMA_MemoryInc = DMA_MemoryInc_Enable;
      spi->dma.dma_tx.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
      spi->dma.dma_tx.init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
      spi->dma.dma_tx.init.DMA_Mode = DMA_Mode_Normal;
      spi->dma.dma_tx.init.DMA_Priority = DMA_Priority_Low;
      spi->dma.dma_tx.init.DMA_FIFOMode = DMA_FIFOMode_Disable;
      spi->dma.dma_tx.init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
      spi->dma.dma_tx.init.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
      spi->dma.dma_tx.init.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
      DMA_Init(spi->dma.dma_tx.instance, &spi->dma.dma_tx.init);
      DMA_ITConfig(spi->dma.dma_tx.instance, DMA_IT_TC, ENABLE);

      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

      DMA_Cmd(spi->dma.dma_tx.instance, ENABLE);

      NVIC_InitStructure.NVIC_IRQChannel = spi->dma.dma_tx.dma_irq;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);

      tmpreg = 0x00U;
      RCC->AHB1ENR |= spi->dma.dma_rx.dma_rcc;
      tmpreg = RCC->AHB1ENR & spi->dma.dma_rx.dma_rcc;
      (void)tmpreg;
    }
}

/**
 * This function                   DMA receiving completion interrupt
 *
 * @param spi_bus           SPI bus pointer
 * 
 * @return                            none
 */
static void DmaRxDoneIsr(struct SpiBus *spi_bus)
{
    struct Stm32Spi *spi = (struct Stm32Spi *) spi_bus->bus.private_data;
    x_size_t recv_len;
    x_base level;

    if (DMA_GetFlagStatus(spi->dma.dma_rx.instance, spi->spi_dma_flag) != RESET)
    {
        level = CriticalAreaLock();

        recv_len = spi->dma.dma_rx.setting_len - spi->dma.dma_rx.last_index;
        spi->dma.dma_rx.last_index = 0;
        CriticalAreaUnLock(level);
        DMA_ClearFlag(spi->dma.dma_rx.instance, spi->spi_dma_flag);
    }
}

/**
 * This function                      DMA sending completion interrupt
 *
 * @param spi_bus             SPI bus pointer
 * 
 * @return                               none
 */
static void DmaTxDoneIsr(struct SpiBus *spi_bus)
{
    struct Stm32Spi *spi = (struct Stm32Spi *) spi_bus->bus.private_data;
    x_size_t send_len;
    x_base level;

    if (DMA_GetFlagStatus(spi->dma.dma_tx.instance, spi->spi_dma_flag) != RESET)
    {
        level = CriticalAreaLock();

        send_len = spi->dma.dma_tx.setting_len - spi->dma.dma_tx.last_index;
        spi->dma.dma_tx.last_index = 0;
        CriticalAreaUnLock(level);
        DMA_ClearFlag(spi->dma.dma_tx.instance, spi->spi_dma_flag);
    }
}
/**
 * This function                         SPI wait timeout function
 *
 * @param SpiInit                 SPI Init structure definition  
 * 
 *  @param SpiInstance       SPI control handle
 * 
 * @param Flag                          SPI  Status flag
 * 
 * @param State                        SPI status
 * 
 * @param Timeout                Overflow time
 * 
 * @param Tickstart                Starting time
 * 
 * @return                                   if   successful   return  EOK
 */
static int SpiWaitUntilTimeout(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, uint32_t Flag, uint32_t State, uint32_t Timeout, uint32_t Tickstart)
{
  while((((SpiInstance->SR & Flag) == (Flag)) ? SET : RESET) != State)
  {
    if(Timeout != 0xFFFFFFFFU)
    {
      if((Timeout == 0U) || ((CurrentTicksGain() * 1000 / TICK_PER_SECOND-Tickstart) >= Timeout))
      {
        SpiInstance->CR2 &= (~(SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

        if((SpiInit.SPI_Mode == SPI_Mode_Master)&&((SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)||(SpiInit.SPI_Direction == SPI_Direction_2Lines_RxOnly)))
        {
          /* Disable SPI peripheral */
          SpiInstance->CR1 &= (~SPI_CR1_SPE);
        }

        return 3;
      }
    }
  }
  return 0;
}
/**
 * This function                                   SPI sends data through DMA
 *
 * @param SpiInit                           SPI Init structure  
 * 
 *  @param SpiInstance                SPI control handle
 * 
 *  @param dma_init                        DMA Init structure
 * 
 *  @param dma_instance             DMA Controller
 * 
 *  @param pData                              Send data buffer address
 * 
 * @param Size                                   Amount of data sent
 * 
 * @return                                            if   successful   return  EOK
 */
int SpiTransmitDma(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, DMA_InitTypeDef dma_init, DMA_Stream_TypeDef *dma_instance, uint8_t *pData, uint16_t Size)
{
  int errorcode = 0;
  __IO uint16_t              TxXferCount;
  uint8_t                    *pTxBuffPtr;
  uint16_t                   TxXferSize;

  if((pData == NONE) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  TxXferCount = Size;
  pTxBuffPtr  = (uint8_t *)pData;
  TxXferSize  = Size;

  /* Configure communication direction : 1Line */
  if(SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)
  {
    SpiInstance->CR1 |= SPI_CR1_BIDIOE;
  }

  /* Clear DBM bit */
  dma_instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  dma_instance->NDTR = TxXferCount;

  /* Memory to Peripheral */
  if((dma_init.DMA_DIR) == DMA_DIR_MemoryToPeripheral)
  {
    /* Configure DMA Stream destination address */
    dma_instance->PAR = SpiInstance->DR;

    /* Configure DMA Stream source address */
    dma_instance->M0AR = *pTxBuffPtr;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Stream source address */
    dma_instance->PAR = *pTxBuffPtr;

    /* Configure DMA Stream destination address */
    dma_instance->M0AR = SpiInstance->DR;
  }

    /* Enable Common interrupts*/
    dma_instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_HT;
    dma_instance->FCR |= DMA_IT_FE;
    dma_instance->CR |=  DMA_SxCR_EN;

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |=  SPI_CR1_SPE;
  }

  /* Enable the SPI Error Interrupt Bit */
  SpiInstance->CR2 |= SPI_CR2_ERRIE;

  /* Enable Tx DMA Request */
  SpiInstance->CR2 |=SPI_CR2_TXDMAEN;

error :
  return errorcode;
}
/**
 * This function                             SPI carries out duplex communication through DMA
 *
 * @param SpiInit                       SPI Init structure  
 * 
 *  @param SpiInstance            SPI control handle
 * 
 * @param dmarx_init                DMA Init structure---Rx
 * 
 * @param dmarx_instance     DMA Controller
 * 
 * @param dmatx_init                DMA Init structure---Tx
 * 
 * @param dmatx_instance     DMA Controller
 * 
 * @param pTxData                      Send data buffer address
 * 
 * @param pRxData                     Receive data buffer address
 * 
 * @param Size                              Amount of data 
 * 
 * @return                                        if   successful   return  EOK
 */
int SpiTransmitreceiveDma(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, DMA_InitTypeDef dmarx_init, DMA_Stream_TypeDef *dmarx_instance, DMA_InitTypeDef dmatx_init, DMA_Stream_TypeDef *dmatx_instance, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
  uint32_t tmp = 0U;
  int errorcode = 0;
  __IO uint16_t              TxXferCount;
  __IO uint16_t              RxXferCount;
  uint8_t                    *pTxBuffPtr;
  uint8_t                    *pRxBuffPtr;
  uint16_t                   TxXferSize;
  uint16_t                   RxXferSize;

  tmp = SpiInit.SPI_Mode;
  if(!((tmp == SPI_Mode_Master) && (SpiInit.SPI_Direction == SPI_Direction_2Lines_FullDuplex)))
  {
    errorcode = 2;
    goto error;
  }

  if((pTxData == NONE ) || (pRxData == NONE ) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  /* Set the transaction information */
  pTxBuffPtr  = (uint8_t*)pTxData;
  TxXferSize  = Size;
  TxXferCount = Size;
  pRxBuffPtr  = (uint8_t*)pRxData;
  RxXferSize  = Size;
  RxXferCount = Size;

  /* Clear DBM bit */
  dmarx_instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  dmarx_instance->NDTR = RxXferCount;

  /* Memory to Peripheral */
  if((dmarx_init.DMA_DIR) == DMA_DIR_MemoryToPeripheral)
  {
    /* Configure DMA Stream destination address */
    dmarx_instance->PAR = SpiInstance->DR;

    /* Configure DMA Stream source address */
    dmarx_instance->M0AR = *pRxBuffPtr;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Stream source address */
    dmarx_instance->PAR = *pRxBuffPtr;

    /* Configure DMA Stream destination address */
    dmarx_instance->M0AR = SpiInstance->DR;
  }

  /* Enable Common interrupts*/
  dmarx_instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_HT;
  dmarx_instance->FCR |= DMA_IT_FE;
  dmarx_instance->CR |=  DMA_SxCR_EN;

  /* Enable Rx DMA Request */
  SpiInstance->CR2 |= SPI_CR2_RXDMAEN;

  /* Clear DBM bit */
  dmatx_instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  dmatx_instance->NDTR = TxXferCount;

  /* Memory to Peripheral */
  if((dmatx_init.DMA_DIR) == DMA_DIR_MemoryToPeripheral)
  {
    /* Configure DMA Stream destination address */
    dmatx_instance->PAR = SpiInstance->DR;

    /* Configure DMA Stream source address */
    dmatx_instance->M0AR = *pTxBuffPtr;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Stream source address */
    dmatx_instance->PAR = *pTxBuffPtr;

    /* Configure DMA Stream destination address */
    dmatx_instance->M0AR = SpiInstance->DR;
  }

  /* Enable Common interrupts*/
  dmatx_instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_HT;
  dmatx_instance->FCR |= DMA_IT_FE;
  dmatx_instance->CR |=  DMA_SxCR_EN;

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |= SPI_CR1_SPE;
  }
  /* Enable the SPI Error Interrupt Bit */
  SpiInstance->CR2 |= SPI_CR2_ERRIE;

  /* Enable Tx DMA Request */
  SpiInstance->CR2 |= SPI_CR2_TXDMAEN;

error :
  return errorcode;
}
/**
 * This function                             SPI receives data through DMA
 *
 * @param SpiInit                       SPI Init structure  
 * 
 *  @param SpiInstance            SPI control handle
 * 
 * @param dmarx_init                DMA Init structure---Rx
 * 
 * @param dmarx_instance     DMA Controller
 * 
 * @param dmatx_init                DMA Init structure---Tx
 * 
 * @param dmatx_instance     DMA Controller
 * 
 * @param pData                         Receive data buffer address
 * 
 * @param Size                             Amount of data 
 * 
 * @return                                        if   successful   return  EOK
 */
int SpiReceiveDma(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, DMA_InitTypeDef dmarx_init, DMA_Stream_TypeDef *dmarx_instance, DMA_InitTypeDef dmatx_init, DMA_Stream_TypeDef *dmatx_instance, uint8_t *pData, uint16_t Size)
{
  int errorcode = 0;
  __IO uint16_t              RxXferCount;
  uint8_t                    *pRxBuffPtr;
  uint16_t                   RxXferSize;

  if((SpiInit.SPI_Direction == SPI_Direction_2Lines_FullDuplex)&&(SpiInit.SPI_Mode == SPI_Mode_Master))
  {
     /* Call transmit-receive function to send Dummy data on Tx line and generate clock on CLK line */
     return SpiTransmitreceiveDma(SpiInit, SpiInstance, dmarx_init, dmarx_instance, dmatx_init, dmatx_instance, pData, pData, Size);
  }

  if((pData == NONE) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  RxXferCount = Size;
  RxXferSize  = Size;
  pRxBuffPtr  = (uint8_t *)pData;

  /* Configure communication direction : 1Line */
  if(SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)
  {
    SpiInstance->CR1 &= (~SPI_CR1_BIDIOE);
  }
  /* Clear DBM bit */
  dmarx_instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  dmarx_instance->NDTR = RxXferCount;

  /* Memory to Peripheral */
  if((dmarx_init.DMA_DIR) == DMA_DIR_MemoryToPeripheral)
  {
    /* Configure DMA Stream destination address */
    dmarx_instance->PAR = SpiInstance->DR;

    /* Configure DMA Stream source address */
    dmarx_instance->M0AR = *pRxBuffPtr;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Stream source address */
    dmarx_instance->PAR = *pRxBuffPtr;

    /* Configure DMA Stream destination address */
    dmarx_instance->M0AR = SpiInstance->DR;
  }

    /* Enable Common interrupts*/
    dmarx_instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_HT;
    dmarx_instance->FCR |= DMA_IT_FE;
    dmarx_instance->CR |=  DMA_SxCR_EN;

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |= SPI_CR1_SPE;
  }

  /* Enable the SPI Error Interrupt Bit */
  SpiInstance->CR2 |= SPI_CR2_ERRIE;

  /* Enable Rx DMA Request */
  SpiInstance->CR2 |= SPI_CR2_RXDMAEN;

error:
  return errorcode;
}
/**
 * This function                             SPI receives data 
 *
 * @param SpiInit                       SPI Init structure  
 * 
 *  @param SpiInstance            SPI control handle
 * 
 * @param pData                         Transmit data buffer address
 * 
 * @param Size                             Amount of data 
 * 
 * @param Timeout                    waiting time
 * 
 * @return                                        if   successful   return  EOK
 */
int SpiTransmit(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = 0U;
  int errorcode = 0;
  __IO uint16_t              TxXferCount;
  __IO uint32_t SPITimeout;

  /* Init tickstart for timeout management*/
  tickstart = CurrentTicksGain() * 1000 / TICK_PER_SECOND;

  if((pData == NONE ) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  TxXferCount = Size;

  /* Configure communication direction : 1Line */
  if(SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)
  {
    SpiInstance->CR1 |= SPI_CR1_BIDIOE;
  }

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |=  SPI_CR1_SPE;
  }

  /* Transmit data in 16 Bit mode */
  if(SpiInit.SPI_DataSize == SPI_DataSize_16b)
  {
    if((SpiInit.SPI_Mode == SPI_Mode_Slave) || (TxXferCount == 0x01))
    {
      SpiInstance->DR = *((uint16_t *)pData);

      pData += sizeof(uint16_t);
      TxXferCount--;
    }
    /* Transmit data in 16 Bit mode */
    while (TxXferCount > 0U)
    {
      SPITimeout = 0x1000;
      /* Wait until TXE flag is set to send data */
      while (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_TXE) == RESET)
      {
        if((SPITimeout--) == 0)
        {
          KPrintf("spi time out\n");
          errorcode = 3;
          goto error;
        }
      }
      /* Write to the data register and write the data to be written into the transmit buffer */
      SpiInstance->DR = *((uint16_t *)pData);

      pData += sizeof(uint16_t);
      TxXferCount--;
    }
  }
  /* Transmit data in 8 Bit mode */
  else
  {
    if((SpiInit.SPI_Mode == SPI_Mode_Slave) || (TxXferCount == 0x01U))
    {
      *((__IO uint8_t*)&SpiInstance->DR) = (*pData);

      pData += sizeof(uint8_t);
      TxXferCount--;
    }

    while(TxXferCount > 0)
    {
      SPITimeout = 0x1000;

      /* Wait for the send buffer to be empty, TXE event*/
      while (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_TXE) == RESET)
      {
        if((SPITimeout--) == 0)
        {
          KPrintf("spi time out\n");
          errorcode = 3;
          goto error;
        }
      }

      /* Write to the data register and write the data to be written into the transmit buffer*/
      *((__IO uint8_t*)&SpiInstance->DR) = (*pData);

      pData += sizeof(uint8_t);
      TxXferCount--;
    }
}

  /* Wait until TXE flag */
  if(SpiWaitUntilTimeout(SpiInit, SpiInstance, SPI_FLAG_TXE, SET, Timeout, tickstart) != 0)
  {
    errorcode = 3;
    goto error;
  }
  
  /* Check Busy flag */
  if(SpiWaitUntilTimeout(SpiInit, SpiInstance, SPI_FLAG_BSY, RESET, Timeout, tickstart) != 0)
  {
    errorcode = 1;
    goto error;
  }

error:
  return errorcode;
}
/**
 * This function                             SPI Transmit   and  receive
 *
 * @param SpiInit                       SPI Init structure  
 * 
 *  @param SpiInstance            SPI control handle
 * 
 * @param pTxData                      Transmit data buffer address
 * 
 * @param pRxData                     receive data buffer address
 * 
  * @param Size                              Amount of data 
 * 
 * @param Timeout                      waiting time
 * 
 * @return                                        if   successful   return  EOK
 */
int SpiTransmitreceive(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tmp = 0U;

  uint32_t tickstart = 0U;
  /* Variable used to alternate Rx and Tx during transfer */
  uint32_t txallowed = 1U;
  int errorcode = 0;
  __IO uint16_t              TxXferCount;
  __IO uint16_t              RxXferCount;

  /* Init tickstart for timeout management*/
  tickstart = CurrentTicksGain() * 1000 / TICK_PER_SECOND;
  
  tmp  = SpiInit.SPI_Mode;
  
  if(!((tmp == SPI_Mode_Master) && (SpiInit.SPI_Direction == SPI_Direction_2Lines_FullDuplex)))
  {
    errorcode = 2;
    goto error;
  }

  if((pTxData == NONE) || (pRxData == NONE) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  TxXferCount = Size;
  RxXferCount = Size;

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |=  SPI_CR1_SPE;
  }

  /* Transmit and Receive data in 16 Bit mode */
  if(SpiInit.SPI_DataSize == SPI_DataSize_16b)
  {
    if((SpiInit.SPI_Mode == SPI_Mode_Slave) || (TxXferCount == 0x01U))
    {
      SPI_I2S_ReceiveData(SpiInstance);
      SpiInstance->DR = *((uint16_t *)pTxData);

      pTxData += sizeof(uint16_t);
      TxXferCount--;
    }
    while ((TxXferCount > 0U) || (RxXferCount > 0U))
    {
      /* Check TXE flag */
      if(txallowed && (TxXferCount > 0U) && (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_TXE) == SET))
      {
        SPI_I2S_ReceiveData(SpiInstance);
        SpiInstance->DR = *((uint16_t *)pTxData);
        pTxData += sizeof(uint16_t);
        TxXferCount--;
        /* Next Data is a reception (Rx). Tx not allowed */ 
        txallowed = 0U;
      }

      /* Check RXNE flag */
      if((RxXferCount > 0U) && (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_RXNE) == SET))
      {
        *((uint16_t *)pRxData) = SpiInstance->DR;
        pRxData += sizeof(uint16_t);
        RxXferCount--;
        /* Next Data is a Transmission (Tx). Tx is allowed */ 
        txallowed = 1U;
      }
      if((Timeout != 0xFFFFFFFFU) && ((CurrentTicksGain() * 1000 / TICK_PER_SECOND-tickstart) >=  Timeout))
      {
        errorcode = 3;
        goto error;
      }
    }
  }
  /* Transmit and Receive data in 8 Bit mode */
  else
  {
    if((SpiInit.SPI_Mode == SPI_Mode_Slave) || (TxXferCount == 0x01U))
    {
      SPI_I2S_ReceiveData(SpiInstance);
      *((__IO uint8_t*)&SpiInstance->DR) = (*pTxData);

      pTxData += sizeof(uint8_t);
      TxXferCount--;
    }
    while((TxXferCount > 0U) || (RxXferCount > 0U))
    {
      /* Wait for the send buffer to be empty, TXE event */
      if (txallowed && (TxXferCount > 0U) && (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_TXE) == SET))
      {
        /* Write to the data register and write the data to be written into the transmit buffer */
        SPI_I2S_ReceiveData(SpiInstance);
        *((__IO uint8_t*)&SpiInstance->DR) = (*pTxData);

        pTxData += sizeof(uint8_t);
        TxXferCount--;
        txallowed = 0U;
      }

      if ((RxXferCount > 0U) && SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_RXNE) == SET)
      {
        *(uint8_t *)pRxData = SpiInstance->DR;

        pRxData += sizeof(uint8_t);
        RxXferCount--;
        txallowed = 1U;
      }
      if((Timeout != 0xFFFFFFFFU) && ((CurrentTicksGain() * 1000 / TICK_PER_SECOND-tickstart) >=  Timeout))
      {
        errorcode = 3;
        goto error;
      }
    }
  }

    /* Wait until TXE flag */
    if(SpiWaitUntilTimeout(SpiInit, SpiInstance, SPI_FLAG_TXE, SET, Timeout, tickstart) != 0)
    {
        errorcode = 3;
        goto error;
    }

    /* Check Busy flag */
    if(SpiWaitUntilTimeout(SpiInit, SpiInstance, SPI_FLAG_BSY, RESET, Timeout, tickstart) != 0)
    {
        errorcode = 1;
        goto error;
    }
  
error :
    return errorcode;
}
/**
 * This function                             SPI   receive  data
 *
 * @param SpiInit                       SPI Init structure  
 * 
 *  @param SpiInstance            SPI control handle
 * 
 * @param pData                          data buffer address
 * 
  * @param Size                              Amount of data 
 * 
 * @param Timeout                      waiting time
 * 
 * @return                                        if   successful   return  EOK
 */
int SpiReceive(SPI_InitTypeDef SpiInit, SPI_TypeDef *SpiInstance, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = 0U;
  int errorcode = 0;
  __IO uint16_t              RxXferCount;
  __IO uint32_t             SPITimeout;

  if((SpiInit.SPI_Mode == SPI_Mode_Master) && (SpiInit.SPI_Direction == SPI_Direction_2Lines_FullDuplex))
  {
     /* Call transmit-receive function to send Dummy data on Tx line and generate clock on CLK line */
    return SpiTransmitreceive(SpiInit, SpiInstance,pData,pData,Size,Timeout);
  }

  /* Init tickstart for timeout management*/
  tickstart = CurrentTicksGain() * 1000 / TICK_PER_SECOND;

  if((pData == NONE ) || (Size == 0))
  {
    errorcode = 1;
    goto error;
  }

  RxXferCount = Size;

  /* Configure communication direction: 1Line */
  if(SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)
  {
    SpiInstance->CR1 &= (~SPI_CR1_BIDIOE);
  }

  /* Check if the SPI is already enabled */
  if((SpiInstance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    SpiInstance->CR1 |=  SPI_CR1_SPE;
  }

    /* Receive data in 8 Bit mode */
  if(SpiInit.SPI_DataSize == SPI_DataSize_8b)
  {
    /* Transfer loop */
    while(RxXferCount > 0U)
    {
        SPITimeout = 0x1000;

        /* Wait for the send buffer to be empty, TXE event*/
        while (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_RXNE) == RESET)
        {
          if((SPITimeout--) == 0)
          {
            KPrintf("spi time out\n");
            errorcode = 3;
            goto error;
          }
        }

        *(uint8_t *)pData = SpiInstance->DR;

        pData += sizeof(uint8_t);
        RxXferCount--;
    }
  }
  else
  {
    /* Transfer loop */
    while(RxXferCount > 0U)
    {
      SPITimeout = 0x1000;
      /* Check the RXNE flag */
      while (SPI_I2S_GetFlagStatus(SpiInstance, SPI_I2S_FLAG_RXNE) == RESET)
      {
        if((SPITimeout--) == 0)
          {
            KPrintf("spi time out\n");
            errorcode = 3;
            goto error;
          }
      }
      *((uint16_t*)pData) = SpiInstance->DR;
      pData += sizeof(uint16_t);
      RxXferCount--;
    }
  }

  /* Check the end of the transaction */
  if((SpiInit.SPI_Mode == SPI_Mode_Master)&&((SpiInit.SPI_Direction == SPI_Direction_1Line_Rx)||(SpiInit.SPI_Direction == SPI_Direction_2Lines_RxOnly)))
  {
    /* Disable SPI peripheral */
    SpiInstance->CR1 &= (~SPI_CR1_SPE);
  }

error :
  return errorcode;
}

/**
 * This function                             SPI write data
 *
 * @param spi_dev                             SPI device structure  handle
 * 
 *  @param spi_datacfg                    SPI device information structure  handle
 * 
 * @return                                      datacfg  length
 */
static uint32 Stm32SpiWriteData(struct SpiHardwareDevice *spi_dev, struct SpiDataStandard *spi_datacfg)
{
    int state;
    x_size_t message_length, already_send_length;
    uint16 send_length;
    const uint8 *WriteBuf;

    NULL_PARAM_CHECK(spi_dev);
    NULL_PARAM_CHECK(spi_datacfg);

    struct Stm32Spi *StmSpi = CONTAINER_OF(spi_dev->haldev.owner_bus, struct Stm32Spi, spi_bus);
    SPI_TypeDef *SpiInstance = StmSpi->instance;
    SPI_InitTypeDef *SpiInit = &StmSpi->init;
    struct Stm32HwSpiCs *cs = (struct Stm32HwSpiCs *)spi_dev->private_data;

  while(NONE != spi_datacfg) {    
    if (spi_datacfg->spi_chip_select) {
        GPIO_WriteBit(cs->GPIOx, cs->GPIO_Pin, Bit_RESET);
    }

    message_length = spi_datacfg->length;
    WriteBuf = spi_datacfg->tx_buff;
    while (message_length) {
        if (message_length > 65535) {
            send_length = 65535;
            message_length = message_length - 65535;
        } else {
            send_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = spi_datacfg->length - send_length - message_length;
        WriteBuf = (uint8 *)spi_datacfg->tx_buff + already_send_length;
        
        /* start once data exchange in DMA mode */
        if (spi_datacfg->tx_buff) {
            if (StmSpi->spi_dma_flag & SPI_USING_TX_DMA_FLAG) {
                state = SpiTransmitDma(*SpiInit, SpiInstance, StmSpi->dma.dma_tx.init, StmSpi->dma.dma_tx.instance, (uint8_t *)WriteBuf, send_length);
            } else {
                state = SpiTransmit(*SpiInit, SpiInstance, (uint8_t *)WriteBuf, send_length, 1000);
            }
        }

        if (state != 0) {
            KPrintf("spi transfer error : %d\n", state);
            spi_datacfg->length = 0;
        }
    }

    if (spi_datacfg->spi_cs_release) {
        GPIO_WriteBit(cs->GPIOx, cs->GPIO_Pin, Bit_SET);
    }
  
    spi_datacfg = spi_datacfg->next;
  }

    return spi_datacfg->length;
}

/**
 * This function                             SPI read data
 *
 * @param spi_dev                             SPI device structure  handle
 * 
 *  @param spi_datacfg                    SPI device information structure  handle
 * 
 * @return                                      datacfg  length
 */
static uint32 Stm32SpiReadData(struct SpiHardwareDevice *spi_dev, struct SpiDataStandard *spi_datacfg)
{
    int state;
    x_size_t message_length, already_send_length;
    uint16 send_length;
    const uint8 *ReadBuf;

    NULL_PARAM_CHECK(spi_dev);
    NULL_PARAM_CHECK(spi_datacfg);

    struct Stm32Spi *StmSpi = CONTAINER_OF(spi_dev->haldev.owner_bus, struct Stm32Spi, spi_bus);
    SPI_TypeDef *SpiInstance = StmSpi->instance;
    SPI_InitTypeDef *SpiInit = &StmSpi->init;
    struct Stm32HwSpiCs *cs = (struct Stm32HwSpiCs *)spi_dev->private_data;

  while(NONE != spi_datacfg) {    
    if(spi_datacfg->spi_chip_select) {
        GPIO_WriteBit(cs->GPIOx, cs->GPIO_Pin, Bit_RESET);
    }

    message_length = spi_datacfg->length;
    ReadBuf = spi_datacfg->rx_buff;
    while (message_length) {
        if (message_length > 65535){
            send_length = 65535;
            message_length = message_length - 65535;
        } else {
            send_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = spi_datacfg->length - send_length - message_length;
        ReadBuf = (uint8 *)spi_datacfg->rx_buff + already_send_length;
        
        /* start once data exchange in DMA mode */
        if (spi_datacfg->rx_buff) {
            memset((uint8_t *)ReadBuf, 0xff, send_length);
            if (StmSpi->spi_dma_flag & SPI_USING_RX_DMA_FLAG) {
                state = SpiReceiveDma(*SpiInit, SpiInstance, StmSpi->dma.dma_rx.init, StmSpi->dma.dma_rx.instance, StmSpi->dma.dma_tx.init, StmSpi->dma.dma_tx.instance, (uint8_t *)ReadBuf, send_length);
            } else {
                state = SpiReceive(*SpiInit, SpiInstance, (uint8_t *)ReadBuf, send_length, 1000);
            }
        }

        if (state != 0) {
            KPrintf("spi transfer error : %d\n", state);
            spi_datacfg->length = 0;
        }
    }

    if (spi_datacfg->spi_cs_release) {
        GPIO_WriteBit(cs->GPIOx, cs->GPIO_Pin, Bit_SET);
    }
  
    spi_datacfg = spi_datacfg->next;
  }

    return spi_datacfg->length;
}

/**
 * This function                                 SPI driver initialization function
 *
 * @param spi_drv                            SPI driver structure  handle
 * 
 * @return                                             if   successful   return  EOK
 */
static uint32 SpiDrvInit(struct SpiDriver *spi_drv)
{
    NULL_PARAM_CHECK(spi_drv);

    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_drv->driver.private_data);

    struct Stm32Spi *StmSpi = CONTAINER_OF(spi_drv->driver.owner_bus, struct Stm32Spi, spi_bus);

    return Stm32SpiInit(StmSpi, dev_param->spi_master_param);
}

/**
 * This function                                 SPI driver configuration param
 *
 * @param spi_drv                           SPI driver structure  handle
 * 
 *  @param spi_param                   SPI master param structure  handle
 * 
 * @return                                            if   successful   return  EOK
 */
static uint32 SpiDrvConfigure(struct SpiDriver *spi_drv, struct SpiMasterParam *spi_param)
{
    NULL_PARAM_CHECK(spi_drv);
    NULL_PARAM_CHECK(spi_param);

    SpiDeviceParam *dev_param = (SpiDeviceParam *)(spi_drv->driver.private_data);

    dev_param->spi_master_param = spi_param;
    dev_param->spi_master_param->spi_work_mode = dev_param->spi_master_param->spi_work_mode  & SPI_MODE_MASK;    

    return EOK;
}

/*Configure the spi device param, make sure struct (configure_info->private_data) = (SpiMasterParam)*/
static uint32 Stm32SpiDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;   
    struct SpiDriver *spi_drv = (struct SpiDriver *)drv;
    struct SpiMasterParam *spi_param;

    switch (configure_info->configure_cmd)
    {
      case OPE_INT:
          ret = SpiDrvInit(spi_drv);
          break;
      case OPE_CFG:
          spi_param = (struct SpiMasterParam *)configure_info->private_data;
          ret = SpiDrvConfigure(spi_drv, spi_param);
          break;
      default:
          break;
    }

    return ret;
}

/*manage the spi device operations*/
static const struct SpiDevDone spi_dev_done =
{
  .dev_open = NONE,
  .dev_close = NONE,
  .dev_write = Stm32SpiWriteData,
  .dev_read = Stm32SpiReadData,
};

#if defined(BSP_USING_SPI1)
struct Stm32Spi spi1;
#if defined(BSP_SPI1_TX_USING_DMA)
void DMA2_Stream3_IRQHandler(int irq_num, void *arg)
{
    DmaTxDoneIsr(&spi1.spi_bus);
}
DECLARE_HW_IRQ(DMA2_Stream3_IRQn, DMA2_Stream3_IRQHandler, NONE);
#endif

#if defined(BSP_SPI1_RX_USING_DMA)
void DMA2_Stream0_IRQHandler(int irq_num, void *arg)
{
    DmaRxDoneIsr(&spi1.spi_bus);
}
DECLARE_HW_IRQ(DMA2_Stream0_IRQn, DMA2_Stream0_IRQHandler, NONE);
#endif
#endif

#if defined(BSP_USING_SPI2)
struct Stm32Spi spi2;
#if defined(BSP_SPI2_TX_USING_DMA)
void DMA1_Stream4_IRQHandler(int irq_num, void *arg)
{
    DmaTxDoneIsr(&spi2.spi_bus);
}
DECLARE_HW_IRQ(DMA1_Stream4_IRQn, DMA1_Stream4_IRQHandler, NONE);
#endif

#if defined(BSP_SPI2_RX_USING_DMA)
void DMA1_Stream3_IRQHandler(int irq_num, void *arg)
{
    DmaTxDoneIsr(&spi2.spi_bus);
}
DECLARE_HW_IRQ(DMA1_Stream3_IRQn, DMA1_Stream3_IRQHandler, NONE);
#endif
#endif

#if defined(BSP_USING_SPI3)
struct Stm32Spi spi3;
#if defined(BSP_SPI3_TX_USING_DMA)
void DMA1_Stream7_IRQHandler(int irq_num, void *arg)
{
    DmaTxDoneIsr(&spi3.spi_bus);
}
DECLARE_HW_IRQ(DMA1_Stream7_IRQn, DMA1_Stream7_IRQHandler, NONE);
#endif

#if defined(BSP_SPI3_RX_USING_DMA)
/**
 * This function                 DMA2  Stream2  Interrupt service function
 *
 * @return                            none
 */
void DMA1_Stream2_IRQHandler(int irq_num, void *arg)
{
    DmaRxDoneIsr(&spi3.spi_bus);
}
DECLARE_HW_IRQ(DMA1_Stream2_IRQn, DMA1_Stream2_IRQHandler, NONE);
#endif
#endif

/**
 * This function                 RCC clock configuration function  
 *
 * @return                            none
 */
static void RCCConfiguration(void)
{
#ifdef BSP_USING_SPI1
    RCC_AHB1PeriphClockCmd(SPI1_GPIO_RCC, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APBPeriph_SPI1, ENABLE);
#endif

#ifdef BSP_USING_SPI2
    RCC_AHB1PeriphClockCmd(SPI2_GPIO_RCC | SPI2_GPIO_RCC_SCK, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APBPeriph_SPI2, ENABLE);
#endif

#ifdef BSP_USING_SPI3
    RCC_AHB1PeriphClockCmd(SPI3_GPIO_RCC | SPI3_GPIO_RCC_NSS, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APBPeriph_SPI3, ENABLE);
#endif
}
/**
 * This function                   GPIO  Configuration      function  
 * 
 * @return                            none
 */
static void GPIOConfiguration(void)
{
  GPIO_InitTypeDef gpio_initstructure;

  gpio_initstructure.GPIO_Mode  = GPIO_Mode_AF;                 /*  Reuse function */
  gpio_initstructure.GPIO_OType = GPIO_OType_PP;             /*    Multiplex push-pull*/
  gpio_initstructure.GPIO_PuPd  = GPIO_PuPd_UP;               /*   pull  up */
  gpio_initstructure.GPIO_Speed = GPIO_Speed_2MHz;      /*    Level reversal speed  */

#ifdef BSP_USING_SPI1
  gpio_initstructure.GPIO_Pin = SPI1_GPIO_NSS | SPI1_GPIO_SCK | SPI1_GPIO_MISO | SPI1_GPIO_MOSI;
    /* Connect alternate function */
  GPIO_PinAFConfig(SPI1_GPIO, SPI1_NSS_PIN_SOURCE, GPIO_AF_SPI1);
  GPIO_PinAFConfig(SPI1_GPIO, SPI1_SCK_PIN_SOURCE, GPIO_AF_SPI1);
  GPIO_PinAFConfig(SPI1_GPIO, SPI1_MISO_PIN_SOURCE, GPIO_AF_SPI1);
  GPIO_PinAFConfig(SPI1_GPIO, SPI1_MOSI_PIN_SOURCE, GPIO_AF_SPI1);
  
  GPIO_Init(SPI1_GPIO, &gpio_initstructure);                                                              /*SPI pin initialization*/
#endif

#ifdef BSP_USING_SPI2
  gpio_initstructure.GPIO_Pin = SPI2_GPIO_SCK;
  /* Connect alternate function */
  GPIO_PinAFConfig(SPI2_SCK, SPI2_SCK_PIN_SOURCE, GPIO_AF_SPI2);

  GPIO_Init(SPI2_SCK, &gpio_initstructure);

  gpio_initstructure.GPIO_Pin = SPI2_GPIO_NSS | SPI2_GPIO_MISO | SPI2_GPIO_MOSI;
  /* Connect alternate function */
  GPIO_PinAFConfig(SPI2_GPIO, SPI2_NSS_PIN_SOURCE, GPIO_AF_SPI2);
  GPIO_PinAFConfig(SPI2_GPIO, SPI2_MISO_PIN_SOURCE, GPIO_AF_SPI2);
  GPIO_PinAFConfig(SPI2_GPIO, SPI2_MOSI_PIN_SOURCE, GPIO_AF_SPI2);
  
  GPIO_Init(SPI2_GPIO, &gpio_initstructure);
#endif

#ifdef BSP_USING_SPI3
  gpio_initstructure.GPIO_Pin = SPI3_GPIO_NSS;
  /* Connect alternate function */
  GPIO_PinAFConfig(SPI3_NSS, SPI3_NSS_PIN_SOURCE, GPIO_AF_SPI3);

  GPIO_Init(SPI3_NSS, &gpio_initstructure);

  gpio_initstructure.GPIO_Pin = SPI3_GPIO_SCK | SPI3_GPIO_MISO | SPI3_GPIO_MOSI;
  GPIO_PinAFConfig(SPI3_GPIO, SPI3_SCK_PIN_SOURCE, GPIO_AF_SPI3);
  GPIO_PinAFConfig(SPI3_GPIO, SPI3_MISO_PIN_SOURCE, GPIO_AF_SPI3);
  GPIO_PinAFConfig(SPI3_GPIO, SPI3_MOSI_PIN_SOURCE, GPIO_AF_SPI3);
  
  GPIO_Init(SPI3_GPIO, &gpio_initstructure);
#endif
}

/**
 * This function                                   Init the spi bus 、spi driver and attach to the bus          
 *
 * @param  spi_bus                           Spi bus info pointer
 * 
 *  @param spi_driver                      Spi driver info pointer
 * 
 * @return                                               EOK
 */
static int BoardSpiBusInit(struct Stm32Spi *stm32spi_bus, struct SpiDriver *spi_driver, char* drv_name)
{
    x_err_t ret = EOK;

    /*Init the spi bus */
    ret = SpiBusInit(&stm32spi_bus->spi_bus, stm32spi_bus->bus_name);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the spi driver*/
    ret = SpiDriverInit(spi_driver, drv_name);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the spi driver to the spi bus*/
    ret = SpiDriverAttachToBus(drv_name, stm32spi_bus->bus_name);
    if (EOK != ret) {
        KPrintf("Board_Spi_init SpiDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/**
 * This function                    SPI bus initialization            
 *
 * @return                              EOK
 */
static int Stm32HwSpiBusInit(void)
{
    x_err_t ret = EOK;
    struct Stm32Spi *StmSpiBus;

    RCCConfiguration();
    GPIOConfiguration();

#ifdef BSP_USING_SPI1
    StmSpiBus = &spi1;
    StmSpiBus->instance = SPI1;
    StmSpiBus->bus_name = SPI_BUS_NAME_1;
    StmSpiBus->spi_bus.private_data = &spi1;
    DmaSpiConfig(&StmSpiBus->spi_bus, 0, NONE, NONE);

    static struct SpiDriver spi_driver_1;
    memset(&spi_driver_1, 0, sizeof(struct SpiDriver));

    spi_driver_1.configure = &(Stm32SpiDrvConfigure);

    ret = BoardSpiBusInit(StmSpiBus, &spi_driver_1, SPI_1_DRV_NAME);
    if (EOK != ret) {
      KPrintf("Board_Spi_Init spi_bus_init %s error ret %u\n", StmSpiBus->bus_name, ret);
      return ERROR;
    }
#endif

#ifdef BSP_USING_SPI2
    StmSpiBus = &spi2;
    StmSpiBus->instance = SPI2;
    StmSpiBus->bus_name = SPI_BUS_NAME_2;
    StmSpiBus->spi_bus.private_data = &spi2;
    DmaSpiConfig(&StmSpiBus->spi_bus, 0, NONE, NONE);

    static struct SpiDriver spi_driver_2;
    memset(&spi_driver_2, 0, sizeof(struct SpiDriver));

    spi_driver_2.configure = &(Stm32SpiDrvConfigure);

    ret = BoardSpiBusInit(StmSpiBus, &spi_driver_2, SPI_2_DRV_NAME);
    if (EOK != ret) {
      return ERROR;
    }
#endif

#ifdef BSP_USING_SPI3
    StmSpiBus = &spi3;
    StmSpiBus->instance = SPI3;
    StmSpiBus->bus_name = SPI_BUS_NAME_3;
    StmSpiBus->spi_bus.private_data = &spi3;
    DmaSpiConfig(&StmSpiBus->spi_bus, 0, NONE, NONE);

    static struct SpiDriver spi_driver_3;
    memset(&spi_driver_3, 0, sizeof(struct SpiDriver));

    spi_driver_3.configure = &(Stm32SpiDrvConfigure);

    ret = BoardSpiBusInit(StmSpiBus, &spi_driver_3, SPI_3_DRV_NAME);
    if (EOK != ret) {
      KPrintf("Board_Spi_Init spi_bus_init %s error ret %u\n", StmSpiBus->bus_name, ret);
      return ERROR;
    }
#endif
    return EOK;
}

/**
 * This function                                   Mount the spi device to the bus           
 *
 * @param bus_name                         Bus   Name
 * 
 *  @param device_name                spi   device  name
 * 
 *  @param cs_gpiox                         GPIO pin configuration handle
 * 
 * @param cs_gpio_pin                    GPIO  number
 * 
 * @return                                               EOK
 */
x_err_t HwSpiDeviceAttach(const char *bus_name, const char *device_name, GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin)
{
    NULL_PARAM_CHECK(bus_name);
    NULL_PARAM_CHECK(device_name);

    x_err_t result;
    struct SpiHardwareDevice *spi_device;
    struct Stm32HwSpiCs *cs_pin_param;
    static SpiDeviceParam spi_dev_param;
    memset(&spi_dev_param, 0, sizeof(SpiDeviceParam));

    /* initialize the cs pin && select the slave*/
    GPIO_InitTypeDef GPIO_Initure;
    GPIO_Initure.GPIO_Pin = cs_gpio_pin;
    GPIO_Initure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Initure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(cs_gpiox, &GPIO_Initure);
    GPIO_WriteBit(cs_gpiox, cs_gpio_pin, Bit_SET);

    /* attach the device to spi bus*/
    spi_device = (struct SpiHardwareDevice *)x_malloc(sizeof(struct SpiHardwareDevice));
    CHECK(spi_device);
    memset(spi_device, 0, sizeof(struct SpiHardwareDevice));
    cs_pin_param = (struct Stm32HwSpiCs *)x_malloc(sizeof(struct Stm32HwSpiCs));
    CHECK(cs_pin_param);
    memset(cs_pin_param, 0, sizeof(struct Stm32HwSpiCs));
    cs_pin_param->GPIOx = cs_gpiox;
    cs_pin_param->GPIO_Pin = cs_gpio_pin;

    spi_device->spi_dev_done = &spi_dev_done;
    spi_device->private_data = (void *)cs_pin_param;

    result = SpiDeviceRegister(spi_device, (void *)&spi_dev_param, device_name);
    if (result != EOK) {
        SYS_ERR("%s device %p register faild, %d\n", device_name, spi_device, result);
    }

    result = SpiDeviceAttachToBus(device_name, bus_name);
    if (result != EOK) {
        SYS_ERR("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    CHECK(result == EOK);

    KPrintf("%s attach to %s done\n", device_name, bus_name);

    return result;
}

/**
 * This function                   Get DMA information            
 * 
 * @return                              none
 */
static void Stm32GetDmaInfo(void)
{
#ifdef BSP_SPI1_RX_USING_DMA                                            /*SPI1 uses DMA receive enable*/
    spi1.spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    spi1.dma.dma_rx.instance = DMA2_Stream0;
    spi1.dma.dma_rx.dma_rcc = RCC_AHB1ENR_DMA2EN;
    spi1.dma.dma_rx.channel = DMA_Channel_3;
    spi1.dma.dma_rx.dma_irq = DMA2_Stream0_IRQn;
#endif
#ifdef BSP_SPI1_TX_USING_DMA                                      /*SPI1 uses DMA    send   enable*/         
    spi1.spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    spi1.dma.dma_tx.instance = DMA2_Stream3;
    spi1.dma.dma_tx.dma_rcc = RCC_AHB1ENR_DMA2EN;
    spi1.dma.dma_tx.channel = DMA_Channel_3;
    spi1.dma.dma_tx.dma_irq = DMA2_Stream3_IRQn;
#endif

#ifdef BSP_SPI2_RX_USING_DMA                                                      /*SPI2  uses DMA receive enable*/
    spi2.spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    spi2.dma.dma_rx.instance = DMA1_Stream3;                         /* DMA1 Data stream 3*/
    spi2.dma.dma_rx.dma_rcc = RCC_AHB1ENR_DMA1EN;
    spi2.dma.dma_rx.channel = DMA_Channel_0;
    spi2.dma.dma_rx.dma_irq = DMA1_Stream3_IRQn;
#endif
#ifdef BSP_SPI2_TX_USING_DMA                                      /*SPI2 uses DMA send   enable*/
    spi2.spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    spi2.dma.dma_tx.instance = DMA1_Stream4;                      /* DMA1 Data stream 4*/
    spi2.dma.dma_tx.dma_rcc = RCC_AHB1ENR_DMA1EN;
    spi2.dma.dma_tx.channel = DMA_Channel_0;
    spi2.dma.dma_tx.dma_irq = DMA1_Stream4_IRQn;         /*Enable DMA interrupt line*/
#endif

#ifdef BSP_SPI3_RX_USING_DMA                                        /*SPI3 uses DMA receive enable*/
    spi3.spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    spi3.dma.dma_rx.instance = DMA1_Stream2;             /* DMA1 Data stream 2*/
    spi3.dma.dma_rx.dma_rcc = RCC_AHB1ENR_DMA1EN;
    spi3.dma.dma_rx.channel = DMA_Channel_0;
    spi3.dma.dma_rx.dma_irq = DMA1_Stream2_IRQn;         /*Enable DMA interrupt line*/
#endif
#ifdef BSP_SPI3_TX_USING_DMA                                       /*SPI3 uses DMA send   enable*/
    spi3.spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    spi3.dma.dma_tx.instance = DMA1_Stream7;           /* DMA1 Data stream 7*/
    spi3.dma.dma_tx.dma_rcc = RCC_AHB1ENR_DMA1EN;
    spi3.dma.dma_tx.channel = DMA_Channel_0;
    spi3.dma.dma_tx.dma_irq = DMA1_Stream7_IRQn;       /*Enable DMA interrupt line*/
#endif
}

/**
 * This function             hardware spi initialization             
 *
 * @return                             EOK
 */
int Stm32HwSpiInit(void)
{
    Stm32GetDmaInfo();
    return Stm32HwSpiBusInit();
}
