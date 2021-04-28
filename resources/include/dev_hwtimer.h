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
* @file dev_hwtimer.h
* @brief define hwtimer dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_HWTIMER_H
#define DEV_HWTIMER_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HwtimerCallBackInfo
{
    void (*TimeoutCb) (void* param);
    void *param;
};

struct HwtimerDeviceParam
{
    uint32 period_millisecond;
    uint32 repeat;
    struct HwtimerCallBackInfo cb_info;
};

struct HwtimerDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev,struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct HwtimerHardwareDevice
{
    struct HardwareDev haldev;
    struct HwtimerDeviceParam hwtimer_param;

    const struct HwtimerDevDone *dev_done;
    
    void *private_data;
};

/*Register the hwtimer device*/
int HwtimerDeviceRegister(struct HwtimerHardwareDevice *hwtimer_device, void *hwtimer_param, const char *device_name);

/*Register the hwtimer device to the hwtimer bus*/
int HwtimerDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register hwtimer device*/
HardwareDevType HwtimerDeviceFind(const char *dev_name);

#ifdef __cplusplus
}
#endif

#endif
