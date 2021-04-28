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
* @file:    xs_delay.h
* @brief:   function declaration and structure defintion of delay
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/11
*
*/

#ifndef XS_DELAY_H
#define XS_DELAY_H

#include <xsconfig.h>
#include <xs_kdbg.h>
#include <xs_base.h>
#include <xs_klist.h>

#define TASK_DELAY_INACTIVE 0
#define TASK_DELAY_ACTIVE   1

struct Delay {
	struct TaskDescriptor *task;
	x_ticks_t ticks;
	uint8 status;
	DoubleLinklistType  link;
};
typedef  struct Delay *delay_t;

x_err_t KTaskSetDelay(struct TaskDescriptor *task, x_ticks_t ticks);
x_err_t KTaskUnSetDelay(struct TaskDescriptor *task);
void CheckTaskDelay(void);

#endif