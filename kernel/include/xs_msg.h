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
* @file:    xs_msg.h
* @brief:   function declaration and structure defintion of message queue
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/2
*
*/

#ifndef XS_MSG_H
#define XS_MSG_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>

#ifdef KERNEL_MESSAGEQUEUE

struct MsgQueue
{
    struct IdNode        id;
    void                 *msg_buf;
	uint16               index;
    uint16               num_msgs;
    uint16               each_len;
    uint16               max_msgs;

    DoubleLinklistType   send_pend_list;
    DoubleLinklistType   recv_pend_list;
	DoubleLinklistType   link;
    struct MsgQueueDone  *Done;
};
typedef struct MsgQueue  *MsgQueueType;

struct MsgQueueDone
{
   x_err_t (*init)(struct MsgQueue *mq, x_size_t msg_size, x_size_t max_msgs );
   x_err_t (*urgensend)(struct MsgQueue *mq, const void *buffer, x_size_t size);
   x_err_t (*send)(struct MsgQueue *mq, const void *buffer, x_size_t size,int32  timeout);
   x_err_t (*recv)(struct MsgQueue *mq, void *buffer, x_size_t  size, int32 timeout);
   x_err_t (*reinit)(struct MsgQueue *mq);
   x_err_t (*Delete)(struct MsgQueue *mq);
};

int32   KCreateMsgQueue( x_size_t msg_size, x_size_t max_msgs );
x_err_t KMsgQueueSendwait(int32  id, const void *buffer, x_size_t  size, int32 timeout);
x_err_t KMsgQueueUrgentSend(int32 id, const void *buffer, x_size_t size);
x_err_t KMsgQueueSend(int32 id, const void *buffer, x_size_t size);
x_err_t KMsgQueueRecv(int32 id, void *buffer, x_size_t size, int32 timeout);
x_err_t KMsgQueueReinit(int32 id);
x_err_t KDeleteMsgQueue(int32 id);
#endif

#endif
