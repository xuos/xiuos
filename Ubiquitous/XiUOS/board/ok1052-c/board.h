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
* @file board.h
* @brief define imxrt1052-board init configure and start-up function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-28
*/

/*************************************************
File name: board.h
Description: define imxrt1052-board board init function and struct
Others: 
History: 
1. Date: 2021-05-28
Author: AIIT XUOS Lab
Modification: 
1. define imxrt-board InitBoardHardware
2. define imxrt-board heap struct
*************************************************/

#ifndef __BOARD_H__
#define __BOARD_H__

#include "fsl_common.h"
#include "clock_config.h"
#include <xiuos.h>
#include <arch_interrupt.h>

extern int heap_start;
extern int heap_end;
#define HEAP_BEGIN          (&heap_start)
#define HEAP_END            (&heap_end)


#define HEAP_SIZE           ((uint32_t)HEAP_END - (uint32_t)HEAP_BEGIN)

void InitBoardHardware(void);

#endif

