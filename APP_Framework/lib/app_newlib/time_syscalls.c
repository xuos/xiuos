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

#include <sys/time.h>

time_t time(time_t *t)
{
    NULL_PARAM_CHECK(t);
    time_t current = 0;

#ifdef RESOURCES_RTC
    struct RtcSetParam rtc_set_param;
    rtc_set_param.rtc_set_cmd = OPER_RTC_GET_TIME;
    rtc_set_param.time = &current;

    RtcDrvSetFunction(RTC_DRV_NAME, &rtc_set_param);
#endif

    *t = current;

    return current;
}

