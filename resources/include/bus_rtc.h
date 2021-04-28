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
* @file bus_rtc.h
* @brief define rtc bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_RTC_H
#define BUS_RTC_H

#include <bus.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RtcDateParam
{
    uint32 year;
    uint32 month;
    uint32 day;
};

struct RtcTimeParam
{
    uint32 hour;
    uint32 minute;
    uint32 second;
};

struct RtcSetParam
{
    int rtc_set_cmd;
    time_t *time;
    struct RtcDateParam date_param;
    struct RtcTimeParam time_param;
};

struct RtcDrvConfigureParam
{
    int rtc_operation_cmd;
    time_t *time;
};

struct RtcDriver
{
    struct Driver driver;

    uint32 (*configure) (void *drv, struct BusConfigureInfo *configure_info);
};

struct RtcBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the rtc bus*/
int RtcBusInit(struct RtcBus *rtc_bus, const char *bus_name);

/*Register the rtc driver*/
int RtcDriverInit(struct RtcDriver *rtc_driver, const char *driver_name);

/*Release the rtc device*/
int RtcReleaseBus(struct RtcBus *rtc_bus);

/*Register the rtc driver to the rtc bus*/
int RtcDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int RtcDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType RtcDriverFind(const char *drv_name, enum DriverType drv_type);

/*Set Rtc time and date*/
int RtcDrvSetFunction(char *driver_name, struct RtcSetParam *rtc_set_param);

#ifdef __cplusplus
}
#endif

#endif
