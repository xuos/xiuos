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
* @file:    gatherblock.c
* @brief:   block memory management file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/8
*
*/

#include <xiuos.h>
#include <xs_hook.h>

#ifdef KERNEL_MEMBLOCK

/* a global list, which record all the gatherblocks*/
DoubleLinklistType xiaoshan_memgather_head = {&xiaoshan_memgather_head, &xiaoshan_memgather_head};

/**
 * This function initializes a gather block memory structure.
 *
 * @param gm_handler the gatherblock structure
 * @param gm_name the name of gatherblock
 * @param begin_address the start address of gatherblock
 * @param gm_size the total size of gatherblock
 * @param one_block_size one block size in gatherblock
 *
 * @return EOK
 */
x_err_t InitMemGather(struct MemGather *gm_handler, const char *gm_name, void  *begin_address, x_size_t gm_size, x_size_t  one_block_size)
{
    x_base lock = 0;
    register x_size_t off_block = 0;
    register x_base critical_value = 0;
    uint8 *block_ptr = NONE;
    struct SysDoubleLinklistNode *list_entry = NONE;
    
    /* parameter detection */
    NULL_PARAM_CHECK(gm_handler);
    NULL_PARAM_CHECK(gm_name);
    NULL_PARAM_CHECK(begin_address);

    CHECK(gm_size >= 1 && one_block_size >= 1);

    lock = CriticalAreaLock();

    /* try to find gatherblock object */
    for (list_entry  = xiaoshan_memgather_head.node_next;
            list_entry != &(xiaoshan_memgather_head);
            list_entry  = list_entry->node_next) {
        GatherMemType gm;

        gm = SYS_DOUBLE_LINKLIST_ENTRY(list_entry, struct MemGather, m_link);
        if (gm) {
            CHECK(gm != gm_handler);
        }
    }
    CriticalAreaUnLock(lock);

    /* set the type attribute of gatherblock object */
    gm_handler->m_kind = Cmpt_KindN_Static;
    /* set the name of gatherblock object */
    strncpy(gm_handler->m_name, gm_name, NAME_NUM_MAX);

    critical_value = CriticalAreaLock();

    /* insert gatherblock object into the global list */
    DoubleLinkListInsertNodeAfter(&xiaoshan_memgather_head, &(gm_handler->m_link));

    CriticalAreaUnLock(critical_value);

    /* initialize the other attributes of gatherblock object */
    gm_handler->m_start_address = begin_address;
    gm_handler->m_size = ALIGN_MEN_DOWN(gm_size, MEM_ALIGN_SIZE);
    one_block_size = ALIGN_MEN_UP(one_block_size, MEM_ALIGN_SIZE);
    gm_handler->one_block_size = one_block_size;
    gm_handler->block_total_number = gm_handler->m_size / (gm_handler->one_block_size + sizeof(uint8 *));
    gm_handler->block_free_number  = gm_handler->block_total_number;

    InitDoubleLinkList(&(gm_handler->wait_task));

    /* links all the blocks in gatherblock object */
    block_ptr = (uint8 *)gm_handler->m_start_address;
    for (off_block = 0; off_block < gm_handler->block_total_number; off_block ++) {
        *(uint8 **)(block_ptr + off_block * (one_block_size + sizeof(uint8 *))) =
            (uint8 *)(block_ptr + (off_block + 1) * (one_block_size + sizeof(uint8 *)));
    }

    *(uint8 **)(block_ptr + (off_block - 1) * (one_block_size + sizeof(uint8 *))) =
        NONE;

    gm_handler->m_block_link = block_ptr;

    return EOK;
}



/**
 * This function will remove the gatherblock from the global list, which is created by MemGatherInit function
 *
 * @param gm_handler the gatherblock to be removed
 *
 * @return EOK
 */
x_err_t RemoveMemGather(struct MemGather *gm_handler)
{
    register x_ubase critical_value = 0;
    struct TaskDescriptor *task  = NONE;

    /* parameter detection */
    NULL_PARAM_CHECK(gm_handler);

    CHECK((gm_handler->m_kind & Cmpt_KindN_Static)!=0);

    /* resume all the suspend tasks on gatherblock object */
    while (!IsDoubleLinkListEmpty(&(gm_handler->wait_task))) {
        critical_value = CriticalAreaLock();

        task = SYS_DOUBLE_LINKLIST_ENTRY(gm_handler->wait_task.node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
        task->exstatus = -ERROR;

        KTaskWakeup(task->id.id);

        CriticalAreaUnLock(critical_value);
    }

    /* set the type attribute */
    gm_handler->m_kind = 0;

    critical_value = CriticalAreaLock();

    /*  remove the gatherblock object from the global links */
    DoubleLinkListRmNode(&(gm_handler->m_link));

    CriticalAreaUnLock(critical_value);

    return EOK;
}


/**
 * This function will create a gatherblock object.
 *
 * @param gm_name the name of gatherblock
 * @param block_number the number of blocks in gatherblock
 * @param one_block_size one block size
 *
 * @return EOK on success; NONE on failure
 */
GatherMemType CreateMemGather(const char *gm_name, x_size_t block_number, x_size_t one_block_size)
{
    
    register x_size_t off_block = 0;
    register x_base critical_value = 0;
    uint8 *block_ptr = NONE;
    struct MemGather *gm_handler = NONE;

    KDEBUG_NOT_IN_INTERRUPT;

    /* parameter detection */
    NULL_PARAM_CHECK(gm_name);
    CHECK(block_number >= 1 && one_block_size >= 1);

    KDEBUG_NOT_IN_INTERRUPT;

    /* allocate memory for gatherblock object */
    gm_handler = (struct MemGather *)KERNEL_MALLOC(sizeof(struct MemGather));

    if (NONE == gm_handler)
        return NONE;
    /* clear the gatherblock object */
    memset(gm_handler, 0x0, sizeof(struct MemGather));
    /* set the name attribute */
    strncpy(gm_handler->m_name, gm_name, NAME_NUM_MAX);
    /* set the flag attribute */
    gm_handler->m_sign = 0;

    critical_value = CriticalAreaLock();

    /* insert gatherblock object into the global list */
    DoubleLinkListInsertNodeAfter(&xiaoshan_memgather_head, &(gm_handler->m_link));

    CriticalAreaUnLock(critical_value);

    /* set the other attributes of gatherblock object */
    one_block_size     = ALIGN_MEN_UP(one_block_size, MEM_ALIGN_SIZE);
    gm_handler->one_block_size = one_block_size;
    gm_handler->m_size       = (one_block_size + sizeof(uint8 *)) * block_number;

    /* allocate memory for gather blocks */
    gm_handler->m_start_address = x_malloc((one_block_size + sizeof(uint8 *)) * block_number);
    if (NONE == gm_handler->m_start_address) {
        gm_handler->m_kind = 0;
        critical_value = CriticalAreaLock();

        /*  remove the gatherblock object from the global links */
        DoubleLinkListRmNode(&(gm_handler->m_link));

        CriticalAreaUnLock(critical_value);

        /* release the memory for gather block structure  */
        KERNEL_FREE(gm_handler);

        return NONE;
    }

    gm_handler->block_total_number = block_number;
    gm_handler->block_free_number  = gm_handler->block_total_number;

    InitDoubleLinkList(&(gm_handler->wait_task));

    /* links all the gather blocks */
    block_ptr = (uint8 *)gm_handler->m_start_address;
    for (off_block = 0; off_block < gm_handler->block_total_number; off_block ++) {
        *(uint8 **)(block_ptr + off_block * (one_block_size + sizeof(uint8 *)))
            = block_ptr + (off_block + 1) * (one_block_size + sizeof(uint8 *));
    }

    *(uint8 **)(block_ptr + (off_block - 1) * (one_block_size + sizeof(uint8 *)))
        = NONE;

    gm_handler->m_block_link = block_ptr;

    return gm_handler;
}

/**
 * This function will delete a gatherblock object, which is created by MemGatherCreate function.
 *
 * @param gm_handler the gatherblock object to be deleted
 *
 * @return EOK
 */
x_err_t DeleteMemGather(GatherMemType gm_handler)
{
    register x_ubase critical_value = 0;
    struct TaskDescriptor *task = NONE;

    KDEBUG_NOT_IN_INTERRUPT;

    /* parameter detection */
    NULL_PARAM_CHECK(gm_handler);
    CHECK(((gm_handler->m_kind & Cmpt_KindN_Static)==0));

    /* resume all the suspend tasks on gatherblock object */
    while (!IsDoubleLinkListEmpty(&(gm_handler->wait_task))) {
        critical_value = CriticalAreaLock();

        task = SYS_DOUBLE_LINKLIST_ENTRY(gm_handler->wait_task.node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
        task->exstatus = -ERROR;

        KTaskWakeup(task->id.id);

        CriticalAreaUnLock(critical_value);
    }

    /* release the memory for gather blocks */
    x_free(gm_handler->m_start_address);

    gm_handler->m_kind = 0;

    critical_value = CriticalAreaLock();

    /*  remove the gatherblock object from the global links */
    DoubleLinkListRmNode(&(gm_handler->m_link));

    CriticalAreaUnLock(critical_value);

    /* release the memory for gather block structure  */
    KERNEL_FREE(gm_handler);
    return EOK;
}

/**
 * This function will allocate a data block from gatherblock object
 *
 * @param gm_handler the gatherblock object to get data block
 * @param msec waiting time ,millisecond
 *
 * @return block pointer on success; NONE on failure
 */
void *AllocBlockMemGather(GatherMemType gm_handler, int32 msec)
{
    int32 wait_time = 0;
    uint32 before_sleep = 0;
    register x_base critical_value = 0;
    uint8 *block_ptr = NONE;
    struct TaskDescriptor *task = NONE;

    /* parameter detection */
    NULL_PARAM_CHECK(gm_handler);

    /* get descriptor of task */
    task = GetKTaskDescriptor();
    wait_time = CalculteTickFromTimeMs(msec);

    critical_value = CriticalAreaLock();
    /* no free gatherblock*/
    while (0 == gm_handler->block_free_number) {
        if (wait_time == 0) {
            CriticalAreaUnLock(critical_value);

            KUpdateExstatus(ETIMEOUT);

            return NONE;
        }

        KDEBUG_NOT_IN_INTERRUPT;

        task->exstatus = EOK;

        /* suspend current task */
        SuspendKTask(task->id.id);
        DoubleLinkListInsertNodeAfter(&(gm_handler->wait_task), &(task->task_dync_sched_member.sched_link));

        if (wait_time > 0) {
            before_sleep = CurrentTicksGain();
            /* start the timer */
            KTaskSetDelay(task,wait_time);
        }

        CriticalAreaUnLock(critical_value);

        /* schedule */
        DO_KTASK_ASSIGN;

        if (EOK != task->exstatus)
            return NONE;

        if (wait_time > 0) {
            wait_time -= CurrentTicksGain() - before_sleep;
            if (wait_time < 0)
                wait_time = 0;
        }
        critical_value = CriticalAreaLock();
    }
    /* decrease the number of free gatherblocks */
    gm_handler->block_free_number--;

    block_ptr = gm_handler->m_block_link;
    NULL_PARAM_CHECK(block_ptr);

    /* set the block_list attribute of gather_block structure */
    gm_handler->m_block_link = *(uint8 **)block_ptr;

    *(uint8 **)block_ptr = (uint8 *)gm_handler;

    CriticalAreaUnLock(critical_value);

    HOOK(hook.mem.hook_GmAlloc, (gm_handler,  (uint8 *)(block_ptr + sizeof(uint8 *))));

    return (uint8 *)(block_ptr + sizeof(uint8 *));
}

/**
 * This function will release a data block to gatherblock object
 *
 * @param data_block the data block to be released
 */
void FreeBlockMemGather(void *data_block)
{
    uint8 **block_ptr;
    register x_base critical_value = 0;
    struct MemGather *gm_handler  = NONE;
    struct TaskDescriptor *task = NONE;

    /* parameter detection */
    NULL_PARAM_CHECK(data_block);

    /* get gatherblock structure */
    block_ptr = (uint8 **)((uint8 *)data_block - sizeof(uint8 *));
    gm_handler        = (struct MemGather *)*block_ptr;

    HOOK(hook.mem.hook_GmFree, (gm_handler, data_block));

    critical_value = CriticalAreaLock();

    /* increase the number of gatherblocks */
    gm_handler->block_free_number ++;

    *block_ptr = gm_handler->m_block_link;
    /* set the block_list attribute of gather_block object */
    gm_handler->m_block_link = (uint8 *)block_ptr;

    if (!IsDoubleLinkListEmpty(&(gm_handler->wait_task))) {
        task = SYS_DOUBLE_LINKLIST_ENTRY(gm_handler->wait_task.node_next, struct TaskDescriptor, task_dync_sched_member.sched_link);
        task->exstatus = EOK;

        /* resume a suspend task */
        KTaskWakeup(task->id.id);

        CriticalAreaUnLock(critical_value);

        DO_KTASK_ASSIGN;

        return;
    }

    CriticalAreaUnLock(critical_value);
}
#endif

