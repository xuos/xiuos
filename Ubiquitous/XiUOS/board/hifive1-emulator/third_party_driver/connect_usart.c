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
* @file connect_usart.c
* @brief support aiit-arm32-board usart function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <xiuos.h>
#include <device.h>

#include <encoding.h>
#include <platform.h>
#include <arch_interrupt.h>
#include <uart.h>
#include <board.h>

static void SerialCfgParamCheck(struct SerialCfgParam *serial_cfg_default, struct SerialCfgParam *serial_cfg_new)
{
    struct SerialDataCfg *data_cfg_default = &serial_cfg_default->data_cfg;
    struct SerialDataCfg *data_cfg_new = &serial_cfg_new->data_cfg;

    if ((data_cfg_default->serial_baud_rate != data_cfg_new->serial_baud_rate) && (data_cfg_new->serial_baud_rate)) {
        data_cfg_default->serial_baud_rate = data_cfg_new->serial_baud_rate;
    }

    if ((data_cfg_default->serial_bit_order != data_cfg_new->serial_bit_order) && (data_cfg_new->serial_bit_order)) {
        data_cfg_default->serial_bit_order = data_cfg_new->serial_bit_order;
    }

    if ((data_cfg_default->serial_buffer_size != data_cfg_new->serial_buffer_size) && (data_cfg_new->serial_buffer_size)) {
        data_cfg_default->serial_buffer_size = data_cfg_new->serial_buffer_size;
    }

    if ((data_cfg_default->serial_data_bits != data_cfg_new->serial_data_bits) && (data_cfg_new->serial_data_bits)) {
        data_cfg_default->serial_data_bits = data_cfg_new->serial_data_bits;
    }

    if ((data_cfg_default->serial_invert_mode != data_cfg_new->serial_invert_mode) && (data_cfg_new->serial_invert_mode)) {
        data_cfg_default->serial_invert_mode = data_cfg_new->serial_invert_mode;
    }

    if ((data_cfg_default->serial_parity_mode != data_cfg_new->serial_parity_mode) && (data_cfg_new->serial_parity_mode)) {
        data_cfg_default->serial_parity_mode = data_cfg_new->serial_parity_mode;
    }

    if ((data_cfg_default->serial_stop_bits != data_cfg_new->serial_stop_bits) && (data_cfg_new->serial_stop_bits)) {
        data_cfg_default->serial_stop_bits = data_cfg_new->serial_stop_bits;
    }
}

static void usart_handler(int vector, void *param)
{
    struct SerialBus *serial_bus = (struct SerialBus *)param;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)serial_bus->bus.owner_haldev;
    
    SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
}

static uint32 SerialInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;

    if (configure_info->private_data) {
        struct SerialCfgParam *serial_cfg_new = (struct SerialCfgParam *)configure_info->private_data;
        SerialCfgParamCheck(serial_cfg, serial_cfg_new);
    }
 
    GPIO_REG(GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_EN) |= IOF0_UART0_MASK;

    UART0_REG(UART_REG_DIV) = get_cpu_freq() / serial_cfg->data_cfg.serial_baud_rate - 1;
    UART0_REG(UART_REG_TXCTRL) |= UART_TXEN;
    UART0_REG(UART_REG_RXCTRL) |= UART_RXEN;
    UART0_REG(UART_REG_IE) = UART_IP_RXWM;

    return EOK;
}

static uint32 SerialConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    return EOK;
}

static uint32 SerialDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    int serial_operation_cmd;
    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;

    switch (configure_info->configure_cmd)
    {
        case OPE_INT:
            ret = SerialInit(serial_drv, configure_info);
            break;
        case OPE_CFG:
            serial_operation_cmd = *(int *)configure_info->private_data;
            ret = SerialConfigure(serial_drv, serial_operation_cmd);
            break;
        default:
            break;
    }

    return ret;
}

static int SerialPutChar(struct SerialHardwareDevice *serial_dev, char c)
{
    while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
    UART0_REG(UART_REG_TXFIFO) = c;

    return 0;
}

static int SerialGetChar(struct SerialHardwareDevice *serial_dev)
{
    int32_t val = UART0_REG(UART_REG_RXFIFO);
    if (val > 0)
        return (uint8_t)val;
    else
        return -1;
}

static const struct SerialDataCfg data_cfg_init = 
{
    .serial_baud_rate = BAUD_RATE_115200,
    .serial_data_bits = DATA_BITS_8,
    .serial_stop_bits = STOP_BITS_1,
    .serial_parity_mode = PARITY_NONE,
    .serial_bit_order = BIT_ORDER_LSB,
    .serial_invert_mode = NRZ_NORMAL,
    .serial_buffer_size = SERIAL_RB_BUFSZ,
};

/*manage the serial device operations*/
static const struct SerialDrvDone drv_done =
{
    .init = SerialInit,
    .configure = SerialConfigure,
};

/*manage the serial device hal operations*/
static struct SerialHwDevDone hwdev_done =
{
    .put_char = SerialPutChar,
    .get_char = SerialGetChar,
};

static int BoardSerialBusInit(struct SerialBus *serial_bus, struct SerialDriver *serial_driver, const char *bus_name, const char *drv_name)
{
    x_err_t ret = EOK;

    /*Init the serial bus */
    ret = SerialBusInit(serial_bus, bus_name);
    if (EOK != ret) {
        KPrintf("InitHwUart SerialBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the serial driver*/
    ret = SerialDriverInit(serial_driver, drv_name);
    if (EOK != ret) {
        KPrintf("InitHwUart SerialDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the serial driver to the serial bus*/
    ret = SerialDriverAttachToBus(drv_name, bus_name);
    if (EOK != ret) {
        KPrintf("InitHwUart SerialDriverAttachToBus error %d\n", ret);
        return ERROR;
    } 

    return ret;
}

/*Attach the serial device to the serial bus*/
static int BoardSerialDevBend(struct SerialHardwareDevice *serial_device, void *serial_param, const char *bus_name, const char *dev_name)
{
    x_err_t ret = EOK;

    ret = SerialDeviceRegister(serial_device, serial_param, dev_name);
    if (EOK != ret) {
        KPrintf("InitHwUart SerialDeviceInit device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    ret = SerialDeviceAttachToBus(dev_name, bus_name);
    if (EOK != ret) {
        KPrintf("InitHwUart SerialDeviceAttachToBus device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    return  ret;
}

int InitHwUart(void)
{
    x_err_t ret = EOK;

    static struct SerialBus serial_bus;
    memset(&serial_bus, 0, sizeof(struct SerialBus));

    static struct SerialDriver serial_driver;
    memset(&serial_driver, 0, sizeof(struct SerialDriver));

    static struct SerialHardwareDevice serial_device;
    memset(&serial_device, 0, sizeof(struct SerialHardwareDevice));

    static struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param;
    memset(&serial_dev_param, 0, sizeof(struct SerialDevParam));
    
    serial_driver.drv_done = &drv_done;
    serial_driver.configure = &SerialDrvConfigure;
    serial_device.hwdev_done = &hwdev_done;

    serial_cfg.data_cfg = data_cfg_init;

    serial_cfg.hw_cfg.serial_register_base   = UART0_CTRL_ADDR;
    serial_cfg.hw_cfg.serial_irq_interrupt = INT_UART0_BASE;
    serial_driver.private_data = (void *)&serial_cfg;

    serial_dev_param.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device.haldev.private_data = (void *)&serial_dev_param;

    ret = BoardSerialBusInit(&serial_bus, &serial_driver, SERIAL_BUS_NAME, SERIAL_DRV_NAME);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device, (void *)&serial_cfg, SERIAL_BUS_NAME, SERIAL_DEVICE_NAME);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }    

    isrManager.done->registerIrq(serial_cfg.hw_cfg.serial_irq_interrupt, usart_handler, &serial_bus);
    isrManager.done->enableIrq(serial_cfg.hw_cfg.serial_irq_interrupt);

    return ret;
}
