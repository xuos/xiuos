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
* @file:    xs_memory.h
* @brief:   function declaration and structure defintion of memory
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/2
*
*/

#ifndef XS_MEMORY_H
#define XS_MEMORY_H

#include <xsconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MM_PAGE_SIZE
#define MM_PAGE_SIZE                 4096
#endif

#define MM_PAGE_MASK                 (MM_PAGE_SIZE - 1)
#define MM_PAGE_BITS                 12

/* Set static memory block */
#ifndef SMALL_NUMBER_32B
#define SMALL_NUMBER_32B            (0)     /* Calculate the numbers of SIZEOF_32B blocks*/
#endif

#ifndef SMALL_NUMBER_64B
#define SMALL_NUMBER_64B            (0)     /* Calculate the numbers of SIZEOF_64B blocks*/
#endif

#ifndef KERNEL_MALLOC
#define KERNEL_MALLOC(sz)            x_malloc(sz)
#endif

#ifndef KERNEL_FREE
#define KERNEL_FREE(ptr)             x_free(ptr)
#endif

#ifndef KERNEL_REALLOC
#define KERNEL_REALLOC(ptr, size)    x_realloc(ptr, size)
#endif      

#ifdef KERNEL_MEMBLOCK
struct MemGather
{
    char                m_name[NAME_NUM_MAX];
    uint8               m_kind;
    uint8               m_sign;

    DoubleLinklistType  m_link;

    void                *m_start_address;
    x_size_t            m_size;
    x_size_t            one_block_size;
    uint8               *m_block_link;

    x_size_t            block_total_number;
    x_size_t            block_free_number;
    DoubleLinklistType  wait_task;
};
typedef struct MemGather *GatherMemType;

x_err_t InitMemGather(struct MemGather *gm_handler, const char  *gm_name, void *begin_address, x_size_t gm_size, x_size_t one_block_size);
x_err_t RemoveMemGather(struct MemGather *gm_handler);
GatherMemType CreateMemGather(const char *gm_name, x_size_t   block_number, x_size_t   one_block_size);
x_err_t DeleteMemGather(GatherMemType gm_handler);
void *AllocBlockMemGather(GatherMemType gm_handler, int32 wait_time);
void FreeBlockMemGather(void *data_block);
#endif

void InitBoardMemory(void *begin_addr, void *end_addr);
#ifdef DATA_IN_ExtSRAM
void ExtSramInitBoardMemory(void *start_phy_address, void *end_phy_address, uint8 extsram_idx);
#endif
void *x_malloc(x_size_t nbytes);
void x_free(void *ptr);
void *x_realloc(void *ptr, x_size_t nbytes);
void *x_calloc(x_size_t count, x_size_t size);

#ifdef USER_APPLICATION
void UserInitBoardMemory(void *begin_addr, void *end_addr);
void *x_umalloc(x_size_t nbytes);
void  x_ufree(void *ptr);
void *x_urealloc(void *ptr, x_size_t nbytes);
void *x_ucalloc(x_size_t count, x_size_t size);
#endif

void MemoryInfo(uint32 *total, uint32 *used, uint32 *max_used);

#ifdef __cplusplus
}
#endif

#endif
