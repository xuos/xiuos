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
* @file:    pthread_arm.h
* @brief:   the attribute definition of posix pthread for arm
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef PTHREAD_ARM_H
#define PTHREAD_ARM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/time.h>

typedef int   pid_t;
typedef unsigned long int pthread_t; 

struct sched_param {
    int    sched_priority; /* process execution scheduling priority */
    size_t slice;          /* time slice in SCHED_RR mode (ms) */
};
typedef struct pthread_attr {
    unsigned char      is_initialized;  /* if the attr is initialized set to 1, otherwise set to 0 */
    void              *stackaddr;       /* the start addr of the stack of the pthead */
    size_t             stacksize;       /* the size of the stack of the pthead */
    unsigned char      contentionscope; /* the scope of contention, only PTHREAD_SCOPE_SYSTEM is supported */
    unsigned char      inheritsched;    /* when set to PTHREAD_INHERIT_SCHED, specifies that the thread scheduling attributes
                                           shall be inherited from the creating thread, and the scheduling attributes in this
                                           attr argument shall be ignored */
    unsigned char      schedpolicy;     /* the sched policy of the thread */
    struct sched_param schedparam;      /* the parameter of the thread scheduling */
    size_t             guardsize;       /* guardsize is set to protect the stack, not supported */
    unsigned char      detachstate;     /* when set to PTHREAD_CREATE_JOINABLE, thread will not end untill the creating thread end */
} pthread_attr_t;

typedef struct pthread_mutexattr {
    int is_initialized;
    int type;
    int protocol;
    int prioceiling;
    int pshared;
} pthread_mutexattr_t;

typedef int pthread_mutex_t ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PTHREAD_H */