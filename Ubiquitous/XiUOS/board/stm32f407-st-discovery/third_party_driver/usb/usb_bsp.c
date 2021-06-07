/**
  ******************************************************************************
  * @file              usb_bsp.c
  * @author     xiuos Team
  * @version    V1.0.0
  * @date         2020-9-3
  * @brief         This file is based on    usb_bsp.c
  *                       This file is responsible to offer board support package and is
  *                        configurable by user.
  ******************************************************************************
  * @file            usb_bsp.c
  * @author    MCD Application Team
  * @version   V1.0.0
  * @date         30-September-2011
  * ********************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      <http://www.st.com/SLA0044>
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"

#include <xiuos.h>
#include <hardware_gpio.h>
#include <hardware_rcc.h>
#include <stm32f4xx.h>
#include <misc.h>

/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_BSP
  * @brief This file is responsible to offer board support package
  * @{
  */ 

/** @defgroup USB_BSP_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USB_BSP_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 





/** @defgroup USB_BSP_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBH_BSP_Private_Variables
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBH_BSP_Private_FunctionPrototypes
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_BSP_Private_Functions
  * @{
  */ 


/**
  * @brief  USB_OTG_BSP_Init
  *         Initializes BSP configurations
  * @param  None
  * @retval None
  */

int USB_OTG_BSP_Init_count = 0;

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
  GPIO_InitTypeDef  gpio_initstructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);

  gpio_initstructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  gpio_initstructure.GPIO_Mode = GPIO_Mode_AF;
  gpio_initstructure.GPIO_OType = GPIO_OType_PP;
  gpio_initstructure.GPIO_Speed = GPIO_Speed_100MHz;
  gpio_initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gpio_initstructure);

  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11, GPIO_AF_OTG_FS);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12, GPIO_AF_OTG_FS);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  gpio_initstructure.GPIO_Pin = GPIO_Pin_0;
  gpio_initstructure.GPIO_Mode = GPIO_Mode_OUT;
  gpio_initstructure.GPIO_OType = GPIO_OType_PP;
  gpio_initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio_initstructure);
}
/**
  * @brief  USB_OTG_BSP_ENABLE_INTERRUPT
  *         Enable USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_ENABLE_INTERRUPT(USB_OTG_CORE_HANDLE *pdev)
{
  isrManager.done->enableIrq(OTG_FS_IRQn);
}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  speed : Full, Low 
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev,uint8_t state)
{
  if (state == 1)
    GPIO_ResetBits(GPIOC, GPIO_Pin_0);
  else
    GPIO_SetBits(GPIOC, GPIO_Pin_0);
}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  Speed : Full, Low 
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
  * @brief  USB_OTG_BSP_TimeInit
  *         Initialises delay unit Systick timer /Timer2
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_TimeInit ( void )
{

}

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{

  uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do {
    if ( ++count > utime ) {
      return ;
    }
  }
  while (1); 
  
}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{

  DelayKTask(msec * TICK_PER_SECOND / 1000);   

}


/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_TimerIRQ (void)
{

} 

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
