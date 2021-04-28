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
 * @file voice_d124.c
 * @brief D124 voice example
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.23
 */

#include "../user_api/switch_api/user_api.h"
#include <sensor.h>

/**
 * @description: Read a voice
 * @return 0
 */
void VoiceD124(void)
{
    struct SensorQuantity *voice = SensorQuantityFind(SENSOR_QUANTITY_D124_VOICE, SENSOR_QUANTITY_VOICE);
    SensorQuantityOpen(voice);
    UserTaskDelay(2000);
    uint16 result = SensorQuantityRead(voice);
    printf("voice : %d.%d dB\n", result/(10*voice->value.decimal_places), result%(10*voice->value.decimal_places));
    SensorQuantityClose(voice);
}