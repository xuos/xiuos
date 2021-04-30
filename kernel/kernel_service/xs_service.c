
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
* @file:    xs_service.c
* @brief:   this file is provided for kernel switch of userapi
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/8
*
*/
#include <string.h>
#include <xs_ktask.h>
#include <xs_service.h>

#ifdef FS_VFS
#include <iot-vfs_posix.h>
#endif
#ifdef TASK_ISOLATION
#include <xs_isolation.h>
#endif
extern inline struct TaskDescriptor *GetTaskWithIdnodeInfo(int32 id);
#ifdef SEPARATE_COMPILE
extern long ShowTask(void);
extern void ShowMemory(void);
extern long ShowSem(void);
extern long ShowEvent(void);
extern long ShowMutex(void);
//extern long ShowMemPool(void);
extern long ShowMsgQueue(void);
//extern long showdevice(void);
extern long ShowTimer(void);
uintptr_t KsPrintInfo(uint32_t knum,uintptr_t *param, uint8_t num )
{
    uintptr_t arg = param[0];
    uintptr_t ret = arg;
    struct TaskDescriptor *tid = NONE;
    KPrintf("ecall from user mode : arg: %d\n", arg );
    switch (arg)
    {
    case 1:
#ifdef TOOL_SHELL
       ShowTask();
#endif
       break;
    case 2:
       ShowMemory();
       break;
    case 3:
#ifdef KERNEL_SEMAPHORE
       //ShowSem();
#endif
       break;
    case 4:
#ifdef KERNEL_EVENT
      ShowEvent();
#endif
       break;
    case 5:
#ifdef KERNEL_MUTEX
       ShowMutex();
#endif
       break;
    case 6:
       //ShowMemPool();
       break;
    case 7:
#ifdef KERNEL_MESSAGEQUEUE
       ShowMsgQueue();
#endif
       break;
    case 8:
       //showdevice();
       break;
   case 9:
#ifdef KERNEL_SOFTTIMER
       ShowTimer();
#endif 
       break;
    default:
       KPrintf("unsurport  \n");
       break;
    }
    return ret;
}

uintptr_t KsTaskCreate(uint32_t knum,uintptr_t *param, uint8_t num)
{
   return  (uintptr_t) UTaskCreate((char *)(param[0]), (void *)(param[1]),(void *)(param[2]),(uint32)(param[3]),(uint8)(param[4]));
   
}

uintptr_t KsStartupTask(uint32_t knum,uintptr_t *param, uint8_t num)
{
    x_err_t ret ;
    int32 id = (int32)(param[0]);
    ret = StartupKTask(id);
    return  (uintptr_t)ret;
}

uintptr_t KsTaskDelete(uint32_t knum,uintptr_t *param, uint8_t num)
{
    x_err_t ret ;
    ret = KTaskDelete((int32) (param[0]));
    return  (uintptr_t)ret;
}

extern void KTaskQuit(void);
uintptr_t KsTaskQuit(uint32_t knum,uintptr_t *param, uint8_t num )
{
    KTaskQuit();
    return 0;
}

uintptr_t KsTaskCoreCombine(uint32_t knum,uintptr_t *param, uint8_t num )
{
    x_err_t ret = EOK ;
    int32 id;
#ifdef ARCH_SMP
    if( param[0] == NONE){
       id = GetKTaskDescriptor()->id.id;
    }else{
       id = (int32 )(param[0]);
    }
   ret = KTaskCoreCombine(id, (uint8)(param[1]));
#endif
   return  (uintptr_t)ret;
}

uintptr_t KsTaskCoreUnCombine(uint32_t knum,uintptr_t *param, uint8_t num )
{
    x_err_t ret = EOK;
    int32 id;
#ifdef ARCH_SMP
    KTaskDescriptorType tid;
    if( param[0] == NONE){
       id = GetKTaskDescriptor()->id.id;
    }else{
       id = (int32 )(param[0]);
    }
   ret = KTaskCoreUnCombine(id);
#endif
   return  (uintptr_t)ret;
}

uintptr_t KsMdelayTask(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = MdelayKTask((int32)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsGetTaskID(uint32_t knum,uintptr_t *param, uint8_t num )
{
   return  (uintptr_t)GetKTaskDescriptor()->id.id;
}

uintptr_t KsGetTaskName(uint32_t knum,uintptr_t *param, uint8_t num )
{
    KTaskDescriptorType task ;
    uint8_t *buf = (uint8_t *)(param[1]);
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
    if ( task == NONE )
       return -ERROR;
      
    strncpy(buf, task->task_base_info.name, NAME_NUM_MAX);
    return EOK;
}

uintptr_t KsGetTaskStat(uint32_t knum,uintptr_t *param, uint8_t num )
{
    KTaskDescriptorType task ;
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
    return  (uintptr_t)task->task_dync_sched_member.stat;
}

uintptr_t KsGetTaskCombinedCore(uint32_t knum,uintptr_t *param, uint8_t num )
{
#ifdef ARCH_SMP
    KTaskDescriptorType task ;
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
    return  (uintptr_t)task->task_smp_info.combined_coreid;
#else
   return  (uintptr_t)0;
#endif
}

uintptr_t KsGetTaskRunningCore(uint32_t knum,uintptr_t *param, uint8_t num )
{
#ifdef ARCH_SMP
    KTaskDescriptorType task ;
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
   return  (uintptr_t)task->task_smp_info.runing_coreid;
#else
   return  (uintptr_t)0;
#endif
}

uintptr_t KsGetTaskErrorstatus(uint32_t knum,uintptr_t *param, uint8_t num )
{
    KTaskDescriptorType task ;
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
    return  (uintptr_t)task->exstatus;
}

uintptr_t KsGetTaskPriority (uint32_t knum,uintptr_t *param, uint8_t num )
{
    KTaskDescriptorType task ;
    task = GetTaskWithIdnodeInfo( (int32)(param[0]));
    return  (uintptr_t)task->task_dync_sched_member.cur_prio;
}

uintptr_t KsMalloc(uint32_t knum,uintptr_t *param, uint8_t num )
{
    uintptr_t *ret =  (uintptr_t *)x_umalloc( (x_size_t)param[0]);
#ifdef MOMERY_PROTECT_ENABLE
   if( mem_access.AddRegion && ret != NONE){
      struct TaskDescriptor *task;
	   task = GetKTaskDescriptor();
      mem_access.AddRegion(task->task_dync_sched_member.isolation, (x_ubase)ret ,  (x_size_t)param[0] , REGION_TYPE_HEAP );
   }
#endif 
    return (uintptr_t)ret;
}

uintptr_t KsFree(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_ufree((void *)(param[0]));
#ifdef MOMERY_PROTECT_ENABLE
   if(mem_access.ClearRegion){
      struct TaskDescriptor *task;
	   task = GetKTaskDescriptor();
	   mem_access.ClearRegion(task->task_dync_sched_member.isolation, (x_ubase)param[0]);
   }
#endif
   return 0;
}
#ifdef KERNEL_MUTEX
uintptr_t KsCreateMutex(uint32_t knum,uintptr_t *param, uint8_t num )
{
    int32 ret;
    ret = KMutexCreate();
    return (uintptr_t)ret;
}

uintptr_t KsDeleteMutex(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret = EOK;
   KMutexDelete((int32)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsMutexObtain(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = KMutexObtain((int32)(param[0]),(int32)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsMutexAbandon(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = KMutexAbandon((int32)(param[0]));
   return  (uintptr_t)ret;
}
#endif
#ifdef KERNEL_SEMAPHORE
uintptr_t KsCreateSemaphore(uint32_t knum,uintptr_t *param, uint8_t num)
{
    int32 ret;
    ret = KSemaphoreCreate((uint16)param[0]);
    return (uintptr_t)ret ;
}

uintptr_t KsDeleteSemaphore(uint32_t knum,uintptr_t *param, uint8_t num )
{
   KSemaphoreDelete((int32)(param[0]));
   return  0;
}

uintptr_t KsSemaphoreObtain(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = KSemaphoreObtain((int32)(param[0]),(int32)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsSemaphoreAbandon(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = KSemaphoreAbandon((int32)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsSemaphoreSetValue(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret ;
   ret = KSemaphoreSetValue((int32)(param[0]),(uint16)(param[1]));
   return  (uintptr_t)ret;
}
#endif

#ifdef KERNEL_EVENT
uintptr_t KsCreateEvent(uint32_t knum,uintptr_t *param, uint8_t num )
{
   EventIdType ret;
   ret = KEventCreate((uint8)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsDeleteEvent(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret = 0;
    KEventDelete((EventIdType)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsEventTrigger(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KEventTrigger((EventIdType)(param[0]), (uint32)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsEventProcess(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KEventProcess((EventIdType)(param[0]), (uint32)(param[1]), (uint8)(param[2]), (int32)(param[3]), (uint32 *)(param[4]) );
   return  (uintptr_t)ret;
}

#endif

#ifdef KERNEL_MESSAGEQUEUE
uintptr_t KsCreateMsgQueue(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int32 ret;
   ret = KCreateMsgQueue((int32)(param[0]), (x_size_t)(param[1]) );
   return  (uintptr_t)ret;
}

uintptr_t KsDeleteMsgQueue(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KDeleteMsgQueue((int32)(param[0]) );
   return  (uintptr_t)ret;
}

uintptr_t KsMsgQueueSendwait(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KMsgQueueSendwait((int32)(param[0]), (const void *)(param[1]), (x_size_t)(param[2]), (int32)(param[3]) );
   return  (uintptr_t)ret;
}

uintptr_t KsMsgQueueSend(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KMsgQueueSend((int32)(param[0]), (const void *)(param[1]), (x_size_t)(param[2]) );
   return  (uintptr_t)ret;
}

uintptr_t KsMsgQueueUrgentSend(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KMsgQueueUrgentSend((int32)(param[0]), (const void *)(param[1]), (x_size_t)(param[2]) );
   return  (uintptr_t)ret;
}

uintptr_t KsMsgQueueRecv(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KMsgQueueRecv((int32)(param[0]), (void *)(param[1]), (x_size_t)(param[2]), (int32)(param[3]) );
   return  (uintptr_t)ret;
}

uintptr_t KsMsgQueueReinit(uint32_t knum,uintptr_t *param, uint8_t num )
{
   x_err_t ret;
   ret = KMsgQueueReinit((int32)(param[0]) );
   return  (uintptr_t)ret;
}
#endif
/* fs posix*/

#ifdef FS_VFS
uintptr_t KsOpen(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = open((const char *)(param[0]), (int)(param[1]), (mode_t)(param[2]));
   return  (uintptr_t)ret;
}

uintptr_t KsRead(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = read((int)(param[0]), (void *)(param[1]), (size_t)(param[2]) );
   return  (uintptr_t)ret;
}

uintptr_t KsWrite(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = write((int)(param[0]), (const void *)(param[1]), (size_t)(param[2]));
   return  (uintptr_t)ret;
}

uintptr_t KsClose(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = close((int)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsIoctl(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = ioctl((int)(param[0]), (int)(param[1]), (void *)(param[2]));
   return  (uintptr_t)ret;
}

uintptr_t KsLseek(uint32_t knum,uintptr_t *param, uint8_t num )
{
   off_t ret;
   ret = lseek((int)(param[0]), (off_t)(param[1]), (int)(param[2]) );
   return  (uintptr_t)ret;
}

uintptr_t KsRename(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = rename((const char *)(param[0]), (const char *)(param[1]) );
   return  (uintptr_t)ret;
}

uintptr_t KsUnlink(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = unlink((const char *)(param[0]) );
   return  (uintptr_t)ret;
}

uintptr_t KsStat(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = stat((const char *)(param[0]), (struct stat *)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsFstat(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = fstat((int)(param[0]), (struct stat *)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsFsync(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = fsync((int)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsFtruncate(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = ftruncate((int)(param[0]), (off_t)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsMkdir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = mkdir((const char *)(param[0]), (mode_t)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsOpendir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   DIR *ret;
   ret = opendir((const char *)(param[0]) );
   return  (uintptr_t)ret;
}

uintptr_t KsClosedir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = closedir((DIR *)(param[0]) );
   return  (uintptr_t)ret;
}

uintptr_t KsReaddir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   struct dirent *ret;
   ret = readdir((DIR *)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsRmdir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = rmdir((const char *)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsChdir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = chdir((const char *)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsGetcwd(uint32_t knum,uintptr_t *param, uint8_t num )
{
   char *ret;
   ret = getcwd((char *)(param[0]), (size_t)(param[1]));
   return  (uintptr_t)ret;
}

uintptr_t KsTelldir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   long ret = 0;
   //ret = telldir((DIR *)(param[0]));
   return  (uintptr_t)ret;
}

uintptr_t KsSeekdir(uint32_t knum,uintptr_t *param, uint8_t num )
{
    seekdir((DIR *)(param[0]), (off_t)(param[1]));
   return 0;
}

uintptr_t KsRewinddir(uint32_t knum,uintptr_t *param, uint8_t num )
{
   rewinddir((DIR *)(param[0]));
   return 0;
}

uintptr_t KsStatfs(uint32_t knum,uintptr_t *param, uint8_t num )
{
   int ret;
   ret = statfs((const char *)(param[0]), (struct statfs *)(param[1]));
   return  (uintptr_t)ret;
}
#endif

struct KernelService g_service_table[256]  __attribute__ ((section (".g_service_table"))) = 
{
	 [KS_USER_PRINT_INFO]           = { KsPrintInfo, 1 },

     /*************** Task ************/
    [KS_USER_TASK_CREATE]          = { KsTaskCreate, 5 },
    [KS_USER_TASK_STARTUP]         = { KsStartupTask, 1 },
    [KS_USER_TASK_DELETE]          = { KsTaskDelete, 1 },
	 [KS_USER_TASK_EXECEXIT]      = { KsTaskQuit, 0 },
    [KS_USER_TASK_CORE_COMBINE]     = { KsTaskCoreCombine, 2 },
    [KS_USER_TASK_CORE_UNCOMBINE]   = { KsTaskCoreUnCombine, 1 },
    [KS_USER_TASK_DELAY]           = { KsMdelayTask, 1 },
    [KS_USER_GET_TASK_NAME]          = { KsGetTaskName, 2 },
    [KS_USER_GET_TASK_ID]           = { KsGetTaskID, 0 },
    [KS_USER_GET_TASK_STAT]         = { KsGetTaskStat, 1 },
    [KS_USER_GET_TASK_COMBINEED_CORE] = { KsGetTaskCombinedCore, 1 },
    [KS_USER_GET_TASK_RUNNING_CORE]  = { KsGetTaskRunningCore, 1 },
    [KS_USER_GET_TASK_ERROR_STATUS]  = { KsGetTaskErrorstatus, 1 },
    [KS_USER_GET_TASK_PRIORITY]     = { KsGetTaskPriority, 1 },

     /*************** Memory ************/
    [KS_USER_MALLOC]              = { KsMalloc, 1 },
    [KS_USER_FREE]                = { KsFree, 1 },
#ifdef KERNEL_MUTEX
     /*************** Mutex ************/
    [KS_USER_MUTEX_CREATE]         = { KsCreateMutex, 0 },
    [KS_USER_MUTEX_DELETE]         = { KsDeleteMutex, 1 },
    [KS_USER_MUTEX_OBTAIN]         = { KsMutexObtain, 2 },
    [KS_USER_MUTEX_ABANDON]        = { KsMutexAbandon, 1 },
#endif
#ifdef KERNEL_SEMAPHORE
     /*************** Semaphore ************/
    [KS_USER_SEMAPHORE_CREATE]     = { KsCreateSemaphore, 1 },
    [KS_USER_SEMAPHORE_DELETE]     = { KsDeleteSemaphore, 1 },
    [KS_USER_SEMAPHORE_OBTAIN]     = { KsSemaphoreObtain, 2 },
    [KS_USER_SEMAPHORE_ABANDON]    = { KsSemaphoreAbandon, 1 },
    [KS_USER_SEMAPHORE_SETVALUE]   = { KsSemaphoreSetValue, 2 },
#endif
     /*************** Event ************/
#ifdef KERNEL_EVENT
    [KS_USER_EVENT_CREATE]         = { KsCreateEvent, 1 },
    [KS_USER_EVENT_DELETE]         = { KsDeleteEvent, 1 },
    [KS_USER_EVENT_TRIGGER]        = { KsEventTrigger, 2 },
    [KS_USER_EVENT_PROCESS]        = { KsEventProcess, 5 },
#endif
#ifdef KERNEL_MESSAGEQUEUE
     /*************** Msg queue ************/
    [KS_USER_MSGQUEUE_CREATE]      = { KsCreateMsgQueue, 2 },
    [KS_USER_MSGQUEUE_DELETE]      = { KsDeleteMsgQueue, 1 },
    [KS_USER_MSGQUEUE_SENDWAIT]    = { KsMsgQueueSendwait, 4 },
    [KS_USER_MSGQUEUE_SEND]        = { KsMsgQueueSend, 3 },
    [KS_USER_MSGQUEUE_URGENTSEND]  = { KsMsgQueueUrgentSend, 3 },
    [KS_USER_MSGQUEUE_RECV]        = { KsMsgQueueRecv, 4 },
    [KS_USER_MSGQUEUE_REINIT]      = { KsMsgQueueReinit, 1 },
#endif
#ifdef FS_VFS
    /*************** fs poxix ************/
    [KS_USER_OPEN]                = { KsOpen , 3 },
    [KS_USER_READ]                = { KsRead , 3 },
    [KS_USER_WRITE]               = { KsWrite , 3 },
    [KS_USER_CLOSE]               = { KsClose , 1 },
    [KS_USER_IOCTL]               = { KsIoctl , 3 },
    [KS_USER_LSEEK]               = { KsLseek , 3 },
    [KS_USER_RENAME]              = { KsRename , 2 },
    [KS_USER_UNLINK]              = { KsUnlink , 1 },
    [KS_USER_STAT]                = { KsStat , 2 },
    [KS_USER_FS_STAT]             = { KsFstat , 2 },
    [KS_USER_FS_SYNC]             = { KsFsync , 1 },
    [KS_USER_FTRUNCATE]           = { KsFtruncate , 2 },
    [KS_USER_MKDIR]               = { KsMkdir , 2 },
    [KS_USER_OPENDIR]             = { KsOpendir , 1 },
    [KS_USER_CLOSEDIR]            = { KsClosedir , 1 },
    [KS_USER_READDIR]             = { KsReaddir , 1 },
    [KS_USER_RMDIR]               = { KsRmdir , 1 },
    [KS_USER_CHDIR]               = { KsChdir , 1 },
    [KS_USER_GETCWD]              = { KsGetcwd, 2 },
    [KS_USER_TELLDIR]             = { KsTelldir, 1 },
    [KS_USER_SEEKDIR]             = { KsSeekdir, 2 },
    [KS_USER_REWIND_DIR]          = { KsRewinddir, 1 },
    [KS_USER_STAT_FS]             = { KsStatfs, 2 },
#endif
    [KS_USER_END ... 255]         = {NONE, 0}

};
#else
struct utask
{
	char        name[NAME_NUM_MAX];         
    void        *func_entry;                
    void        *func_param;     
    int32_t     stack_size;  
    uint8_t     prio; 
};
typedef struct utask UtaskType;
int32_t UserTaskCreate(UtaskType utask)
{
   return KTaskCreate(utask.name, utask.func_entry, utask.func_param,utask.stack_size,utask.prio);
}

x_err_t UserGetTaskName(int32_t id ,char *name)
{
   KTaskDescriptorType task ;
   uint8_t *buf = (uint8_t *)(name);
   task = GetTaskWithIdnodeInfo(id);
   if ( task == NONE )
   return -ERROR;
   
   strncpy(buf, task->task_base_info.name, NAME_NUM_MAX);
   return EOK;
}

int32_t UserGetTaskID(void)
{
   return  (uintptr_t)GetKTaskDescriptor()->id.id;
}

uint8_t UserGetTaskStat(int32_t id)
{
   KTaskDescriptorType task ;
   task = GetTaskWithIdnodeInfo(id);
   return  (uintptr_t)task->task_dync_sched_member.stat;
}

#ifdef ARCH_SMP
uint8_t UserGetTaskCombinedCore(int32_t id)
{
   KTaskDescriptorType task ;
   task = GetTaskWithIdnodeInfo(id);
   return  (uintptr_t)task->task_smp_info.combined_coreid;
}

uint8_t UserGetTaskRunningCore(int32_t id)
{
   KTaskDescriptorType task ;
   task = GetTaskWithIdnodeInfo(id);
   return  (uintptr_t)task->task_smp_info.runing_coreid;
}
#endif

x_err_t UserGetTaskErrorstatus(int32_t id)
{
   KTaskDescriptorType task ;
   task = GetTaskWithIdnodeInfo(id);
   return  (uintptr_t)task->exstatus;
}

uint8_t UserGetTaskPriority(int32_t id)
{
   KTaskDescriptorType task ;
   task = GetTaskWithIdnodeInfo(id);
   return  (uintptr_t)task->task_dync_sched_member.cur_prio;
}

long occupy_g_service_table  __attribute__ ((section (".g_service_table"))) = 0;

#endif