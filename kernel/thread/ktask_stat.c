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
* @file:    ktask_stat.c
* @brief:   the stat function definition of task
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#ifndef XS_KTASK_STAT_H
#define XS_KTASK_STAT_H

#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_ktask.h>
#include <stdbool.h>
#include <xs_ktask_stat.h>

void KTaskStateSet(KTaskDescriptorType task, uint8 stat)
{
    NULL_PARAM_CHECK(task);
	task->task_dync_sched_member.stat = stat | (task->task_dync_sched_member.stat & ~KTASK_STAT_MASK);
}

void KTaskStatSetAsInit(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
	KTaskStateSet(task, KTASK_INIT);
}

void KTaskStatSetAsReady(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
	KTaskStateSet(task, KTASK_READY);
}

void KTaskStatSetAsSuspend(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
	KTaskStateSet(task, KTASK_SUSPEND);
}

void KTaskStatSetAsRunning(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
	KTaskStateSet(task, KTASK_RUNNING);
}

void KTaskStatSetAsClose(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
	KTaskStateSet(task, KTASK_CLOSE);
}

uint8 KTaskStatGet(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return task->task_dync_sched_member.stat & KTASK_STAT_MASK;
}

bool JudgeKTaskStatIsInit(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return ((task->task_dync_sched_member.stat & KTASK_STAT_MASK) == KTASK_INIT);
}

bool JudgeKTaskStatIsReady(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return ((task->task_dync_sched_member.stat & KTASK_STAT_MASK) == KTASK_READY);
}

bool JudgeKTaskStatIsSuspend(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return ((task->task_dync_sched_member.stat & KTASK_STAT_MASK) == KTASK_SUSPEND);
}

bool JudgeKTaskStatIsRunning(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return ((task->task_dync_sched_member.stat & KTASK_STAT_MASK) == KTASK_RUNNING);
}

bool JudgeKTaskStatIsClose(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    return ((task->task_dync_sched_member.stat & KTASK_STAT_MASK) == KTASK_CLOSE);
}

#endif


