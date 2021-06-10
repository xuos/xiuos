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
 * @file xiuos.c
 * @brief Converts the framework interface to an operating system interface
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.06.07
 */

#include "transform.h"

/**************************mutex***************************/

int32_t PrivMutexCreate(void)
{
    return 0;
}

void PrivMutexDelete(int32_t mutex)
{
    return;
}

int32_t PrivMutexObtain(int32_t mutex, int32_t wait_time)
{
    return 0;
}

int32_t PrivMutexAbandon(int32_t mutex)
{
    return 0;
}

/**********************semaphore****************************/

int32_t PrivSemaphoreCreate(uint16_t val)
{
    return 0;
}

int32_t PrivSemaphoreDelete(int32_t sem)
{
    return 0;
}

int32_t PrivSemaphoreObtain(int32_t sem, int32_t wait_time)
{
    return 0;
}

int32_t PrivSemaphoreAbandon(int32_t sem)
{
    return 0;
}

int32_t PrivSemaphoreSetValue(int32_t sem, uint16_t val)
{
    return 0;
}

/**************************task*************************/

int32_t PrivTaskCreate(UtaskType utask)
{
    return 0;
}

int32_t PrivTaskStartup(int32_t id)
{
    return 0;
}

int32_t PrivTaskDelete(int32_t id)
{
    return 0;
}

void PrivTaskQuit(void)
{
    return;
}

int32_t PrivTaskDelay(int32_t ms)
{
    return 0;
}

/*********************fs**************************/

int PrivOpen(const char *path, int flags, ...)
{
    return 0;
}

int PrivRead(int fd, void *buf, size_t len)
{
    return 0;
}

int PrivWrite(int fd, const void *buf, size_t len)
{
    return 0;
}

int PrivClose(int fd)
{
    return 0;
}

int PrivIoctl(int fd, int cmd, void *args)
{
    return 0;
}
