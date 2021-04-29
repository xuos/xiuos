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
* @file board.c
* @brief support hifive1-rev-B-board init configure and start-up
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <board.h>
#include <platform.h>
#include <encoding.h>
#include <arch_interrupt.h>
#include <xiuos.h>
#include <device.h>
#include <plic_driver.h>

extern void use_default_clocks(void);
extern void use_pll(int refsel, int bypass, int r, int f, int q);
plic_instance_t g_plic = {0} ;

void InitBoardHardware(void)
{

    /* initialize the system clock */
    use_default_clocks();
    use_pll(0, 0, 1, 31, 1);

    /* initialize hardware interrupt */
    asm volatile(
        "la t0, save_hw_context\n"
        "csrw mtvec, t0"
    );
    
    PLIC_init(&g_plic,
            PLIC_CTRL_ADDR,
            PLIC_NUM_INTERRUPTS,
            PLIC_NUM_PRIORITIES);
 
    SET_CSR(mie, MIP_MEIP);

    /* initialize timer*/
    CLINT_MTIMECMP_ADDR = CLINT_MTIME_ADDR + TICK;
    SET_CSR(mie, MIP_MTIP);

    extern int InitHwUart(void);
    InitHwUart();
    InstallConsole(SERIAL_BUS_NAME, SERIAL_DRV_NAME, SERIAL_DEVICE_NAME);

	KPrintf("\nconsole init completed.\n");
    KPrintf("board initialization......\n");

    KPrintf("memory address range: [0x%08x - 0x%08x], size: %d\n", (x_ubase) MEMORY_START_ADDRESS, (x_ubase) MEMORY_END_ADDRESS, MEMORY_SIZE);
    /* initialize memory system */
	InitBoardMemory(MEMORY_START_ADDRESS, MEMORY_END_ADDRESS);

    return;
}

