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
* @file dev_touch.h
* @brief define touch dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_TOUCH_H
#define DEV_TOUCH_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TouchDataStandard
{
    uint16 x;
    uint16 y;
};

struct TouchDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev,struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct TouchHardwareDevice
{
    struct HardwareDev haldev;

    const struct TouchDevDone *dev_done;
    
    void *private_data;
};

/*Register the touch device*/
int TouchDeviceRegister(struct TouchHardwareDevice *touch_device, void *touch_param, const char *device_name);

/*Register the touch device to the touch bus*/
int TouchDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register touch device*/
HardwareDevType TouchDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
