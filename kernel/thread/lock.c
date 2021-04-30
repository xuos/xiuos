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
* @file:    lock.c
* @brief:   system spinlock file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_spinlock.h>
#include <xiuos.h>
#include <xs_assign.h>

#ifdef ARCH_SMP
struct   SpinLockfileOps  spinlock;
/*
 * lock scheduler
 */
static void DisablePreempt(void)
{
    x_base lock = 0;

    lock = DisableLocalInterrupt();
    Assign.assign_lock[GetCpuId()] ++;
    EnableLocalInterrupt(lock);
}

/*
 * enable scheduler
 */
static void EnablePreempt(void)
{
    x_base lock = 0;

    lock = DisableLocalInterrupt();
    Assign.assign_lock[GetCpuId()] --;
    DO_KTASK_ASSIGN;
    EnableLocalInterrupt(lock);
}

 void InitSpinLock(  struct   SpinLockfileOps  * spinlock)
{
        InitHwSpinlock(&spinlock->node_lock.lock);
        spinlock->SPinLock = _SpinLock;
        spinlock->UnlockSpinLock  =_UnlockSpinLock;
        spinlock->UnlockSpinLockIrqRestore = _UnlockSpinLockIrqRestore;
        spinlock->SpinLockIrqSave  =  _SpinLockIrqSave;

}

void _SpinLock(struct   SpinLockfileOps  * spinlock)
{
    DisablePreempt();
    HwLockSpinlock(&spinlock->node_lock.lock);
}

void _UnlockSpinLock(struct   SpinLockfileOps  * spinlock)
{
    HwUnlockSpinlock(&spinlock->node_lock.lock);
    EnablePreempt();
}

x_base _SpinLockIrqSave(struct   SpinLockfileOps  * spinlock)
{
    x_base lock = 0;

    DisablePreempt();

    lock = DisableLocalInterrupt();
    HwLockSpinlock(&spinlock->node_lock.lock);

    return lock;
}

void _UnlockSpinLockIrqRestore(struct   SpinLockfileOps  * spinlock, x_base lock)
{
    HwUnlockSpinlock(&spinlock->node_lock.lock);
    EnableLocalInterrupt(lock);

    EnablePreempt();
}
#endif
/**
 * This function will restore the scheduler lock status
 * 
 */
void RestoreCpusLockStatus(struct TaskDescriptor *task)
{
#ifdef ARCH_SMP
    Assign.smp_os_running_task[GetCpuId()] = task;
    HwUnlockSpinlock(&AssignSpinLock);
#else
    Assign.os_running_task = task;
#endif
}



