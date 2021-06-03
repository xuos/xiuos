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
* @file:    isolation.c
* @brief:   memory access structure definitions of ARM/RISCV
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_isolation.h>

#ifdef TASK_ISOLATION

#if defined(ARCH_ARM) && defined(SURPORT_MPU)

struct Mpu *isolation = NONE;
struct MemoryAccessLimit mem_access = {
    .Enable = MpuEnable ,
    .Disable    = MpuDisable , 
    .Init       =  MpuInit , 
    .InitIsolation = NONE,
    .AddRegion  = NONE,
    .ClearRegion = NONE ,
    .Free    =  NONE,
    .Load = MpuLoad,
    .FaultHandle = NONE,
};
#endif

#if defined(ARCH_RISCV) && defined(SURPORT_PMP)

struct Pmp  *isolation = NONE;
struct MemoryAccessLimit  mem_access = {
    .Enable = NONE ,
    .Disable    = NONE , 
    .Init       =  NONE ,
    .InitIsolation = PmpInitIsolation,
    .AddRegion  = PmpAddTorRegion,
    .ClearRegion = PmpClearRegion ,
    .Free    =  PmpFree,
    .Load = PmpLoad,
    .FaultHandle = PmpAccessFaultHandle,
};

#endif

#endif