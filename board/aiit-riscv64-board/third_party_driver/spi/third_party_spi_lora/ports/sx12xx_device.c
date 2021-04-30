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
 * 2019-02-20     XiaojieFan     the first version
 */

/*************************************************
File name: sx12xx_device.c
Description: support aiit board configure and register function
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. replace original macro and basic date type with AIIT XUOS Lab's own defination
*************************************************/

#include "connect_spi.h"
#include "board.h"
// #include "gpio.h"
#include "stm32f4xx.h"
#include "hardware_gpio.h"


