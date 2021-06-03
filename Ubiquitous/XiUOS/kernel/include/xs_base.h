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
* @file:    xs_base.h
* @brief:   basic data type defintions 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_BASE_H
#define XS_BASE_H

/*  import  board special configuration */
#include <xsconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0

/* the basic types of date*/
#ifdef ARCH_CPU_64BIT
typedef  unsigned  long int size_t;
typedef  signed long int   ssize_t; 
#else
 typedef unsigned int size_t;
typedef signed int   ssize_t;  
#endif

#endif


#ifndef CONFIG_ARCH_DATA_TYPE
typedef signed   char                   int8;      
typedef signed   short                  int16;    
typedef signed   int                    int32;     
typedef unsigned char                   uint8;    
typedef unsigned short                  uint16;    
typedef unsigned int                    uint32;    

#ifdef ARCH_CPU_64BIT
typedef signed long                     int64;    
typedef unsigned long                   uint64;    
#else
typedef signed long long                int64;     
typedef unsigned long long              uint64;    
#endif
#endif

typedef int                             x_bool;      
typedef long                            x_base;      
typedef unsigned long                   x_ubase;     

typedef x_base                          x_err_t;       
typedef uint32                          x_time_t;      
typedef uint32                          x_ticks_t;      
typedef x_base                          x_flag_t;     
typedef x_ubase                         x_size_t;      
typedef x_ubase                         x_dev_t;      
typedef x_base                          x_OffPos;      

#define RET_TRUE                        1          
#define RET_FALSE                       0         

#define UINT8_SIZE_MAX                  0xff          
#define UINT16_SIZE_MAX                 0xffff          
#define UINT32_SIZE_MAX                 0xffffffff      
#define TICK_SIZE_MAX                   UINT32_SIZE_MAX   


/* the type of components*/
#define Cmpt_KindN_Null                 0                    
#define Cmpt_KindN_Task                 1             
#define Cmpt_KindN_Semaphore            2
#define Cmpt_KindN_Mutex                3                                               
#define Cmpt_KindN_Event                4               
#define Cmpt_KindN_MessageQueue         5                               
#define Cmpt_KindN_MemPool              6             
#define Cmpt_KindN_Timer                7                  
#define Cmpt_KindN_Bus                  8               
#define Cmpt_KindN_Static               0x80


/* the type of error */
#define   EOK                           0
#define   ERROR                         1
#define   ETIMEOUT                      2
#define   EFULL                         3
#define   EEMPTY                        4
#define   ENOMEMORY                     5
#define   ENONESYS                      6
#define   EDEV_BUSY                     7
#define   EPIO                          8
#define   EINTER                        9
#define   EINVALED                      10
#define   INVALID_TASK_ERROR            11



#define ALIGN_MEN_UP(size, align)           ((((size) + (align) - 1) /(align))*(align))
#define ALIGN_MEN_DOWN(size, align)         ((size)/(align)*(align))
#define NONE                                (0)

#ifndef SECTION
    #if defined(__GNUC__)
        #define SECTION(x)                  __attribute__((section(x)))
    #else
        #define SECTION(x)
    #endif
#endif

#define WAITING_FOREVER              -1                 

void KPrintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
