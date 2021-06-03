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
* @file:    xs_timer.h
* @brief:   function declaration and structure defintion of timer
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_TIMER_H
#define XS_TIMER_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>
#include <xs_id.h>

#define TIMER_TRIGGER_ONCE      (1 << 0)
#define TIMER_TRIGGER_PERIODIC  (1 << 1)

#define TIMER_ACTIVE_FALSE      (1 << 0)
#define TIMER_ACTIVE_TRUE       (1 << 1)


struct Timer
{
    struct IdNode   id_node;
    char            name[NAME_NUM_MAX];                                                         
    uint8           active_status;          
    uint8           trigger_mode;              
    void (*func_callback)(void *param);            
    void            *param;                         
    x_ticks_t        origin_timeslice;                         
    x_ticks_t        deadline_timeslice;    
    DoubleLinklistType  link;                                   
    DoubleLinklistType  sortlist;          
    uint8 prio;
    struct Work * t_work;
    struct TimerDone* done;
};
typedef struct Timer *TimerType;

struct TimerDone
{
    void (*Init)(struct Timer *timer,
                    const char *name,
                    void (*timeout)(void *parameter),
                    void *parameter,
                    x_ticks_t time,
                    uint8 trigger_mode);

    x_err_t (*Delete)(TimerType timer);
    x_err_t (*StartRun)(TimerType timer);
    x_err_t (*QuitRun)(TimerType timer);
    x_err_t (*Modify)(TimerType timer, x_ticks_t ticks);
};

int32 KCreateTimer(const char *name,
                           void (*timeout)(void *parameter),
                           void       *parameter,
                           x_ticks_t   time,
                           uint8  trigger_mode);
x_err_t KDeleteTimer(int32 timer_id);
x_err_t KTimerStartRun(int32 timer_id);
x_err_t KTimerQuitRun(int32 timer_id);
x_err_t KTimerModify(int32 timer_id, x_ticks_t ticks);
x_err_t KTimerAssignMemberRun(int32 timer_id, x_ticks_t ticks);

TimerType KGetTimer(int32 id);
int32 KGetTimerID(TimerType timer);
x_ticks_t TimerNextTimeoutTick(void);
void CheckTimerList(void);

#endif
