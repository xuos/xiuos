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
* @file bus_i2c.h
* @brief define i2c bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_I2C_H
#define BUS_I2C_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct I2cHalDrvDone
{
    void *data;
    void (*SetSdaState) (void *data, uint8 sda_state);
    void (*SetSclState) (void *data, uint8 scl_state);
    uint8 (*GetSdaState) (void *data);
    uint8 (*GetSclState) (void *data);
#ifdef ARCH_RISCV
    int (*udelay)(uint64 us);
#endif
#ifdef ARCH_ARM
    int (*udelay)(uint32 us);
#endif

    uint32 delay_us;
    uint32 timeout;
};

typedef struct
{
    uint8 i2c_sda_pin;
    uint8 i2c_scl_pin;
}I2cBusParam;

struct I2cDriver
{
    struct Driver driver;

    void *private_data;
};

struct I2cBus
{
    struct Bus bus;

    void *private_data;
};

/*Register the I2C bus*/
int I2cBusInit(struct I2cBus *i2c_bus, const char *bus_name);

/*Register the I2C driver*/
int I2cDriverInit(struct I2cDriver *i2c_driver, const char *driver_name);

/*Release the I2C device*/
int I2cReleaseBus(struct I2cBus *i2c_bus);

/*Register the I2C driver to the I2C bus*/
int I2cDriverAttachToBus(const char *drv_name, const char *bus_name);

/*Register the driver, manage with the double linklist*/
int I2cDriverRegister(struct Driver *driver);

/*Find the register driver*/
DriverType I2cDriverFind(const char *drv_name, enum DriverType drv_type);

#ifdef __cplusplus
}
#endif

#endif
