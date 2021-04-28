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
* @file:    workqueue.c
* @brief:   workqueue file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <device.h>
queue *sys_workq = NONE;

static void WorkQueueKTaskEntry(void *parameter)
{
    x_base lock = 0;
    WorkqueueType *p_queue = (WorkqueueType *)parameter;

    while (1)
    {
        lock = CriticalAreaLock();
        if (p_queue->front != p_queue->rear) { // The team is not empty
            struct Work *work = &(p_queue->base[p_queue->front]);

            p_queue->front = (p_queue->front + 1) % MAX_WORK_QUEUE; 
            CriticalAreaUnLock(lock);

            if (work->cb_func) {
                work->cb_func(work, work->cb_param); 
            }
        } else {
            CriticalAreaUnLock(lock);

            SuspendKTask(p_queue->task);
            DO_KTASK_ASSIGN;
        }
    }
}

void DeInitWorkQueue(WorkqueueType *p_queue)
{
    if (NONE != p_queue->task) {
        KTaskDelete(p_queue->task);
    }

    if (NONE != p_queue) {
        if (NONE != p_queue->base) {
            x_free(p_queue->base);
        }
        x_free(p_queue);
    }
}

WorkqueueType *CreateWorkQueue(const char *name, uint16 stack_size, uint8 priority)
{
    WorkqueueType *p_queue = NONE;
    do {
        p_queue = x_malloc(sizeof(WorkqueueType));

        if (NONE == p_queue) {
            KPrintf("CreateWorkQueue x_malloc(sizeof(WorkqueueType) failed\n");
            break;
        }

        memset(p_queue, 0, sizeof(WorkqueueType));

        p_queue->base = x_malloc(MAX_WORK_QUEUE * sizeof(struct Work));
        if (NONE == p_queue->base) {
            KPrintf("CreateWorkQueue x_malloc(MAX_WORK_QUEUE * sizeof(struct Work) failed\n");
            break;
        }
        memset(p_queue->base, 0, MAX_WORK_QUEUE * sizeof(struct Work));

        p_queue->front = p_queue->rear = 0;

        // start work thread
        p_queue->task = KTaskCreate(name, WorkQueueKTaskEntry, p_queue, 2048, priority);
        if (NONE == p_queue->task) {
            KPrintf("CreateWorkQueue KTaskCreate failed\n");
            break;
        }
        StartupKTask(p_queue->task);

        return p_queue;
    } while (0);

    DeInitWorkQueue(p_queue);
    return NONE;
}

void WorkInit(struct Work *work, void (*work_func)(struct Work *work, void *work_data), void *work_data)
{
    work->cb_func = work_func;
    work->cb_param = work_data;
}

void work_elem_timeout(void *in_param)
{
    WorkTimerParamType *param = (WorkTimerParamType *)in_param;
    WorkSubmit_immediate(param->p_queue, param->p_work);
}

x_err_t WorkSubmit_immediate(WorkqueueType *sys_workq, struct Work *work)
{
    x_base lock = CriticalAreaLock();

    NULL_PARAM_CHECK(sys_workq);
    NULL_PARAM_CHECK(work);

    // check queue is full
    if ((sys_workq->rear + 1 + MAX_WORK_QUEUE) % MAX_WORK_QUEUE == sys_workq->front) {
        CriticalAreaUnLock(lock);
        return -ERROR;
    }
    // add to queue
    sys_workq->base[sys_workq->rear].cb_func = work->cb_func;
    sys_workq->base[sys_workq->rear].cb_param = work->cb_param;
    sys_workq->rear = (sys_workq->rear + 1) % MAX_WORK_QUEUE;

    // awake work thread
    KTaskWakeup(sys_workq->task);
    CriticalAreaUnLock(lock);
    DO_KTASK_ASSIGN;
}

x_err_t WorkSubmit(WorkqueueType *sys_workq, struct Work *work, x_ticks_t time)
{
    NULL_PARAM_CHECK(sys_workq);
    NULL_PARAM_CHECK(work);
    
    if (0 == time) {
        WorkSubmit_immediate(sys_workq, work);
    } else {
        // init a timer, add to queue later 
        WorkTimerParamType *param = x_malloc(sizeof(WorkTimerParamType));
        param->p_queue = sys_workq;
        param->p_work = work;
        param->p_timer = KCreateTimer("wk_timer", work_elem_timeout, param, time, TIMER_TRIGGER_ONCE);
        if (NONE == param->p_timer) {
            KPrintf("WorkSubmit KCreateTimer failed \n");
            x_free(param);
            return -ERROR;
        }
        KTimerStartRun(param->p_timer);
    }
    return EOK;
}

/**
 * This function will create a workqueue.
 *
 * @return EOK on success;ERROR on failure
 */
int WorkSysWorkQueueInit(void)
{
    if (sys_workq != NONE)
        return 0;

	sys_workq = x_malloc(sizeof(queue));
	sys_workq->done = g_queue_done[WORK_QUEUE];
    sys_workq->property = ((WorkQueueDoneType*)sys_workq->done)->CreateWorkQueue("sys_work", WORKQUEUE_KTASK_STACKSIZE,
                                   WORKQUEUE_KTASK_PRIORITY);

    if (sys_workq->property == NONE) {
        KPrintf("log sys_workq create failed\n");
    } else {
        KPrintf("log sys_workq create success\n");
    }

    return sys_workq->property == NONE ? ERROR : EOK;
}