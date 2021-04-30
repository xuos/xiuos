/*
 * Original Copyright (c) 2006-2018, RT-Thread Development Team
 * Modified Copyright (c) 2020 AIIT XUOS Lab
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Change Logs:
 * Date           Author         Notes
 * 2019-02-25     XiaojieFan     the first version
 */

/* 
 * Modified by:     AIIT XUOS Lab
 * Modified date:   2020-09-01
 * Description:     
 */
/*************************************************
File name: spi_lora_sx12xx.h
Description: support aiit board configure and register function
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. replace original macro and basic date type with AIIT XUOS Lab's own defination
*************************************************/


#ifndef __SPI_LORA_SX12XX_H_
#define __SPI_LORA_SX12XX_H_

#include <xiuos.h>
#include <stdbool.h>

/*!
 * \brief Gets the SX1272 DIO0 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio0(void);

/*!
 * \brief Gets the SX1272 DIO1 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio1(void);

/*!
 * \brief Gets the SX1272 DIO2 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio2(void);

/*!
 * \brief Gets the SX1272 DIO3 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio3(void);

/*!
 * \brief Gets the SX1272 DIO4 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio4(void);

/*!
 * \brief Gets the SX1272 DIO5 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8_t SX1276ReadDio5(void);

void SX1276Write(uint8_t addr, uint8_t data);
void SX1276Read(uint8_t addr, uint8_t *data);
void SX1276WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);
void SX1276ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);
void SX1276WriteFifo(uint8_t *buffer, uint8_t size);
void SX1276ReadFifo(uint8_t *buffer, uint8_t size);
void SX1276SetReset(uint8_t state);
uint8_t SX1276_Spi_Check(void);
void SX1276WriteRxTx(uint8_t txEnable);
#endif