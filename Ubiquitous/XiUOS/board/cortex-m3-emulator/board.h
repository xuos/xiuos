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
* @brief define cortex-m3-emulator init configure and start-up function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-13
*/

#ifndef __BOARD_H__
#define __BOARD_H__


extern  void *__bss_end;
extern  void *_heap_end;
#define MEM_OFFSET  0x20002000 
#define LM3S_SRAM_START         ( ( ((unsigned long)(&__bss_end)) > MEM_OFFSET)? (unsigned long)(&__bss_end):(MEM_OFFSET)  )
#define LM3S_SRAM_END          ( &_heap_end )

#define BSP_USING_UART1
#define SERIAL_BUS_NAME_1 "uart0"
#define SERIAL_DRV_NAME_1 "uart0_drv"
#define SERIAL_DEVICE_NAME_1 "uart0_dev0"



#endif
