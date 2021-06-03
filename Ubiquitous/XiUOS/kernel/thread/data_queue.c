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
* @file:    data_queue.c
* @brief:   data queue file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <device.h>

/**
 * This function allocates dynamic memory from dynamic buddy memory.
 *
 * @param p_queue dataqueue structure
 * @param NodeNumber the number of dataqueue 
 *
 * @return EOK on success; ERROR on failure
 */
x_err_t InitDataqueue(DataQueueType *p_queue, uint16 NodeNumber)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(p_queue);

    KPrintf("InitDataqueue\n");

    lock = CriticalAreaLock();
    do {
        memset(p_queue, 0, sizeof(DataQueueType));
        p_queue->front = p_queue->rear = 0;
        p_queue->max_len = NodeNumber;
        p_queue->base = (DataElemType *)x_malloc(p_queue->max_len * sizeof(DataElemType));
        if (!p_queue->base) {
            break;
        }

        // The semaphore is initialized to len-1, because the full condition of the loop queue is real + 1 = = front, 
        // a vacancy is always wasted
        p_queue->sem_blank = KSemaphoreCreate( p_queue->max_len - 1);
        if (!p_queue->sem_blank) {
            break;
        }

        p_queue->sem_data = KSemaphoreCreate( 0);
        if (!p_queue->sem_data) {
            break;
        }

        CriticalAreaUnLock(lock);
        return EOK;
    } while (0);

    CriticalAreaUnLock(lock);
    DeInitDataqueue(p_queue);
    return -ERROR;
}

/**
 * This function realses all dataqueue resource.
 *
 * @param p_queue dataqueue structure 
 *
 */

void DeInitDataqueue(DataQueueType *p_queue)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(p_queue);

    KPrintf("DeInitDataqueue\n");

    lock = DISABLE_INTERRUPT();

    if (p_queue->base) {
        x_free(p_queue->base);
        p_queue->base = NONE;
    }

    if (p_queue->sem_blank) {
        KSemaphoreDelete(p_queue->sem_blank);
        p_queue->sem_blank = NONE;
    }

    if (p_queue->sem_data) {
        KSemaphoreDelete(p_queue->sem_data);
        p_queue->sem_data = NONE;
    }
    ENABLE_INTERRUPT(lock);
}

/**
 * This function will push a data into dataqueue .
 *
 * @param p_queue dataqueue structure
 * @param StartAddr the data needed to be pushed
 * @param DataSize data size
 * @param timeout timeout
 *
 * @return EOK on success; ERROR on failure
 */
x_err_t PushDataqueue(DataQueueType *p_queue, const void *StartAddr, x_size_t DataSize, int32 timeout)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(p_queue);
    NULL_PARAM_CHECK(StartAddr);

    KPrintf("PushDataqueue\n");

    do {
        if (WAITING_FOREVER == timeout) {
            // block
            KSemaphoreObtain(p_queue->sem_blank, WAITING_FOREVER);
            break;
        } else if (0 == timeout) {
            // Nonblocking
            if (EOK == KSemaphoreObtain(p_queue->sem_blank, 0)) {
                break;
            } else {
                return -ERROR;
            }
        }
    } while (0);

    lock = CriticalAreaLock();

    if ((p_queue->rear + 1) % p_queue->max_len != p_queue->front) {
        DataElemType *pElem = &(p_queue->base[p_queue->rear]);
        pElem->data = StartAddr;
        pElem->length = DataSize;
        p_queue->rear = (p_queue->rear + 1) % p_queue->max_len;
    }
    CriticalAreaUnLock(lock);

    KSemaphoreAbandon(p_queue->sem_data);
    return EOK;
}

/**
 * This function will pop a data from dataqueue .
 *
 * @param p_queue dataqueue structure
 * @param StartAddr the data needed to be popped
 * @param DataSize data size
 * @param timeout timeout
 *
 * @return EOK on success; ERROR on failure
 */
x_err_t PopDataqueue(DataQueueType *p_queue, const void **StartAddr, x_size_t *size, int32 timeout)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(p_queue);

    KPrintf("PopDataqueue\n");

    do {
        if (WAITING_FOREVER == timeout) {
            // block
            KSemaphoreObtain(p_queue->sem_data, WAITING_FOREVER);
            break;
        } else if (0 == timeout) {
            // Nonblocking
            if (EOK == KSemaphoreObtain(p_queue->sem_data, 0)) {
                // Get success
                break;
            } else {
                // Get failed, exit directly
                return -ERROR;
            }
        }
    } while (0);

    
    lock = CriticalAreaLock();
    // Semaphore surplus, read data directly
    if (p_queue->front != p_queue->rear) {
        DataElemType *pElem = &(p_queue->base[p_queue->front]);
        *StartAddr = pElem->data;
        *size = pElem->length;
        p_queue->front = (p_queue->front + 1) % p_queue->max_len;
    }
    CriticalAreaUnLock(lock);

    // Release data semaphore
    KSemaphoreAbandon(p_queue->sem_blank);
    return EOK;
}

/**
 * This function will get the first data of dataqueue .
 *
 * @param p_queue dataqueue structure
 * @param StartAddr the data needed to be popped
 * @param size data size
 *
 * @return EOK on success; ERROR on failure
 */
x_err_t DataqueuePeak(DataQueueType *p_queue, const void **StartAddr, x_size_t *size)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(p_queue);

    KPrintf("DataqueuePeak\n");

    if (p_queue->front != p_queue->rear) {
        
        lock = CriticalAreaLock();
        DataElemType *pElem = &(p_queue->base[p_queue->front]);
        *StartAddr = pElem->data;
        *size = pElem->length;
        CriticalAreaUnLock(lock);
        return EOK;
    } else {
        return -ERROR;
    }
}