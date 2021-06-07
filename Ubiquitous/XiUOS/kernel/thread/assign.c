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
* @file:    assign.c
* @brief:   system scheduler of single cpu
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <xs_assign.h>
#include <xs_ktask_stat.h>
#include <xs_assign.h>
#include <xs_hook.h>

struct Assign Assign;

static struct PriorityReadyVectorDone SingleReadyVectorDone =
{
    OsAssignReadyVectorInit,
    KTaskInsertToReadyVector,
    KTaskOsAssignRemoveKTask,
};

/**
 * task schedule function.getting the highest priority task then switching to it
 */
void KTaskOsAssign(void)
{
    
    x_ubase highest_prio = 0;
    int need_insert_from_task = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *from_task = NONE;

    if(GetOsAssignLockLevel() >= 1) {
        return;
    }

    if( isrManager.done->isInIsr() ){
        isrManager.done->setSwitchTrigerFlag();
        return;
    }

    /* if the bitmap is empty then do not switch */
    if(RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) {
        return;
    }

    highest_prio = Assign.os_assign_read_vector.highest_prio;
    new_task = ChooseTaskWithHighestPrio(&Assign.os_assign_read_vector);

    if(RET_TRUE != JudgeKTaskStatIsRunning(Assign.os_running_task)) {
        CHECK(NONE != new_task);
        goto SWITCH;
    }

    /* if the running task’ priority is the highest and this task is not be yield then do not switch */
    if(highest_prio < Assign.os_running_task->task_dync_sched_member.cur_prio) {
        return;
    } else {
        need_insert_from_task = 1;
    }
    
SWITCH:
    if (new_task != Assign.os_running_task) {
        Assign.current_priority = (uint8)highest_prio;
        from_task = Assign.os_running_task;

        HOOK(hook.assign.hook_Assign, (from_task, new_task));

        if (need_insert_from_task) {
            Assign.ready_vector_done->insert(from_task);
        }

        Assign.ready_vector_done->remove(new_task);
        KTaskStatSetAsRunning(new_task);

        SYS_KDEBUG_LOG(KDBG_SCHED,
                ("[%d]switch to priority#%d "
                    "task:%.*s(sp:0x%08x), "
                    "from task:%.*s(sp: 0x%08x)\n",
                    isrManager.done->getCounter(), highest_prio,
                    NAME_NUM_MAX, new_task->task_base_info.name, new_task->stack_point,
                    NAME_NUM_MAX, from_task->task_base_info.name, from_task->stack_point));

#ifdef KERNEL_STACK_OVERFLOW_CHECK
        _KTaskOsAssignStackCheck(new_task);
#endif


        SwitchKtaskContext((x_ubase)&from_task->stack_point,
                    (x_ubase)&new_task->stack_point, new_task);

    } else {
        Assign.ready_vector_done->remove(Assign.os_running_task);
        KTaskStatSetAsRunning(Assign.os_running_task);
    }
    return;
}

void KTaskOsAssignDoIrqSwitch(void *context)
{
    
    x_ubase highest_prio = 0;
    int need_insert_from_task = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *from_task = NONE;
    
    if(GetOsAssignLockLevel() >= 1) {
        return;
    }

    if( isrManager.done->getSwitchTrigerFlag() == 0 ) {
        return;
    }
    
    isrManager.done->clearSwitchTrigerFlag();
    /* if the bitmap is empty then do not switch */
    if(RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) {
        return;
    }

    highest_prio = Assign.os_assign_read_vector.highest_prio;
    new_task = ChooseTaskWithHighestPrio(&Assign.os_assign_read_vector);

    if(RET_TRUE != JudgeKTaskStatIsRunning(Assign.os_running_task)) {
        CHECK(NONE != new_task);
        goto SWITCH;
    }

    /* if the running task’ priority is the highest and this task is not be yield then do not switch */
    if(highest_prio < Assign.os_running_task->task_dync_sched_member.cur_prio) {
        return;
    } else {
        need_insert_from_task = 1;
    }

SWITCH:

    if (new_task != Assign.os_running_task) {
        Assign.current_priority = (uint8)highest_prio;
        from_task = Assign.os_running_task;

        HOOK(hook.assign.hook_Assign, (from_task, new_task));

        if (need_insert_from_task) {
            Assign.ready_vector_done->insert(from_task);
        }

        Assign.ready_vector_done->remove(new_task);
        KTaskStatSetAsRunning(new_task);

        SYS_KDEBUG_LOG(KDBG_SCHED,
                ("[%d]switch to priority#%d "
                    "task:%.*s(sp:0x%08x), "
                    "from task:%.*s(sp: 0x%08x)\n",
                    isrManager.done->getCounter(), highest_prio,
                    NAME_NUM_MAX, new_task->task_base_info.name, new_task->stack_point,
                    NAME_NUM_MAX, from_task->task_base_info.name, from_task->stack_point));

#ifdef KERNEL_STACK_OVERFLOW_CHECK
        _KTaskOsAssignStackCheck(new_task);
#endif


        SYS_KDEBUG_LOG(KDBG_SCHED, ("switch in interrupt\n"));
        HwInterruptcontextSwitch((x_ubase)&from_task->stack_point,
                    (x_ubase)&new_task->stack_point, new_task, context);

    } else {
        Assign.ready_vector_done->remove(Assign.os_running_task);
        KTaskStatSetAsRunning(Assign.os_running_task);
    }
}

void KTaskOsAssignAfterIrq(void *context)
{
    x_base lock = 0;                                 
    lock = DISABLE_INTERRUPT();                  
    KTaskOsAssignDoIrqSwitch(context);
    ENABLE_INTERRUPT(lock);
}
/*
 * insert a ready task to system ready table with READY state and remove it from suspend list
 *
 * @param task the task descriptor
 *
 */
void KTaskInsertToReadyVector(struct TaskDescriptor *task)
{
    x_base lock = 0;

	NULL_PARAM_CHECK(task);

    lock = DISABLE_INTERRUPT();

    KTaskStatSetAsReady(task);
	AssignPolicyInsert(task, &Assign.os_assign_read_vector);

    SYS_KDEBUG_LOG(KDBG_SCHED, ("insert task[%.*s], the priority: %d\n",
                                      NAME_NUM_MAX, task->task_base_info.name, task->task_dync_sched_member.cur_prio));

#if KTASK_PRIORITY_MAX > 32
    MERGE_FLAG(&Assign.os_assign_read_vector.ready_vector[task->task_dync_sched_member.bitmap_offset], task->task_dync_sched_member.bitmap_row);
#endif
    MERGE_FLAG(&Assign.os_assign_read_vector.priority_ready_group, task->task_dync_sched_member.bitmap_column);

    ENABLE_INTERRUPT(lock);
}

/*
 * a task will be removed from ready table.
 *
 * @param task  task descriptor
 *
 */
void KTaskOsAssignRemoveKTask(struct TaskDescriptor *task)
{
    x_base lock = 0;
    x_ubase number = 0;
    x_ubase highest_priority = 0;

	NULL_PARAM_CHECK(task);

    lock = DISABLE_INTERRUPT();

    SYS_KDEBUG_LOG(KDBG_SCHED, ("remove task[%.*s], the priority: %d\n",
                                      NAME_NUM_MAX, task->task_base_info.name,
                                      task->task_dync_sched_member.cur_prio));

    DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
    
    if (IsDoubleLinkListEmpty(&(Assign.os_assign_read_vector.priority_ready_vector[task->task_dync_sched_member.cur_prio]))) {
#if KTASK_PRIORITY_MAX > 32
        CLEAR_FLAG(&Assign.os_assign_read_vector.ready_vector[task->task_dync_sched_member.bitmap_offset], task->task_dync_sched_member.bitmap_row);
        if (Assign.os_assign_read_vector.ready_vector[task->task_dync_sched_member.bitmap_offset] == 0) {
            CLEAR_FLAG(&Assign.os_assign_read_vector.priority_ready_group, task->task_dync_sched_member.bitmap_column);
        }
        number = PrioCaculate(Assign.os_assign_read_vector.priority_ready_group);
        highest_priority = (number * 8) + PrioCaculate(Assign.os_assign_read_vector.ready_vector[number]);
#else
        CLEAR_FLAG(&Assign.os_assign_read_vector.priority_ready_group, task->task_dync_sched_member.bitmap_column);
        highest_priority = PrioCaculate(Assign.os_assign_read_vector.priority_ready_group);
#endif
        Assign.os_assign_read_vector.highest_prio = highest_priority;
    }

    ENABLE_INTERRUPT(lock);
}


static inline void SwitchToFirstRunningTask(struct TaskDescriptor* task)
{
    NULL_PARAM_CHECK(task);

    Assign.ready_vector_done->remove(task);
    KTaskStatSetAsRunning(task);
    SwitchKtaskContextTo((x_ubase)&task->stack_point, task);
}

static inline void SetSystemRunningTask(struct TaskDescriptor* task)
{
    NULL_PARAM_CHECK(task);

    Assign.os_running_task = task;
}

/**
 * This function will yield current task.
 *
 * @return EOK
 */

x_err_t YieldOsAssign(void)
{
    x_base lock = 0;
    x_ubase highest_prio = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *from_task = NONE;

    lock = DISABLE_INTERRUPT();

    if(GetOsAssignLockLevel() >= 1) {
        ENABLE_INTERRUPT(lock);
        goto __exit;
    }

    if (isrManager.done->isInIsr()) {
        ENABLE_INTERRUPT(lock);
        goto __exit;
    }

    /* if the bitmap is empty then do not switch */
    if(RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) {
        ENABLE_INTERRUPT(lock);
        return -ERROR;
    }

    highest_prio = Assign.os_assign_read_vector.highest_prio;
    new_task = ChooseTaskWithHighestPrio(&Assign.os_assign_read_vector);

    from_task = Assign.os_running_task;
    
    if(RET_TRUE != JudgeKTaskStatIsRunning(from_task)) {
        CHECK(NONE != new_task);
    } else {
        Assign.ready_vector_done->insert(from_task);
    }

    if (new_task != from_task) {
        Assign.current_priority = (uint8)highest_prio;

        HOOK(hook.assign.hook_Assign, (from_task, new_task));

        Assign.ready_vector_done->remove(new_task);
        KTaskStatSetAsRunning(new_task);

        SYS_KDEBUG_LOG(KDBG_SCHED,
                ("[%d]switch to priority#%d "
                    "task:%.*s(sp:0x%08x), "
                    "from task:%.*s(sp: 0x%08x)\n",
                    isrManager.done->getCounter(), highest_prio,
                    NAME_NUM_MAX, new_task->task_base_info.name, new_task->stack_point,
                    NAME_NUM_MAX, from_task->task_base_info.name, from_task->stack_point));

#ifdef KERNEL_STACK_OVERFLOW_CHECK
        _KTaskOsAssignStackCheck(new_task);
#endif

        
        SwitchKtaskContext((x_ubase)&from_task->stack_point,
                (x_ubase)&new_task->stack_point, new_task);

        ENABLE_INTERRUPT(lock);
        goto __exit;
    } else {
        Assign.ready_vector_done->remove(Assign.os_running_task);
        KTaskStatSetAsRunning(Assign.os_running_task);
    }

    ENABLE_INTERRUPT(lock);

__exit:
    return EOK;
}

/**
 * 
 * OsAssign startup function
 * .
 */
void StartupOsAssign(void)
{
    struct TaskDescriptor *FirstRunningTask = NONE;

    FirstRunningTask = ChooseTaskWithHighestPrio(&Assign.os_assign_read_vector);

    SetSystemRunningTask(FirstRunningTask);

    SwitchToFirstRunningTask(FirstRunningTask);
}

/**
 *
 * system OsAssign init function
 */
void SysInitOsAssign(void)
{
    SYS_KDEBUG_LOG(KDBG_SCHED, ("start Os Assign: max priority 0x%02x\n",
                                      KTASK_PRIORITY_MAX));

    Assign.ready_vector_done = &SingleReadyVectorDone;
    Assign.ready_vector_done->init(&Assign.os_assign_read_vector);

    ResetCriticalAreaLock();
}