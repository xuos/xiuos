/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/28     Bernard      The unify RISC-V porting code.
 */

/*************************************************
File name: tick.c
Description: system tick interrupt related routines
Others: take RT-Thread v4.0.2/libcpu/risc-v/k210/tick.c
        https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
*************************************************/

#include <clint.h>
#include <encoding.h>
#include <sysctl.h>
#include <xs_ktick.h>

static volatile unsigned long tick_cycles = 0;

int TickIsr(void)
{
    uint64_t core_id = current_coreid();

    clint->mtimecmp[core_id] += tick_cycles;
    TickAndTaskTimesliceUpdate();

    return 0;
}

int InitHwTick(void)
{
    unsigned long core_id = current_coreid();
    unsigned long interval = 1000 / TICK_PER_SECOND;

    CLEAR_CSR(mie, MIP_MTIP);

#ifdef BSP_USING_QEMU
    tick_cycles = (10000000 / TICK_PER_SECOND);
#else
    tick_cycles = interval * SysctlClockGetFreq(SYSCTL_CLOCK_CPU) / CLINT_CLOCK_DIV / 1000ULL - 1;
#endif
  
    clint->mtimecmp[core_id] = clint->mtime + tick_cycles;

    SET_CSR(mie, MIP_MTIP);

    return 0;
}