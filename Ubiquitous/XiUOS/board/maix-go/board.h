/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file board.h
* @brief define maix-go-board init configure and start-up function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: board.h
Description: define maix-go-board board init function and struct
Others: https://canaan-creative.com/developer
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. define maix-go-board InitBoardHardware
2. define maix-go-board data and bss struct
*************************************************/

#ifndef BOARD_H__
#define BOARD_H__

#include <xsconfig.h>
#include <stdint.h>
#include <xs_service.h>

extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern unsigned int __stack_end__;
extern unsigned int g_service_table_start;
extern unsigned int g_service_table_end;


#ifdef SEPARATE_COMPILE
#define G_SERVICE_TABLE_LENGTH (0x1000)

#define MEMORY_START_ADDRESS    (void*)&__stack_end__
#define MEMORY_END_ADDRESS      (void*)((0x80000000 + 1 * 1024 * 1024)) /* 1M SRAM */

typedef int (*main_t)(int argc, char *argv[]);
typedef void (*exit_t)(void);
struct UserSpaceS
{
  main_t    us_entrypoint;
  exit_t    us_taskquit;
  uintptr_t us_textstart;
  uintptr_t us_textend;
  uintptr_t us_datasource;
  uintptr_t us_datastart;
  uintptr_t us_dataend;
  uintptr_t us_bssstart;
  uintptr_t us_bssend;
  uintptr_t us_heapend;
};
#define USERSPACE (( struct UserSpaceS *)(MEMORY_END_ADDRESS + G_SERVICE_TABLE_LENGTH))

#ifndef SERVICE_TABLE_ADDRESS
#define SERVICE_TABLE_ADDRESS    (0x80100000)
#endif

#define USER_MEMORY_START_ADDRESS (USERSPACE->us_bssend)

#define USER_MEMORY_END_ADDRESS         (void*)((0x80000000 + 6 * 1024 * 1024) )

#else

#define MEMORY_START_ADDRESS    (void*)&__stack_end__
#define MEMORY_END_ADDRESS      (void*)(0x80000000 + 6 * 1024 * 1024)
#endif



void InitBoardHardware(void);

#endif
