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

#include "pmp.h"
#include <board.h>
#include <encoding.h>
#include <xs_isolation.h>

#define USER_TEXT_START   (uintptr_t)( USERSPACE )
#define USER_TEXT_END     (uintptr_t)( USERSPACE->us_textend ) 
#define USER_SRAM_START   (uintptr_t)( USERSPACE->us_datastart )
#define USER_SRAM_END     (uintptr_t)( USERSPACE->us_bssend )


/**
 * This function add a pmp tor region to task pmp config
 *
 * @param task_pmp the task pmp config structure
 * @param start the memory start address
 * @param size the memory address size
 * @param type the memory type
 *
 * @return EOK
 */
x_err_t PmpAddTorRegion(void *task_pmp, x_ubase start , size_t size , uint8_t type )
{
    if( task_pmp == NONE)
        return -ERROR;
    if (size == 0)
       return EOK;

    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;    
    struct PmpRegionTor *region ;
    region = (struct PmpRegionTor *)x_malloc(sizeof(struct PmpRegionTor ));
    if (region == NONE)
       return -ENOMEM;

    uint8_t flag = 0;
    switch (type)
    {
    case REGION_TYPE_CODE:
        flag = PMP_R | PMP_X ;
        break;
    case REGION_TYPE_DATA :
        flag = PMP_R | PMP_W ;
        break;
    case REGION_TYPE_BSS :
        flag = PMP_R | PMP_W ;
        break;
    case REGION_TYPE_HEAP :
        flag = PMP_R | PMP_W ;
        break;
    
    default:
        break;
    }
    memset(region,0,sizeof(struct PmpRegionTor ));
    InitDoubleLinkList(&(region->link));
    region->region_type = PMP_TOR_TYPE;
    region->start = start;
    region->end = start + size ;
    region->entry[0].pmpcfg = PMP_NA4 | flag;
    region->entry[0].pmpaddr = TO_PMP_ADDR(start);

    region->entry[1].pmpcfg = PMP_TOR | flag;
    region->entry[1].pmpaddr = TO_PMP_ADDR(ALIGN_MEN_UP(region->end,4));


    if (pmp->count <= PMP_MAX_ENTRY_NUMBER - 2 )
    {
        DoubleLinkListInsertNodeAfter(&pmp->tor_list, &region->link);
        pmp->count = pmp->count + 2 ;
    }else
    {
        struct PmpRegionTor *node ;
        node = (struct PmpRegionTor *)pmp->tor_list.node_next ;
        DoubleLinkListRmNode(&(node->link));
        DoubleLinkListInsertNodeAfter(&pmp->tor_list, &region->link);
        DoubleLinkListInsertNodeAfter(&pmp->tor_swap_list, &node->link);
        node->swap_count ++;
    }
    return EOK;
}

/**
 * This function free a pmp region 
 *
 * @param task_pmp the task pmp config structure
 * @param addr the memory address
 *
 * @return EOK
 */
x_err_t PmpClearRegion(void *task_pmp, x_ubase addr)
{
    // KPrintf("PmpClearRegion\n");
    if( task_pmp == NONE)
        return -ERROR;
    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;

    struct PmpRegionTor *tor_node = NONE;
    DoubleLinklistType *link = NONE;

    if (!IsDoubleLinkListEmpty(&pmp->tor_list)) {
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_list) {
            tor_node = CONTAINER_OF(link, struct PmpRegionTor, link);
            if ( addr = tor_node->start  ){
                pmp->count = pmp->count -2;
               goto __free ;
            }
        }
    } 
    if (!IsDoubleLinkListEmpty(&pmp->tor_swap_list)) {
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_swap_list) {
            tor_node = CONTAINER_OF(link, struct PmpRegionTor, link);
            if ( addr = tor_node->start  ) {
               goto __free;
            }
        }
    }

__free: 
    DoubleLinkListRmNode(&(tor_node->link));
    x_free(tor_node);
    return EOK;
}

/**
 * This function init a task pmp config structure and add some pmp region
 *
 * @param task_pmp the task pmp config structure
 * @param stack_start the task stack address
 * @param stack_size the task stack size
 *
 * @return EOK
 */
x_err_t PmpInitIsolation(void **task_pmp, x_ubase stack_start , size_t stack_size)
{

    struct Pmp *pmp ;
    pmp = (struct Pmp *)x_malloc(sizeof(struct Pmp));
    if(pmp == NONE)
       return -ENOMEMORY;
    memset(pmp,0,sizeof(struct Pmp));
    InitDoubleLinkList(&(pmp->tor_list));
    InitDoubleLinkList(&(pmp->tor_swap_list));
    uint8_t index = 0;

    // text 
    if (USER_TEXT_END - USER_TEXT_START > 0 ) {
        pmp->pmp_entry_reserve[index].pmpcfg  = PMP_NA4 | PMP_R | PMP_X ;
        pmp->pmp_entry_reserve[index].pmpaddr = TO_PMP_ADDR(USER_TEXT_START);
        pmp->pmp_entry_reserve[index + 1].pmpcfg  = PMP_TOR | PMP_R | PMP_X ;
        pmp->pmp_entry_reserve[index + 1].pmpaddr = TO_PMP_ADDR(ALIGN_MEN_UP(USER_TEXT_END,4));
        index = index + 2 ;
        pmp->count = pmp->count + 2;
        pmp->reserve = pmp->reserve + 2;
    }
    
    // data & bss 
    if (USER_SRAM_END - USER_SRAM_START > 0 ) {
        pmp->pmp_entry_reserve[index].pmpcfg  = PMP_NA4 | PMP_R | PMP_W  ;
        pmp->pmp_entry_reserve[index].pmpaddr = TO_PMP_ADDR(USER_SRAM_START);
        pmp->pmp_entry_reserve[index + 1].pmpcfg  = PMP_TOR | PMP_R | PMP_W  ;
        pmp->pmp_entry_reserve[index + 1].pmpaddr = TO_PMP_ADDR(ALIGN_MEN_UP(USER_SRAM_END,4));
        index = index + 2 ;
        pmp->count = pmp->count + 2;
        pmp->reserve = pmp->reserve + 2;
    }

    if (stack_size > 0 ) {
        pmp->pmp_entry_reserve[index].pmpcfg  = PMP_NA4 | PMP_R | PMP_W | PMP_X ;
        pmp->pmp_entry_reserve[index].pmpaddr = TO_PMP_ADDR(stack_start);
        pmp->pmp_entry_reserve[index + 1].pmpcfg  = PMP_TOR | PMP_R | PMP_W | PMP_X ;
        pmp->pmp_entry_reserve[index + 1].pmpaddr = TO_PMP_ADDR(ALIGN_MEN_UP((stack_start + stack_size),4));
        pmp->count = pmp->count + 2;
        pmp->reserve = pmp->reserve + 2;
    }

    *task_pmp = (void *)pmp;
    return EOK;
}

/**
 * This function free task pmp config structure
 *
 * @param task_pmp the task pmp config structure
 *
 */
void PmpFree(void *task_pmp){
    // KPrintf("pmp free \n");

    if( task_pmp == NONE)
        return ;
    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;

    struct PmpRegionTor *tor_node = NONE;
    DoubleLinklistType *link = NONE;

    if (!IsDoubleLinkListEmpty(&(pmp->tor_list)) ){
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_list)
        {
            tor_node = CONTAINER_OF(link, struct PmpRegionTor, link);
            DoubleLinkListRmNode(&(tor_node->link));
            x_free(tor_node);
        }
    }

    if (!IsDoubleLinkListEmpty(&pmp->tor_swap_list)) {
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_swap_list) {
            tor_node = CONTAINER_OF(link, struct PmpRegionTor, link);
            DoubleLinkListRmNode(&(tor_node->link));
            x_free(tor_node);
        }
    }

    x_free(pmp);
}

/**
 * This function write value to pmp config register
 * @param index pmpcfg register index
 * @param value pmpcfg register value
 *
 */
void WritePmpcfg(uint8_t index , x_ubase value)
{
    if ( index == 0 ) {
        WRITE_CSR(pmpcfg0, value);
    }else if ( index == 1 ) {
        WRITE_CSR(pmpcfg1, value);
    }else if ( index == 2 ) {
        WRITE_CSR(pmpcfg2, value);
    }else if ( index == 3 ) {
        WRITE_CSR(pmpcfg3, value);
    }
}

/**
 * This function write value to pmp addr register
 * @param index pmpaddr register index
 * @param value pmpaddr register value
 *
 */
void WritePmpaddr(uint8_t index, x_ubase value)
{

    if ( index == 0 ) {
        WRITE_CSR(pmpaddr0, value);
    }else if ( index == 1 ) {
        WRITE_CSR(pmpaddr1, value);
    }else if ( index == 2 ) {
        WRITE_CSR(pmpaddr2, value);
    }else if ( index == 3 ) {
        WRITE_CSR(pmpaddr3, value);
    }else if ( index == 4 ) {
        WRITE_CSR(pmpaddr4, value);
    }else if ( index == 5 ) {
        WRITE_CSR(pmpaddr5, value);
    }else if ( index == 6 ) {
        WRITE_CSR(pmpaddr6, value);
    }else if ( index == 7 ) {
        WRITE_CSR(pmpaddr7, value);
    }else if ( index == 8 ) {
        WRITE_CSR(pmpaddr8, value);
    }else if ( index == 9 ) {
        WRITE_CSR(pmpaddr9, value);
    }else if ( index == 10 ) {
        WRITE_CSR(pmpaddr10, value);
    }else if ( index == 11 ) {
        WRITE_CSR(pmpaddr11, value);
    }else if ( index == 12 ) {
        WRITE_CSR(pmpaddr12, value);
    }else if ( index == 13 ) {
        WRITE_CSR(pmpaddr13, value);
    }else if ( index == 14 ) {
        WRITE_CSR(pmpaddr14, value);
    }else if ( index == 15 ) {
        WRITE_CSR(pmpaddr15, value);
    }
}

/**
 * This function handle user task access memory fault, check addr  in pmp swap list or not ,if addr is in
 * pmp swap list, swap in to pmp list.
 *
 * @param task_pmp the task pmp config structure
 * @param addr access addr when fault
 *
 * @return EOK
 */
x_bool PmpAccessFaultHandle(void *task_pmp, x_ubase addr)
{
    if( task_pmp == NONE)
        return RET_FALSE;
    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;    

    x_bool ret = RET_FALSE;
    uint8_t region_type;
    DoubleLinklistType *link = NONE;
    struct PmpRegionTor *tor_node = NONE;


    if (!IsDoubleLinkListEmpty(&pmp->tor_swap_list) ) {
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_swap_list) {
            tor_node = CONTAINER_OF(link, struct PmpRegionTor, link);
            if ( addr >= tor_node->start && addr <= (tor_node->end) ) {
                region_type = PMP_TOR;
                goto __swap;
            }
        }
    }

    goto __exit;

__swap:

    if ( region_type = PMP_TOR ) {
        if( pmp->count < PMP_MAX_ENTRY_NUMBER - 2) {
            DoubleLinkListRmNode(&(tor_node->link));
            DoubleLinkListInsertNodeBefore(&pmp->tor_list, &tor_node->link);
            pmp->count = pmp->count + 2;
        } else {
            struct PmpRegionTor *swap_node;
            swap_node = (struct PmpRegionTor *)pmp->tor_list.node_next;
            DoubleLinkListRmNode(&(swap_node->link));
            DoubleLinkListRmNode(&(tor_node->link));
            DoubleLinkListInsertNodeBefore(&pmp->tor_list, &tor_node->link);
            DoubleLinkListInsertNodeBefore(&pmp->tor_swap_list, &swap_node->link);
            swap_node->swap_count ++;

        }
        
    }
    ret = RET_TRUE;

__exit:
   return ret;
}

#if defined(ARCH_RISCV64)
/**
 * This function clear pmp registers.
 */
void PmpClear(void)
{
    WRITE_CSR(pmpcfg0, 0);
    WRITE_CSR(pmpcfg2, 0);

    WRITE_CSR(pmpaddr0, 0);
    WRITE_CSR(pmpaddr1, 0);
    WRITE_CSR(pmpaddr2, 0);
    WRITE_CSR(pmpaddr3, 0);
    WRITE_CSR(pmpaddr4, 0);
    WRITE_CSR(pmpaddr5, 0);
    WRITE_CSR(pmpaddr6, 0);
    WRITE_CSR(pmpaddr7, 0);
    WRITE_CSR(pmpaddr8, 0);
    WRITE_CSR(pmpaddr9, 0);
    WRITE_CSR(pmpaddr10, 0);
    WRITE_CSR(pmpaddr11, 0);
    WRITE_CSR(pmpaddr12, 0);
    WRITE_CSR(pmpaddr13, 0);
    WRITE_CSR(pmpaddr14, 0);
    WRITE_CSR(pmpaddr15, 0);
    
}

/**
 * This function load task pmp config to pmp register.
 * @param task_pmp the task pmp config structure
 * 
 */
void PmpLoad(void *task_pmp)
{
    // KPrintf("pmpLoad \n");
    if( task_pmp == NONE)
        return ;
    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;

    PmpClear();
    uint8_t index = 0;
    x_ubase pmpcfg0 = 0 ;
    x_ubase pmpcfg2 = 0 ;


    for( int i = 0 ; i < pmp->reserve; i++){
        pmpcfg0 |=  pmp->pmp_entry_reserve[i].pmpcfg << ( index * 8) ;
        WritePmpaddr(index , pmp->pmp_entry_reserve[i].pmpaddr );
        index ++ ;
    }

    DoubleLinklistType *link = NONE;
    struct PmpRegionTor *node2 = NONE;

    if (!IsDoubleLinkListEmpty(&pmp->tor_list) ){
        DOUBLE_LINKLIST_FOR_EACH(link, &pmp->tor_list)
        {
            node2 = CONTAINER_OF(link, struct PmpRegionTor, link);
            WritePmpaddr(index ,node2->entry[0].pmpaddr );
            if (index < 8)
              pmpcfg0 |= node2->entry[0].pmpcfg << ( index * 8);
            else
              pmpcfg2 |= node2->entry[0].pmpcfg << ( (index - 8) * 8);
            index ++;

            WritePmpaddr(index ,node2->entry[1].pmpaddr );
            if (index < 8)
              pmpcfg0 |= node2->entry[1].pmpcfg << ( index * 8);
            else
              pmpcfg2 |= node2->entry[1].pmpcfg << ( (index - 8) * 8);
            index ++;
        }
    }
    WritePmpcfg(0, pmpcfg0 );
    WritePmpcfg(2, pmpcfg2 );
}

#else

/**
 * This function clear pmp registers.
 */
void PmpClear(void)
{   
    WRITE_CSR(pmpcfg0, 0);
    WRITE_CSR(pmpcfg1, 0);
    WRITE_CSR(pmpcfg2, 0);
    WRITE_CSR(pmpcfg3, 0);

    WRITE_CSR(pmpaddr0, 0);
    WRITE_CSR(pmpaddr1, 0);
    WRITE_CSR(pmpaddr2, 0);
    WRITE_CSR(pmpaddr3, 0);
    WRITE_CSR(pmpaddr4, 0);
    WRITE_CSR(pmpaddr5, 0);
    WRITE_CSR(pmpaddr6, 0);
    WRITE_CSR(pmpaddr7, 0);
    WRITE_CSR(pmpaddr8, 0);
    WRITE_CSR(pmpaddr9, 0);
    WRITE_CSR(pmpaddr10, 0);
    WRITE_CSR(pmpaddr11, 0);
    WRITE_CSR(pmpaddr12, 0);
    WRITE_CSR(pmpaddr13, 0);
    WRITE_CSR(pmpaddr14, 0);
    WRITE_CSR(pmpaddr15, 0);
}
#define PMP_CFG_CONVERSE(a,b,c,d) ( ((uint32_t)(d) << 24) | ((uint32_t)(c) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(a) )

/**
 * This function load task pmp config to pmp register.
 * @param task_pmp the task pmp config structure
 * 
 */
void PmpLoad(void *task_pmp)
{
    // KPrintf("pmpLoad \n");
    if( task_pmp == NONE)
        return ;
    struct Pmp *pmp;
    pmp = (struct Pmp *)task_pmp ;

    PmpClear();
    uint8_t index = 0;
     
    uint8_t  pmpcfg[16] = { 0 }  ;
    uint32_t pmpcfg0 = 0 ;
    uint32_t pmpcfg1 = 0 ;
    uint32_t pmpcfg2 = 0 ;
    uint32_t pmpcfg3 = 0 ;

    for( int i = 0 ; i < pmp->reserve; i++){
        pmpcfg[index] =  pmp->pmp_entry_reserve[i].pmpcfg  ;
        WritePmpaddr(index , pmp->pmp_entry_reserve[i].pmpaddr );
        index ++ ;
    }

    DoubleLinklistType *node = NONE;
    struct PmpRegionTor *tor_node = NONE;
    if (!IsDoubleLinkListEmpty(&(pmp->tor_list)) ){
        DOUBLE_LINKLIST_FOR_EACH(node, &(pmp->tor_list))
        {
            tor_node = CONTAINER_OF(node, struct PmpRegionTor, link);
            WritePmpaddr(index ,tor_node->entry[0].pmpaddr );
            pmpcfg[index] = tor_node->entry[0].pmpcfg;
            index ++;

            WritePmpaddr(index ,tor_node->entry[1].pmpaddr );
            pmpcfg[index] = tor_node->entry[1].pmpcfg;
            index ++;
        }
    }

    pmpcfg0 = PMP_CFG_CONVERSE(pmpcfg[0], pmpcfg[1], pmpcfg[2], pmpcfg[3]);
    pmpcfg1 = PMP_CFG_CONVERSE(pmpcfg[4], pmpcfg[5], pmpcfg[6], pmpcfg[7]);
    pmpcfg2 = PMP_CFG_CONVERSE(pmpcfg[8], pmpcfg[9], pmpcfg[10], pmpcfg[11]);
    pmpcfg3 = PMP_CFG_CONVERSE(pmpcfg[12], pmpcfg[13], pmpcfg[14], pmpcfg[15]);
    WritePmpcfg(0, pmpcfg0);
    WritePmpcfg(1, pmpcfg1);
    WritePmpcfg(2, pmpcfg2);
    WritePmpcfg(3, pmpcfg3);
}

#endif
