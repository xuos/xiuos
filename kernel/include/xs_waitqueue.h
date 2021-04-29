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
* @file:    xs_waitqueue.h
* @brief:   function declaration and structure defintion of waitqueue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_WAITQUEUE_H
#define XS_WAITQUEUE_H

#include <xs_ktask.h>
#include <xs_timer.h>
#include <xs_queue_manager.h>

#define WQ_FLAG_CLEAN    0x00
#define WQ_FLAG_WAKEUP   0x01

struct WaitQueue
{
    DoubleLinklistType block_threads_list;
};
typedef struct WaitQueue WaitQueueType;

struct WaitqueueNode;
typedef int (*wqueue_func_t)(struct WaitqueueNode *wait, void *key);

struct WaitqueueNode
{
    KTaskDescriptorType  polling_task;
    DoubleLinklistType   list;
    wqueue_func_t        cb;
    x_ticks_t            deadline_tick;
    uint32               key;
};
typedef struct WaitqueueNode WaitqueueNodeType;

void InitWqueue(WaitQueueType *queue);
void WqueueAdd(WaitQueueType *queue, struct WaitqueueNode *node);
void WqueueRemove(struct WaitqueueNode *node);
int  WqueueWait(WaitQueueType *queue, x_ticks_t tick);
void WakeupWqueue(WaitQueueType *queue, void *key);

typedef struct
{
    void (*InitWqueue)(WaitQueueType *queue);
    void (*WqueueAdd)(WaitQueueType *queue, WaitqueueNodeType *node);
    void (*WqueueRemove)(WaitqueueNodeType *node);
    int  (*WqueueWait)(WaitQueueType *queue, x_ticks_t tick);
    void (*WakeupWqueue)(WaitQueueType *queue, void *key);
} WaitQueueDoneType;

#endif
