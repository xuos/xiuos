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
* @file TestRtc.c
* @brief support to test rtc function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <device.h>

#ifdef RESOURCES_RTC

static int TestRtc(int argc, char *argv[])
{    
    time_t now;

    struct RtcSetParam rtc_set_param;

    KPrintf("TestRtc cmd %s\n", argv[1]);

    if (0 == strcmp("-s", argv[1])) {
        rtc_set_param.rtc_set_cmd = OPER_RTC_SET_TIME;
        rtc_set_param.date_param.year = 2021;
        rtc_set_param.date_param.month = 4;
        rtc_set_param.date_param.day = 6;
        rtc_set_param.time_param.hour = 16;
        rtc_set_param.time_param.minute = 0;
        rtc_set_param.time_param.second = 0;

        RtcDrvSetFunction(RTC_DRV_NAME, &rtc_set_param);

        rtc_set_param.rtc_set_cmd = OPER_RTC_GET_TIME;
        rtc_set_param.time = &now;

        RtcDrvSetFunction(RTC_DRV_NAME, &rtc_set_param);

        KPrintf("%s\n", ctime(&now));
    } else if (0 == strcmp("-g", argv[1])) {
        rtc_set_param.rtc_set_cmd = OPER_RTC_GET_TIME;
        rtc_set_param.time = &now;

        RtcDrvSetFunction(RTC_DRV_NAME, &rtc_set_param);

        KPrintf("%s\n", ctime(&now));
    }

    return EOK;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(4),
                                                TestRtc, TestRtc, Test the RTC Function);

#endif
