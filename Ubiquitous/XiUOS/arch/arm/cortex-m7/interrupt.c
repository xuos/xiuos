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
* @file interrupt.c
* @brief support arm cortex-m4 interrupt function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-28
*/

#include <xs_base.h>
#include <xs_isr.h>


x_base __attribute__((naked)) DisableLocalInterrupt()
{
    asm volatile ("MRS     r0, PRIMASK");
    asm volatile ("CPSID   I");
    asm volatile ("BX      LR ");
}

void __attribute__((naked)) EnableLocalInterrupt(x_base level)
{
    asm volatile ("MSR     PRIMASK, r0");
    asm volatile ("BX      LR");
}

int32 ArchEnableHwIrq(uint32 irq_num)
{
    return EOK;
}

int32 ArchDisableHwIrq(uint32 irq_num)
{
    return EOK;
}

extern  void KTaskOsAssignAfterIrq(void *context);

void IsrEntry()
{
    uint32 ipsr;

    __asm__ volatile("MRS %0, IPSR" : "=r"(ipsr));
    isrManager.done->incCounter();
    isrManager.done->handleIrq(ipsr);
    KTaskOsAssignAfterIrq(NONE);
    isrManager.done->decCounter();
    
}

void UsageFault_Handler(int irqn, void *arg)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

void BusFault_Handler(int irqn, void *arg)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

void NMI_Handler(int irqn, void *arg)
{
    while (1)
  {
  }
}
