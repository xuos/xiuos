
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
* @file:    zombierecycle.c
* @brief:   a recycle task of system
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#ifdef TASK_ISOLATION
#include <xs_isolation.h>
#endif

DoubleLinklistType KTaskZombie;

#ifndef ZOMBIE_KTASK_STACKSIZE
#define ZOMBIE_KTASK_STACKSIZE 2048
#endif

 int32 zombie_recycle;

int JudgeZombieKTaskIsNotEmpty(void)
{
    return (IsDoubleLinkListEmpty(&KTaskZombie) ? 0 : RET_TRUE);
}

static void ZombieKTaskEntry(void *parameter)
{
    x_base lock = 0;
    KTaskDescriptorType task = NONE;

    while(RET_TRUE)
    {
        KDEBUG_NOT_IN_INTERRUPT;
        lock = CriticalAreaLock();
        if (JudgeZombieKTaskIsNotEmpty()) {
            task = SYS_DOUBLE_LINKLIST_ENTRY(KTaskZombie.node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
            DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
            CriticalAreaUnLock(lock);
#ifdef SEPARATE_COMPILE
            if(1 == task->task_dync_sched_member.isolation_flag ){
#ifdef MOMERY_PROTECT_ENABLE
               if(mem_access.Free)
                 mem_access.Free(task->task_dync_sched_member.isolation);
#endif
                x_ufree(task->task_base_info.stack_start);
            } else
#endif
            {
                KERNEL_FREE(task->task_base_info.stack_start);
            }

            DoubleLinkListRmNode(&(task->link));

            KTaskIdDelete(task->id.id);

            if(task->task_dync_sched_member.delay != NONE){
                KERNEL_FREE(task->task_dync_sched_member.delay);
            }
            KERNEL_FREE(task);
        } else {
            SuspendKTask(zombie_recycle);
            CriticalAreaUnLock(lock);
            DO_KTASK_ASSIGN;
        }  
    }
}

void ZombieTaskRecycleInit(void)
{
    InitDoubleLinkList(&KTaskZombie);
    
    zombie_recycle = KTaskCreate("ZombieRecycleKTask",
                         ZombieKTaskEntry,
                         NONE,
                         ZOMBIE_KTASK_STACKSIZE,
                         KTASK_LOWEST_PRIORITY + 1);

    StartupKTask(zombie_recycle);
}