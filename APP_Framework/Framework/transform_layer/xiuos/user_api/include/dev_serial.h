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
* @file dev_serial.h
* @brief define serial dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date
*/

#ifndef DEV_SERIAL_H
#define DEV_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define BAUD_RATE_2400                       2400
#define BAUD_RATE_4800                       4800
#define BAUD_RATE_9600                       9600
#define BAUD_RATE_19200                  19200
#define BAUD_RATE_38400                  38400
#define BAUD_RATE_57600                  57600
#define BAUD_RATE_115200             115200
#define BAUD_RATE_230400             230400
#define BAUD_RATE_460800             460800
#define BAUD_RATE_921600             921600
#define BAUD_RATE_2000000        2000000
#define BAUD_RATE_3000000        3000000

#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

#define STOP_BITS_1                     1
#define STOP_BITS_2                     2
#define STOP_BITS_3                     3
#define STOP_BITS_4                     4

#define PARITY_NONE                     1
#define PARITY_ODD                        2
#define PARITY_EVEN                      3

#define BIT_ORDER_LSB                   1
#define BIT_ORDER_MSB                  2

#define NRZ_NORMAL                      1     
#define NRZ_INVERTED                   2    

#ifndef SERIAL_RB_BUFSZ
#define SERIAL_RB_BUFSZ              128
#endif

#ifdef __cplusplus
}
#endif

#endif
