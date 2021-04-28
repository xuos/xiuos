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
* @file coreclock.c
* @brief derived from ST standard peripheral library
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

.globl  InterruptVectors

/******************************************************************************
*******************************************************************************/
  .section  .isr_vector,"a",%progbits
  .type  InterruptVectors, %object
  .size  InterruptVectors, .-InterruptVectors

InterruptVectors:
  .word  __stack_end__

  .word     Reset_Handler
  .word     NMI_Handler
  .word     HardFaultHandler
  .word     MemFaultHandler
  .word     BusFault_Handler
  .word     UsageFault_Handler
  .word     IsrEntry
  .word     IsrEntry
  .word     IsrEntry
  .word     IsrEntry
  .word     SVC_Entry     /* SVC                          */
  .word     IsrEntry     /* DebugMon                     */
  .word     IsrEntry
  .word     PendSV_Handler
  .word     IsrEntry     /* SysTick                      */

  .word     IsrEntry     /* Window WatchDog              */                                        
  .word     IsrEntry     /* PVD through EXTI Line detection */                        
  .word     IsrEntry     /* Tamper and TimeStamps through the EXTI line */            
  .word     IsrEntry     /* RTC Wakeup through the EXTI line */                      
  .word     IsrEntry     /* FLASH                        */                                          
  .word     IsrEntry     /* RCC                          */                                            
  .word     IsrEntry     /* EXTI Line0                   */                        
  .word     IsrEntry     /* EXTI Line1                   */                          
  .word     IsrEntry     /* EXTI Line2                   */                          
  .word     IsrEntry     /* EXTI Line3                   */                          
  .word     IsrEntry     /* EXTI Line4                   */                          
  .word     IsrEntry     /* DMA1 Stream 0                */                  
  .word     IsrEntry     /* DMA1 Stream 1                */                   
  .word     IsrEntry     /* DMA1 Stream 2                */                   
  .word     IsrEntry     /* DMA1 Stream 3                */                   
  .word     IsrEntry     /* DMA1 Stream 4                */                   
  .word     IsrEntry     /* DMA1 Stream 5                */                   
  .word     IsrEntry     /* DMA1 Stream 6                */                   
  .word     IsrEntry     /* ADC1, ADC2 and ADC3s         */                   
  .word     IsrEntry     /* CAN1 TX                      */                         
  .word     IsrEntry     /* CAN1 RX0                     */                          
  .word     IsrEntry     /* CAN1 RX1                     */                          
  .word     IsrEntry     /* CAN1 SCE                     */                          
  .word     IsrEntry     /* External Line[9:5]s          */                          
  .word     IsrEntry     /* TIM1 Break and TIM9          */         
  .word     IsrEntry     /* TIM1 Update and TIM10        */         
  .word     IsrEntry     /* TIM1 Trigger and Commutation and TIM11 */
  .word     IsrEntry     /* TIM1 Capture Compare         */                          
  .word     IsrEntry     /* TIM2                         */                   
  .word     IsrEntry     /* TIM3                         */                   
  .word     IsrEntry     /* TIM4                         */                   
  .word     IsrEntry     /* I2C1 Event                   */                          
  .word     IsrEntry     /* I2C1 Error                   */                          
  .word     IsrEntry     /* I2C2 Event                   */                          
  .word     IsrEntry     /* I2C2 Error                   */                            
  .word     IsrEntry     /* SPI1                         */                   
  .word     IsrEntry     /* SPI2                         */                   
  .word     IsrEntry     /* USART1                       */                   
  .word     IsrEntry     /* USART2                       */                   
  .word     IsrEntry     /* USART3                       */                   
  .word     IsrEntry     /* External Line[15:10]s        */                          
  .word     IsrEntry     /* RTC Alarm (A and B) through EXTI Line */                 
  .word     IsrEntry     /* USB OTG FS Wakeup through EXTI line */                       
  .word     IsrEntry     /* TIM8 Break and TIM12         */         
  .word     IsrEntry     /* TIM8 Update and TIM13        */         
  .word     IsrEntry     /* TIM8 Trigger and Commutation and TIM14 */
  .word     IsrEntry     /* TIM8 Capture Compare         */                          
  .word     IsrEntry     /* DMA1 Stream7                 */                          
  .word     IsrEntry     /* FSMC                         */                   
  .word     IsrEntry     /* SDIO                         */                   
  .word     IsrEntry     /* TIM5                         */                   
  .word     IsrEntry     /* SPI3                         */                   
  .word     IsrEntry     /* UART4                        */                   
  .word     IsrEntry     /* UART5                        */                   
  .word     IsrEntry     /* TIM6 and DAC1&2 underrun errors */                   
  .word     IsrEntry     /* TIM7                         */
  .word     IsrEntry     /* DMA2 Stream 0                */                   
  .word     IsrEntry     /* DMA2 Stream 1                */                   
  .word     IsrEntry     /* DMA2 Stream 2                */                   
  .word     IsrEntry     /* DMA2 Stream 3                */                   
  .word     IsrEntry     /* DMA2 Stream 4                */                   
  .word     IsrEntry     /* Ethernet                     */                   
  .word     IsrEntry     /* Ethernet Wakeup through EXTI line */                     
  .word     IsrEntry     /* CAN2 TX                      */                          
  .word     IsrEntry     /* CAN2 RX0                     */                          
  .word     IsrEntry     /* CAN2 RX1                     */                          
  .word     IsrEntry     /* CAN2 SCE                     */                          
  .word     IsrEntry     /* USB OTG FS                   */                   
  .word     IsrEntry     /* DMA2 Stream 5                */                   
  .word     IsrEntry     /* DMA2 Stream 6                */                   
  .word     IsrEntry     /* DMA2 Stream 7                */                   
  .word     IsrEntry     /* USART6                       */                    
  .word     IsrEntry     /* I2C3 event                   */                          
  .word     IsrEntry     /* I2C3 error                   */                          
  .word     IsrEntry     /* USB OTG HS End Point 1 Out   */                   
  .word     IsrEntry     /* USB OTG HS End Point 1 In    */                   
  .word     IsrEntry     /* USB OTG HS Wakeup through EXTI */                         
  .word     IsrEntry     /* USB OTG HS                   */                   
  .word     IsrEntry     /* DCMI                         */                   
  .word     IsrEntry     /* CRYP crypto                  */                   
  .word     IsrEntry     /* Hash and Rng                 */
  .word     IsrEntry     /* FPU                          */
