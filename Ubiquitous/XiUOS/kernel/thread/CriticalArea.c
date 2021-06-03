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
* @file:    critical area.c
* @brief:   the critical area lock functions definitions
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#ifndef  __CRITICAL__AREA__
#define  __CRITICAL__AREA__

#include <xs_isr.h>
#include <xs_ktask_stat.h>
#include <xs_assign.h>
#include <xs_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef   ARCH_SMP
 HwSpinlock _CriticalLock;

/**
 * This function will do OsAssign lock.
 *
 * @return lock
 */
x_base CriticalAreaLock(void)
{
    x_base lock = 0;

    lock = DisableLocalInterrupt();

    if (Assign.smp_os_running_task[GetCpuId()]) {
        if (Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt == 0) {
            Assign.assign_lock[GetCpuId()] ++;
            HwLockSpinlock(&_CriticalLock);
        }

        Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt ++; 
    }
    return lock;
}



/**
 * This function will do OsAssign unlock.
 *
 * @return lock
 */
void CriticalAreaUnLock(x_base lock)
{
    if (Assign.smp_os_running_task[GetCpuId()]) {
        if (Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt > 0) {
            Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt --;
        }
    
        if (Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt == 0) {
            if(Assign.assign_lock[GetCpuId()] > 0)
                Assign.assign_lock[GetCpuId()]--;
            HwUnlockSpinlock(&_CriticalLock);
        }
    } 

    EnableLocalInterrupt(lock);
}

/**
 * This function will get critical lock  level.
 *
 * @return critical lock level
 */
uint16 GetOsAssignLockLevel(void)
{
    return Assign.smp_os_running_task[GetCpuId()]->task_smp_info.critical_lock_cnt;
}

#else

#include <xs_hook.h>

static int16 KTaskOsAssignLockNest;

/**
 * This function will get critical lock  level.
 *
 * @return critical lock level
 */
uint16 GetOsAssignLockLevel(void) {
    return KTaskOsAssignLockNest;
}

inline void ResetCriticalAreaLock(void ) {
    KTaskOsAssignLockNest = 0;
}
/**
 * This function will do OsAssign lock.
 *
 * @return lock
 */
x_base CriticalAreaLock(void) {
    x_base lock;

    lock = DISABLE_INTERRUPT();

    KTaskOsAssignLockNest ++;

    return lock;
}

/**
 * This function will do OsAssign unlock.
 *
 * @return lock
 */
void CriticalAreaUnLock(x_base lock) {
    if (KTaskOsAssignLockNest >= 1) {
        KTaskOsAssignLockNest --;
    }

    ENABLE_INTERRUPT(lock);
}

#endif

#ifdef __cplusplus
}
#endif


#endif 