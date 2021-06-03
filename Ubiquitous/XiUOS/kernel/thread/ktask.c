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
* @file:    ktask.c
* @brief:   ktask file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <string.h>
#include <xs_ktask_stat.h>
#include <xs_hook.h>
#include <xs_assign.h>
#include <xs_spinlock.h>
#ifdef TASK_ISOLATION
#include <xs_isolation.h>
#endif

static volatile int __exstatus;

extern DoubleLinklistType KTaskZombie;
extern int32 zombie_recycle;
extern uint8 KTaskStackSetup(struct TaskDescriptor *task);
extern int JudgeZombieKTaskIsNotEmpty(void);

DoubleLinklistType xiaoshan_task_head ={&xiaoshan_task_head, &xiaoshan_task_head};    ///< global task manage list

#if KTASK_PRIORITY_MAX > 32
#define BITMAP_CACULATE_COLUMN_OFFSET(offset,n)     (offset = n / 8)
#endif

#define BITMAP_SETCOLUMN(column,offset)             (column = (1 << offset))
#define BITMAP_SETROW(row,offset)                   (row = (1 << offset))
#define BITLOWMASK_3BIT                             (0x7)


DECLARE_ID_MANAGER(k_task_id_manager, ID_NUM_MAX);

void KTaskIdDelete(int32 id)
{
    IdRemoveObj(&k_task_id_manager, id);
}

inline struct TaskDescriptor *GetTaskWithIdnodeInfo(int32 id)
{
    struct TaskDescriptor *task = NONE;
    struct IdNode *idnode = NONE;
    x_base lock = 0;

    if (id < 0)
        return NONE;

    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_task_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return NONE;
    }
    task =CONTAINER_OF(idnode, struct TaskDescriptor, id);
    CriticalAreaUnLock(lock);
    return task;
}

static inline void __BitmapSiteMask(struct TaskDescriptor *task)
{
    NULL_PARAM_CHECK(task);

#if KTASK_PRIORITY_MAX > 32
            BITMAP_CACULATE_COLUMN_OFFSET(task->task_dync_sched_member.bitmap_offset,task->task_dync_sched_member.cur_prio);
            BITMAP_SETCOLUMN(task->task_dync_sched_member.bitmap_column,task->task_dync_sched_member.bitmap_offset);
            BITMAP_SETROW(task->task_dync_sched_member.bitmap_row,(task->task_dync_sched_member.cur_prio & BITLOWMASK_3BIT));
#else
            BITMAP_SETCOLUMN(task->task_dync_sched_member.bitmap_column,task->task_dync_sched_member.cur_prio);
#endif

}

static inline void _KTaskResourceDelete(struct TaskDescriptor *task)
{
    NULL_PARAM_CHECK(task);

#ifdef ARCH_SMP
    HwLockSpinlock(&AssignSpinLock);
#endif 
    Assign.ready_vector_done->remove(task);
#ifdef ARCH_SMP
    HwUnlockSpinlock(&AssignSpinLock);
#endif 
    KTaskStatSetAsClose(task);

    return;
}

static inline x_err_t __JudgeKTaskIsIdleOrZombierecycle(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);
    if(0 == strncmp(task->task_base_info.name,"ktaskidle0",strlen("ktaskidle0")) || 
        0 == strncmp(task->task_base_info.name,"ktaskidle1",strlen("ktaskidle1")) ||
        0 == strncmp(task->task_base_info.name,"ZombieRecycleKTask",strlen("ZombieRecycleKTask")) )
    {
        return RET_TRUE;
    }

    return EOK;
}

/**
 * find a task in manage list.
 *
 * @param name task name
 *
 * @note in interrupt status,this function is not permitted to call.
 */
KTaskDescriptorType KTaskSearch(char *name)
{
    x_base lock = 0;
    KTaskDescriptorType temp_task = NONE;
    struct SysDoubleLinklistNode *node = NONE;
	
	lock = CriticalAreaLock();

    DOUBLE_LINKLIST_FOR_EACH(node,&xiaoshan_task_head)
	{
		temp_task = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDescriptor, link);
		if (0 == strncmp(temp_task->task_base_info.name, name, NAME_NUM_MAX))
		{
			CriticalAreaUnLock(lock);
			return temp_task;
		}
	}

	CriticalAreaUnLock(lock);

	return NONE;
}

/**
 * This function will get a task descriptor
 *
 */
KTaskDescriptorType GetKTaskDescriptor(void)
{
    x_base lock = 0;
    KTaskDescriptorType task = NONE;
   
    lock = CriticalAreaLock();
#ifdef ARCH_SMP
    HwLockSpinlock(&AssignSpinLock);
    if(Assign.smp_os_running_task[GetCpuId()] != NONE){
        task = Assign.smp_os_running_task[GetCpuId()];
    }
    HwUnlockSpinlock(&AssignSpinLock);
#else
    if(Assign.os_running_task != NONE){
        task = Assign.os_running_task;
    }
#endif
    
    CriticalAreaUnLock(lock);
    return task;
}


/**
 * This function will delay current task running with some ticks.
 *
 * @param tick delay ticks
 *
 */
x_err_t _DelayKTask(KTaskDescriptorType task, x_ticks_t ticks)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(task);

    if (ticks == 0) {
        KPrintf("Timeout ticks must be setted more than 0.\n");
        return -EINVALED;
    }

    lock = CriticalAreaLock();
    SuspendKTask(task->id.id);
    KTaskSetDelay(task,ticks);
    CriticalAreaUnLock(lock);

    DO_KTASK_ASSIGN;

    if (task->exstatus == -ETIMEOUT)
        task->exstatus = EOK;

    return EOK;
}

/**
 * This function will delay current task wite milliseconds.
 *
 * @param ms milliseconds of delay time
 *
 */
x_err_t _MdelayKTask(KTaskDescriptorType task, uint32 ms)
{
    x_ticks_t ticks = 0;

    NULL_PARAM_CHECK(task);

    if (0 == ms) {
        return -EINVALED;
    }

    ticks = CalculteTickFromTimeMs(ms);

    return  _DelayKTask(task, ticks);
}


/**
 * This function sets the task's priority.
 *
 * @param id task id
 * @param prio priority value
 *
 * @return EOK
 */
x_err_t _KTaskPrioSet(KTaskDescriptorType task, uint8 prio)
{
     extern long ShowTask(void);
    x_base lock = 0;
    int ret = EOK;
    uint8 task_stat = 0;

    NULL_PARAM_CHECK(task);
    lock = CriticalAreaLock();
   
    if (0 == strncmp("ktaskidle",task->task_base_info.name, strlen("ktaskidle"))) 
    {
        KPrintf("IDLE task [%s] is forbidden to change priority.\n",task->task_base_info.name);
        CriticalAreaUnLock(lock);
        return -ERROR;
    }
    task_stat = KTaskStatGet(task);
    
    switch(task_stat) 
    {
        case KTASK_READY:
#ifdef ARCH_SMP
            HwLockSpinlock(&AssignSpinLock);
#endif
            task->task_dync_sched_member.cur_prio = prio;
            Assign.ready_vector_done->remove(task);
            __BitmapSiteMask(task);
            Assign.ready_vector_done->insert(task);
#ifdef ARCH_SMP
            HwUnlockSpinlock(&AssignSpinLock);
#endif
            break;
        case KTASK_INIT:
        case KTASK_SUSPEND:
        case KTASK_RUNNING:
             task->task_dync_sched_member.cur_prio = prio; KTaskDescriptorType tid;
        case KTASK_CLOSE:
            ShowTask();
            KPrintf("the close stat task is forbidden to change priority.\n");

            ret = -ERROR;
            break;
        default:
            KPrintf("invalid stat task is forbidden to change priority.\n");
            ret = -EINVALED;
            break;
    }
    CriticalAreaUnLock(lock);

    return ret;
}

#ifdef ARCH_SMP
/**
 * This function binds a task to cpu core.
 *
 * @param id task id
 * @param coreid cpu core id 
 *
 * @return EOK
 */
x_err_t _KTaskCoreCombine(KTaskDescriptorType task, uint8 coreid)
{
    x_base lock = 0;
    int ret = EOK;
    uint8 task_stat = 0;

    NULL_PARAM_CHECK(task);

    lock = CriticalAreaLock();
    task_stat = KTaskStatGet(task);
    switch(task_stat)
    {
        case KTASK_READY:
            HwLockSpinlock(&AssignSpinLock); 
            Assign.ready_vector_done->remove(task);
            task->task_smp_info.combined_coreid = coreid > CPU_NUMBERS ? UNCOMBINE_CPU_CORE : coreid;
            Assign.ready_vector_done->insert(task);
            HwUnlockSpinlock(&AssignSpinLock);
            break;
        case KTASK_INIT:
        case KTASK_SUSPEND:
            task->task_smp_info.combined_coreid = coreid > CPU_NUMBERS ? UNCOMBINE_CPU_CORE : coreid;
            break;
        case KTASK_CLOSE:
        case KTASK_RUNNING:
            KPrintf("the CLOSE and RUNNING stat of task is forbidden to change cpu core.\n");
            ret = -ERROR;
            break;
        default:
            KPrintf("%s invalid task stat.\n",__func__);
            ret = -EINVALED;
            break;
    }
    CriticalAreaUnLock(lock);

    return ret;
}

/**
 * This function unbinds a task with cpu core.
 *
 * @param id task id
 *
 * @return EOK
 */
x_err_t _KTaskCoreUnCombine(KTaskDescriptorType task)
{
    x_base lock = 0;
    int ret = EOK;
    uint8 task_stat = 0;

    NULL_PARAM_CHECK(task);

    lock = CriticalAreaLock();
     
    if (0 == strncmp("ktaskidle",task->task_base_info.name, strlen("ktaskidle"))) 
    {
        KPrintf("IDLE task is forbidden to uncombine cpu.\n");
        
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    task_stat = KTaskStatGet(task);
    switch(task_stat)
    {
        case KTASK_READY:
            HwLockSpinlock(&AssignSpinLock);
            Assign.ready_vector_done->remove(task);
            task->task_smp_info.combined_coreid = UNCOMBINE_CPU_CORE;
            Assign.ready_vector_done->insert(task);
            HwUnlockSpinlock(&AssignSpinLock);
            break;
        case KTASK_INIT:
        case KTASK_SUSPEND:
            task->task_smp_info.combined_coreid = UNCOMBINE_CPU_CORE;
            break;
        case KTASK_CLOSE:
        case KTASK_RUNNING:
            KPrintf("the CLOSE and RUNNING stat of task is forbidden to change cpu core.\n");
            ret = -ERROR;
            break;
        default:
            KPrintf("%s invalid task stat.\n",__func__);
            ret = -EINVALED;
            break;
    }
    CriticalAreaUnLock(lock);

    return ret;
}
#endif



/**
 * timeout function of task timer,this function removes the suspend task and add to ready queue
 * then start a new schedule
 *
 * @param parameter arg for task timeout function
 */
void KTaskTimeout(void *parameter)
{
    x_base lock = 0;
    struct TaskDescriptor *task = NONE;
    
	NULL_PARAM_CHECK(parameter);

    lock = CriticalAreaLock();
    
    task = (struct TaskDescriptor *)parameter;

    if(RET_TRUE == JudgeKTaskStatIsSuspend(task)) {
        task->exstatus = -ETIMEOUT;
        DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
#ifdef ARCH_SMP
        HwLockSpinlock(&AssignSpinLock);
#endif
        Assign.ready_vector_done->insert(task);
#ifdef ARCH_SMP
        HwUnlockSpinlock(&AssignSpinLock);
#endif
        CriticalAreaUnLock(lock);
        DO_KTASK_ASSIGN;
    } else {
        CriticalAreaUnLock(lock);
    }
}


/*
 * update the exception status
 *
 */
void KUpdateExstatus(x_err_t status)
{
    if (status < EOK || status >= INVALID_TASK_ERROR) {
        KPrintf("Illegal status code %d\n", status);
        return;
    }

    if (isrManager.done->getCounter() == 0) {
        KTaskDescriptorType CurrentTask;
        CurrentTask = GetKTaskDescriptor();
        if (CurrentTask != NONE) {
            CurrentTask->exstatus = -status;
        } else {
            __exstatus = -status;
            return;
        }
    } else {
        __exstatus = -status;
        return;
    }   
}


/**
 * obtain current exception status
 *
 */
int *KObtainExstatus(void)
{
    if (isrManager.done->getCounter() == 0) {
        KTaskDescriptorType CurrentTask;
        CurrentTask = GetKTaskDescriptor();
        if (CurrentTask != NONE) {
            return (int *) & (CurrentTask->exstatus);
        } else {
            return (int *)&__exstatus;
        }
    } else {
        return (int *)&__exstatus;
    }
}

/**
 * This function will resume a task.
 *
 * @param id task id
 *
 */
x_err_t _KTaskWakeup(KTaskDescriptorType task)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(task);

    if (RET_TRUE != JudgeKTaskStatIsSuspend(task)) {
        SYS_KDEBUG_LOG(KDBG_KTASK, ("task stat must be suspend error stat is , %d\n", KTaskStatGet(task)));
        return -ERROR;
    }

    SYS_KDEBUG_LOG(KDBG_KTASK, ("wakeup task name:  %s\n", task->task_base_info.name));

    lock = CriticalAreaLock();
    
    DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
    KTaskUnSetDelay(task);
#ifdef ARCH_SMP
    HwLockSpinlock(&AssignSpinLock);
#endif 
    Assign.ready_vector_done->insert(task);
#ifdef ARCH_SMP
    HwUnlockSpinlock(&AssignSpinLock);
#endif
    CriticalAreaUnLock(lock);
	
    HOOK(hook.task.hook_TaskResume,(task));
    return EOK;
}

/**
 * This function will suspend a task.
 *
 * @param id task id
 *
 */
x_err_t _SuspendKTask(KTaskDescriptorType task)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(task);

    if (task != GetKTaskDescriptor()) {
        SYS_KDEBUG_LOG(KDBG_KTASK, ("running task is forbidden to suspend the others.\n"));
        return -ERROR;
    }
    
    if(RET_TRUE != JudgeKTaskStatIsReady(task) && RET_TRUE != JudgeKTaskStatIsRunning(task)) {
        SYS_KDEBUG_LOG(KDBG_KTASK, ("task stat  0x%2x is forbidden to suspend.\n", KTaskStatGet(task)));
        return -ERROR;
    }

    if(RET_TRUE == JudgeKTaskStatIsSuspend(task)) {
        SYS_KDEBUG_LOG(KDBG_KTASK, ("task stat is already suspend.\n"));
        return -ERROR;
    }

    SYS_KDEBUG_LOG(KDBG_KTASK, ("suspend task name:  %s\n", task->task_base_info.name));

    lock = CriticalAreaLock();
#ifdef ARCH_SMP
    HwLockSpinlock(&AssignSpinLock);
#endif
    Assign.ready_vector_done->remove(task);
#ifdef ARCH_SMP
    HwUnlockSpinlock(&AssignSpinLock);
#endif
    KTaskStateSet(task, KTASK_SUSPEND);
    KTaskUnSetDelay(task);

    CriticalAreaUnLock(lock);

    HOOK(hook.task.hook_TaskSuspend,(task));
    return EOK;
}

/**
 * 
 * kernel task exit function,inster the task to defuction task.
 *
 */
void KTaskQuit(void)
{
    x_base lock = 0;
    struct TaskDescriptor *task = NONE;

    lock = CriticalAreaLock();
	task = GetKTaskDescriptor();
    if(NONE != task) {

        _KTaskResourceDelete(task);

        DoubleLinkListInsertNodeAfter(&KTaskZombie, &(task->task_dync_sched_member.sched_link));

        if (JudgeZombieKTaskIsNotEmpty()) {
            KTaskWakeup(zombie_recycle);
        }
    }
    CriticalAreaUnLock(lock);

    DO_KTASK_ASSIGN;
}

/**
 * This function will insert task to ready queue then schedule
 *
 * @param id task id
 *
 */
x_err_t _StartupKTask(KTaskDescriptorType task)
{
    x_base lock = 0;

    lock = CriticalAreaLock();
    
    NULL_PARAM_CHECK(task);

    if(JudgeKTaskStatIsInit(task) != RET_TRUE) {
       SYS_KDEBUG_LOG(KDBG_KTASK, ("task [%s] stat [%d] is not init\n",
                                   task->task_base_info.name, task->task_dync_sched_member.stat));
        CriticalAreaUnLock(lock);                           
        return -ERROR;
    }

    task->task_dync_sched_member.cur_prio = task->task_base_info.origin_prio;
    
    __BitmapSiteMask(task);

    DoubleLinkListRmNode(&(task->task_dync_sched_member.sched_link));
    KTaskUnSetDelay(task);
#ifdef ARCH_SMP
    HwLockSpinlock(&AssignSpinLock);
#endif
    Assign.ready_vector_done->insert(task);
#ifdef ARCH_SMP
    HwUnlockSpinlock(&AssignSpinLock);
#endif
    CriticalAreaUnLock(lock);

    SYS_KDEBUG_LOG(KDBG_KTASK, ("task [%s] is ready to run,the priority is [%d]\n",
                                   task->task_base_info.name, task->task_base_info.origin_prio));

    if (GetKTaskDescriptor() != NONE) {
        DO_KTASK_ASSIGN;
    }

    return EOK;
}

/**
 * This function will remove a dynamic task out of the task manage list.
 *
 * @param id task id
 *
 */
x_err_t _DeleteKTask(KTaskDescriptorType task)
{
    x_base lock = 0;
    
    NULL_PARAM_CHECK(task);

    if(RET_TRUE == __JudgeKTaskIsIdleOrZombierecycle(task)) {
        KPrintf("idle or zombierecycle is not permitted to delete.\n");
        return -EINVALED;
    }

    lock = CriticalAreaLock();

    _KTaskResourceDelete(task);

    DoubleLinkListInsertNodeAfter(&KTaskZombie, &(task->task_dync_sched_member.sched_link));
    if (JudgeZombieKTaskIsNotEmpty()) {
        KTaskWakeup(zombie_recycle);
    }
    CriticalAreaUnLock(lock);

    DO_KTASK_ASSIGN;

    return EOK;
}

/**
 * 
 * This function will init a kernel task's base info .
 *
 * @param task kernel task descripter
 * @param name task name
 * @param entry task process function
 * @param parameter task arg
 * @param stack_size task stack size
 * @param priority task priority
 * 
 */
static x_err_t _KTaskBaseInfoParse(KTaskDescriptorType task,
                            const char *name,
							void (*entry)(void *parameter),
							void       *parameter,
							uint32 stack_depth,
							uint8  priority)
{

    NULL_PARAM_CHECK(task);
    NULL_PARAM_CHECK(name);
    NULL_PARAM_CHECK(entry);

    if (priority >= KTASK_PRIORITY_MAX) {
        KPrintf("%s %d Invalid task priority[%d] set.\n",__func__,__LINE__,priority);
        return  -EINVALED;
    }
#ifdef SEPARATE_COMPILE
    if(1 == task->task_dync_sched_member.isolation_flag ) {
        task->task_base_info.stack_start = (void *)x_umalloc(stack_depth);
    } else
#endif 
    {
        task->task_base_info.stack_start = (void *)KERNEL_MALLOC(stack_depth);
    }
    

    if (task->task_base_info.stack_start == NONE) {
        KPrintf("%s %d malloc task %s stack depth %d failed.\n",__func__,__LINE__,name,stack_depth);
		return -ENOMEMORY;
	}
	task->task_base_info.stack_depth = stack_depth;

	memset(task->task_base_info.stack_start, '#', task->task_base_info.stack_depth);

    strncpy(task->task_base_info.name, name, NAME_NUM_MAX);
    task->task_base_info.func_entry = (void *)entry;
	task->task_base_info.func_param = parameter;
    task->task_base_info.origin_prio = priority;

    KTaskStackSetup(task);

    return EOK;

}
/**
 * 
 * This function will init a kernel task's dynamic member .
 *
 * @param task task descriptor
 * @param tick task time slice 
 */
static void _KTaskDyncMemberInit(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);

    InitDoubleLinkList(&(task->task_dync_sched_member.sched_link));

    task->task_dync_sched_member.cur_prio = task->task_base_info.origin_prio;
	task->task_dync_sched_member.bitmap_column = 0;
#if KTASK_PRIORITY_MAX > 32
	task->task_dync_sched_member.bitmap_offset = 0;
	task->task_dync_sched_member.bitmap_row = 0;
#endif

#if defined(SCHED_POLICY_FIFO)
    FifoTaskTimesliceInit(task);
#elif defined (SCHED_POLICY_RR)
    RoundRobinTaskTimesliceInit(task);
#elif defined (SCHED_POLICY_RR_REMAINSLICE)
    RoundRobinRemainTaskTimesliceInit(task);
#endif

	task->task_dync_sched_member.advance_cnt = 0;
    task->exstatus = EOK;
    KTaskStatSetAsInit(task);

}

#ifdef ARCH_SMP
/**
 * 
 * This function will init a kernel task's smp info .
 *
 * @param task task descriptor 
 */
static void _KTaskSmpInfoInit(KTaskDescriptorType task)
{
    NULL_PARAM_CHECK(task);

    task->task_smp_info.combined_coreid = UNCOMBINE_CPU_CORE;
	task->task_smp_info.runing_coreid = UNCOMBINE_CPU_CORE;
	task->task_smp_info.critical_lock_cnt = 0;
}
#endif
/**
 * 
 * This function init a kernel task in dynamic way .
 *
 * @param name task name
 * @param parameter task process function
 * @param parameter task arg
 * @param stack_size task stack size
 * @param priority task priority
 * @param tick task time slice
 */
int32 _InitKTask(KTaskDescriptorType task,
                            const char *name,
							void (*entry)(void *parameter),
							void       *parameter,
							uint32 stack_depth,
							uint8  priority)
{
    int32 ret = 0;
    NULL_PARAM_CHECK(task);

    ret = _KTaskBaseInfoParse(task, name, entry, parameter, stack_depth, priority);
    if (ret < 0) {
        KPrintf("%s %d task %s baseinfo parse failed.\n",__func__,__LINE__,name);
        return -ERROR;
    }

    _KTaskDyncMemberInit(task);

#ifdef ARCH_SMP
     _KTaskSmpInfoInit(task);
#endif
	return EOK;
}

static struct KTaskDone Done = {
    .init =  _InitKTask,
    .start = _StartupKTask,
    .Delete = _DeleteKTask,
    .delay = _DelayKTask,
    .mdelay = _MdelayKTask,
    .prioset = _KTaskPrioSet,
#ifdef ARCH_SMP
    .combine = _KTaskCoreCombine,
    .uncombine = _KTaskCoreUnCombine,
#endif
    .suspend = _SuspendKTask,
    .wake = _KTaskWakeup
};

/**
 * 
 * This function init a kernel task in dynamic way .
 *
 * @param name task name
 * @param parameter task process function
 * @param parameter task arg
 * @param stack_size task stack size
 * @param priority task priority
 * @param tick task time slice
 * 
 * @return EOK on success; ERROR/ENOMEMORY on failure
 */
int32 KTaskCreate(const char *name,
							void (*entry)(void *parameter),
							void       *parameter,
							uint32 stack_depth,
							uint8  priority)
{
	struct TaskDescriptor *task = NONE;
    int32 id = 0;
    x_base lock = 0;

	KDEBUG_NOT_IN_INTERRUPT;
	task = (struct TaskDescriptor *)KERNEL_MALLOC(sizeof(struct TaskDescriptor));
	if (task == NONE) {
        KPrintf("%s %d TaskDescriptor %s malloc failed.\n",__func__,__LINE__,name);
		return -ENOMEMORY;
	}
	memset(task, 0x0, sizeof(struct TaskDescriptor));
    lock = CriticalAreaLock();
    id = IdInsertObj(&k_task_id_manager, &task->id);
    if (id < 0) {
        KPrintf("%s %d task id malloc failed.\n",__func__,__LINE__);
        CriticalAreaUnLock(lock);
        KERNEL_FREE(task);
        return -ENOMEMORY;
    }

	DoubleLinkListInsertNodeAfter(&xiaoshan_task_head, &(task->link));
    
    task->Done = &Done;
    CriticalAreaUnLock(lock);

    if( task->Done->init(task, name, entry, parameter, stack_depth, priority) == EOK ) {
        HOOK(hook.task.hook_TaskCreate, (task));
        return id;
    } else {
        KPrintf("%s %d task init failed.\n",__func__,__LINE__);
        lock = CriticalAreaLock();
        IdRemoveObj(&k_task_id_manager, id);
        CriticalAreaUnLock(lock);
        KERNEL_FREE(task);
        return -ERROR;
    }
}

/**
 * This function will insert task to ready queue then schedule
 *
 * @param id task id
 *
 * @return EOK on success; EINVALED on failure
 */
x_err_t StartupKTask(int32 id)
{
    struct TaskDescriptor *task = NONE;

    if(id < 0) {
        return -EINVALED;
    }

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->start(task);
}

/**
 * This function will remove a dynamic task out of the task manage list.
 *
 * @param id task id
 *
 * @return EOK on success; EINVALED on failure
 */
x_err_t KTaskDelete(int32 id)
{
    struct TaskDescriptor *task = NONE;

    if(id < 0) {
        return -EINVALED;
    }

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->Delete(task);
}

/**
 * This function will delay current task running with some ticks.
 *
 * @param tick delay ticks
 * @return EOK on success; EINVALED/EEMPTY on failure
 */
x_err_t DelayKTask(x_ticks_t ticks)
{
    KTaskDescriptorType task = NONE;

    if (ticks == 0) {
        KPrintf("Timeout ticks must be setted more than 0.\n");
        return -EINVALED;
    }

    task = GetKTaskDescriptor();
    if (task == NONE)
    {
        return -EEMPTY;
    }
    return task->Done->delay(task, ticks);
}

/**
 * This function will delay current task wite milliseconds.
 *
 * @param ms milliseconds of delay time
 * 
 * @return EOK on success; EINVALED/EEMPTY on failure
 *
 */
x_err_t MdelayKTask(uint32 ms)
{
    KTaskDescriptorType task = NONE;

    if (0 == ms) {
        return -EINVALED;
    }

    task = GetKTaskDescriptor();
    if (task == NONE)
    {
        return -EEMPTY;
    }
    return task->Done->mdelay(task, ms);
}

/**
 * This function sets the task's priority.
 *
 * @param id task id
 * @param prio priority value
 *
 * @return EOK
 */
x_err_t KTaskPrioSet(int32 id, uint8 prio)
{
    struct TaskDescriptor *task = NONE;

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->prioset(task, prio);
}

#ifdef ARCH_SMP
/**
 * This function binds a task to cpu core.
 *
 * @param id task id
 * @param coreid cpu core id 
 *
 * @return EOK
 */
x_err_t KTaskCoreCombine(int32 id, uint8 coreid)
{
    struct TaskDescriptor *task = NONE;

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->combine(task,coreid);
}

/**
 * This function unbinds a task with cpu core.
 *
 * @param id task id
 *
 * @return EOK
 */
x_err_t KTaskCoreUnCombine(int32 id)
{
    struct TaskDescriptor *task = NONE;

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->uncombine(task);
}
#endif
/**
 * This function will suspend a task.
 *
 * @param id task id
 *
 * @return EOK
 */
x_err_t SuspendKTask(int32 id)
{
    struct TaskDescriptor *task = NONE;

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->suspend(task);
}

/**
 * This function will resume a task.
 *
 * @param id task id
 *
 * @return EOK
 */
x_err_t KTaskWakeup(int32 id)
{
    x_base lock = 0;
    struct TaskDescriptor *task = NONE;

    task = GetTaskWithIdnodeInfo(id);
    NULL_PARAM_CHECK(task);

    return task->Done->wake(task);
}


#ifdef SEPARATE_COMPILE
/**
 *
 * This function init a user task in dynamic way .
 *
 * @param name task name
 * @param parameter task process function
 * @param parameter task arg
 * @param stack_size task stack size
 * @param priority task priority
 * @param tick task time slice
 * 
 * @return EOK on success; ENOMEMORY/EEMPTY on failure
 */

int32 UTaskCreate(const char *name,
							void (*entry)(void *parameter),
							void       *parameter,
							uint32 stack_depth,
							uint8  priority)
{
	struct TaskDescriptor *task = NONE;
    int32 id = 0;
    x_base lock = 0;

	KDEBUG_NOT_IN_INTERRUPT;
    KPrintf("create user task.\n");
	task = (struct TaskDescriptor *)KERNEL_MALLOC(sizeof(struct TaskDescriptor));
	if (task == NONE)
	{
        KPrintf("%s %d TaskDescriptor malloc failed.\n",__func__,__LINE__);
		return -ENOMEMORY;
	}
	memset(task, 0x0, sizeof(struct TaskDescriptor));

    task->task_dync_sched_member.isolation_flag = 1;
    task->task_dync_sched_member.isolation_status = 0;

    lock = CriticalAreaLock();
    id = IdInsertObj(&k_task_id_manager, &task->id);
    if (id < 0) {
        KPrintf("%s %d task id malloc failed.\n",__func__,__LINE__);
        CriticalAreaUnLock(lock);
        KERNEL_FREE(task);
        return -ENOMEMORY;
    }

	DoubleLinkListInsertNodeAfter(&xiaoshan_task_head, &(task->link));
    
    task->Done = &Done;
    CriticalAreaUnLock(lock);

    if( task->Done->init(task, name, entry, parameter, stack_depth, priority) == EOK ) {
        HOOK(hook.task.hook_TaskCreate, (task));
#ifdef  MOMERY_PROTECT_ENABLE
//setup pmp 
      if( mem_access.InitIsolation != NONE) 
      {
          mem_access.InitIsolation(&task->task_dync_sched_member.isolation, (x_ubase)task->task_base_info.stack_start , task->task_base_info.stack_depth);
      }
#endif
        return id;
    } else {
        KPrintf("%s %d task init failed.\n",__func__,__LINE__);
        lock = CriticalAreaLock();
        IdRemoveObj(&k_task_id_manager, id);
        CriticalAreaUnLock(lock);
        KERNEL_FREE(task);
        return -ERROR;
    }
}

#endif
