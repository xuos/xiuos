/****************************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/**
* @file kswitch.h
* @brief risc-v ecall function 
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: kswitch.h
Description: risc-v ecall function 
Others: take incubator-nuttx arch/risc-v/include/rv64gc/syscall.h for references
                https://github.com/apache/incubator-nuttx/tree/master/arch/risc-v/include/rv64gc
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. Modify function name for a unified  
2. Add some functions when there is no system call
*************************************************/

#ifndef __XS_RISC_V_KSWITCH_H__
#define __XS_RISC_V_KSWITCH_H__

#include <stdint.h>

#include "../../../kernel/include/xs_service.h"

#ifdef TASK_ISOLATION

/****************************************************************************
 * Name: KSwitch0
 *
 * Description:
 *      ecall with zero parameters.
 *
 ****************************************************************************/
static inline uintptr_t KSwitch0(unsigned int nbr)
{
    register long r0 asm("a0") = (long)(nbr);

    asm volatile
    (
        "ecall"
        :: "r"(r0)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}

/****************************************************************************
 * Name: KSwitch1
 *
 * Description:
 *      ecall with one parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch1(unsigned int nbr, uintptr_t parm1)
{
    register long r0 asm("a0") = (long)(nbr);
    register long r1 asm("a1") = (long)(parm1);

    asm volatile
    (
        "ecall"
        :: "r"(r0), "r"(r1)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}

/****************************************************************************
 * Name: KSwitch2
 *
 * Description:
 *     ecall with two parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch2(unsigned int nbr, uintptr_t parm1,
                                uintptr_t parm2)
{
    register long r0 asm("a0") = (long)(nbr);
    register long r1 asm("a1") = (long)(parm1);
    register long r2 asm("a2") = (long)(parm2);

    asm volatile
    (
        "ecall"
        :: "r"(r0), "r"(r1), "r"(r2)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}

/****************************************************************************
 * Name: KSwitch3
 *
 * Description:
 *     ecall with three parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch3(unsigned int knum, uintptr_t arg1,
                                uintptr_t arg2, uintptr_t arg3)
{
    register long reg0 asm("a0") = (long)(knum);
    register long reg1 asm("a1") = (long)(arg1);
    register long reg2 asm("a2") = (long)(arg2);
    register long reg3 asm("a3") = (long)(arg3);

    asm volatile
    (
        "ecall"
        :: "r"(reg0), "r"(reg1), "r"(reg2), "r"(reg3)
    );

    asm volatile("nop" : "=r"(reg0));

    return reg0;
}


/****************************************************************************
 * Name: KSwitch4
 *
 * Description:
 *     ecall with four parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch4(unsigned int nbr, uintptr_t parm1,
                                uintptr_t parm2, uintptr_t parm3,
                                uintptr_t parm4)
{
    register long r0 asm("a0") = (long)(nbr);
    register long r1 asm("a1") = (long)(parm1);
    register long r2 asm("a2") = (long)(parm2);
    register long r3 asm("a3") = (long)(parm3);
    register long r4 asm("a4") = (long)(parm4);

    asm volatile
    (
        "ecall"
        :: "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}

/****************************************************************************
 * Name: KSwitch5
 *
 * Description:
 *     ecall with five parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch5(unsigned int nbr, uintptr_t parm1,
                                uintptr_t parm2, uintptr_t parm3,
                                uintptr_t parm4, uintptr_t parm5)
{
    register long r0 asm("a0") = (long)(nbr);
    register long r1 asm("a1") = (long)(parm1);
    register long r2 asm("a2") = (long)(parm2);
    register long r3 asm("a3") = (long)(parm3);
    register long r4 asm("a4") = (long)(parm4);
    register long r5 asm("a5") = (long)(parm5);

    asm volatile
    (
        "ecall"
        :: "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}

/****************************************************************************
 * Name: KSwitch6
 *
 * Description:
 *     ecall with six parameters.
 *
 ****************************************************************************/

static inline uintptr_t KSwitch6(unsigned int nbr, uintptr_t parm1,
                                uintptr_t parm2, uintptr_t parm3,
                                uintptr_t parm4, uintptr_t parm5,
                                uintptr_t parm6)
{
    register long r0 asm("a0") = (long)(nbr);
    register long r1 asm("a1") = (long)(parm1);
    register long r2 asm("a2") = (long)(parm2);
    register long r3 asm("a3") = (long)(parm3);
    register long r4 asm("a4") = (long)(parm4);
    register long r5 asm("a5") = (long)(parm5);
    register long r6 asm("a6") = (long)(parm6);

    asm volatile
    (
        "ecall"
        :: "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6)
    );

    asm volatile("nop" : "=r"(r0));

    return r0;
}
#else
static inline unsigned long KSwitch0(unsigned int knum)
{
    uintptr_t param[1] = {0};
    uint8_t num = 0;
    SERVICETABLE[knum].fun(knum, param, num);
}

static inline unsigned long KSwitch1(unsigned int knum, unsigned long arg1)
{
     uintptr_t param[1] = {0};
     uint8_t num = 0;
     param[0] = arg1;
     SERVICETABLE[knum].fun(knum, param, num );
}


static inline unsigned long KSwitch2(unsigned int knum, unsigned long arg1,
                                    unsigned long arg2)
{
     uintptr_t param[2] = {0};
     uint8_t num = 0;
     param[0] = arg1;
     param[1] = arg2;
     SERVICETABLE[knum].fun(knum, param, num );
}


static inline unsigned long KSwitch3(unsigned int knum, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3)
{
    uintptr_t param[3] = {0};
    uint8_t num = 0;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;

    SERVICETABLE[knum].fun(knum, param, num );
}

static inline unsigned long KSwitch4(unsigned int knum, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3,
                                    unsigned long arg4)
{
    uintptr_t param[4] = {0};
    uint8_t num = 0;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    SERVICETABLE[knum].fun(knum, param, num );
}

static inline unsigned long KSwitch5(unsigned int knum, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3,
                                    unsigned long arg4, unsigned long arg5)
{
    uintptr_t param[5] = {0};
    uint8_t num = 0;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    param[4] = arg5;
    SERVICETABLE[knum].fun(knum, param, num );
}

static inline unsigned long KSwitch6(unsigned int knum, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3,
                                    unsigned long arg4, unsigned long arg5,
                                    unsigned long arg6)
{
    uintptr_t param[6] = {0};
    uint8_t num = 0;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    param[4] = arg5;
    param[5] = arg6;
    SERVICETABLE[knum].fun(knum, param, num );
}

#endif
#endif
