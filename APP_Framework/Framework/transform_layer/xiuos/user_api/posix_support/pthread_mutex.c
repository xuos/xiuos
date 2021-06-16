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
* @file:    pthread_mutex.c
* @brief:   posix api of mutex
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include <time.h>
#include "include/pthread.h"

int pthread_mutex_init(pthread_mutex_t *p_mutex, const pthread_mutexattr_t *attr)
{
    *p_mutex = UserMutexCreate();
    if (*p_mutex < 0) {
        return -1;
    } 
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *p_mutex)
{
     UserMutexDelete(*p_mutex);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *p_mutex)
{
    int ret = -1;


    ret = UserMutexObtain(*p_mutex, WAITING_FOREVER);
    
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *p_mutex)
{
    int ret = -1;

    ret = UserMutexAbandon( *p_mutex );
    return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *p_mutex)
{
    int ret = -1;
    ret = UserMutexObtain( *p_mutex , 0);
    return ret;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict attr, int *restrict protocol)
{

    return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    return 0;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *restrict mutex, int *restrict prioceiling)
{
    return 0;
}

int pthread_mutex_setprioceiling(pthread_mutex_t *restrict mutex, int prioceiling, int *restrict old_ceiling)
{
    return 0;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict attr, int *restrict prioceiling)
{
    return 0;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
    return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    return 0;
}

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
{
    return 0;
}

