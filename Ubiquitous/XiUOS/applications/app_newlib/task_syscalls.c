/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file task_syscalls.c
* @brief support newlib abort
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: task_syscalls.c
Description: support newlib abort
Others: take RT-Thread v4.0.2/components/libc/compilers/newlib/syscalls.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: Use abort function
*************************************************/

#include <xiuos.h>

void abort(void)
{
    KTaskDescriptorType current = GetKTaskDescriptor();
    if (current)
    {
        KPrintf("Task:%-8.*s will be aborted!\n", NAME_NUM_MAX, current->task_base_info.name);
        /* pend current task */
        SuspendKTask(current->id.id);
        /* schedule */
        DO_KTASK_ASSIGN;
    }

    while (1);
}