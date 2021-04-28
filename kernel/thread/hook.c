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
* @file:    hook.c
* @brief:   the general interface functions of hook
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/20
*
*/

#include <xs_kdbg.h>
#include <xs_hook.h>

struct KernelHook hook;

/**
 *  a hook function, it will run when switch task
 *
 * @param from task descriptor
 * @param to task descriptor
 */
void AssignHook(KTaskDescriptorType from, KTaskDescriptorType to)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], from task: %s, to task:%s\n",__func__,from->task_base_info.name,to->task_base_info.name));
    /* add your code  */
}

/**
 * 
 * This hook function will be run after task inited .
 *
 * @param  task task descripter
 *
 * @note no blocked or suspend function.
 */
void TaskCreatehook(KTaskDescriptorType task)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Create task: %s\n",__func__,task->task_base_info.name));
     /* add your code  */
}

/**
 * 
 * This hook function will be run after task suspended .
 *
 * @param  task descripter
 *
 * @note no blocked or suspend function.
 */
void TaskSuspendhook(KTaskDescriptorType task)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Suspend task: %s\n",__func__,task->task_base_info.name));
     /* add your code  */
}

/**
 * 
 * This hook function will be run after task resumed .
 *
 * @param  task descripter
 *
 * @note no blocked or suspend function.
 */
void TaskResumehook(KTaskDescriptorType task)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Resume task: %s\n",__func__,task->task_base_info.name));
     /* add your code  */
}

/**
 *  a MallocHook function. The MallocHook function will be run before a memory block is allocated from buddy-memory.
 *
 * @param ptr alloc mem point
 * @param size alloc mem size
 */
void MemMallochook(void *ptr, x_size_t size)
{
     SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Malloc memory: ptr:%p, size: %d\n",__func__,ptr,size));
     /* add your code  */
}

/**
 * a FreeHook function. The FreeHook function will be run after a memory block is freed to buddy-memory.
 *
 * @param ptr free mem point
 */
void MemFreehook(void *ptr)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Free memory: ptr:%p\n",__func__,ptr));
   /* add your code  */
}
#ifdef KERNEL_MEMBLOCK
/**
 *  a allocation hook function before a gatherblock allocation.
 *
 * @param gm the alloc hook function
 * @param date_ptr  mem point
 */
void GmAllocHook(struct MemGather *gm, void *date_ptr)
{
    /* add your code  */
       return;
}

/**
 * a free hook function after a gatherblock release.
 *
 * @param gm the free hook function
 * @param date_ptr mem point
 */
void GmFreeHook(struct MemGather *gm, void *date_ptr)
{
     /* add your code  */
       return;
}
#endif
/**
 * this hook function will run when the system interrupt
 *
 * @note no blocked or suspend.
 */
void IsrEnterHook(void)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Enter Isr\n",__func__));
     /* add your code  */
}

/**
 * this hook function will run when the system leave interrupt
 *
 * @note no blocked or suspend.
 */
void IsrLeaveHook(void)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Leave Isr\n",__func__));
     /* add your code  */
}

void TimerEnterHook(struct Timer *timer)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Timer Enter\n",__func__));
     /* add your code  */
}

void TimerExitHook(struct Timer *timer)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Timer Exit\n",__func__));
     /* add your code  */
}

void IdleTaskHook(void)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s], Idle0 \n",__func__));
     /* add your code  */
}



void testhook(const char *str)
{
    SYS_KDEBUG_LOG(KDBG_HOOK,
                        ("HOOK: Function[%s],Test: %s\n",__func__,str));
     /* add your code  */
    return;
}


int hook_init(void)
{
    REGISTER_HOOK(hook.assign.hook_Assign,AssignHook);

    REGISTER_HOOK(hook.task.hook_TaskCreate,TaskCreatehook);
    REGISTER_HOOK(hook.task.hook_TaskSuspend,TaskSuspendhook);
    REGISTER_HOOK(hook.task.hook_TaskResume,TaskResumehook);
    
    REGISTER_HOOK(hook.mem.hook_Malloc,MemMallochook);
    REGISTER_HOOK(hook.mem.hook_Free,MemFreehook);

#ifdef KERNEL_MEMBLOCK
    REGISTER_HOOK(hook.mem.hook_GmAlloc,GmAllocHook);
    REGISTER_HOOK(hook.mem.hook_GmFree,GmFreeHook);
#endif

    REGISTER_HOOK(hook.timer.hook_TimerEnter,TimerEnterHook);
    REGISTER_HOOK(hook.timer.hook_TimerExit,TimerExitHook);

    REGISTER_HOOK(hook.idle.hook_Idle,IdleTaskHook);
    //REGISTER_HOOK(hook.idle[1].hook_Idle,IdleTask1Hook);

    REGISTER_HOOK(hook.isr.hook_IsrEnter,IsrEnterHook);
    REGISTER_HOOK(hook.isr.hook_IsrLeave,IsrLeaveHook);
    REGISTER_HOOK(hook.test.hook_test,testhook);
    return EOK;
}