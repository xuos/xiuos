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
 * @file sensor.h
 * @brief Structure and function declarations of the sensor framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.03.24
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <list.h>
#include <stdio.h>
#include <string.h>
#include <transform.h>

#define SENSOR_QUANTITY_VALUE_ERROR ((uint32_t)0xffffffff)

/* Sensor quantity report mode */
#define SENSOR_DEVICE_PASSIVE    0x00
#define SENSOR_DEVICE_ACTIVE     0x01

#define SENSOR_RECEIVE_BUFFSIZE 32


#if SENSOR_TYPE_END > 32
#error "Too many sensor types"
#endif

/* Sensor ability */
#define SENSOR_ABILITY_CO2      ((uint32_t)(1 << SENSOR_QUANTITY_CO2))
#define SENSOR_ABILITY_TEMP     ((uint32_t)(1 << SENSOR_QUANTITY_TEMP))
#define SENSOR_ABILITY_HUMI     ((uint32_t)(1 << SENSOR_QUANTITY_HUMI))
#define SENSOR_ABILITY_HCHO     ((uint32_t)(1 << SENSOR_QUANTITY_HCHO))
#define SENSOR_ABILITY_CO       ((uint32_t)(1 << SENSOR_QUANTITY_CO))
#define SENSOR_ABILITY_PM       ((uint32_t)(1 << SENSOR_QUANTITY_PM))
#define SENSOR_ABILITY_VOICE    ((uint32_t)(1 << SENSOR_QUANTITY_VOICE))

struct SensorProductInfo {
    uint32_t ability;           /* Bitwise OR of sensor ability */
    const char *vendor_name;
    const char *model_name;
};

struct SensorDevice;

struct SensorDone {
    int (*open)(struct SensorDevice *sdev);
    int (*close)(struct SensorDevice *sdev);
    int (*read)(struct SensorDevice *sdev, size_t len);
    int (*write)(struct SensorDevice *sdev, const void *buf, size_t len);
    int (*ioctl)(struct SensorDevice *sdev, int cmd);
};

struct SensorDevice {
    char *name;                             /* Name of sensor */
    struct SensorProductInfo *info;         /* Sensor model info */
    struct SensorDone *done;
    int fd;                                 /* File descriptor */
    int status;                             /* Sensor work mode */
    uint8_t buffer[SENSOR_RECEIVE_BUFFSIZE];  /* Buffer for read data */

    int ref_cnt;                            /* Reference count */
    DoublelistType quant_list;          /* Sensor quantity link */
    struct DoublelistNode link;      /* Sensors link node */
};

enum SensorQuantityType {
    SENSOR_QUANTITY_CO2 = 0,
    SENSOR_QUANTITY_TEMP,
    SENSOR_QUANTITY_HUMI,
    SENSOR_QUANTITY_HCHO,
    SENSOR_QUANTITY_CO,
    SENSOR_QUANTITY_PM,
    SENSOR_QUANTITY_VOICE,
    /* ...... */
    SENSOR_QUANTITY_END,
};

struct SensorQuantityValue {
    uint8_t decimal_places;   /* The decimal place of the result */
    uint32_t last_value;      /* The last read value, if it does not exist, is SENSOR_QUANTITY_VALUE_ERROR */
    uint32_t min_value;       /* The minimum read value, if it does not exist, is SENSOR_QUANTITY_VALUE_ERROR */
    uint32_t max_value;       /* The maximum read value, if it does not exist, is SENSOR_QUANTITY_VALUE_ERROR */
    uint32_t min_std;         /* The minimum standard value, if it does not exist, is SENSOR_QUANTITY_VALUE_ERROR */
    uint32_t max_std;         /* The maximum standard value, if it does not exist, is SENSOR_QUANTITY_VALUE_ERROR */
};

struct SensorQuantity {
    char *name;
    enum SensorQuantityType type;
    struct SensorQuantityValue value;
    struct SensorDevice *sdev;

    int32_t (*ReadValue)(struct SensorQuantity *quant);

    struct DoublelistNode quant_link;
    struct DoublelistNode link;
};

int SensorDeviceRegister(struct SensorDevice *sdev);
int SensorDeviceUnregister(struct SensorDevice *sdev);

struct SensorQuantity *SensorQuantityFind(const char *name, enum SensorQuantityType type);
int SensorQuantityRegister(struct SensorQuantity *quant);
int SensorQuantityUnregister(struct SensorQuantity *quant);
int SensorQuantityOpen(struct SensorQuantity *quant);
int SensorQuantityClose(struct SensorQuantity *quant);
int SensorQuantityRead(struct SensorQuantity *quant);
int SensorQuantityControl(struct SensorQuantity *quant, int cmd);

uint32_t Crc16(uint8_t * data, uint8_t length);
uint8_t GetCheckSum(uint8_t *data, uint8_t head, uint8_t length);

#endif
