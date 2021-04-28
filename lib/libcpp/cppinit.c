/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
* Change Logs:
* Date           Author       Notes
* 2014-12-03     Bernard      Add copyright header.
* 2014-12-29     Bernard      Add cplusplus initialization for ARMCC.
* 2016-06-28     Bernard      Add _init/_fini routines for GCC.
* 2016-10-02     Bernard      Add WEAK for cplusplus_system_init routine.
*/

/**
* @file:    cppinit.c
* @brief:   cplusplus initialzation
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/25
*/

/*************************************************
File name: cppinit.c
Description: support cppinit function
Others: take RT-Thread v4.0.2/components/cplusplus/crt_init.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support cppinit function
*************************************************/

int cplusplus_system_init(void)   

{
    typedef void(*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;

    for (p = __ctors_start__; p < __ctors_end__; p++)
        (*p)();

    return 0;
}
