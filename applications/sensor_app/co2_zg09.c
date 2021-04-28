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
 * @file co2_zg09.c
 * @brief ZG09 CO2 example
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.23
 */

#include "../user_api/switch_api/user_api.h"
#include <sensor.h>

/**
 * @description: Read a CO2
 * @return 0
 */
void Co2Zg09(void)
{
    struct SensorQuantity *co2 = SensorQuantityFind(SENSOR_QUANTITY_ZG09_CO2, SENSOR_QUANTITY_CO2);
    SensorQuantityOpen(co2);
    printf("CO2 : %d ppm\n", SensorQuantityRead(co2));
    SensorQuantityClose(co2);
}