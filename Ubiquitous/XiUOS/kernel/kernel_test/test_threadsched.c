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
* @file test_threadsched.c
* @brief support to test thread sched function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <string.h>
#include <xs_assign.h>

extern long ShowTask(void);
extern unsigned int msleep(uint64_t msec);

static int32 tid1 = NONE;
static int32 tid2 = NONE;
static int32 tid3 = NONE;
static int32 tid4 = NONE;
static int32 tid5 = NONE;

#define DYNAMIC_TASK_STACK_SIZE     3072
#define PRIORITY         15
static void Task1Entry(void *parameter)
{
	int cnt = 10;
	DoubleLinklistType* node = NONE;
	DoubleLinklistType* head = NONE;
	struct TaskDescriptor *obj = NONE;
	TaskDyncSchedMembeType *tmp = NONE;
	register x_base level;

	#ifdef ARCH_SMP
	if (0 == strncmp(parameter,"-b",strlen("-b"))) { ///< if tasks bind to cpu 0
		head = &(Assign.smp_os_assign_ready_rector[0].priority_ready_vector[PRIORITY]);
	} else {
		head = &(Assign.os_assign_read_vector.priority_ready_vector[PRIORITY]);
	}
	#else
		head = &(Assign.os_assign_read_vector.priority_ready_vector[PRIORITY]);
	#endif
	while(cnt--) {
#ifdef TOOL_SHELL
		ShowTask();
#endif
		KPrintf("\n");
		
		DOUBLE_LINKLIST_FOR_EACH(node, head) {
			tmp = SYS_DOUBLE_LINKLIST_ENTRY(node, struct TaskDyncSchedMember, sched_link);
			obj =CONTAINER_OF(tmp,struct TaskDescriptor, task_dync_sched_member);
			KPrintf("task ready table node name = %s node remaining_tick= %d node advance_cnt =%d\n",obj->task_base_info.name, 
			obj->task_dync_sched_member.rest_timeslice, obj->task_dync_sched_member.advance_cnt); 
		}
#ifdef ARCH_SMP
#ifdef SCHED_POLICY_FIFO
		MdelayKTask(30);
#else
		msleep(30);
#endif
#else
#ifdef ARCH_ARM
		MdelayKTask(3);
#else
		MdelayKTask(80);
#endif
#endif
	}
	KTaskDelete(tid2);
	KTaskDelete(tid3);
	KTaskDelete(tid4);
	KTaskDelete(tid5);

}

static void Task2Entry(void *parameter)
{
	int i = 0;
	while(RET_TRUE) {
		i++;
	}
}
static void Task3Entry(void *parameter)
{
	int i = 0;
	while(RET_TRUE) {
		i++;
	}
}
static void Task4Entry(void *parameter)
{
	int i = 0;
	while(RET_TRUE) {
		i++;
	}
}
static void Task5Entry(void *parameter)
{
	int i = 0;
	while(RET_TRUE) {
		i++;
	}
}

void DynamicTaskSchedTest(char* parm)
{
	char t_parm[4];
#ifdef ARCH_SMP
	if (0 == strncmp("-b", parm, strlen("-b")) || 0 == strncmp("-bind", parm, strlen("-bind"))){
		strncpy(t_parm,"-b", 4);
	}
#endif	
	tid1 = KTaskCreate("d_tid1",
							Task1Entry,
							t_parm,
							DYNAMIC_TASK_STACK_SIZE,
							16);
	if (tid1 >= 0)
		StartupKTask(tid1);
	
	tid2 = KTaskCreate("d_tid2",
							Task2Entry,
							"d_tid2",
							1024,
							15);
#ifdef ARCH_SMP
	if (0 == strncmp("-b", parm, strlen("-b")) || 0 == strncmp("-bind", parm, strlen("-bind"))){
		KTaskCoreCombine(tid2, 0);
	}
#endif
	if (tid2 >= 0)
		StartupKTask(tid2);
	
	tid3 = KTaskCreate("d_tid3",
							Task3Entry,
							"d_tid3",
							1024,
							15);
#ifdef ARCH_SMP
	if (0 == strncmp("-b", parm, strlen("-b")) || 0 == strncmp("-bind", parm, strlen("-bind"))){
		KTaskCoreCombine(tid3, 0);
	}
#endif
	if (tid3 >= 0)
		StartupKTask(tid3);
	
	tid4 = KTaskCreate("d_tid4",
							Task4Entry,
							"d_tid4",
							1024,
							15);
#ifdef ARCH_SMP
	if (0 == strncmp("-b", parm, strlen("-b")) || 0 == strncmp("-bind", parm, strlen("-bind"))){
		KTaskCoreCombine(tid4, 0);
	}
#endif
	if (tid4 >= 0)
		StartupKTask(tid4);
	
	tid5 = KTaskCreate("d_tid5",
							Task5Entry,
							"d_tid5",
							1024,
							15);

#ifdef ARCH_SMP
	if (0 == strncmp("-b", parm, strlen("-b")) || 0 == strncmp("-bind", parm, strlen("-bind"))){
		KTaskCoreCombine(tid5, 0);
	}
#endif
	if (tid5 >= 0)
		StartupKTask(tid5);
}

/********************************************************************/
static void UsageHelp(void) 
{
	KPrintf("test_task_ready_usage.\n");
}

int TestTaskReadyAndSched(int argc, char * argv[])
{
	DynamicTaskSchedTest(argv[0]);
	
	return 0;
}


