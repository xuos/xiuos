/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/12/23     Bernard      The first version
 * 2018/12/27     Jesven       Add secondary cpu boot
 */

/*************************************************
File name: smp_support.c
Description: SMP support routines
Others: take RT-Thread v4.0.2/libcpu/risc-v/k210/cpuport_smp.c
        https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
*************************************************/

#include "board.h"
#include <atomic.h>
#include <clint.h>
#include <encoding.h>
#include <xs_spinlock.h>
#include <stdint.h>
#include <xs_assign.h>



void HwSendIpi(int ipi_vector, unsigned int cpu_mask)
{
    int idx;

    for (idx = 0; idx < CPU_NUMBERS; idx ++)
        if (cpu_mask & (1 << idx))
            clint_ipi_send(idx);
}

int GetCpuId(void)
{
    return READ_CSR(mhartid);
}

void InitHwSpinlock(HwSpinlock *lock)
{
    ((spinlock_t *)lock)->lock = 0;
}

void HwLockSpinlock(HwSpinlock *lock)
{
    spinlock_lock((spinlock_t *)lock);
}

void HwUnlockSpinlock(HwSpinlock *lock)
{
    spinlock_unlock((spinlock_t *)lock);
}

extern uint64 cpu2_boot_flag;
void StartupSecondaryCpu(void)
{
    mb();
    cpu2_boot_flag = 0x2018050420191010;
}

extern void InitHwScondaryInterrupt(void);
extern int InitHwTick(void);
extern int EnableHwclintIpi(void);

void SecondaryCpuCStart(void)
{
    HwLockSpinlock(&AssignSpinLock);
    InitHwScondaryInterrupt();

    InitHwTick();

    EnableHwclintIpi();

    StartupOsAssign();
}

