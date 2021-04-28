/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file pm1_0_ps5308.c
 * @brief PS5308 PM1.0 example
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.23
 */

#include "../user_api/switch_api/user_api.h"
#include <sensor.h>

/**
 * @description: Read a PM1.0
 * @return 0
 */
void Pm1_0Ps5308(void)
{
    struct SensorQuantity *pm1_0 = SensorQuantityFind(SENSOR_QUANTITY_PS5308_PM1_0, SENSOR_QUANTITY_PM);
    SensorQuantityOpen(pm1_0);
    UserTaskDelay(2000);
    printf("PM1.0 : %d ug/m³\n", SensorQuantityRead(pm1_0));
    SensorQuantityClose(pm1_0);
}