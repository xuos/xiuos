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
* @file drv_rtc.c
* @brief register rtc drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_rtc.h>
#include <dev_rtc.h>

static DoubleLinklistType rtcdrv_linklist;

/*Create the driver linklist*/
static void RtcDrvLinkInit()
{
    InitDoubleLinkList(&rtcdrv_linklist);
}

DriverType RtcDriverFind(const char *drv_name, enum DriverType drv_type)
{
    NULL_PARAM_CHECK(drv_name);
    
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &rtcdrv_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if ((!strcmp(driver->drv_name, drv_name)) && (drv_type == driver->driver_type)) {
            return driver;
        }
    }

    KPrintf("RtcDriverFind cannot find the %s driver.return NULL\n", drv_name);
    return NONE;
}

int RtcDriverRegister(struct Driver *driver)
{
    NULL_PARAM_CHECK(driver);

    x_err_t ret = EOK;
    static x_bool driver_link_flag = RET_FALSE;

    if (!driver_link_flag) {
        RtcDrvLinkInit();
        driver_link_flag = RET_TRUE;
    }

    DoubleLinkListInsertNodeAfter(&rtcdrv_linklist, &(driver->driver_link));

    return ret;
}

int RtcDrvSetFunction(char *driver_name, struct RtcSetParam *rtc_set_param)
{
    NULL_PARAM_CHECK(driver_name);
    
    x_err_t ret = EOK;
    time_t now;
    struct tm *tm_now;
    struct tm tm_tmp;
    x_base lock;

    struct Driver *driver;
    struct BusConfigureInfo configure_info;
    struct RtcDrvConfigureParam drv_param;
    configure_info.private_data = &drv_param;

    driver = RtcDriverFind(driver_name, TYPE_RTC_DRV);
    if (NONE == driver) {
        KPrintf("RtcDrvSetFunction find rtc driver %s  error\n", driver_name);
        return ERROR;
    }

    if (OPER_RTC_SET_TIME == rtc_set_param->rtc_set_cmd) {
        now = time(NONE);
        lock = CriticalAreaLock();
        tm_now = localtime(&now);
        memcpy(&tm_tmp, tm_now, sizeof(struct tm));
        CriticalAreaUnLock(lock);

        tm_tmp.tm_year = rtc_set_param->date_param.year - 1900;
        tm_tmp.tm_mon  = rtc_set_param->date_param.month - 1; 
        tm_tmp.tm_mday = rtc_set_param->date_param.day;
        tm_tmp.tm_hour = rtc_set_param->time_param.hour;
        tm_tmp.tm_min  = rtc_set_param->time_param.minute;
        tm_tmp.tm_sec  = rtc_set_param->time_param.second;

        now = mktime(&tm_tmp);

        drv_param.rtc_operation_cmd = OPER_RTC_SET_TIME;
        drv_param.time = &now;

        ret = driver->configure(driver, &configure_info);
        if (EOK != ret) {
            KPrintf("RtcDrvSetFunction set time error %d\n", ret);
            return ret;
        }
    } else if (OPER_RTC_GET_TIME == rtc_set_param->rtc_set_cmd) {
        drv_param.rtc_operation_cmd = OPER_RTC_GET_TIME;
        drv_param.time = rtc_set_param->time;
        ret = driver->configure(driver, &configure_info);
        if (EOK != ret) {
            KPrintf("RtcDrvSetFunction set time error %d\n", ret);
            return ret;
        }
    }
    return EOK;
}

#ifdef USING_SOFT_RTC

static x_ticks_t SoftRtc_InitTick;
static time_t SoftRtc_InitTime;

static int SoftRtcInitTime(struct tm *time, struct RtcDateParam *date_param, struct RtcTimeParam *time_param)
{
    NULL_PARAM_CHECK(time);

    time->tm_year = date_param->year - 1900;
    time->tm_mon = date_param->month - 1;
    time->tm_mday = date_param->day;
    time->tm_hour = time_param->hour;
    time->tm_min = time_param->minute;
    time->tm_sec = time_param->second;

    return EOK;
}

static uint32 SoftRtcConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);

    struct RtcDriver *rtc_drv = (struct RtcDriver *)drv;
    struct RtcDrvConfigureParam *drv_param = (struct RtcDrvConfigureParam *)configure_info->private_data;

    int cmd = drv_param->rtc_operation_cmd;
    time_t *time = drv_param->time;

    switch(cmd)
    {
        case OPER_RTC_GET_TIME:
        {
            *time = SoftRtc_InitTime + (CurrentTicksGain() - SoftRtc_InitTick) / TICK_PER_SECOND;
            break;
        }
        case OPER_RTC_SET_TIME:
        {
            SoftRtc_InitTime = *time - (CurrentTicksGain() - SoftRtc_InitTick) / TICK_PER_SECOND;
            break;
        }
    }

    return EOK;
}

static struct RtcDateParam date_param = 
{
    .year = 2021,
    .month = 1,
    .day = 1,
};

static struct RtcTimeParam time_param = 
{
    .hour = 0,
    .minute = 0,
    .second = 0,
};

static int SoftRtcBusInit(struct RtcBus *softrtc_bus, struct RtcDriver *softrtc_driver)
{
    x_err_t ret = EOK;

    /*Init the soft rtc bus */
    ret = RtcBusInit(softrtc_bus, SOFT_RTC_BUS_NAME);
    if(EOK != ret)
    {
        KPrintf("SoftRtcBusInit RtcBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the soft rtc driver*/
    ret = RtcDriverInit(softrtc_driver, SOFT_RTC_DRV_NAME);
    if(EOK != ret)
    {
        KPrintf("SoftRtcBusInit RtcDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the soft rtc driver to the soft rtc bus*/
    ret = RtcDriverAttachToBus(SOFT_RTC_DRV_NAME, SOFT_RTC_BUS_NAME);
    if(EOK != ret)
    {
        KPrintf("SoftRtcBusInit RtcDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

int SoftRtcInit(void)
{
    x_err_t ret = EOK;
    struct tm time;

    SoftRtcInitTime(&time, &date_param, &time_param);

    static struct RtcBus softrtc_bus;
    memset(&softrtc_bus, 0, sizeof(struct RtcBus));

    static struct RtcDriver softrtc_driver;
    memset(&softrtc_driver, 0, sizeof(struct RtcDriver));
    
    softrtc_driver.configure = &(SoftRtcConfigure);

    ret = SoftRtcBusInit(&softrtc_bus, &softrtc_driver);
    if (EOK != ret) {
        KPrintf("SoftRtcInit error ret %u\n", ret);
        return ERROR;
    }

    SoftRtc_InitTick = CurrentTicksGain();
    SoftRtc_InitTime = mktime(&time);

    return ret;
}

#endif 
