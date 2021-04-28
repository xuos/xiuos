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
* @file TestMain.c
* @brief support to test function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <stdio.h>
#define MAX_ITEM_NAME_LENGTH    16
extern int TestIwg(void);

int TestTmrD(void);
int TestTmrM(void);

extern int TestTmr(int argc, char *argv[]);
extern int TestMem(int argc, char *argv[]);
extern int TestGatherblock(char *argv[]);
extern int TestTaskReadyAndSched(int argc, char *argv[]);
extern int TestRealtime(int argc, char * argv[]);

void KernelTestusage(void)
{
	KPrintf("test usage.\n");
	KPrintf("e.g., TestMain -mem\n");
}
struct TestSubitem
{
	char name[MAX_ITEM_NAME_LENGTH];
	int item;
};

enum TestItem
{
	USAGE = 0,            /* usage idex */
	MEM,                  /* mem test item index */
	TIMER,                /* timer test item index */
	GATHERBLOCK,
	SCHED,                /* task sched test */
	IWG,                  /* iwg test item index */
	REALTIME,
	INVALID_ITEM,         /* invalid index */
};

static struct TestSubitem kernel_subitem[INVALID_ITEM] =
{
	{ "-h",         USAGE },
	{ "-mem",       MEM },
    { "-timer",     TIMER},
	{ "-gm",        GATHERBLOCK},
    { "-sched",     SCHED},
	{ "-iwg",       IWG},
	{ "-realtime",  REALTIME},

};

int TestMain(int argc, char*argv[])
{
	char name[MAX_ITEM_NAME_LENGTH] = {0};
	int i = 0;
	int item = -1;

	strncpy(name, argv[1], MAX_ITEM_NAME_LENGTH); ///< getting input name of test item 
	for(i = 0; i < INVALID_ITEM; i++) {
		if(0 == strncmp(kernel_subitem[i].name, name, MAX_ITEM_NAME_LENGTH) ){
			item = kernel_subitem[i].item;
			break;
		}
	}
	
	switch(item)
	{
		case USAGE:
			KernelTestusage();
			break;
		case MEM:
#ifdef KERNEL_TEST_MEM
		    TestMem(argc-2,&argv[2]);
#endif
			break;
	    case TIMER:
#ifdef KERNEL_TEST_TIMER
            TestTmr(argc-2,&argv[2]);
#endif
            break;
		case GATHERBLOCK:
#ifdef KERNEL_TEST_MEM
	        TestGatherblock(&argv[2]);
#endif
	        break;
		case SCHED:
#ifdef KERNEL_TEST_SCHED
			TestTaskReadyAndSched(argc-2, &argv[2]);
#endif
			break;
		case IWG:
#ifdef KERNEL_TEST_IWG
			TestIwg();
#endif
		case REALTIME:
#ifdef KERNEL_TEST_REALTIME
			TestRealtime(argc-2, &argv[2]);
#endif
		default:
			break;
	}
	
	return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_DISABLE_RETURN|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                TestMain, TestMain, test main sample );
