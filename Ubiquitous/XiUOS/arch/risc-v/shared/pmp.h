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

#ifndef  PMP_H
#define  PMP_H

#include <xs_klist.h>

#define PMP_MAX_ENTRY_NUMBER     16
#define PMP_ENTRY_RESERVE         6
#define PMP_ENTRY_MAX 255

#define PMP_R		0x01	/* Allow read */
#define PMP_W		0x02	/* Allow write */
#define PMP_X		0x04	/* Allow execute */
#define PMP_L		0x80	/* PMP entry is locked */
#define PMP_OFF		0x00	/* Null region */
#define PMP_TOR		0x08	/* Top of range */
#define PMP_NA4		0x10	/* Naturally aligned four-byte region */
#define PMP_NAPOT	0x18	/* Naturally aligned power-of-two region */

#define PMP_TOR_TYPE		0	/* Top of range */
#define PMP_NAPOT_TYPE	    1	/* Naturally aligned power-of-two region */

#define PMP_SHIFT_ADDR	2
#define PMP_TYPE_MASK		0x18
#define TO_PMP_ADDR(addr)	((addr) >> PMP_SHIFT_ADDR)
#define FROM_PMP_ADDR(addr)	((addr) << PMP_SHIFT_ADDR)
#define TO_NAPOT_RANGE(size)	(((size) - 1) >> 1)
#define TO_PMP_NAPOT(addr, size)	TO_PMP_ADDR(addr | TO_NAPOT_RANGE(size))

struct PmpEntry
{
    uint8_t pmpcfg;
#if defined(ARCH_RISCV64)
    uint64_t pmpaddr; 
#else
    uint32_t pmpaddr;
#endif
    
};

struct PmpRegionNapot
{
    x_ubase  start ;
    x_ubase  end;
    uint16_t swap_count;
    uint8_t  region_type;
    struct PmpEntry  entry;

    DoubleLinklistType  link;
};

struct PmpRegionTor
{
    x_ubase  start ;
    x_ubase  end;
    uint16_t swap_count;
    uint8_t  region_type;
    struct PmpEntry  entry[2];

    DoubleLinklistType  link;
};

struct Pmp 
{
    uint8_t count;
    uint8_t reserve;
    struct PmpEntry   pmp_entry_reserve[PMP_ENTRY_RESERVE];
    DoubleLinklistType   tor_list;

    DoubleLinklistType   tor_swap_list;
};


x_err_t PmpAddTorRegion(void *task_pmp, x_ubase start , size_t size , uint8_t flag );
x_err_t PmpInitIsolation(void **task_pmp, x_ubase stack_start , size_t stack_size);
void PmpFree(void *task_pmp);
void PmpClear(void);
x_err_t PmpClearRegion(void *task_pmp, x_ubase addr);
void PmpLoad(void *task_pmp);
x_bool PmpAccessFaultHandle(void *task_pmp, x_ubase addr);

#endif
