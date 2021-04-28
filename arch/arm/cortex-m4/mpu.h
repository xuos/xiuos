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
#ifndef MPU_H
#define MPU_H

#include <xs_base.h>
#include <xs_klist.h>


#define MPU_ALIGN(size, align)           (((size)) & ~((align) - 1))

#define USER_TEXT_START   (uintptr_t)( USERSPACE )
#define USER_TEXT_END     (uintptr_t)( USERSPACE->us_textend ) 
#define USER_SRAM_START   (uintptr_t)( USERSPACE->us_datastart )
#define USER_SRAM_END     (uintptr_t)( USER_MEMORY_END_ADDRESS )



#define MPU_MAX_REGION_NUM       8
#define MPU_SYS_REGION_RESERVER  8


/* MPU Control Register Bit Definitions */
#define MPU_ENABLE         (1 << 0)  /* Bit 0: Enable the MPU */
#define MPU_HFNMIENA       (1 << 1)  /* Bit 1: Enable MPU during hard fault, NMI, and FAULTMAS */
#define MPU_PRIVDEFENA     (1 << 2)  /* Bit 2: Enable privileged access to default memory map */

#define MPU_RBAR_VALID          (1 << 4)

#define MPU_RASR_ENABLE         (1 << 0)                   /* Bit 0: Region enable */
#define MPU_RASR_REGION_SIZE(n) ((n-1) << 1)

#define MPU_RASR_AP_NO_NO    (0 << 24)  /* P:None U:None */
#define MPU_RASR_AP_RW_NO    (1 << 24)  /* P:RW   U:None */
#define MPU_RASR_AP_RW_RO    (2 << 24)  /* P:RW   U:RO   */
#define MPU_RASR_AP_RW_RW    (3 << 24)  /* P:RW   U:RW   */
#define MPU_RASR_AP_RO_NO    (5 << 24)  /* P:RO   U:None */
#define MPU_RASR_AP_RO_RO    (6 << 24)  /* P:RO   U:RO   */

#define MPU_RASR_RASR_XN     (1 << 28)                 /* Bit 28: Instruction access disable */

#define MPU_RASR_SRD_MASK     (0xff << 8)
#define MPU_RASR_SRD_0        (0x01 << 8)
#define MPU_RASR_SRD_1        (0x02 << 8)
#define MPU_RASR_SRD_2        (0x04 << 8)
#define MPU_RASR_SRD_3        (0x08 << 8)
#define MPU_RASR_SRD_4        (0x10 << 8)
#define MPU_RASR_SRD_5        (0x20 << 8)
#define MPU_RASR_SRD_6        (0x40 << 8)
#define MPU_RASR_SRD_7        (0x80 << 8)

#define MPU_RASR_TEX_SHIFT    (19)      /* Bits 19-21: TEX Address Permission */
#define MPU_RASR_TEX_MASK     (7 << 19)
#define MPU_RASR_TEX_0        (0 << 19) /* Strongly Ordered */
#define MPU_RASR_TEX_1        (1 << 19) /* Normal           */
#define MPU_RASR_TEX_2        (2 << 19) /* Device           */
#define MPU_RASR_TEX_BB(bb)   ((4|(bb)) << 19)

#define MPU_RASR_B            (1 << 16)                  /* Bit 16: Bufferable */
#define MPU_RASR_C            (1 << 17)                  /* Bit 17: Cacheable */
#define MPU_RASR_S            (1 << 18)                  /* Bit 18: Shareable */

struct MpuConfig 
{
    uint32_t rasr;
    uint32_t rbar;
};

struct MpuRegion 
{
    x_base start;
    x_base size;
    struct MpuConfig config;
};

struct Mpu 
{

    uint8_t count;
    struct MpuRegion region[MPU_MAX_REGION_NUM];
};


void MpuEnable( uint32_t option);
void MpuDisable(void);
x_err_t MpuInit(void **task_mpu);
void MpuLoad(void *task_mpu);

#endif
