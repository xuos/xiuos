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
* @file test_timer.c
* @brief support to test timer function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>

extern long ShowTimer(void);
/*****************************************************************************************************************/
/***********************************test create timer with dynamic style *****************************************/


static int32 dynamic_timer1;
static int32 dynamic_timer2;
static int dynamic_cnt = 0;


static void DynamicTimeout1Entry(void *parameter)
{
    KPrintf("periodic timer is timeout %d\n", dynamic_cnt);
    if (dynamic_cnt++>= 9){
        KTimerQuitRun(dynamic_timer1);
        KPrintf("periodic timer was stopped! \n");
    }
}

static void DynamicTimeout2Entry(void *parameter)
{
    KPrintf("one shot timer is timeout\n");
}

int TestDynamicTimer(void)
{
    dynamic_timer1 = KCreateTimer("d_tmr1", DynamicTimeout1Entry,
                             NONE, 10,
                             TIMER_TRIGGER_PERIODIC);

    if (dynamic_timer1 != NONE) KTimerStartRun(dynamic_timer1);

    dynamic_timer2 = KCreateTimer("d_tmr2", DynamicTimeout2Entry,
                             NONE,  30,
                             TIMER_TRIGGER_ONCE);

    if (dynamic_timer2 != NONE) KTimerStartRun(dynamic_timer2);
    ShowTimer();
    return 0;
}

int TestTmrD(void)
{
    KPrintf("test dynamic timer\n");
    TestDynamicTimer();
    return 0;
}

/*******************************test mixed timers with dynamic and static style **********************************/
static int32 mixed_timer_d;
static int32 test_delete_timer;

static int num = 0;


static void MexedTimeout2Entry(void *parameter)
{
    KPrintf("mixed\n");
    KTimerQuitRun(mixed_timer_d);
}

int TimerMix(void)
{
    mixed_timer_d = KCreateTimer("mix_t2", MexedTimeout2Entry, NONE, 30, TIMER_TRIGGER_ONCE);

    if (mixed_timer_d != NONE) KTimerStartRun(mixed_timer_d);
    
    test_delete_timer = KCreateTimer("del_t3", MexedTimeout2Entry, NONE, 30, TIMER_TRIGGER_ONCE);
    ShowTimer();
    if(test_delete_timer != NONE)
        KDeleteTimer(test_delete_timer);

    return 0;
}

int TestTmrM(void)
{
    KPrintf("test mixed timer\n");
    TimerMix();
    return 0;
}

int TestTmr(int argc, char*argv[]) {
  
    KPrintf("/******************************************** START **************************************************/\n");
    TestTmrD();
    MdelayKTask(4000);
    KPrintf("/******************************************* boundary ************************************************/\n");
    TestTmrM();
    MdelayKTask(4000);
    KPrintf("/********************************************* END **************************************************/\n");

    return 0;
}
