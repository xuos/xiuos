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
* @file:    delay.c
* @brief:   set task delay and check the timeout
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_klist.h>
#include<string.h>
#include<xs_isr.h>
#include <xs_ktask.h>
#include <xs_memory.h>
#include<xs_ktick.h>
#include<xiuos.h>

DoubleLinklistType xiaoshan_delay_head = {&xiaoshan_delay_head, &xiaoshan_delay_head};

/**
 * This function will delay a task with sone ticks.
 *
 * @param task the task needed to be delayed
 * @param ticks delay timeout
 *
 * @return EOK on success; ENOMEMORY on failure
 */
x_err_t KTaskSetDelay(KTaskDescriptorType task, x_ticks_t ticks)
{
	x_base lock = 0;
    struct Delay *delay = NONE;
    struct Delay *dnode = NONE;
    DoubleLinklistType *nodelink = NONE;

    NULL_PARAM_CHECK(task);

    if (task->task_dync_sched_member.delay == NONE) {
	   delay = (struct Delay *)x_malloc(sizeof(struct Delay));
       if (delay == NONE)
       {
           return -ENOMEMORY;
       }
       memset(delay, 0x0, sizeof(struct Delay));
       task->task_dync_sched_member.delay = delay;
    } else {
        delay = task->task_dync_sched_member.delay;
        lock = CriticalAreaLock();
        if(delay->status == TASK_DELAY_ACTIVE){
            delay->status = TASK_DELAY_INACTIVE;
            DoubleLinkListRmNode(&(delay->link));
        }
        CriticalAreaUnLock(lock);
    }

	delay->ticks = CurrentTicksGain() + ticks;
    delay->task = task;
    delay->status = TASK_DELAY_ACTIVE;

	lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &xiaoshan_delay_head) {
        dnode =CONTAINER_OF(nodelink, struct Delay, link);
        if (delay->ticks < dnode->ticks) {
            DoubleLinkListInsertNodeBefore(nodelink, &delay->link);

            break;
        }
    }

    if (nodelink == &xiaoshan_delay_head) {
        DoubleLinkListInsertNodeBefore(&xiaoshan_delay_head, &(delay->link));
    }

    CriticalAreaUnLock(lock);

	return EOK;
}

/**
 * This function will wakeup a task from delay list.
 *
 * @param task the task needed to be wakeup
 *
 * @return EOK 
 */
x_err_t KTaskUnSetDelay(KTaskDescriptorType task)
{
	x_base lock = 0;
    struct Delay *delay = NONE;

	NULL_PARAM_CHECK(task);

	delay = task->task_dync_sched_member.delay;
   
	if( delay != NONE && delay->status == TASK_DELAY_ACTIVE) {
	    lock = CriticalAreaLock();
        delay->status = TASK_DELAY_INACTIVE;
        DoubleLinkListRmNode(&(delay->link));
        CriticalAreaUnLock(lock);
	}

	return EOK;
}


void CheckTaskDelay(void)
{ 
    x_base level = 0;
    x_ticks_t current_tick = 0;
    struct Delay *node = NONE;

    SYS_KDEBUG_LOG(0, ("delay check enter\n"));

    current_tick = CurrentTicksGain();

    level = CriticalAreaLock();

    while (!IsDoubleLinkListEmpty(&xiaoshan_delay_head))
    {
        node = SYS_DOUBLE_LINKLIST_ENTRY(xiaoshan_delay_head.node_next,
                                     struct Delay, link);

        if ((current_tick - node->ticks) < TICK_SIZE_MAX / 2)
        {
            current_tick = CurrentTicksGain();
            SYS_KDEBUG_LOG(KDBG_SOFTTIMER, ("current tick: %d\n", current_tick));
            KTaskUnSetDelay(node->task);
            CriticalAreaUnLock(level);
			KTaskTimeout(node->task);
            level = CriticalAreaLock();
        }
        else
            break;
    }

    CriticalAreaUnLock(level);

    SYS_KDEBUG_LOG(0, ("delay check leave\n"));
}