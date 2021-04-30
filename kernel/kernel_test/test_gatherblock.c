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
* @file test_gatherblock.c
* @brief support to test gather block function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <string.h>

extern long ShowMemPool(void);
extern void ShowMemory(void);
extern void ListBuddy(void);
/**************************single gatherblock test sample***********************************/
static uint8 *ptr[30];
static uint8 mempool[2048];
static struct MemGather gm;
static GatherMemType gm_d;

#define TASK_PRIORITY           25
#define TASK_STACK_SIZE     2048
#define TASK_TIMESLICE        5

/* thread control pointer */
static int32 tid1;
static int32 tid2;

/* task1 entry */
static void Task1GmAlloc(void *parameter)
{
    int i;
    for (i = 0 ; i < 30 ; i++){
        if (ptr[i] == NONE){
            /* alloc memory blocks for 50 times, if failed, suspend task1 and start task2 */
            if (0 == strncmp("static", parameter, strlen("static"))){
                ptr[i] = AllocBlockMemGather(&gm, WAITING_FOREVER);
                if (ptr[i] != NONE)
                    KPrintf("task1: allocate No.%d from static gatherblock\n", i);
            }
            else{
                ptr[i] = AllocBlockMemGather(gm_d, WAITING_FOREVER);
                if (ptr[i] != NONE)
                    KPrintf("task1: allocate No.%d from dynamic gatherblock\n", i);
            }
        }
    }
}


/* task2 entry, its priority is lower than task1 */
static void Task2GmRelease(void *parameter)
{
    int i;
    KPrintf("task2 try to release block\n");
    for (i = 0; i < 30 ; i++){
        DelayKTask(10);

        /* release all memory blocks */
        if (ptr[i] != NONE){
            KPrintf("release block %d\n", i);
            FreeBlockMemGather(ptr[i]);
            ptr[i] = NONE;
        }
    }
    if (0 == strncmp("static", parameter, strlen("static"))) {
        KPrintf("****************   detach gatherblock test    *****************\n");
        RemoveMemGather(&gm);
    } else {
        KPrintf("****************   delete gatherblock test    *****************\n");
        DeleteMemGather(gm_d);
    }
}


int SingleGatherblockTest(char * parameter)
{
    int i;
    for (i = 0; i < 30; i ++) ptr[i] = NONE;
    /* init memory blocks object */
    if(0 == strncmp("static", parameter, strlen("static"))){
        KPrintf("test static create gatherblock.\n");
        InitMemGather(&gm, "mp_s", &mempool[0], sizeof(mempool), 80);
        ShowMemPool();
    }

    if(0 == strncmp("dynamic", parameter, strlen("dynamic"))){
        KPrintf("test dynamic create gatherblock.\n");
        //FreeBlockMemGather(&mp, "mp1", &mempool[0], sizeof(mempool), 80);
        gm_d = CreateMemGather("mp_d",20,80);
        if(gm_d == NONE){
            KPrintf("%s: allocate failure.\n",__func__);
            return -1;
        }

        ShowMemPool();
    }
    /* create task1, alloc memory blocks */
    tid1 = KTaskCreate("task1", Task1GmAlloc, parameter,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY);
    if (tid1 >= 0){
        KPrintf("single test: task1 startup.\n");
        StartupKTask(tid1);
    }

    /* create task2, release memory blocks */
    tid2 = KTaskCreate("task2", Task2GmRelease, parameter,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY + 1);
    if (tid2 >= 0){
        StartupKTask(tid2);
    }
    else{
        KPrintf("create task2 for release failure.\n");
    }

    return 0;
}
/**************************************************************************************/


/**********************mutiple gatherblock test sample**************************************/
struct MemGather s_gm1;
struct MemGather *d_gm1 = NONE;

static uint8 mempool_gm1[1024];

#define MAX_SIZE    11
#define TASK_PRIORITY      25
#define TASK_STACK_SIZE    2048
#define TASK_TIMESLICE     5

/* thread control pointer */
static int32 m_tid1 = NONE;
static int32 m_tid2 = NONE;

static uint8 *multiple_s_ptr[20];
static uint8 *multiple_d_ptr[20];

void Task1AllocEntry(void *parameter){
    int index = 0;
    for(int i=0;i<20;i++){

        multiple_s_ptr[i] = AllocBlockMemGather(&s_gm1, WAITING_FOREVER);
        if(multiple_s_ptr[i] != NONE){
            KPrintf("task1: allocate No.%d from static\n", i);
        }
        multiple_d_ptr[i] = AllocBlockMemGather(d_gm1, WAITING_FOREVER);

        if (ptr[i] != NONE)
            KPrintf("task1: allocate No.%d from dynamic\n", i);
    }
}

void Task2FreeEntry(void *parameter){
    DelayKTask(100);
    for(int i=0;i<20;i++){

        DelayKTask(10);
        if(multiple_s_ptr[i]){
            FreeBlockMemGather(multiple_s_ptr[i]);
            KPrintf("task2: free No.%d of static\n", i);
        }
        DelayKTask(10);
        if(multiple_d_ptr[i]){
            FreeBlockMemGather(multiple_d_ptr[i]);

            KPrintf("task2: free No.%d of dynamic\n", i);
        }
    }
    KPrintf("****************   detach gatherblock test    *****************\n");
    RemoveMemGather(&s_gm1);
    KPrintf("****************   delete gatherblock test    *****************\n");
    DeleteMemGather(d_gm1);
}

int MultipleGatherblockTest(void)
{

	KPrintf("****************mutiple gatherblock test start*****************\n");

	for(int i=0;i<20;i++){
        multiple_s_ptr[i] = NONE;
        multiple_d_ptr[i] = NONE;
	}

    InitMemGather(&s_gm1, "m_gm_s1", &mempool_gm1[0], sizeof(mempool_gm1), 80);

    d_gm1 = CreateMemGather("m_gm_d1", MAX_SIZE, 80);
    if (mempool_gm1 == NONE){
        KPrintf("create m_gm_d2 failed.");
        CHECK(0);
    }
    ShowMemPool();

    /* create task1, alloc memory blocks */
    m_tid1 = KTaskCreate("task1_m", Task1AllocEntry, NONE,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY);
    if (m_tid1 >= 0){
        StartupKTask(m_tid1);
    }

    /* create task2, release memory blocks */
    m_tid2 = KTaskCreate("task2_m", Task2FreeEntry, NONE,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY + 1);
    if (m_tid2 >= 0){
        StartupKTask(m_tid2);
    }
    else{
        KPrintf("create task2 for release failure.\n");
    }

    return 0;
}
/**************************************************************************************/

/* thread control pointer */
static int32 l_tid1 = NONE;
static int32 l_tid2 = NONE;
struct MemGather *gm_l = NONE;
void *l_ptr[50];

void LTask1AllocEntry(void *parameter){
    int index = 0;
    for(int i=0;i<50;i++){
        l_ptr[i] = AllocBlockMemGather(gm_l, WAITING_FOREVER);
        if(l_ptr[i] != NONE){
            KPrintf("l_task1: allocate No.%d\n", i);
        }
    }
}

void LTask2FreeEntry(void *parameter){
    DelayKTask(100);
    for(int i=0;i<50;i++){
        DelayKTask(10);
        if(l_ptr[i]){
            FreeBlockMemGather(l_ptr[i]);
            KPrintf("l_task2: free No.%d\n", i);
        }
    }
    KPrintf("****************   delete gatherblock test    *****************\n");
    DeleteMemGather(gm_l);
}


void GatherblockLimitTest(char *name, int count, int blocksize){
    gm_l = CreateMemGather(name, count, blocksize);
    if(gm_l == NONE){
        KPrintf("no memory.\n");
        return;
    }
    ListBuddy();
    /* create task1, alloc memory blocks */
    l_tid1 = KTaskCreate("task1_l", LTask1AllocEntry, NONE,
                              TASK_STACK_SIZE,
                              TASK_PRIORITY);
    if (l_tid1 >= 0){
        StartupKTask(l_tid1);
    }

    /* create task2, release memory blocks */
    l_tid2 = KTaskCreate("task2_l", LTask2FreeEntry, NONE,
                              TASK_STACK_SIZE,
                              TASK_PRIORITY + 1);
    if (l_tid2 >= 0){
        StartupKTask(l_tid2);
    }
}

#ifdef  ARCH_ARM
#define CACHE_COUNT  1024
#else
#define CACHE_COUNT  8000
#endif

struct MemGather *pools[CACHE_COUNT];
void GatherblockLimitcountTest(char *name, int count, int blocksize, int poolcount){
    if(poolcount>CACHE_COUNT || poolcount<=0){
        KPrintf("type the poolcount between 0-%d\n",CACHE_COUNT);
        return;
    }
    int index;
    for(index=0;index < poolcount;index++){
        pools[index] = CreateMemGather(name, count, blocksize);
        if(pools[index] == NONE){
            KPrintf("gatherblock allocation failure,count[%d],size[%d]\n", count, blocksize);
            break;
        }
        KPrintf("gatherblock allocation,count[%d],size[%d]\n", count, blocksize);
    }
    ListBuddy();

    index--;
    for(;index>=0;index--){
        DeleteMemGather(pools[index]);
    }
}


/*****************************************************************/
static int32 traverse_tid = NONE;
static struct MemGather *traverse_gm = NONE;

void TraverseGmEntry(void *parameter){
    int count,size;
    int limit_count, limit_size;

    for(count =1; count <= 30; count++){
        for(size = 1; ;size++){
            traverse_gm = CreateMemGather("hello",count,size);
            if( traverse_gm == NONE ){
                KPrintf("gatherblock allocation failure,count[%d],size[%d]\n", count, size);

                if(count == 1)
                    limit_size = size;
                break;
            }else{
                DeleteMemGather(traverse_gm);
            }
        }
        if(size ==1){
            limit_count = count;
            break;
        }
    }
    KPrintf("\n\n--------------------------- bound -----------------------------\n\n");
    KPrintf("\n\n--------------------------- over end -----------------------------\n\n");
}

#define TRAVERSE_TASK_PRIORITY 5
void TraverseGmTest(){

    traverse_tid = KTaskCreate("traverse_task", TraverseGmEntry, NONE,
                                 TASK_STACK_SIZE ,
                                    TRAVERSE_TASK_PRIORITY);
    if(traverse_tid >= 0){
        StartupKTask(traverse_tid);
    }else{
        KPrintf("Fail.\n");
    }
}
/********************************************************************/
/***************************test random read and write **************/
static uint8 *random_ptr[50];
static struct MemGather random_static_gm;
static GatherMemType random_dynamic_gm;
static uint8 dynamic_mempool[2048];

/* thread control pointer */
static int32 random_tid1 = NONE;
static int32 random_tid2 = NONE;
/* task1 entry */
int time_seed = 1;
char temp_parameter[40];

static void RandomTask1GmAlloc(void *parameter)
{
    int i;
    int total=0;
    time_seed++;
    srand(time_seed);
    char string[10];

    while(total<40){
        i = rand()%40;
        if (random_ptr[i] == NONE){
            /* alloc memory blocks for 50 times, if failed, suspend task1 and start task2 */
            if (0 == strncmp("static", temp_parameter, strlen("static"))){
                random_ptr[i] = AllocBlockMemGather(&random_static_gm, WAITING_FOREVER);
                if (random_ptr[i] != NONE){
                    itoa(i,string,10);
                    memcpy(random_ptr[i],string,sizeof(string));
                    KPrintf("%s No.%d: %s\n",temp_parameter, i, random_ptr[i]);
                }
            }else{
                random_ptr[i] = AllocBlockMemGather(random_dynamic_gm, WAITING_FOREVER);
                if (random_ptr[i] != NONE){
                    itoa(i,string,10);
                    memcpy(random_ptr[i],string,sizeof(string));
                    KPrintf("%s No.%d: %s\n", temp_parameter, i, random_ptr[i]);
                }
            }
            total++;
        }
    }
}


/* task2 entry, its priority is lower than task1 */
static void RandomTask2GmRelease(void *parameter)
{
    int i;
    time_seed++;
    srand(time_seed);


    int total = 0;
 
    while(total<40){
        i = rand()%40;

        /* release all memory blocks */
        if (random_ptr[i] != NONE){
            DelayKTask(10);
            KPrintf("release block %d: %s\n",i, random_ptr[i]);
            FreeBlockMemGather(random_ptr[i]);
            random_ptr[i] = NONE;
            total++;
        }
    }
    if (0 == strncmp("static", temp_parameter, strlen("static"))) {
        KPrintf("****************   detach gatherblock test    *****************\n");
        RemoveMemGather(&random_static_gm);
    } else {
        KPrintf("****************   delete gatherblock test    *****************\n");
        DeleteMemGather(random_dynamic_gm);
    }
}


int RandomAllocFreeTest(void *parameter)
{
    int i;
    for (i = 0; i < 40; i ++) random_ptr[i] = NONE;
    memcpy(temp_parameter,parameter, 10);
    /* init memory block object */
    if(0 == strncmp("static", parameter, strlen("static"))){
        KPrintf("test static create gatherblock-%s.\n",parameter);
        InitMemGather(&random_static_gm, "ran_mp_s", &dynamic_mempool[0], sizeof(dynamic_mempool), 80);
        ShowMemPool();
    }else{
        KPrintf("test dynamic create gatherblock.\n");
        random_dynamic_gm = CreateMemGather("ran_mp_d",40,80);
        if(random_dynamic_gm == NONE){
            KPrintf("%s: allocate failure.\n",__func__);
            return -1;
        }
        ShowMemPool();
    }
    /* create task1, alloc memory blocks */
    random_tid1 = KTaskCreate("r_task1", RandomTask1GmAlloc, parameter,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY);
    if (random_tid1 >= 0){
        StartupKTask(random_tid1);
    }

    /* create task2, release memory blocks */
    random_tid2 = KTaskCreate("r_task2", RandomTask2GmRelease, parameter,
                            TASK_STACK_SIZE,
                            TASK_PRIORITY + 1);
    if (random_tid2 >= 0){
        StartupKTask(random_tid2);
    }
    else{
        KPrintf("create task2 for release failure.\n");
    }

    return 0;
}


/********************************************************************/
void TestGatherblockUsage(void)
{
	KPrintf("TestGatherblockUsage.\n");
    KPrintf("test_main gm -a                                      --static, dynamic and multiple test.\n");
	KPrintf("test_main gm -s static/dynamic                       --test static create gatherblock to allocate and free.\n");
	KPrintf("test_main gm -m                                      --test multiple gatherblock to allocate and free.\n");
    KPrintf("test_main gm count_size [count] [size]               --create a gatherblock with typed count and size "
               "to allocate and free.\n");
    KPrintf("test_main gm multiple_gms [count] [size] [gm_ns]     --create multiple gatherblocks with typed count and \n");
    KPrintf("                                                       size to allocate and free.\n");
    KPrintf("test_main gm traverse                                --traverse.\n");
    KPrintf("test_main gm -r static/dynamic                       --random allocate and free.\n");

}

int TestGatherblock(char *argv[])
{
	int ret = 0;
	
	if (NONE == argv || 0 == strncmp("-h", argv[0], strlen("-h")) || 0 == strncmp("usage", argv[0], strlen("usage"))) {
        TestGatherblockUsage();
		return -EINVALED;
	}
	/**************** gatherblock auto test ******************/
	if (0 == strncmp("-a", argv[0], strlen("-a"))) {
		ret = SingleGatherblockTest("static");      ///< static creat single gatherblock test
		MdelayKTask(7000);
		ret |= SingleGatherblockTest("dynamic");    ///< dynamic creat single gatherblock test
		MdelayKTask(7000);
		ret |= MultipleGatherblockTest();
		MdelayKTask(7000);
        ret |= RandomAllocFreeTest("static");
        MdelayKTask(7000);
        ret |= RandomAllocFreeTest("dynamic");
		goto out;
	}
	
	/**************** single gatherblock test ****************/
	if (0 == strncmp("-s",argv[0],strlen("-s")) ) {
		if (0 == strncmp("static",argv[1],strlen("static"))) {
			ret = SingleGatherblockTest("static"); ///< static creat single gatherblock test
		} else {
			ret = SingleGatherblockTest("dynamic"); ///< dynamic creat single gatherblock test
		}
		goto out;
	} 
	
	/**************** mutiple gatherblock test ****************/
	if (0 == strncmp("-m",argv[0],strlen("-m")) ) {
		ret = MultipleGatherblockTest();
		goto out;
	}
    /******************** limitation test **********************/
    if (0 == strncmp("count_size",argv[0],strlen("count_size")) ) {
        int size = atoi(argv[2]);
        int count = atoi(argv[1]);
        GatherblockLimitTest("xsos",count,size);
        ret = 1;
        goto out;
    }

    /******************** limitation test **********************/
    if (0 == strncmp("multiple_gms",argv[0],strlen("multiple_gms")) ) {
        int size = atoi(argv[2]);
        int count = atoi(argv[1]);
        int poolnumber = atoi(argv[3]);
        GatherblockLimitcountTest("xsos",count,size,poolnumber);
        ret = 1;
        goto out;
    }

    /********************** traverse test **********************/
    if (0 == strncmp("traverse",argv[0],strlen("traverse")) ) {
        TraverseGmTest();
        goto out;
    }

    /**************** random alloc and free test ****************/
    if (0 == strncmp("-r",argv[0],strlen("-r")) ) {
        ret = RandomAllocFreeTest(argv[1]);
        goto out;
    }

out:
	if (ret < 0) {
		KPrintf("test gatherblock failed ret = %d.\n",ret);
		return -ERROR;
	}
	return 0;
}

