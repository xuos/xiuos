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
* @file TestMem.c
* @brief support to test mem function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <string.h>

extern void ShowMemory(void);
extern void ListBuddy(void);

/**************************test memory usage***********************************/

#include<time.h>
#include<stdlib.h>

#define TASK_PRIORITY 5
#define TASK_STACK_SIZE	4096
#define TASK_TIMESLICE	5
#define MEM_TEST_NUMBER	23

int number_imm=0;
static int arr[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};

void Fimm(void *parameter)
{
    KPrintf("%s: task starts.\n",__func__);
    int i=0;
    unsigned long size;
    char *ptr = NONE;

    srand(number_imm);
    while (i<50) {
        size = 1 << arr[rand() % MEM_TEST_NUMBER];
        ptr = x_malloc(size);

        if (ptr != NULL) {
            KPrintf("alloc-memory: %d btye\n",size);
            x_free(ptr);
            KPrintf("free--memory: %d byte\n",size);
            ptr=NONE;
        } else {
            KPrintf("try to get %d byte memory failed!\n",size);
            return;
        }
        i++;
    }
    number_imm++;
}

int FreeImmediatelly(void){

    KPrintf("\033[32;1m***********test limitations*************\033[0m\n");
    KPrintf("allocation and free pointer immdiatetely. \n");

    int32 tid;
    tid = KTaskCreate("Fimm",Fimm,NONE,TASK_STACK_SIZE,TASK_PRIORITY);

    if (tid >= 0)
        StartupKTask(tid);
    return 0;
}

static int num_FTillEnd=1;

void FTillEnd(void *parameter)
{
	int i=0;
	unsigned long size;
	char *ptr[55];
	unsigned long arrsize[55];
	//	srand((unsigned)time(NULL)*10);
	srand(num_FTillEnd);
    num_FTillEnd++;

	while (i<50) {
		size = 1 << arr[rand() % MEM_TEST_NUMBER];
		ptr[i] = x_malloc(size);
		arrsize[i]=size;

		if (ptr[i] != NULL) {
			KPrintf("get memory: %d btye\n",size);
		} else {
			KPrintf("try to get %d byte memory failed!\n",size);
			break;
		}
		i++;
	}
	i -= 1;
	for( ; i>=0 ; i-- ) {
		x_free(ptr[i]);
		KPrintf("free memory: %d byte\n",arrsize[i]);
	}
	return;
}

int FreeEnd(void)
{
    KPrintf("\033[32;1m***********test limitations*************\033[0m\n");
    KPrintf("-------allocation until no memory-------\n");

    int32 tid;
    tid = KTaskCreate("fend",FTillEnd,NONE,TASK_STACK_SIZE,TASK_PRIORITY);

    if (tid >= 0)
        StartupKTask(tid);
    return 0;
}

#ifdef ARCH_ARM
#define MEM_GRIN_COUNT  1024
#else
#define MEM_GRIN_COUNT  8000
#endif

char *ptr[MEM_GRIN_COUNT]; 
unsigned int arr_grin[MEM_GRIN_COUNT];

extern void ListBuddy();
int FendGrin()
{
    KPrintf("\033[32;1m***********test limitations*************\033[0m\n");
    KPrintf("-------allocation for 1 byte with step 1-------\n");
    int i=0;
    int tempvalue = 1;

    while( i< MEM_GRIN_COUNT) {
        KPrintf("\033[32;1malloc memory [%d]\033[0m\n",tempvalue);
        ptr[i] = x_malloc(tempvalue);

        if (ptr[i]) {
            arr_grin[i] = tempvalue;
            i++;
            tempvalue++;
        } else {
            if (tempvalue == 60)
                break;
            tempvalue=60;
        }
    }
    KPrintf("------------- limitation until [%d] -----------\n",i);
    ListBuddy();

    DelayKTask(2000);

    i -= 1;
    for ( ; i>=0 ; i-- ) {
        x_free(ptr[i]);
        KPrintf("free memory: %d byte\n",arr_grin[i]);
    }
    return 0;
}

#ifdef ARCH_ARM
#define MEM_CACHE_COUNT  1024
#else
#define MEM_CACHE_COUNT  8000
#endif

char *cache_ptr[MEM_CACHE_COUNT];

void TestCacheLimitation(int timers,int init)
{
    KPrintf("\033[32;1m******test small memory limit****\033[0m\n");
    int temp = init;
    int index;
    void *temp_ptr;

    timers = timers > MEM_CACHE_COUNT? MEM_CACHE_COUNT: timers;

    for (index = 1; index <= timers;index++) {
        if (index&1) {
            KPrintf("%d: x_malloc size [%d]. \n", index, temp);
            cache_ptr[index] = x_malloc(temp);
        } else {
            KPrintf("%d: x_calloc size [%d]. \n", index, temp);
            cache_ptr[index] = x_calloc(1,temp);
        }

        if (cache_ptr[index] == NONE) {
            KPrintf("%d: malloc or calloc failure. \n", index);
            break;
        }

        if ((index % 4) == 0) {
            KPrintf("%d: x_realloc size [%d]. \n", index, temp+index);
            temp_ptr = cache_ptr[index];
            cache_ptr[index] = x_realloc(cache_ptr[index], temp+index);
            if (cache_ptr[index] == NONE) {
                KPrintf("%d: x_realloc failure, roll back old pointer. \n", index);
                cache_ptr[index] = temp_ptr;
            }
        }

        temp++;
        temp %= 64;
        temp = ((temp==0) ? (temp + 1) : temp);
    }
    ListBuddy();

    DelayKTask(1000);

    index--;

    for(; index > 0; index--){
        //KPrintf("free %d\n",index);
        x_free(cache_ptr[index]);
    }
}

void UsageMem()
{
    KPrintf("%s: invalidate parameter\n",__func__);
    KPrintf("%s: usage for memory    \n",__func__);
    KPrintf("%s: -s, test malloc allocation,  e.g., test_main mem -s 64 \n",__func__);
    KPrintf("%s: -c, test calloc allocation,  e.g., test_main mem -c 64 \n",__func__);
    KPrintf("%s: -r, test realloc allocation, e.g., test_main mem -r 64 80 \n",__func__);
    KPrintf("%s: -Fimm, test limit allocation, e.g., test_main mem -Fimm \n",__func__);
    KPrintf("%s: -fend, test limit allocation, e.g., test_main mem -fend \n",__func__);
    KPrintf("%s: -grin, test limit allocation, e.g., test_main mem -grin \n",__func__);
    KPrintf("%s: rw, test read and write opts, e.g., test_main mem rw 100 \"oldcontent\" \"newcontent\"\n",__func__);
    KPrintf("%s: cache, test limit allocation, e.g., test_main mem cache 100 1,\n",__func__);
    KPrintf("%s:                                     allocate small mem for 100 timers started from 1. \n",__func__);
    KPrintf("%s: exception, make exception, e.g., test_main mem exception 100\n",__func__);
}

void TestMalloc(x_size_t size)
{
    KPrintf("\033[32;1m**********x_malloc & x_free***********\033[0m\n");
    KPrintf("the memory size is [%u] for allocation. \n", size);
    KPrintf("-------------before x_malloc-----------\n");

    ShowMemory();

    void *ptr = x_malloc(size);
    if (ptr) {
        KPrintf("----------x_malloc successfully--------\n");
        ShowMemory();
        x_free(ptr);
        KPrintf("-----------x_free successfully---------\n");
        ShowMemory();
    } else {
        KPrintf("------------x_malloc failure-----------\n");
    }
    KPrintf("****************** end *****************\n");
}

void TestReAlloc(x_size_t size_prev, x_size_t size_next)
{
    KPrintf("\033[32;1m**********x_realloc & x_free**********\033[0m\n");
    KPrintf("the memory size is [%u] for x_malloc. \n", size_prev);
    KPrintf("-------------before x_malloc-----------\n");
    ShowMemory();

    void *ptr = x_malloc(size_prev);
    void *ptr2;
    if (ptr) {
        KPrintf("---------x_malloc successfully--------\n");
        ShowMemory();
        KPrintf("-----------before x_realloc-----------\n");
        KPrintf("the memory size is [%u] for x_malloc. \n", size_next);
        ptr2 = x_realloc(ptr, size_next);
        if (ptr2) {
            KPrintf("-----------realloc successfully---------\n");
            ShowMemory();
            x_free(ptr2);
            KPrintf("-----------x_free successfully---------\n");
            ShowMemory();
        } else {
            KPrintf("-----------x_realloc failure-----------\n");
            if(size_next !=0)
            x_free(ptr);
            KPrintf("-----------x_free successfully---------\n");
            ShowMemory();
        }
    } else {
        KPrintf("-----------x_malloc failure------------\n");
    }
    KPrintf("****************** end *****************\n");
}

void TestCalloc(x_size_t size)
{
    KPrintf("\033[32;1m**********x_calloc & x_free***********\033[0m\n");
    KPrintf("the memory size is [%u] for x_malloc. \n", size);
    KPrintf("------------before x_calloc------------\n");
    ShowMemory();

    void*ptr = x_calloc(1, size);
    if (ptr) {
        KPrintf("---------x_calloc successfully---------\n");
        ShowMemory();
        x_free(ptr);
        KPrintf("----------x_free successfully----------\n");
        ShowMemory();
    } else {
        KPrintf("------------x_calloc failure-----------\n");
    }
    KPrintf("****************** end *****************\n");
}

void TestReadWrite(x_size_t size, char *raw_content, char *new_content)
{
    KPrintf("\033[32;1m**********read and write opts***********\033[0m\n");
    KPrintf("the memory size is [%u] for allocation. \n", size);
    KPrintf("-------------before x_malloc-----------\n");
    int endposition;

    void *ptr = x_malloc(size);
    if (ptr) {
        KPrintf("----------x_malloc successfully--------\n");
        KPrintf("----------   show raw content   --------\n");
        memcpy(ptr,raw_content, size > strlen(raw_content) ? strlen(raw_content): size);
        endposition = size > strlen(raw_content) ? strlen(raw_content): size;
        *((char*)ptr + endposition) = 0;

        KPrintf("raw content is [\033[41;1m%s\033[0m]\n",ptr);
        KPrintf("-----  write and show new content  -----\n");
        memcpy(ptr,new_content, size > strlen(new_content)? strlen(new_content): size);
        endposition = size>strlen(new_content)? strlen(new_content): size;

        *((char*)ptr + endposition) = 0;
        DelayKTask(100);

        KPrintf("new content is [\033[41;1m%s\033[0m]\n",ptr);
        x_free(ptr);
        KPrintf("-----------x_free successfully---------\n");
    } else {
        KPrintf("------------x_malloc failure-----------\n");
    }
    KPrintf("\n------------------ end -----------------\n\n");

    ptr = x_calloc(1,size);
    if (ptr) {
        KPrintf("----------x_calloc successfully--------\n");
        KPrintf("----------   show raw content   --------\n");
        memcpy(ptr,raw_content,size > strlen(raw_content) ? strlen(raw_content): size);
        endposition = size > strlen(raw_content) ? strlen(raw_content): size;
        *((char*)ptr + endposition) = 0;

        KPrintf("raw content is [\033[41;1m%s\033[0m]\n",ptr);
        KPrintf("-----  write and show new content  -----\n");
        memcpy(ptr,new_content,size  > strlen(new_content) ? strlen(new_content): size);
        endposition = size > strlen(new_content) ? strlen(new_content): size;
        *((char*)ptr + endposition)=0;

        DelayKTask(100);
        KPrintf("new content is [\033[41;1m%s\033[0m]\n",ptr);
        x_free(ptr);
        KPrintf("-----------x_free successfully---------\n");
    } else {
        KPrintf("------------x_calloc failure-----------\n");
    }
    KPrintf("****************** end *****************\n");
}

void TestException(x_size_t size)
{
    KPrintf("\033[32;1m**********test make exception***********\033[0m\n");
    KPrintf("the memory size is [%u] for allocation. \n", size);

    void*ptr = x_malloc(size);
    if (ptr == NONE) {
        KPrintf("------------x_malloc failure-----------\n");
    } else {
        memset(ptr-8, 5, size+8);
        x_free(ptr);
    }
    KPrintf("****************** end *****************\n");
}

int TestMem(int argc,char*argv[])
{
    x_size_t size;
    x_size_t newsize;
    if (0 == strncmp("-h", argv[0], strlen("-h"))) {
	    UsageMem();
	}

    if (0 == strncmp("cache", argv[0], strlen("cache"))) {
        if (argc == 2) {
            size = atoi(argv[1]);
            newsize =1;
        }

        if(argc == 3) {
            size = atoi(argv[1]);
            newsize = atoi(argv[2]);
            newsize = newsize==0 ? 1: newsize;
        }

        TestCacheLimitation(size, newsize);
    }

	if (0 == strncmp("-s", argv[0], strlen("-s"))) {
        size = atoi(argv[1]);
	    TestMalloc(size);
	}

	if (0 == strncmp("-r", argv[0], strlen("-r"))) {
        size = atoi(argv[1]);
        newsize = atoi(argv[2]);
        TestReAlloc(size,newsize);
	}

	if (0 == strncmp("-c", argv[0], strlen("-c"))) {
		size = atoi(argv[1]);
        TestCalloc(size);
	}

    if (0 == strncmp("-Fimm", argv[0], strlen("-Fimm"))) {
        FreeImmediatelly();
    }

    if (0 == strncmp("-fend", argv[0], strlen("-fend"))) {
        FreeEnd();
    }

    if (0 == strncmp("-grin", argv[0], strlen("-grin"))) {
        FendGrin();
    }

    if (0 == strncmp("rw", argv[0], strlen("rw"))) {
        size = atoi(argv[1]);
        TestReadWrite(size, argv[2], argv[3]);
    }

    if (0 == strncmp("exception",argv[0],strlen("exception"))) {
        size = atoi(argv[1]);
        TestException(size);
    }

	return 0;
}

/**************************      end      ***********************************/
