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
 * @file d124.c
 * @brief D124 voice driver base perception
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <sensor.h>

static struct SensorDevice d124;
static int32_t active_task_id;
static int buff_lock;

static struct SensorProductInfo info =
{
    SENSOR_ABILITY_VOICE,
    "龙戈电子",
    "D124",
};

/**
 * @description: Read sensor task
 * @param sdev - sensor device pointer
 */
static void ReadTask(struct SensorDevice *sdev)
{
    while (1) {
        UserMutexObtain(buff_lock, WAITING_FOREVER);
        sdev->done->read(sdev, 5);
        UserMutexAbandon(buff_lock);
        UserTaskDelay(750);
    }
}

/**
 * @description: Open D124 voice device
 * @param sdev - sensor device pointer
 * @return success: EOK , failure: other
 */
static int SensorDeviceOpen(struct SensorDevice *sdev)
{
    int result = EOK;

    buff_lock = UserMutexCreate();

    sdev->fd = open(SENSOR_DEVICE_D124_DEV, O_RDWR);
    
    struct SerialDataCfg cfg;
    cfg.serial_baud_rate    = BAUD_RATE_9600;
    cfg.serial_data_bits    = DATA_BITS_8;
    cfg.serial_stop_bits    = STOP_BITS_1;
    cfg.serial_buffer_size  = 64;
    cfg.serial_parity_mode  = PARITY_NONE;
    cfg.serial_bit_order    = 0;
    cfg.serial_invert_mode  = 0;
    cfg.ext_uart_no         = SENSOR_DEVICE_D124_DEV_EXT_PORT;
    cfg.port_configure      = PORT_CFG_INIT;

    result = ioctl(sdev->fd, OPE_INT, &cfg);

    utask_x active_task;
    const char name[NAME_NUM_MAX] = "d124_task";

    strncpy(active_task.name, name, strlen(name));
    active_task.func_entry  = ReadTask;
    active_task.func_param  = sdev;
    active_task.prio        = KTASK_PRIORITY_MAX/2;
    active_task.stack_size  = 2048;

    active_task_id = UserTaskCreate(active_task);
    result = UserTaskStartup(active_task_id);

    return result;
}

/**
 * @description: Close D124 sensor device
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
        if ((tmp == 0xAA) || (timeout >= 1000))
            break;
        UserTaskDelay(10);
        ++timeout;
    }
    
    if(tmp != 0xAA)
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

static struct SensorDone done =
{
    SensorDeviceOpen,
    SensorDeviceClose,
    SensorDeviceRead,
    NONE,
    NONE,
};

/**
 * @description: Init D124 sensor and register
 * @return void
 */
static void D124Init(void)
{
    d124.name = SENSOR_DEVICE_D124;
    d124.info = &info;
    d124.done = &done;
    d124.status = SENSOR_DEVICE_ACTIVE;

    SensorDeviceRegister(&d124);
}


static struct SensorQuantity d124_voice;

/**
 * @description: Analysis D124 voice result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32 ReadVoice(struct SensorQuantity *quant)
{
    if (!quant)
        return -ERROR;

    uint32 result;
    if (quant->sdev->done->read != NONE) {
        UserMutexObtain(buff_lock, WAITING_FOREVER);      
        
        if (quant->sdev->buffer[3] == quant->sdev->buffer[1] + quant->sdev->buffer[2]) {
            result = ((uint16)quant->sdev->buffer[1] << 8) + (uint16)quant->sdev->buffer[2];
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
 * @description: Init D124 voice quantity and register
 * @return 0
 */
int D124VoiceInit(void)
{
    D124Init();
    
    d124_voice.name = SENSOR_QUANTITY_D124_VOICE;
    d124_voice.type = SENSOR_QUANTITY_VOICE;
    d124_voice.value.decimal_places = 1;
    d124_voice.value.max_std = 600;
    d124_voice.value.min_std = 0;
    d124_voice.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    d124_voice.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    d124_voice.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    d124_voice.sdev = &d124;
    d124_voice.ReadValue = ReadVoice;

    SensorQuantityRegister(&d124_voice);

    return 0;
}
