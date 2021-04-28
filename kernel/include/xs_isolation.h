/**
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
* @file:    xs_isolation.h
* @brief:   The macro and callback functions definitions for memory protect
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/14
*
*/

#ifndef  MEMORY_PROTECT_H 
#define  MEMORY_PROTECT_H

#include <xsconfig.h>

#ifdef TASK_ISOLATION
#include <xiuos.h>
#include <board.h>

#if defined(ARCH_ARM) && defined(SURPORT_MPU)
#include <mpu.h>
extern struct Mpu *isolation ;
#define MOMERY_PROTECT_ENABLE
#endif

#if defined(ARCH_RISCV) && defined(SURPORT_PMP)
#include <pmp.h>
extern struct Pmp *isolation ;
#define MOMERY_PROTECT_ENABLE
#endif

#define REGION_TYPE_CODE      (1)
#define REGION_TYPE_DATA      (2)
#define REGION_TYPE_BSS       (3)
#define REGION_TYPE_HEAP      (4)

struct MemoryAccessLimit {
    void    (* Enable)( uint32_t option);
    void    (* Disable)(void);
    x_err_t (* Init)(void **mem_access);
    x_err_t (* InitIsolation)(void **mem_access, x_ubase stack_start , size_t stack_size);
    x_err_t (* AddRegion)(void *mem_access, x_ubase start , size_t size , uint8_t region_type );
    x_err_t (* ClearRegion)(void *mem_access, x_ubase addr);
    void    (* Load)(void *mem_access);
    void    (* Free)(void *mem_access);
    x_bool  (* FaultHandle)(void *mem_access, x_ubase addr);
};

extern struct MemoryAccessLimit mem_access ;

#endif
#endif