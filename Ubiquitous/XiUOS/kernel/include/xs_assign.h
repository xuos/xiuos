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
* @file:   xs_assign.h
* @brief:  function declaration and structure defintion of system assign
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_ASSIGN_H
#define XS_ASSIGN_H

#include <xs_base.h>
#include <xs_ktask.h>
#include <xs_klist.h>
#include <xs_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MERGE_FLAG(src, flag) (*(src) |= flag)
#define CLEAR_FLAG(src, flag)  (*(src) &= ~flag)

extern struct Assign Assign;

struct PriorityReadyVectorDone
{
    void (*init)(struct OsAssignReadyVector *ready_vector);
    void (*insert)(struct TaskDescriptor *task);
    void (*remove)(struct TaskDescriptor *task);
};

#ifdef ARCH_SMP
struct smp_assign_done
{
    x_ubase (*GetHighest)(void);
    struct TaskDescriptor* (*select)(void);
    void (*SwitchToNew)(struct TaskDescriptor* old_task, struct TaskDescriptor* new_task);
    void (*SwitchToFirst)(struct TaskDescriptor* task);
    void (*SetSystemTask)(struct TaskDescriptor* task);
    void (*SmpInit)(void);
};
struct Assign
{
    struct OsAssignReadyVector os_assign_read_vector;
    struct OsAssignReadyVector smp_os_assign_ready_rector[CPU_NUMBERS];
    struct TaskDescriptor *smp_os_running_task[CPU_NUMBERS];

    struct PriorityReadyVectorDone *ready_vector_done;
    struct smp_assign_done *smp_assign_done;

    uint8 current_priority[CPU_NUMBERS];
    uint16 assign_lock[CPU_NUMBERS];
};

void SwitchKtaskContext(x_ubase from, x_ubase to, struct TaskDescriptor *to_task);
void SwitchKtaskContextTo(x_ubase to, struct TaskDescriptor *to_task);
void HwInterruptcontextSwitch( x_ubase from, x_ubase to,struct TaskDescriptor *to_task , void *context);

#else
struct Assign
{
    struct OsAssignReadyVector os_assign_read_vector;
    struct TaskDescriptor *os_running_task;

    struct PriorityReadyVectorDone *ready_vector_done;
    
    uint8 current_priority;
};

void SwitchKtaskContext(x_ubase from, x_ubase to , struct TaskDescriptor *to_task);
void SwitchKtaskContextTo(x_ubase to, struct TaskDescriptor *to_task);
void HwInterruptcontextSwitch( x_ubase from, x_ubase to, struct TaskDescriptor *to_task ,void *context);
#endif

void KTaskInsertToReadyVector(struct TaskDescriptor *task);
int PrioCaculate(uint32 bitmap);
void SysInitOsAssign(void);
void ResetCriticalAreaLock(void );
void StartupOsAssign(void);
x_base CriticalAreaLock(void);
void CriticalAreaUnLock(x_base lock);
uint16 GetOsAssignLockLevel(void);
int32 JudgeAssignReadyBitmapIsEmpty(struct OsAssignReadyVector *ready_vector);
void OsAssignReadyVectorInit(struct OsAssignReadyVector *ready_vector);
void AssignPolicyInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table);
struct TaskDescriptor * ChooseTaskWithHighestPrio(struct OsAssignReadyVector *ready_vector);

#ifdef KERNEL_STACK_OVERFLOW_CHECK
void _KTaskOsAssignStackCheck(struct TaskDescriptor *task);
#endif

#ifdef ARCH_SMP
extern HwSpinlock AssignSpinLock;
#define DO_KTASK_ASSIGN                              \
    do                                               \
    {                                                \
        NOT_IN_CRITICAL_AREA;                        \
        x_base lock;                                 \
        lock = DISABLE_INTERRUPT();                  \
        HwLockSpinlock(&AssignSpinLock);             \
        KTaskOsAssign();                             \
        HwUnlockSpinlock(&AssignSpinLock);           \
        ENABLE_INTERRUPT(lock);                      \
    } while (0);

#else

#define DO_KTASK_ASSIGN                              \
    do                                               \
    {                                                \
        NOT_IN_CRITICAL_AREA;                        \
        x_base lock;                                 \
        lock = DISABLE_INTERRUPT();                  \
        KTaskOsAssign();                             \
        ENABLE_INTERRUPT(lock);                      \
    } while (0);

#endif   

#ifdef __cplusplus
}
#endif

#endif