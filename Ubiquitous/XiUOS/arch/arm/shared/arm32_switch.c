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

#define SCB_VTOR "0xE000ED08"
#define NVIC_INT_CTRL "0xE000ED04"
#define NVIC_SYSPRI2 "0xE000ED20"
#define NVIC_PENDSV_PRI "0x00FF0000"
#define NVIC_PENDSVSET "0x10000000"

void __attribute__((naked)) HwInterruptcontextSwitch(x_ubase from, x_ubase to, struct TaskDescriptor *to_task, void *context)
{
    asm volatile ("LDR r4, =KtaskSwitchInterruptFlag");
    asm volatile ("LDR r5, [r4]");
    asm volatile ("CMP r5, #1");
    asm volatile ("BEQ Arm32SwitchReswitch");
    asm volatile ("MOV r5, #1");
    asm volatile ("STR r5, [r4]");
    asm volatile ("LDR r4, =InterruptFromKtask");
    asm volatile ("STR r0, [r4]");
    asm volatile ("B Arm32SwitchReswitch");
}

void __attribute__((naked)) Arm32SwitchReswitch()
{
    asm volatile ("LDR r4, =InterruptToKtask");
    asm volatile ("STR r1, [r4]");
    asm volatile ("LDR r4, =InterruptToKtaskDescriptor");
    asm volatile ("STR r2, [r4]");
    asm volatile ("LDR r0, =" NVIC_INT_CTRL);
    asm volatile ("LDR r1, =" NVIC_PENDSVSET);
    asm volatile ("STR r1, [r0]");
    asm volatile ("BX LR");
}

void __attribute__((naked)) SwitchKtaskContext(x_ubase from, x_ubase to, struct TaskDescriptor *to_task)
{
    asm volatile("B HwInterruptcontextSwitch");
}

void __attribute__((naked)) SwitchKtaskContextTo(x_ubase to, struct TaskDescriptor *to_task)
{
    asm volatile ("LDR r2, =InterruptToKtask");
    asm volatile ("STR r0, [r2]");
    asm volatile ("LDR r2, =InterruptToKtaskDescriptor");
    asm volatile ("STR r1, [r2]");
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    asm volatile ("MRS     r2, CONTROL");
    asm volatile ("BIC     r2, #0x04");
    asm volatile ("MSR     CONTROL, r2");
#endif
    asm volatile ("LDR r1, =InterruptFromKtask");
    asm volatile ("MOV r0, #0x0");
    asm volatile ("STR r0, [r1]");
    asm volatile ("LDR r1, =KtaskSwitchInterruptFlag");
    asm volatile ("MOV r0, #1");
    asm volatile ("STR r0, [r1]");
    asm volatile ("LDR r0, =" NVIC_SYSPRI2);
    asm volatile ("LDR r1, =" NVIC_PENDSV_PRI);
    asm volatile ("LDR.W   r2, [r0,#0x00]");
    asm volatile ("ORR     r1,r1,r2");
    asm volatile ("STR     r1, [r0]");
    asm volatile ("LDR r0, =" NVIC_INT_CTRL);
    asm volatile ("LDR r1, =" NVIC_PENDSVSET);
    asm volatile ("STR r1, [r0]");
    asm volatile ("LDR     r0, =" SCB_VTOR);
    asm volatile ("LDR     r0, [r0]");
    asm volatile ("LDR     r0, [r0]");
    asm volatile ("NOP");
    asm volatile ("MSR     msp, r0");
    asm volatile ("CPSIE   F");
    asm volatile ("CPSIE   I");
    asm volatile ("BX  lr");
}

void __attribute__((naked)) HardFaultHandler()
{
    asm volatile ("MRS     r0, msp");
    asm volatile ("TST     lr, #0x04");
    asm volatile ("BEQ     Arm32SwitchGetSpDone");
    asm volatile ("MRS     r0, psp");
    asm volatile ("B       Arm32SwitchGetSpDone");
}

void __attribute__((naked)) Arm32SwitchGetSpDone()
{
    asm volatile ("MRS     r3, primask");
    asm volatile ("STMFD   r0!, {r3 - r11}");
    asm volatile ("STMFD   r0!, {lr}");
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    asm volatile ("MOV     r4, #0x00");
    asm volatile ("TST     lr, #0x10");
    asm volatile ("MOVEQ   r4, #0x01");
    asm volatile ("STMFD   r0!, {r4}");
#endif
    asm volatile ("TST     lr, #0x04");
    asm volatile ("BEQ     Arm32SwitchUpdateMsp");
    asm volatile ("MSR     psp, r0");
    asm volatile ("B       Arm32SwitchUpdateDone");
    asm volatile ("B       Arm32SwitchUpdateMsp");
}

void __attribute__((naked)) Arm32SwitchUpdateMsp()
{
    asm volatile ("MSR     msp, r0");
    asm volatile ("B       Arm32SwitchUpdateDone");
}

void __attribute__((naked)) Arm32SwitchUpdateDone()
{
    asm volatile ("PUSH    {LR}");
    asm volatile ("BL      HwHardFaultException");
    asm volatile ("POP     {LR}");
    asm volatile ("ORR     lr, lr, #0x04");
    asm volatile ("BX      lr");
}

void __attribute__((naked)) MemFaultHandler()
{
    asm volatile ("MRS     r0, msp");
    asm volatile ("TST     lr, #0x04");
    asm volatile ("BEQ     Arm32Switch1");
    asm volatile ("MRS     r0, psp");
    asm volatile ("B       Arm32Switch1");
}

void __attribute__((naked)) Arm32Switch1()
{
    asm volatile ("MRS     r3, primask");
    asm volatile ("STMFD   r0!, {r3 - r11}");
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    asm volatile ("MOV     r4, #0x00");
    asm volatile ("TST     lr, #0x10");
    asm volatile ("MOVEQ   r4, #0x01");
    asm volatile ("STMFD   r0!, {r4}");
#endif
    asm volatile ("STMFD   r0!, {lr}");
    asm volatile ("PUSH    {LR}");
    asm volatile ("BL      MemFaultHandle");
    asm volatile ("POP     {LR}");
    asm volatile ("ORR     lr, lr, #0x04");
    asm volatile ("BX      lr");
}
