/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-19     ZYH          first version
 */

/**
* @file connect_gpio.c
* @brief support gpio function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_gpio.c
Description: support  gpio configure and register to bus framework
Others: take RT-Thread v4.0.2/bsp/k210/driver/drv_gpio.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: add bus driver framework support for gpio
*************************************************/

#include <xiuos.h>
#include <device.h>
#include <fpioa.h>
#include <gpiohs.h>
#include "drv_io_config.h"
#include <plic.h>
#include <utils.h>
#include "connect_gpio.h"

#define FUNC_GPIOHS(n) (FUNC_GPIOHS0 + n)

static int pin_alloc_table[FPIOA_NUM_IO];
static uint32_t free_pin = 0;

static int AllocPinChannel(x_base pin_index)
{
    if (free_pin == 31) {
        SYS_ERR("no free gpiohs channel to alloc");
        return -1;
    }

    if (pin_alloc_table[pin_index] != -1) {
        SYS_WARN("already alloc gpiohs channel for pin %d", pin_index);
        return pin_alloc_table[pin_index];
    }

    pin_alloc_table[pin_index] = free_pin;
    free_pin++;

    FpioaSetFunction(pin_index, FUNC_GPIOHS(pin_alloc_table[pin_index]));
    return pin_alloc_table[pin_index];
}

static int GetPinChannel(x_base pin_index)
{
    return pin_alloc_table[pin_index];
}

static uint32 GpioConfigMode(int mode, uint8_t pin_channel)
{
    switch (mode)
    {
        case GPIO_CFG_OUTPUT:
            gpiohs_set_drive_mode(pin_channel, GPIO_DM_OUTPUT);
            break;
        case GPIO_CFG_INPUT:
            gpiohs_set_drive_mode(pin_channel, GPIO_DM_INPUT);
            break;
        case GPIO_CFG_INPUT_PULLUP:
            gpiohs_set_drive_mode(pin_channel, GPIO_DM_INPUT_PULL_UP);
            break;
        case GPIO_CFG_INPUT_PULLDOWN:
            gpiohs_set_drive_mode(pin_channel, GPIO_DM_INPUT_PULL_DOWN);
            break;
        case GPIO_CFG_OUTPUT_OD:
            break;
        default:
            break;
    }
    return EOK;

}

static struct 
{
    void (*hdr)(void *args);
    void *args;
    GpioPinEdgeT edge;
} IrqTable[32];

static void pin_irq(int vector, void *param)
{
    int pin_channel = vector - IRQN_GPIOHS0_INTERRUPT;
    if (IrqTable[pin_channel].edge & GPIO_PE_FALLING) {
        set_gpio_bit(gpiohs->fall_ie.u32, pin_channel, 0);
        set_gpio_bit(gpiohs->fall_ip.u32, pin_channel, 1);
        set_gpio_bit(gpiohs->fall_ie.u32, pin_channel, 1);
    }

    if (IrqTable[pin_channel].edge & GPIO_PE_RISING) {
        set_gpio_bit(gpiohs->rise_ie.u32, pin_channel, 0);
        set_gpio_bit(gpiohs->rise_ip.u32, pin_channel, 1);
        set_gpio_bit(gpiohs->rise_ie.u32, pin_channel, 1);
    }

    if (IrqTable[pin_channel].edge & GPIO_PE_LOW) {
        set_gpio_bit(gpiohs->low_ie.u32, pin_channel, 0);
        set_gpio_bit(gpiohs->low_ip.u32, pin_channel, 1);
        set_gpio_bit(gpiohs->low_ie.u32, pin_channel, 1);
    }

    if (IrqTable[pin_channel].edge & GPIO_PE_HIGH) {
        set_gpio_bit(gpiohs->high_ie.u32, pin_channel, 0);
        set_gpio_bit(gpiohs->high_ip.u32, pin_channel, 1);
        set_gpio_bit(gpiohs->high_ie.u32, pin_channel, 1);
    }

    if (IrqTable[pin_channel].hdr) {
        IrqTable[pin_channel].hdr(IrqTable[pin_channel].args);
    }
}

static uint32 GpioIrqRegister(int32 pin_channel, int32 mode, void (*hdr)(void *args), void *args)
{
    IrqTable[pin_channel].hdr = hdr;
    IrqTable[pin_channel].args = args;
    switch (mode)
    {
        case GPIO_IRQ_EDGE_RISING:
            IrqTable[pin_channel].edge = GPIO_PE_RISING;
            break;
        case GPIO_IRQ_EDGE_FALLING:
            IrqTable[pin_channel].edge = GPIO_PE_FALLING;
            break;
        case GPIO_IRQ_EDGE_BOTH:
            IrqTable[pin_channel].edge = GPIO_PE_BOTH;
            break;
        case GPIO_IRQ_LEVEL_HIGH:
            IrqTable[pin_channel].edge = GPIO_PE_LOW;
            break;
        case GPIO_IRQ_LEVEL_LOW:
            IrqTable[pin_channel].edge = GPIO_PE_HIGH;
            break;
        default:
            break;
    }
    gpiohs_set_pin_edge(pin_channel, IrqTable[pin_channel].edge);

    isrManager.done->registerIrq(IRQN_GPIOHS0_INTERRUPT + pin_channel, pin_irq, NONE);
 
    return EOK;
}

static uint32 GpioIrqFree(int32 pin_channel)
{
    IrqTable[pin_channel].hdr = NONE;
    IrqTable[pin_channel].args = NONE;
    return EOK;
}

static uint32 GpioIrqEnable(int32 pin_channel)
{
    isrManager.done->enableIrq(IRQN_GPIOHS0_INTERRUPT + pin_channel);
    
    return EOK;
}

static uint32 GpioIrqDisable(int32 pin_channel)
{
     isrManager.done->disableIrq(IRQN_GPIOHS0_INTERRUPT + pin_channel);
     return EOK;
}

static uint32 PinConfigure(struct PinParam *param)
{
    NULL_PARAM_CHECK(param);
    int ret = EOK;

    int pin_channel = GetPinChannel(param->pin);
    if (pin_channel == -1) {
        pin_channel = AllocPinChannel(param->pin);
        if (pin_channel == -1) {
            return ERROR;
        }
    }

    switch (param->cmd)
    {
        case GPIO_CONFIG_MODE:
            GpioConfigMode(param->mode, pin_channel);
            break;
        case GPIO_IRQ_REGISTER:
            ret = GpioIrqRegister(pin_channel,param->irq_set.irq_mode,param->irq_set.hdr,param->irq_set.args);
            break;
        case GPIO_IRQ_FREE:
            ret = GpioIrqFree(pin_channel);
            break;
        case GPIO_IRQ_ENABLE:
            ret = GpioIrqEnable(pin_channel);
            break;
        case GPIO_IRQ_DISABLE:
            ret = GpioIrqDisable(pin_channel);
            break;
        default:
            ret = -EINVALED;
            break;
    }

    return ret;
}

static uint32 GpioDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    struct PinParam *param;

    switch (configure_info->configure_cmd)
    {
        case OPE_CFG:
            param = (struct PinParam *)configure_info->private_data;
            ret = PinConfigure(param);
            break;
        default:
            break;
    }

    return ret;
}

uint32 PinWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);
    struct PinStat *pinstat = (struct PinStat *)write_param->buffer;
    
    int pin_channel = GetPinChannel(pinstat->pin);
    if (pin_channel == -1) {
        SYS_ERR("pin %d not set mode", pinstat->pin);
        return ERROR;
    }
    gpiohs_set_pin(pin_channel, pinstat->val == GPIO_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
}

uint32 PinRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);
    struct PinStat *pinstat = (struct PinStat *)read_param->buffer;
    
    int pin_channel = GetPinChannel(pinstat->pin);
    if (pin_channel == -1) {
        SYS_ERR("pin %d not set mode", pinstat->pin);
        return ERROR;
    }

    if (gpiohs_get_pin(pin_channel) == GPIO_PV_HIGH){
        pinstat->val = GPIO_HIGH;
        return GPIO_HIGH;
    } else {
        pinstat->val = GPIO_LOW;
        return GPIO_LOW;
    }
}

static const struct PinDevDone dev_done =
{
    .open  = NONE,
    .close = NONE,
    .write = PinWrite,
    .read  = PinRead,
};

int HwGpioInit(void)
{
    x_err_t ret = EOK;

    static struct PinBus pin;

    memset(pin_alloc_table, -1, sizeof pin_alloc_table);
    free_pin = GPIO_ALLOC_START;

    ret = PinBusInit(&pin, PIN_BUS_NAME);
    if (ret != EOK) {
        KPrintf("gpio bus init error %d\n", ret);
        return ERROR;
    }

    static struct PinDriver drv;
    drv.configure = &GpioDrvConfigure;
    
    ret = PinDriverInit(&drv, PIN_DRIVER_NAME, NONE);
    if (ret != EOK) {
        KPrintf("pin driver init error %d\n", ret);
        return ERROR;
    }

    ret = PinDriverAttachToBus(PIN_DRIVER_NAME, PIN_BUS_NAME);
    if (ret != EOK) {
        KPrintf("pin driver attach error %d\n", ret);
        return ERROR;
    }

    static struct PinHardwareDevice dev;
    dev.dev_done = &dev_done;

    ret = PinDeviceRegister(&dev, NONE, PIN_DEVICE_NAME);
    if (ret != EOK) {
        KPrintf("pin device register error %d\n", ret);
        return ERROR;
    }

    ret = PinDeviceAttachToBus(PIN_DEVICE_NAME, PIN_BUS_NAME);
    if (ret != EOK) {
        KPrintf("pin device register error %d\n", ret);
        return ERROR;
    }

    return ret;
}