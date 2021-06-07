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
* @file:    assign_roundrobin.c
* @brief:   ready table and timeslice functions definitions of remain ticks first scheduler algorithm
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

void RoundRobinRemainTaskTimesliceInit(struct TaskDescriptor *task)
{
    NULL_PARAM_CHECK(task);

    task->task_dync_sched_member.origin_timeslice = 10;
	task->task_dync_sched_member.rest_timeslice = 10;
}


void RoundRobinRemainReadyVectorInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table)
{
    DoubleLinklistType* node = NONE;
	DoubleLinklistType* tail = NONE;
	struct TaskDescriptor *tmp = NONE;

    NULL_PARAM_CHECK(task);
    NULL_PARAM_CHECK(ready_table);

    if ( task->task_dync_sched_member.advance_cnt < KTASK_MAX_ADVANCE_PROCESS_TIME ) {
		DOUBLE_LINKLIST_FOR_EACH(node, &ready_table->priority_ready_vector[task->task_dync_sched_member.cur_prio]) {
			tmp = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDescriptor, task_dync_sched_member.sched_link);
			if (task->task_dync_sched_member.rest_timeslice < tmp->task_dync_sched_member.rest_timeslice) {
				break;
			}
		}

		if ( node != &ready_table->priority_ready_vector[task->task_dync_sched_member.cur_prio]) {
			DoubleLinkListInsertNodeBefore(node, &(task->task_dync_sched_member.sched_link));
		} else {
			tail = ready_table->priority_ready_vector[task->task_dync_sched_member.cur_prio].node_prev;
			DoubleLinkListInsertNodeAfter(tail, &(task->task_dync_sched_member.sched_link));
		}
	} else {
		tail = ready_table->priority_ready_vector[task->task_dync_sched_member.cur_prio].node_prev;
		DoubleLinkListInsertNodeAfter(tail, &(task->task_dync_sched_member.sched_link));
	}

}

void RoundRobinRemainTaskTimesliceUpdate(struct TaskDescriptor *task)
{
    NULL_PARAM_CHECK(task);
	
    -- task->task_dync_sched_member.rest_timeslice;
    if (task->task_dync_sched_member.rest_timeslice == 0)
    {
        if ( task->task_dync_sched_member.advance_cnt < KTASK_MAX_ADVANCE_PROCESS_TIME ) {
			task->task_dync_sched_member.advance_cnt++;
		}

        task->task_dync_sched_member.rest_timeslice = task->task_dync_sched_member.origin_timeslice;

        DO_KTASK_ASSIGN;
    }
}

