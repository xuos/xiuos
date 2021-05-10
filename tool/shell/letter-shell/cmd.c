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
                                                Version,Version, show XiUOS Version information);

static __inline void PrintSpace(int len)
{
    while (len--) KPrintf(" ");
}

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

long ShowTask(void)
{
    int k = 0;
    x_ubase lock = 0;
    uint8 stat = 0;
    uint8 *ptr = NONE;
    const char *item_title = "TASK";
    struct TaskDescriptor *task = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("*************************************************************************************************\n");
#ifdef ARCH_SMP
    for(k = 0;k < CPU_NUMBERS; k++){
        KPrintf(" CPU[%d] CURRENT RUNNING TASK[ %s ] PRI[ %d ] \n",k,Assign.smp_os_running_task[k]->task_base_info.name,Assign.smp_os_running_task[k]->task_dync_sched_member.cur_prio);
    }
#else
    KPrintf(" CURRENT RUNNING TASK[ %s ] PRI[ %d ]\n",Assign.os_running_task->task_base_info.name,Assign.os_running_task->task_dync_sched_member.cur_prio);
#endif

#ifdef SCHED_POLICY_RR_REMAINSLICE
    KPrintf(" SCHED POLICY [ RR_REMAINSLICE ]\n");
#endif
#ifdef SCHED_POLICY_RR
    KPrintf(" SCHED POLICY [ RR ]\n");
#endif
#ifdef SCHED_POLICY_FIFO
    KPrintf(" SCHED POLICY [ FIFO ]\n");
#endif

    KPrintf("*************************************************************************************************\n");
#ifndef SCHED_POLICY_FIFO
#ifdef ARCH_SMP
    KPrintf(" STAT    ID  %-*.s   PRI CORE STACK_DEPTH USED  LEFT_TICKS  ERROR_STAT\n", NAME_NUM_MAX, item_title); 
#else
    KPrintf(" STAT    ID  %-*.s  PRI STACK_DEPTH  USED  LEFT_TICKS  ERROR_STAT\n", NAME_NUM_MAX, item_title);
#endif
#else
#ifdef ARCH_SMP
    KPrintf(" STAT    ID  %-*.s   PRI CORE STACK_DEPTH USED  ERROR_STAT\n", NAME_NUM_MAX, item_title); 
#else
    KPrintf(" STAT    ID  %-*.s  PRI STACK_DEPTH  USED  ERROR_STAT\n", NAME_NUM_MAX, item_title);
#endif
#endif
    
    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &xiaoshan_task_head) {
        task = CONTAINER_OF(nodelink, struct TaskDescriptor, link);
        if(NONE != task) {
            stat = KTaskStatGet(task);
            if (stat == KTASK_READY)        KPrintf(" READY  ");
            else if (stat == KTASK_SUSPEND) KPrintf(" SUSPEND");
            else if (stat == KTASK_INIT)    KPrintf(" INIT   ");
            else if (stat == KTASK_CLOSE)   KPrintf(" CLOSE  ");
            else if (stat == KTASK_RUNNING) KPrintf(" RUNNING");

#ifdef ARCH_SMP
            KPrintf("%3d  %-*.*s  %3d %3d ", task->id.id,NAME_NUM_MAX, NAME_NUM_MAX, task->task_base_info.name, task->task_dync_sched_member.cur_prio,task->task_smp_info.runing_coreid );

#else
            KPrintf("%3d  %-*.*s %3d ", task->id.id,NAME_NUM_MAX, NAME_NUM_MAX, task->task_base_info.name, task->task_dync_sched_member.cur_prio);
#endif
                    
#ifndef SCHED_POLICY_FIFO
            ptr = (uint8 *)task->task_base_info.stack_start;
            while (*ptr == '#')ptr ++;

            KPrintf("     %-10d%d%%     %-10d%04d\n",
                    task->task_base_info.stack_depth,
                    (task->task_base_info.stack_depth - ((x_ubase) ptr - (x_ubase) task->task_base_info.stack_start)) * 100 / task->task_base_info.stack_depth,
                    task->task_dync_sched_member.rest_timeslice, task->exstatus);

#else
            ptr = (uint8 *)task->task_base_info.stack_start;
            while (*ptr == '#')ptr ++;

            KPrintf("     %-10d%d%%     %04d\n",
                    task->task_base_info.stack_depth,
                    (task->task_base_info.stack_depth - ((x_ubase) ptr - (x_ubase) task->task_base_info.stack_start)) * 100 / task->task_base_info.stack_depth, task->exstatus);

#endif
        }

    }
    CriticalAreaUnLock(lock);    

    KPrintf("*************************************************************************************************\n");

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowTask,ShowTask, show task information);

static void GetSuspendedTask(struct SysDoubleLinklistNode *list)
{
    struct TaskDescriptor *task = NONE;
    DoubleLinklistType *nodelink = NONE;
    DOUBLE_LINKLIST_FOR_EACH(nodelink, list){
        task = CONTAINER_OF(nodelink, struct TaskDescriptor, task_dync_sched_member.sched_link);
        if(NONE != task) {
            KPrintf("%s | ", task->task_base_info.name);
        }
    }
}

#ifdef KERNEL_SEMAPHORE

long ShowSem(void)
{
    x_base lock = 0;
    struct Semaphore *sem = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("******************************************************************\n");
    KPrintf(" SEM_ID  VALUE   SUSPEND_TASK\n");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &k_sem_list) {
        sem = CONTAINER_OF(nodelink, struct Semaphore, link);
        if(NONE != sem) {
            if (!IsDoubleLinkListEmpty(&sem->pend_list)) {
                KPrintf("  %-8d%-8d :", sem->id.id, sem->value);
                GetSuspendedTask(&(sem->pend_list));
                KPrintf("\n");
            } else {
                KPrintf("  %-8d%-8d NONE\n", sem->id.id, sem->value);
            }
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("******************************************************************\n");

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowSem,ShowSem, show semaphore in system);

#endif

#ifdef KERNEL_EVENT

long ShowEvent(void)
{
    x_base lock = 0;
    struct Event *event = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("******************************************************************\n");
    KPrintf(" EVENT_ID  OPTIONS   EVENTS       SUSPEND_TASK\n");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &k_event_list) {
        event = CONTAINER_OF(nodelink, struct Event, link);
        if(NONE != event) {
            if (!IsDoubleLinkListEmpty(&event->pend_list)) {
                KPrintf("   %-8d0x%-8x0x%-12x", event->id.id, event->options, event->events);
                GetSuspendedTask(&(event->pend_list));
                KPrintf("\n");
            } else {
                KPrintf("   %-8d0x%-8x0x%-12x NONE\n", event->id.id, event->options, event->events);
            }
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("******************************************************************\n");
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,
                                                ShowEvent,ShowEvent, show event in system);

#endif
#ifdef KERNEL_MUTEX
long ShowMutex(void)
{
    x_base lock = 0;
    struct Mutex *mutex = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("******************************************************************\n");
    KPrintf("MUTEX_ID  HOLDER  RECRUSIVE_CNT  SUSPEND_TASK\n");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &k_mutex_list) {
        mutex = CONTAINER_OF(nodelink, struct Mutex, link);
        if(NONE != mutex) {
            if (!IsDoubleLinkListEmpty(&mutex->pend_list)) {
                KPrintf("  %-8d%-10s%-12d", mutex->id.id, (mutex->holder == NONE ?"NONE":(mutex->holder->task_base_info.name)), mutex->recursive_cnt);
                GetSuspendedTask(&(mutex->pend_list));
                KPrintf("\n");
            } else {
                KPrintf("  %-8d%-10s%-12d NONE\n", mutex->id.id, (mutex->holder == NONE ?"NONE":(mutex->holder->task_base_info.name)), mutex->recursive_cnt);
            }
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("******************************************************************\n");

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN,ShowMutex, ShowMutex,show mutex in system);
#endif

#ifdef KERNEL_MESSAGEQUEUE
long ShowMsgQueue(void)
{
    x_base lock = 0;
    struct MsgQueue *msgq = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("****************************************************************************************\n");
    KPrintf(" MSG_ID NUM_MSGS EACH_LEN MAX_MSGS %-32s%s\n","SEND_SUSPEND_TASK","RECV_SUSPEND_TASK");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &k_mq_list) {
        msgq = CONTAINER_OF(nodelink, struct MsgQueue, link);
        if(NONE != msgq) {
            KPrintf("   %-8d%-8d%-8d%-8d", msgq->id.id, msgq->num_msgs, msgq->each_len,msgq->max_msgs);
            if (!IsDoubleLinkListEmpty(&msgq->send_pend_list)) {
                GetSuspendedTask(&(msgq->send_pend_list));
            } else {
                PrintSpace(32);
            }
            
            if (!IsDoubleLinkListEmpty(&msgq->recv_pend_list)){
                GetSuspendedTask(&(msgq->recv_pend_list));
            } 

            KPrintf("\n");
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("****************************************************************************************\n");

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, ShowMsgQueue,ShowMsgQueue, show message queue in system);

#endif

#ifdef KERNEL_MEMBLOCK
long ShowGatherMem(void)
{
    x_base lock = 0;
    struct MemGather *mg = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("****************************************************************************************\n");
    KPrintf("GATHERMEM BLOCK TOTAL FREE SUSPEND_TASK\n");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &xiaoshan_memgather_head) {
        mg = CONTAINER_OF(nodelink, struct MemGather, m_link);
        if(NONE != mg) {
            if (!IsDoubleLinkListEmpty(&mg->wait_task)) {
                 KPrintf("   %-8s%-6d%-6d%-6d :",
                            mg->m_name,
                            mg->one_block_size,
                            mg->block_total_number,
                            mg->block_free_number);
                GetSuspendedTask(&(mg->wait_task));
                KPrintf("\n");
            } else {
                KPrintf("   %-8s%-6d%-6d%-6d NONE\n",
                            mg->m_name,
                            mg->one_block_size,
                            mg->block_total_number,
                            mg->block_free_number);
            }
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("****************************************************************************************\n");

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, ShowGatherMem,ShowGatherMem, show memory block in system);

#endif
#ifdef KERNEL_SOFTTIMER
long ShowTimer(void)
{
    x_base lock = 0;
    struct Timer *timer = NONE;
    DoubleLinklistType *nodelink = NONE;

    KPrintf("****************************************************************************************\n");
    KPrintf(" TIMER_ID  NAME    STATUS  PERIODIC  DEADLINE\n");

    lock = CriticalAreaLock();
    DOUBLE_LINKLIST_FOR_EACH(nodelink, &k_timer_list) {
        timer = CONTAINER_OF(nodelink, struct Timer, link);
        if(NONE != timer) {
            KPrintf("   %-8d%-8s%-10s%-10d%d\n", timer->id_node.id, timer->name, (timer->active_status == TIMER_ACTIVE_TRUE ? "active" : "unactive"),timer->origin_timeslice,timer->deadline_timeslice);
        }
    }
    CriticalAreaUnLock(lock);
    KPrintf("****************************************************************************************\n");

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, ShowTimer,ShowTimer, show timer in system);
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

static DriverType ShowBusFindDriver(struct Bus *bus)
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
    BusType bus;
    DriverType driver;
    HardwareDevType device;

    DoubleLinklistType *bus_node = NONE;
    DoubleLinklistType *bus_head = &bus_linklist;

    int i = 0; 
    int dev_cnt, maxlen;
    const char *item_type = "bus_type";
    const char *item_name_0 = "bus_name";
    const char *item_name_1 = "drv_name";
    const char *item_name_2 = "dev_name";
    const char *item_cnt = "cnt";

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

    bus_node = bus_head->node_next;

    do
    {
        bus = SYS_DOUBLE_LINKLIST_ENTRY(bus_node, struct Bus, bus_link);
        if (bus) {
            KPrintf("%s", " ");
            KPrintf("%-15s%-15s", 
                bus_type_str[bus->bus_type], 
                bus->bus_name);
            
            driver = ShowBusFindDriver(bus);

            if (driver) {
                KPrintf("%-15s", driver->drv_name);
            }  else  {
                KPrintf("%-15s", "nil");
            }

            if (bus->haldev_cnt) {
                DoubleLinklistType *dev_node = NONE;
                DoubleLinklistType *dev_head = &bus->bus_devlink;

                dev_node = dev_head->node_next;
                dev_cnt = 1;
                while (dev_node != dev_head) {
                    device = SYS_DOUBLE_LINKLIST_ENTRY(dev_node, struct HardwareDev, dev_link);

                    if (1 == dev_cnt) {
                        if (device) {
                            KPrintf("%-16s%-4d\n", device->dev_name, dev_cnt);
                        } else {
                            KPrintf("%-16s%-4d\n", "nil", dev_cnt);
                        }
                    } else {
                        KPrintf("%46s", " ");
                        if (device) {
                            KPrintf("%-16s%-4d\n", device->dev_name, dev_cnt);
                        } else {
                            KPrintf("%-16s%-4d\n", "nil", dev_cnt);
                        }
                    }
                    dev_cnt++;
                    dev_node = dev_node->node_next;
                }
            } else {
                KPrintf("\n");
            }
        }
        bus_node = bus_node->node_next;
    }
    while (bus_node != bus_head);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(1)|SHELL_CMD_DISABLE_RETURN,ShowBus, ShowBus, show bus all device and driver in system);

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
