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

#include "tick.h"
#include <clint.h>
#include <interrupt.h>
#include <plic.h>
#include <xs_assign.h>
#include <xs_base.h>
#include <xs_isr.h>
#include <xs_ktask.h>
#ifdef  TASK_ISOLATION
#include <xs_service.h>
#endif

#define CPU_NUM         2

x_base DisableLocalInterrupt()
{
    x_base level;

    asm volatile ("csrrci %0, mstatus, 8" : "=r"(level));

    return level;
}

void EnableLocalInterrupt(x_base level)
{
    asm volatile ("csrw mstatus, %0" :: "r"(level));
}

int EnableHwclintIpi(void)
{
    SET_CSR(mie, MIP_MSIP);
    return 0;
}

int DisableHwclintIpi(void)
{
    CLEAR_CSR(mie, MIP_MSIP);
    return 0;
}

int EnableHwplicIrq(plic_irq_t irq_number)
{
    unsigned long core_id = 0;

    if (PLIC_NUM_SOURCES < irq_number || 0 > irq_number)
        return -1;
    uint32_t current = plic->target_enables.target[core_id].enable[irq_number / 32];
    current |= (uint32_t)1 << (irq_number % 32);
    plic->target_enables.target[core_id].enable[irq_number / 32] = current;
    return 0;
}

int DisableHwplicIrq(plic_irq_t irq_number)
{
    unsigned long core_id = 0;

    if (PLIC_NUM_SOURCES < irq_number || 0 > irq_number)
        return -1;
    uint32_t current = plic->target_enables.target[core_id].enable[irq_number / 32];
    current &= ~((uint32_t)1 << (irq_number % 32));
    plic->target_enables.target[core_id].enable[irq_number / 32] = current;
    return 0;
}

void InitHwinterrupt(void)
{
    int idx;
    int cpuid;

    cpuid = current_coreid();

    for (idx = 0; idx < ((PLIC_NUM_SOURCES + 32u) / 32u); idx ++)
        plic->target_enables.target[cpuid].enable[idx] = 0;

    for (idx = 0; idx < PLIC_NUM_SOURCES; idx++)
        plic->source_priorities.priority[idx] = 0;

    plic->targets.target[cpuid].priority_threshold = 0;

    SET_CSR(mie, MIP_MEIP);
}

void InitHwScondaryInterrupt(void)
{
    int idx;
    int cpuid;

    cpuid = current_coreid();

    for (idx = 0; idx < ((PLIC_NUM_SOURCES + 32u) / 32u); idx ++)
        plic->target_enables.target[cpuid].enable[idx] = 0;

    plic->targets.target[cpuid].priority_threshold = 0;

    SET_CSR(mie, MIP_MEIP);
}

int32 ArchEnableHwIrq(uint32 irq_num)
{
    plic_set_priority(irq_num, 1);
    EnableHwplicIrq(irq_num);
}

int32 ArchDisableHwIrq(uint32 irq_num)
{
    DisableHwplicIrq(irq_num);
}

__attribute__((weak))
void PlicIrqHandle(plic_irq_t irq)
{
    KPrintf("UN-handled interrupt %d occurred!!!\n", irq);
    return ;
}

uintptr_t HandleIrqMExt(uintptr_t cause, uintptr_t epc)
{
    if (READ_CSR(mip) & MIP_MEIP) {
        uint64_t core_id = current_coreid();
        uint64_t ie_flag = READ_CSR(mie);
        uint32_t int_num = plic->targets.target[core_id].claim_complete;
        uint32_t int_threshold = plic->targets.target[core_id].priority_threshold;
        plic->targets.target[core_id].priority_threshold = plic->source_priorities.priority[int_num];

        CLEAR_CSR(mie, MIP_MTIP | MIP_MSIP);

        isrManager.done->handleIrq(int_num);

        plic->targets.target[core_id].claim_complete = int_num;
        SET_CSR(mstatus, MSTATUS_MPIE | MSTATUS_MPP);
        WRITE_CSR(mie, ie_flag);
        plic->targets.target[core_id].priority_threshold = int_threshold;
    }

    return epc;
}
struct ExceptionStackFrame
{
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t x30;
    uint64_t x31;
};

void PrintStackFrame(uintptr_t * sp)
{
    struct ExceptionStackFrame * esf = (struct ExceptionStackFrame *)(sp+1);

    KPrintf("\n=================================================================\n");
    KPrintf("x1 (ra   : Return address                ) ==> 0x%08x%08x\n", esf->x1 >> 32  , esf->x1 & UINT32_MAX);
    KPrintf("x2 (sp   : Stack pointer                 ) ==> 0x%08x%08x\n", esf->x2 >> 32  , esf->x2 & UINT32_MAX);
    KPrintf("x3 (gp   : Global pointer                ) ==> 0x%08x%08x\n", esf->x3 >> 32  , esf->x3 & UINT32_MAX);
    KPrintf("x4 (tp   : Task pointer                  ) ==> 0x%08x%08x\n", esf->x4 >> 32  , esf->x4 & UINT32_MAX);
    KPrintf("x5 (t0   : Temporary                     ) ==> 0x%08x%08x\n", esf->x5 >> 32  , esf->x5 & UINT32_MAX);
    KPrintf("x6 (t1   : Temporary                     ) ==> 0x%08x%08x\n", esf->x6 >> 32  , esf->x6 & UINT32_MAX);
    KPrintf("x7 (t2   : Temporary                     ) ==> 0x%08x%08x\n", esf->x7 >> 32  , esf->x7 & UINT32_MAX);
    KPrintf("x8 (s0/fp: Save register,frame pointer   ) ==> 0x%08x%08x\n", esf->x8 >> 32  , esf->x8 & UINT32_MAX);
    KPrintf("x9 (s1   : Save register                 ) ==> 0x%08x%08x\n", esf->x9 >> 32  , esf->x9 & UINT32_MAX);
    KPrintf("x10(a0   : Function argument,return value) ==> 0x%08x%08x\n", esf->x10 >> 32 , esf->x10 & UINT32_MAX);
    KPrintf("x11(a1   : Function argument,return value) ==> 0x%08x%08x\n", esf->x11 >> 32 , esf->x11 & UINT32_MAX);
    KPrintf("x12(a2   : Function argument             ) ==> 0x%08x%08x\n", esf->x12 >> 32 , esf->x12 & UINT32_MAX);
    KPrintf("x13(a3   : Function argument             ) ==> 0x%08x%08x\n", esf->x13 >> 32 , esf->x13 & UINT32_MAX);
    KPrintf("x14(a4   : Function argument             ) ==> 0x%08x%08x\n", esf->x14 >> 32 , esf->x14 & UINT32_MAX);
    KPrintf("x15(a5   : Function argument             ) ==> 0x%08x%08x\n", esf->x15 >> 32 , esf->x15 & UINT32_MAX);
    KPrintf("x16(a6   : Function argument             ) ==> 0x%08x%08x\n", esf->x16 >> 32 , esf->x16 & UINT32_MAX);
    KPrintf("x17(a7   : Function argument             ) ==> 0x%08x%08x\n", esf->x17 >> 32 , esf->x17 & UINT32_MAX);
    KPrintf("x18(s2   : Save register                 ) ==> 0x%08x%08x\n", esf->x18 >> 32 , esf->x18 & UINT32_MAX);
    KPrintf("x19(s3   : Save register                 ) ==> 0x%08x%08x\n", esf->x19 >> 32 , esf->x19 & UINT32_MAX);
    KPrintf("x20(s4   : Save register                 ) ==> 0x%08x%08x\n", esf->x20 >> 32 , esf->x20 & UINT32_MAX);
    KPrintf("x21(s5   : Save register                 ) ==> 0x%08x%08x\n", esf->x21 >> 32 , esf->x21 & UINT32_MAX);
    KPrintf("x22(s6   : Save register                 ) ==> 0x%08x%08x\n", esf->x22 >> 32 , esf->x22 & UINT32_MAX);
    KPrintf("x23(s7   : Save register                 ) ==> 0x%08x%08x\n", esf->x23 >> 32 , esf->x23 & UINT32_MAX);
    KPrintf("x24(s8   : Save register                 ) ==> 0x%08x%08x\n", esf->x24 >> 32 , esf->x24 & UINT32_MAX);
    KPrintf("x25(s9   : Save register                 ) ==> 0x%08x%08x\n", esf->x25 >> 32 , esf->x25 & UINT32_MAX);
    KPrintf("x26(s10  : Save register                 ) ==> 0x%08x%08x\n", esf->x26 >> 32 , esf->x26 & UINT32_MAX);
    KPrintf("x27(s11  : Save register                 ) ==> 0x%08x%08x\n", esf->x27 >> 32 , esf->x27 & UINT32_MAX);
    KPrintf("x28(t3   : Temporary                     ) ==> 0x%08x%08x\n", esf->x28 >> 32 , esf->x28 & UINT32_MAX);
    KPrintf("x29(t4   : Temporary                     ) ==> 0x%08x%08x\n", esf->x29 >> 32 , esf->x29 & UINT32_MAX);
    KPrintf("x30(t5   : Temporary                     ) ==> 0x%08x%08x\n", esf->x30 >> 32 , esf->x30 & UINT32_MAX);
    KPrintf("x31(t6   : Temporary                     ) ==> 0x%08x%08x\n", esf->x31 >> 32 , esf->x31 & UINT32_MAX);
    KPrintf("=================================================================\n");
}

#ifdef  TASK_ISOLATION
uintptr_t HandleTrap(uintptr_t mcause, uintptr_t epc, uintptr_t * sp)
{
    int cause = mcause & CAUSE_MACHINE_IRQ_REASON_MASK;

    if (mcause & (1UL << 63)) {
        isrManager.done->incCounter();
        switch (cause) {
        case IRQ_M_SOFT: {
                uint64_t core_id = current_coreid();
                clint_ipi_clear(core_id);
                
                DO_KTASK_ASSIGN;
            }
            break;
        case IRQ_M_EXT:
            HandleIrqMExt(mcause, epc);
            break;
        case IRQ_M_TIMER:
            TickIsr();
            break;
        }
        isrManager.done->decCounter();
    } else {
        x_base temp;
        temp = DISABLE_INTERRUPT();
        KTaskDescriptorType tid;
        extern long ShowTask();
        tid = GetKTaskDescriptor();
        
        if(cause == CAUSE_USER_ECALL) {
            tid->task_dync_sched_member.isolation_status = 1;
            sp[0] += 4;
            unsigned long service_num = (unsigned long)(sp[10]);
            //KPrintf("Environment call from U-mode,service_num: %ld\n",service_num);
            uint8_t param_num =  g_service_table[service_num].param_num;
            uintptr_t *param = sp + 11;
            ENABLE_INTERRUPT(temp);
            sp[10] =  g_service_table[service_num].fun(service_num,param,param_num) ;
            tid->task_dync_sched_member.isolation_status = 0;
            
        } else if (cause == CAUSE_MACHINE_ECALL) {
            unsigned long service_num = (unsigned long)(sp[10]);
            KPrintf("Environment call from M-mode, task:%s, flag: %d,service_num: %d\n \n",tid->task_base_info.name,tid->task_dync_sched_member.isolation_flag, service_num);
            sp[0] += 4;
            ENABLE_INTERRUPT(temp);
        } else {
            KPrintf("\nException:\n");
            tid = GetKTaskDescriptor();
            switch (cause) {
            case CAUSE_MISALIGNED_FETCH:
                KPrintf("Instruction address misaligned");
                break;
            case CAUSE_FAULT_FETCH:
                KPrintf("Instruction access fault");
                break;
            case CAUSE_ILLEGAL_INSTRUCTION:
                KPrintf("Illegal instruction");
                break;
            case CAUSE_BREAKPOINT:
                KPrintf("Breakpoint");
                break;
            case CAUSE_MISALIGNED_LOAD:
                KPrintf("Load address misaligned");
                break;
            case CAUSE_FAULT_LOAD:
                KPrintf("Load access fault");
                break;
            case CAUSE_MISALIGNED_STORE:
                KPrintf("Store address misaligned");
                break;
            case CAUSE_FAULT_STORE:
                KPrintf("Store access fault");
                break;
            case CAUSE_SUPERVISOR_ECALL:
                KPrintf("Environment call from S-mode");
                break;
            case CAUSE_HYPERVISOR_ECALL:
                KPrintf("Environment call from H-mode");
                break;
            default:
                KPrintf("Uknown exception : %08lX", cause);
                break;
            }
            KPrintf("\n");
            PrintStackFrame(sp);
            KPrintf("exception pc => 0x%08x\n", epc);
            KPrintf("current thread: %.*s\n", NAME_NUM_MAX, tid->task_base_info.name);
#ifdef TOOL_SHELL
            ShowTask();
#endif
            while (RET_TRUE);
        }
        
    }
    return epc;
}
#else
uintptr_t HandleTrap(uintptr_t mcause, uintptr_t epc, uintptr_t * sp)
{
    int cause = mcause & CAUSE_MACHINE_IRQ_REASON_MASK;
    if (mcause & (1UL << 63)) {
        isrManager.done->incCounter();
        switch (cause) {
        case IRQ_M_SOFT: {
                uint64_t core_id = current_coreid();
                clint_ipi_clear(core_id);
                DO_KTASK_ASSIGN;
            }
            break;
        case IRQ_M_EXT:
            HandleIrqMExt(mcause, epc);
            break;
        case IRQ_M_TIMER:
            TickIsr();
            break;
        }
        isrManager.done->decCounter();
    } else {
        KTaskDescriptorType tid;
        extern long ShowTask();

        DISABLE_INTERRUPT();
        tid = GetKTaskDescriptor();
        KPrintf("\nException:\n");
        switch (cause) {
        case CAUSE_MISALIGNED_FETCH:
            KPrintf("Instruction address misaligned");
            break;
        case CAUSE_FAULT_FETCH:
            KPrintf("Instruction access fault");
            break;
        case CAUSE_ILLEGAL_INSTRUCTION:
            KPrintf("Illegal instruction");
            break;
        case CAUSE_BREAKPOINT:
            KPrintf("Breakpoint");
            break;
        case CAUSE_MISALIGNED_LOAD:
            KPrintf("Load address misaligned");
            break;
        case CAUSE_FAULT_LOAD:
            KPrintf("Load access fault");
            break;
        case CAUSE_MISALIGNED_STORE:
            KPrintf("Store address misaligned");
            break;
        case CAUSE_FAULT_STORE:
            KPrintf("Store access fault");
            break;
        case CAUSE_USER_ECALL:
            KPrintf("Environment call from U-mode");
            break;
        case CAUSE_SUPERVISOR_ECALL:
            KPrintf("Environment call from S-mode");
            break;
        case CAUSE_HYPERVISOR_ECALL:
            KPrintf("Environment call from H-mode");
            break;
        case CAUSE_MACHINE_ECALL:
            KPrintf("Environment call from M-mode");
            break;
        default:
            KPrintf("Uknown exception : %08lX", cause);
            break;
        }
        KPrintf("\n");
        PrintStackFrame(sp);
        KPrintf("exception pc => 0x%08x\n", epc);
        KPrintf("current task: %.*s\n", NAME_NUM_MAX, tid->task_base_info.name);
#ifdef TOOL_SHELL
        ShowTask();
#endif
        while (RET_TRUE);
    }
    return epc;
}
#endif