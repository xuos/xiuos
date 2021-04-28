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

#ifndef RISCV_PLIC_H__
#define RISCV_PLIC_H__

#ifndef PLIC_BASE_ADDR
#define PLIC_BASE_ADDR 0x0
#endif

#define PLIC_PRIORITY_OFFSET (0x00000000UL)
#define PLIC_PRIORITY_SHIFT_PER_SOURCE 2

#define PLIC_PENDING_OFFSET (0x00001000UL)
#define PLIC_PENDING_SHIFT_PER_SOURCE 0

#define PLIC_ENABLE_OFFSET (0x00002000UL)
#define PLIC_ENABLE_SHIFT_PER_TARGET 7

#define PLIC_THRESHOLD_OFFSET (0x00200000UL)
#define PLIC_THRESHOLD_SHIFT_PER_TARGET 12

#define PLIC_CLAIM_OFFSET (0x00200004UL)
#define PLIC_CLAIM_SHIFT_PER_TARGET 12

#if defined(__GNUC__) && !defined(__ASSEMBLER__)
__attribute__((always_inline)) static inline void PlicSetFeature(unsigned int feature)
{
    volatile unsigned int *feature_ptr = (volatile unsigned int *)PLIC_BASE_ADDR;
    *feature_ptr = feature;
}

__attribute__((always_inline)) static inline void PlicSetThreshold(unsigned int threshold)
{
    unsigned int hart_id = READ_CSR(mhartid);
    volatile unsigned int *threshold_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                     PLIC_THRESHOLD_OFFSET +
                                                                     (hart_id << PLIC_THRESHOLD_SHIFT_PER_TARGET));
    *threshold_ptr = threshold;
}

__attribute__((always_inline)) static inline void PlicSetPriority(unsigned int source, unsigned int priority)
{
    volatile unsigned int *priority_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                    PLIC_PRIORITY_OFFSET +
                                                                    (source << PLIC_PRIORITY_SHIFT_PER_SOURCE));
    *priority_ptr = priority;
}

__attribute__((always_inline)) static inline void PlicSetPending(unsigned int source)
{
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_PENDING_OFFSET +
                                                                   ((source >> 5) << 2));
    *current_ptr = (1 << (source & 0x1F));
}

__attribute__((always_inline)) static inline void PlicIrqEnable(unsigned int source)
{
    unsigned int hart_id = READ_CSR(mhartid);
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_ENABLE_OFFSET +
                                                                   (hart_id << PLIC_ENABLE_SHIFT_PER_TARGET) +
                                                                   ((source >> 5) << 2));
    unsigned int current = *current_ptr;
    current = current | (1 << (source & 0x1F));
    *current_ptr = current;
}

__attribute__((always_inline)) static inline void PlicIrqDisable(unsigned int source)
{
    unsigned int hart_id = READ_CSR(mhartid);
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_ENABLE_OFFSET +
                                                                   (hart_id << PLIC_ENABLE_SHIFT_PER_TARGET) +
                                                                   ((source >> 5) << 2));
    unsigned int current = *current_ptr;
    current = current & ~((1 << (source & 0x1F)));
    *current_ptr = current;
}

__attribute__((always_inline)) static inline unsigned int PlicIrqClaim(void)
{
    unsigned int hart_id = READ_CSR(mhartid);
    volatile unsigned int *claim_addr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                  PLIC_CLAIM_OFFSET +
                                                                  (hart_id << PLIC_CLAIM_SHIFT_PER_TARGET));
    return *claim_addr;
}

__attribute__((always_inline)) static inline void PlicIrqComplete(unsigned int source)
{
    unsigned int hart_id = READ_CSR(mhartid);
    volatile unsigned int *claim_addr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                  PLIC_CLAIM_OFFSET +
                                                                  (hart_id << PLIC_CLAIM_SHIFT_PER_TARGET));
    *claim_addr = source;
}
#endif 
#endif
