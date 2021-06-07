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

#include <arch_interrupt.h>
#include <board.h>
#include <encoding.h>
#include <platform.h>
#include <plic_driver.h>
#include <xs_assign.h>
#include <xs_base.h>
#include <xs_isr.h>
#include <xs_ktask.h>
#ifdef  TASK_ISOLATION
#include <xs_isolation.h>
#include <xs_service.h>
#endif

#define MAX_HANDLERS    PLIC_NUM_INTERRUPTS
extern plic_instance_t g_plic ;

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


/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
int32_t ArchDisableHwIrq(int irq)
{
    PLIC_disable_interrupt(&g_plic, irq);
    return 0;
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
int32_t ArchEnableHwIrq(int irq)
{
    PLIC_enable_interrupt(&g_plic, irq);
    PLIC_set_priority(&g_plic, irq, 1);
    return 0;
}

uint32_t GetInterruptNumber(void)
{
    return (uint32_t)PLIC_claim_interrupt(&g_plic);
}

void HwInterruptAck(uint32_t irq)
{
    PLIC_complete_interrupt(&g_plic, irq);
}

/**
 * This function will be call when external machine-level 
 * interrupt from PLIC occurred.
 */
uintptr_t HandleIrqMExt(uintptr_t cause, uintptr_t epc)
{
    uint32_t irq;

    /* get irq number */
    irq = GetInterruptNumber();
    /* get interrupt service routine */
    isrManager.done->handleIrq(irq);
    HwInterruptAck(irq);
}

extern int TickIsr(void);

#ifdef TASK_ISOLATION
uintptr_t HandleTrap(uintptr_t mcause, uintptr_t epc, uintptr_t * sp)
{
    int cause = mcause & MCAUSE_CAUSE ;

    if (mcause & MCAUSE_INT) {
        isrManager.done->incCounter();
        switch (cause) {
        case IRQ_M_EXT:
            HandleIrqMExt(mcause, epc);
            break;
        case IRQ_M_TIMER:
            CLINT_MTIMECMP_ADDR = CLINT_MTIME_ADDR + TICK;
            TickIsr();
            break;
        }
        isrManager.done->decCounter();
    }else {
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
            
        } else if( cause == CAUSE_MACHINE_ECALL) {
            unsigned long service_num = (unsigned long)(sp[10]);
            KPrintf("Environment call from M-mode, task:%s, flag: %d,service_num: %d\n \n",tid->task_base_info.name,tid->task_dync_sched_member.isolation_flag, service_num);
            sp[0] += 4;
            ENABLE_INTERRUPT(temp);
        }
        else if (cause == CAUSE_FAULT_LOAD || cause == CAUSE_FAULT_STORE || cause == CAUSE_FAULT_FETCH ){
            if ( tid->task_dync_sched_member.isolation_flag == 1) {
                x_ubase fault_addr = READ_CSR(mtval);
                //  KPrintf("access fault ,addr : 0x%08x\n",fault_addr);
                x_bool result ;
                result = mem_access.FaultHandle(tid->task_dync_sched_member.isolation , fault_addr);
                if(result)
                    mem_access.Load(tid->task_dync_sched_member.isolation);
                else{
                    KPrintf("\nSegmentation fault, task: %s\n",tid->task_base_info.name);
                    KTaskQuit();
                }
            }else
              goto  __print;
        }
        else
        {
            KPrintf("\nException:\n");
            tid = GetKTaskDescriptor();
            switch (cause)
            {
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
__print:
            KPrintf("\n");
            //PrintStackFrame(sp);
            KPrintf("exception pc => 0x%08x\n", epc);
            KPrintf("current thread: %.*s\n", NAME_NUM_MAX, tid->task_base_info.name);
#ifdef TOOL_SHELL
            ShowTask();
#endif
            while(RET_TRUE);
        }
    }
    return epc;
}
#else
uintptr_t HandleTrap(uintptr_t mcause, uintptr_t epc, uintptr_t * sp)
{
    int cause = mcause & MCAUSE_CAUSE ;

    if (mcause & MCAUSE_INT)
    {
        isrManager.done->incCounter();
        switch (cause)
        {
            case IRQ_M_EXT:
                HandleIrqMExt(mcause, epc);
                break;
            case IRQ_M_TIMER:
                CLINT_MTIMECMP_ADDR = CLINT_MTIME_ADDR + TICK;
                TickIsr();
                break;
        }
        isrManager.done->decCounter();
    }
    else
    {

        KTaskDescriptorType tid;
        extern long ShowTask();


        DISABLE_INTERRUPT();

        tid = GetKTaskDescriptor();
        KPrintf("\nException:\n");
        switch (cause)
        {
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
            //PrintStackFrame(sp);
            KPrintf("exception pc => 0x%08x\n", epc);
            KPrintf("current thread: %.*s\n", NAME_NUM_MAX, tid->task_base_info.name);
#ifdef TOOL_SHELL
            ShowTask();
#endif
            while (RET_TRUE);
        }
        return epc;
    }
#endif

