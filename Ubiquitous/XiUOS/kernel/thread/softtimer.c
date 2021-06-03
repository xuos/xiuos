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
* @file:    softtimer.c
* @brief:   softtimer file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

DoubleLinklistType xiaoshan_timer_sort_head = {&xiaoshan_timer_sort_head, &xiaoshan_timer_sort_head};
DoubleLinklistType k_timer_list = {&k_timer_list, &k_timer_list};

DECLARE_ID_MANAGER(k_softtimer_id_manager, ID_NUM_MAX);
extern queue *sys_workq;

void _Init(struct Timer *timer,
                 const char *name,
                 void (*timeout)(void *parameter),
                 void *parameter,
                 x_ticks_t time,
                 uint8 trigger_mode)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(timer);

    strncpy(timer->name, name, NAME_NUM_MAX);

    // insert to link
    lock = CriticalAreaLock();
    DoubleLinkListInsertNodeAfter(&(k_timer_list), &(timer->link));
    CriticalAreaUnLock(lock);

    timer->trigger_mode = trigger_mode;
    timer->active_status = TIMER_ACTIVE_FALSE;
    timer->func_callback = timeout;
    timer->param = parameter;
    timer->deadline_timeslice = 0;
    timer->origin_timeslice = time;
    timer->prio = GetKTaskDescriptor()->task_base_info.origin_prio;
}

x_err_t _Delete(TimerType timer)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(timer);

    lock = CriticalAreaLock();

    DoubleLinkListRmNode(&(timer->sortlist));
    DoubleLinkListRmNode(&(timer->link));

    CriticalAreaUnLock(lock);
    KERNEL_FREE(timer);
    timer = NONE;

    return EOK;
}

x_err_t _StartRun(TimerType timer)
{
    NULL_PARAM_CHECK(timer);

    x_base lock = CriticalAreaLock();
    timer->active_status = TIMER_ACTIVE_FALSE;

    CHECK(timer->origin_timeslice < TICK_SIZE_MAX / 2);
    timer->deadline_timeslice = CurrentTicksGain() + timer->origin_timeslice;
    timer->active_status = TIMER_ACTIVE_TRUE;

    DoubleLinklistType *pLink = NONE;
    TimerType pNode = NONE;

    DOUBLE_LINKLIST_FOR_EACH(pLink, &xiaoshan_timer_sort_head) {
        pNode = CONTAINER_OF(pLink, struct Timer, sortlist);
        if (timer->deadline_timeslice < pNode->deadline_timeslice) {
            DoubleLinkListInsertNodeBefore(pLink, &timer->sortlist);

            break;
        }
    }
    if (pLink == &xiaoshan_timer_sort_head) {
        DoubleLinkListInsertNodeBefore(&xiaoshan_timer_sort_head, &timer->sortlist);
    }

    CriticalAreaUnLock(lock);

    return EOK;
}

// Just stop working, don't free memory
x_err_t _QuitRun(TimerType timer)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(timer);

    if (!(timer->active_status == TIMER_ACTIVE_TRUE))
        return -ERROR;

    lock = CriticalAreaLock();

    timer->active_status = TIMER_ACTIVE_FALSE;
    timer->deadline_timeslice = 0;
    DoubleLinkListRmNode(&(timer->sortlist));

    CriticalAreaUnLock(lock);

    return EOK;
}

x_err_t _Modify(TimerType timer, x_ticks_t ticks)
{
    NULL_PARAM_CHECK(timer);
    if (0 == ticks) {
        KPrintf("timeout ticks must be setted more then 0.\n");
        return -EINVALED;
    }
    timer->origin_timeslice = ticks;
    return EOK;
}

static struct TimerDone Done = 
{
    .Init = _Init,
    .Delete = _Delete,
    .StartRun = _StartRun,
    .QuitRun = _QuitRun,
    .Modify = _Modify,
};

/**
 * This function will create a softtimer.
 *
 * @param name the length of the msg queue.
 * @param timeout the callback of the timer.
 * @param parameter the parameter of the callback function
 * @param time the timeout time
 * @param trigger_mode the trigger way of the timer
 *
 * @return id on success
 */

int32 KCreateTimer(const char *name,
                   void (*timeout)(void *parameter),
                   void *parameter,
                   x_ticks_t time,
                   uint8 trigger_mode)
{
    struct Timer *timer = NONE;

    timer = (struct Timer *)x_malloc(sizeof(struct Timer));
    if (timer == NONE) {
        return NONE;
    }
    memset(timer, 0x0, sizeof(struct Timer));

    // Generate ID
    int32 id = IdInsertObj(&k_softtimer_id_manager, &timer->id_node);
    if (id < 0) {
        x_free(timer);
        return NONE;
    }

    timer->done = &Done;
    timer->done->Init(timer, name, timeout, parameter, time, trigger_mode);

    return id;
}

/**
 * This function will delete a timer.
 *
 * @param timer_id the id number of timer.
 *
 * @return 
 */

x_err_t KDeleteTimer(int32 timer_id)
{
    TimerType timer = KGetTimer(timer_id);

    timer->done->Delete(timer);
}

/**
 * This function will startup a timer.
 *
 * @param timer_id the id number of timer.
 *
 * @return 
 */
x_err_t KTimerStartRun(int32 timer_id)
{
    TimerType timer = KGetTimer(timer_id);

    return timer->done->StartRun(timer);
}

/**
 * This function will stop a timer.
 *
 * @param timer_id the id number of timer.
 *
 * @return 
 */
x_err_t KTimerQuitRun(int32 timer_id)
{
    TimerType timer = KGetTimer(timer_id);

    timer->done->QuitRun(timer);
}

/**
 * This function will modify the timeout of a timer.
 *
 * @param timer_id the id number of timer.
 * @param ticks timeout ticks
 *
 * @return 
 */
x_err_t KTimerModify(int32 timer_id, x_ticks_t ticks)
{
    TimerType timer = KGetTimer(timer_id);

    timer->done->Modify(timer, ticks);
}

static void TimerCBEnter(void *param)
{
    struct Timer *t = (struct Timer *)param;

    t->func_callback(t->param);
    free(t->t_work);
}

void timer_work_func(struct Work *work, void *work_data)
{
    x_err_t flag;
    struct Timer *t = work_data;

    int32 timer_work = KTaskCreate("timer_work_thr", TimerCBEnter, t, 2048, t->prio);
    flag = StartupKTask(timer_work);
    if (flag != EOK) {
        KPrintf("timer create callback thread failed .\n");
    }
    
    return;
}

void CheckTimerList(void)
{
    x_base lock = 0;
    struct Timer *t = NONE;
    x_ticks_t current_tick = 0;

    SYS_KDEBUG_LOG(KDBG_SOFTTIMER, ("timer check enter\n"));

    current_tick = CurrentTicksGain();

    lock = CriticalAreaLock();

    while (!IsDoubleLinkListEmpty(&xiaoshan_timer_sort_head)) {
        t = SYS_DOUBLE_LINKLIST_ENTRY(xiaoshan_timer_sort_head.node_next,
                                      struct Timer, sortlist);

        if ((current_tick - t->deadline_timeslice) < TICK_SIZE_MAX / 2) {
            if (t->active_status == TIMER_ACTIVE_TRUE) {
                DoubleLinkListRmNode(&t->sortlist); // Take it off the list first

                current_tick = CurrentTicksGain();

                SYS_KDEBUG_LOG(KDBG_SOFTTIMER, ("current tick: %d\n", current_tick));

                if (t->trigger_mode == TIMER_TRIGGER_PERIODIC) {
                    t->done->StartRun(t); // Periodic timer, re insert the appropriate position of the list
                } else {
                    t->done->QuitRun(t);
                }

                // Throw it to the task queue and start a new thread
                t->t_work = x_malloc(sizeof(struct Work));

                ((WorkQueueDoneType *)sys_workq->done)->WorkInit(t->t_work, timer_work_func, t);
                CriticalAreaUnLock(lock);
                ((WorkQueueDoneType *)sys_workq->done)->WorkSubmit((WorkqueueType *)sys_workq->property, t->t_work, 0);
                lock = CriticalAreaLock();
            } else {
                KPrintf("sortlist run unactive timer(%s), quit this timer\n", t->name);
            }
        }
        else
            break;
    }

    CriticalAreaUnLock(lock);

    SYS_KDEBUG_LOG(KDBG_SOFTTIMER, ("timer check leave\n"));
}

x_err_t KTimerAssignMemberRun(int32 timer_id, x_ticks_t ticks)
{
    TimerType timer = KGetTimer(timer_id);

    NULL_PARAM_CHECK(timer);
    if (ticks == 0) {
        KPrintf("Timeout ticks must be setted more than 0.\n");
        return -EINVALED;
    }
    timer->origin_timeslice = ticks;

    return timer->done->StartRun(timer);
}

/**
 * This function will get the timer with id.
 *
 * @param id the id number of timer.
 *
 * @return timer structrue
 */
TimerType KGetTimer(int32 id)
{
    x_base lock = 0;
    TimerType timer = NONE;
    
    lock = CriticalAreaLock();
    struct IdNode *idnode = IdGetObj(&k_softtimer_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return NONE;
    }

    timer = CONTAINER_OF(idnode, struct Timer, id_node);
    CriticalAreaUnLock(lock);
    return timer;
}

int32 KGetTimerID(TimerType timer)
{
    return timer->id_node.id;
}