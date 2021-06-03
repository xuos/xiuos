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
* @file:    xs_sem.h
* @brief:   function declaration and structure defintion of sem
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/12
*
*/

#ifndef XS_SEM_H
#define XS_SEM_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>
#include <xs_id.h>

#ifdef KERNEL_SEMAPHORE

struct Semaphore
{
    struct IdNode       id;
    uint16              value;

    DoubleLinklistType  pend_list;
    DoubleLinklistType  link;
};

typedef struct {
    int32 (*SemaphoreCreate)(uint16 val);
    void  (*SemaphoreDelete)(struct Semaphore *sem);
    int32 (*SemaphoreObtain)(struct Semaphore *sem, int32 wait_time);
    int32 (*SemaphoreAbandon)(struct Semaphore *sem);
    int32 (*SemaphoreSetValue)(struct Semaphore *sem, uint16 val);
} SemaphoreDoneType;

int32 KSemaphoreCreate(uint16 val);
void  KSemaphoreDelete(int32 id);
int32 KSemaphoreObtain(int32 id, int32 wait_time);
int32 KSemaphoreAbandon(int32 id);
int32 KSemaphoreSetValue(int32 id, uint16 val);

#endif
#endif
