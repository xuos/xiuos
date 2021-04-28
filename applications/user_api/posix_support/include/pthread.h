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
* @file:    pthread.h
* @brief:   the attribute definition of posix pthread
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef PTHREAD_H
#define PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../../switch_api/user_api.h"
#include <time.h>
#include <sys/time.h>

#if defined(ARCH_ARM)
#include "pthread arm.h"
#endif

// enum {
//     PTHREAD_BARRIER_SERIAL_THREAD,
//     PTHREAD_CANCEL_ASYNCHRONOUS,
//     PTHREAD_CANCEL_ENABLE,
//     PTHREAD_CANCEL_DEFERRED,
//     PTHREAD_CANCEL_DISABLE,
//     PTHREAD_CANCELED,
//     PTHREAD_CREATE_DETACHED,
//     PTHREAD_CREATE_JOINABLE,
//     PTHREAD_EXPLICIT_SCHED,
//     PTHREAD_INHERIT_SCHED,
//     PTHREAD_MUTEX_DEFAULT,
//     PTHREAD_MUTEX_ERRORCHECK,
//     PTHREAD_MUTEX_NORMAL,
//     PTHREAD_MUTEX_RECURSIVE,
//     PTHREAD_MUTEX_ROBUST,
//     PTHREAD_MUTEX_STALLED,
//     PTHREAD_ONCE_INIT,
//     PTHREAD_PRIO_INHERIT,
//     PTHREAD_PRIO_NONE,
//     PTHREAD_PRIO_PROTECT,
//     PTHREAD_PROCESS_SHARED,
//     PTHREAD_PROCESS_PRIVATE,
//     PTHREAD_SCOPE_PROCESS,
//     PTHREAD_SCOPE_SYSTEM,
// };

typedef int   pid_t;

/* function in pthread.c */
int       pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
int       pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                         void *(*start_routine)(void *), void *arg);
void      pthread_exit(void *value_ptr);
int       pthread_detach(pthread_t thread);
int       pthread_join(pthread_t thread, void **retval);
int       pthread_cancel(pthread_t thread);
void      pthread_testcancel(void);
int       pthread_setcancelstate(int state, int *oldstate);
int       pthread_setcanceltype(int type, int *oldtype);
int       pthread_kill(pthread_t thread, int sig);
int       pthread_equal(pthread_t t1, pthread_t t2);
int       pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *pParam);
void      pthread_cleanup_pop(int execute);
void      pthread_cleanup_push(void (*routine)(void *), void *arg);
pthread_t pthread_self(void);
int       pthread_getcpuclockid(pthread_t thread_id, clockid_t *clock_id);
int       pthread_setconcurrency(int new_level);
int       pthread_getconcurrency(void);
int       pthread_setschedprio(pthread_t thread, int prio);
int       pthread_setname_np(pthread_t thread, const char *name);
int       pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

/* function in pthread_mutex.c */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif