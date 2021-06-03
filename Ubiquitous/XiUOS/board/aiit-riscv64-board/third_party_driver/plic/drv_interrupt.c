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
* @file drv_interrupt.c
* @brief add from Canaan k210 SDK
*                https://canaan-creative.com/developer
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <plic.h>

void PlicIrqHandle(plic_irq_t irq)
{
    plic_instance_t (*plic_instance)[IRQN_MAX] = plic_get_instance();
    if (plic_instance[0][irq].callback)
    {
        plic_instance[0][irq].callback(
            plic_instance[0][irq].ctx);
    }
    else if (plic_instance[1][irq].callback)
    {
        plic_instance[1][irq].callback(
            plic_instance[1][irq].ctx);
    }
}

