/**
  ******************************************************************************
  * @file      startup_stm32f407xx.s
  * @author    MCD Application Team
  * @brief     STM32F407xx Devices vector table for GCC based toolchains. 
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/**
* @file boot.S
* @brief derived from ST standard peripheral library
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: boot.S
Description: Reset and init function
Others: 
History: 
1. Date: 2021-04-29
Author: AIIT XUOS Lab
Modification: 
1. take startup_stm32f407xx.s for XiUOS
*************************************************/

  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.word  _sidata
.word  _sdata
.word  _edata
.word  __bss_start
.word  __bss_end

  .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr   sp, =__stack_tp
  movs  r1, #0

/* Copy the data segment initializers from flash to SRAM */
DataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds r2, r0, r1
  cmp  r2, r3
  bcs  DataInitEnd
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds r1, r1, #4
  b  DataInit

DataInitEnd:
  ldr  r2, =__bss_start

/* Zero fill the bss segment. */
BSSInit:
  ldr  r3, = __bss_end
  cmp  r2, r3
  bcs  BSSInitEnd
  movs r3, #0
  str  r3, [r2], #4
  b  BSSInit

BSSInitEnd:
  bl  SystemInit
  bl  stm32f407_start
  bx  lr
.size  Reset_Handler, .-Reset_Handler