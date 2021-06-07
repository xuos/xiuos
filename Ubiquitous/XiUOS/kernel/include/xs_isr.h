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
* @file:   xs_isr.h
* @brief: function declaration and structure defintion of isr
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_ISR_H
#define XS_ISR_H

#include <xs_base.h>
#include <arch_interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif


#define DECLARE_HW_IRQ(_irq_num, _handler, _arg) \
    const uint32 __irq_desc_idx_##_handler SECTION(".isrtbl.idx") = _irq_num +   ARCH_IRQ_NUM_OFFSET  ; \
    const struct IrqDesc __irq_desc_##_handler SECTION(".isrtbl") = { \
        .handler = _handler, \
        .param = _arg, \
    }

typedef void (*IsrHandlerType)(int vector, void *param);

struct IrqDesc
{
    IsrHandlerType handler;
    void *param;

#ifdef CONFIG_INTERRUPT_INFO
    char name[NAME_NUM_MAX];
    uint32 counter;
#endif
};

struct IsrDone
{
    x_bool (*isInIsr)();
    int32  (*registerIrq)(uint32 irq_num, IsrHandlerType handler, void *arg);
    int32  (*freeIrq)(uint32 irq_num);
    int32  (*enableIrq)(uint32 irq_num);
    int32  (*disableIrq)(uint32 irq_num);
    void   (*handleIrq)(uint32 irq_num);
    uint16 (*getCounter)() ;
    void   (*incCounter)();
    void   (*decCounter)();
    uint8  (*getSwitchTrigerFlag)();
    void   (*setSwitchTrigerFlag)();
    void   (*clearSwitchTrigerFlag)();
};

struct InterruptServiceRoutines {

#ifdef ARCH_SMP
       volatile uint16 isr_count[CPU_NUMBERS];
       volatile uint8  isr_switch_trigger_flag[CPU_NUMBERS];
#else
       volatile uint16 isr_count ;
       volatile uint8  isr_switch_trigger_flag;
#endif
    struct IrqDesc irq_table[ARCH_MAX_IRQ_NUM];
    struct IsrDone *done;
};

extern struct InterruptServiceRoutines isrManager ;

x_base DisableLocalInterrupt();
void EnableLocalInterrupt(x_base level);

#define DISABLE_INTERRUPT DisableLocalInterrupt
#define ENABLE_INTERRUPT  EnableLocalInterrupt

void SysInitIsrManager();
void InitHwinterrupt(void);


#ifdef __cplusplus
}
#endif

#endif
