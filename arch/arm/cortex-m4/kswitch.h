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
Description: arm svc function 
Others: take incubator-nuttx arch/arm/include/armv7-m/syscall.h for references
                https://github.com/apache/incubator-nuttx/tree/master/arch/arm/include/armv7-m/syscall.h
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. Modify function name for a unified  
2. Add some functions when there is no system call
*************************************************/
#ifndef __XS_ARM_M4_KSWITCH_H__
#define __XS_ARM_M4_KSWITCH_H__

#include <stdint.h>
// #include <xs_service.h>
#include "../../../kernel/include/xs_service.h"

#ifdef TASK_ISOLATION
#define KERNEL_SWITCH 0x00

/****************************************************************************
 * kernel switch functions
 ****************************************************************************/

/* SVC call with  call number and no parameters */
static inline unsigned long KSwitch0(unsigned int nbr)
{
    register long reg0 __asm__("r0") = (long)(nbr);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0)
        : "memory"
    );

    return reg0;
}

/* SVC call with call number and one parameter */
static inline unsigned long KSwitch1(unsigned int nbr, unsigned long parm1)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg1 __asm__("r1") = (long)(parm1);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1)
        : "memory"
    );

    return reg0;
}

/* SVC call with  call number and two parameters */
static inline unsigned long KSwitch2(unsigned int nbr, unsigned long parm1,
                                unsigned long  parm2)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg2 __asm__("r2") = (long)(parm2);
    register long reg1 __asm__("r1") = (long)(parm1);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1), "r"(reg2)
        : "memory"
    );

    return reg0;
}


/* SVC call with  call number and four parameters.
 *
 */
static inline unsigned long KSwitch3(unsigned int nbr, unsigned long parm1,
                                  unsigned long parm2, unsigned long parm3)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg1 __asm__("r1") = (long)(parm1);
    register long reg2 __asm__("r2") = (long)(parm2);
    register long reg3 __asm__("r3") = (long)(parm3);



    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1), "r"(reg2),
          "r"(reg3)
        : "memory"
    );

    return reg0;
}

static inline unsigned long KSwitch4(unsigned int nbr, unsigned long parm1,
                                  unsigned long  parm2, unsigned long parm3,
                                  unsigned long  parm4)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg4 __asm__("r4") = (long)(parm4);
    register long reg3 __asm__("r3") = (long)(parm3);
    register long reg2 __asm__("r2") = (long)(parm2);
    register long reg1 __asm__("r1") = (long)(parm1);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1), "r"(reg2),
          "r"(reg3), "r"(reg4)
        : "memory"
    );

    return reg0;
}

/* SVC call with  call number and five parameters.
 *
 */
static inline unsigned long  KSwitch5(unsigned int nbr, unsigned long parm1,
                                  unsigned long parm2, unsigned long parm3,
                                  unsigned long parm4, unsigned long parm5)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg5 __asm__("r5") = (long)(parm5);
    register long reg4 __asm__("r4") = (long)(parm4);
    register long reg3 __asm__("r3") = (long)(parm3);
    register long reg2 __asm__("r2") = (long)(parm2);
    register long reg1 __asm__("r1") = (long)(parm1);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1), "r"(reg2),
          "r"(reg3), "r"(reg4), "r"(reg5)
        : "memory"
    );

    return reg0;
}

/* SVC call with  call number and six parameters.
 *
 */
static inline unsigned long KSwitch6(unsigned int nbr, unsigned long parm1,
                                  unsigned long parm2, unsigned long parm3,
                                  unsigned long parm4, unsigned long parm5,
                                  unsigned long parm6)
{
    register long reg0 __asm__("r0") = (long)(nbr);
    register long reg6 __asm__("r6") = (long)(parm6);
    register long reg5 __asm__("r5") = (long)(parm5);
    register long reg4 __asm__("r4") = (long)(parm4);
    register long reg3 __asm__("r3") = (long)(parm3);
    register long reg2 __asm__("r2") = (long)(parm2);
    register long reg1 __asm__("r1") = (long)(parm1);

    __asm__ __volatile__
    (
        "svc %1"
        : "=r"(reg0)
        : "i"(KERNEL_SWITCH), "r"(reg0), "r"(reg1), "r"(reg2),
          "r"(reg3), "r"(reg4), "r"(reg5), "r"(reg6)
        : "memory"
    );

    return reg0;
}
#else

static inline unsigned long KSwitch0(unsigned int knum)
{
    uintptr_t param[1] = {0};
    uint8_t num = 0;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}

static inline unsigned long KSwitch1(unsigned int knum, unsigned long arg1)
{
    uintptr_t param[1] = {0};
    uint8_t num = 1;
    param[0] = arg1;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}


static inline unsigned long KSwitch2(unsigned int knum, unsigned long arg1,
                                  unsigned long arg2)
{
    uintptr_t param[2] = {0};
    uint8_t num = 2;
    param[0] = arg1;
    param[1] = arg2;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}


static inline unsigned long KSwitch3(unsigned int knum, unsigned long arg1,
                                  unsigned long arg2, unsigned long arg3)
{
    uintptr_t param[3] = {0};
    uint8_t num = 3;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;

    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}

static inline unsigned long KSwitch4(unsigned int knum, unsigned long arg1,
                                  unsigned long arg2, unsigned long arg3,
                                  unsigned long arg4)
{
    uintptr_t param[4] = {0};
    uint8_t num = 4;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}

static inline unsigned long KSwitch5(unsigned int knum, unsigned long arg1,
                                  unsigned long arg2, unsigned long arg3,
                                  unsigned long arg4, unsigned long arg5)
{
    uintptr_t param[5] = {0};
    uint8_t num = 5;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    param[4] = arg5;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}

static inline unsigned long KSwitch6(unsigned int knum, unsigned long arg1,
                                  unsigned long arg2, unsigned long arg3,
                                  unsigned long arg4, unsigned long arg5,
                                  unsigned long arg6)
{
    uintptr_t param[6] = {0};
    uint8_t num = 6;
    param[0] = arg1;
    param[1] = arg2;
    param[2] = arg3;
    param[3] = arg4;
    param[4] = arg5;
    param[5] = arg6;
    (struct Kernel_Service*)SERVICETABLE[knum].fun(knum, param, num);
}

#endif
#endif
