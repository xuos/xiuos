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
* @file:    xs_event.h
* @brief:   function declaration and structure defintion of event
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_EVENT_H
#define XS_EVENT_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>
#include <xs_id.h>

#ifdef KERNEL_EVENT

#define EVENT_EVENTS_MASK  0x1FFFFFFF
#define EVENT_OPTIONS_MASK 0x7

#define EVENT_AND          (1 << 0)
#define EVENT_OR           (1 << 1)
#define EVENT_AUTOCLEAN    (1 << 2)

typedef int32 EventIdType;

struct Event 
{
    struct IdNode id;
    uint32 options : 3;
    uint32 events : 29;

    DoubleLinklistType pend_list;
    DoubleLinklistType link;
};

typedef struct 
{
    int32 (*EventCreate)(uint32 options);
    void  (*EventDelete)(struct Event *event);
    int32 (*EventTrigger)(struct Event *event, uint32 events);
    int32 (*EventProcess)(struct Event *event, uint32 events, uint32 options, int32 msec, uint32 *processed);
} EventDoneType;

int32 KEventCreate(uint32 options);
void  KEventDelete(int32 id);
int32 KEventTrigger(int32 id, uint32 events);
int32 KEventProcess(int32 id, uint32 events, uint32 options, int32 msec, uint32 *processed);

#endif
#endif
