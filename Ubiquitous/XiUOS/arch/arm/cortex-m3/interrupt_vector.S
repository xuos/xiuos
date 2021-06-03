//*****************************************************************************
//
// startup_gcc.c - Startup code for use with GNU tools.
//
// Copyright (c) 2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
// 
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the  
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This is part of revision 10636 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

/**
* @file interrupt_vector.S
* @brief derived from Stellaris Firmware Development Package
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-13
*/

/*************************************************
File name: interrupt_vector.S
Description: vector table for a Cortex M3
Others: 
History: 
1. Date: 2021-05-13
Author: AIIT XUOS Lab
Modification: 
1. take startup_gcc.c from revision 10636 of the Stellaris Firmware Development Package   for XiUOS
*************************************************/


//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//
//*****************************************************************************
.globl  InterruptVectors

/******************************************************************************
*******************************************************************************/
  .section  .isr_vector,"a",%progbits
  .type  InterruptVectors, %object
  .size  InterruptVectors, .-InterruptVectors

InterruptVectors:
  .word  _sp
  .word  Reset_Handler
  .word  NMI_Handler                     //NMI_Handler
  .word  HardFaultHandler
  .word  MemFaultHandler                     //MemManage_Handler
  .word  BusFault_Handler                     //BusFault_Handler
  .word  UsageFault_Handler                     //UsageFault_Handler
  .word  IsrEntry
  .word  IsrEntry
  .word  IsrEntry
  .word  IsrEntry
  .word  IsrEntry                     //SVC_Handler
  .word  IsrEntry                     //DebugMon_Handler
  .word  IsrEntry
  .word  PendSV_Handler
  .word  IsrEntry                     //systick 
  .word  IsrEntry                     // GPIO Port A              
  .word  IsrEntry                     // GPIO Port B              
  .word  IsrEntry                     // GPIO Port C              
  .word  IsrEntry                     // GPIO Port D              
  .word  IsrEntry                     // GPIO Port E              
  .word  IsrEntry                      // UART0 Rx and Tx  
  .word  IsrEntry                     // UART1 Rx and Tx          
  .word  IsrEntry                     // SSI Rx and Tx            
  .word  IsrEntry                     // I2C Master and Slave     
  .word  IsrEntry                     // PWM Fault                
  .word  IsrEntry                     // PWM Generator 0          
  .word  IsrEntry                     // PWM Generator 1          
  .word  IsrEntry                     // PWM Generator 2          
  .word  IsrEntry                     // Quadrature Encoder       
  .word  IsrEntry                     // ADC Sequence 0           
  .word  IsrEntry                     // ADC Sequence 1           
  .word  IsrEntry                     // ADC Sequence 2           
  .word  IsrEntry                     // ADC Sequence 3           
  .word  IsrEntry                     // Watchdog timer           
  .word  IsrEntry                     // Timer 0 subtimer A       
  .word  IsrEntry                     // Timer 0 subtimer B       
  .word  IsrEntry                     // Timer 1 subtimer A       
  .word  IsrEntry                     // Timer 1 subtimer B       
  .word  IsrEntry                     // Timer 2 subtimer A       
  .word  IsrEntry                     // Timer 2 subtimer B       
  .word  IsrEntry                     // Analog Comparator 0      
  .word  IsrEntry                     // Analog Comparator 1      
  .word  IsrEntry                     // Analog Comparator 2      
  .word  IsrEntry                     // System Control (PLL, OSC,
  .word  IsrEntry                     // FLASH Control            
  .word  IsrEntry                     // GPIO Port F              
  .word  IsrEntry                     // GPIO Port G              
  .word  IsrEntry                     // GPIO Port H              
  .word  IsrEntry                     // UART2 Rx and Tx          
  .word  IsrEntry                     // SSI1 Rx and Tx           
  .word  IsrEntry                     // Timer 3 subtimer A       
  .word  IsrEntry                     // Timer 3 subtimer B       
  .word  IsrEntry                     // I2C1 Master and Slave    
  .word  IsrEntry                     // Quadrature Encoder 1     
  .word  IsrEntry                     // CAN0                     
  .word  IsrEntry                     // CAN1                     
  .word  IsrEntry                     // CAN2                     
  .word  IsrEntry                            // Ethernet              
  .word  IsrEntry                     // Hibernate                
  .word  IsrEntry                     // USB0                     
  .word  IsrEntry                     // PWM Generator 3          
  .word  IsrEntry                     // uDMA Software Transfer   
  .word  IsrEntry                       // uDMA Error               
