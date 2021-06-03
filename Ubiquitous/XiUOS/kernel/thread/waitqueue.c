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
* @file:    waitqueue.c
* @brief:   waitqueue file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <stdint.h>

#include <xs_isr.h>
#include <device.h>
#include <xs_klist.h>
#include <xs_memory.h>
/**
 * This function will add a waitqueue node to the wait queue list
 *@parm queue waitqueue descriptor
 *@parm work the node need to be added
 *
 */
void WqueueAdd(WaitQueueType *queue, struct WaitqueueNode *node)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(queue);
    NULL_PARAM_CHECK(node);

    lock = CriticalAreaLock();
    DoubleLinkListInsertNodeBefore(&(queue->block_threads_list), &(node->list));
    CriticalAreaUnLock(lock);
}
/**
 * This function will remove a waitqueue node from the wait queue list
 *@parm work the node need to be removed
 *
 */
void WqueueRemove(struct WaitqueueNode *node)
{
    x_base lock = 0;
    NULL_PARAM_CHECK(node);

    lock = CriticalAreaLock();
    DoubleLinkListRmNode(&(node->list));
    CriticalAreaUnLock(lock);
}

/**
 * This function restarts the suspended task of the queue
 * @parm waitqueue pointer
 * @parm the awake key 
 */
void WakeupWqueue(WaitQueueType *queue, void *key)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(queue);
    NULL_PARAM_CHECK(key);

    lock = CriticalAreaLock();
    struct SysDoubleLinklistNode *node = NONE;
    DOUBLE_LINKLIST_FOR_EACH(node, &(queue->block_threads_list)) {
        struct WaitqueueNode *pEnter;
        pEnter = SYS_DOUBLE_LINKLIST_ENTRY(node, struct WaitqueueNode, list);
        if (pEnter->cb != NONE && pEnter->cb(pEnter, key) != 0) {
            continue;
        } else {
            KTaskWakeup(pEnter->polling_task->id.id);
        }
    }
    CriticalAreaUnLock(lock);

    if (node == &(queue->block_threads_list)) {
        return;
    }

    DO_KTASK_ASSIGN;
}
/**
 * This function will suspend current task and start up a timer
 * @parm queue waitqueue descriptor
 * @parm msec waiting time
 *
 */
int WqueueWait(WaitQueueType *queue, x_ticks_t tick)
{
    NULL_PARAM_CHECK(queue);

    if (tick == 0)
        return 0;

    struct WaitqueueNode* pin_node = x_malloc(sizeof(struct WaitqueueNode));

    // set timeout awake
    if (tick != WAITING_FOREVER) {
        pin_node->deadline_tick = CurrentTicksGain() + tick;
        KTaskSetDelay(GetKTaskDescriptor(), tick);
    } else {
        pin_node->deadline_tick = 0;
    }

    // init block node
    pin_node->polling_task = GetKTaskDescriptor();
    pin_node->key = 0;
    pin_node->cb = NONE;
    pin_node->list.node_next = &pin_node->list;
    pin_node->list.node_prev = &pin_node->list;

    // start block
    x_base lock;
    lock = CriticalAreaLock();
    WqueueAdd(queue, pin_node);
    SuspendKTask(GetKTaskDescriptor()->id.id);
    CriticalAreaUnLock(lock);

    DO_KTASK_ASSIGN;

    // end block, delete node
    WqueueRemove(pin_node);
    x_free(pin_node);

    return 0;
}

/**
 * This function will create a wait queue.
 *
 * @param queue queue waitqueue descriptor
 *
 */
void InitWqueue(WaitQueueType *queue)
{
    NULL_PARAM_CHECK(queue);

    InitDoubleLinkList(&(queue->block_threads_list));
}