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
* @file connect_rtc.c
* @brief support kd233-board rtc function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "sysctl.h"
#include "connect_rtc.h"
#include "hardware_rtc.h"

static int GetWeekDay(int year, int month, int day)
{
    /* Magic method to get weekday */
    int weekday  = (day += month < 3 ? year-- : year - 2, 
        23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400) % 7;
    return weekday;
}

static uint32 RtcConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);

    struct RtcDriver *rtc_drv = (struct RtcDriver *)drv;
    struct RtcDrvConfigureParam *drv_param = (struct RtcDrvConfigureParam *)configure_info->private_data;

    int cmd = drv_param->rtc_operation_cmd;
    time_t *time = drv_param->time;

    switch (cmd)
    {
        case OPER_RTC_GET_TIME:
        {
            struct tm ct;
            int year,month,day,hour,minute,second;
            memset(&ct,0,sizeof(struct tm));

            rtc_timer_get(&year, &month, &day, &hour, &minute, &second);

            ct.tm_year = year - 1900;
            ct.tm_mon = month - 1;
            ct.tm_mday = day;
            ct.tm_wday = GetWeekDay(year, month, day);

            ct.tm_hour = hour;
            ct.tm_min = minute;
            ct.tm_sec = second;

            *time = mktime(&ct);
        }
        break;
        case OPER_RTC_SET_TIME:
        {
            struct tm *ct;
            struct tm tm_new;
            x_base lock;

            lock = CriticalAreaLock();
            ct = localtime(time);
            memcpy(&tm_new, ct, sizeof(struct tm));
            CriticalAreaUnLock(lock);

            sysctl_reset(SYSCTL_RESET_RTC);
            sysctl_clock_enable(SYSCTL_CLOCK_RTC);
            rtc_protect_set(0);
            rtc_timer_set_clock_frequency(SysctlClockGetFreq(SYSCTL_CLOCK_IN0));
            rtc_timer_set_clock_count_value(1);
            rtc_timer_set_mode(RTC_TIMER_RUNNING);

            if (rtc_timer_set(tm_new.tm_year+1900,tm_new.tm_mon+1,tm_new.tm_mday,
                            tm_new.tm_hour,tm_new.tm_min,tm_new.tm_sec)==-1)
                return ERROR;
        }
        break;
    }
    return EOK;
}

/*manage the rtc device operations*/
static const struct RtcDevDone dev_done =
{
    .open = NONE,
    .close = NONE,
    .write = NONE,
    .read = NONE,
};

static int BoardRtcBusInit(struct RtcBus *rtc_bus, struct RtcDriver *rtc_driver)
{
    x_err_t ret = EOK;

    /*Init the rtc bus */
    ret = RtcBusInit(rtc_bus, RTC_BUS_NAME);
    if (EOK != ret) {
        KPrintf("HwRtcInit RtcBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the rtc driver*/
    ret = RtcDriverInit(rtc_driver, RTC_DRV_NAME);
    if (EOK != ret) {
        KPrintf("HwRtcInit RtcDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the rtc driver to the rtc bus*/
    ret = RtcDriverAttachToBus(RTC_DRV_NAME, RTC_BUS_NAME);
    if (EOK != ret) {
        KPrintf("HwRtcInit RtcDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the rtc device to the rtc bus*/
static int BoardRtcDevBend(void)
{
    x_err_t ret = EOK;

    static struct RtcHardwareDevice rtc_device;
    memset(&rtc_device, 0, sizeof(struct RtcHardwareDevice));

    rtc_device.dev_done = &(dev_done);

    ret = RtcDeviceRegister(&rtc_device, NONE, RTC_DEVICE_NAME);
    if (EOK != ret) {
        KPrintf("HwRtcInit RtcDeviceInit device %s error %d\n", RTC_DEVICE_NAME, ret);
        return ERROR;
    }  

    ret = RtcDeviceAttachToBus(RTC_DEVICE_NAME, RTC_BUS_NAME);
    if (EOK != ret) {
        KPrintf("HwRtcInit RtcDeviceAttachToBus device %s error %d\n", RTC_DEVICE_NAME, ret);
        return ERROR;
    }  

    return  ret;
}

int HwRtcInit(void)
{
    x_err_t ret = EOK;

    static struct RtcBus rtc_bus;
    memset(&rtc_bus, 0, sizeof(struct RtcBus));

    static struct RtcDriver rtc_driver;
    memset(&rtc_driver, 0, sizeof(struct RtcDriver));
    
    rtc_driver.configure = &(RtcConfigure);

    ret = BoardRtcBusInit(&rtc_bus, &rtc_driver);
    if (EOK != ret) {
        KPrintf("HwRtcInit error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardRtcDevBend();
    if (EOK != ret) {
        KPrintf("HwRtcInit error ret %u\n", ret);
        return ERROR;
    }    

    rtc_init();

    return ret;
}
