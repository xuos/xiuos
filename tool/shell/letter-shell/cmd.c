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

#include <xiuos.h>
#include "shell.h"

#include <bus.h>
#include <xs_ktask_stat.h>
#include <string.h>



#define LIST_FIND_OBJ_NR          8

long Hello(void)
{
    KPrintf("Hello World!\n");

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                Hello, Hello, say Hello world);

extern void ShowBanner(void);
long Version(void)
{
    ShowBanner();

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                Version,Version, show xios Version information);

static __inline void ObjectSplit(int len)
{
    while (len--) KPrintf("-");
}

typedef struct
{
    DoubleLinklistType *list;
    DoubleLinklistType **array;
    uint8 type;
    int nr;
    int nr_out;
} ListGetNext_t;

extern DoubleLinklistType xiaoshan_task_head;

#ifdef KERNEL_SEMAPHORE
extern DoubleLinklistType k_sem_list;
#endif

#ifdef KERNEL_MUTEX
extern DoubleLinklistType k_mutex_list;
#endif

#ifdef KERNEL_EVENT
extern DoubleLinklistType k_event_list;
#endif

#ifdef KERNEL_MESSAGEQUEUE
extern DoubleLinklistType k_mq_list;
#endif

#ifdef KERNEL_SOFTTIMER
extern DoubleLinklistType k_timer_list;
#endif

#ifdef KERNEL_MEMBLOCK
extern DoubleLinklistType xiaoshan_memgather_head;
#endif

extern DoubleLinklistType bus_linklist;

static DoubleLinklistType * GetTypelistManagelist(uint8 type)
{
	DoubleLinklistType *list = NONE;
	switch(type)
	{
		case Cmpt_KindN_Task:
			list = &xiaoshan_task_head;
			break;
		case Cmpt_KindN_Semaphore:
#ifdef KERNEL_SEMAPHORE
			list = &k_sem_list;
#endif
			break;
		case Cmpt_KindN_Mutex:
#ifdef KERNEL_MUTEX
			list = &k_mutex_list;
#endif
			break;
		case Cmpt_KindN_Event:
#ifdef KERNEL_EVENT
			list = &k_event_list;
#endif
			break;

		case Cmpt_KindN_MessageQueue:
#ifdef KERNEL_MESSAGEQUEUE
		    list = &k_mq_list;
#endif
			break;
		case Cmpt_KindN_MemPool:
#ifdef KERNEL_MEMBLOCK
		    list = &xiaoshan_memgather_head;
#endif
			break;
		case Cmpt_KindN_Timer:
#ifdef KERNEL_SOFTTIMER
		    list = &k_timer_list;
#endif
			break;
        case Cmpt_KindN_Bus:
		    list = &bus_linklist;
			break;
		default:
			break;
	}
	return list;
}

static void ListFindManagelistInit(ListGetNext_t *p, uint8 type, DoubleLinklistType **array, int nr)
{
	DoubleLinklistType *list;

	list = GetTypelistManagelist(type);
	if (NONE == list) {
		return;
	}
	p->list = list;
	p->type = type;
	p->array = array;
	p->nr = nr;
	p->nr_out = 0;
}

static DoubleLinklistType *ListGetNext(DoubleLinklistType *current, ListGetNext_t *arg)
{
    int first_flag = 0;
    x_ubase level;
    DoubleLinklistType *node, *list;
    DoubleLinklistType **array;
    int nr;

    arg->nr_out = 0;

    if (!arg->nr || !arg->type) {
        return (DoubleLinklistType *)NONE;
    }

    list = arg->list;

    if (!current) {
        node = list;
        first_flag = 1;
    } else {
        node = current;
    }

    level = CriticalAreaLock();

    if (!first_flag) {
        struct CommonMember *obj;
        obj = SYS_DOUBLE_LINKLIST_ENTRY(node, struct CommonMember, list);
        if ((obj->type & ~Cmpt_KindN_Static) != arg->type) {
            CriticalAreaUnLock(level);
            return (DoubleLinklistType *)NONE;
        }
    }

    nr = 0;
    array = arg->array;
    while (1) {
        node = node->node_next;

        if (node == list) {
            node = (DoubleLinklistType *)NONE;
            break;
        }
        nr++;
        *array++ = node;
        if (nr == arg->nr) {
            break;
        }
    }
    
    CriticalAreaUnLock(level);
    arg->nr_out = nr;
    return node;
}

long ShowTask(void)
{
    x_ubase level;
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;
    const char *item_title = "task";
    int maxlen;

    ListFindManagelistInit(&find_arg, Cmpt_KindN_Task, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;
#ifndef SCHED_POLICY_FIFO
#ifdef ARCH_SMP
    KPrintf("%-*.s cpu pri  status      sp     stack size max used left tick  error\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " --- ---  ------- ---------- ----------  ------  ---------- ---\n");
#else
    KPrintf("%-*.s pri  status      sp     stack size max used left tick  error\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " ---  ------- ---------- ----------  ------  ---------- ---\n");
#endif
#else
#ifdef ARCH_SMP
    KPrintf("%-*.s cpu pri  status      sp     stack size max used  error\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " --- ---  ------- ---------- ----------  ------   ---\n");
#else
    KPrintf("%-*.s pri  status      sp     stack size max used  error\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " ---  ------- ---------- ----------  ------   ---\n");
#endif
#endif
    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct TaskDescriptor *obj;
                struct TaskDescriptor task_info, *task;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct TaskDescriptor, link);
                level = CriticalAreaLock();

                memcpy(&task_info, obj, sizeof task_info);
                CriticalAreaUnLock(level);

                task = (struct TaskDescriptor*)obj;
                {
                    uint8 stat;
                    uint8 *ptr;

#ifdef ARCH_SMP
                    KPrintf("%-*.*s %3d %3d ", maxlen, NAME_NUM_MAX, task->task_base_info.name, task->task_smp_info.runing_coreid, task->task_dync_sched_member.cur_prio);

#else
                    KPrintf("%-*.*s %3d ", maxlen, NAME_NUM_MAX, task->task_base_info.name, task->task_dync_sched_member.cur_prio);
#endif
                    stat = KTaskStatGet(task);
                    if (stat == KTASK_READY)        KPrintf(" ready  ");
                    else if (stat == KTASK_SUSPEND) KPrintf(" suspend");
                    else if (stat == KTASK_INIT)    KPrintf(" init   ");
                    else if (stat == KTASK_CLOSE)   KPrintf(" close  ");
                    else if (stat == KTASK_RUNNING) KPrintf(" running");
#ifndef SCHED_POLICY_FIFO

                    ptr = (uint8 *)task->task_base_info.stack_start;
                    while (*ptr == '#')ptr ++;

                    KPrintf(" 0x%08x 0x%08x    %02d%%   0x%08x %03d\n",
                            task->task_base_info.stack_depth + ((x_ubase)task->task_base_info.stack_start - (x_ubase)task->stack_point),
                            task->task_base_info.stack_depth,
                            (task->task_base_info.stack_depth - ((x_ubase) ptr - (x_ubase) task->task_base_info.stack_start)) * 100
                            / task->task_base_info.stack_depth,
                            task->task_dync_sched_member.rest_timeslice,
                            task->exstatus);

#else
                    ptr = (uint8 *)task->task_base_info.stack_start;
                    while (*ptr == '#')ptr ++;

                    KPrintf(" 0x%08x 0x%08x    %02d%%    %03d\n",
                            task->task_base_info.stack_depth + ((x_ubase)task->task_base_info.stack_start - (x_ubase)task->stack_point),
                            task->task_base_info.stack_depth,
                            (task->task_base_info.stack_depth - ((x_ubase) ptr - (x_ubase) task->task_base_info.stack_start)) * 100
                            / task->task_base_info.stack_depth,
                            task->exstatus);

#endif
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowTask,ShowTask, list task information);

static void ShowWaitQueue(struct SysDoubleLinklistNode *list)
{
    struct TaskDescriptor *task;
    struct SysDoubleLinklistNode *node;

    for (node = list->node_next; node != list; node = node->node_next) {
        task = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDescriptor, task_dync_sched_member.sched_link);
        KPrintf("%s", task->task_base_info.name);

        if (node->node_next != list)
            KPrintf("/");
    }
}

#ifdef KERNEL_SEMAPHORE
long ShowSem(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int maxlen;
    const char *item_title = "semaphore";

	ListFindManagelistInit(&find_arg, Cmpt_KindN_Semaphore, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s v   suspend task\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " --- --------------\n");

    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct Semaphore *obj;
                struct Semaphore *sem;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct Semaphore, link);
                sem = obj;
                if (!IsDoubleLinkListEmpty(&sem->pend_list)) {
                    KPrintf("%-*.*d %03d %d:",
                            maxlen, NAME_NUM_MAX,
                            sem->id.id,
                            sem->value,
                            DoubleLinkListLenGet(&sem->pend_list));
                    ShowWaitQueue(&(sem->pend_list));
                    KPrintf("\n");
                } else {
                    KPrintf("%-*.*d %03d %d\n",
                            maxlen, NAME_NUM_MAX,
                            sem->id.id,
                            sem->value,
                            DoubleLinkListLenGet(&sem->pend_list));
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowSem,ShowSem, show xios semaphore in system);

#endif

#ifdef KERNEL_EVENT
long ShowEvent(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int maxlen;
    const char *item_title = "event";

	ListFindManagelistInit(&find_arg, Cmpt_KindN_Event, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s      set    suspend task\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     "  ---------- --------------\n");

    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct Event *obj;
                struct Event *e;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct Event, link);
                e = obj;
                if (!IsDoubleLinkListEmpty(&e->pend_list)) {
                    KPrintf("%-*.*d  0x%x %03d:",
                            maxlen, NAME_NUM_MAX,
                            e->id.id,
                            e->events,
                            DoubleLinkListLenGet(&e->pend_list));
                    ShowWaitQueue(&(e->pend_list));
                    KPrintf("\n");
                } else {
                    KPrintf("%-*.*d  0x%08x 0\n",
                            maxlen, NAME_NUM_MAX, e->id.id, e->events);
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowEvent,ShowEvent, list  event in system);

#endif
#ifdef KERNEL_MUTEX
long ShowMutex(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = NONE;

    int maxlen;
    const char *item_title = "mutex";

	ListFindManagelistInit(&find_arg, Cmpt_KindN_Mutex, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s   owner  hold suspend task cnt\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " -------- ---- --------------\n");

    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct Mutex *obj;
                struct Mutex *m;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct Mutex, link);
                m = obj;
                KPrintf("%-*.*d %-8.*s %04d %d\n",
                        maxlen, NAME_NUM_MAX,
                        m->id.id,
                        NAME_NUM_MAX,
                        m->holder->task_base_info.name,
                        m->recursive_cnt,
                        DoubleLinkListLenGet(&m->pend_list));

            }
        }
    }
    while (next != NONE);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,ShowMutex, ShowMutex,show xios semaphore in system);
#endif

#ifdef KERNEL_MESSAGEQUEUE
long ShowMsgQueue(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int maxlen;
    const char *item_title = "msgqueue";

    ListFindManagelistInit(&find_arg, Cmpt_KindN_MessageQueue, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s entry suspend task\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " ----  --------------\n");
    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct MsgQueue *obj;
                struct MsgQueue *m;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct MsgQueue, link);

                m = obj;
                if (!IsDoubleLinkListEmpty(&m->recv_pend_list))
                {
                    KPrintf("%-*.*d %04d  %d:",
                            maxlen, NAME_NUM_MAX,
                            m->id.id,
                            m->num_msgs,
                            DoubleLinkListLenGet(&m->recv_pend_list));
                    ShowWaitQueue(&(m->recv_pend_list));
                    KPrintf("\n");
                } else {
                    KPrintf("%-*.*d %04d  %d\n",
                            maxlen, NAME_NUM_MAX,
                            m->id.id,
                            m->num_msgs,
                            DoubleLinkListLenGet(&m->recv_pend_list));
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowMsgQueue,ShowMsgQueue, list  message queue in system);

#endif

#ifdef KERNEL_MEMBLOCK
long ShowMemPool(void)
{
    x_ubase level;
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int maxlen;
    const char *item_title = "mempool";

    ListFindManagelistInit(&find_arg, Cmpt_KindN_MemPool, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s block total free suspend task\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " ----  ----  ---- --------------\n");
    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct MemGather *mp;
                int suspend_task_count;
                DoubleLinklistType *node;

                mp = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct MemGather, m_link);

                suspend_task_count = 0;
                DOUBLE_LINKLIST_FOR_EACH(node, &mp->wait_task)
                {
                    suspend_task_count++;
                }

                if (suspend_task_count > 0) {
                    KPrintf("%-*.*s %04d  %04d  %04d %d:",
                            maxlen, NAME_NUM_MAX,
                            mp->m_name,
                            mp->one_block_size,
                            mp->block_total_number,
                            mp->block_free_number,
                            suspend_task_count);
                    ShowWaitQueue(&(mp->wait_task));
                    KPrintf("\n");
                } else {
                    KPrintf("%-*.*s %04d  %04d  %04d %d\n",
                            maxlen, NAME_NUM_MAX,
                            mp->m_name,
                            mp->one_block_size,
                            mp->block_total_number,
                            mp->block_free_number,
                            suspend_task_count);
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowMemPool,ShowMemPool, list  memory pool in system);

#endif
#ifdef KERNEL_SOFTTIMER
long ShowTimer(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int maxlen;
    const char *item_title = "timer";

    ListFindManagelistInit(&find_arg, Cmpt_KindN_Timer, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    maxlen = NAME_NUM_MAX;

    KPrintf("%-*.s  periodic   timeout       flag\n", maxlen, item_title); ObjectSplit(maxlen);
    KPrintf(     " ---------- ---------- -----------\n");
    do {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct Timer *obj;
                struct Timer *timer;

                obj = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct Timer, link);

                timer = obj;
                KPrintf("%-*.*s 0x%08x 0x%08x 0x%08x",
                        maxlen, NAME_NUM_MAX,
                        timer->name,
                        timer->origin_timeslice,
                        timer->deadline_timeslice,
                        timer->id_node.id);
                if (timer->active_status == TIMER_ACTIVE_TRUE)
                    KPrintf("activated\n");
                else
                    KPrintf("deactivated\n");

            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);

    KPrintf("current tick:0x%08x\n", CurrentTicksGain());

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowTimer,ShowTimer, list  timer in system);
#endif

static char *const bus_type_str[] =
{
    "I2C_BUS",
    "SPI_BUS",
    "HWT_BUS",
    "USB_BUS",
    "CAN_BUS",
    "WDT_BUS",
    "SDIO_BUS",
    "TOUCH_BUS",
    "LCD_BUS",
    "PIN_BUS",
    "RTC_BUS",
    "SERIAL_BUS",
    "Unknown"
};

static DriverType showBusFindDriver(struct Bus *bus)
{
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &bus->bus_drvlink;

    for(node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        return driver;
    }
    return NONE;
}

long ShowBus(void)
{
    ListGetNext_t find_arg;
    DoubleLinklistType *obj_list[LIST_FIND_OBJ_NR];
    DoubleLinklistType *next = (DoubleLinklistType*)NONE;

    int i = 0; 
    int dev_cnt, maxlen;
    const char *item_type = "bus_type";
    const char *item_name_0 = "bus_name";
    const char *item_name_1 = "drv_name";
    const char *item_name_2 = "dev_name";
    const char *item_cnt = "cnt";

    ListFindManagelistInit(&find_arg, Cmpt_KindN_Bus, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    KPrintf(" %-15s%-15s%-15s%-15s%-20s\n", item_type, item_name_0, item_name_1, item_name_2, item_cnt); 
    maxlen = 65;
    while (i < maxlen) {
        i++;
        if (maxlen == i) {
            KPrintf("-\n");
        } else {
            KPrintf("-");
        }
    }

    do
    {
        next = ListGetNext(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++) {
                struct Bus *bus;

                bus = SYS_DOUBLE_LINKLIST_ENTRY(obj_list[i], struct Bus, bus_link);
                if (bus) {
                    KPrintf("%s", " ");
                    KPrintf("%-15s%-15s", 
                        bus_type_str[bus->bus_type], 
                        bus->bus_name);
                    
                    DriverType driver = showBusFindDriver(bus);

                    if (driver) {
                        KPrintf("%-15s", driver->drv_name);
                    }  else  {
                        KPrintf("%-15s", "nil");
                    }

                    if (bus->haldev_cnt) {
                        DoubleLinklistType *node = NONE;
                        DoubleLinklistType *head = &bus->bus_devlink;

                        node = head->node_next;
                        dev_cnt = 1;
                        while (node != head)  {
                            HardwareDevType device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);

                            if (1 == dev_cnt)  {
                                if (device)  {
                                    KPrintf("%-16s%-4d\n", device->dev_name, dev_cnt);
                                } else {
                                    KPrintf("%-16s%-4d\n", "nil", dev_cnt);
                                }
                            } else {
                                KPrintf("%46s", " ");
                                if (device)  {
                                    KPrintf("%-16s%-4d\n", device->dev_name, dev_cnt);
                                } else {
                                    KPrintf("%-16s%-4d\n", "nil", dev_cnt);
                                }
                            }
                            dev_cnt++;
                            node = node->node_next;
                        }
                    } else {
                        KPrintf("\n");
                    }
                }
            }
        }
    }
    while (next != (DoubleLinklistType*)NONE);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(1)|SHELL_CMD_DISABLE_RETURN,
                                                ShowBus, ShowBus, show bus all device and driver in system);

#ifdef RESOURCES_RTC
#include "bus_rtc.h"
static void SetDateAndTime(char argc, char **argv)
{
    if (1 == argc) {
        time_t now;
     
        now = time(NONE);
        KPrintf("%s", ctime(&now));
    } else if (argc >= 7) {
        struct RtcSetParam rtc_set_param;

        rtc_set_param.rtc_set_cmd = OPER_RTC_SET_TIME;
        uint16 year;
        uint8 month, day, hour, min, sec;
        year = atoi(argv[1]);
        month = atoi(argv[2]);
        day = atoi(argv[3]);
        hour = atoi(argv[4]);
        min = atoi(argv[5]);
        sec = atoi(argv[6]);
        if (year > 2099 || year < 2000) {
            KPrintf("year is out of range [2000-2099]\n");
            return;
        }
        if (month == 0 || month > 12) {
            KPrintf("month is out of range [1-12]\n");
            return;
        }
        if (day == 0 || day > 31) {
            KPrintf("day is out of range [1-31]\n");
            return;
        }
        if (hour > 23) {
            KPrintf("hour is out of range [0-23]\n");
            return;
        }
        if (min > 59) {
            KPrintf("minute is out of range [0-59]\n");
            return;
        }
        if (sec > 59) {
            KPrintf("second is out of range [0-59]\n");
            return;
        }

        rtc_set_param.date_param.year = year;
        rtc_set_param.date_param.month = month;
        rtc_set_param.date_param.day = day;
        rtc_set_param.time_param.hour = hour;
        rtc_set_param.time_param.minute = min;
        rtc_set_param.time_param.second = sec;

        RtcDrvSetFunction(RTC_DRV_NAME, &rtc_set_param);
    } else {
        KPrintf("only support input [year month day hour min sec]\n");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(9),
                                                SetDateAndTime, SetDateAndTime,  set date and time[year month day hour min sec] );

#endif
