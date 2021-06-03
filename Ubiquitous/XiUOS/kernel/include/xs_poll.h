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
* @file:    xs_pool.h
* @brief:   function declaration and structure defintion of poll
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_POLL_H
#define XS_POLL_H

#include <xs_base.h>
#include <xs_waitqueue.h>
#include <sys/time.h> 

#if !defined(POLLIN) && !defined(POLLOUT)

#define POLLIN                   0x001
#define POLLOUT                  0x002
#define POLLERR                  0x004
#define POLLHUP                  0x008
#define POLLNVAL                 0x010

#define POLLMASK_DEFAULT (POLLIN | POLLOUT )

typedef unsigned long NfdsType;

struct pollfd
{
    int   fd;
    short events;
    short revents;
};
#endif 
struct Pollreq;
typedef void (*poll_queue_proc)(WaitQueueType *, struct Pollreq *);
typedef struct Pollreq
{
    poll_queue_proc _proc;
    short _key;
} pollreqType;

void PollAdd(WaitQueueType *wq, pollreqType *req);
int poll(struct pollfd *fds, NfdsType nfds, int timeout);

#endif
