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
* @file:    tick.c
* @brief:   system heartbeat_ticks update
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <xs_spinlock.h>
#include <xs_delay.h>

extern void Time_Update_LwIP(void);

#ifdef ARCH_SMP
static x_ticks_t heartbeat_ticks[CPU_NUMBERS] = {0};
#else
static x_ticks_t heartbeat_ticks = 0;
#endif

#ifdef ARCH_SMP 
#define UPDATE_GLOBAL_HEARTBEAT()         \
    do {                                     \
       heartbeat_ticks[GetCpuId()]++;  \
    } while(0); 
#else
#define UPDATE_GLOBAL_HEARTBEAT()         \
    do {                                     \
       heartbeat_ticks ++;                \
    } while(0); 
#endif

/**
 * get haertbeat count
 *
 * @return CurrentTicks
 */
x_ticks_t CurrentTicksGain(void)
{
#ifdef ARCH_SMP
    return heartbeat_ticks[GetCpuId()];
#else
    return heartbeat_ticks;
#endif
}

/**
 * this function increases global systick in tick interrupt,and process task time slice
 * 
 */
void TickAndTaskTimesliceUpdate(void)
{
    struct TaskDescriptor *task = NONE;

    UPDATE_GLOBAL_HEARTBEAT();

#if defined(SCHED_POLICY_FIFO)
    FifoTaskTimesliceUpdate();
#elif defined (SCHED_POLICY_RR)
    task = GetKTaskDescriptor();
    RoundRobinTaskTimesliceUpdate(task);
#elif defined (SCHED_POLICY_RR_REMAINSLICE)
    task = GetKTaskDescriptor();
    RoundRobinRemainTaskTimesliceUpdate(task);
#endif
    CheckTaskDelay();
#ifdef KERNEL_SOFTTIMER
    CheckTimerList();
#endif
#ifdef BSP_USING_LWIP
    Time_Update_LwIP();
#endif

}

/**
 * This function will convert ms to ticks.
 *
 * @param ms time need to be converted
 * @return ticks
 */
#define MIN_TICKS    1

x_ticks_t CalculteTickFromTimeMs(uint32 ms)
{
    uint32 tmp = 0;
    x_ticks_t ticks = 0;
    
    ticks = (uint32)(((uint64)(ms * TICK_PER_SECOND)) / 1000) ;
    if (0 == ticks) {
        ticks = MIN_TICKS;
    } else {
        tmp = (uint32)(((uint64)(ms * TICK_PER_SECOND)) % 1000);

        if(tmp >= 5 && tmp <= 9) {
            ticks = ticks + MIN_TICKS;
        }
    }
    
    return ticks;
}

/**
 * This function will convert ticks to ms.
 *
 * @param ticks ticks need to be converted
 * @return ms
 */
uint32 CalculteTimeMsFromTick(x_ticks_t ticks)
{
    uint32 ms = 0;

    ms = (uint32)(((uint64)(ticks * 1000)) / TICK_PER_SECOND);
    
    return ms;
}
