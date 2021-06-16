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
 * @file zg09.c
 * @brief ZG09 CO2 driver base perception
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <sensor.h>

static uint8_t zg09_set_passive[8]={0xFE, 0x06, 0x00, 0x05, 0x00, 0x00, 0x8D, 0xC4};
static uint8_t zg09_set_active[8]={0xFE, 0x06, 0x00, 0x05, 0x00, 0x01, 0x4C, 0x04};
static uint8_t zg09_read_instruction[8]={0xFE, 0x03, 0x00, 0x0B, 0x00, 0x01, 0xE1, 0xC7};

static struct SensorDevice zg09;

static struct SensorProductInfo info =
{
    SENSOR_ABILITY_CO2,
    "ZyAura",
    "ZG09",
};

/**
 * @description: Open ZG09 sensor device
 * @param sdev - sensor device pointer
 * @return success: 1 , failure: other
 */
static int SensorDeviceOpen(struct SensorDevice *sdev)
{
    int result = 1;

    sdev->fd = PrivOpen(SENSOR_DEVICE_ZG09_DEV, O_RDWR);
    
    struct SerialDataCfg cfg;
    cfg.serial_baud_rate    = BAUD_RATE_9600;
    cfg.serial_data_bits    = DATA_BITS_8;
    cfg.serial_stop_bits    = STOP_BITS_1;
    cfg.serial_buffer_size  = 128;
    cfg.serial_parity_mode  = PARITY_NONE;
    cfg.serial_bit_order    = 0;
    cfg.serial_invert_mode  = 0;
    cfg.ext_uart_no         = SENSOR_DEVICE_ZG09_DEV_EXT_PORT;
    cfg.port_configure      = PORT_CFG_INIT;

    struct PrivIoctlCfg ioctl_cfg;
    ioctl_cfg.ioctl_driver_type = SERIAL_TYPE;
    ioctl_cfg.args = &cfg;
    result = PrivIoctl(sdev->fd, OPE_INT, &ioctl_cfg);

    sdev->done->ioctl(sdev, SENSOR_DEVICE_PASSIVE);

    return result;
}

/**
 * @description: Read sensor device
 * @param sdev - sensor device pointer
 * @param len - the length of the read data
 * @return get data length
 */
static int SensorDeviceRead(struct SensorDevice *sdev, size_t len)
{
    uint8_t tmp = 0;
    int timeout = 0;
    while (1) {
        PrivRead(sdev->fd, &tmp, 1);
        if ((tmp == 0xFE) || (timeout >= 1000))
            break;
        UserTaskDelay(10);
        ++timeout;
    }
    
    if(tmp != 0xFE)
        return -1;

    uint8_t idx = 0;
    sdev->buffer[idx++] = tmp;

    while ((idx < len) && (timeout < 1000)) {
        if (PrivRead(sdev->fd, &tmp, 1) == 1) {
            timeout = 0;
            sdev->buffer[idx++] = tmp;
        }
        ++timeout;
    }

    return idx;
}

/**
 * @description: set sensor work mode
 * @param sdev - sensor device pointer
 * @param cmd - mode command
 * @return success: 1 , failure: -1
 */
static int SensorDeviceIoctl(struct SensorDevice *sdev, int cmd)
{
    switch (cmd)
    {
    case SENSOR_DEVICE_PASSIVE:
        PrivWrite(sdev->fd, zg09_set_passive, 8);
        sdev->done->read(sdev, 8);
        if (memcmp(sdev->buffer, zg09_set_passive, 8) == 0) {
            sdev->status = SENSOR_DEVICE_PASSIVE;
            return 1;
        }
        break;

    case SENSOR_DEVICE_ACTIVE:
        PrivWrite(sdev->fd, zg09_set_active, 8);
        sdev->done->read(sdev, 8);
        if (memcmp(sdev->buffer, zg09_set_active, 8) == 0) {
            sdev->status = SENSOR_DEVICE_ACTIVE;
            return 1;
        }
        break;
    
    default:
        printf("This device does not have this command!\n");
        break;
    }

    return -1;
}

static struct SensorDone done =
{
    SensorDeviceOpen,
    NULL,
    SensorDeviceRead,
    NULL,
    SensorDeviceIoctl,
};

/**
 * @description: Init ZG09 sensor and register
 * @return void
 */
static void SensorDeviceZg09Init(void)
{
    zg09.name = SENSOR_DEVICE_ZG09;
    zg09.info = &info;
    zg09.done = &done;

    SensorDeviceRegister(&zg09);
}



static struct SensorQuantity zg09_co2;

/**
 * @description: Analysis ZG09 CO2 result
 * @param quant - sensor quantity pointer
 * @return quantity value
 */
static int32_t QuantityRead(struct SensorQuantity *quant)
{
    if (!quant)
        return -1;

    uint32_t result;
    if (quant->sdev->done->read != NULL) {
        if(quant->sdev->status == SENSOR_DEVICE_PASSIVE) {
            PrivWrite(quant->sdev->fd, zg09_read_instruction, 8);
            quant->sdev->done->read(quant->sdev, 7);
            if(Crc16(quant->sdev->buffer, 5) == ((uint32_t)quant->sdev->buffer[6] << 8) + ((uint32_t)quant->sdev->buffer[5])) {
                result = (uint32_t)quant->sdev->buffer[3] * 256 + (uint32_t)quant->sdev->buffer[4];
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
 * @description: Init ZG09 CO2 quantity and register
 * @return 0
 */
int Zg09Co2Init(void)
{
    SensorDeviceZg09Init();
    
    zg09_co2.name = SENSOR_QUANTITY_ZG09_CO2;
    zg09_co2.type = SENSOR_QUANTITY_CO2;
    zg09_co2.value.decimal_places = 0;
    zg09_co2.value.max_std = 1000;
    zg09_co2.value.min_std = 350;
    zg09_co2.value.last_value = SENSOR_QUANTITY_VALUE_ERROR;
    zg09_co2.value.max_value = SENSOR_QUANTITY_VALUE_ERROR;
    zg09_co2.value.min_value = SENSOR_QUANTITY_VALUE_ERROR;
    zg09_co2.sdev = &zg09;
    zg09_co2.ReadValue = QuantityRead;

    SensorQuantityRegister(&zg09_co2);

    return 0;
}