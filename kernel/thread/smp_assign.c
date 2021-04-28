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
* @file:    smp_assign.c
* @brief:   system scheduler of multiple cpu
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_isr.h>
#include <xs_spinlock.h>
#include <xs_ktask_stat.h>
#include <xs_assign.h>
#include <xs_hook.h>
#include <stdio.h>

struct Assign Assign;
HwSpinlock AssignSpinLock;

static struct PriorityReadyVectorDone ready_vector_done =
{
    OsAssignReadyVectorInit,
    KTaskInsertToReadyVector,
    KTaskOsAssignRemoveKTask,
};

static inline x_ubase SmpGetReadyVectorHighestPrio(void)
{
    uint8 coreid = GetCpuId();

    return ((Assign.os_assign_read_vector.highest_prio > Assign.smp_os_assign_ready_rector[coreid].highest_prio) ? Assign.os_assign_read_vector.highest_prio : Assign.smp_os_assign_ready_rector[coreid].highest_prio);
}

/*
 * get target highest priority task in ready queue
 */
static inline struct TaskDescriptor* SmpAssignTargetTaskSelect(void)
{

    uint8 coreid = GetCpuId();

    if (Assign.os_assign_read_vector.highest_prio > Assign.smp_os_assign_ready_rector[coreid].highest_prio)
    {
        return ChooseTaskWithHighestPrio(&Assign.os_assign_read_vector);
    }
    else
    {
        return ChooseTaskWithHighestPrio(&Assign.smp_os_assign_ready_rector[coreid]);
    }
}

static inline void SmpOsAssignSwtichToNewTask(struct TaskDescriptor* old_task, struct TaskDescriptor* new_task)
{
    NULL_PARAM_CHECK(old_task);
    NULL_PARAM_CHECK(new_task);

    Assign.ready_vector_done->remove(new_task);
    KTaskStatSetAsRunning(new_task);

#ifdef USING_OVERFLOW_CHECK
    _KTaskOsAssignStackCheck(new_task);
#endif

    SwitchKtaskContext((x_ubase)&old_task->stack_point, (x_ubase)&new_task->stack_point, new_task);
}

static inline void SmpSwitchToFirstRunningTask(struct TaskDescriptor* task)
{
    NULL_PARAM_CHECK(task);

    Assign.ready_vector_done->remove(task);
    KTaskStatSetAsRunning(task);
    
    SwitchKtaskContextTo((x_ubase)&task->stack_point, task);

}

static inline void SetSystemRunningTask(struct TaskDescriptor* task)
{
    NULL_PARAM_CHECK(task);

    task->task_smp_info.runing_coreid = GetCpuId();
}

static void SmpOsAssignInit(void)
{
    int coreid = 0;
    while(coreid < CPU_NUMBERS) {
        Assign.ready_vector_done->init(&Assign.smp_os_assign_ready_rector[coreid]);
        Assign.smp_os_running_task[coreid] = NONE;
        
#ifdef ARCH_SMP
        isrManager.isr_switch_trigger_flag[coreid] = 0;
#else
        isrManager.isr_switch_trigger_flag = 0;
#endif
        Assign.current_priority[coreid] = KTASK_PRIORITY_MAX - 1;
        Assign.assign_lock[coreid] = 0;
        coreid++;
    }
}

struct smp_assign_done smp_assign_done =
{
    SmpGetReadyVectorHighestPrio,
    SmpAssignTargetTaskSelect,
    SmpOsAssignSwtichToNewTask,
    SmpSwitchToFirstRunningTask,
    SetSystemRunningTask,
    SmpOsAssignInit,
};



/**
 * task schedule function.getting the highest priority task then switching to it
 */
void KTaskOsAssign(void)
{
    x_base lock = 0;
    int coreid = 0;
    x_ubase highest_prio = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *runningtask = NONE;
    
    coreid = GetCpuId();
    if (isrManager.done->isInIsr()) {
        isrManager.done->setSwitchTrigerFlag();
        return;
    }

    if(Assign.assign_lock[coreid] >= 1) {
        return;
    }
    
    runningtask = Assign.smp_os_running_task[coreid];

    /* if the bitmap is empty then do not switch */
    if((RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) &&
        (RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.smp_os_assign_ready_rector[coreid]))) {
        return;
    }

    highest_prio = Assign.smp_assign_done->GetHighest();
    new_task = Assign.smp_assign_done->select();
 

    if(RET_TRUE != JudgeKTaskStatIsRunning(runningtask)) {
        CHECK(NONE != new_task);
        goto SWITCH;
    }
    /* if the running task’ priority is the highest and this task is not be yield then do not switch */
    if(highest_prio < runningtask->task_dync_sched_member.cur_prio) {
        return;
    } else {   
        Assign.ready_vector_done->insert(runningtask);
    }
SWITCH:

    new_task->task_smp_info.runing_coreid = coreid;
    Assign.current_priority[coreid] = (uint8)highest_prio;

    HOOK(hook.assign.hook_Assign,(runningtask, new_task));

    SYS_KDEBUG_LOG(KDBG_SCHED,
            ("[%d]switch to priority#%d "
                "task:%.*s(sp:0x%08x), "
                "from task:%.*s(sp: 0x%08x)\n",
                isrManager.done->getCounter(), highest_prio,
                NAME_NUM_MAX, new_task->task_base_info.name, new_task->stack_point,
                NAME_NUM_MAX, runningtask->task_base_info.name, runningtask->stack_point));

    Assign.smp_assign_done->SwitchToNew(runningtask,new_task);
}

/**
 * task  switch in IRQ context. 
 */
void KTaskOsAssignDoIrqSwitch(void *context)
{
    int coreid = 0;
    x_base lock = 0;
    x_ubase highest_priority = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *runningtask = NONE;
    
    coreid = GetCpuId();
    if ( isrManager.done->getSwitchTrigerFlag() == 0) {
        return;
    }

    if (Assign.assign_lock[coreid] >= 1 || isrManager.done->getCounter() != 0) {
        return;
    }

    
    isrManager.done->clearSwitchTrigerFlag();

    runningtask = Assign.smp_os_running_task[coreid];

    if((RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) &&
        (RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.smp_os_assign_ready_rector[coreid]))) {
        return;
    }

    highest_priority = Assign.smp_assign_done->GetHighest();
    new_task = Assign.smp_assign_done->select();

    if (RET_TRUE == JudgeKTaskStatIsRunning(runningtask)) {
        if (runningtask->task_dync_sched_member.cur_prio > highest_priority) {
            new_task = runningtask;
        } else {
            Assign.ready_vector_done->insert(runningtask);
        }
    }

    new_task->task_smp_info.runing_coreid = coreid;
    if (new_task != runningtask) {
       Assign.current_priority[coreid] = (uint8)highest_priority;
       HOOK(hook.assign.hook_Assign, (runningtask, new_task));
       Assign.ready_vector_done->remove(new_task);
       KTaskStatSetAsRunning(new_task);

#ifdef KERNEL_STACK_OVERFLOW_CHECK
         _KTaskOsAssignStackCheck(new_task);
#endif
        SYS_KDEBUG_LOG(KDBG_SCHED, ("switch in interrupt\n"));


        HwInterruptcontextSwitch( (x_ubase)&runningtask->stack_point,
                        (x_ubase)&new_task->stack_point, new_task, context);
    }
}
void KTaskOsAssignAfterIrq(void *context)
{
    x_base lock = 0;                                 
    lock = DISABLE_INTERRUPT();                  
    HwLockSpinlock(&AssignSpinLock);
    KTaskOsAssignDoIrqSwitch(context);
    HwUnlockSpinlock(&AssignSpinLock);           
    ENABLE_INTERRUPT(lock);
}

static void UncombineInsert(struct TaskDescriptor *task)
{
    uint32 cpu_mask = 0;

    NULL_PARAM_CHECK(task);

#if KTASK_PRIORITY_MAX > 32
    MERGE_FLAG(&Assign.os_assign_read_vector.ready_vector[task->task_dync_sched_member.bitmap_offset], task->task_dync_sched_member.bitmap_row);
#endif
    MERGE_FLAG(&Assign.os_assign_read_vector.priority_ready_group, task->task_dync_sched_member.bitmap_column);
    AssignPolicyInsert(task, &Assign.os_assign_read_vector);
    cpu_mask = CPU_MASK ^ (1 << GetCpuId());
    HwSendIpi(ASSIGN_IPI, cpu_mask);
}

static void ComnbineInsert(struct TaskDescriptor *task, int coreid)
{
    NULL_PARAM_CHECK(task);

#if KTASK_PRIORITY_MAX > 32
    MERGE_FLAG(&Assign.smp_os_assign_ready_rector[coreid].ready_vector[task->task_dync_sched_member.bitmap_offset], task->task_dync_sched_member.bitmap_row);
#endif
    MERGE_FLAG(&Assign.smp_os_assign_ready_rector[coreid].priority_ready_group, task->task_dync_sched_member.bitmap_column);
    AssignPolicyInsert(task, &Assign.smp_os_assign_ready_rector[coreid]);
    if (coreid != task->task_smp_info.combined_coreid)
    {
        uint32 cpu_mask;
        cpu_mask = 1 << task->task_smp_info.combined_coreid;
        HwSendIpi(ASSIGN_IPI, cpu_mask);
    }
}

/*
 * insert a ready task to system ready table with READY state and remove it from suspend list
 *
 * @param task the task descriptor
 *
 */
void KTaskInsertToReadyVector(struct TaskDescriptor *task)
{
    int coreid = 0;

	NULL_PARAM_CHECK(task);

    KTaskStatSetAsReady(task);

    coreid = task->task_smp_info.combined_coreid;

    switch (coreid)
    {
    case UNCOMBINE_CPU_CORE:
        UncombineInsert(task);
        break;
    
    default:
        ComnbineInsert(task, coreid);
        break;
    }

    SYS_KDEBUG_LOG(KDBG_SCHED, ("insert task[%.*s], the priority: %d\n",
                                      NAME_NUM_MAX, task->task_base_info.name, task->task_dync_sched_member.cur_prio));

}

static void UncombineRemove(struct TaskDescriptor *task)
{
    register x_ubase number = 0;
    register x_ubase highest_priority = 0;

    NULL_PARAM_CHECK(task);

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
}

static void CombineRemove(struct TaskDescriptor *task)
{
    register x_ubase number = 0;
    register x_ubase highest_prio_on_core = 0;
    uint8 combined_coreid = task->task_smp_info.combined_coreid;

    NULL_PARAM_CHECK(task);

    if (IsDoubleLinkListEmpty(&(Assign.smp_os_assign_ready_rector[combined_coreid].priority_ready_vector[task->task_dync_sched_member.cur_prio]))) {
#if KTASK_PRIORITY_MAX > 32
        CLEAR_FLAG(&Assign.smp_os_assign_ready_rector[combined_coreid].ready_vector[task->task_dync_sched_member.bitmap_offset], task->task_dync_sched_member.bitmap_row);
        if (Assign.os_assign_read_vector.ready_vector[task->task_dync_sched_member.bitmap_offset] == 0) {
            CLEAR_FLAG(&Assign.smp_os_assign_ready_rector[combined_coreid].priority_ready_group, task->task_dync_sched_member.bitmap_column);
        }
        number = PrioCaculate(Assign.smp_os_assign_ready_rector[combined_coreid].priority_ready_group);
        highest_prio_on_core = (number * 8) + PrioCaculate(Assign.smp_os_assign_ready_rector[combined_coreid].ready_vector[number]);
#else
        CLEAR_FLAG(&Assign.smp_os_assign_ready_rector[combined_coreid].priority_ready_group, task->task_dync_sched_member.bitmap_column);
        highest_prio_on_core = PrioCaculate(Assign.smp_os_assign_ready_rector[combined_coreid].priority_ready_group);
#endif
        Assign.smp_os_assign_ready_rector[combined_coreid].highest_prio = highest_prio_on_core;
    }
}

/*
 * a task will be removed from ready table.
 *
 * @param task  task descriptor
 *
 */
void KTaskOsAssignRemoveKTask(struct TaskDescriptor *task)
{
    
	NULL_PARAM_CHECK(task);

    SYS_KDEBUG_LOG(KDBG_SCHED, ("remove task[%.*s], the priority: %d\n",
                                      NAME_NUM_MAX, task->task_base_info.name,
                                      task->task_dync_sched_member.cur_prio));

    DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
    switch (task->task_smp_info.combined_coreid)
    {
    case UNCOMBINE_CPU_CORE:
        UncombineRemove(task);
        break;
    
    default:
        CombineRemove(task);
        break;
    }
}

x_err_t YieldOsAssign(void)
{
    x_base lock = 0;
    int coreid = 0;
    x_ubase highest_prio = 0;
    struct TaskDescriptor *new_task = NONE;
    struct TaskDescriptor *runningtask = NONE;
    
    lock  = DISABLE_INTERRUPT();
    HwLockSpinlock(&AssignSpinLock);
    coreid = GetCpuId();
    runningtask = Assign.smp_os_running_task[coreid];

    if (isrManager.done->getCounter()) {
        HwUnlockSpinlock(&AssignSpinLock);
        ENABLE_INTERRUPT(lock);
        return -ERROR;
    }

    if(Assign.assign_lock[coreid] >= 1) {
        HwUnlockSpinlock(&AssignSpinLock);
        ENABLE_INTERRUPT(lock);
        return -ERROR;
    }
    /* if the bitmap is empty then do not switch */
    if((RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.os_assign_read_vector)) &&
        (RET_TRUE == JudgeAssignReadyBitmapIsEmpty(&Assign.smp_os_assign_ready_rector[coreid]))) {
        HwUnlockSpinlock(&AssignSpinLock);
        ENABLE_INTERRUPT(lock);
        return -ERROR;
    }

    highest_prio = Assign.smp_assign_done->GetHighest();
    new_task = Assign.smp_assign_done->select();

    if(RET_TRUE != JudgeKTaskStatIsRunning(runningtask)) {
        CHECK(NONE != new_task);
    } else {
        Assign.ready_vector_done->insert(runningtask);
    }

    new_task->task_smp_info.runing_coreid = coreid;
    Assign.current_priority[coreid] = (uint8)highest_prio;

    HOOK(hook.assign.hook_Assign,(runningtask, new_task));

    SYS_KDEBUG_LOG(KDBG_SCHED,
            ("[%d]switch to priority#%d "
                "task:%.*s(sp:0x%08x), "
                "from task:%.*s(sp: 0x%08x)\n",
                isrManager.done->getCounter(), highest_prio,
                NAME_NUM_MAX, new_task->task_base_info.name, new_task->stack_point,
                NAME_NUM_MAX, runningtask->task_base_info.name, runningtask->stack_point));

    Assign.smp_assign_done->SwitchToNew(runningtask,new_task);

    ENABLE_INTERRUPT(lock);

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

    FirstRunningTask = Assign.smp_assign_done->select();
    Assign.smp_assign_done->SetSystemTask(FirstRunningTask);
    Assign.smp_assign_done->SwitchToFirst(FirstRunningTask);
}

/**
 *
 * system OsAssign init function
 */
void SysInitOsAssign(void)
{
    SYS_KDEBUG_LOG(KDBG_SCHED, ("start Os Assign: max priority 0x%02x\n",
                                      KTASK_PRIORITY_MAX));

    Assign.ready_vector_done = &ready_vector_done;
    Assign.smp_assign_done = &smp_assign_done;
    Assign.ready_vector_done->init(&Assign.os_assign_read_vector);
    Assign.smp_assign_done->SmpInit();

}