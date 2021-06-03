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
* @file connect_uart.c
* @brief support cortex_m3_emulator board uart function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-10
*/

#include <board.h>
#include <xiuos.h>
#include <device.h>
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_uart.h>

#ifdef BSP_USING_UART1
static struct SerialBus serial_bus_1;
static struct SerialDriver serial_driver_1;
static struct SerialHardwareDevice serial_device_1;
#endif
#ifdef BSP_USING_UART2
static struct SerialBus serial_bus_2;
static struct SerialDriver serial_driver_2;
static struct SerialHardwareDevice serial_device_2;
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

static void UartHandler(struct SerialBus *serial_bus, struct SerialDriver *serial_drv)
{
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)serial_bus->bus.owner_haldev;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data; 

    uint32 status;

    status = UARTIntStatus(serial_cfg->hw_cfg.serial_register_base, RET_TRUE);

    /* clear interrupt status */
    UARTIntClear(serial_cfg->hw_cfg.serial_register_base, status);

    while (UARTCharsAvail(serial_cfg->hw_cfg.serial_register_base)) {
        SerialSetIsr(serial_dev, SERIAL_EVENT_RX_IND);
    }
}

#ifdef BSP_USING_UART1
void UartIsr1(int vector, void *param)
{
	/* get serial bus 1 */
	UartHandler(&serial_bus_1, &serial_driver_1);
}
DECLARE_HW_IRQ(UART1_IRQn, UartIsr1, NONE);
#endif

#ifdef BSP_USING_UART2
void UartIsr2(int irqno)
{
	/* get serial bus 2 */
	UartHandler(&serial_bus_2, &serial_driver_2);
}
#endif

static uint32 SerialInit(struct SerialDriver *serial_drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(serial_drv);

    return EOK;
}

static uint32 SerialConfigure(struct SerialDriver *serial_drv, int serial_operation_cmd)
{
    NULL_PARAM_CHECK(serial_drv);

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_drv->private_data; 
    struct HardwareDev *dev = serial_drv->driver.owner_bus->owner_haldev;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;

    if (OPER_CLR_INT == serial_operation_cmd) {
        if (SIGN_OPER_INT_RX & serial_dev_param->serial_work_mode) {
            /* disable UART rx interrupt */
            UARTIntDisable(serial_cfg->hw_cfg.serial_register_base, UART_INT_RX | UART_INT_RT);
        }
	} else if (OPER_SET_INT == serial_operation_cmd) {
        /* enable interrupt */
        if (UART0_BASE == serial_cfg->hw_cfg.serial_register_base)
            IntEnable(INT_UART0);
        else if (UART1_BASE == serial_cfg->hw_cfg.serial_register_base)
            IntEnable(INT_UART1);

        UARTIntEnable(serial_cfg->hw_cfg.serial_register_base, UART_INT_RX | UART_INT_RT);
    }

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
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    while (UARTCharPutNonBlocking(serial_cfg->hw_cfg.serial_register_base, c) == RET_FALSE);

    return 0;
}

static int SerialGetChar(struct SerialHardwareDevice *serial_dev)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data; 
    long val = UARTCharGetNonBlocking(serial_cfg->hw_cfg.serial_register_base);
    if (val > 0)
        return (int)val;
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

#ifdef BSP_USING_UART1
    memset(&serial_bus_1, 0, sizeof(struct SerialBus));
    memset(&serial_driver_1, 0, sizeof(struct SerialDriver));
    memset(&serial_device_1, 0, sizeof(struct SerialHardwareDevice));

    static struct SerialCfgParam serial_cfg_1;
    memset(&serial_cfg_1, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param_1;
    memset(&serial_dev_param_1, 0, sizeof(struct SerialDevParam));
    
    serial_driver_1.drv_done = &drv_done;
    serial_driver_1.configure = &SerialDrvConfigure;
    serial_device_1.hwdev_done = &hwdev_done;

    serial_cfg_1.data_cfg = data_cfg_init;

    serial_cfg_1.hw_cfg.serial_register_base   = UART0_BASE;
    serial_driver_1.private_data = (void *)&serial_cfg_1;

    serial_dev_param_1.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_1.haldev.private_data = (void *)&serial_dev_param_1;

	/* enable UART0 clock */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	/* set UART0 pinmux */
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	/* Configure the UART for 115,200, 8-N-1 operation. */
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), serial_cfg_1.data_cfg.serial_baud_rate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    ret = BoardSerialBusInit(&serial_bus_1, &serial_driver_1, SERIAL_BUS_NAME_1, SERIAL_DRV_NAME_1);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_1, (void *)&serial_cfg_1, SERIAL_BUS_NAME_1, SERIAL_DEVICE_NAME_1);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }    
#endif

#ifdef BSP_USING_UART2
    memset(&serial_bus_2, 0, sizeof(struct SerialBus));
    memset(&serial_driver_2, 0, sizeof(struct SerialDriver));
    memset(&serial_device_2, 0, sizeof(struct SerialHardwareDevice));

    static struct SerialCfgParam serial_cfg_2;
    memset(&serial_cfg_2, 0, sizeof(struct SerialCfgParam));

    static struct SerialDevParam serial_dev_param_2;
    memset(&serial_dev_param_2, 0, sizeof(struct SerialDevParam));
    
    serial_driver_2.drv_done = &drv_done;
    serial_driver_2.configure = &SerialDrvConfigure;
    serial_device_2.hwdev_done = &hwdev_done;

    serial_cfg_2.data_cfg = data_cfg_init;

    serial_cfg_2.hw_cfg.serial_register_base   = UART1_BASE;
    serial_driver_2.private_data = (void *)&serial_cfg_2;

    serial_dev_param_2.serial_work_mode = SIGN_OPER_INT_RX;
    serial_device_2.haldev.private_data = (void *)&serial_dev_param_2;

    ret = BoardSerialBusInit(&serial_bus_2, &serial_driver_2, SERIAL_BUS_NAME_2, SERIAL_DRV_NAME_2);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }

    ret = BoardSerialDevBend(&serial_device_2, (void *)&serial_cfg_2, SERIAL_BUS_NAME_2, SERIAL_DEVICE_NAME_2);
    if (EOK != ret) {
        KPrintf("InitHwUart uarths error ret %u\n", ret);
        return ERROR;
    }    
#endif

    return ret;
}
