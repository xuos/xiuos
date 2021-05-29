/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
* @file connect_uart.c
* @brief support imxrt1052-board uart function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-28
*/

/*************************************************
File name: connect_uart.c
Description: support imxrt1052-board uart configure and uart bus register function
Others: take SDK_2.6.1_MIMXRT1052xxxxB/components/uart/lpuart_adapter.c for references
History: 
1. Date: 2021-05-28
Author: AIIT XUOS Lab
Modification: 
1. support imxrt1052-board uart configure, write and read
2. support imxrt1052-board uart bus device and driver register
*************************************************/

#include <board.h>
#include <connect_uart.h>
#include <fsl_lpuart.h>

static void UartIsr(struct SerialBus *serial, struct SerialDriver *serial_drv, struct SerialHardwareDevice *serial_dev);

#ifdef BSP_USING_LPUART1
struct SerialBus serial_bus_1;
struct SerialDriver serial_driver_1;
struct SerialHardwareDevice serial_device_1;

void LPUART1_IRQHandler(int irqn, void *arg)
{
    x_base lock = 0;
    // KPrintf("LPUART1_IRQHandler \n");
    lock = DISABLE_INTERRUPT();

    UartIsr(&serial_bus_1, &serial_driver_1, &serial_device_1);

    ENABLE_INTERRUPT(lock);
}
DECLARE_HW_IRQ(UART1_IRQn, LPUART1_IRQHandler, NONE);
#endif

#ifdef BSP_USING_LPUART2
struct SerialBus serial_bus_2;
struct SerialDriver serial_driver_2;
struct SerialHardwareDevice serial_device_2;

void LPUART2_IRQHandler(int irqn, void *arg)
{
    x_base lock = 0;
    lock = DISABLE_INTERRUPT();

    UartIsr(&serial_bus_2, &serial_driver_2, &serial_device_2);

    ENABLE_INTERRUPT(lock);
}
DECLARE_HW_IRQ(UART2_IRQn, LPUART2_IRQHandler, NONE);
#endif

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

static void UartIsr(struct SerialBus *serial, struct SerialDriver *serial_drv, struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    LPUART_Type *uart_base = (LPUART_Type *)serial_cfg->hw_cfg.private_data;
    
    /* kLPUART_RxDataRegFullFlag can only cleared or set by hardware */
    if (LPUART_GetStatusFlags(uart_base) & kLPUART_RxDataRegFullFlag) {
        SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
    }

    if (LPUART_GetStatusFlags(uart_base) & kLPUART_RxOverrunFlag) {
        /* Clear overrun flag, otherwise the RX does not work. */
        LPUART_ClearStatusFlags(uart_base, kLPUART_RxOverrunFlag);
    }
}

static uint32 GetUartSrcFreq(void)
{
    uint32 freq;

    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */ {
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    } else {
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }

    return freq;
}

static uint32 SerialInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    LPUART_Type *uart_base = (LPUART_Type *)serial_cfg->hw_cfg.private_data;

    if (configure_info->private_data) {
        struct SerialCfgParam *serial_cfg_new = (struct SerialCfgParam *)configure_info->private_data;
        SerialCfgParamCheck(serial_cfg, serial_cfg_new);
    }

    lpuart_config_t config;
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = serial_cfg->data_cfg.serial_baud_rate;

    switch (serial_cfg->data_cfg.serial_data_bits)
    {
        case DATA_BITS_7:
            config.dataBitsCount = kLPUART_SevenDataBits;
            break;

        default:
            config.dataBitsCount = kLPUART_EightDataBits;
            break;
    }

    switch (serial_cfg->data_cfg.serial_stop_bits)
    {
        case STOP_BITS_2:
            config.stopBitCount = kLPUART_TwoStopBit;
            break;
        default:
            config.stopBitCount = kLPUART_OneStopBit;
            break;
    }

    switch (serial_cfg->data_cfg.serial_parity_mode)
    {
        case PARITY_ODD:
            config.parityMode = kLPUART_ParityOdd;
            break;
        case PARITY_EVEN:
            config.parityMode = kLPUART_ParityEven;
            break;
        default:
            config.parityMode = kLPUART_ParityDisabled;
            break;
    }

    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(uart_base, &config, GetUartSrcFreq());

    return EOK;
}

static uint32 SerialConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data;
    LPUART_Type *uart_base = (LPUART_Type *)serial_cfg->hw_cfg.private_data;

    switch (serial_operation_cmd)
    {
        case OPER_CLR_INT:
            DisableIRQ(serial_cfg->hw_cfg.serial_irq_interrupt);
            break;

        case OPER_SET_INT:
            LPUART_EnableInterrupts(uart_base, kLPUART_RxDataRegFullInterruptEnable);
            NVIC_SetPriority(serial_cfg->hw_cfg.serial_irq_interrupt, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
            EnableIRQ(serial_cfg->hw_cfg.serial_irq_interrupt);
            break;
    }

    return EOK;
}

static int SerialPutChar(struct SerialHardwareDevice *serial_dev, char c)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    LPUART_Type *uart_base = (LPUART_Type *)serial_cfg->hw_cfg.private_data;

    LPUART_WriteByte(uart_base, c);
    while (!(LPUART_GetStatusFlags(uart_base) & kLPUART_TxDataRegEmptyFlag));

    return 1;
}

static int SerialGetChar(struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    LPUART_Type *uart_base = (LPUART_Type *)serial_cfg->hw_cfg.private_data;

    int c = -1;
    if (LPUART_GetStatusFlags(uart_base) & kLPUART_RxDataRegFullFlag) {
        c = LPUART_ReadByte(uart_base);
    }

    return c;
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
        KPrintf("Imrt1052HwUartInit SerialBusInit error %d\n", ret);
        return ERROR;
    }

    /*Init the serial driver*/
    ret = SerialDriverInit(serial_driver, drv_name);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit SerialDriverInit error %d\n", ret);
        return ERROR;
    }

    /*Attach the serial driver to the serial bus*/
    ret = SerialDriverAttachToBus(drv_name, bus_name);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit SerialDriverAttachToBus error %d\n", ret);
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
        KPrintf("Imrt1052HwUartInit SerialDeviceInit device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    ret = SerialDeviceAttachToBus(dev_name, bus_name);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit SerialDeviceAttachToBus device %s error %d\n", dev_name, ret);
        return ERROR;
    }  

    return  ret;
}

int Imrt1052HwUartInit(void)
{
    x_err_t ret = EOK;

#ifdef BSP_USING_LPUART1
    static struct SerialCfgParam serial_cfg_1;
    memset(&serial_cfg_1, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param_1;
    memset(&serial_dev_param_1, 0, sizeof(struct SerialDevParam));
    
    serial_driver_1.drv_done = &drv_done;
    serial_driver_1.configure = &SerialDrvConfigure;
    serial_device_1.hwdev_done = &hwdev_done;

    serial_cfg_1.data_cfg = data_cfg_init;

    serial_cfg_1.hw_cfg.private_data   = (void *)LPUART1;
    serial_cfg_1.hw_cfg.serial_irq_interrupt = LPUART1_IRQn;
    serial_driver_1.private_data = (void *)&serial_cfg_1;

    serial_dev_param_1.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_1.haldev.private_data = (void *)&serial_dev_param_1;

    ret = BoardSerialBusInit(&serial_bus_1, &serial_driver_1, SERIAL_BUS_NAME_1, SERIAL_DRV_NAME_1);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit uart error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_1, (void *)&serial_cfg_1, SERIAL_BUS_NAME_1, SERIAL_1_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit uart error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_LPUART2
    static struct SerialCfgParam serial_cfg_2;
    memset(&serial_cfg_2, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param_2;
    memset(&serial_dev_param_2, 0, sizeof(struct SerialDevParam));
    
    serial_driver_2.drv_done = &drv_done;
    serial_driver_2.configure = &SerialDrvConfigure;
    serial_device_2.hwdev_done = &hwdev_done;

    serial_cfg_2.data_cfg = data_cfg_init;

    serial_cfg_2.hw_cfg.private_data = (void *)LPUART2;
    serial_cfg_2.hw_cfg.serial_irq_interrupt = LPUART2_IRQn;
    serial_driver_2.private_data = (void *)&serial_cfg_2;

    serial_dev_param_2.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_2.haldev.private_data = (void *)&serial_dev_param_2;

    ret = BoardSerialBusInit(&serial_bus_2, &serial_driver_2, SERIAL_BUS_NAME_2, SERIAL_DRV_NAME_2);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit uart error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_2, (void *)&serial_cfg_2, SERIAL_BUS_NAME_2, SERIAL_2_DEVICE_NAME_0);
    if (EOK != ret) {
        KPrintf("Imrt1052HwUartInit uart error ret %u\n", ret);
        return ERROR;
    }  
#endif

    return ret;
}
