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
* @file:    xs_kdbg.h
* @brief:   function declaration and structure defintion of kernel debug
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_KDBG_H
#define XS_KDBG_H

#include <xsconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KERNEL_DEBUG

/*Kernel Section Debug Define*/
#define KDBG_MEM 0
#define KDBG_MEMHEAP 0
#define KDBG_SCHED 0
#define KDBG_KTASK 0
#define KDBG_SOFTTIMER 0
#define KDBG_IRQ 0
#define KDBG_IPC 0
#define KDBG_HOOK 0

#define SYS_KDEBUG_LOG(section, information)         \
    do                                               \
    {                                                \
        if(section) {                                \
            KPrintf information;                     \
        }                                            \
    }while (0)                                       

#define KDYN_NONE            0
#define KDYN_DBG             1
#define KDYN_ERROR           2
#define KDYN_WARNING         3

#ifdef KDYN_LOG_DBG
#define DBG(args, ...) KDYNAMIC_LOG(KDYN_DBG, args, ##__VA_ARGS__)
#define SYS_ERR(args, ...) KDYNAMIC_LOG(KDYN_ERROR, args, ##__VA_ARGS__)
#define SYS_WARN(args, ...) KDYNAMIC_LOG(KDYN_WARNING, args, ##__VA_ARGS__)
#else
#define DBG(args, ...) KDYNAMIC_LOG(KDYN_NONE, args, ##__VA_ARGS__)
#define SYS_ERR(args, ...) KDYNAMIC_LOG(KDYN_ERROR, args, ##__VA_ARGS__)
#define SYS_WARN(args, ...) KDYNAMIC_LOG(KDYN_WARNING, args, ##__VA_ARGS__)
#endif

#define KDYNAMIC_LOG(level, args, ...)             \
    do                                             \
    {                                              \
        switch(level)                              \
        {                                          \
            case KDYN_NONE:                        \
                break;                             \
            case KDYN_DBG:                         \
                KPrintf("[DBG]");                  \
                KPrintf(args, ##__VA_ARGS__);      \
                break;                             \
            case KDYN_ERROR:                       \
                KPrintf("[ERR]");                  \
                KPrintf(args, ##__VA_ARGS__);      \
                break;                             \
            case KDYN_WARNING:                     \
                KPrintf("[WARN]");                 \
                KPrintf(args, ##__VA_ARGS__);      \
                break;                             \
            default:                               \
                break;                             \
        }                                          \
    }while (0)                                     

#define NULL_PARAM_CHECK(param)            \
    do                                     \
    {                                      \
        if(param == NONE) {                \
            KPrintf("PARAM CHECK FAILED ...%s %d %s is NULL.\n",__FUNCTION__,__LINE__,#param); \
            while(RET_TRUE);               \
        }                                  \
    }while (0)                             

#define CHECK(TRUE_CONDITION)                    \
    do                                            \
    {                                             \
        if(!(TRUE_CONDITION)) {                   \
            KPrintf("%s CHECK condition is false at line[%d] of [%s] func.\n",#TRUE_CONDITION,__LINE__,__FUNCTION__);\
            while(RET_TRUE);                             \
        }                                         \
    } while(0)                                       


#define KDEBUG_NOT_IN_INTERRUPT                   \
    do                                            \
    {                                             \
        x_base level;                             \
        level = DISABLE_INTERRUPT();              \
        if (isrManager.done->getCounter() != 0)   \
        {                                         \
            KPrintf("Function[%s] is not supported in ISR\n", __FUNCTION__);                        \
            CHECK(0);                            \
        }                                         \
        ENABLE_INTERRUPT(level);                  \
    } while (0)                                   

#define KDEBUG_IN_KTASK_CONTEXT                  \
    do                                           \
    {                                            \
        x_base level;                            \
        level = DISABLE_INTERRUPT();             \
        if (GetKTaskDescriptor() == NONE)        \
        {                                        \
            KPrintf("Function[%s] is not supported before task assign\n", __FUNCTION__);            \
            CHECK(0);                           \
        }                                        \
        KDEBUG_NOT_IN_INTERRUPT;                 \
        ENABLE_INTERRUPT(level);                 \
    } while (0)                                  

#define NOT_IN_CRITICAL_AREA                 \
    do {                                     \
        if(GetOsAssignLockLevel() != 0){     \
            KPrintf("Function[%s] is not supported switch in critical area.\n", __FUNCTION__); \
            CHECK(0);                       \
        }                                    \
    } while (0)                      

#else

#define SYS_KDEBUG_LOG(section, information)
#define DBG(args, ...)
#define SYS_ERR(args, ...)
#define SYS_WARN(args, ...)
#define NULL_PARAM_CHECK(param)
#define CHECK(TRUE_CONDITION)
#define KDEBUG_NOT_IN_INTERRUPT
#define KDEBUG_IN_KTASK_CONTEXT
#define NOT_IN_CRITICAL_AREA

#endif

#ifdef __cplusplus
}
#endif

#endif 
