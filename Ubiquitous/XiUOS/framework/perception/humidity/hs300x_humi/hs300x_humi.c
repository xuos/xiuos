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
 * @file hs300x_humi.c
 * @brief HS300x humidity driver base perception
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
 * @return EOK
 */
static int SensorDeviceOpen(struct SensorDevice *sdev)
{
    sdev->fd = open(SENSOR_DEVICE_HS300X_DEV, O_RDWR);

    return EOK;
}

/**
 * @description: Read sensor device
 * @param sdev - sensor device pointer
 * @param len - the length of the read data
 * @return success: EOK , failure: -ERROR
 */
static x_size_t SensorDeviceRead(struct SensorDevice *sdev, size_t len)
{
    if (write(sdev->fd, NONE, 0) != 1)
        return -ERROR;
    
    UserTaskDelay(50);

    if (read(sdev->fd, sdev->buffer, len) != 1)
        return -ERROR;

    return EOK;
}

static struct SensorDone done =
{
    SensorDeviceOpen,
    NONE,
    SensorDeviceRead,
    NONE,
    NONE,
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



static struct SensorQuantity hs300x_humidity;

/**
 * @description: Analysis HS300x humidity result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32 ReadHumidity(struct SensorQuantity *quant)
{
    if (!quant)
        return -ERROR;

    float result = 0.0;
    if (quant->sdev->done->read != NONE) {
        if (quant->sdev->status == SENSOR_DEVICE_PASSIVE) {
            quant->sdev->done->read(quant->sdev, 4);
            quant->sdev->done->read(quant->sdev, 4);    /* It takes two reads to get the data right */
            result = ((quant->sdev->buffer[0] << 8 | quant->sdev->buffer[1] ) & 0x3fff) * 100.0 / ( (1 << 14) - 1);

            return (int32)(result * 10);
        }
        if (quant->sdev->status == SENSOR_DEVICE_ACTIVE) {
            printf("Please set passive mode.\n");
        }
    }else{
        printf("%s don't have read done.\n", quant->name);
    }
    
    return -ERROR;
}

/**
 * @description: Init HS300x humidity quantity and register
 * @return 0
 */
int Hs300xHumidityInit(void)
{
    SensorDeviceHs300xInit();
    
    hs300x_humidity.name = SENSOR_QUANTITY_HS300X_HUMIDITY;
    hs300x_humidity.type = SENSOR_QUANTITY_HUMI;
    hs300x_humidity.value.decimal_places = 1;
    hs300x_humidity.value.max_std = 1000;
    hs300x_humidity.value.min_std = 0;
    hs300x_humidity.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_humidity.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_humidity.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    hs300x_humidity.sdev = &hs300x;
    hs300x_humidity.ReadValue = ReadHumidity;

    SensorQuantityRegister(&hs300x_humidity);

    return 0;
}