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
* @file:    xs_hook.h
* @brief:   function declaration and structure defintion of hook
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_HOOK_H
#define XS_HOOK_H

#include <xs_base.h>
#include <xs_ktask.h>
#include <xs_kdbg.h>
#include <xs_memory.h>

#ifdef KERNEL_HOOK
#define HOOK(function, argv) \
    do { if ((function) != NONE) function argv; } while (0)
#else
#define HOOK(function, argv)
#endif

#define REGISTER_HOOK(hook, function) \
    { if ((function) != NONE) hook = function ; }

#define UNREGISTER_HOOK(hook) \
    { hook = NONE ; }

struct AssignHook 
{
    void (*hook_Assign)(KTaskDescriptorType from, KTaskDescriptorType to);
};

struct TaskHook 
{
    void (*hook_TaskCreate) (KTaskDescriptorType task);
    void (*hook_TaskSuspend)(KTaskDescriptorType task);
    void (*hook_TaskResume) (KTaskDescriptorType task);
    
};

struct MemHook 
{
    void (*hook_Malloc)(void *ptr, x_size_t size);
    void (*hook_Free)(void *ptr);
#ifdef KERNEL_MEMBLOCK
    void (*hook_GmAlloc)(struct MemGather *gm, void *date_ptr);
    void (*hook_GmFree)(struct MemGather *gm, void *date_ptr);
#endif
};

struct TimerHook
{
    void (*hook_TimerEnter)(struct Timer *timer);
    void (*hook_TimerExit)(struct Timer *timer);
};

struct IsrHook
{
    void (*hook_IsrEnter)(void);
    void (*hook_IsrLeave)(void);
};

struct IdleHook 
{
    void (*hook_Idle)(void);
};

struct TestHook
{
     void (*hook_test)(const char *buf);
};

struct KernelHook
{
   struct AssignHook assign;
   struct TaskHook   task;
   struct MemHook    mem;
   struct TimerHook  timer;
   struct IsrHook    isr;
   struct IdleHook   idle;
   struct TestHook   test;

};
extern struct KernelHook hook;

int hook_init(void);

#endif
