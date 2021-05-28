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
* @file board.c
* @brief support aiit-arm32-board init configure and start-up
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: board.c
Description: support aiit-arm32-board init configure and driver/task/... init
Others: 
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-arm32-board InitBoardHardware
*************************************************/

#include <board.h>
#include <connect_gpio.h>
#include <connect_usart.h>
#include <hardware_rcc.h>
#include <misc.h>

extern void entry(void);
extern int Stm32HwPinInit(void);
extern int Stm32HwTimerInit(void);
extern int HwWdtInit(void);
extern int Stm32HwI2cInit(void);
extern int Stm32HwCh438Init(void);
extern int Stm32HwSpiInit(void);
extern int Stm32HwUsartInit();
extern int Stm32HwRtcInit();
extern int Stm32HwTouchBusInit(void);
extern int Stm32HwCanBusInit(void);
extern int HwSdioInit();

static void ClockConfiguration()
{
    int cr,cfgr,pllcfgr;
    int cr1,cfgr1,pllcfgr1;
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;
    RCC_HSEConfig(RCC_HSE_ON);
    if (RCC_WaitForHSEStartUp() == SUCCESS)
    {
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div2);
        RCC_PCLK1Config(RCC_HCLK_Div4);
        RCC_PLLConfig(RCC_PLLSource_HSE, 8, 336, 2, 7);

        RCC_PLLCmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        
        FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
    }
    SystemCoreClockUpdate();

}

void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

void  SysTickConfiguration(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    uint32         cnts;

    RCC_GetClocksFreq(&rcc_clocks);

    cnts = (uint32)rcc_clocks.HCLK_Frequency / TICK_PER_SECOND;
    cnts = cnts / 8;

    SysTick_Config(cnts);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

void SysTick_Handler(int irqn, void *arg)
{

    TickAndTaskTimesliceUpdate();

}
DECLARE_HW_IRQ(SysTick_IRQn, SysTick_Handler, NONE);

void stm32f407_start()
{
    entry();
}
struct InitSequenceDesc _board_init[] = 
{
#ifdef BSP_USING_GPIO
    { "hw pin", Stm32HwGpioInit },
#endif
#ifdef BSP_USING_CH438
    { "hw_extuart", Stm32HwCh438Init },
#endif
#ifdef BSP_USING_HWTIMER
	{ "hw timer", Stm32HwTimerInit },
#endif
#ifdef BSP_USING_SPI
	{ "hw spi", Stm32HwSpiInit },
#endif
#ifdef BSP_USING_WDT
	{ "hw wdg", HwWdtInit },
#endif
#ifdef BSP_USING_I2C
	{ "hw i2c", Stm32HwI2cInit},
#endif
#ifdef BSP_USING_RTC
	{ "hw rtc", Stm32HwRtcInit},
#endif
#ifdef BSP_USING_TOUCH
    { "hw touch", Stm32HwTouchBusInit},
#endif
#ifdef  BSP_USING_CAN
  { "hw can", Stm32HwCanBusInit},
#endif   
#ifdef BSP_USING_SDIO
    {"hw sdcard init",HwSdioInit},
#endif
	{ " NONE ",NONE },
};

void InitBoardHardware()
{
	int i = 0;
	int ret = 0;
    ClockConfiguration();

    NVIC_Configuration();

    SysTickConfiguration();
#ifdef BSP_USING_UART
	Stm32HwUsartInit();
#endif
#ifdef KERNEL_CONSOLE
    InstallConsole(KERNEL_CONSOLE_BUS_NAME, KERNEL_CONSOLE_DRV_NAME, KERNEL_CONSOLE_DEVICE_NAME);
    KPrintf("\nconsole init completed.\n");
    
    KPrintf("board initialization......\n");
#endif

#ifdef BSP_USING_EXTMEM
    extern int HwSramInit(void);
    HwSramInit();
#endif

    InitBoardMemory((void*)MEMORY_START_ADDRESS, (void*)MEMORY_END_ADDRESS);

#ifdef SEPARATE_COMPILE

    // init mpu
    //extern int MPU_Init(void);
    //extern struct task_mpu_config *mpuTaskInit(void);
    //extern void MPULoad(struct task_mpu_config *mpu);
    //extern void MPULoad1(struct task_mpu_config *mpu);
    //extern void MPURead(void);
    //MPU_Init();
    //MPULoad1(mpuTaskInit());
    //KPrintf("read mpu after load\n");
    //MPURead();
#endif

	for(i = 0; _board_init[i].fn != NONE; i++) {
		ret = _board_init[i].fn();
		KPrintf("initialize %s %s\n",_board_init[i].fn_name, ret == 0 ? "success" : "failed");
	}
  
    KPrintf("board init done.\n");
	KPrintf("start kernel...\n");
    KPrintf("%d",RCC_GetSYSCLKSource());
}
