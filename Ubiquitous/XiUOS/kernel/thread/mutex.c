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
* @file:    mutex.c
* @brief:   mutex file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

DECLARE_ID_MANAGER(k_mutex_id_manager, ID_NUM_MAX);
DoubleLinklistType k_mutex_list ={&k_mutex_list, &k_mutex_list};    ///< global mutex manage list

static int32 _MutexCreate()
{
    int32 id = 0;
    x_base lock = 0;
    struct Mutex *mutex = NONE;

	mutex = x_malloc(sizeof(struct Mutex));
	if (mutex == NONE) {
		return -ENOMEMORY;
	}

	memset(mutex, 0x0, sizeof(struct Mutex));
    lock = CriticalAreaLock();

    id = IdInsertObj(&k_mutex_id_manager, &mutex->id);
    if (id < 0) {
        CriticalAreaUnLock(lock);
        x_free(mutex);
        return -ENOMEMORY;
    }

	InitDoubleLinkList(&mutex->pend_list);

    mutex->val = 1;
    mutex->holder = NONE;
    mutex->origin_prio = 0xFF;
    mutex->recursive_cnt = 0;

	DoubleLinkListInsertNodeAfter(&k_mutex_list, &mutex->link);
    
    CriticalAreaUnLock(lock);
    
    return id;
}

static void _MutexDelete(struct Mutex *mutex)
{
	x_base lock = 0;

    NULL_PARAM_CHECK(mutex);

	LinklistResumeAll(&mutex->pend_list);
	lock = CriticalAreaLock();
    IdRemoveObj(&k_mutex_id_manager, mutex->id.id);
	
	DoubleLinkListRmNode(&(mutex->link));
    CriticalAreaUnLock(lock);
	x_free(mutex);
}

static int32 _MutexObtain(struct Mutex *mutex, int32 msec)
{
    x_base lock = 0;
    int32  wait_time = 0;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(mutex);

    task = GetKTaskDescriptor();
    wait_time = CalculteTickFromTimeMs(msec);
    lock = CriticalAreaLock();

    SYS_KDEBUG_LOG(KDBG_IPC,
                 ("mutex_take: current task %s, mutex value: %d, hold: %d\n",
                  task->task_base_info.name, mutex->val, mutex->recursive_cnt));

    task->exstatus = EOK;

    if (mutex->holder == task) {
        mutex->recursive_cnt++;
    } else {
        if (mutex->val > 0) {
            mutex->val--;
            mutex->holder = task;
            mutex->origin_prio = task->task_dync_sched_member.cur_prio;
            mutex->recursive_cnt++;
        } else {
            if (wait_time == 0) {
               task->exstatus = -ETIMEOUT;
                CriticalAreaUnLock(lock);
                return -ETIMEOUT; 
            } else {
                SYS_KDEBUG_LOG(KDBG_IPC, ("mutex_take: suspend task: %s\n",
                                            task->task_base_info.name));

                if (task->task_dync_sched_member.cur_prio > mutex->holder->task_dync_sched_member.cur_prio)
                {
                    KTaskPrioSet(mutex->holder->id.id, task->task_dync_sched_member.cur_prio);
                }

                LinklistSuspend(&(mutex->pend_list), task, LINKLIST_FLAG_PRIO);

                if (wait_time > 0) {
                    SYS_KDEBUG_LOG(KDBG_IPC,
                                 ("mutex_take: start the timer of task:%s\n",
                                  task->task_base_info.name));
                    KTaskSetDelay(task,wait_time);
                }

                CriticalAreaUnLock(lock);

                DO_KTASK_ASSIGN;

                if (task->exstatus != EOK) {
                    return task->exstatus;
                } else {
                    lock = CriticalAreaLock();
                }
            }
        }
    }

    CriticalAreaUnLock(lock);

    return EOK;
}

static int32 _MutexAbandon(struct Mutex *mutex)
{
    int resched = 0;
    x_base lock = 0;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(mutex);

    task = GetKTaskDescriptor();
    lock = CriticalAreaLock();

    SYS_KDEBUG_LOG(KDBG_IPC,
                 ("mutex_release:current task %s, mutex value: %d, hold: %d\n",
                  task->task_base_info.name, mutex->val, mutex->recursive_cnt));

    if (task != mutex->holder) {
        task->exstatus = -ERROR;
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    mutex->recursive_cnt --;
    if (mutex->recursive_cnt == 0) {
        if (mutex->origin_prio != mutex->holder->task_dync_sched_member.cur_prio)
        {
            KTaskPrioSet(mutex->holder->id.id, mutex->origin_prio);
        }

        if (!IsDoubleLinkListEmpty(&mutex->pend_list)) {
            task = SYS_DOUBLE_LINKLIST_ENTRY(mutex->pend_list.node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
            SYS_KDEBUG_LOG(KDBG_IPC, ("mutex_release: resume task: %s\n",
                                        task->task_base_info.name));

            mutex->holder             = task;
            mutex->origin_prio = task->task_dync_sched_member.cur_prio;
            mutex->recursive_cnt++;

            LinklistResume(&(mutex->pend_list));

            resched = RET_TRUE;
        } else {
            mutex->val++;
            mutex->holder = NONE;
            mutex->origin_prio = 0xff;
        }
    }

    CriticalAreaUnLock(lock);

    if (resched == RET_TRUE)
        DO_KTASK_ASSIGN;

    return EOK;
}

static MutexDoneType done = {
    .MutexCreate = _MutexCreate,
    .MutexDelete = _MutexDelete,
    .MutexObtain = _MutexObtain,
    .MutexAbandon = _MutexAbandon,
};

/**
 * a mutex will be inited in static way,then this mutex will be inserted to the manage list
 *
 * @param mutex the mutex descriptor
 * @param name mutex name
 * @param flag mutex flag
 * 
 * @return EOK on success
 *
 */
int32 KMutexCreate()
{
	KDEBUG_NOT_IN_INTERRUPT;

    return done.MutexCreate();
}

/**
 * a dynamic mutex will be deleted from the manage list
 *
 * @param mutex mutex descriptor
 *
 */
void KMutexDelete(int32 id)
{
	KDEBUG_NOT_IN_INTERRUPT;

    x_base lock = 0;
    struct Mutex *mutex = NONE;
    struct IdNode *idnode = NONE;
    
    if (id < 0)
        return;

    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_mutex_id_manager, id);
    if (idnode == NONE) {
        CriticalAreaUnLock(lock);
        return;
    }
        
    mutex =CONTAINER_OF(idnode, struct Mutex, id);
    CriticalAreaUnLock(lock);

    done.MutexDelete(mutex);
}

/**
 * a mutex will be taken when mutex is available
 *
 * @param mutex mutex descriptor
 * @param msec the time needed waiting
 * 
 * @return EOK on success;ERROR on failure
 *
 */
int32 KMutexObtain(int32 id, int32 msec)
{
    KDEBUG_IN_KTASK_CONTEXT;

    x_base lock = 0;
    struct Mutex *mutex = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0)
        return -ERROR;

    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_mutex_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return -ERROR;
    }
        

    mutex =CONTAINER_OF(idnode, struct Mutex, id);
    CriticalAreaUnLock(lock);

    return done.MutexObtain(mutex, msec);
}

/**
 * release the mutex and resume corresponding suspended task
 *
 * @param mutex mutex descriptor
 *
 * @return EOK on success;ERROR on failure
 */
int32 KMutexAbandon(int32 id)
{
    KDEBUG_IN_KTASK_CONTEXT;

    x_base lock = 0;
    struct Mutex *mutex = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0)
        return -ERROR;

    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_mutex_id_manager, id);
    if (idnode == NONE) {
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    mutex = CONTAINER_OF(idnode, struct Mutex, id);
    CriticalAreaUnLock(lock);
    return done.MutexAbandon(mutex);
}
