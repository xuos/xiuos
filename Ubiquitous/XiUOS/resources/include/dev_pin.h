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
* @file dev_pin.h
* @brief define pin dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_PIN_H
#define DEV_PIN_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_LOW                         0x00
#define GPIO_HIGH                        0x01

#define GPIO_CFG_OUTPUT                          0x00
#define GPIO_CFG_INPUT                               0x01
#define GPIO_CFG_INPUT_PULLUP            0x02
#define GPIO_CFG_INPUT_PULLDOWN     0x03
#define GPIO_CFG_OUTPUT_OD                  0x04

#define GPIO_IRQ_EDGE_RISING             0x00
#define GPIO_IRQ_EDGE_FALLING          0x01
#define GPIO_IRQ_EDGE_BOTH               0x02
#define GPIO_IRQ_LEVEL_HIGH               0x03
#define GPIO_IRQ_LEVEL_LOW                 0x04

#define GPIO_CONFIG_MODE                 0xffffffff
#define GPIO_IRQ_REGISTER                  0xfffffffe
#define GPIO_IRQ_FREE                            0xfffffffd
#define GPIO_IRQ_DISABLE                     0xfffffffc
#define GPIO_IRQ_ENABLE                      0xfffffffb

struct PinDevIrq
{
    int irq_mode;//< RISING/FALLING/HIGH/LOW
    void (*hdr) (void *args);//< callback function
    void *args;//< the params of callback function
};

struct PinParam
{
    int cmd;//< cmd:GPIO_CONFIG_MODE/GPIO_IRQ_REGISTER/GPIO_IRQ_FREE/GPIO_IRQ_DISABLE/GPIO_IRQ_ENABLE
    x_base  pin;//< pin number
    int mode;//< pin mode: input/output
    struct PinDevIrq irq_set;//< pin irq set
    uint64 arg;
};

struct PinStat
{
    x_base pin;//< pin number
    uint16 val;//< pin level
};

struct PinIrqHdr
{
    int16 pin;
    uint16 mode;
    void (*hdr) (void *args);
    void *args;
};

struct PinDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct PinHardwareDevice
{
    struct HardwareDev haldev;

    const struct PinDevDone *dev_done;
    
    void *private_data;
};

/*Register the Pin device*/
int PinDeviceRegister(struct PinHardwareDevice *pin_device, void *pin_param, const char *device_name);

/*Register the Pin Device to the Pin bus*/
int PinDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register Pin device*/
HardwareDevType PinDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
