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
* @file TestCh438.c
* @brief support to test ch438 function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <device.h>
#include "board.h"
#include "connect_ch438.h"

#if defined BOARD_AIIT_RISCV_EVB
#define EXT_UART_NO 2
#elif defined BOARD_STM32F407_EVB
#define EXT_UART_NO 4
#endif

#define DATA_BUFF_SIZE 255

static struct Bus *bus;
static struct HardwareDev *dev;
static struct Driver *drv;


static void Ch438Read(void *parameter)
{
	uint8 RevLen;
    uint8 ext_uart_no = EXT_UART_NO;
	uint8 i;
    uint16 vol, cur, pwr;

    struct BusBlockReadParam read_param;
    static uint8 Ch438Buff[8][DATA_BUFF_SIZE];
	
    while (1)
    {
        MdelayKTask(100);
        
        read_param.buffer = Ch438Buff[ext_uart_no];
        RevLen = BusDevReadData(dev, &read_param);

		vol = ((uint16)Ch438Buff[ext_uart_no][3] << 8) | Ch438Buff[ext_uart_no][4];
		cur = ((uint16)Ch438Buff[ext_uart_no][5] << 8) | Ch438Buff[ext_uart_no][6];
		pwr = ((uint16)Ch438Buff[ext_uart_no][7] << 8) | Ch438Buff[ext_uart_no][8];
		KPrintf("Board voltage : %d.%d V    current : %d mA    power : %d.%03d W\n", vol/10, vol%10, cur, pwr/1000, pwr%1000);
							
		for(i = 0 ; i < DATA_BUFF_SIZE; i ++) {
			Ch438Buff[ext_uart_no][i] = 0;
        }
	}
}

static void Ch438Write(void *parameter)
{
    x_err_t result;
    uint8 MeterInstruction[8] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0xB0, 0x0B};
    struct BusBlockWriteParam write_param;

    write_param.buffer = MeterInstruction;
    write_param.size = 8;
	
    while (1)
    {
	    MdelayKTask(1000);
        KPrintf("\n");
        BusDevWriteData(dev, &write_param);
	}
}

static void TestCh438Init(void)
{ 
    x_err_t flag;

    struct BusConfigureInfo configure_info;

    bus = BusFind(CH438_BUS_NAME);
    drv = BusFindDriver(bus, CH438_DRIVER_NAME);

#if defined BOARD_AIIT_RISCV_EVB
    dev = BusFindDevice(bus, CH438_DEVICE_NAME_2);
#elif defined BOARD_STM32F407_EVB
    dev = BusFindDevice(bus, CH438_DEVICE_NAME_4);
#endif

    struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
    configure_info.configure_cmd = OPE_INT;
    configure_info.private_data = (void *)&serial_cfg;

    serial_cfg.data_cfg.port_configure = PORT_CFG_INIT;

    serial_cfg.data_cfg.ext_uart_no = EXT_UART_NO;
    serial_cfg.data_cfg.serial_baud_rate = 9600;
    BusDrvConfigure(drv, &configure_info);

	int32 task_CH438_read = KTaskCreate("task_CH438_read", Ch438Read, NONE, 2048, 10); 
	flag = StartupKTask(task_CH438_read);
    if (flag != EOK) {
		KPrintf("StartupKTask task_CH438_read failed .\n");
		return ;
	} 

    int32 task_CH438_write = KTaskCreate("task_CH438_write", Ch438Write, NONE, 2048, 10); 
	flag = StartupKTask(task_CH438_write);
    if (flag != EOK) {
		KPrintf("StartupKTask task_CH438_write failed .\n");
		return ;
	} 
}

void TestCh438(void)
{
	TestCh438Init();
	MdelayKTask(500);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                TestCh438, TestCh438, TestCh438 );
