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
* @file:    xs_ktask.h
* @brief:   function declaration and structure defintion of kernel task
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_KTASK_H
#define XS_KTASK_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>
#include <xs_timer.h>
#include <xs_delay.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define KTASK_LOWEST_PRIORITY       0
#define KTASK_INIT                  0x00                
#define KTASK_READY                 0x01               
#define KTASK_SUSPEND               0x02                
#define KTASK_RUNNING               0x03                 
#define KTASK_CLOSE                 0x04                
#define KTASK_STAT_MASK             0x07

#define KTASK_MAX_ADVANCE_PROCESS_TIME        0x1    ///< task max advance process times
#define UNCOMBINE_CPU_CORE                    CPU_NUMBERS

#ifdef ARCH_SMP

#define CPU_MASK                     ((1 << CPU_NUMBERS) - 1) /**< All CPUs mask bit. */

#ifndef ASSIGN_IPI
#define ASSIGN_IPI                 0
#endif

#endif

struct TaskBaseInfo {
    char        name[NAME_NUM_MAX];   ///< task name
    void        *func_entry;          ///< task function entry  
    void        *func_param;          ///< task parameter
    uint8       origin_prio;          ///< task init priority
    uint32      stack_depth;          ///< task stack size
    void        *stack_start;         ///< task stack start address
};
typedef struct TaskBaseInfo TaskBaseInfoType;

struct TaskDyncSchedMember {
    uint8               stat;                  ///< task stat
    uint8               advance_cnt;           ///< total time slice advance process count
    uint8               cur_prio;              ///< task current priority
    x_ubase             origin_timeslice;      ///< task init timeslice
    x_ubase             rest_timeslice;        ///< task remaining timeslice
#ifdef  SEPARATE_COMPILE
    uint8               isolation_flag;        ///< task isolation flag
    void                *isolation;            ///< task isolation pointer
    uint8               isolation_status;
#if defined(ARCH_ARM)
    uint32_t            svc_return;
    uint32_t            exc_return;
#endif
#endif

    DoubleLinklistType  sched_link; ///< task sched list
#if KTASK_PRIORITY_MAX > 32
    uint8               bitmap_offset;
    uint8               bitmap_row;
#endif
    uint32              bitmap_column;
    delay_t             delay;
};
typedef struct TaskDyncSchedMember TaskDyncSchedMembeType;

#ifdef ARCH_SMP
struct TaskSmpInfo {
    uint8  combined_coreid;       ///< task bind core id                              
    uint8  runing_coreid;         ///< task running core id                                        
    uint16 critical_lock_cnt;     ///< critical lock count           
};
typedef struct TaskSmpInfo TaskSmpInfoType;
#endif

struct TaskDescriptor
{
    void                    *stack_point;                      ///< stack point
    TaskDyncSchedMembeType  task_dync_sched_member;            ///< task dynamic sched member
    TaskBaseInfoType        task_base_info;                    ///< task base information                                                                                            
#ifdef ARCH_SMP
    TaskSmpInfoType         task_smp_info;                     ///< dual core information
#endif 

#if defined(KERNEL_EVENT)
    uint32                  event_id_trigger : 29 ;            ///< event ops (event trigger )
    uint32                  event_mode : 3;                    ///<  event mode (AND/OR/AUTOCLEAN)
#endif

    x_err_t                 exstatus;                          ///< exception status
    DoubleLinklistType      link;                              ///< manage list                                                                  
    struct IdNode           id;                                ///< task id
    struct KTaskDone        *Done;   
};
typedef struct TaskDescriptor *KTaskDescriptorType;

struct OsAssignReadyVector {
	DoubleLinklistType priority_ready_vector[KTASK_PRIORITY_MAX];
	uint32 priority_ready_group;
#if KTASK_PRIORITY_MAX > 32
	/* Maximum priority level, 256 */
	uint8 ready_vector[32];
#endif
	x_ubase highest_prio;
};

struct KTaskDone
{
   int32  (*init)(KTaskDescriptorType task,
                        const char *name,
                        void (*entry)(void *parameter),
                        void       *parameter,
                        uint32 stack_size,
                        uint8  priority);

   x_err_t (*start)(KTaskDescriptorType task);
   x_err_t (*Delete)(KTaskDescriptorType task);
   x_err_t (*delay)(KTaskDescriptorType task, x_ticks_t ticks);
   x_err_t (*mdelay)(KTaskDescriptorType task,uint32 ms);
   x_err_t (*prioset)(KTaskDescriptorType task, uint8 prio);
#ifdef ARCH_SMP
   x_err_t (*combine)(KTaskDescriptorType task, uint8 coreid);
   x_err_t (*uncombine)(KTaskDescriptorType task);
#endif
   x_err_t (*suspend)(KTaskDescriptorType task);
   x_err_t (*wake)(KTaskDescriptorType task);
};
int32 KTaskCreate(const char *name,

                             void (*entry)(void *parameter),
                             void       *parameter,
                             uint32 stack_size,
                             uint8  priority);
void KTaskQuit(void);

#ifdef  SEPARATE_COMPILE
int32 UTaskCreate(const char *name,
							void (*entry)(void *parameter),
							void       *parameter,
							uint32 stack_size,
							uint8  priority);

#endif

void KUpdateExstatus(x_err_t no);
int *KObtainExstatus(void);
#if !defined(LIB_NEWLIB) && !defined(_WIN32)
#ifndef errno
#define errno    *KObtainExstatus()
#endif
#endif

KTaskDescriptorType GetKTaskDescriptor(void);
KTaskDescriptorType KTaskSearch(char *name);
x_err_t StartupKTask(int32 id);
x_err_t KTaskDelete(int32 id);
x_err_t YieldOsAssign(void);
x_err_t DelayKTask(x_ticks_t tick);
x_err_t MdelayKTask(uint32 ms);
x_err_t KTaskPrioSet(int32 id, uint8 prio);
x_err_t KTaskCoreCombine(int32 id, uint8 coreid);
x_err_t KTaskCoreUnCombine(int32 id);
x_err_t SuspendKTask(int32 id);
x_err_t KTaskWakeup(int32 id);
void KTaskTimeout(void *parameter);
void InitIdleKTask(void);
void IdleKTaskExec(void);
KTaskDescriptorType GetIdleKTaskDescripter(void);
void KTaskOsAssign(void);
void KTaskOsAssignRemoveKTask(struct TaskDescriptor *task);

#if defined (SCHED_POLICY_FIFO)
void FifoTaskTimesliceInit(struct TaskDescriptor *task);
void FifoReadyVectorInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table);
void FifoTaskTimesliceUpdate();
#elif defined (SCHED_POLICY_RR)
void RoundRobinTaskTimesliceInit(struct TaskDescriptor *task);
void RoundRobinReadyVectorInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table);
void RoundRobinTaskTimesliceUpdate(struct TaskDescriptor *task);
#elif defined (SCHED_POLICY_RR_REMAINSLICE)
void RoundRobinRemainTaskTimesliceInit(struct TaskDescriptor *task);
void RoundRobinRemainReadyVectorInsert(struct TaskDescriptor *task, struct OsAssignReadyVector* ready_table);
void RoundRobinRemainTaskTimesliceUpdate(struct TaskDescriptor *task);
#endif

x_err_t LinklistSuspend(DoubleLinklistType *list,struct TaskDescriptor *task,uint8 flag);
x_err_t LinklistResume(DoubleLinklistType *list);
x_err_t LinklistResumeAll(DoubleLinklistType *list);
void HwSendIpi(int ipi_vector, unsigned int cpu_mask);
void KTaskIdDelete(int32 id);

#ifdef __cplusplus
 }
#endif

#endif
