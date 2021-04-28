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
* @file:    isr.c
* @brief:   the general management of system isr
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <xs_hook.h>

struct InterruptServiceRoutines isrManager = { 0} ;

#ifdef ARCH_SMP
extern int GetCpuId(void);
#endif
/**
 * This functionwill get the isr nest level.
 *
 * @return isr nest level
 */
 uint16 GetIsrCounter()
{
    uint16 ret = 0;

#ifdef ARCH_SMP
    ret = isrManager.isr_count[GetCpuId()];
#else
    ret = isrManager.isr_count;
#endif
    return ret;
}

 void incIsrCounter()
{
#ifdef ARCH_SMP
     isrManager.isr_count[GetCpuId()] ++ ;
#else
     isrManager.isr_count ++;
#endif
    return ;
}

 void decIsrCounter()
{

#ifdef ARCH_SMP
     isrManager.isr_count[GetCpuId()] -- ;
#else
     isrManager.isr_count --;
#endif
    return ;
}

 x_bool Is_InIsr()
{
#ifdef ARCH_SMP
    return ( isrManager.isr_count[GetCpuId()] != 0 ? RET_TRUE : RET_FALSE ) ;
#else
    return ( isrManager.isr_count != 0 ? RET_TRUE : RET_FALSE ) ;
#endif

}
/**
 * This function will register a new irq.
 *
 * @param irq_num the number of the irq
 * @param handler the callback of the interrupt
 * @param arg param of thge callback
 *
 * @return EOK on success; ERROR on failure
 */
int32 RegisterHwIrq(uint32 irq_num, IsrHandlerType handler, void *arg)
{
    if (irq_num >=   ARCH_MAX_IRQ_NUM )
        return -ERROR;

    struct IrqDesc *desc = &isrManager.irq_table[irq_num];

    desc->handler = handler;
    desc->param = arg;

    return EOK;
}
/**
 * This function will free a irq.
 *
 * @param irq_num the number of the irq
 *
 * @return EOK on success; ERROR on failure
 */
int32 FreeHwIrq(uint32 irq_num)
{
    if (irq_num >=   ARCH_MAX_IRQ_NUM  )
        return -ERROR;

    memset(&isrManager.irq_table[irq_num], 0, sizeof(struct IrqDesc));

    return EOK;
}

/**
 * This function will enable a irq.
 *
 * @param irq_num the number of the irq
 *
 * @return EOK on success; ERROR on failure
 */
int32 EnableHwIrq(uint32 irq_num)
{
    if (irq_num >=   ARCH_MAX_IRQ_NUM  )
        return -ERROR;

    return ArchEnableHwIrq(irq_num);
}
/**
 * This function will disable a irq.
 *
 * @param irq_num the number of the irq
 *
 * @return EOK on success; ERROR on failure
 */

int32 DisableHwIrq(uint32 irq_num)
{
    if (irq_num >=   ARCH_MAX_IRQ_NUM  )
        return -ERROR;

    return ArchDisableHwIrq(irq_num);
}

/* called from arch-specific ISR wrapper */
void IsrCommon(uint32 irq_num)
{
    struct IrqDesc *desc = &isrManager.irq_table[irq_num];

    if (desc->handler == NONE) {
        SYS_KDEBUG_LOG(KDBG_IRQ, ("Spurious interrupt: IRQ No. %d\n", irq_num));
        while (1) {}
    }
    desc->handler(irq_num, desc->param);

}

 void setIsrSwitchTrigerFlag()
 {

#ifdef ARCH_SMP
    isrManager.isr_switch_trigger_flag[GetCpuId()] = 1;
#else
    isrManager.isr_switch_trigger_flag = 1;
#endif

}

 void clearIsrSwitchTrigerFlag()
 {

#ifdef ARCH_SMP
    isrManager.isr_switch_trigger_flag[GetCpuId()] = 0;
#else
    isrManager.isr_switch_trigger_flag = 0;
#endif
}

 uint8 getIsrSwitchTrigerFlag()
 {
   
#ifdef ARCH_SMP
   return  isrManager.isr_switch_trigger_flag[GetCpuId()];
#else
   return isrManager.isr_switch_trigger_flag ;
#endif

}

struct IsrDone isrDone = {
    Is_InIsr,
    RegisterHwIrq ,
    FreeHwIrq,
    EnableHwIrq,
    DisableHwIrq,
    IsrCommon,
    GetIsrCounter,
    incIsrCounter,
    decIsrCounter,
    getIsrSwitchTrigerFlag,
    setIsrSwitchTrigerFlag,
    clearIsrSwitchTrigerFlag
} ;

void SysInitIsrManager()
{
    extern int __isrtbl_idx_start;
    extern int __isrtbl_start;
    extern int __isrtbl_end;
    memset(&isrManager,0,sizeof(struct InterruptServiceRoutines));
    isrManager.done = &isrDone;

    uint32 *index = (uint32 *)&__isrtbl_idx_start;
    struct IrqDesc *desc = (struct IrqDesc *)&__isrtbl_start;

    while (desc != (struct IrqDesc *)&__isrtbl_end)
        isrManager.irq_table[*index++] = *desc++;
}
