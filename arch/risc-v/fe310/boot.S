// See LICENSE for license details.

/**
* @file boot.S
* @brief hifive1-rev-B-board start function 
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: boot.S
Description: hifive1-rev-B-board start function 
Others: take freedom-e-sdk v20180402/bsp/env/start.S for references
                https://github.com/sifive/freedom-e-sdk/releases/tag/v20180402
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. See LICENSE details in xiuos/arch/risc-v/fe310/LICENSE
2. Modify entry function name
3. Modify OS startup function
4. Add interrupt entry function.
*************************************************/

#include <sifive/smp.h>

/* This is defined in sifive/platform.h, but that can't be included from
 * assembly. */
#define CLINT_CTRL_ADDR 0x02000000
#include <encoding.h>
#include "register_para.h"
#include "boot.h"

	.section .init
	.globl _begin
	.type _begin,@function

_begin:
	.cfi_startproc
	.cfi_undefined ra
.option push
.option norelax
	la gp, __global_pointer$
.option pop
	la sp, _sp


	/* Load data section */
	la a0, _data_lma
	la a1, _data
	la a2, _edata
	bgeu a1, a2, 2f
1:
	lw t0, (a0)
	sw t0, (a1)
	addi a0, a0, 4
	addi a1, a1, 4
	bltu a1, a2, 1b
2:

	/* Clear bss section */
	la a0, __bss_start
	la a1, _end
	bgeu a0, a1, 2f
1:
	sw zero, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b
2:

	/* Call global constructors */
	//la a0, __libc_fini_array
	//call atexit
	//call __libc_init_array

#ifndef __riscv_float_abi_soft
	/* Enable FPU */
	li t0, MSTATUS_FS
	csrs mstatus, t0
	csrr t1, mstatus
	and t1, t1, t0
	beqz t1, 1f
	fssr x0
1:
#endif

	auipc ra, 0
	addi sp, sp, -16
	sw ra, 8(sp)

	/* argc = argv = 0 */
	li a0, 0
	li a1, 0
	call entry
	/* tail exit */

1:
	j 1b
	.cfi_endproc


  .section      .text.entry
  .align 2
  .global save_hw_context
save_hw_context:

    /* save all from thread context */
    SAVE_X_REGISTERS

    /* save break thread stack to s0 */
    move s0, sp

#ifdef  TASK_ISOLATION
    csrr a0, mcause
    srli a0, a0, 31
    andi a0, a0, 0x1
    beqz a0, 1f
#endif
    /* switch to interrupt stack */
    la   sp, _sp

1:
    /* interrupt handle */
    csrr  a0, mcause
    csrr  a1, mepc
    mv    a2, sp
    call  HandleTrap

    /* switch to from_thread stack */
    move  sp, s0
    mv  a0, fp
    call KTaskOsAssignAfterIrq
    j   SwitchKTaskContextExit