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
* @file:    xs_service.h
* @brief:   maroc and switch table definition of kernel switch 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_SERVICE_H
#define XS_SERVICE_H
#include <stdint.h>
#include "xs_base.h"

#ifdef SEPARATE_COMPILE
// #include <board.h>

enum KernelService
{ 
    KS_USER_PRINT_INFO = 0,
    KS_USER_TASK_CREATE , 
    KS_USER_TASK_STARTUP , 
    KS_USER_TASK_DELETE,
    KS_USER_TASK_EXECEXIT,
    KS_USER_TASK_CORE_COMBINE,
    KS_USER_TASK_CORE_UNCOMBINE,
    KS_USER_TASK_DELAY,
    KS_USER_GET_TASK_NAME,
    KS_USER_GET_TASK_ID,
    KS_USER_GET_TASK_STAT,
    KS_USER_GET_TASK_COMBINEED_CORE,
    KS_USER_GET_TASK_RUNNING_CORE,
    KS_USER_GET_TASK_ERROR_STATUS,
    KS_USER_GET_TASK_PRIORITY,

    KS_USER_MALLOC,
    KS_USER_FREE,

    KS_USER_MUTEX_CREATE,
    KS_USER_MUTEX_DELETE,
    KS_USER_MUTEX_OBTAIN,
    KS_USER_MUTEX_ABANDON,

    KS_USER_SEMAPHORE_CREATE,
    KS_USER_SEMAPHORE_DELETE,
    KS_USER_SEMAPHORE_OBTAIN,
    KS_USER_SEMAPHORE_ABANDON,
    KS_USER_SEMAPHORE_SETVALUE,


    KS_USER_EVENT_CREATE,
    KS_USER_EVENT_DELETE,
    KS_USER_EVENT_TRIGGER,
    KS_USER_EVENT_PROCESS,

    KS_USER_MSGQUEUE_CREATE,   
    KS_USER_MSGQUEUE_DELETE,   
    KS_USER_MSGQUEUE_SENDWAIT, 
    KS_USER_MSGQUEUE_SEND,     
    KS_USER_MSGQUEUE_URGENTSEND,
    KS_USER_MSGQUEUE_RECV,
    KS_USER_MSGQUEUE_REINIT,

	KS_USER_OPEN,
    KS_USER_READ,
    KS_USER_WRITE,
    KS_USER_CLOSE,
    KS_USER_IOCTL,
    KS_USER_LSEEK,
    KS_USER_RENAME,
    KS_USER_UNLINK,
    KS_USER_STAT,
    KS_USER_FS_STAT,
    KS_USER_FS_SYNC,
    KS_USER_FTRUNCATE,
    KS_USER_MKDIR,
    KS_USER_OPENDIR,
    KS_USER_CLOSEDIR,
    KS_USER_READDIR,
    KS_USER_RMDIR,
    KS_USER_CHDIR,
    KS_USER_GETCWD,
    KS_USER_TELLDIR,
    KS_USER_SEEKDIR,
    KS_USER_REWIND_DIR,
    KS_USER_STAT_FS,

    KS_USER_END

  };
#define SERVICETABLE ((struct Kernel_Service*)SERVICE_TABLE_ADDRESS)
#endif

typedef uintptr_t (*kservice)(uint32_t knum,uintptr_t *param,uint8_t param_num);
struct Kernel_Service
{
	const kservice fun;
    const uint8_t param_num;
};

extern struct Kernel_Service g_service_table[] ;

#endif