/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file connect_uart.c
* @brief support kd233-board uart function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_uart.c
Description: support kd233-board uart configure and uart bus register function
Others: take RT-Thread v4.0.2/bsp/k210/driver/drv_uart.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support kd233-board uart configure, write and read
2. support kd233-board uart bus device and driver register
*************************************************/

#include <sysctl.h>
#include <stdio.h>
#include "plic.h"
#include "connect_uart.h"
#include "hardware_uart.h"
#include "hardware_uarths.h"

static volatile UarthsT *const _uarths = (volatile UarthsT *)UARTHS_BASE_ADDR;

/* START ported from kendryte standalone sdk uart.c */
#define __UART_BRATE_CONST  16

volatile UartT* const  _uart_new[3] =
{
    (volatile UartT*)UART1_BASE_ADDR,
    (volatile UartT*)UART2_BASE_ADDR,
    (volatile UartT*)UART3_BASE_ADDR
};

void _uart_init_new(UartDeviceNumberT channel)
{
    sysctl_clock_enable(SYSCTL_CLOCK_UART1 + channel);
    sysctl_reset(SYSCTL_RESET_UART1 + channel);
}

/* END ported from kendryte standalone sdk uart.c */
static inline UartDeviceNumberT GetUartChannel(uint32 addr)
{
    switch (addr)
    {
        case UART1_BASE_ADDR:
            return UART_DEVICE_1;
        case UART2_BASE_ADDR:
            return UART_DEVICE_2;
        case UART3_BASE_ADDR:
            return UART_DEVICE_3;
        default:
            return UART_DEVICE_MAX;
    }
}

extern void SerialSetIsr(struct SerialHardwareDevice *serial_dev, int event);

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

/* UARTHS ISR */
static void UarthsIrqHandler(int irqno, void *param)
{
    struct SerialBus *serial_bus = (struct SerialBus *)param;
    struct SerialDriver *serial_drv = (struct SerialDriver *)serial_bus->bus.owner_driver;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)serial_bus->bus.owner_haldev;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    CHECK(UARTHS_BASE_ADDR == serial_cfg->hw_cfg.serial_register_base);

    /* read interrupt status and clear it */
    if(_uarths->ip.rxwm)
        SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
}

/* UART ISR */
static void UartIrqHandler(int irqno, void *param)
{
    struct SerialBus *serial_bus = (struct SerialBus *)param;
    struct SerialDriver *serial_drv = (struct SerialDriver *)serial_bus->bus.owner_driver;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)serial_bus->bus.owner_haldev;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;

    UartDeviceNumberT channel = GetUartChannel(serial_cfg->hw_cfg.serial_register_base);
    CHECK(channel != UART_DEVICE_MAX);

    /* read interrupt status and clear it */
    if(_uart_new[channel]->LSR)
        SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
}

static uint32 SerialHsInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;

    if (configure_info->private_data) {
        struct SerialCfgParam *serial_cfg_new = (struct SerialCfgParam *)configure_info->private_data;
        SerialCfgParamCheck(serial_cfg, serial_cfg_new);
    }

    //uint32 freq_hs = SysctlClockGetFreq(SYSCTL_CLOCK_CPU);
    uint32 freq_hs = 26000000;
    uint16 div_hs = freq_hs / serial_cfg->data_cfg.serial_baud_rate - 1;

    if (UARTHS_BASE_ADDR == serial_cfg->hw_cfg.serial_register_base) {
        _uarths->div.div = div_hs;
        _uarths->txctrl.txen = 1;
        _uarths->rxctrl.rxen = 1;
        _uarths->txctrl.txcnt = 0;
        _uarths->rxctrl.rxcnt = 0;
        _uarths->ip.txwm = 1;
        _uarths->ip.rxwm = 1;
        _uarths->ie.txwm = 0;
        _uarths->ie.rxwm = 1;
    } else {
        KPrintf("SerialHsInit error base 0x%x\n", serial_cfg->hw_cfg.serial_register_base);
        return ERROR;
    }

    return EOK;
}

static uint32 SerialHsConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct SerialBus *serial_bus = CONTAINER_OF(serial_drv->driver.owner_bus, struct SerialBus, bus);

    switch (serial_operation_cmd)
    {
        case OPER_CLR_INT:
            isrManager.done->disableIrq(serial_cfg->hw_cfg.serial_irq_interrupt);
            break;
        case OPER_SET_INT:
            isrManager.done->registerIrq(serial_cfg->hw_cfg.serial_irq_interrupt, UarthsIrqHandler, serial_bus);
            isrManager.done->enableIrq(serial_cfg->hw_cfg.serial_irq_interrupt);
            break;
    }

    return EOK;
}

static int SerialHsPutChar(struct SerialHardwareDevice *serial_dev, char c)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    CHECK(serial_cfg->hw_cfg.serial_register_base == UARTHS_BASE_ADDR);

    while (_uarths->txdata.full);
    _uarths->txdata.data = (uint8)c;

    return EOK;
}

static int SerialHsGetChar(struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    CHECK(serial_cfg->hw_cfg.serial_register_base == UARTHS_BASE_ADDR);

    uarths_rxdata_t recv = _uarths->rxdata;
    if (recv.empty)
        return -ERROR;
    else
        return (recv.data & 0xff);

    return -ERROR;
}

static uint32 SerialInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    
    if (configure_info->private_data) {
        struct SerialCfgParam *serial_cfg_new = (struct SerialCfgParam *)configure_info->private_data;
        SerialCfgParamCheck(serial_cfg, serial_cfg_new);
    }

    UartBitwidthPointer DataWidth = (UartBitwidthPointer)serial_cfg->data_cfg.serial_data_bits;
    UartStopbitT stopbit = (UartStopbitT)(serial_cfg->data_cfg.serial_stop_bits - 1);
    UartParityT parity = (UartParityT)(serial_cfg->data_cfg.serial_parity_mode - 1);

    uint32 freq = SysctlClockGetFreq(SYSCTL_CLOCK_APB0);
    uint32 divisor = freq / (uint32)serial_cfg->data_cfg.serial_baud_rate;
    uint8 dlh = divisor >> 12;
    uint8 dll = (divisor - (dlh << 12)) / __UART_BRATE_CONST;
    uint8 dlf = divisor - (dlh << 12) - dll * __UART_BRATE_CONST;

    UartDeviceNumberT channel = GetUartChannel(serial_cfg->hw_cfg.serial_register_base);
    CHECK(channel != UART_DEVICE_MAX);

    CHECK(DataWidth >= 5 && DataWidth <= 8);
    if (DataWidth == 5) {
        CHECK(stopbit != UART_STOP_2);
    } else {
        CHECK(stopbit != UART_STOP_1_5);
    }

    uint32 stopbit_val = stopbit == UART_STOP_1 ? 0 : 1;
    uint32 ParityVal;
    switch (parity)
    {
        case UART_PARITY_NONE:
            ParityVal = 0;
            break;
        case UART_PARITY_ODD:
            ParityVal = 1;
            break;
        case UART_PARITY_EVEN:
            ParityVal = 3;
            break;
        default:
            CHECK(!"Invalid parity");
            break;
    }

    _uart_new[channel]->LCR |= 1u << 7;
    _uart_new[channel]->DLH = dlh;
    _uart_new[channel]->DLL = dll;
    _uart_new[channel]->DLF = dlf;
    _uart_new[channel]->LCR = 0;
    _uart_new[channel]->LCR = (DataWidth - 5) |
                          (stopbit_val << 2) |
                          (ParityVal << 3);
    _uart_new[channel]->LCR &= ~(1u << 7);
    _uart_new[channel]->IER |= 0x80; /* THRE */
    _uart_new[channel]->FCR = UART_RECEIVE_FIFO_1 << 6 |
                          UART_SEND_FIFO_8 << 4 |
                          0x1 << 3 |
                          0x1;

    return EOK;
}

static uint32 SerialConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    struct SerialBus *serial_bus = CONTAINER_OF(serial_drv->driver.owner_bus, struct SerialBus, bus);

    UartDeviceNumberT channel = GetUartChannel(serial_cfg->hw_cfg.serial_register_base);
    CHECK(channel != UART_DEVICE_MAX);

    switch (serial_operation_cmd)
    {
        case OPER_CLR_INT:
        /* Disable the UART Interrupt */
        isrManager.done->disableIrq(serial_cfg->hw_cfg.serial_irq_interrupt);
        _uart_new[channel]->IER &= ~0x1;
        break;
        case OPER_SET_INT:
        /* install interrupt */
        isrManager.done->registerIrq(serial_cfg->hw_cfg.serial_irq_interrupt, UartIrqHandler, serial_bus);
        isrManager.done->enableIrq(serial_cfg->hw_cfg.serial_irq_interrupt);
        _uart_new[channel]->IER |= 0x1;
        break;
    }

    return EOK;
}

static int SerialPutChar(struct SerialHardwareDevice *serial_dev, char c)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    UartDeviceNumberT channel = GetUartChannel(serial_cfg->hw_cfg.serial_register_base);
    CHECK(channel != UART_DEVICE_MAX);

    while (_uart_new[channel]->LSR & (1u << 5));
    _uart_new[channel]->THR = c;

    return EOK;
}

static int SerialGetChar(struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    UartDeviceNumberT channel = GetUartChannel(serial_cfg->hw_cfg.serial_register_base);
    CHECK(channel != UART_DEVICE_MAX);

    if (_uart_new[channel]->LSR & 1)
        return (char)(_uart_new[channel]->RBR & 0xff);
    else
        return -ERROR;
  
    return -ERROR;
}

static uint32 SerialHsDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    int serial_operation_cmd;
    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;

    switch (configure_info->configure_cmd)
    {
        case OPE_INT:
            ret = SerialHsInit(serial_drv, configure_info);
            break;
        case OPE_CFG:
            serial_operation_cmd = *((int *)configure_info->private_data);
            ret = SerialHsConfigure(serial_drv, serial_operation_cmd);
            break;
        default:
            break;
    }

    return ret;
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

/*manage the serial high speed device operations*/
static const struct SerialDrvDone drv_done_hs =
{
    .init = SerialHsInit,
    .configure = SerialHsConfigure,
};

/*manage the serial high speed device hal operations*/
static struct SerialHwDevDone hwdev_done_hs =
{
    .put_char = SerialHsPutChar,
    .get_char = SerialHsGetChar,
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
        KPrintf("hw_serial_init SerialBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the serial driver*/
    ret = SerialDriverInit(serial_driver, drv_name);
    if (EOK != ret) {
        KPrintf("hw_serial_init SerialDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the serial driver to the serial bus*/
    ret = SerialDriverAttachToBus(drv_name, bus_name);
    if (EOK != ret) {
        KPrintf("hw_serial_init SerialDriverAttachToBus error %d\n", ret);
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
        KPrintf("hw_serial_init SerialDeviceInit device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    ret = SerialDeviceAttachToBus(dev_name, bus_name);
    if (EOK != ret) {
        KPrintf("hw_serial_init SerialDeviceAttachToBus device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    return  ret;
}

int HwUartInit(void)
{
    x_err_t ret = EOK;

#ifdef BSP_USING_UART_HS
    static struct SerialBus serial_bus_hs;
    memset(&serial_bus_hs, 0, sizeof(struct SerialBus));

    static struct SerialDriver serial_driver_hs;
    memset(&serial_driver_hs, 0, sizeof(struct SerialDriver));

    static struct SerialHardwareDevice serial_device_hs;
    memset(&serial_device_hs, 0, sizeof(struct SerialHardwareDevice));

    static struct SerialCfgParam serial_cfg_hs;
    memset(&serial_cfg_hs, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param_hs;
    memset(&serial_dev_param_hs, 0, sizeof(struct SerialDevParam));
    
    serial_driver_hs.drv_done = &drv_done_hs;
    serial_driver_hs.configure = &SerialHsDrvConfigure;
    serial_device_hs.hwdev_done = &hwdev_done_hs;

    serial_cfg_hs.data_cfg = data_cfg_init;

    serial_cfg_hs.hw_cfg.serial_register_base   = UARTHS_BASE_ADDR;
    serial_cfg_hs.hw_cfg.serial_irq_interrupt = IRQN_UARTHS_INTERRUPT;
    serial_driver_hs.private_data = (void *)&serial_cfg_hs;

    serial_dev_param_hs.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_hs.haldev.private_data = (void *)&serial_dev_param_hs;

    ret = BoardSerialBusInit(&serial_bus_hs, &serial_driver_hs, SERIAL_BUS_NAME_0, SERIAL_DRV_NAME_0);
    if (EOK != ret) {
        KPrintf("hw_serial_init uarths error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_hs, (void *)&serial_cfg_hs, SERIAL_BUS_NAME_0, SERIAL_0_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("hw_serial_init uarths error ret %u\n", ret);
        return ERROR;
    }    
#endif

// #ifdef BSP_USING_UART1
//     static struct SerialBus serial_bus_1;
//     memset(&serial_bus_1, 0, sizeof(struct SerialBus));

//     static struct SerialDriver serial_driver_1;
//     memset(&serial_driver_1, 0, sizeof(struct SerialDriver));

//     static struct SerialHardwareDevice serial_device_1;
//     memset(&serial_device_1, 0, sizeof(struct SerialHardwareDevice));

//     static struct SerialCfgParam serial_cfg_1;
//     memset(&serial_cfg_1, 0, sizeof(struct SerialCfgParam));

//     static struct SerialDevParam serial_dev_param_1;
//     memset(&serial_dev_param_1, 0, sizeof(struct SerialDevParam));
    
//     serial_driver_1.drv_done = &drv_done;
//     serial_driver_1.configure = &SerialDrvConfigure;
//     serial_device_1.hwdev_done = &hwdev_done;

//     serial_cfg_1.data_cfg = data_cfg_init;

//     serial_cfg_1.hw_cfg.serial_register_base = UART1_BASE_ADDR;
//     serial_cfg_1.hw_cfg.serial_irq_interrupt = IRQN_UART1_INTERRUPT;
//     serial_driver_1.private_data = (void *)&serial_cfg_1;

//     serial_dev_param_1.serial_work_mode = SIGN_OPER_INT_RX;
//     serial_device_1.haldev.private_data = (void *)&serial_dev_param_1;

//     _uart_init_new(UART_DEVICE_1);

//     ret = BoardSerialBusInit(&serial_bus_1, &serial_driver_1, SERIAL_BUS_NAME_1, SERIAL_DRV_NAME_1);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart1 error ret %u\n", ret);
//         return ERROR;
//     }

//     ret = BoardSerialDevBend(&serial_device_1, (void *)&serial_cfg_1, SERIAL_BUS_NAME_1, SERIAL_1_DEVICE_NAME_0);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart1 error ret %u\n", ret);
//         return ERROR;
//     }    
// #endif

// #ifdef BSP_USING_UART2
//     static struct SerialBus serial_bus_2;
//     memset(&serial_bus_2, 0, sizeof(struct SerialBus));

//     static struct SerialDriver serial_driver_2;
//     memset(&serial_driver_2, 0, sizeof(struct SerialDriver));

//     static struct SerialHardwareDevice serial_device_2;
//     memset(&serial_device_2, 0, sizeof(struct SerialHardwareDevice));

//     static struct SerialCfgParam serial_cfg_2;
//     memset(&serial_cfg_2, 0, sizeof(struct SerialCfgParam));

//     static struct SerialDevParam serial_dev_param_2;
//     memset(&serial_dev_param_2, 0, sizeof(struct SerialDevParam));
    
//     serial_driver_2.drv_done = &drv_done;
//     serial_driver_2.configure = &SerialDrvConfigure;
//     serial_device_2.hwdev_done = &hwdev_done;

//     serial_cfg_2.data_cfg = data_cfg_init;

//     serial_cfg_2.hw_cfg.serial_register_base = UART2_BASE_ADDR;
//     serial_cfg_2.hw_cfg.serial_irq_interrupt = IRQN_UART2_INTERRUPT;
//     serial_driver_2.private_data = (void *)&serial_cfg_2;

//     serial_dev_param_2.serial_work_mode = SIGN_OPER_INT_RX;
//     serial_device_2.haldev.private_data = (void *)&serial_dev_param_2;

//     _uart_init_new(UART_DEVICE_2);

//     ret = BoardSerialBusInit(&serial_bus_2, &serial_driver_2, SERIAL_BUS_NAME_2, SERIAL_DRV_NAME_2);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart2 error ret %u\n", ret);
//         return ERROR;
//     }

//     ret = BoardSerialDevBend(&serial_device_2, (void *)&serial_cfg_2, SERIAL_BUS_NAME_2, SERIAL_2_DEVICE_NAME_0);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart2 error ret %u\n", ret);
//         return ERROR;
//     }    
// #endif

// #ifdef BSP_USING_UART3
//     static struct SerialBus serial_bus_3;
//     memset(&serial_bus_3, 0, sizeof(struct SerialBus));

//     static struct SerialDriver serial_driver_3;
//     memset(&serial_driver_3, 0, sizeof(struct SerialDriver));

//     static struct SerialHardwareDevice serial_device_3;
//     memset(&serial_device_3, 0, sizeof(struct SerialHardwareDevice));

//     static struct SerialCfgParam serial_cfg_3;
//     memset(&serial_cfg_3, 0, sizeof(struct SerialCfgParam));

//     static struct SerialDevParam serial_dev_param_3;
//     memset(&serial_dev_param_3, 0, sizeof(struct SerialDevParam));
    
//     serial_driver_3.drv_done = &drv_done;
//     serial_driver_3.configure = &SerialDrvConfigure;
//     serial_device_3.hwdev_done = &hwdev_done;

//     serial_cfg_3.data_cfg = data_cfg_init;

//     serial_cfg_3.hw_cfg.serial_register_base = UART3_BASE_ADDR;
//     serial_cfg_3.hw_cfg.serial_irq_interrupt = IRQN_UART3_INTERRUPT;
//     serial_driver_3.private_data = (void *)&serial_cfg_3;

//     serial_dev_param_3.serial_work_mode = SIGN_OPER_INT_RX;
//     serial_device_3.haldev.private_data = (void *)&serial_dev_param_3;

//     _uart_init_new(UART_DEVICE_3);

//     ret = BoardSerialBusInit(&serial_bus_3, &serial_driver_3, SERIAL_BUS_NAME_3, SERIAL_DRV_NAME_3);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart3 error ret %u\n", ret);
//         return ERROR;
//     }

//     ret = BoardSerialDevBend(&serial_device_3, (void *)&serial_cfg_3, SERIAL_BUS_NAME_3, SERIAL_3_DEVICE_NAME_0);
//     if (EOK != ret) {
//         KPrintf("hw_serial_init uart3 error ret %u\n", ret);
//         return ERROR;
//     }    
// #endif

    return ret;
}
