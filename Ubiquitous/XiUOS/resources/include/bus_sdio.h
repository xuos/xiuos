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
* @file bus_sdio.h
* @brief define sdio bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_SDIO_H
#define BUS_SDIO_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SdioDriver
{
    struct Driver driver;
    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);
};

struct SdioBus
{
    struct Bus bus;
    void *private_data;
};

/*Register the sdio bus*/
int SdioBusInit(struct SdioBus *sdio_bus, const char *bus_name);

/*Register the sdio driver*/
int SdioDriverInit(struct SdioDriver *sdio_driver, const char *driver_name);

/*Release the sdio device*/
int SdioReleaseBus(struct SdioBus *sdio_bus);

/*Register the sdio driver to the sdio bus*/
int SdioDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int SdioDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType SdioDriverFind(const char *drv_name);

#ifdef __cplusplus
}
#endif

#endif
