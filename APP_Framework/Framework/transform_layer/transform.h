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
 * @file transform.h
 * @brief Interface function declarations required by the framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.06.04
 */

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_NUM_MAX            32

#define BAUD_RATE_2400          2400
#define BAUD_RATE_4800          4800
#define BAUD_RATE_9600          9600
#define BAUD_RATE_19200         19200
#define BAUD_RATE_38400         38400
#define BAUD_RATE_57600         57600
#define BAUD_RATE_115200        115200
#define BAUD_RATE_230400        230400
#define BAUD_RATE_460800        460800
#define BAUD_RATE_921600        921600
#define BAUD_RATE_2000000       2000000
#define BAUD_RATE_3000000       3000000

#define DATA_BITS_5             5
#define DATA_BITS_6             6
#define DATA_BITS_7             7
#define DATA_BITS_8             8
#define DATA_BITS_9             9

#define STOP_BITS_1             1
#define STOP_BITS_2             2
#define STOP_BITS_3             3
#define STOP_BITS_4             4

#define PARITY_NONE             1
#define PARITY_ODD              2
#define PARITY_EVEN             3

#define BIT_ORDER_LSB           1
#define BIT_ORDER_MSB           2

#define NRZ_NORMAL              1
#define NRZ_INVERTED            2

#ifndef SERIAL_RB_BUFSZ
#define SERIAL_RB_BUFSZ         128
#endif

struct SerialDataCfg
{
    uint32_t serial_baud_rate;
    uint8_t serial_data_bits;
    uint8_t serial_stop_bits;
    uint8_t serial_parity_mode;
    uint8_t serial_bit_order;
    uint8_t serial_invert_mode;
    uint16_t serial_buffer_size;
};

enum IoctlCmd
{
    SERIAL_CFG_SETS = 0,
    SERIAL_CFG_GETS,
};

/**********************mutex**************************/

int32_t PrivMutexCreate(void);
void PrivMutexDelete(int32_t mutex);
int32_t PrivMutexObtain(int32_t mutex, int32_t wait_time);
int32_t PrivMutexAbandon(int32_t mutex);

/*********************semaphore**********************/

int32_t PrivSemaphoreCreate(uint16_t val);
int32_t PrivSemaphoreDelete(int32_t sem);
int32_t PrivSemaphoreObtain(int32_t sem, int32_t wait_time);
int32_t PrivSemaphoreAbandon(int32_t sem);
int32_t PrivSemaphoreSetValue(int32_t sem, uint16_t val);

/*********************task**************************/

struct utask
{
	char        name[NAME_NUM_MAX];         
    void        *func_entry;                
    void        *func_param;     
    int32_t     stack_size;  
    uint8_t     prio; 
};
typedef struct utask UtaskType;

int32_t PrivTaskCreate(UtaskType utask);
int32_t PrivTaskStartup(int32_t id);
int32_t PrivTaskDelete(int32_t id);
void PrivTaskQuit(void);
int32_t PrivTaskDelay(int32_t ms);

int PrivOpen(const char *path, int flags, ...);
int PrivRead(int fd, void *buf, size_t len);
int PrivWrite(int fd, const void *buf, size_t len);
int PrivClose(int fd);
int PrivIoctl(int fd, int cmd, void *args);

#ifdef __cplusplus
}
#endif

#endif