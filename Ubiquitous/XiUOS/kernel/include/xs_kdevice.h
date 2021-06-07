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
* @file:    xs_kdevice.h
* @brief:   macor and structure defintion of kdevice
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_KDEVICE_H
#define XS_KDEVICE_H

/*  import  board special configuration */
#include <xsconfig.h>
#include <xs_klist.h>

#ifdef TOOL_SHELL
#include "shell.h"
#else
#define SHELL_EXPORT_CMD(_attr, _name, _func, _desc)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OSIGN_OPER_CLOSE                (0U << 0) 
#define OSIGN_OPER_RDONLY             (1U << 0)    
#define OSIGN_OPER_WRONLY            (1U << 1)     
#define OSIGN_OPER_RDWR                 (OSIGN_OPER_WRONLY| OSIGN_OPER_RDONLY )
#define OSIGN_OPER_OPEN                  (1U << 3)  
#define OSIGN_OPER_MASK                  0xf0f  

#define SIGN_OPER_DEACTIVATE        (0U << 0)
#define SIGN_OPER_RDONLY                 (1U << 0)
#define SIGN_OPER_WRONLY                (1U << 1)
#define SIGN_OPER_RDWR                     (SIGN_OPER_RDONLY|SIGN_OPER_WRONLY)
#define SIGN_OPER_REMOVABLE        (1U << 2)
#define SIGN_OPER_STANDALONE      (1U << 3)
#define SIGN_OPER_ACTIVATED           (1U << 4)
#define SIGN_OPER_SUSPENDED        (1U << 5)
#define SIGN_OPER_STREAM                 (1U << 6)
#define SIGN_OPER_INT_RX                   (1U << 8)
#define SIGN_OPER_DMA_RX                 (1U << 9)   
#define SIGN_OPER_INT_TX                   (1U << 10)   
#define SIGN_OPER_DMA_TX                 (1U << 11)

enum SIGN_OPER
{
    OPER_RESUME         = 0x01,
    OPER_SUSPEND      = 0x02,
    OPER_CONFIG         = 0x03,
    OPER_SET_INT        = 0x10,
    OPER_CLR_INT        = 0x11,
    OPER_GET_INT        = 0x12,
    OPER_CHAR_STREAM               = 0x10,
    OPER_BLK_GETGEOME            = 0x10,
    OPER_BLK_SYNC                        = 0x11,
    OPER_BLK_ERASE                      = 0x12,
    OPER_BLK_AUTOREFRESH    = 0x13,
    OPER_NETIF_GETMAC              = 0x10,
    OPER_MTD_FORMAT                 = 0x10,
    OPER_RTC_GET_TIME              = 0x10,
    OPER_RTC_SET_TIME               = 0x11,
    OPER_RTC_GET_ALARM          = 0x12,
    OPER_RTC_SET_ALARM           = 0x13,
};

struct DeviceBlockArrange
{
    uint32 bank_num;
    uint32 size_perbank;
    uint32 block_size;
    uint16 bank_start;
    uint16 bank_end;                      
};

struct DeviceBlockAddr
{
    uint32_t start;
    uint32_t end;
};

#ifdef __cplusplus
}
#endif

#endif
