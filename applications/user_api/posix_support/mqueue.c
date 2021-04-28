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
* @file:    mqueue.c
* @brief:   posix api of mqueue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "include/mqueue.h"

mqd_t mq_open(const char *name, int oflag, ...)
{
    
    mqd_t mq;

    mq = UserMsgQueueCreate( DEFAULT_MQUEUE_SIZE, DEFAULT_MAX_MSG_SIZE);
    if (mq < 0) {
        return -1;
    }

    return mq;
}

int mq_close(mqd_t mqdes)
{
    return UserMsgQueueDelete(mqdes);
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
{
    ssize_t ret;

    *msg_prio = 0;
    ret = UserMsgQueueRecv(mqdes, msg_ptr, (unsigned long)&msg_len, 0);

    return ret;
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
    int ret;
    ret = UserMsgQueueSend(mqdes, (void *)msg_ptr, msg_len);
    return ret;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
    return 0;
}

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
    return 0;
}

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned *msg_prio, const struct timespec *abstime)
{
    ssize_t ret;
    int  ticks;

    *msg_prio = 0;
    ticks = abstime->tv_sec * 1000 +
            (abstime->tv_nsec / 1000000 ) ;

    ret = UserMsgQueueRecv(mqdes,  msg_ptr, msg_len, ticks);
    return ret;
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned msg_prio, const struct timespec *abstime)
{
    int ret;
    int  ticks;
    ticks = abstime->tv_sec * 1000 +
            (abstime->tv_nsec / 1000000 ) ;

    ret = UserMsgQueueSendwait(mqdes, (void *)msg_ptr, msg_len, ticks);
    
    return ret;
}