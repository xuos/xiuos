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

#include "stm32f4xx.h"
#include "mpu.h"
#include "board.h"
#include <xs_isolation.h>


void MpuEnable(uint32_t option)
{
    MPU->CTRL = MPU_ENABLE | option;
    __DSB();
    __ISB();
    return ;
}

void MpuDisable(void)
{
    __DMB();
    MPU->CTRL = 0;
    return;
}

void MpuClearRegion(uint8_t region)
{
    __DMB();
    MPU->RNR  = region;
    MPU->RBAR = 0;
    MPU->RASR = 0;
    return;
}

void MpuClearAllRegion(void)
{
    __DMB();
    for (int i = 0 ; i < 8 ; i++) {
        MPU->RNR  = i;
        MPU->RBAR = 0;
        MPU->RASR = 0;
    }
    return;
}

int8_t MpuAllocateRegion(void *task_mpu)
{
    if(task_mpu == NONE)
       return -1;
    struct Mpu *mpu = (struct Mpu *)task_mpu;
    int8_t free_region;

    if ( mpu->count < MPU_MAX_REGION_NUM - 1) {
        free_region = mpu->count;
        mpu->count ++;
    }
    else
      free_region = -1;

    return free_region ;
}


size_t MpuLog2Ceil(size_t size)
{
    size_t csize = 0;

    while (size > 0) {
        size >>= 1;
        csize ++;
    }
    
    return csize;
}

int8_t MpuAddRegion(void *task_mpu, x_base addr , size_t size , uint8_t type)
{

    if(task_mpu == NONE)
       return -1;
    struct Mpu *mpu = (struct Mpu *)task_mpu;
    uint32_t l2size;
    int8_t region ;
    
    region = MpuAllocateRegion(mpu);
    if (region < 0 ) {
        // KPrintf("MPU region full \n");
        return region;
    }
    uint32_t flag = 0;
    switch (type)
    {
    case REGION_TYPE_CODE:
        flag = MPU_RASR_AP_RO_RO  | MPU_RASR_TEX_0 | MPU_RASR_C  | MPU_RASR_S;
        break;
    case REGION_TYPE_DATA:
        flag = MPU_RASR_AP_RW_RW | MPU_RASR_TEX_0 | MPU_RASR_C  | MPU_RASR_S ;
        break;
    case REGION_TYPE_BSS:
        flag = MPU_RASR_AP_RW_RW | MPU_RASR_TEX_0 | MPU_RASR_C  | MPU_RASR_S ;
        break;
    case REGION_TYPE_HEAP:
        flag = MPU_RASR_AP_RW_RW | MPU_RASR_TEX_0 | MPU_RASR_C  | MPU_RASR_S ;
        break;
    
    default:
        break;
    }

    l2size  = MpuLog2Ceil(size);
    addr = MPU_ALIGN(addr , 1 << l2size );
    // KPrintf( "region:%d , size : 0x%08x, l2size: %d  , mpu_size : 0x%08x \n",region, size, l2size , MPU_RASR_REGION_SIZE(l2size) );
    mpu->region[region].config.rasr =  flag |MPU_RASR_REGION_SIZE(l2size)  | MPU_ENABLE;
    mpu->region[region].config.rbar =  addr | MPU_RBAR_VALID | region ;     //rbar must set region number

    return region;
}

x_err_t MpuInit(void **task_mpu)
{
    
    x_base   addr_start;
    uint32_t addr_size;
    uint32_t flag;

    if (MPU->TYPE == 0) {
        KPrintf("mpu not surport \n");
        return -1 ;
    }
    // KPrintf("mpu init ...\n");

    struct Mpu *mpu;
    mpu = (struct Mpu *)malloc(sizeof(struct Mpu));
    memset(mpu, 0, sizeof(struct Mpu));

    if (mpu == NONE)
       return -ENOMEMORY;

    MpuDisable();
    MpuClearAllRegion(); //clear 

    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    //user flash
    addr_start = USER_TEXT_START;
    addr_size  = USER_TEXT_END - USER_TEXT_START;
    MpuAddRegion(mpu, addr_start , addr_size, REGION_TYPE_CODE);


    //user sram 
    addr_start = USER_SRAM_START;
    addr_size  = USER_SRAM_END - USER_SRAM_START;
    MpuAddRegion(mpu, addr_start , addr_size, REGION_TYPE_HEAP);
    *task_mpu = mpu;

    return EOK;
}

void MpuFree(void *task_mpu)
{
    if(task_mpu == NONE)
       return;
    MpuDisable();
    MpuClearAllRegion(); //clear 
    x_free(task_mpu);
}

void MpuLoad(void *task_mpu)
{
    // KPrintf("MPU load .. \n");
    MpuDisable();
    MpuClearAllRegion(); //clear 
    if(task_mpu == NONE)
       return;
    struct Mpu *mpu = (struct Mpu *)task_mpu;

    uint8_t region = 0 ;

#if 0
    for ( region = 0 ; region <= mpu->count -1  ; region ++ ){
        KPrintf("region: %d\n",region);
        KPrintf("rasr 0x%08x \n",mpu->region[region].config.rasr);
        KPrintf("rbar 0x%08x \n",mpu->region[region].config.rbar);
        KPrintf("\n");
    }
#endif

   for ( region = 0 ; region <= mpu->count -1 ; region ++ ){
       MPU->RNR  = region;
       MPU->RBAR = mpu->region[region].config.rbar;
       MPU->RASR = mpu->region[region].config.rasr;
   }
   __DSB();
   __ISB();
   MpuEnable(MPU_PRIVDEFENA);
}