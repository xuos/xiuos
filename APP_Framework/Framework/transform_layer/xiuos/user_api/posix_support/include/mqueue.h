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
* @file:    msgqueue.h
* @brief:   the function definition of posix msg queue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef MQUEUE_H
#define MQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../../switch_api/user_api.h"
#include <time.h>

#define DEFAULT_MQUEUE_SIZE  (10 * 1024)
#define DEFAULT_MAX_MSG_SIZE (1024)

typedef int mqd_t;

struct mq_attr {
    long mq_flags;    /* message queue flags */
    long mq_maxmsg;   /* maximum number of messages */
    long mq_msgsize;  /* maximum message size */
    long mq_curmsgs;  /* number of messages currently queued */
};

mqd_t   mq_open(const char *name, int oflag, ...);
int     mq_close(mqd_t mqdes);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int     mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int     mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);
int     mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abs_timeout);
int     mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout);
int     mq_unlink(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
