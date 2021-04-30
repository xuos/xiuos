/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file connect_lcd.h
* @brief define aiit-riscv64-board lcd function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_lcd.h
Description: define aiit-riscv64-board lcd function
Others:  https://canaan-creative.com/developer
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. add aiit-riscv64-board lcd function
*************************************************/

#ifndef CONNECT_LCD_H
#define CONNECT_LCD_H

#include <device.h>
#include "hardware_spi.h"
#include <sysctl.h>

#ifdef BSP_USING_TOUCH
#include "connect_touch.h"
#endif

void DrvLcdClear(uint16_t color);
void LcdDrawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2,uint16 color);							
void LcdDrawRectangle(uint16 x1, uint16 y1, uint16 x2, uint16 y2,uint16 color);		   			
void LcdDrawCircle(uint16 x0,uint16 y0,uint8 r,uint16 color);

int HwLcdInit(void);
#endif
