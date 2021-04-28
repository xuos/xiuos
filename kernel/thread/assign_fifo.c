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
* @brief:   ready table and timeslice functions definitions of fifo scheduler algorithm
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

#define NO_TIMESLICE    (0)

void FifoTaskTimesliceInit(struct TaskDescriptor *task)
{
    NULL_PARAM_CHECK(task);

    task->task_dync_sched_member.origin_timeslice = NO_TIMESLICE;
	task->task_dync_sched_member.rest_timeslice = NO_TIMESLICE;
}


void FifoReadyVectorInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table)
{
    DoubleLinklistType* tail = NONE;
    
    NULL_PARAM_CHECK(task);
    NULL_PARAM_CHECK(ready_table);

    tail = ready_table->priority_ready_vector[task->task_dync_sched_member.cur_prio].node_prev;
	DoubleLinkListInsertNodeAfter(tail, &(task->task_dync_sched_member.sched_link));

}


void FifoTaskTimesliceUpdate()
{
    return;
}
