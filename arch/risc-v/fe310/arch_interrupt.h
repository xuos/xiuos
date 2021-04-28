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

#ifndef ARCH_INTERRUPT_H__
#define ARCH_INTERRUPT_H__

#include <stdint.h>
#include <plic_driver.h>

#define   ARCH_MAX_IRQ_NUM   IRQN_MAX
#define   ARCH_IRQ_NUM_OFFSET   0

uint32_t GetInterruptNumber(void);
void HwInterruptAck(uint32_t irq);

int32_t ArchEnableHwIrq(int irq_num);
int32_t ArchDisableHwIrq(int irq_num);

#endif
