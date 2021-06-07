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
* @file:    linklist.c
* @brief:   suspend and wakeup function of task
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

/**
 * a task will be suspended to a pend list
 *
 * @param list  suspend task list
 * @param task the task descriptor need to be suspended
 * @param flag suspend flag
 *
 */
 x_err_t LinklistSuspend(DoubleLinklistType        *list,
                                       struct TaskDescriptor *task,
                                       uint8        flag)
{
    int32 ret = EOK;
    x_ubase lock = 0;
    DoubleLinklistType *node = NONE;
    DoubleLinklistType *tail = NONE;
    struct TaskDescriptor *tmp_task = NONE;

    NULL_PARAM_CHECK(list);
    NULL_PARAM_CHECK(task);

    lock = CriticalAreaLock();

    SuspendKTask(task->id.id);
    switch (flag)
    {
    case LINKLIST_FLAG_FIFO:
        DoubleLinkListInsertNodeBefore(list, &(task->task_dync_sched_member.sched_link));
        break;

    case LINKLIST_FLAG_PRIO:
        DOUBLE_LINKLIST_FOR_EACH(node,list)
        {
            tmp_task = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDescriptor, task_dync_sched_member.sched_link);
            if (task->task_dync_sched_member.cur_prio < tmp_task->task_dync_sched_member.cur_prio)
            {
                break;
            }
        }
        if(node != list) {
            DoubleLinkListInsertNodeBefore(&(tmp_task->task_dync_sched_member.sched_link), &(task->task_dync_sched_member.sched_link));
        } else {
            tail = list->node_prev;
            DoubleLinkListInsertNodeAfter(tail,&(task->task_dync_sched_member.sched_link));
        }       
        break;

    default:
        ret = -EINVALED;
        break;
    }
    CriticalAreaUnLock(lock);
    return ret;
}

/**
 * resume the first task in the suspend list 
 *
 * @param list  task list
 *
 */
 x_err_t LinklistResume(DoubleLinklistType *list)
{
    x_ubase lock = 0;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(list);

    lock = CriticalAreaLock();

    task = SYS_DOUBLE_LINKLIST_ENTRY(list->node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
    SYS_KDEBUG_LOG(KDBG_IPC, ("resume task:%s\n", task->task_base_info.name));

    KTaskWakeup(task->id.id);

    CriticalAreaUnLock(lock);

    return EOK;
}

/**
 * wakeup all the task in the suspend list 
 *
 * @param list  task list
 *
 */
 x_err_t LinklistResumeAll(DoubleLinklistType *list)
{
    x_ubase lock = 0;
    struct TaskDescriptor *task = NONE;
    DoubleLinklistType *node = NONE;

    NULL_PARAM_CHECK(list);

    lock = CriticalAreaLock();
    for(;;) {
        node = list->node_next;
        if(node != list) {
            task = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDescriptor, task_dync_sched_member.sched_link);
            task->exstatus = -ERROR;
            KTaskWakeup(task->id.id);
        } else {
            break;
        }
    }
    CriticalAreaUnLock(lock);

    return EOK;
}
