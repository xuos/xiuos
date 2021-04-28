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
* @file:    xs_queue_manager.h
* @brief:   function declaration and structure defintion of queue manager
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_QUEUE_MANAGER_H
#define XS_QUEUE_MANAGER_H

#include <xsconfig.h>

#ifndef QUEUE_MAX
#define QUEUE_MAX 16
#endif 

typedef struct
{
    void *property;
    void *done;
} queue;

enum QUEUE_TYPE
{
    DATA_QUEUE = 1,
    WORK_QUEUE,
    WAIT_QUEUE,
};

extern void *g_queue_done[];
extern void queuemanager_done_register();

#endif