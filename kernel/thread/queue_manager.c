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
* @file:    queue_manager.c
* @brief:   Unified management of queue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_queue_manager.h>
#include <xs_dataqueue.h>
#include <xs_workqueue.h>
#include <xs_waitqueue.h>

#include <xs_memory.h>

void* g_queue_done[QUEUE_MAX];

void QueuemanagerDoneRegister()
{
    DataQueueDoneType* pdata_queue_done = (DataQueueDoneType*)x_malloc(sizeof(DataQueueDoneType));
    pdata_queue_done->InitDataqueue = InitDataqueue;
    pdata_queue_done->PushDataqueue = PushDataqueue;
    pdata_queue_done->PopDataqueue = PopDataqueue;
    pdata_queue_done->DataqueuePeak = DataqueuePeak;
    pdata_queue_done->DeInitDataqueue = DeInitDataqueue;

    WorkQueueDoneType* pwork_queue_done = (WorkQueueDoneType*)x_malloc(sizeof(WorkQueueDoneType));
    pwork_queue_done->CreateWorkQueue = CreateWorkQueue;
    pwork_queue_done->WorkInit = WorkInit;
    pwork_queue_done->WorkSubmit = WorkSubmit;
    pwork_queue_done->WorkSubmit_immediate = WorkSubmit_immediate;

    WaitQueueDoneType* pwait_queue_done = (WaitQueueDoneType*)x_malloc(sizeof(WaitQueueDoneType));
    pwait_queue_done->InitWqueue = InitWqueue;
    pwait_queue_done->WqueueAdd  = WqueueAdd;
    pwait_queue_done->WqueueRemove = WqueueRemove;
    pwait_queue_done->WqueueWait = WqueueWait;
    pwait_queue_done->WakeupWqueue = WakeupWqueue;

    g_queue_done[DATA_QUEUE] = pdata_queue_done;
    g_queue_done[WORK_QUEUE] = pwork_queue_done;
    g_queue_done[WAIT_QUEUE] = pwait_queue_done;
}