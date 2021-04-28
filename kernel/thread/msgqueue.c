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
* @file:    msgqueue.c
* @brief:   msgqueue file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <xs_delay.h>

DECLARE_ID_MANAGER(k_mq_id_manager, ID_NUM_MAX);
DoubleLinklistType k_mq_list = {&k_mq_list, &k_mq_list};

struct mq_message
{
    struct mq_message *next;
};

static struct MsgQueue *GetMsgQueueById(int32 id)
{
    x_base lock = 0;
    struct MsgQueue *mq = NONE;
    struct IdNode *idnode = NONE;

    if (id < 0 )
       return NONE;
    
    lock = CriticalAreaLock();
    idnode = IdGetObj(&k_mq_id_manager, id);
    if (idnode == NONE){
        CriticalAreaUnLock(lock);
        return NONE;
    } 
    mq = CONTAINER_OF(idnode, struct MsgQueue, id);
    CriticalAreaUnLock(lock);
    return mq;
}

static x_err_t _InitMsgQueue( struct MsgQueue *mq ,x_size_t   msg_size,
                            x_size_t   max_msgs )
{
    x_base lock = 0;

    NULL_PARAM_CHECK(mq);

    mq->max_msgs = max_msgs;
    mq->num_msgs = 0;
    mq->each_len = ALIGN_MEN_UP(msg_size, MEM_ALIGN_SIZE);
    mq->index = 0;
    
    InitDoubleLinkList(&mq->send_pend_list);
    InitDoubleLinkList(&(mq->recv_pend_list));

    mq->msg_buf = x_malloc( mq->each_len * mq->max_msgs);
    if (mq->msg_buf == NONE) {
        lock = CriticalAreaLock();
        DoubleLinkListRmNode(&(mq->link));
        CriticalAreaUnLock(lock);
        KERNEL_FREE(mq);
        return -ENOMEMORY;
    }

    return EOK;
}

static x_err_t _MsgQueueSend(struct MsgQueue *mq,
                              const void *buffer,
                              x_size_t   size,
                              int32  msec)
{
    x_ubase lock = 0;
    uint32 tick_delta = 0;
    int32  timeout = 0;
     uint8 *msg = NONE;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(mq);
    NULL_PARAM_CHECK(buffer);

    if (size > mq->each_len)
        return -ERROR;

    tick_delta = 0;
    task = GetKTaskDescriptor();
    timeout = CalculteTickFromTimeMs(msec);
    lock = CriticalAreaLock();
    if (mq->num_msgs >= mq->max_msgs && timeout == 0) {
        CriticalAreaUnLock(lock);
        return -EFULL;
    }
    
    while(mq->num_msgs >= mq->max_msgs ) {
        
        task->exstatus = EOK;
        if (timeout == 0) {
            CriticalAreaUnLock(lock);
            return -EFULL;
        }
        KDEBUG_IN_KTASK_CONTEXT;
        LinklistSuspend(&(mq->send_pend_list), task, LINKLIST_FLAG_FIFO);

        if (timeout > 0) {
            tick_delta = CurrentTicksGain();
            SYS_KDEBUG_LOG(KDBG_IPC, ("mq_send_wait: start timer of task:%s\n",
                                        task->task_base_info.name));
            KTaskSetDelay(task,timeout);
        }

        CriticalAreaUnLock(lock);
        DO_KTASK_ASSIGN;

        if (task->exstatus != EOK) {
            return task->exstatus;
        }

        lock = CriticalAreaLock();

        if (timeout > 0) {
            tick_delta = CurrentTicksGain() - tick_delta;
            timeout -= tick_delta;
            if (timeout < 0)
                timeout = 0;
        }
    }

    msg = mq->msg_buf + ( ( mq->index + mq->num_msgs ) % mq->max_msgs ) * mq->each_len ;
    memcpy(msg, buffer, size);
    mq->num_msgs ++;
    if (!IsDoubleLinkListEmpty(&mq->recv_pend_list)) {
        LinklistResume(&(mq->recv_pend_list));
        CriticalAreaUnLock(lock);
        DO_KTASK_ASSIGN;
        return EOK;
    }

    CriticalAreaUnLock(lock);
    return EOK;
}

static x_err_t _MsgQueueUrgentSend(struct MsgQueue *mq, const void *buffer, x_size_t size)
{
    x_ubase lock = 0;
    uint8 *msg = NONE;

    NULL_PARAM_CHECK(mq);
    NULL_PARAM_CHECK(buffer);

    if (size > mq->each_len)
        return -ERROR;

    lock = CriticalAreaLock();
    if (mq->num_msgs >= mq->max_msgs) {
        CriticalAreaUnLock(lock);
        return -EFULL;
    }

    mq->index --;
    if (mq->index < 0)
        mq->index += mq->max_msgs;
    
    msg = mq->msg_buf + ( ( mq->index + mq->num_msgs ) % mq->max_msgs ) * mq->each_len ;
    memcpy(msg , buffer, size);
    mq->num_msgs ++;
    if (!IsDoubleLinkListEmpty(&mq->send_pend_list)) {
        LinklistResume(&(mq->send_pend_list));
        CriticalAreaUnLock(lock);
        DO_KTASK_ASSIGN;
        return EOK;
    }

    CriticalAreaUnLock(lock);

    return EOK;
}

static x_err_t _MsgQueueRecv(struct MsgQueue *mq,
                          void *buffer,
                          x_size_t  size,
                          int32 msec)
{
    x_ubase lock = 0;
    uint32 tick_delta = 0;
    int32 timeout = 0;
    struct mq_message *msg = NONE;
    struct TaskDescriptor *task = NONE;

    NULL_PARAM_CHECK(mq);
    NULL_PARAM_CHECK(buffer);

    tick_delta = 0;
    task = GetKTaskDescriptor();
    timeout = CalculteTickFromTimeMs(msec);
    lock = CriticalAreaLock();

    if (mq->index == 0 && timeout == 0) {
        CriticalAreaUnLock(lock);
        return -ETIMEOUT;
    }

    for( ; mq->num_msgs <= 0 ; ) {
        KDEBUG_IN_KTASK_CONTEXT;

        task->exstatus = EOK;
        if (timeout == 0) {
            CriticalAreaUnLock(lock);
            task->exstatus = -ETIMEOUT;
            return -ETIMEOUT;
        }

        LinklistSuspend(&(mq->recv_pend_list),
                            task,
                            LINKLIST_FLAG_FIFO);

        if (timeout > 0) {
            tick_delta = CurrentTicksGain();
            SYS_KDEBUG_LOG(KDBG_IPC, ("set task:%s to timer list\n",
                                        task->task_base_info.name));
            KTaskSetDelay(task,timeout);
        }

        CriticalAreaUnLock(lock);

        DO_KTASK_ASSIGN;

        if (task->exstatus != EOK) {
            return task->exstatus;
        }

        lock = CriticalAreaLock();

        if (timeout > 0) {
            tick_delta = CurrentTicksGain() - tick_delta;
            timeout -= tick_delta;
            if (timeout < 0)
                timeout = 0;
        }
    }

    msg = mq->msg_buf + mq->index * mq->each_len;
    mq->index = (mq->index + 1) % mq->max_msgs;
    memcpy(buffer, msg , size > mq->each_len ? mq->each_len : size);
    mq->num_msgs --;

    if (!IsDoubleLinkListEmpty(&(mq->send_pend_list))) {
        LinklistResume(&(mq->send_pend_list));
        CriticalAreaUnLock(lock);

        DO_KTASK_ASSIGN;
        return EOK;
    }

    CriticalAreaUnLock(lock);

    return EOK;
}

static x_err_t _MsgQueueReinit(struct MsgQueue *mq)
{
    x_ubase lock = 0;

    NULL_PARAM_CHECK(mq);

    lock = CriticalAreaLock();

    LinklistResumeAll(&mq->send_pend_list);
    LinklistResumeAll(&(mq->recv_pend_list));
    mq->index = 0;
    mq->num_msgs = 0;
    CriticalAreaUnLock(lock);
    DO_KTASK_ASSIGN;

    return EOK;
}

static x_err_t _DeleteMsgQueue(struct MsgQueue *mq)
{
    x_base lock = 0;

    NULL_PARAM_CHECK(mq);

    LinklistResumeAll(&(mq->send_pend_list));
    LinklistResumeAll(&(mq->recv_pend_list));
    KERNEL_FREE(mq->msg_buf);

    lock = CriticalAreaLock();
    DoubleLinkListRmNode(&(mq->link));
    CriticalAreaUnLock(lock);
    KERNEL_FREE(mq);

    return EOK;
}

static struct MsgQueueDone Done = {
    .init = _InitMsgQueue ,
    .urgensend = _MsgQueueUrgentSend,
    .send =  _MsgQueueSend,
    .recv = _MsgQueueRecv,
    .reinit = _MsgQueueReinit,
    .Delete = _DeleteMsgQueue
};

/**
 * This function will create a msg queue.
 *
 * @param msg_size the length of the msg queue.
 * @param max_msgs the max length of the msg queue.
 *
 * @return id on success;ENOMEMORY/ERROR on failure
 */
int32 KCreateMsgQueue(x_size_t   msg_size,
                            x_size_t   max_msgs)
{
    int32 id = 0;
    x_base temp = 0;
    x_base lock = 0;
    struct MsgQueue *mq = NONE;

    mq = (struct MsgQueue *)x_malloc(sizeof(struct MsgQueue));
    if (mq == NONE)
        return -ENOMEMORY;
    memset(mq,0x0,sizeof(struct MsgQueue));

    lock = CriticalAreaLock();
    id = IdInsertObj(&k_mq_id_manager, &mq->id);
    CriticalAreaUnLock(lock);
    if (id < 0) {
        x_free(mq);
        return -ENOMEMORY;
    }

    lock = CriticalAreaLock();
    DoubleLinkListInsertNodeAfter(&k_mq_list, &mq->link);
    CriticalAreaUnLock(lock);
    mq->Done = &Done;
    if( mq->Done->init(mq, msg_size,max_msgs) == EOK )
      return mq->id.id;
    else
       return -ERROR;
}

/**
 * This function will reset a event.
 *
 * @param id the id number of event.
 *
 * @return EOK on success;EINVALED on failure
 */
x_err_t KMsgQueueReinit(int32 id)
{
    struct MsgQueue *mq = NONE;

    if (id < 0)
       return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
       return mq->Done->reinit(mq);
    else
      return -EINVALED;

}

/**
 * receive message with some waiting time
 *
 * @param id the message id
 * @param buffer message info
 * @param size the size of buffer
 * @param timeout time needed waiting
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t KMsgQueueRecv(int32 id,
                          void *buffer,
                          x_size_t  size,
                          int32 timeout)
{
    struct MsgQueue *mq = NONE;
    if (id < 0)
       return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
       return mq->Done->recv(mq,buffer,size,timeout);
    else
      return -EINVALED;

}

/**
 * a dynamic messagequeue will be deleted from the manage list
 *
 * @param id the message id
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t KDeleteMsgQueue(int32 id)
{
    struct MsgQueue *mq = NONE;
    if (id < 0)
       return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
       return mq->Done->Delete(mq);
    else
      return -EINVALED;
}

/**
 * send urgent message without waiting time, this message will be inserted to the head of the queue
 *
 * @param id the message id
 * @param buffer message info
 * @param size the size of buffer
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t KMsgQueueUrgentSend(int32 id, const void *buffer, x_size_t size)
{
    struct MsgQueue *mq = NONE;

   if (id < 0)
      return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
       return mq->Done->urgensend(mq,buffer,size);
    else
      return -EINVALED;
}

/**
 * send message without waiting time,current suspend task will be resumed
 *
 * @param id the message id
 * @param buffer message info
 * @param size the size of buffer
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t KMsgQueueSend(int32 id, const void *buffer, x_size_t size)
{
    struct MsgQueue *mq = NONE;

   if (id < 0)
      return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
      return mq->Done->send(mq,buffer,size,0);
    else
      return -EINVALED;
}
/**
 * send message with waiting time,current suspend task will be resumed
 *
 * @param id the message id
 * @param buffer message info
 * @param size the size of buffer
 * @param timeout waiting time
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t KMsgQueueSendwait(int32 id, const void *buffer, x_size_t size,int32  timeout)
{
    struct MsgQueue *mq = NONE;

    if (id < 0)
       return -EINVALED;

    mq = GetMsgQueueById(id);
    if (mq != NONE)
       return mq->Done->send(mq,buffer,size,timeout);
    else
      return -EINVALED;
}