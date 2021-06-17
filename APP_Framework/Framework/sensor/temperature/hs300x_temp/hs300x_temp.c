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
 * @file hs300x_temp.c
 * @brief HS300x temperature driver base perception
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <sensor.h>

static struct SensorDevice hs300x;

static struct SensorProductInfo info =
{
    (SENSOR_ABILITY_HUMI | SENSOR_ABILITY_TEMP),
    "Renesas",
    "HS300x",
};

/**
 * @description: Open HS300x sensor device
 * @param sdev - sensor device pointer
 * @return 1
 */
static int SensorDeviceOpen(struct SensorDevice *sdev)
{
    sdev->fd = PrivOpen(SENSOR_DEVICE_HS300X_DEV, O_RDWR);

    return 1;
}

/**
 * @description: Read sensor device
 * @param sdev - sensor device pointer
 * @param len - the length of the read data
 * @return success: 1 , failure: -1
 */
static int SensorDeviceRead(struct SensorDevice *sdev, size_t len)
{
    if (PrivWrite(sdev->fd, NULL, 0) != 1)
        return -1;
    
    UserTaskDelay(50);

    if (PrivRead(sdev->fd, sdev->buffer, len) != 1)
        return -1;

    return 1;
}

static struct SensorDone done =
{
    SensorDeviceOpen,
    NULL,
    SensorDeviceRead,
    NULL,
    NULL,
};

/**
 * @description: Init HS300x sensor and register
 * @return void
 */
static void SensorDeviceHs300xInit(void)
{
    hs300x.name = SENSOR_DEVICE_HS300X;
    hs300x.info = &info;
    hs300x.done = &done;
    hs300x.status = SENSOR_DEVICE_PASSIVE;

    SensorDeviceRegister(&hs300x);
}



static struct SensorQuantity hs300x_temperature;

/**
 * @description: Analysis HS300x temperature result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32_t ReadTemperature(struct SensorQuantity *quant)
{
    if (!quant)
        return -1;

    float result;
    if (quant->sdev->done->read != NULL) {
        if (quant->sdev->status == SENSOR_DEVICE_PASSIVE) {
            quant->sdev->done->read(quant->sdev, 4);
            quant->sdev->done->read(quant->sdev, 4);    /* It takes two reads to get the data right */
            result = ((quant->sdev->buffer[2]  << 8 | quant->sdev->buffer[3]) >> 2) * 165.0 /( (1 << 14) - 1) -  40.0;

            return (int32_t)(result * 10);
        }
        if (quant->sdev->status == SENSOR_DEVICE_ACTIVE) {
            printf("Please set passive mode.\n");
        }
    }else{
        printf("%s don't have read done.\n", quant->name);
    }
    
    return -1;
}

/**
 * @description: Init HS300x temperature quantity and register
 * @return 0
 */
int Hs300xTemperatureInit(void)
{
    SensorDeviceHs300xInit();
    
    hs300x_temperature.name = SENSOR_QUANTITY_HS300X_TEMPERATURE;
    hs300x_temperature.type = SENSOR_QUANTITY_TEMP;
    hs300x_temperature.value.decimal_places = 1;
    hs300x_temperature.value.max_std = 1000;
    hs300x_temperature.value.min_std = 0;
    hs300x_temperature.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_temperature.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_temperature.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_temperature.sdev = &hs300x;
    hs300x_temperature.ReadValue = ReadTemperature;

    SensorQuantityRegister(&hs300x_temperature);

    return 0;
}