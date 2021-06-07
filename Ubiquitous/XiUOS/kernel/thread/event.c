/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS  is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file:    event.c
* @brief:   event file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

DECLARE_ID_MANAGER(k_event_id_manager, ID_NUM_MAX);
DoubleLinklistType k_event_list ={&k_event_list, &k_event_list};

static int32 _EventCreate(uint32 options)
{
    int32 id = 0;
    x_base lock = 0;
    struct Event *event = NONE;
    
    event = (struct Event *)x_malloc(sizeof(struct Event));
    if (event == NONE)
        return -ENOMEMORY;

    memset(event, 0, sizeof(struct Event));
    lock = CriticalAreaLock();
    id = IdInsertObj(&k_event_id_manager, &event->id);
    if (id < 0) {
        CriticalAreaUnLock(lock);
        x_free(event);
        return -ENOMEMORY;
    }

    event->options = options;
    InitDoubleLinkList(&event->pend_list);

    DoubleLinkListInsertNodeAfter(&k_event_list, &event->link);
    CriticalAreaUnLock(lock);

    return id;
}

static void _EventDelete(struct Event *event)
{
    int resched = 0;
    x_base lock = 0;

    NULL_PARAM_CHECK(event);

    if (!IsDoubleLinkListEmpty(&event->pend_list)) {
        resched = 1;
        LinklistResumeAll(&event->pend_list);
    }

    lock = CriticalAreaLock();
    IdRemoveObj(&k_event_id_manager, event->id.id);

    DoubleLinkListRmNode(&event->link);
    CriticalAreaUnLock(lock);

    x_free(event);

    if (resched)
        DO_KTASK_ASSIGN;
}

static int32 _EventTrigger(struct Event *event, uint32 events)
{
    x_bool resched;
    x_ubase lock = 0;
    x_base status = 0;
    struct TaskDescriptor *task = NONE;
    struct SysDoubleLinklistNode *n = NONE;
    
    NULL_PARAM_CHECK(event);

    if (events == 0)
        return -ERROR;

    resched = RET_FALSE;

    lock = CriticalAreaLock();

    event->events |= events;

    if (IsDoubleLinkListEmpty(&event->pend_list)) {
        CriticalAreaUnLock(lock);
        return EOK;
    }

    n = event->pend_list.node_next;
    while (n != &(event->pend_list)) {
        task = SYS_DOUBLE_LINKLIST_ENTRY(n, struct TaskDescriptor, task_dync_sched_member.sched_link);
        status = -ERROR;
        if (task->event_mode & EVENT_AND) {
            if ((task->event_id_trigger & (event->events & EVENT_EVENTS_MASK)) == task->event_id_trigger)
                status = EOK;
        } else if (task->event_mode & EVENT_OR) {
            if (task->event_id_trigger & (event->events & EVENT_EVENTS_MASK)) {
                task->event_id_trigger = task->event_id_trigger & (event->events & EVENT_EVENTS_MASK);
                status = EOK;
            }
        }

        n = n->node_next;

        if (status == EOK) {
            if (task->event_mode & EVENT_AUTOCLEAN)
                event->events &= ~task->event_id_trigger;

            KTaskWakeup(task->id.id);

            resched = RET_TRUE;
        }
    }

    CriticalAreaUnLock(lock);

    if (resched == RET_TRUE)
        DO_KTASK_ASSIGN;

    return EOK;
}

static int32 _EventProcess(struct Event *event, uint32 events, uint32 options, int32 msec, uint32 *processed)
{
    
    x_ubase lock = 0;
    x_base status = 0;
    int32 timeout = 0;
    struct TaskDescriptor *task = NONE;

    KDEBUG_IN_KTASK_CONTEXT;

    NULL_PARAM_CHECK(event);

    if (events == 0)
        return -ERROR;

    status = -ERROR;
    task = GetKTaskDescriptor();
    task->exstatus = EOK;

    timeout = CalculteTickFromTimeMs(msec);

    lock = CriticalAreaLock();

    if (options & EVENT_AND) {
        if ((event->events & events) == events)
            status = EOK;
    } else if (options & EVENT_OR) {
        if (event->events & events)
            status = EOK;
    } else {
        CHECK(0);
    }

    if (status == EOK) {
        if (processed)
            *processed = (event->events & events);

        if (options & EVENT_AUTOCLEAN)
            event->events &= ~events;
    } else if (timeout == 0) {
        task->exstatus = -ETIMEOUT;
    } else {
        task->event_id_trigger  = (events & EVENT_EVENTS_MASK);
        task->event_mode  = (options & EVENT_OPTIONS_MASK);

        LinklistSuspend(&(event->pend_list), task, event->options);

        if (timeout > 0)
            KTaskSetDelay(task,timeout);

        CriticalAreaUnLock(lock);

        DO_KTASK_ASSIGN;

        if (task->exstatus != EOK)
            return task->exstatus;

        lock = CriticalAreaLock();

        if (processed)
           *processed = task->event_id_trigger;
    }

    CriticalAreaUnLock(lock);

    return task->exstatus;
}

static EventDoneType done = {
    .EventCreate = _EventCreate,
    .EventDelete = _EventDelete,
    .EventTrigger = _EventTrigger,
    .EventProcess = _EventProcess,
};

static struct Event *FindEventById(int32 id)
{
    x_base lock = 0;
    struct IdNode *idnode = NONE;
    struct Event *event = NONE;

    if (id < 0)
        return NONE;
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_event_id_manager, id);
    if (idnode == NONE) {
        CriticalAreaUnLock(lock);
        return NONE;
    }

    event = CONTAINER_OF(idnode, struct Event, id);
    CriticalAreaUnLock(lock);
    
    return event;
}

/**
 * This function will create a event.
 *
 * @param options the trigger way of event.
 *
 * @return id
 */
int32 KEventCreate(uint32 options)
{
    KDEBUG_NOT_IN_INTERRUPT;

    return done.EventCreate(options);
}

/**
 * This function will delete a event.
 *
 * @param id the id number of event.
 *
 * @return 
 */
void KEventDelete(int32 id)
{
    KDEBUG_NOT_IN_INTERRUPT;

    struct Event *event = FindEventById(id);

    if (event == NONE)
        return;

    done.EventDelete(event);
}
/**
 * This function will trigger the event
 *
 * @param id the id number of event
 * @param events trigger way & events flag
 *
 * @return EOK on success.
 */
int32 KEventTrigger(int32 id, uint32 events)
{
    KDEBUG_NOT_IN_INTERRUPT;

    struct Event *event = FindEventById(id);

    if (event == NONE)
        return -ERROR;

    return done.EventTrigger(event, events);
}
/**
 * This function will get the event and process this event
 *
 * @param id the id number of event
 * @param events  events flag
 * @param options trigger way
 * @param msec timeout
 * @processed event processed flag
 *
 * @return EOK on success.
 */
int32 KEventProcess(int32 id, uint32 events, uint32 options, int32 msec, uint32 *processed)
{
    KDEBUG_NOT_IN_INTERRUPT;

    struct Event *event = FindEventById(id);

    return done.EventProcess(event, events, options, msec, processed);
}
