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
* @brief define hifive1-rev-B-board init configure and start-up function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef __BOARD_H__
#define __BOARD_H__

#include <xsconfig.h>
#include <stdint.h>

#define TICK  (RTC_FREQ / TICK_PER_SECOND)

#define CLINT_MTIME_ADDR       (*((volatile uint64_t *)(CLINT_CTRL_ADDR + CLINT_MTIME)))
#define CLINT_MTIMECMP_ADDR    (*((volatile uint64_t *)(CLINT_CTRL_ADDR + CLINT_MTIMECMP)))

#define SERIAL_BUS_NAME "uart0"
#define SERIAL_DRV_NAME "uart0_drv"
#define SERIAL_DEVICE_NAME "uart0_dev0"

#define SURPORT_PMP

#ifdef SEPARATE_COMPILE
#define MEMORY_SIZE    12
#else
#define MEMORY_SIZE    16
#endif

extern  void *_end;
extern  void *_heap_end;
extern  void *__stack_end__;

extern unsigned int g_service_table_start;
extern unsigned int g_service_table_end;

#define MEMORY_START_ADDRESS   (void *)&__stack_end__
#define MEMORY_END_ADDRESS     (void *)( 0x80000000 + MEMORY_SIZE * 1024 )


#ifdef SEPARATE_COMPILE
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
#define USERSPACE (( struct UserSpaceS *)(0x20080000))

#ifndef SERVICE_TABLE_ADDRESS
#define SERVICE_TABLE_ADDRESS    (0x2007F0000)  //in flash
#endif

#define USER_SRAM_SIZE         4
#define USER_MEMORY_START_ADDRESS (USERSPACE->us_bssend)
#define USER_MEMORY_END_ADDRESS   (0x80003000 + USER_SRAM_SIZE * 1024)
#endif

#endif
