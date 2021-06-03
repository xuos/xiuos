/* ------------------------------------------------------------------------- */
/*  @file:    startup_MIMXRT1052.s                                           */
/*  @purpose: CMSIS Cortex-M7 Core Device Startup File                       */
/*            MIMXRT1052                                                     */
/*  @version: 1.0                                                            */
/*  @date:    2018-9-21                                                      */
/*  @build:   b180921                                                        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/* Copyright 1997-2016 Freescale Semiconductor, Inc.                         */
/* Copyright 2016-2018 NXP                                                   */
/* All rights reserved.                                                      */
/*                                                                           */
/* SPDX-License-Identifier: BSD-3-Clause                                     */
/*****************************************************************************/
/* Version: GCC for ARM Embedded Processors                                  */
/*****************************************************************************/


/**
* @file interrupt_vector.S
* @brief vector table for Cortex M7
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-28
*/

/*************************************************
File name: interrupt_vector.S
Description: vector table for a Cortex M7
Others: 
History: 
1. Date: 2021-05-28
Author: AIIT XUOS Lab
Modification: 
1. add IsrEntry as default isr function
*************************************************/

    .syntax unified
    .arch armv7-m

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                                      /* Top of Stack */
    .long   Reset_Handler                                   /* Reset Handler */
    .long   NMI_Handler                                     /* NMI Handler*/
    .long   HardFaultHandler                               /* Hard Fault Handler*/
    .long   MemFaultHandler                               /* MPU Fault Handler*/
    .long   BusFault_Handler                                /* Bus Fault Handler*/
    .long   UsageFault_Handler                              /* Usage Fault Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   IsrEntry                                     /* SVCall Handler*/
    .long   IsrEntry                                /* Debug Monitor Handler*/
    .long   0                                               /* Reserved*/
    .long   PendSV_Handler                                  /* PendSV Handler*/
    .long   IsrEntry                                 /* SysTick Handler*/

                                                            /* External Interrupts*/
    .long   IsrEntry                           /* DMA channel 0/16 transfer complete*/
    .long   IsrEntry                           /* DMA channel 1/17 transfer complete*/
    .long   IsrEntry                           /* DMA channel 2/18 transfer complete*/
    .long   IsrEntry                           /* DMA channel 3/19 transfer complete*/
    .long   IsrEntry                           /* DMA channel 4/20 transfer complete*/
    .long   IsrEntry                           /* DMA channel 5/21 transfer complete*/
    .long   IsrEntry                           /* DMA channel 6/22 transfer complete*/
    .long   IsrEntry                           /* DMA channel 7/23 transfer complete*/
    .long   IsrEntry                           /* DMA channel 8/24 transfer complete*/
    .long   IsrEntry                           /* DMA channel 9/25 transfer complete*/
    .long   IsrEntry                          /* DMA channel 10/26 transfer complete*/
    .long   IsrEntry                          /* DMA channel 11/27 transfer complete*/
    .long   IsrEntry                          /* DMA channel 12/28 transfer complete*/
    .long   IsrEntry                          /* DMA channel 13/29 transfer complete*/
    .long   IsrEntry                          /* DMA channel 14/30 transfer complete*/
    .long   IsrEntry                          /* DMA channel 15/31 transfer complete*/
    .long   IsrEntry                            /* DMA error interrupt channels 0-15 / 16-31*/
    .long   IsrEntry                           /* CTI0_Error*/
    .long   IsrEntry                           /* CTI1_Error*/
    .long   IsrEntry                                 /* CorePlatform exception IRQ*/
    .long   IsrEntry                              /* LPUART1 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART2 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART3 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART4 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART5 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART6 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART7 TX interrupt and RX interrupt*/
    .long   IsrEntry                              /* LPUART8 TX interrupt and RX interrupt*/
    .long   IsrEntry                               /* LPI2C1 interrupt*/
    .long   IsrEntry                               /* LPI2C2 interrupt*/
    .long   IsrEntry                               /* LPI2C3 interrupt*/
    .long   IsrEntry                               /* LPI2C4 interrupt*/
    .long   IsrEntry                               /* LPSPI1 single interrupt vector for all sources*/
    .long   IsrEntry                               /* LPSPI2 single interrupt vector for all sources*/
    .long   IsrEntry                               /* LPSPI3 single interrupt vector for all sources*/
    .long   IsrEntry                               /* LPSPI4  single interrupt vector for all sources*/
    .long   IsrEntry                                 /* CAN1 interrupt*/
    .long   IsrEntry                                 /* CAN2 interrupt*/
    .long   IsrEntry                              /* FlexRAM address out of range Or access hit IRQ*/
    .long   IsrEntry                                  /* Keypad nterrupt*/
    .long   IsrEntry                              /* TSC interrupt*/
    .long   IsrEntry                              /* GPR interrupt*/
    .long   IsrEntry                                /* LCDIF interrupt*/
    .long   IsrEntry                                  /* CSI interrupt*/
    .long   IsrEntry                                  /* PXP interrupt*/
    .long   IsrEntry                                /* WDOG2 interrupt*/
    .long   IsrEntry                      /* SRTC Consolidated Interrupt. Non TZ*/
    .long   IsrEntry                   /* SRTC Security Interrupt. TZ*/
    .long   IsrEntry                      /* ON-OFF button press shorter than 5 secs (pulse event)*/
    .long   IsrEntry                                  /* CSU interrupt*/
    .long   IsrEntry                                  /* DCP_IRQ interrupt*/
    .long   IsrEntry                              /* DCP_VMI_IRQ interrupt*/
    .long   IsrEntry                           /* Reserved interrupt*/
    .long   IsrEntry                                 /* TRNG interrupt*/
    .long   IsrEntry                                  /* SJC interrupt*/
    .long   IsrEntry                                  /* BEE interrupt*/
    .long   IsrEntry                                 /* SAI1 interrupt*/
    .long   IsrEntry                                 /* SAI1 interrupt*/
    .long   IsrEntry                              /* SAI3 interrupt*/
    .long   IsrEntry                              /* SAI3 interrupt*/
    .long   IsrEntry                                /* SPDIF interrupt*/
    .long   IsrEntry                            /* Brown-out event interrupt*/
    .long   IsrEntry                           /* Reserved interrupt*/
    .long   IsrEntry                        /* TempSensor low/high interrupt*/
    .long   IsrEntry                           /* TempSensor panic interrupt*/
    .long   IsrEntry                             /* USBPHY (UTMI0), Interrupt*/
    .long   IsrEntry                             /* USBPHY (UTMI0), Interrupt*/
    .long   IsrEntry                                 /* ADC1 interrupt*/
    .long   IsrEntry                                 /* ADC2 interrupt*/
    .long   IsrEntry                                 /* DCDC interrupt*/
    .long   IsrEntry                           /* Reserved interrupt*/
    .long   IsrEntry                           /* Reserved interrupt*/
    .long   IsrEntry                          /* Active HIGH Interrupt from INT0 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT1 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT2 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT3 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT4 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT5 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT6 from GPIO*/
    .long   IsrEntry                 /* Active HIGH Interrupt from INT7 from GPIO*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO1 signal 0 throughout 15*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO1 signal 16 throughout 31*/
    .long   IsrEntry                  /* Combined interrupt indication for GPIO2 signal 0 throughout 15*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO2 signal 16 throughout 31*/
    .long   IsrEntry                  /* Combined interrupt indication for GPIO3 signal 0 throughout 15*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO3 signal 16 throughout 31*/
    .long   IsrEntry                  /* Combined interrupt indication for GPIO4 signal 0 throughout 15*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO4 signal 16 throughout 31*/
    .long   IsrEntry                  /* Combined interrupt indication for GPIO5 signal 0 throughout 15*/
    .long   IsrEntry                 /* Combined interrupt indication for GPIO5 signal 16 throughout 31*/
    .long   IsrEntry                              /* FLEXIO1 interrupt*/
    .long   IsrEntry                              /* FLEXIO2 interrupt*/
    .long   IsrEntry                                /* WDOG1 interrupt*/
    .long   IsrEntry                               /* RTWDOG interrupt*/
    .long   IsrEntry                                  /* EWM interrupt*/
    .long   IsrEntry                                /* CCM IRQ1 interrupt*/
    .long   IsrEntry                                /* CCM IRQ2 interrupt*/
    .long   IsrEntry                                  /* GPC interrupt*/
    .long   IsrEntry                                  /* SRC interrupt*/
    .long   IsrEntry                          /* Reserved interrupt*/
    .long   IsrEntry                    /* GPT1 interrupt*/
    .long   IsrEntry                    /* GPT2 interrupt*/
    .long   IsrEntry                    /* PWM1 capture 0, compare 0, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM1 capture 1, compare 1, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM1 capture 2, compare 2, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM1 capture 3, compare 3, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM1 fault or reload error interrupt*/
    .long   IsrEntry                    /* Reserved interrupt*/
    .long   IsrEntry                    /* FlexSPI0 interrupt*/
    .long   IsrEntry                    /* Reserved interrupt*/
    .long   IsrEntry                    /* USDHC1 interrupt*/
    .long   IsrEntry                    /* USDHC2 interrupt*/
    .long   IsrEntry                    /* USBO2 USB OTG2*/
    .long   IsrEntry                    /* USBO2 USB OTG1*/
    .long   IsrEntry                    /* ENET interrupt*/
    .long   IsrEntry                    /* ENET_1588_Timer interrupt*/
    .long   IsrEntry                    /* XBAR1 interrupt*/
    .long   IsrEntry                    /* XBAR1 interrupt*/
    .long   IsrEntry                    /* ADCETC IRQ0 interrupt*/
    .long   IsrEntry                    /* ADCETC IRQ1 interrupt*/
    .long   IsrEntry                    /* ADCETC IRQ2 interrupt*/
    .long   IsrEntry                    /* ADCETC Error IRQ interrupt*/
    .long   IsrEntry                    /* PIT interrupt*/
    .long   IsrEntry                    /* ACMP interrupt*/
    .long   IsrEntry                    /* ACMP interrupt*/
    .long   IsrEntry                    /* ACMP interrupt*/
    .long   IsrEntry                    /* ACMP interrupt*/
    .long   IsrEntry                    /* Reserved interrupt*/
    .long   IsrEntry                    /* Reserved interrupt*/
    .long   IsrEntry                    /* ENC1 interrupt*/
    .long   IsrEntry                    /* ENC2 interrupt*/
    .long   IsrEntry                    /* ENC3 interrupt*/
    .long   IsrEntry                    /* ENC4 interrupt*/
    .long   IsrEntry                    /* TMR1 interrupt*/
    .long   IsrEntry                    /* TMR2 interrupt*/
    .long   IsrEntry                    /* TMR3 interrupt*/
    .long   IsrEntry                    /* TMR4 interrupt*/
    .long   IsrEntry                    /* PWM2 capture 0, compare 0, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM2 capture 1, compare 1, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM2 capture 2, compare 2, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM2 capture 3, compare 3, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM2 fault or reload error interrupt*/
    .long   IsrEntry                    /* PWM3 capture 0, compare 0, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM3 capture 1, compare 1, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM3 capture 2, compare 2, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM3 capture 3, compare 3, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM3 fault or reload error interrupt*/
    .long   IsrEntry                    /* PWM4 capture 0, compare 0, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM4 capture 1, compare 1, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM4 capture 2, compare 2, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM4 capture 3, compare 3, or reload 0 interrupt*/
    .long   IsrEntry                    /* PWM4 fault or reload error interrupt*/
    .long   IsrEntry                                      /* 168*/
    .long   IsrEntry                                      /* 169*/
    .long   IsrEntry                                      /* 170*/
    .long   IsrEntry                                      /* 171*/
    .long   IsrEntry                                      /* 172*/
    .long   IsrEntry                                      /* 173*/
    .long   IsrEntry                                      /* 174*/
    .long   IsrEntry                                      /* 175*/
    .long   IsrEntry                                      /* 176*/
    .long   IsrEntry                                      /* 177*/
    .long   IsrEntry                                      /* 178*/
    .long   IsrEntry                                      /* 179*/
    .long   IsrEntry                                      /* 180*/
    .long   IsrEntry                                      /* 181*/
    .long   IsrEntry                                      /* 182*/
    .long   IsrEntry                                      /* 183*/
    .long   IsrEntry                                      /* 184*/
    .long   IsrEntry                                      /* 185*/
    .long   IsrEntry                                      /* 186*/
    .long   IsrEntry                                      /* 187*/
    .long   IsrEntry                                      /* 188*/
    .long   IsrEntry                                      /* 189*/
    .long   IsrEntry                                      /* 190*/
    .long   IsrEntry                                      /* 191*/
    .long   IsrEntry                                      /* 192*/
    .long   IsrEntry                                      /* 193*/
    .long   IsrEntry                                      /* 194*/
    .long   IsrEntry                                      /* 195*/
    .long   IsrEntry                                      /* 196*/
    .long   IsrEntry                                      /* 197*/
    .long   IsrEntry                                      /* 198*/
    .long   IsrEntry                                      /* 199*/
    .long   IsrEntry                                      /* 200*/
    .long   IsrEntry                                      /* 201*/
    .long   IsrEntry                                      /* 202*/
    .long   IsrEntry                                      /* 203*/
    .long   IsrEntry                                      /* 204*/
    .long   IsrEntry                                      /* 205*/
    .long   IsrEntry                                      /* 206*/
    .long   IsrEntry                                      /* 207*/
    .long   IsrEntry                                      /* 208*/
    .long   IsrEntry                                      /* 209*/
    .long   IsrEntry                                      /* 210*/
    .long   IsrEntry                                      /* 211*/
    .long   IsrEntry                                      /* 212*/
    .long   IsrEntry                                      /* 213*/
    .long   IsrEntry                                      /* 214*/
    .long   IsrEntry                                      /* 215*/
    .long   IsrEntry                                      /* 216*/
    .long   IsrEntry                                      /* 217*/
    .long   IsrEntry                                      /* 218*/
    .long   IsrEntry                                      /* 219*/
    .long   IsrEntry                                      /* 220*/
    .long   IsrEntry                                      /* 221*/
    .long   IsrEntry                                      /* 222*/
    .long   IsrEntry                                      /* 223*/
    .long   IsrEntry                                      /* 224*/
    .long   IsrEntry                                      /* 225*/
    .long   IsrEntry                                      /* 226*/
    .long   IsrEntry                                      /* 227*/
    .long   IsrEntry                                      /* 228*/
    .long   IsrEntry                                      /* 229*/
    .long   IsrEntry                                      /* 230*/
    .long   IsrEntry                                      /* 231*/
    .long   IsrEntry                                      /* 232*/
    .long   IsrEntry                                      /* 233*/
    .long   IsrEntry                                      /* 234*/
    .long   IsrEntry                                      /* 235*/
    .long   IsrEntry                                      /* 236*/
    .long   IsrEntry                                      /* 237*/
    .long   IsrEntry                                      /* 238*/
    .long   IsrEntry                                      /* 239*/
    .long   IsrEntry                                      /* 240*/
    .long   IsrEntry                                      /* 241*/
    .long   IsrEntry                                      /* 242*/
    .long   IsrEntry                                      /* 243*/
    .long   IsrEntry                                      /* 244*/
    .long   IsrEntry                                      /* 245*/
    .long   IsrEntry                                      /* 246*/
    .long   IsrEntry                                      /* 247*/
    .long   IsrEntry                                      /* 248*/
    .long   IsrEntry                                      /* 249*/
    .long   IsrEntry                                      /* 250*/
    .long   IsrEntry                                      /* 251*/
    .long   IsrEntry                                      /* 252*/
    .long   IsrEntry                                      /* 253*/
    .long   IsrEntry                                      /* 254*/
    .long   0xFFFFFFFF                                      /*  Reserved for user TRIM value*/

    .size    __isr_vector, . - __isr_vector

    .text
    .thumb
