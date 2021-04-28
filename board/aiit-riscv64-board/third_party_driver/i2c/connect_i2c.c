/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 */

/**
* @file connect_i2c.c
* @brief support aiit-riscv64-board i2c function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_i2c.c
Description: support aiit-riscv64-board i2c configure and i2c bus register function
Others: take RT-Thread v4.0.2/components/drivers/i2c/i2c-bit-ops.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support aiit-riscv64-board i2c bit configure, write and read
2. support aiit-riscv64-board i2c bus device and driver register
*************************************************/

#include <board.h>
#include <connect_i2c.h>
#include <fpioa.h>
#include <gpio_common.h>
#include <gpio.h>
#include <sleep.h>
#include <sysctl.h>

#ifndef BSP_USING_I2C1
#define BSP_USING_I2C1
#endif

#define I2C_SDA_FUNC_GPIO 3
#define I2C_SCL_FUNC_GPIO 4

static I2cBusParam i2c_bus_param =
{
    I2C_SDA_FUNC_GPIO,
    I2C_SCL_FUNC_GPIO,
};

#define SET_SDA(done, val)   done->SetSdaState(done->data, val)
#define SET_SCL(done, val)   done->SetSclState(done->data, val)
#define GET_SDA(done)        done->GetSdaState(done->data)
#define GET_SCL(done)        done->GetSclState(done->data)
#define SdaLow(done)        SET_SDA(done, 0)
#define SdaHigh(done)       SET_SDA(done, 1)
#define SclLow(done)          SET_SCL(done, 0)

void I2cGpioInit(const I2cBusParam *bus_param)
{
    gpio_init ();
    FpioaSetFunction(BSP_I2C_SDA , FUNC_GPIO3 );//RISC-V FPIOA CFG
    FpioaSetFunction(BSP_I2C_SCL , FUNC_GPIO4 );//RISC-V FPIOA CFG
    gpio_set_drive_mode(bus_param->i2c_sda_pin , GPIO_DM_OUTPUT );
    gpio_set_drive_mode(bus_param->i2c_scl_pin, GPIO_DM_OUTPUT );
    gpio_set_pin(bus_param->i2c_sda_pin , GPIO_PV_HIGH );
    gpio_set_pin(bus_param->i2c_scl_pin , GPIO_PV_HIGH );    
}

static void SetSdaState(void *data, uint8 sda_state)
{
    I2cBusParam *bus_param = (I2cBusParam *)data;
    if (sda_state) {
       gpio_set_drive_mode(bus_param->i2c_sda_pin, GPIO_DM_OUTPUT );
       gpio_set_pin(bus_param->i2c_sda_pin , GPIO_PV_HIGH );  
    } else {
       gpio_set_drive_mode(bus_param->i2c_sda_pin, GPIO_DM_OUTPUT );
       gpio_set_pin(bus_param->i2c_sda_pin , GPIO_PV_LOW );  
    }
}

static void SetSclState(void *data, uint8 scl_state)
{
    I2cBusParam *bus_param = (I2cBusParam *)data;
    if (scl_state) {
        gpio_set_drive_mode(bus_param->i2c_scl_pin, GPIO_DM_OUTPUT );
        gpio_set_pin(bus_param->i2c_scl_pin , GPIO_PV_HIGH ); 
    } else {
        gpio_set_drive_mode(bus_param->i2c_scl_pin, GPIO_DM_OUTPUT );
        gpio_set_pin(bus_param->i2c_scl_pin , GPIO_PV_LOW ); 
    }
}

static uint8 GetSdaState(void *data)
{
    I2cBusParam *bus_param = (I2cBusParam *)data;
    gpio_set_drive_mode (bus_param->i2c_sda_pin, GPIO_DM_INPUT_PULL_UP );  
    return gpio_get_pin(bus_param->i2c_sda_pin);
}

static uint8 GetSclState(void *data)
{
    I2cBusParam *bus_param = (I2cBusParam *)data;
    gpio_set_drive_mode (bus_param->i2c_scl_pin, GPIO_DM_INPUT_PULL_UP );
    return gpio_get_pin(bus_param->i2c_scl_pin);
}

static const struct I2cHalDrvDone i2c_hal_drv_done =
{
    .data = (&i2c_bus_param),
    .SetSdaState = SetSdaState,
    .SetSclState = SetSclState,
    .GetSdaState = GetSdaState,
    .GetSclState = GetSclState,
    .udelay = usleep,
    .delay_us = 1,
    .timeout = 100
};

static x_err_t I2cBusReset(const I2cBusParam *bus_param)
{
    int32 i = 0;
    gpio_set_drive_mode(bus_param->i2c_sda_pin, GPIO_DM_INPUT_PULL_UP );   
    if (GPIO_LOW == gpio_get_pin(bus_param->i2c_sda_pin)) {
        while (i++ < 9)
        {
            gpio_set_drive_mode(bus_param->i2c_scl_pin, GPIO_DM_OUTPUT );
            gpio_set_pin(bus_param->i2c_scl_pin , GPIO_PV_HIGH ); 
            usleep(100);
            gpio_set_pin(bus_param->i2c_scl_pin , GPIO_PV_LOW ); 
            usleep(100);
        }
    }
    gpio_set_drive_mode(bus_param->i2c_sda_pin, GPIO_DM_INPUT_PULL_UP );   
    if (GPIO_LOW == gpio_get_pin(bus_param->i2c_sda_pin)) {
        return -ERROR;
    }
    return EOK;
}

static __inline void I2cDelay(struct I2cHalDrvDone *done)
{
    done->udelay((done->delay_us + 1) >> 1);
}

static __inline void I2cDelay2(struct I2cHalDrvDone *done)
{
    done->udelay(done->delay_us);
}

static x_err_t SclHigh(struct I2cHalDrvDone *done)
{
    x_ticks_t start;

    SET_SCL(done, 1);

    if (!done->GetSclState)
        goto done;

    start = CurrentTicksGain();
    while (!GET_SCL(done))
    {
        if ((CurrentTicksGain() - start) > done->timeout)
            return -ETIMEOUT;
        DelayKTask((done->timeout + 1) >> 1);
    }

done:
    I2cDelay(done);

    return EOK;
}

static void I2cStart(struct I2cHalDrvDone *done)
{
    SdaLow(done);
    I2cDelay(done);
    SclLow(done);
}

static void I2cRestart(struct I2cHalDrvDone *done)
{
    SdaHigh(done);
    SclHigh(done);
    I2cDelay(done);
    SdaLow(done);
    I2cDelay(done);
    SclLow(done);
}

static void I2cStop(struct I2cHalDrvDone *done)
{
    SdaLow(done);
    I2cDelay(done);
    SclHigh(done);
    I2cDelay(done);
    SdaHigh(done);
    I2cDelay2(done);
}

static __inline x_bool I2cWaitack(struct I2cHalDrvDone *done)
{
    x_bool ack;

    SdaHigh(done);
     GET_SDA(done);    
    I2cDelay(done);

    if (SclHigh(done) < 0) {
        KPrintf("wait ack timeout");
        return -ETIMEOUT;
    }

    ack = !GET_SDA(done);

    SclLow(done);

    return ack;
}

static int32 I2cWriteb(struct I2cBus *bus, uint8 data)
{
    int32 i;
    uint8 bit;

    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;

    for (i = 7; i >= 0; i--)
    {
        SclLow(done);
        bit = (data >> i) & 1;
        SET_SDA(done, bit);
        I2cDelay(done);
        if (SclHigh(done) < 0) {
            KPrintf("I2cWriteb: 0x%02x, "
                    "wait scl pin high timeout at bit %d",
                    data, i);

            return -ETIMEOUT;
        }
    }
    SclLow(done);
    I2cDelay(done);

    return I2cWaitack(done);
}

static int32 I2cReadb(struct I2cBus *bus)
{
    uint8 i;
    uint8 data = 0;
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;

    SdaHigh(done);
    GET_SDA(done);
    I2cDelay(done);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        if (SclHigh(done) < 0) {
            KPrintf("I2cReadb: wait scl pin high "
                    "timeout at bit %d", 7 - i);

            return -ETIMEOUT;
        }

        if (GET_SDA(done))
            data |= 1;
        SclLow(done);
        I2cDelay2(done);
    }

    return data;
}

static x_size_t I2cSendBytes(struct I2cBus *bus, struct I2cDataStandard *msg)
{
    int32 ret;
    x_size_t bytes = 0;
    const uint8 *ptr = msg->buf;
    int32 count = msg->len;
    uint16 ignore_nack = msg->flags & I2C_IGNORE_NACK;

    while (count > 0)
    {
        ret = I2cWriteb(bus, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0))) {
            count --;
            ptr ++;
            bytes ++;
        } else if (ret == 0) {
            //KPrintf("send bytes: NACK.");

            return 0;
        } else {
            KPrintf("send bytes: error %d", ret);

            return ret;
        }
    }

    return bytes;
}

static x_err_t I2cSendAckOrNack(struct I2cBus *bus, int ack)
{
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;

    if (ack)
        SET_SDA(done, 0);
    I2cDelay(done);
    if (SclHigh(done) < 0) {
        KPrintf("ACK or NACK timeout.");

        return -ETIMEOUT;
    }
    SclLow(done);

    return EOK;
}

static x_size_t I2cRecvBytes(struct I2cBus *bus, struct I2cDataStandard *msg)
{
    int32 val;
    int32 bytes = 0;
    uint8 *ptr = msg->buf;
    int32 count = msg->len;
    const uint32 flags = msg->flags;

    while (count > 0)
    {
        val = I2cReadb(bus);
        if (val >= 0) {
            *ptr = val;
            bytes ++;
        } else {
            break;
        }

        ptr ++;
        count --;

        if (!(flags & I2C_NO_READ_ACK)) {
            val = I2cSendAckOrNack(bus, count);
            if (val < 0)
                return val;
        }
    }

    return bytes;
}

static int32 I2cSendAddress(struct I2cBus *bus, uint8 addr, int32 retries)
{
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;
    int32 i;
    x_err_t ret = 0;

    for (i = 0; i <= retries; i++)
    {
        ret = I2cWriteb(bus, addr);
        if (ret == 1 || i == retries)
            break;
        I2cStop(done);
        I2cDelay2(done);
        I2cStart(done);
    }

    return ret;
}

static x_err_t I2cBitSendAddress(struct I2cBus *bus, struct I2cDataStandard *msg)
{
    uint16 flags = msg->flags;
    uint16 ignore_nack = msg->flags & I2C_IGNORE_NACK;
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;

    uint8 addr1, addr2;
    int32 retries;
    x_err_t ret;

    retries = ignore_nack ? 0 : msg->retries;

    if (flags & I2C_ADDR_10BIT) {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;

        ret = I2cSendAddress(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack) {
            KPrintf("NACK: sending first addr");

            return -EPIO;
        }

        ret = I2cWriteb(bus, addr2);
        if ((ret != 1) && !ignore_nack) {
            //KPrintf("NACK: sending second addr");

            return -EPIO;
        }

        if (flags & I2C_RD) {
            I2cRestart(done);
            addr1 |= 0x01;
            ret = I2cSendAddress(bus, addr1, retries);
            if ((ret != 1) && !ignore_nack) {

                return -EPIO;
            }
        }
    } else {
        addr1 = msg->addr << 1;
        if (flags & I2C_RD)
            addr1 |= 1;
        ret = I2cSendAddress(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
            return -EPIO;
    }

    return EOK;
}

static uint32 I2cWriteData(struct I2cHardwareDevice *i2c_dev, struct I2cDataStandard *msg)
{
    struct I2cBus *bus = (struct I2cBus *)i2c_dev->haldev.owner_bus;
    bus->private_data = i2c_dev->haldev.owner_bus->private_data;
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;
    int32 ret;
    int32 i = 0;
    uint16 ignore_nack;

    I2cStart(done);
    while(NONE != msg)
    {
        ignore_nack = msg->flags & I2C_IGNORE_NACK;
        if (!(msg->flags & I2C_NO_START)) {
            if (i) {
                I2cRestart(done);
            }
            ret = I2cBitSendAddress(bus, msg);
            if ((ret != EOK) && !ignore_nack) {
                goto out;
            }
        }

        if (msg->flags & I2C_WR) {
            ret = I2cSendBytes(bus, msg);
            if (ret >= 1)
                //KPrintf("write %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len) {
                if (ret >= 0)
                    ret = -ERROR;
                goto out;
            }
        }
        msg = msg->next;
        i++;
    }
    ret = i;

out:
    I2cStop(done);

    return ret;
}

static uint32 I2cReadData(struct I2cHardwareDevice *i2c_dev, struct I2cDataStandard *msg)
{
    struct I2cBus *bus = (struct I2cBus *)i2c_dev->haldev.owner_bus;
    bus->private_data = i2c_dev->haldev.owner_bus->private_data;
    struct I2cHalDrvDone *done = (struct I2cHalDrvDone *)bus->private_data;
    int32 ret;
    int32 i = 0;
    uint16 ignore_nack;

    I2cStart(done);
    while (NONE != msg)
    {
        ignore_nack = msg->flags & I2C_IGNORE_NACK;
        if (!(msg->flags & I2C_NO_START)) {
            if (i) {
                I2cRestart(done);
            }
            ret = I2cBitSendAddress(bus, msg);
            if ((ret != EOK) && !ignore_nack) {
                goto out;
            }
        }

        if (msg->flags & I2C_RD) {
            ret = I2cRecvBytes(bus, msg);
            if (ret >= 1)
                //KPrintf("read %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len) {
                if (ret >= 0)
                    ret = -EPIO;
                goto out;
            }
        }
        msg = msg->next;
        i++;
    }
    ret = i;

out:
    I2cStop(done);

    return ret;
}

/*manage the i2c device operations*/
static const struct I2cDevDone i2c_dev_done =
{
    .open = NONE,
    .close = NONE,
    .write = I2cWriteData,
    .read = I2cReadData,
};

/*Init i2c bus*/
static int BoardI2cBusInit(struct I2cBus *i2c_bus, struct I2cDriver *i2c_driver)
{
    x_err_t ret = EOK;

    /*Init the i2c bus */
    i2c_bus->private_data = (void *)&i2c_hal_drv_done;
    ret = I2cBusInit(i2c_bus, I2C_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("board_i2c_init I2cBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the i2c driver*/
    i2c_driver->private_data = (void *)&i2c_hal_drv_done;
    ret = I2cDriverInit(i2c_driver, I2C_DRV_NAME_1);
    if (EOK != ret) {
        KPrintf("board_i2c_init I2cDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the i2c driver to the i2c bus*/
    ret = I2cDriverAttachToBus(I2C_DRV_NAME_1, I2C_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("board_i2c_init I2cDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the i2c device to the i2c bus*/
static int BoardI2cDevBend(void)
{
    x_err_t ret = EOK;
    static struct I2cHardwareDevice i2c_device0;
    memset(&i2c_device0, 0, sizeof(struct I2cHardwareDevice));

    i2c_device0.i2c_dev_done = &i2c_dev_done;

    ret = I2cDeviceRegister(&i2c_device0, NONE, I2C_1_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("board_i2c_init I2cDeviceInit device %s error %d\n", I2C_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    ret = I2cDeviceAttachToBus(I2C_1_DEVICE_NAME_0, I2C_BUS_NAME_1);
    if (EOK != ret) {
        KPrintf("board_i2c_init I2cDeviceAttachToBus device %s error %d\n", I2C_1_DEVICE_NAME_0, ret);
        return ERROR;
    }  

    return ret;
}

/*RISC-V 64 BOARD I2C INIT*/
int HwI2cInit(void)
{
    x_err_t ret = EOK;
    static struct I2cBus i2c_bus;
    memset(&i2c_bus, 0, sizeof(struct I2cBus));

    static struct I2cDriver i2c_driver;
    memset(&i2c_driver, 0, sizeof(struct I2cDriver));

#ifdef  BSP_USING_I2C1
    I2cGpioInit(&i2c_bus_param);

    ret = BoardI2cBusInit(&i2c_bus, &i2c_driver);
    if (EOK != ret) {
        KPrintf("board_i2c_Init error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardI2cDevBend();
    if (EOK != ret) {
        KPrintf("board_i2c_Init error ret %u\n", ret);
        return ERROR;
    }   

    I2cBusReset(&i2c_bus_param);
#endif

    return ret;
}
