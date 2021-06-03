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
* @file:    xs_mutex.h
* @brief:   function declaration and structure defintion of mutex
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_MUTEX_H
#define XS_MUTEX_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>
#include <xs_id.h>

#ifdef KERNEL_MUTEX

struct Mutex {
    struct IdNode          id;
    uint16                 val;
    uint8                  recursive_cnt;
    uint8                  origin_prio;
    struct TaskDescriptor  *holder;
    DoubleLinklistType     pend_list;
    DoubleLinklistType     link;
};

typedef struct {
    int32 (*MutexCreate)();
    void (*MutexDelete)(struct Mutex *mutex);
    int32 (*MutexObtain)(struct Mutex *mutex, int32 wait_time);
    int32 (*MutexAbandon)(struct Mutex *mutex);
} MutexDoneType;

int32 KMutexCreate();
void KMutexDelete(int32 id);
int32 KMutexObtain(int32 id, int32 wait_time);
int32 KMutexAbandon(int32 id);
#endif

#endif
