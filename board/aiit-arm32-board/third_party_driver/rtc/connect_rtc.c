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
* @brief support aiit-arm32-board rtc function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <connect_rtc.h>
#include <hardware_rtc.h>
#include <hardware_rcc.h>
#include <hardware_pwr.h>
#include <stm32f4xx.h>
#include <time.h>

#define RTC_BACKUP_REGISTER 0x32F2

static int GetWeekDay(int year, int month, int day)
{
	if (month==1||month==2) {
		year -=1;
		month +=12;
	}
	return (day+1+2*month+3*(month+1)/5+year+(year/4)-year/100+year/400)%7+1;
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
			RTC_TimeTypeDef	t;
			RTC_DateTypeDef	d;

			memset(&ct,0,sizeof(struct tm));
			RTC_GetTime(RTC_Format_BIN,&t);
			RTC_GetDate(RTC_Format_BIN,&d);

			ct.tm_year = d.RTC_Year + 100;
			ct.tm_mon = d.RTC_Month - 1;
			ct.tm_mday = d.RTC_Date;
			ct.tm_wday = d.RTC_WeekDay;

			ct.tm_hour = t.RTC_Hours;
			ct.tm_min = t.RTC_Minutes;
			ct.tm_sec = t.RTC_Seconds;

			*time = mktime(&ct);
		}
		break;

    	case OPER_RTC_SET_TIME:
		{
			struct tm *ct;
			struct tm tm_new;
			x_base lock;
			RTC_TimeTypeDef  rtc_time_structure;
			RTC_InitTypeDef  rtc_init_structure;
			RTC_DateTypeDef  rtc_date_structure;

			lock = CriticalAreaLock();
			ct = localtime(time);
			memcpy(&tm_new, ct, sizeof(struct tm));
			CriticalAreaUnLock(lock);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
			PWR_BackupAccessCmd(ENABLE);
			rtc_init_structure.RTC_AsynchPrediv = 0x7F;
			rtc_init_structure.RTC_SynchPrediv = 0xFF;
			rtc_init_structure.RTC_HourFormat = RTC_HourFormat_24;
			RTC_Init(&rtc_init_structure);
			rtc_date_structure.RTC_Year = tm_new.tm_year - 100;
			rtc_date_structure.RTC_Month = tm_new.tm_mon + 1;
			rtc_date_structure.RTC_Date = tm_new.tm_mday;
			rtc_date_structure.RTC_WeekDay = GetWeekDay(tm_new.tm_year+1900,tm_new.tm_mon+1,tm_new.tm_mday);
			RTC_SetDate(RTC_Format_BIN, &rtc_date_structure);
			if (tm_new.tm_hour > 11) {
				rtc_time_structure.RTC_H12 = RTC_H12_PM;
			}
			else{
				rtc_time_structure.RTC_H12 = RTC_H12_AM;
			}		
			rtc_time_structure.RTC_Hours   = tm_new.tm_hour;
			rtc_time_structure.RTC_Minutes = tm_new.tm_min;
			rtc_time_structure.RTC_Seconds = tm_new.tm_sec; 

			RTC_SetTime(RTC_Format_BIN, &rtc_time_structure); 		

			RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);        
		}
		break;
    }

    return EOK;
}

int RtcConfiguration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)
	RCC_LSICmd(ENABLE);

	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

	/* ck_spre(1Hz) = RTCCLK(LSI) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	//uwSynchPrediv = 0xFF;
	//uwAsynchPrediv = 0x7F;

#elif defined (RTC_CLOCK_SOURCE_LSE)
	RCC_LSEConfig(RCC_LSE_ON);

	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
#error Please select the RTC Clock source inside the main.c file
#endif

	RCC_RTCCLKCmd(ENABLE);

	RTC_WaitForSynchro();

	return 0;
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
        KPrintf("hw_rtc_init RtcBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the rtc driver*/
    ret = RtcDriverInit(rtc_driver, RTC_DRV_NAME);
    if (EOK != ret) {
        KPrintf("hw_rtc_init RtcDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the rtc driver to the rtc bus*/
    ret = RtcDriverAttachToBus(RTC_DRV_NAME, RTC_BUS_NAME);
    if (EOK != ret) {
        KPrintf("hw_rtc_init RtcDriverAttachToBus error %d\n", ret);
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
        KPrintf("hw_rtc_init RtcDeviceInit device %s error %d\n", RTC_DEVICE_NAME, ret);
        return ERROR;
    }  

    ret = RtcDeviceAttachToBus(RTC_DEVICE_NAME, RTC_BUS_NAME);
    if (EOK != ret) {
        KPrintf("hw_rtc_init RtcDeviceAttachToBus device %s error %d\n", RTC_DEVICE_NAME, ret);
        return ERROR;
    }  

    return  ret;
}

int Stm32HwRtcInit(void)
{
    x_err_t ret = EOK;

    static struct RtcBus rtc_bus;
    memset(&rtc_bus, 0, sizeof(struct RtcBus));

    static struct RtcDriver rtc_driver;
    memset(&rtc_driver, 0, sizeof(struct RtcDriver));
    
	rtc_driver.configure = &(RtcConfigure);

    ret = BoardRtcBusInit(&rtc_bus, &rtc_driver);
    if (EOK != ret) {
        KPrintf("hw_rtc_init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardRtcDevBend();
    if (EOK != ret) {
        KPrintf("hw_rtc_init error ret %u\n", ret);
        return ERROR;
    }    

    if (RTC_BACKUP_REGISTER != RTC_ReadBackupRegister(RTC_BKP_DR0)) {
        if (0 != RtcConfiguration()) {
            KPrintf("hw_rtc_init RtcConfiguration error...\n");
            return ERROR;
        }
    } else {
        RTC_WaitForSynchro();
    }

    return ret;
}

#ifdef TOOL_SHELL
void ShowTime(void)
{
	RTC_TimeTypeDef	time;
	RTC_DateTypeDef	date;
	RTC_GetDate(RTC_Format_BIN,&date);
	RTC_GetTime(RTC_Format_BIN, &time);
	KPrintf("Now Time = %d %02d %02d[%02d]-%0.2d:%0.2d:%0.2d \r\n", \
		date.RTC_Year, 
		date.RTC_Month,
		date.RTC_Date,
		date.RTC_WeekDay,
		time.RTC_Hours,
		time.RTC_Minutes,
		time.RTC_Seconds);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
												ShowTime, ShowTime, Arm test rtc function show time);
#endif
