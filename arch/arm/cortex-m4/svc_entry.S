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
  
  #include <xsconfig.h>

  .globl		SVC_Entry
  .syntax		unified
  .thumb
  .file		"svc_entry.S"

  .text
  .type	SVC_Entry, %function
  .thumb_func
SVC_Entry:
    MRS r3, PRIMASK
    CPSID   I
    MRS		r0, ipsr
	MRS		r1, psp					/* R1=The process stack pointer (PSP) */

    STMDB	r1!, {r3-r11}		/* Save the remaining registers plus the SP/PRIMASK values */

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    MOV     r2, #0x00
    TST     lr, #0x10
    MOVEQ   r2, #0x01
    STMFD   r1!, {r2}
#endif
    //LDR     sp, =__stack_tp
    PUSH    {lr}
    BL		Svcall				/* R0=IRQ, R1=register save (msp) */
    POP     {lr}
    
    MOV     r1, r0

#ifdef TASK_ISOLATION
    PUSH    {lr}
    BL      GetTaskPrivilege
    POP     {lr}
#endif

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    LDMFD   r1!, {r2}
#endif

    LDMIA	r1!, {r3-r11}		/* Recover R4-R11, FPU flag and  PRIMASK*/

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    ORR     lr, lr, #0x10
    CMP     r2,  #0
    BICNE   lr, lr, #0x10
#endif
    MSR     psp,  r1
    MRS		r2, control

#ifdef TASK_ISOLATION
    CMP     r0,  #1
    BEQ     unprivilege

privilege:
    BIC		r2, r2, #0x01
    B       exit

unprivilege:
    ORR		r2, r2, #0x01
#else
    BIC		r2, r2, #0x01
#endif

exit:
    MSR	control,  r2
    ORR lr, lr, #0x04
    MSR PRIMASK,  r3
    BX  lr