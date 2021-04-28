/**
  ******************************************************************************
  * @file    IO_Toggle/stm32f4xx_conf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Library configuration file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/**
* @file: hardware_conf.h
* @brief: define hardware conf function
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/25
*/

/*************************************************
File name: hardware_conf.h
Description: define hardware conf function
Others: 
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. rename stm32f4xx_conf.h for XiUOS
*************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HARDWARE_CONF_H__
#define __HARDWARE_CONF_H__

#if defined  (HSE_VALUE)
/* Redefine the HSE value; it's equal to 8 MHz on the STM32F4-DISCOVERY Kit */
 #undef HSE_VALUE
 #define HSE_VALUE    ((uint32_t)8000000)
#endif /* HSE_VALUE */

/* Includes ------------------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
#include "hardware_can.h"
#include "hardware_dbgmcu.h"
#include "hardware_dma.h"
#include "hardware_exti.h"
#include "hardware_fsmc.h"
#include "hardware_gpio.h"
#include "hardware_i2c.h"
#include "hardware_iwdg.h"
#include "hardware_pwr.h"
#include "hardware_rcc.h"
#include "hardware_rtc.h"
#include "hardware_sdio.h"
#include "hardware_spi.h"
#include "hardware_syscfg.h"
#include "hardware_tim.h"
#include "hardware_usart.h"
#include "hardware_wwdg.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* If an external clock source is used, then the value of the following define
   should be set to the value of the external clock source, else, if no external
   clock is used, keep this define commented */
/*#define I2S_EXTERNAL_CLOCK_VAL   12288000 */ /* Value of the external clock in Hz */


/* Uncomment the line below to expanse the "assert_param" macro in the
   Standard Peripheral Library drivers code */
#define USE_FULL_ASSERT    1

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __HARDWARE_CONF_H__ */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
