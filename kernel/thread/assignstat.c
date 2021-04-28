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
* @file:    assignstat.c
* @brief:   check system read ready and task stack
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_assign.h>

#ifdef KERNEL_STACK_OVERFLOW_CHECK
/**
 * a task stack check function
 *
 * @param task task descriptor
 */
void _KTaskOsAssignStackCheck(struct TaskDescriptor *task)
{
	NULL_PARAM_CHECK(task);

    if ((x_ubase)task->stack_point<= (x_ubase)task->task_base_info.stack_start ||
        (x_ubase)task->stack_point >
        (x_ubase)task->task_base_info.stack_start + (x_ubase)task->task_base_info.stack_depth) {
       
        KPrintf("task name:%s id:%d stack overflow,sp %p stacktop %p stack depth 0x%x\n", task->task_base_info.name,task->id.id,task->stack_point,(uint32*)task->task_base_info.stack_start + task->task_base_info.stack_depth,task->task_base_info.stack_depth);

        while (1);
    }
}
#endif

int32 JudgeAssignReadyBitmapIsEmpty(struct OsAssignReadyVector *ready_vector)
{
    NULL_PARAM_CHECK(ready_vector);
    return ((ready_vector->priority_ready_group == 0) ? RET_TRUE: 0);
}

struct TaskDescriptor * ChooseTaskWithHighestPrio(struct OsAssignReadyVector *ready_vector)
{
    struct TaskDescriptor *TargetTask = NONE;

    NULL_PARAM_CHECK(ready_vector);

    TargetTask = SYS_DOUBLE_LINKLIST_ENTRY(ready_vector->priority_ready_vector[ready_vector->highest_prio].node_next,
                                  struct TaskDescriptor,
                                  task_dync_sched_member.sched_link);

    return TargetTask;
}

void OsAssignReadyVectorInit(struct OsAssignReadyVector *ready_vector)
{
    register x_base prio = 0;
    NULL_PARAM_CHECK(ready_vector);

    while(prio < KTASK_PRIORITY_MAX)
    {
        InitDoubleLinkList(&ready_vector->priority_ready_vector[prio]);
        prio++;
    }
    ready_vector->highest_prio = 0;
    ready_vector->priority_ready_group = 0;
#if KTASK_PRIORITY_MAX > 32
        memset(ready_vector->ready_vector, 0, sizeof(ready_vector->ready_vector));
#endif
}

/*
 * This function will insert a task to system ready table according to sched policy(FIFO/RR/RR+remain timeslice)
 *
 * @param task task descriptor
 * @param ready_table task ready_table
 */
void AssignPolicyInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table)
{
	DoubleLinklistType* node = NONE;
	DoubleLinklistType* tail = NONE;
	struct TaskDescriptor *tmp = NONE;

	NULL_PARAM_CHECK(task);
    NULL_PARAM_CHECK(ready_table);

#if defined (SCHED_POLICY_FIFO)
    FifoReadyVectorInsert(task, ready_table);
#elif defined (SCHED_POLICY_RR)
    RoundRobinReadyVectorInsert(task, ready_table);
#elif defined (SCHED_POLICY_RR_REMAINSLICE)
    RoundRobinRemainReadyVectorInsert(task, ready_table);
#endif

    if(NONE == task || NONE == ready_table) {
		KPrintf("PARAM CHECK FAILED ...%s %d param is NULL.\n",__FUNCTION__,__LINE__);
		return;
	}

    if(task->task_dync_sched_member.cur_prio > ready_table->highest_prio) {
		ready_table->highest_prio = task->task_dync_sched_member.cur_prio;
	}
}