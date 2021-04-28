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
 
/**
* @file connect_ch438.h
* @brief define aiit-arm32-board ch438 function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef CONNECT_CH438_H
#define CONNECT_CH438_H

#include <board.h>
#include <device.h>

#define CH438_BUFFSIZE 255

/******************************************************************************************/

/* chip definition */
/* CH438serial port0 register address */

#define REG_RBR0_ADDR        0x00      /* serial port0receive buffer register address */
#define REG_THR0_ADDR        0x00      /* serial port0send hold register address */
#define REG_IER0_ADDR        0x01      /* serial port0interrupt enable register address */
#define REG_IIR0_ADDR        0x02      /* serial port0interrupt identifies register address */
#define REG_FCR0_ADDR        0x02      /* serial port0FIFO controls register address */
#define REG_LCR0_ADDR        0x03      /* serial port0circuit control register address */
#define REG_MCR0_ADDR        0x04      /* serial port0MODEM controls register address */
#define REG_LSR0_ADDR        0x05      /* serial port0line status register address */
#define REG_MSR0_ADDR        0x06      /* serial port0address of MODEM status register */
#define REG_SCR0_ADDR        0x07      /* serial port0the user can define the register address */
#define REG_DLL0_ADDR        0x00      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM0_ADDR        0x01      /* Baud rate divisor latch high 8-bit byte address */

/* CH438serial port1 register address */

#define REG_RBR1_ADDR       0x10      /* serial port1receive buffer register address */
#define REG_THR1_ADDR       0x10      /* serial port1send hold register address */
#define REG_IER1_ADDR       0x11      /* serial port1interrupt enable register address */
#define REG_IIR1_ADDR       0x12      /* serial port1interrupt identifies register address */
#define REG_FCR1_ADDR       0x12      /* serial port1FIFO controls register address */
#define REG_LCR1_ADDR       0x13      /* serial port1circuit control register address */
#define REG_MCR1_ADDR       0x14      /* serial port1MODEM controls register address */
#define REG_LSR1_ADDR       0x15      /* serial port1line status register address */
#define REG_MSR1_ADDR       0x16      /* serial port1address of MODEM status register */
#define REG_SCR1_ADDR       0x17      /* serial port1the user can define the register address */
#define REG_DLL1_ADDR       0x10      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM1_ADDR       0x11      /* Baud rate divisor latch high 8-bit byte address */


/* CH438serial port2 register address */

#define REG_RBR2_ADDR       0x20      /* serial port2receive buffer register address */
#define REG_THR2_ADDR       0x20      /* serial port2send hold register address */
#define REG_IER2_ADDR       0x21      /* serial port2interrupt enable register address */
#define REG_IIR2_ADDR       0x22      /* serial port2interrupt identifies register address */
#define REG_FCR2_ADDR       0x22      /* serial port2FIFO controls register address */
#define REG_LCR2_ADDR       0x23      /* serial port2circuit control register address */
#define REG_MCR2_ADDR       0x24      /* serial port2MODEM controls register address */
#define REG_LSR2_ADDR       0x25      /* serial port2line status register address */
#define REG_MSR2_ADDR       0x26      /* serial port2address of MODEM status register */
#define REG_SCR2_ADDR       0x27      /* serial port2the user can define the register address */
#define REG_DLL2_ADDR       0x20      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM2_ADDR       0x21      /* Baud rate divisor latch high 8-bit byte address */



/* CH438serial port3 register address */

#define REG_RBR3_ADDR       0x30      /* serial port3receive buffer register address */
#define REG_THR3_ADDR       0x30      /* serial port3send hold register address */
#define REG_IER3_ADDR       0x31      /* serial port3interrupt enable register address */
#define REG_IIR3_ADDR       0x32      /* serial port3interrupt identifies register address */
#define REG_FCR3_ADDR       0x32      /* serial port3FIFO controls register address */
#define REG_LCR3_ADDR       0x33      /* serial port3circuit control register address */
#define REG_MCR3_ADDR       0x34      /* serial port3MODEM controls register address */
#define REG_LSR3_ADDR       0x35      /* serial port3line status register address */
#define REG_MSR3_ADDR       0x36      /* serial port3address of MODEM status register */
#define REG_SCR3_ADDR       0x37      /* serial port3the user can define the register address */
#define REG_DLL3_ADDR       0x30      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM3_ADDR       0x31      /* Baud rate divisor latch high 8-bit byte address */


/* CH438serial port4 register address */

#define REG_RBR4_ADDR       0x08      /* serial port4receive buffer register address */
#define REG_THR4_ADDR       0x08      /* serial port4send hold register address */
#define REG_IER4_ADDR       0x09      /* serial port4interrupt enable register address */
#define REG_IIR4_ADDR       0x0A      /* serial port4interrupt identifies register address */
#define REG_FCR4_ADDR       0x0A      /* serial port4FIFO controls register address */
#define REG_LCR4_ADDR       0x0B      /* serial port4circuit control register address */
#define REG_MCR4_ADDR       0x0C      /* serial port4MODEM controls register address */
#define REG_LSR4_ADDR       0x0D      /* serial port4line status register address */
#define REG_MSR4_ADDR       0x0E      /* serial port4address of MODEM status register */
#define REG_SCR4_ADDR       0x0F      /* serial port4the user can define the register address */
#define REG_DLL4_ADDR       0x08      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM4_ADDR       0x09      /* Baud rate divisor latch high 8-bit byte address */



/* CH438serial port5 register address */

#define REG_RBR5_ADDR       0x18      /* serial port5receive buffer register address */
#define REG_THR5_ADDR       0x18      /* serial port5send hold register address */
#define REG_IER5_ADDR       0x19      /* serial port5interrupt enable register address */
#define REG_IIR5_ADDR       0x1A      /* serial port5interrupt identifies register address */
#define REG_FCR5_ADDR       0x1A      /* serial port5FIFO controls register address */
#define REG_LCR5_ADDR       0x1B      /* serial port5circuit control register address */
#define REG_MCR5_ADDR       0x1C      /* serial port5MODEM controls register address */
#define REG_LSR5_ADDR       0x1D      /* serial port5line status register address */
#define REG_MSR5_ADDR       0x1E      /* serial port5address of MODEM status register */
#define REG_SCR5_ADDR       0x1F      /* serial port5the user can define the register address */
#define REG_DLL5_ADDR       0x18      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM5_ADDR       0x19      /* Baud rate divisor latch high 8-bit byte address */


/* CH438serial port6 register address */

#define REG_RBR6_ADDR       0x28      /* serial port6receive buffer register address */
#define REG_THR6_ADDR       0x28      /* serial port6send hold register address */
#define REG_IER6_ADDR       0x29      /* serial port6interrupt enable register address */
#define REG_IIR6_ADDR       0x2A      /* serial port6interrupt identifies register address */
#define REG_FCR6_ADDR       0x2A      /* serial port6FIFO controls register address */
#define REG_LCR6_ADDR       0x2B      /* serial port6circuit control register address */
#define REG_MCR6_ADDR       0x2C      /* serial port6MODEM controls register address */
#define REG_LSR6_ADDR       0x2D      /* serial port6line status register address */
#define REG_MSR6_ADDR       0x2E      /* serial port6address of MODEM status register */
#define REG_SCR6_ADDR       0x2F      /* serial port6the user can define the register address */
#define REG_DLL6_ADDR       0x28      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM6_ADDR       0x29      /* Baud rate divisor latch high 8-bit byte address */


/* CH438serial port7 register address */

#define REG_RBR7_ADDR       0x38      /* serial port7receive buffer register address */
#define REG_THR7_ADDR       0x38      /* serial port7send hold register address */
#define REG_IER7_ADDR       0x39      /* serial port7interrupt enable register address */
#define REG_IIR7_ADDR       0x3A      /* serial port7interrupt identifies register address */
#define REG_FCR7_ADDR       0x3A      /* serial port7FIFO controls register address */
#define REG_LCR7_ADDR       0x3B      /* serial port7circuit control register address */
#define REG_MCR7_ADDR       0x3C      /* serial port7MODEM controls register address */
#define REG_LSR7_ADDR       0x3D      /* serial port7line status register address */
#define REG_MSR7_ADDR       0x3E      /* serial port7address of MODEM status register */
#define REG_SCR7_ADDR       0x3F      /* serial port7the user can define the register address */
#define REG_DLL7_ADDR       0x38      /* Baud rate divisor latch low 8-bit byte address */
#define REG_DLM7_ADDR       0x39      /* Baud rate divisor latch high 8-bit byte address */


#define REG_SSR_ADDR        0x4F       /* pecial status register address */


/* IER register bit */

#define BIT_IER_RESET       0x80      /* The bit is 1 soft reset serial port */
#define BIT_IER_LOWPOWER    0x40      /* The bit is 1 close serial port internal reference clock */
#define BIT_IER_SLP         0x20      /* serial port0 is SLP, 1 close clock vibrator  */
#define BIT_IER1_CK2X       0x20      /* serial port1 is CK2X, 1 force the external clock signal after 2 times as internal reference clock */
#define BIT_IER_IEMODEM     0x08      /* The bit is 1 allows MODEM input status to interrupt */
#define BIT_IER_IELINES     0x04      /* The bit is 1 allow receiving line status to be interrupted */
#define BIT_IER_IETHRE      0x02      /* The bit is 1 allows the send hold register to break in mid-air */
#define BIT_IER_IERECV      0x01      /* The bit is 1 allows receiving data interrupts */

/* IIR register bit */

#define BIT_IIR_FIFOENS1    0x80
#define BIT_IIR_FIFOENS0    0x40      /* The two is 1 said use FIFO */

/* Interrupt type: 0001 has no interrupt, 0110 receiving line status is interrupted, 0100 receiving data can be interrupted,
1100 received data timeout interrupt, 0010THR register air interrupt, 0000MODEM input change interrupt */
#define BIT_IIR_IID3        0x08
#define BIT_IIR_IID2        0x04
#define BIT_IIR_IID1        0x02
#define BIT_IIR_NOINT       0x01

/* FCR register bit */

/* Trigger point: 00 corresponds to 1 byte, 01 corresponds to 16 bytes, 10 corresponds to 64 bytes, 11 corresponds to 112 bytes */
#define BIT_FCR_RECVTG1     0x80      /* Set the trigger point for FIFO interruption and automatic hardware flow control */
#define BIT_FCR_RECVTG0     0x40      /* Set the trigger point for FIFO interruption and automatic hardware flow control */

#define BIT_FCR_TFIFORST    0x04      /* The bit is 1 empty the data sent in FIFO */
#define BIT_FCR_RFIFORST    0x02      /* The bit is 1 empty the data sent in FIFO */
#define BIT_FCR_FIFOEN      0x01      /* The bit is 1 use FIFO, 0 disable FIFO */

/* LCR register bit */

#define BIT_LCR_DLAB        0x80      /* To access DLL, DLM, 0 to access RBR/THR/IER */
#define BIT_LCR_BREAKEN     0x40      /* 1 forces a BREAK line interval*/

/* Set the check format: when PAREN is 1, 00 odd check, 01 even check, 10 MARK (set 1), 11 blank (SPACE, clear 0) */
#define BIT_LCR_PARMODE1    0x20      /* Sets the parity bit format */
#define BIT_LCR_PARMODE0    0x10      /* Sets the parity bit format */

#define BIT_LCR_PAREN       0x08      /* A value of 1 allows you to generate and receive parity bits when sending */
#define BIT_LCR_STOPBIT     0x04      /* If is 1, then two stop bits, is 0, a stop bit */

/* Set word length: 00 for 5 data bits, 01 for 6 data bits, 10 for 7 data bits and 11 for 8 data bits */
#define BIT_LCR_WORDSZ1     0x02      /* Set the word length length */
#define BIT_LCR_WORDSZ0     0x01

/* MCR register bit */

#define BIT_MCR_AFE         0x20      /* For 1 allows automatic flow control of CTS and RTS hardware */
#define BIT_MCR_LOOP        0x10      /* Is the test mode of 1 enabling internal loop */
#define BIT_MCR_OUT2        0x08      /* 1 Allows an interrupt request for the serial port output  */
#define BIT_MCR_OUT1        0x04      /* The MODEM control bit defined for the user */
#define BIT_MCR_RTS         0x02      /* The bit is 1 RTS pin  output  effective  */
#define BIT_MCR_DTR         0x01      /* The bit is 1 DTR pin  output  effective  */

/* LSR register bit */

#define BIT_LSR_RFIFOERR    0x80      /* 1 said There is at least one error in receiving FIFO */
#define BIT_LSR_TEMT        0x40      /* 1 said THR and TSR are empty */
#define BIT_LSR_THRE        0x20      /* 1 said THR is empty*/
#define BIT_LSR_BREAKINT    0x10      /* The bit is 1 said the BREAK line interval was detected*/
#define BIT_LSR_FRAMEERR    0x08      /* The bit is 1 said error reading data frame */
#define BIT_LSR_PARERR      0x04      /* The bit is 1 said parity error */
#define BIT_LSR_OVERR       0x02      /*  1 said receive FIFO buffer overflow */
#define BIT_LSR_DATARDY     0x01      /* The bit is 1 said receive data received in FIFO */

/* MSR register bit */

#define BIT_MSR_DCD         0x80      /* The bit is 1 said DCD pin  effective  */
#define BIT_MSR_RI          0x40      /* The bit is 1 said RI pin  effective  */
#define BIT_MSR_DSR         0x20      /* The bit is 1 said DSR pin  effective  */
#define BIT_MSR_CTS         0x10      /* The bit is 1 said CTS pin  effective  */
#define BIT_MSR_DDCD        0x08      /* The bit is 1 said DCD pin The input state has changed */
#define BIT_MSR_TERI        0x04      /* The bit is 1 said RI pin The input state has changed */
#define BIT_MSR_DDSR        0x02      /* The bit is 1 said DSR pin The input state has changed */
#define BIT_MSR_DCTS        0x01      /* The bit is 1 said CTS pin The input state has changed */

/* Interrupt status code */

#define INT_NOINT           0x01      /* There is no interruption */
#define INT_THR_EMPTY       0x02      /* THR empty interruption */
#define INT_RCV_OVERTIME    0x0C      /* Receive timeout interrupt */
#define INT_RCV_SUCCESS     0x04      /* Interrupts are available to receive data */
#define INT_RCV_LINES       0x06      /* Receiving line status interrupted */
#define INT_MODEM_CHANGE    0x00      /* MODEM input changes interrupt */

#define CH438_IIR_FIFOS_ENABLED 0xC0  /* use FIFO */

#define Fpclk           1843200         /* Define the internal clock frequency  */

#if 0
#define CH438_D0_PIN	GET_PIN(E,2)
#define CH438_D1_PIN	GET_PIN(E,3)
#define CH438_D2_PIN	GET_PIN(E,4)
#define CH438_D3_PIN	GET_PIN(E,5)
#define CH438_D4_PIN	GET_PIN(E,6)
#define CH438_D5_PIN	GET_PIN(F,6)
#define CH438_D6_PIN	GET_PIN(F,7)
#define CH438_D7_PIN	GET_PIN(F,8)
#define CH438_NWR_PIN	GET_PIN(C,4)
#define CH438_NRD_PIN	GET_PIN(C,5)
#define CH438_NCS_PIN	GET_PIN(B,1)
#define CH438_ALE_PIN	GET_PIN(B,2)
#define CH438_INT_PIN	GET_PIN(C,13)
#endif
#define CH438_D0_PIN	1
#define CH438_D1_PIN	2
#define CH438_D2_PIN	3
#define CH438_D3_PIN	4
#define CH438_D4_PIN	5
#define CH438_D5_PIN	18
#define CH438_D6_PIN	19
#define CH438_D7_PIN	20
#define CH438_NWR_PIN	44
#define CH438_NRD_PIN	45
#define CH438_NCS_PIN	47
#define CH438_ALE_PIN	48
#define CH438_INT_PIN	7
#define	DIR_485CH1_PIN	22	//485ch1 = ext_uart3
#define	DIR_485CH2_PIN	21	//485ch2 = ext_uart2
#define	DIR_485CH3_PIN	123	//485ch3 = ext_uart7

void CH438RegTest(unsigned char num);
void Set485Input(uint8	ch_no);
void set_485_output(uint8	ch_no);

#endif
