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

#include <xs_base.h>
#include <xs_ktask.h>
#include <xs_assign.h>
#include "svc_handle.h"
#include "stm32f4xx.h"
#include <board.h>

#if (defined ( __GNUC__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__))
#define USE_FPU   1
#else
#define USE_FPU   0
#endif

uint32 InterruptFromKtask;
uint32 InterruptToKtask;
uint32 KtaskSwitchInterruptFlag;
uint32 InterruptToKtaskDescriptor;
#define RunningKTask  Assign.os_running_task

static x_err_t (*ExceptionHook)(void *context) = NONE;

struct ExceptionStackRegister
{
    uint32 r0;
    uint32 r1;
    uint32 r2;
    uint32 r3;
    uint32 r12;
    uint32 lr;
    uint32 pc;
    uint32 psr;
};

struct StackRegisterContent
{
#if defined ( __VFP_FP__ ) && !defined(__SOFTFP__)
    uint32 flag;
#endif
    uint32 primask;
    uint32 r4;
    uint32 r5;
    uint32 r6;
    uint32 r7;
    uint32 r8;
    uint32 r9;
    uint32 r10;
    uint32 r11;
    // uint32 exc_ret;

    struct ExceptionStackRegister ExErrorStackContex;
};

struct ExceptionStackFrameFpu
{
    uint32 r0;
    uint32 r1;
    uint32 r2;
    uint32 r3;
    uint32 r12;
    uint32 lr;
    uint32 pc;
    uint32 psr;

#if USE_FPU
    uint32 S0;
    uint32 S1;
    uint32 S2;
    uint32 S3;
    uint32 S4;
    uint32 S5;
    uint32 S6;
    uint32 S7;
    uint32 S8;
    uint32 S9;
    uint32 S10;
    uint32 S11;
    uint32 S12;
    uint32 S13;
    uint32 S14;
    uint32 S15;
    uint32 FPSCR;
    uint32 NO_NAME;
#endif
};

struct StackFrameFpu
{
    uint32 flag;

    uint32 r4;
    uint32 r5;
    uint32 r6;
    uint32 r7;
    uint32 r8;
    uint32 r9;
    uint32 r10;
    uint32 r11;

#if USE_FPU
    uint32 s16;
    uint32 s17;
    uint32 s18;
    uint32 s19;
    uint32 s20;
    uint32 s21;
    uint32 s22;
    uint32 s23;
    uint32 s24;
    uint32 s25;
    uint32 s26;
    uint32 s27;
    uint32 s28;
    uint32 s29;
    uint32 s30;
    uint32 s31;
#endif

    struct ExceptionStackFrameFpu ExErrorStackContex;
};

uint8 KTaskStackSetup(struct TaskDescriptor *task)
{
    struct StackRegisterContent* StackContex;
    int i = 0;

    task->stack_point  = (uint8 *)ALIGN_MEN_DOWN((x_ubase)(task->task_base_info.stack_start + task->task_base_info.stack_depth), 8);
    task->stack_point -= sizeof(struct StackRegisterContent);

    StackContex = (struct StackRegisterContent*)task->stack_point;

    for (i = 0; i < sizeof(struct StackRegisterContent) / sizeof(uint32); i++)
        ((uint32 *)StackContex)[i] = 0xfadeface;

    StackContex->ExErrorStackContex.r0  = (unsigned long)task->task_base_info.func_param;

    StackContex->ExErrorStackContex.pc  = (unsigned long)task->task_base_info.func_entry ;
    StackContex->ExErrorStackContex.psr = 0x01000000L;
    StackContex->primask = 0x00000000L;
#ifdef SEPARATE_COMPILE
    if(task->task_dync_sched_member.isolation_flag == 1 ) {
        //StackContex->exc_ret = EXC_RETURN_UNPRIVTHR;
        StackContex->ExErrorStackContex.lr  = (unsigned long)USERSPACE->us_taskquit;
    } else {
        //StackContex->exc_ret = EXC_RETURN_PRIVTHR;
        StackContex->ExErrorStackContex.lr  = (unsigned long)KTaskQuit; 
    }
#else
    //StackContex->exc_ret = EXC_RETURN_PRIVTHR;
    StackContex->ExErrorStackContex.lr  = (unsigned long)KTaskQuit; 
#endif

#if USE_FPU
    StackContex->flag = 0;
#endif

    return EOK;
}


void HwExceptionInstall(x_err_t (*exception_handle)(void *context))
{
    ExceptionHook = exception_handle;
}

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28)
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C)
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34)
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38)
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)
#define SCB_RESET_VALUE 0x05FA0004

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A)

#ifdef TOOL_SHELL
static void UsageFaultTrack(void)
{
    KPrintf("usage fault:\n");
    KPrintf("SCB_CFSR_UFSR:0x%02X ", SCB_CFSR_UFSR);

    if(SCB_CFSR_UFSR & (1<<0))
        KPrintf("UNDEFINSTR ");

    if(SCB_CFSR_UFSR & (1<<1))
        KPrintf("INVSTATE ");

    if(SCB_CFSR_UFSR & (1<<2))
        KPrintf("INVPC ");

    if(SCB_CFSR_UFSR & (1<<3))
        KPrintf("NOCP ");

    if(SCB_CFSR_UFSR & (1<<8))
        KPrintf("UNALIGNED ");

    if(SCB_CFSR_UFSR & (1<<9))
        KPrintf("DIVBYZERO ");

    KPrintf("\n");
}

static void BusFaultTrack(void)
{
    KPrintf("bus fault:\n");
    KPrintf("SCB_CFSR_BFSR:0x%02X ", SCB_CFSR_BFSR);

    if(SCB_CFSR_BFSR & (1<<0))
        KPrintf("IBUSERR ");

    if(SCB_CFSR_BFSR & (1<<1))
        KPrintf("PRECISERR ");

    if(SCB_CFSR_BFSR & (1<<2))
        KPrintf("IMPRECISERR ");

    if(SCB_CFSR_BFSR & (1<<3))
        KPrintf("UNSTKERR ");

    if(SCB_CFSR_BFSR & (1<<4))
        KPrintf("STKERR ");

    if(SCB_CFSR_BFSR & (1<<7))
        KPrintf("SCB->BFAR:%08X\n", SCB_BFAR);
    else
        KPrintf("\n");
}

static void MemManageFaultTrack(void)
{
    KPrintf("mem manage fault:\n");
    KPrintf("SCB_CFSR_MFSR:0x%02X ", SCB_CFSR_MFSR);

    if(SCB_CFSR_MFSR & (1<<0))
        KPrintf("IACCVIOL ");
    if(SCB_CFSR_MFSR & (1<<1))
        KPrintf("DACCVIOL ");

    if(SCB_CFSR_MFSR & (1<<3))
        KPrintf("MUNSTKERR ");

    if(SCB_CFSR_MFSR & (1<<4))
        KPrintf("MSTKERR ");

    if(SCB_CFSR_MFSR & (1<<7))
        KPrintf("SCB->MMAR:%08X\n", SCB_MMAR);
    else
        KPrintf("\n");
}

static void HardFaultTrack(void)
{
    if(SCB_HFSR & (1UL<<1))
        KPrintf("failed vector fetch\n");

    if(SCB_HFSR & (1UL<<30)) {
        if(SCB_CFSR_BFSR)
            BusFaultTrack();

        if(SCB_CFSR_MFSR)
            MemManageFaultTrack();

        if(SCB_CFSR_UFSR)
            UsageFaultTrack();
    }

    if(SCB_HFSR & (1UL<<31))
        KPrintf("debug event\n");
}
#endif

struct ExceptionInfo
{
    uint32 ExcReturn;
    struct StackRegisterContent stackframe;
};

void HwHardFaultException(struct ExceptionInfo *ExceptionInfo)
{
    extern long ShowTask(void);
    struct ExErrorStackContex* ExceptionStack = &ExceptionInfo->stackframe.ExErrorStackContex;
    struct StackRegisterContent* context = &ExceptionInfo->stackframe;

    if (ExceptionHook != NONE) {
        x_err_t result = ExceptionHook(ExceptionStack);
        if (result == EOK) return;
    }

    KPrintf("psr: 0x%08x\n", context->ExErrorStackContex.psr);
    KPrintf("r00: 0x%08x\n", context->ExErrorStackContex.r0);
    KPrintf("r01: 0x%08x\n", context->ExErrorStackContex.r1);
    KPrintf("r02: 0x%08x\n", context->ExErrorStackContex.r2);
    KPrintf("r03: 0x%08x\n", context->ExErrorStackContex.r3);
    KPrintf("r04: 0x%08x\n", context->r4);
    KPrintf("r05: 0x%08x\n", context->r5);
    KPrintf("r06: 0x%08x\n", context->r6);
    KPrintf("r07: 0x%08x\n", context->r7);
    KPrintf("r08: 0x%08x\n", context->r8);
    KPrintf("r09: 0x%08x\n", context->r9);
    KPrintf("r10: 0x%08x\n", context->r10);
    KPrintf("r11: 0x%08x\n", context->r11);
    //KPrintf("exc_ret: 0x%08x\n", context->exc_ret);
    KPrintf("r12: 0x%08x\n", context->ExErrorStackContex.r12);
    KPrintf(" lr: 0x%08x\n", context->ExErrorStackContex.lr);
    KPrintf(" pc: 0x%08x\n", context->ExErrorStackContex.pc);

    if (ExceptionInfo->ExcReturn & (1 << 2)) {
        KPrintf("hard fault on task: %s\r\n\r\n", GetKTaskDescriptor()->task_base_info.name);
#ifdef TOOL_SHELL
        ShowTask();
#endif
    } else {
        KPrintf("hard fault on handler\r\n\r\n");
    }

    if ( (ExceptionInfo->ExcReturn & 0x10) == 0)
        KPrintf("FPU active!\r\n");

#ifdef TOOL_SHELL
    HardFaultTrack();
#endif

    while (1);
}

void UpdateRunningTask(void)
{
    RunningKTask = (struct TaskDescriptor *)InterruptToKtaskDescriptor;
}


void MemFaultExceptionPrint(struct ExceptionInfo *ExceptionInfo)
{
    extern long ShowTask(void);
    struct ExErrorStackContex* ExceptionStack = &ExceptionInfo->stackframe.ExErrorStackContex;
    struct StackRegisterContent* context = &ExceptionInfo->stackframe;

    if (ExceptionHook != NONE) {
        x_err_t result = ExceptionHook(ExceptionStack);
        if (result == EOK) return;
    }

    KPrintf("psr: 0x%08x\n", context->ExErrorStackContex.psr);
    KPrintf("r00: 0x%08x\n", context->ExErrorStackContex.r0);
    KPrintf("r01: 0x%08x\n", context->ExErrorStackContex.r1);
    KPrintf("r02: 0x%08x\n", context->ExErrorStackContex.r2);
    KPrintf("r03: 0x%08x\n", context->ExErrorStackContex.r3);
    KPrintf("r04: 0x%08x\n", context->r4);
    KPrintf("r05: 0x%08x\n", context->r5);
    KPrintf("r06: 0x%08x\n", context->r6);
    KPrintf("r07: 0x%08x\n", context->r7);
    KPrintf("r08: 0x%08x\n", context->r8);
    KPrintf("r09: 0x%08x\n", context->r9);
    KPrintf("r10: 0x%08x\n", context->r10);
    KPrintf("r11: 0x%08x\n", context->r11);
    KPrintf("exc_ret: 0x%08x\n", ExceptionInfo->ExcReturn);
    KPrintf("r12: 0x%08x\n", context->ExErrorStackContex.r12);
    KPrintf(" lr: 0x%08x\n", context->ExErrorStackContex.lr);
    KPrintf(" pc: 0x%08x\n", context->ExErrorStackContex.pc);

    if (ExceptionInfo->ExcReturn & (1 << 2)) {
        KPrintf("hard fault on task: %s\r\n\r\n", GetKTaskDescriptor()->task_base_info.name);
#ifdef TOOL_SHELL
        ShowTask();
#endif
    } else {
        KPrintf("hard fault on handler\r\n\r\n");
    }

    if ((ExceptionInfo->ExcReturn & 0x10) == 0)
        KPrintf("FPU active!\r\n");

    KPrintf("CFSR: 0x%08x \n", (*((volatile unsigned long *)(SCB->CFSR))) );
    KPrintf("HFSR: 0x%08x \n",  (*((volatile unsigned long *)(SCB->HFSR))) );
    KPrintf("DFSR: 0x%08x \n",(*((volatile unsigned long *)(SCB->DFSR))) );
    KPrintf("MMFAR: 0x%08x \n",(*((volatile unsigned long *)(SCB->MMFAR))));
    KPrintf("BFAR: 0x%08x \n",(*((volatile unsigned long *)(SCB->BFAR))));
    KPrintf("AFSR: 0x%08x \n",(*((volatile unsigned long *)(SCB->AFSR))));

#ifdef TOOL_SHELL
    HardFaultTrack();
#endif

    while (1);
}

void MemFaultHandle(uintptr_t *sp)
{
#ifdef TASK_ISOLATION
   struct TaskDescriptor *task;
   task = GetKTaskDescriptor();
   if( task->task_dync_sched_member.isolation_flag == 1){
       KPrintf("\nSegmentation fault, task: %s\n", task->task_base_info.name);
       KTaskQuit();
   }
   else
#endif
   {
       MemFaultExceptionPrint((struct ExceptionInfo *)sp);
   }
      
}

void ShutdownCpu(void)
{
    KPrintf("shutdown...\n");
    CHECK(0);
}

__attribute__((weak)) void HwCpuReset(void)
{
    SCB_AIRCR = SCB_RESET_VALUE;
}
