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
* @file connect_fsmc.c
* @brief support extern memory by fsmc
* @version 1.0
* @author AIIT XUOS Lab
* @date 2021-05-28
*/

#include "connect_fsmc.h"
#include "hardware_fsmc.h"
#include "hardware_gpio.h"
#include "hardware_rcc.h"
#include <string.h>
#include <xs_base.h>

#define FSMC_BANK1_NORSRAM3_START_ADDRESS 0x68000000

static FSMC_NORSRAMInitTypeDef hsram3;
static FSMC_NORSRAMTimingInitTypeDef hsram_read3;
static FSMC_NORSRAMTimingInitTypeDef hsram_write3;

extern void ExtSramInitBoardMemory(void *start_phy_address, void *end_phy_address, uint8 extsram_idx);

int HwSramInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC); 
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);
 
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);
	
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource0,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource1,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource2,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource3,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource4,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource5,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource12,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource13,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource14,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource15,GPIO_AF_FSMC);
		
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource0,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource1,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource2,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource3,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource4,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource5,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);

	hsram3.FSMC_ReadWriteTimingStruct = &hsram_read3;
	hsram3.FSMC_WriteTimingStruct = &hsram_write3;

    /* hsram3.Init */
    hsram3.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    hsram3.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    hsram3.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    hsram3.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    hsram3.FSMC_WrapMode = FSMC_WrapMode_Disable;
    hsram3.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    hsram3.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    hsram3.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    hsram3.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    hsram3.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

    hsram_read3.FSMC_AddressSetupTime = 0;
    hsram_read3.FSMC_AddressHoldTime = 0;
    hsram_read3.FSMC_DataSetupTime = 8;
    hsram_read3.FSMC_BusTurnAroundDuration = 0;
    hsram_read3.FSMC_CLKDivision = 0;
    hsram_read3.FSMC_DataLatency = 0;
    hsram_read3.FSMC_AccessMode = FSMC_AccessMode_A;

    hsram_write3.FSMC_AddressSetupTime = 0;
    hsram_write3.FSMC_AddressHoldTime = 0;
    hsram_write3.FSMC_DataSetupTime = 8;
    hsram_write3.FSMC_BusTurnAroundDuration = 0;
    hsram_write3.FSMC_CLKDivision = 0;
    hsram_write3.FSMC_DataLatency = 0;
    hsram_write3.FSMC_AccessMode = FSMC_AccessMode_A;

#ifdef BSP_USING_FSMC_BANK1_NORSRAM3
    hsram3.FSMC_Bank = FSMC_Bank1_NORSRAM3;
#if BANK1_NORSRAM3_DATA_WIDTH == 8
    hsram3.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
#elif BANK1_NORSRAM3_DATA_WIDTH == 16
    hsram3.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
#else
    hsram3.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_32b;
#endif
	FSMC_NORSRAMInit(&hsram3);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
	
	ExtSramInitBoardMemory((void*)(FSMC_BANK1_NORSRAM3_START_ADDRESS), (void*)((FSMC_BANK1_NORSRAM3_START_ADDRESS + BANK1_NORSRAM3_SIZE)), 2);

#endif

    return 0;
}