/*
 * Copyright (c) 2020 AIIT XUOS Lab
 * XiUOS  is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *        http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

/**
* @file:    semaphore.c
* @brief:   posix api of semphore
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "include/semaphore.h"
#include <time.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    int32 ret = 0;
    ret = UserSemaphoreCreate(value);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

sem_t *sem_open(const char *name, int oflag, ...)
{
    return 0;
}

int sem_post(sem_t *sem)
{
    int ret;

    ret = UserSemaphoreAbandon(*sem);
    return ret;
}

int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
    int ret ;
    int ticks = -1 ;

    if (abstime != NULL) {
       ticks = abstime->tv_sec * 1000 +
            (abstime->tv_nsec / 1000000 ) ;
    }

    ret = UserSemaphoreObtain(*sem, ticks);
    return ret;
}

int sem_trywait(sem_t *sem)
{
    int ret ;
    ret = KSemaphoreObtain(*sem, 0);
    return ret;
}

int sem_unlink(const char *name)
{
    return 0;
}

int sem_wait(sem_t *sem)
{
    int ret ;
    ret = KSemaphoreObtain(*sem, -1);
    return ret;
}

int sem_getvalue(sem_t *sem, int *sval)
{
    return 0;
}

int sem_close(sem_t *sem)
{
    return 0;
}

int sem_destroy(sem_t *sem)
{
    UserSemaphoreDelete(*sem);
    return 0;
}

