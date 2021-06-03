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
* @brief define stm32f407-st-discovery-board init configure and start-up function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: board.h
Description: define stm32f407-st-discovery-board board init function and struct
Others: 
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. define stm32f407-st-discovery-board InitBoardHardware
2. define stm32f407-st-discovery-board data and bss struct
*************************************************/

#ifndef BOARD_H
#define BOARD_H

#include <stm32f4xx.h>
#include <stdint.h>

extern int __stack_end__;
extern unsigned int g_service_table_start;
extern unsigned int g_service_table_end;

#define STM32_USE_SDIO			0

#define STM32_EXT_SRAM          0
#define STM32_EXT_SRAM_BEGIN    0x68000000
#define STM32_EXT_SRAM_END      0x68080000

#define SURPORT_MPU

#ifdef __ICCARM__
extern char __ICFEDIT_region_RAM_end__;
#define STM32_SRAM_END          &__ICFEDIT_region_RAM_end__
#else

#define MEMORY_START_ADDRESS (&__stack_end__)
#define STM32_SRAM_SIZE         128
#define MEMORY_END_ADDRESS      (0x20000000 + STM32_SRAM_SIZE * 1024)

#ifdef SEPARATE_COMPILE
typedef int (*main_t)(int argc, char *argv[]);
typedef void (*exit_t)(void);
struct userspace_s
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
#define USERSPACE (( struct userspace_s *)(0x08080000))

#ifndef SERVICE_TABLE_ADDRESS
#define SERVICE_TABLE_ADDRESS    (0x20000000)
#endif

#define USER_SRAM_SIZE         64
#define USER_MEMORY_START_ADDRESS (USERSPACE->us_bssend)
#define USER_MEMORY_END_ADDRESS   (0x10000000 + USER_SRAM_SIZE * 1024)
#endif
#endif

void InitBoardHardware(void);
void stm32f407_start(void);

#endif