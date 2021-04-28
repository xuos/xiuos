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
* @file:    idle.c
* @brief:   idle file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <xs_hook.h>
#include <xs_spinlock.h>

#if defined (KERNEL_HOOK)
#ifndef KERNEL_IDLE_HOOK
#define KERNEL_IDLE_HOOK
#endif
#endif

#ifndef IDLE_KTASK_STACKSIZE
#define IDLE_KTASK_STACKSIZE  256
#endif

#ifdef ARCH_SMP
#define CORE_NUM CPU_NUMBERS
#else
#define CORE_NUM 1
#endif


static int32 idle[CORE_NUM];
__attribute__((aligned(MEM_ALIGN_SIZE)))

void RunningIntoLowPowerMode()
{
#ifdef ARCH_ARM 
    __asm volatile("WFI");    
#endif

#ifdef ARCH_RISCV 
    asm volatile ("wfi");
#endif
}

static void IdleKTaskEntry(void *arg)
{
    while (1) {
#ifdef KERNEL_IDLE_HOOK
        HOOK(hook.idle.hook_Idle,());
#endif
        RunningIntoLowPowerMode();
    }
}
/**
 *
 * init system idle task,then startup the idle task
 *
 */
void InitIdleKTask(void)
{
    uint8 coreid = 0;
    char ktaskidle[NAME_NUM_MAX] = {0};

    for (coreid = 0; coreid < CORE_NUM; coreid++) {
        sprintf(ktaskidle, "ktaskidle%d", coreid);

        idle[coreid] = KTaskCreate(ktaskidle,IdleKTaskEntry,NONE,IDLE_KTASK_STACKSIZE,KTASK_LOWEST_PRIORITY);

#ifdef ARCH_SMP
        KTaskCoreCombine(idle[coreid], coreid);
#endif
        StartupKTask(idle[coreid]);
    }
}

/**
 *
 * This function will return the idle task descriptor
 *
 */
KTaskDescriptorType GetIdleKTaskDescripter(void)
{
    KTaskDescriptorType idle_p = NONE;

#ifdef ARCH_SMP
    register int id = GetCpuId();
#else
    register int id = 0;
#endif

    idle_p = CONTAINER_OF(&idle[id], struct TaskDescriptor, id);
    return  idle_p;
}
