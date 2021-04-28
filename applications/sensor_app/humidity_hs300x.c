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
 * @file humidity_hs300x.c
 * @brief HS300x humidity example
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.23
 */

#include "../user_api/switch_api/user_api.h"
#include <sensor.h>

/**
 * @description: Read a himidity
 * @return 0
 */
void HumiHs300x(void)
{
    struct SensorQuantity *humi = SensorQuantityFind(SENSOR_QUANTITY_HS300X_HUMIDITY, SENSOR_QUANTITY_HUMI);
    SensorQuantityOpen(humi);
    int32 humidity = SensorQuantityRead(humi);
    printf("Humidity : %d.%d %%RH\n", humidity/10, humidity%10);
    SensorQuantityClose(humi);
}