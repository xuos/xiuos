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
* @file test_hwtimer.c
* @brief support to test hwtimer function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <bus.h>
#include <xsconfig.h>
#include <dev_hwtimer.h>

#if defined (ARCH_ARM)
#define HWTIMER_BUS_NAME HWTIMER_BUS_NAME_2
#define HWTIMER_DEVICE_NAME HWTIMER_2_DEVICE_NAME_2
#elif defined (ARCH_RISCV)
#define HWTIMER_BUS_NAME HWTIMER_BUS_NAME_1
#define HWTIMER_DEVICE_NAME HWTIMER_1_DEVICE_NAME_1
#elif
#define HWTIMER_BUS_NAME
#define HWTIMER_DEVICE_NAME
#endif

void TimeoutCb(void* param){
    KPrintf("resource_sample callback come ... \n");
}




 void HwtimerSample(void)//int argc, char *argv[]
{
    struct Bus * hwtimer_bus = BusFind(HWTIMER_BUS_NAME);
    if (NULL == hwtimer_bus){
        KPrintf("hwtimer bus find failed !\n");
        return ;
    }
    
    struct HardwareDev* dev = BusFindDevice(hwtimer_bus, HWTIMER_DEVICE_NAME);
    struct HwtimerHardwareDevice* hwtimer_dev = (struct HwtimerHardwareDevice*)dev;
    if (NULL == hwtimer_dev){
        KPrintf("hwtimer hwtimer_dev find failed !\n");
        return ;
    }
    
    hwtimer_dev->hwtimer_param.repeat = 1;
    hwtimer_dev->hwtimer_param.period_millisecond = 3000;
    hwtimer_dev->hwtimer_param.cb_info.param = NULL;
    hwtimer_dev->hwtimer_param.cb_info.TimeoutCb = TimeoutCb;

    BusDevOpen(dev);

    MdelayKTask(20000);

    KPrintf("close hwtimer...\n");
    BusDevClose(dev);

}
/* export to shell cmd */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_DISABLE_RETURN|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                HwtimerSample, HwtimerSample, HwtimerSample );
