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
* @file bus_serial.h
* @brief define serial bus and drv function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date
*/

#ifndef BUS_SERIAL_H
#define BUS_SERIAL_H

#include <xs_klist.h>
#ifdef __cplusplus
extern "C" {
#endif

enum ExtSerialPortConfigure
{
    PORT_CFG_INIT = 0,
    PORT_CFG_PARITY_CHECK,
    PORT_CFG_DISABLE,
    PORT_CFG_DIV,
};

struct SerialDataCfg
{
    uint32 serial_baud_rate;
    uint8 serial_data_bits;
    uint8 serial_stop_bits;
    uint8 serial_parity_mode;
    uint8 serial_bit_order;
    uint8 serial_invert_mode;
    uint16 serial_buffer_size;

    uint8 ext_uart_no;
    enum ExtSerialPortConfigure port_configure;
};

struct SerialHwCfg
{
    uint32 serial_register_base;
    uint32 serial_irq_interrupt;
    void *private_data;
};

struct SerialCfgParam
{
    struct SerialDataCfg data_cfg;
    struct SerialHwCfg hw_cfg;
};
#ifdef __cplusplus
}
#endif

#endif
