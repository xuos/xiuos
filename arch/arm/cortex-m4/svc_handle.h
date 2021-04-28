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

#ifndef __INC_SVC_HANDLE_H__
#define __INC_SVC_HANDLE_H__

#if defined ( __VFP_FP__ ) && !defined(__SOFTFP__)
#define INT_FPU_REGS         (1)
#else
#define INT_FPU_REGS         (0)
#endif

#define HW_INT_REGS             (8)
#define SW_INT_REGS            (9 + INT_FPU_REGS)

#define REG_INT_R0              (SW_INT_REGS + 0) /* R0 */
#define REG_INT_R1              (SW_INT_REGS + 1) /* R1 */
#define REG_INT_R2              (SW_INT_REGS + 2) /* R2 */
#define REG_INT_R3              (SW_INT_REGS + 3) /* R3 */
#define REG_INT_R12             (SW_INT_REGS + 4) /* R12 */
#define REG_INT_R14             (SW_INT_REGS + 5) /* R14 = LR */
#define REG_INT_PC              (SW_INT_REGS + 6) /* R15 = PC */
#define REG_INT_XPSR            (SW_INT_REGS + 7) /* xPSR */

#if defined ( __VFP_FP__ ) && !defined(__SOFTFP__)
#define REG_INT_FPU_FLAG        (0)  /* fpu flag */
#define REG_INT_PRIMASK         (1)  /* PRIMASK */
#define REG_INT_R4              (2)  /* R4 */
#define REG_INT_R5              (3)  /* R5 */
#define REG_INT_R6              (4)  /* R6 */
#define REG_INT_R7              (5)  /* R7 */
#define REG_INT_R8              (6)  /* R8 */
#define REG_INT_R9              (7)  /* R9 */
#define REG_INT_R10             (8)  /* R10 */
#define REG_INT_R11             (9)  /* R11 */
//#define REG_INT_EXC_RETURN      (10) /* EXC_RETURN */
#else
#define REG_INT_PRIMASK         (0)  /* PRIMASK */
#define REG_INT_R4              (1)  /* R4 */
#define REG_INT_R5              (2)  /* R5 */
#define REG_INT_R6              (3)  /* R6 */
#define REG_INT_R7              (4)  /* R7 */
#define REG_INT_R8              (5)  /* R8 */
#define REG_INT_R9              (6)  /* R9 */
#define REG_INT_R10             (7)  /* R10 */
#define REG_INT_R11             (8)  /* R11 */
//#define REG_INT_EXC_RETURN      (9) /* EXC_RETURN */
#endif

#define EXC_RETURN_BASE          0xffffffe1


#define EXC_RETURN_PROCESS_STACK (1 << 2)
#define EXC_RETURN_THREAD_MODE   (1 << 3)
#define EXC_RETURN_STD_CONTEXT   (1 << 4)

#define EXC_RETURN_PRIVTHR     (EXC_RETURN_BASE | EXC_RETURN_STD_CONTEXT | EXC_RETURN_THREAD_MODE |EXC_RETURN_PROCESS_STACK)
#define EXC_RETURN_UNPRIVTHR   (EXC_RETURN_BASE | EXC_RETURN_STD_CONTEXT | EXC_RETURN_THREAD_MODE |EXC_RETURN_PROCESS_STACK)

#endif
