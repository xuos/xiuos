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
#include <stdint.h>
#include <xs_poll.h>
#include <device.h>
#include <xiuos.h>
#include <iot-vfs.h>
#include <iot-vfs_posix.h>


void PollAdd(WaitQueueType *wq, pollreqType *req)
{
    if (req && req->_proc && wq)
        req->_proc(wq, req);
}


struct rt_poll_node;

struct rt_poll_table
{
    pollreqType req;
    uint32 triggered; 
    KTaskDescriptorType polling_thread;
    struct rt_poll_node *nodes;
};

struct rt_poll_node
{
    struct WaitqueueNode wqn;
    struct rt_poll_table *pt;
    struct rt_poll_node *next;
};

static int WqueuePollWake(struct WaitqueueNode *wait, void *key)
{
    struct rt_poll_node *pn;

    if (key && !((x_ubase)key & wait->key))
        return -1;

    pn =CONTAINER_OF(wait, struct rt_poll_node, wqn);
    pn->pt->triggered = 1;

    return 0;
}

static void _poll_add(WaitQueueType *wq, pollreqType *req)
{
    struct rt_poll_table *pt;
    struct rt_poll_node *node;

    node = (struct rt_poll_node *)x_malloc(sizeof(struct rt_poll_node));
    if (node == NONE)
        return;

    pt =CONTAINER_OF(req, struct rt_poll_table, req);

    node->wqn.key = req->_key;
    InitDoubleLinkList(&(node->wqn.list));
    node->wqn.polling_task = pt->polling_thread;
    node->wqn.cb = WqueuePollWake;
    node->next = pt->nodes;
    node->pt = pt;
    pt->nodes = node;
    WqueueAdd(wq, &node->wqn);
}



static int PollWaitTimeout(struct rt_poll_table *pt, int msec)
{
    int32 timeout;
    int ret = 0;
    struct TaskDescriptor *thread;
    x_base level;

    thread = pt->polling_thread;

    timeout = CalculteTickFromTimeMs(msec);

    level = CriticalAreaLock();

    if (timeout != 0 && !pt->triggered) {
        SuspendKTask(thread->id.id);
        if (timeout > 0)
            KTaskSetDelay(thread,timeout);

        CriticalAreaUnLock(level);

        DO_KTASK_ASSIGN;

        level = CriticalAreaLock();
    }

    ret = !pt->triggered;
    CriticalAreaUnLock(level);

    return ret;
}

static int DoPollFd(struct pollfd *pollfd, pollreqType *req)
{
    int mask = 0;
    int fd;

    fd = pollfd->fd;

    if (fd >= 0) {
        struct FileDescriptor *fdp = GetFileDescriptor(fd);
        mask = POLLNVAL;

        if (fdp) {
            mask = POLLMASK_DEFAULT;
            if (fdp->mntp->fs->poll) {
                req->_key = pollfd->events | POLLERR | POLLHUP;

                mask = fdp->mntp->fs->poll(fdp, req);
            }
            /* Mask out unneeded events. */
            mask &= pollfd->events | POLLERR | POLLHUP;
        }
    }
    pollfd->revents = mask;

    return mask;
}

static int PollDo(struct pollfd *fds, NfdsType nfds, struct rt_poll_table *pt, int msec)
{
    int num;
    int istimeout = 0;
    int n;
    struct pollfd *pf;

    if (msec == 0) {
        pt->req._proc = NONE;
        istimeout = 1;
    }

    while (1) {
        pf = fds;
        num = 0;

        for (n = 0; n < nfds; n ++) {
            if (DoPollFd(pf, &pt->req)) {
                num ++;
                pt->req._proc = NONE;
            }
            pf ++;
        }

        pt->req._proc = NONE;

        if (num || istimeout)
            break;

        if (PollWaitTimeout(pt, msec))
            istimeout = 1;
    }

    return num;
}


int poll(struct pollfd *fds, NfdsType nfds, int timeout)
{
    int num;
    struct rt_poll_table table;
    struct rt_poll_node *node, *temp;

    table.req._proc = _poll_add;
    table.triggered = 0;
    table.nodes = NULL;
    table.polling_thread = GetKTaskDescriptor();

    num = PollDo(fds, nfds, &table, timeout);

    node = table.nodes;
    while (node) {
        temp = node;
        node= node->next;
        WqueueRemove(&temp->wqn);
        x_free(temp);
    }

    return num;
}

