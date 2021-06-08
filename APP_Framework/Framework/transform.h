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
 * @brief define the struct and transform function
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.06.07
 */

#include <xiuos.h>
#include <iot-vfs.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

//for test
#define XIUOS_OS

#define OPE_INT        0x0000
#define OPE_CFG        0x0001

#ifdef XIUOS_OS

/************************Serial Configure define***********************/
#define BAUD_RATE_2400        2400
#define BAUD_RATE_4800        4800
#define BAUD_RATE_9600        9600
#define BAUD_RATE_19200        19200
#define BAUD_RATE_38400        38400
#define BAUD_RATE_57600        57600
#define BAUD_RATE_115200        115200
#define BAUD_RATE_230400        230400
#define BAUD_RATE_460800        460800
#define BAUD_RATE_921600        921600
#define BAUD_RATE_2000000      2000000
#define BAUD_RATE_3000000      3000000

#define DATA_BITS_5        5
#define DATA_BITS_6        6
#define DATA_BITS_7        7
#define DATA_BITS_8        8
#define DATA_BITS_9        9

#define STOP_BITS_1        1
#define STOP_BITS_2        2
#define STOP_BITS_3        3
#define STOP_BITS_4        4

#define PARITY_NONE        1
#define PARITY_ODD        2
#define PARITY_EVEN        3

#define BIT_ORDER_LSB        1
#define BIT_ORDER_MSB        2

#define NRZ_NORMAL        1
#define NRZ_INVERTED        2

#ifndef SERIAL_RB_BUFSZ
#define SERIAL_RB_BUFSZ        128
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

/************************Driver Posix Transform***********************/
enum IoctlDriverType
{
    SERIAL_TYPE = 0,
    SPI_TYPE,
    I2C_TYPE,
    DEFAULT_TYPE,
}

struct PrivIoctlCfg
{
    enum IoctlDriverType ioctl_driver_type;
    void *args;
}

int PrivOpen(const char *path, int flags, ...);

int PrivClose(int fd, void *buf, size_t len);

int PrivRead(int fd, void *buf, size_t len);

int PrivWrite(int fd, const void *buf, size_t len);

int PrivIoctl(int fd, int cmd, void *args);

int PrivMutexCreate(pthread_mutex_t *p_mutex, const pthread_mutexattr_t *attr);
int PrivMutexDelete(pthread_mutex_t *p_mutex);
int PrivMutexObtain(pthread_mutex_t *p_mutex);
int PrivMutexAbandon(pthread_mutex_t *p_mutex);
int PrivSemaphoreCreate(sem_t *sem, int pshared, unsigned int value);
int PrivSemaphoreDelete(sem_t *sem);
int PrivSemaphoreObtainWait(sem_t *sem, const struct timespec *abstime);
int PrivSemaphoreObtainNoWait(sem_t *sem);
int PrivSemaphoreAbandon(sem_t *sem);
int PrivTaskCreate(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
int PrivTaskStartup(pthread_t *thread);
int PrivTaskDelete(pthread_t thread, int sig);
void PrivTaskQuit(void *value_ptr);
int PrivTaskDelay(const struct timespec *rqtp, struct timespec *rmtp);

#endif
