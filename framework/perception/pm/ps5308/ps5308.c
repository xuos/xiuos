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
 * @file ps5308.c
 * @brief PS5308 PM1.0, PM2.5, PM10, driver base perception
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <sensor.h>

static struct SensorDevice ps5308;
static int32_t active_task_id;
static int buff_lock;

static struct SensorProductInfo info =
{
    SENSOR_ABILITY_PM,
    "Pulsensor",
    "PS5308",
};

/**
 * @description: Read sensor task
 * @param sdev - sensor device pointer
 */
static void ReadTask(struct SensorDevice *sdev)
{
    while (1) {
        UserMutexObtain(buff_lock, WAITING_FOREVER);
        sdev->done->read(sdev, 32);
        UserMutexAbandon(buff_lock);
        UserTaskDelay(750);
    }
}

/**
 * @description: Open PS5308 sensor device
 * @param sdev - sensor device pointer
 * @return success: EOK , failure: other
 */
static int SensorDeviceOpen(struct SensorDevice *sdev)
{
    int result = EOK;

    buff_lock = UserMutexCreate();

    sdev->fd = open(SENSOR_DEVICE_PS5308_DEV, O_RDWR);
    
    struct SerialDataCfg cfg;
    cfg.serial_baud_rate    = BAUD_RATE_9600;
    cfg.serial_data_bits    = DATA_BITS_8;
    cfg.serial_stop_bits    = STOP_BITS_1;
    cfg.serial_buffer_size  = 128;
    cfg.serial_parity_mode  = PARITY_NONE;
    cfg.serial_bit_order    = 0;
    cfg.serial_invert_mode  = 0;
    cfg.ext_uart_no         = SENSOR_DEVICE_PS5308_DEV_EXT_PORT;
    cfg.port_configure      = PORT_CFG_INIT;

    result = ioctl(sdev->fd, OPE_INT, &cfg);

    utask_x active_task;
    const char name[NAME_NUM_MAX] = "ps5308_task";

    strncpy(active_task.name, name, strlen(name));
    active_task.func_entry  = ReadTask;
    active_task.func_param  = sdev;
    active_task.prio        = KTASK_PRIORITY_MAX/2;
    active_task.stack_size  = 1024;

    active_task_id = UserTaskCreate(active_task);
    result = UserTaskStartup(active_task_id);

    return result;
}

/**
 * @description: Close PS5308 sensor device
 * @param sdev - sensor device pointer
 * @return EOK
 */
static int SensorDeviceClose(struct SensorDevice *sdev)
{
    UserTaskDelete(active_task_id);
    UserMutexDelete(buff_lock);
    return EOK;
}

/**
 * @description: Read sensor device
 * @param sdev - sensor device pointer
 * @param len - the length of the read data
 * @return get data length
 */
static x_size_t SensorDeviceRead(struct SensorDevice *sdev, size_t len)
{
    uint8 tmp = 0;
    int timeout = 0;
    while (1) {
        read(sdev->fd, &tmp, 1);
        if ((tmp == 0x44) || (timeout >= 1000))
            break;
        UserTaskDelay(10);
        ++timeout;
    }
    
    if (tmp != 0x44)
        return -ERROR;

    uint8 idx = 0;
    sdev->buffer[idx++] = tmp;

    while ((idx < len) && (timeout < 1000)) {
        if (read(sdev->fd, &tmp, 1) == 1) {
            timeout = 0;
            sdev->buffer[idx++] = tmp;
        }
        ++timeout;
    }

    return idx;
}

static struct SensorDone SensorDeviceDone =
{
    SensorDeviceOpen,
    SensorDeviceClose,
    SensorDeviceRead,
    NONE,
    NONE,
};

/**
 * @description: Init PS5308 sensor and register
 * @return void
 */
static void Ps5308Init(void)
{
    ps5308.name = SENSOR_DEVICE_PS5308;
    ps5308.info = &info;
    ps5308.done = &SensorDeviceDone;
    ps5308.status = SENSOR_DEVICE_ACTIVE;

    SensorDeviceRegister(&ps5308);
}


#ifdef SENSOR_QUANTITY_PS5308_PM1_0
static struct SensorQuantity ps5308_pm1_0;

/**
 * @description: Analysis PS5308 PM1.0 result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32 ReadPm1_0(struct SensorQuantity *quant)
{
    if (!quant)
        return -ERROR;

    uint32 result;
    if (quant->sdev->done->read != NONE) {
        uint16 checksum = 0;
        UserMutexObtain(buff_lock, WAITING_FOREVER);

        for (uint8 i = 0; i < 30; i++)
            checksum += quant->sdev->buffer[i];          
        
        if (checksum == (((uint16)quant->sdev->buffer[30] << 8) + ((uint16)quant->sdev->buffer[31]))) {
            result = ((uint16)quant->sdev->buffer[4] << 8) + (uint16)quant->sdev->buffer[5];
            if (result > quant->value.max_value)
                quant->value.max_value = result;
            else if (result < quant->value.min_value)
                quant->value.min_value = result;
            quant->value.last_value = result;

            return result;
        }else{
            printf("This reading is wrong\n");
            result = SENSOR_QUANTITY_VALUE_ERROR;
            return result;
        }
    }else{
        printf("%s don't have read done.\n", quant->name);
    }
    
    return -ERROR;
}

/**
 * @description: Init PS5308 PM1.0 quantity and register
 * @return 0
 */
int Ps5308Pm1_0Init(void)
{
    Ps5308Init();
    
    ps5308_pm1_0.name = SENSOR_QUANTITY_PS5308_PM1_0;
    ps5308_pm1_0.type = SENSOR_QUANTITY_PM;
    ps5308_pm1_0.value.decimal_places = 0;
    ps5308_pm1_0.value.max_std = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm1_0.value.min_std = 0;
    ps5308_pm1_0.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm1_0.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm1_0.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm1_0.sdev = &ps5308;
    ps5308_pm1_0.ReadValue = ReadPm1_0;

    SensorQuantityRegister(&ps5308_pm1_0);

    return 0;
}
#endif

#ifdef SENSOR_QUANTITY_PS5308_PM2_5
static struct SensorQuantity ps5308_pm2_5;

/**
 * @description: Analysis PS5308 PM2.5 result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32 ReadPm2_5(struct SensorQuantity *quant)
{
    if (!quant)
        return -ERROR;

    uint32 result;
    if (quant->sdev->done->read != NONE) {
        uint16 checksum = 0;
        UserMutexObtain(buff_lock, WAITING_FOREVER);

        for (uint i = 0; i < 30; i++)
            checksum += quant->sdev->buffer[i];
        
        if (checksum == (((uint16)quant->sdev->buffer[30] << 8) + ((uint16)quant->sdev->buffer[31]))) {
            result = ((uint16)quant->sdev->buffer[6] << 8) + (uint16)quant->sdev->buffer[7];
            if (result > quant->value.max_value)
                quant->value.max_value = result;
            else if (result < quant->value.min_value)
                quant->value.min_value = result;
            quant->value.last_value = result;

            return result;
        }else{
            printf("This reading is wrong\n");
            result = SENSOR_QUANTITY_VALUE_ERROR;
            return result;
        }
    }else{
        printf("%s don't have read done.\n", quant->name);
    }
    
    return -ERROR;
}

/**
 * @description: Init PS5308 PM2.5 quantity and register
 * @return 0
 */
int Ps5308Pm2_5Init(void)
{
    Ps5308Init();
    
    ps5308_pm2_5.name = SENSOR_QUANTITY_PS5308_PM2_5;
    ps5308_pm2_5.type = SENSOR_QUANTITY_PM;
    ps5308_pm2_5.value.decimal_places = 0;
    ps5308_pm2_5.value.max_std = 35;
    ps5308_pm2_5.value.min_std = 0;
    ps5308_pm2_5.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm2_5.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm2_5.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm2_5.sdev = &ps5308;
    ps5308_pm2_5.ReadValue = ReadPm2_5;

    SensorQuantityRegister(&ps5308_pm2_5);

    return 0;
}
#endif

#ifdef SENSOR_QUANTITY_PS5308_PM10
static struct SensorQuantity ps5308_pm10;

/**
 * @description: Analysis PS5308 PM10 result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32 ReadPm10(struct SensorQuantity *quant)
{
    if (!quant)
        return -ERROR;

    uint32 result;
    if (quant->sdev->done->read != NONE) {
        uint16 checksum = 0;
        UserMutexObtain(buff_lock, WAITING_FOREVER);

        for (uint i = 0; i < 30; i++)
            checksum += quant->sdev->buffer[i];
        
        if (checksum == (((uint16)quant->sdev->buffer[30] << 8) + ((uint16)quant->sdev->buffer[31]))) {
            result = ((uint16)quant->sdev->buffer[8] << 8) + (uint16)quant->sdev->buffer[9];
            if (result > quant->value.max_value)
                quant->value.max_value = result;
            else if (result < quant->value.min_value)
                quant->value.min_value = result;
            quant->value.last_value = result;

            return result;
        }else{
            printf("This reading is wrong\n");
            result = SENSOR_QUANTITY_VALUE_ERROR;
            return result;
        }
    }else{
        printf("%s don't have read done.\n", quant->name);
    }
    
    return -ERROR;
}

/**
 * @description: Init PS5308 PM10 quantity and register
 * @return 0
 */
int Ps5308Pm10Init(void)
{
    Ps5308Init();
    
    ps5308_pm10.name = SENSOR_QUANTITY_PS5308_PM10;
    ps5308_pm10.type = SENSOR_QUANTITY_PM;
    ps5308_pm10.value.decimal_places = 0;
    ps5308_pm10.value.max_std = 75;
    ps5308_pm10.value.min_std = 0;
    ps5308_pm10.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm10.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm10.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    ps5308_pm10.sdev = &ps5308;
    ps5308_pm10.ReadValue = ReadPm10;

    SensorQuantityRegister(&ps5308_pm10);

    return 0;
}
#endif
