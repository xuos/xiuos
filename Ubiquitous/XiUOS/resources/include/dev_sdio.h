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
* @file dev_sdio.h
* @brief define sdio dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_SDIO_H
#define DEV_SDIO_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SdioDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct SdioHardwareDevice
{
    struct HardwareDev haldev;

    struct SdioDevDone *dev_done;
    
    void *private_data;
};

/*Register the sdio device*/
int SdioDeviceRegister(struct SdioHardwareDevice *sdio_device, const char *device_name);

/*Register the sdio device to the sdio bus*/
int SdioDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register sdio device*/
HardwareDevType SdioDeviceFind(const char *dev_name);

#ifdef __cplusplus
}
#endif

#endif
