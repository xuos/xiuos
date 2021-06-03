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
* @file:    xs_workqueue.h
* @brief:   function declaration and structure defintion of workqueue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_WORKQUEUE_H
#define XS_WORKQUEUE_H

#include <xs_ktask.h>
#include <xs_timer.h>
#include <xs_queue_manager.h>

#define MAX_WORK_QUEUE 32 

struct Work
{
    void (*cb_func)(struct Work *work, void *work_data);
    void *cb_param;
};

typedef struct 
{
    int          front;
    int          rear;
    struct Work* base;
    int32        task;
}WorkqueueType;

typedef struct
{
    WorkqueueType*    p_queue;
    struct Work*      p_work;
    int32             p_timer;
}WorkTimerParamType;

WorkqueueType *CreateWorkQueue(const char *name, uint16 stack_size, uint8 priority);
void WorkInit(struct Work *work, void (*work_func)(struct Work *work,void *work_data), void *work_data);
x_err_t WorkSubmit(WorkqueueType* sys_workq, struct Work *work, x_ticks_t time);
x_err_t WorkSubmit_immediate(WorkqueueType* sys_workq, struct Work *work);
int WorkSysWorkQueueInit(void);

typedef struct
{
    WorkqueueType *(*CreateWorkQueue)(const char *name, uint16 stack_size, uint8 priority);
    void (*WorkInit)(struct Work *work, void (*work_func)(struct Work *work, void *work_data), void *work_data);
    x_err_t (*WorkSubmit)(WorkqueueType *sys_workq, struct Work *work, x_ticks_t time);
    x_err_t (*WorkSubmit_immediate)(WorkqueueType *sys_workq, struct Work *work);
} WorkQueueDoneType;

#endif
