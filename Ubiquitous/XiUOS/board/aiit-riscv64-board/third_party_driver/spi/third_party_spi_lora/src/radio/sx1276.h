/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276.h
 * \brief      SX1276 RF chip high level functions driver
 *
 * \remark     Optional support functions.
 *             These functions are defined only to easy the change of the
 *             parameters.
 *             For a final firmware the radio parameters will be known so
 *             there is no need to support all possible parameters.
 *             Removing these functions will greatly reduce the final firmware
 *             size.
 *
 * \version    2.0.0 
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
/*************************************************
File name: sx1276.h
Description: support aiit board configure and register function
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. replace original macro and basic date type with AIIT XUOS Lab's own defination
*************************************************/


#ifndef __SX1276_H__
#define __SX1276_H__

#include <stdint.h>
#include <stdbool.h>

extern uint8_t SX1276Regs[0x70];     						 //SX1276寄存器数组

void SX1276Init( void );									 //初始化SX1276

void SX1276Reset( void );									 //重置SX1276

/*以下函数都没有被使用到，因为在sx1276-LoRa.h里面又定义了一系列与下面作用相同的函数*/
void SX1276_SetLoRaOn( bool enable ); 						 //启用LoRa调制解调器或FSK调制解调器

bool SX1276_GetLoRaOn( void );								 //获取LoRa调制解调器状态

void SX1276SetOpMode( uint8_t opMode );  					 //设置SX1276操作模式

uint8_t SX1276_GetOpMode( void );     						 //获取SX1276操作模式

uint8_t SX1276_ReadRxGain( void );   						 //读取当前Rx增益设置

double SX1276ReadRssi( void );       						 //读取无线信号强度

uint8_t SX1276_GetPacketRxGain( void );                      //获取数据时的增益值

int8_t SX1276_GetPacketSnr( void );							 //获取数据时的信噪比值，信号和噪声的比值，信噪比越高，说明信号干扰越小。

double SX1276_GetPacketRssi( void );  					     //获取数据是的无线信号强度

/*!
 * \brief Gets the AFC value measured while receiving the packet
 *
 * \retval afcValue Current AFC value in [Hz]
 */
uint32_t SX1276GetPacketAfc( void );                          //此函数不知道作用


void SX1276StartRx( void );                                   //开始接收

void SX1276GetRxPacket( void *buffer, uint16_t *size );       //得到接收的数据

void SX1276SetTxPacket( const void *buffer, uint16_t size );  //发送数据

uint8_t SX1276GetRFState( void );       				      //得到RFLRState状态

void SX1276SetRFState( uint8_t state );						  //设置RFLRState状态，RFLRState的值决定了下面的函数处理哪一步的代码

uint32_t SX1276Process( void );         			          //SX1276模块接发收数据的处理函数

uint32_t SX1276ChannelEmpty( void );

#endif 
