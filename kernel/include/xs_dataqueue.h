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
* @file:   xs_dataqueue.h
* @brief:  function declaration and structure defintion of circular area
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef DATAQUEUE_H
#define DATAQUEUE_H

#include <xs_ktask.h>
#include <xs_timer.h>
#include <xs_sem.h>
#include <xs_queue_manager.h>

typedef struct 
{
    const void *data;       ///< data pointer
    x_size_t length;        ///< data length
}DataElemType;

typedef struct 
{
    uint16 front;
    uint16 rear;
    uint16 max_len;
    DataElemType *base;

    int sem_blank;
    int sem_data;
}DataQueueType;

x_err_t InitDataqueue(DataQueueType *p_queue, uint16 NodeNumber);
x_err_t PushDataqueue(DataQueueType *queue,const void *StartAddr, x_size_t DataSize, int32 timeout);
x_err_t PopDataqueue(DataQueueType *queue, const void **StartAddr, x_size_t *size, int32 timeout);
x_err_t DataqueuePeak(DataQueueType *queue, const void **StartAddr, x_size_t *size);
void DeInitDataqueue(DataQueueType *p_queue);

typedef struct
{
    x_err_t (*InitDataqueue)(DataQueueType *p_queue, uint16 NodeNumber);
    x_err_t (*PushDataqueue)(DataQueueType *queue,
                                 const void *StartAddr,
                                 x_size_t DataSize,
                                 int32 timeout);
    x_err_t (*PopDataqueue)(DataQueueType *queue,
                                const void **StartAddr,
                                x_size_t *size,
                                int32 timeout);
    x_err_t (*DataqueuePeak)(DataQueueType *queue,
                                 const void **StartAddr,
                                 x_size_t *size);
    void (*DeInitDataqueue)(DataQueueType *p_queue);
} DataQueueDoneType;
#endif
