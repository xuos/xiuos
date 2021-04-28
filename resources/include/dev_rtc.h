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
* @file dev_rtc.h
* @brief define rtc dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_RTC_H
#define DEV_RTC_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RtcDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *datacfg);
    uint32 (*read) (void *dev, struct BusBlockReadParam *datacfg);
};

struct RtcHardwareDevice
{
    struct HardwareDev haldev;

    const struct RtcDevDone *dev_done;
    
    void *private_data;
};

/*Register the rtc device*/
int RtcDeviceRegister(struct RtcHardwareDevice *rtc_device, void *rtc_param, const char *device_name);

/*Register the rtc device to the rtc bus*/
int RtcDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register rtc device*/
HardwareDevType RtcDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
