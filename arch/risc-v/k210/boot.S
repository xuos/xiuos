/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/01     Bernard      The first version
 * 2018/12/27     Jesven       Add SMP support
 */

/*************************************************
File name: boot.S
Description: K210 boot code
Others: take RT-Thread v4.0.2/libcpu/risc-v/k210/startup_gcc.S
        https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
*************************************************/

#include "boot.h"

#define MSTATUS_FS      0x00006000U 
#define __STACKSIZE__ 4096
#define K210_CPU0_STACKTOP (__stack_tp0)
#define K210_CPU1_STACKTOP (__stack_tp1)

  .globl _begin
  .section ".start", "ax"
_begin:
  j 1f
  .word 0xdeadbeef
  .align 3
  .globl g_wake_up
  g_wake_up:
      .dword 1
      .dword 0
1:
  csrw mideleg, 0
  csrw medeleg, 0
  csrw mie, 0
  csrw mip, 0
  la t0, save_hw_context
  csrw mtvec, t0

  ZERO_X_REGISTERS

  li t0, MSTATUS_FS
  csrs mstatus, t0

#ifndef BSP_USING_QEMU
  ZERO_F_REGISTERS
#endif

.option push
.option norelax
  la gp, __global_pointer$
.option pop

  csrr a0, mhartid
  
  bnez a0, 1f
  la   sp, K210_CPU0_STACKTOP
  j    2f
1:
  la   sp, K210_CPU1_STACKTOP
2:

  j    Kd233Start


#ifdef ARCH_SMP
.data
.globl cpu2_boot_flag
.align 3
cpu2_boot_flag:
    .8byte 0
#endif

.section      .text.entry
.align 2
.globl save_hw_context
save_hw_context:

    SAVE_X_REGISTERS

    mv  fp, sp

    csrr  t0, mhartid

#ifdef  TASK_ISOLATION
    csrr a0, mcause
    srli a0, a0, 63
    andi a0, a0, 0x1
    beqz a0, 1f
#endif
    la    sp, __stack_start__
    addi  t1, t0, 1
    li    t2, __STACKSIZE__
    mul   t1, t1, t2
    add   sp, sp, t1 /* sp = (cpuid + 1) * __STACKSIZE__ + __stack_start__ */

1:
    csrr  a0, mcause
    csrr  a1, mepc
    mv    a2, fp
    call  HandleTrap

    mv  sp, fp
    mv  a0, fp
    call KTaskOsAssignAfterIrq
    j   SwitchKTaskContextExit