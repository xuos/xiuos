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
* @file:    semaphore.c
* @brief:   semaphore file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

#ifdef KDEBUG_NOT_IN_INTERRUPT
#undef KDEBUG_NOT_IN_INTERRUPT
#endif

#define KDEBUG_NOT_IN_INTERRUPT

DECLARE_ID_MANAGER(k_sem_id_manager, ID_NUM_MAX);
DoubleLinklistType k_sem_list = {&k_sem_list, &k_sem_list};

static int32 _SemaphoreCreate(uint16 val)
{
    int32 id = 0;
    x_base lock = 0;
    struct Semaphore *sem = NONE;

    sem = (struct Semaphore *)x_malloc(sizeof(struct Semaphore));
    if (sem == NONE)
        return -ENOMEMORY;

    memset(sem, 0, sizeof(struct Semaphore));
    lock = CriticalAreaLock();
    id = IdInsertObj(&k_sem_id_manager, &sem->id);
    if (id < 0) {
        x_free(sem);
        CriticalAreaUnLock(lock);
        return -ENOMEMORY;
    }

    sem->value = val;
    InitDoubleLinkList(&sem->pend_list);

    
    DoubleLinkListInsertNodeAfter(&k_sem_list, &sem->link);
    CriticalAreaUnLock(lock);

    SYS_KDEBUG_LOG(KDBG_IPC, ("created semaphore: id %d, value %d\n", id, (int)val));

    return id;
}

static void _SemaphoreDelete(struct Semaphore *sem)
{
    int resched = 0;
    x_base lock = 0;

    NULL_PARAM_CHECK(sem);

    SYS_KDEBUG_LOG(KDBG_IPC, ("deleted semaphore: id %d\n", (int)sem->id.id));
    lock = CriticalAreaLock();
    if (!IsDoubleLinkListEmpty(&sem->pend_list)) {
        resched = 1;
        LinklistResumeAll(&sem->pend_list);
    }

    IdRemoveObj(&k_sem_id_manager, sem->id.id);
    DoubleLinkListRmNode(&sem->link);
    CriticalAreaUnLock(lock);

    x_free(sem);

    if (resched){
        DO_KTASK_ASSIGN;
    }
        
}

static int32 _SemaphoreObtain(struct Semaphore *sem, int32 msec)
{
    int lock = 0;
    int32  wait_time = 0;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(sem);

    wait_time = CalculteTickFromTimeMs(msec);
    lock = CriticalAreaLock();

    SYS_KDEBUG_LOG(KDBG_IPC, ("obtain semaphore: id %d, value %d, by task %s\n",
            (int)sem->id.id, (int)sem->value, GetKTaskDescriptor()->task_base_info.name));

    if (sem->value > 0) {
        sem->value--;
        CriticalAreaUnLock(lock);
        return EOK;
    }

    if (wait_time == 0) {
        CriticalAreaUnLock(lock);
        return -ETIMEOUT;
    }

    task = GetKTaskDescriptor();
    task->exstatus = EOK;

    SYS_KDEBUG_LOG(KDBG_IPC, ("obtain semaphore: suspending task %s\n",
            GetKTaskDescriptor()->task_base_info.name));

    LinklistSuspend(&sem->pend_list, task, LINKLIST_FLAG_PRIO);

    if (wait_time > 0) {
         KTaskSetDelay(task, wait_time);
    }

    CriticalAreaUnLock(lock);


    DO_KTASK_ASSIGN;

    return task->exstatus;
}

static int32 _SemaphoreAbandon(struct Semaphore *sem)
{
    int lock = 0;
    int resched = 0;

    NULL_PARAM_CHECK(sem);

    lock = CriticalAreaLock();

    SYS_KDEBUG_LOG(KDBG_IPC, ("abandon semaphore: id %d, value %d, by task %s\n",
            (int)sem->id.id, (int)sem->value, GetKTaskDescriptor()->task_base_info.name));

    if (!IsDoubleLinkListEmpty(&sem->pend_list)) {
        resched = 1;
        LinklistResume(&sem->pend_list);
    } else {
        sem->value++;
    }

    CriticalAreaUnLock(lock);

    if (resched){
        DO_KTASK_ASSIGN;
    }  

    return EOK;
}

static int32 _SemaphoreSetValue(struct Semaphore *sem, uint16 val)
{
    int lock = 0;
    int resched = 0;

    NULL_PARAM_CHECK(sem);

    lock = CriticalAreaLock();

    SYS_KDEBUG_LOG(KDBG_IPC, ("set semaphore value: id %d, old value %d, new value %d, by task %s\n",
            (int)sem->id.id, (int)sem->value, (int)val, GetKTaskDescriptor()->task_base_info.name));

    if (sem->value == val) {
        CriticalAreaUnLock(lock);
        return EOK;
    }

    if (!IsDoubleLinkListEmpty(&sem->pend_list)) {
        resched = 1;
        LinklistResumeAll(&sem->pend_list);
    }
    sem->value = val;

    CriticalAreaUnLock(lock);

    if (resched)
        DO_KTASK_ASSIGN;

    return EOK;
}

static SemaphoreDoneType done = {
    .SemaphoreCreate = _SemaphoreCreate,
    .SemaphoreDelete = _SemaphoreDelete,
    .SemaphoreObtain = _SemaphoreObtain,
    .SemaphoreAbandon = _SemaphoreAbandon,
    .SemaphoreSetValue = _SemaphoreSetValue,
};

/**
 * Create a new semaphore with specified initial value.
 * 
 * @param val initial value
 * @return id of the semaphore
 */
int32 KSemaphoreCreate(uint16 val)
{
    KDEBUG_NOT_IN_INTERRUPT;

    return done.SemaphoreCreate(val);
}

/**
 * Delete a semaphore and wakeup all pending tasks on it.
 * 
 * @param id id of the semaphore to be deleted
 */
void KSemaphoreDelete(int32 id)
{
    x_base lock = 0;
    struct Semaphore *sem = NONE;
    struct IdNode *idnode = NONE;

    KDEBUG_NOT_IN_INTERRUPT;

    if (id < 0)
        return;
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_sem_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return;
    }
    CriticalAreaUnLock(lock);

    sem = CONTAINER_OF(idnode, struct Semaphore, id);
    done.SemaphoreDelete(sem);
}

/**
 * Obtain a semaphore when its value is greater than 0; pend on it otherwise.
 * 
 * @param id id of the semaphore to be obtained
 * @param msec wait time in millisecond
 * @return EOK on success, error code on failure
 */
int32 KSemaphoreObtain(int32 id, int32 msec)
{
    KDEBUG_NOT_IN_INTERRUPT;

    x_base lock = 0;
    struct Semaphore *sem = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0)
        return -ERROR;
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_sem_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    sem =CONTAINER_OF(idnode, struct Semaphore, id);
    CriticalAreaUnLock(lock);
    return done.SemaphoreObtain(sem, msec);
}

/**
 * Abandon a semaphore and wakeup a pending task if any.
 * 
 * @param id id of the semaphore to be abandoned
 * @return EOK on success, error code on failure
 */
int32 KSemaphoreAbandon(int32 id)
{
    KDEBUG_NOT_IN_INTERRUPT;

    x_base lock = 0;
    struct Semaphore *sem = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0)
        return -ERROR;
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_sem_id_manager, id);
    if (idnode == NONE) {
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    sem =CONTAINER_OF(idnode, struct Semaphore, id);
    CriticalAreaUnLock(lock);

    return done.SemaphoreAbandon(sem);
}

/**
 * Set the value of a semaphore, wakeup all pending tasks if new value is positive.
 * 
 * @param id id of the semaphore for which to set value
 * @param val new value
 * @return EOK on success, error code on failure
 */
int32 KSemaphoreSetValue(int32 id, uint16 val)
{
    KDEBUG_NOT_IN_INTERRUPT;

    x_base lock = 0;
    struct Semaphore *sem = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0)
        return -ERROR;
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_sem_id_manager, id);
    if (idnode == NONE) {
        CriticalAreaUnLock(lock);
        return -ERROR;
    }

    sem = CONTAINER_OF(idnode, struct Semaphore, id);
    CriticalAreaUnLock(lock);
    return done.SemaphoreSetValue(sem, val);
}
