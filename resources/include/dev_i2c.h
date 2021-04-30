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
* @file dev_i2c.h
* @brief define i2c dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_I2C_H
#define DEV_I2C_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SLAVE_ADDR           0x44
#define I2C_WR                               0x0000
#define I2C_RD                                (1u << 0)
#define I2C_ADDR_10BIT            (1u << 2)
#define I2C_NO_START                (1u << 4)
#define I2C_IGNORE_NACK        (1u << 5)
#define I2C_NO_READ_ACK       (1u << 6)

struct I2cDataStandard
{
    uint16 addr;
    uint16 flags;
    uint16 len;
    uint16 retries;
    uint8 *buf;

    struct I2cDataStandard *next;
};

struct I2cHardwareDevice;

struct I2cDevDone
{
    uint32 (*open) (struct I2cHardwareDevice *i2c_device);
    uint32 (*close) (struct I2cHardwareDevice *i2c_device);
    uint32 (*write) (struct I2cHardwareDevice *i2c_device, struct I2cDataStandard *msg);
    uint32 (*read) (struct I2cHardwareDevice *i2c_device, struct I2cDataStandard *msg);
};

struct I2cHardwareDevice
{
    struct HardwareDev haldev;
    const struct I2cDevDone *i2c_dev_done;

    void *private_data;
};

/*Register the I2C device*/
int I2cDeviceRegister(struct I2cHardwareDevice *i2c_device, void *i2c_param, const char *device_name);

/*Register the I2C device to the I2C bus*/
int I2cDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register I2C device*/
HardwareDevType I2cDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
