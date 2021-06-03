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
* @file:    user_msg.c
* @brief:   the priviate user api of msg queue for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"

/**
 * This function will create a msg queue.
 *
 * @param msg_size the length of the msg queue.
 * @param max_msgs the max length of the msg queue.
 *
 * @return id on success;ENOMEMORY/ERROR on failure
 */
int32_t UserMsgQueueCreate( size_t   msg_size, 
                                            size_t   max_msgs){
   return  (int32_t)KSwitch2(KS_USER_MSGQUEUE_CREATE,  (uintptr_t)msg_size, (uintptr_t)max_msgs);
}

/**
 * a dynamic messagequeue will be deleted from the manage list
 *
 * @param id the message id
 * 
 * @return EOK on success;EINVALED on failure
 *
 */
x_err_t UserMsgQueueDelete(int32_t mq ){
   return  (x_err_t)KSwitch1(KS_USER_MSGQUEUE_DELETE, (uintptr_t)mq);
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
x_err_t UserMsgQueueSendwait(int32_t mq, const void *buffer,
                                      size_t   size, int32_t  wait_time){
    return  (x_err_t)KSwitch3(KS_USER_MSGQUEUE_SEND , (uintptr_t)mq, (uintptr_t)buffer, (uintptr_t)size );
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
x_err_t UserMsgQueueSend(int32_t mq, const void *buffer, size_t size){
   
   return  (x_err_t)KSwitch3(KS_USER_MSGQUEUE_SEND , (uintptr_t)mq, (uintptr_t)buffer, (uintptr_t)size );
    
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
x_err_t UserMsgQueueUrgentSend(int32_t mq, const void *buffer, size_t size){
   
   return (x_err_t)KSwitch3(KS_USER_MSGQUEUE_URGENTSEND , (uintptr_t)mq, (uintptr_t)buffer, (uintptr_t)size );
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
x_err_t UserMsgQueueRecv(int32_t mq, void *buffer,
                                      size_t  size,int32_t wait_time){
    return (x_err_t)KSwitch4(KS_USER_MSGQUEUE_RECV ,(uintptr_t)mq, (uintptr_t)buffer, (uintptr_t)size, (uintptr_t)wait_time );
}

/**
 * This function will reset a event.
 *
 * @param id the id number of event.
 *
 * @return EOK on success;EINVALED on failure
 */
x_err_t UserMsgQueueReinit(int32_t mq){
    return (x_err_t)KSwitch1(KS_USER_MSGQUEUE_REINIT ,(uintptr_t)mq );
}
