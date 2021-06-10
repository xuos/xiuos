/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file mem_syscalls.c
* @brief support newlib memory
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: mem_syscalls.c
Description: support newlib memory
Others: take RT-Thread v4.0.2/components/libc/compilers/newlib/syscalls.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: Use malloc, realloc, calloc and free functions
*************************************************/

#include <stddef.h>

void *_malloc_r (struct _reent *ptr, size_t size)
{
    void* result = (void*)UserMalloc(size);

    if (result == NULL)
    {
        ptr->_errno = 12;
    }

    return result;
}

void *_realloc_r (struct _reent *ptr, void *old, size_t newlen)
{
    void* result = (void*)UserRealloc(old, newlen);

    if (result == NULL)
    {
        ptr->_errno = 12;
    }

    return result;
}

void *_calloc_r (struct _reent *ptr, size_t size, size_t len)
{
    void* result = (void*)UserCalloc(size, len);

    if (result == NULL)
    {
        ptr->_errno = 12;
    }

    return result;
}

void _free_r (struct _reent *ptr, void *address)
{
    UserFree (address);
}