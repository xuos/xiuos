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
* @file riscv_test_can.c
* @brief support to test can function on aiit-riscv64-board
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>

#ifdef BOARD_AIIT_RISCV_EVB

#include <device.h>
#include <gpiohs.h>
#include "board.h"
#include "connect_ch438.h"

static struct Bus *bus;
static struct HardwareDev *dev;
static struct Driver *drv;

static int32 Timer = NONE;

static uint8 idx = 0;

static void Ch438Read(void *parameter)
{
    uint8 rev_len;
	uint8 ext_uart_no = 3;
	static uint8 dat,i,count;

    struct BusBlockReadParam read_param;
    static uint8 Ch438Buff[8][BUFFSIZE];
	
    while (1)
    {
        read_param.buffer = Ch438Buff[3];
        rev_len = BusDevReadData(dev, &read_param);

        for(count = 0 ;count < rev_len ;count++)    
        {  
			KPrintf("%#2x  ",Ch438Buff[3][count]);
            KPrintf("\n");
        }
							
		for(i=0;i<BUFFSIZE;i++)
        {
			Ch438Buff[3][i] = 0;
        }
	}
}

static void TestCh438Init(void)
{
    x_err_t flag;

    struct BusConfigureInfo configure_info;
	
	int32 task_CH438 = KTaskCreate("task_CH438", Ch438Read, NONE, 2048, 10); 
	flag = StartupKTask(task_CH438);
    if (flag != EOK) {
		KPrintf("StartupKTask failed .\n");
		return ;
	} 

    bus = BusFind(CH438_BUS_NAME);
    drv = BusFindDriver(bus, CH438_DRIVER_NAME);
    dev = BusFindDevice(bus, CH438_DEVICE_NAME_3);

    struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
    configure_info.configure_cmd = OPE_INT;
    configure_info.private_data = (void *)&serial_cfg;

    serial_cfg.data_cfg.port_configure = PORT_CFG_INIT;

    serial_cfg.data_cfg.ext_uart_no = 3;
    serial_cfg.data_cfg.serial_baud_rate = 115200;
    BusDrvConfigure(drv, &configure_info);
}

void CanTestSend(void)
{
    uint8 MeterInstruction[8]={0x01, 0x04, 0x77, 0x66, 0x20, 0x03, 0xB0, 0x0B};

    struct BusBlockWriteParam write_param;

    write_param.buffer = MeterInstruction;
    write_param.size = 8;

	TestCh438Init();
	MdelayKTask(500);
    while(1)
    {
        BusDevWriteData(dev, &write_param);
        KPrintf("test_can_send!\n");
        MdelayKTask(500);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                CanTestSend, CanTestSend, CanTestSend );

void CanTestRecv(void)
{
	TestCh438Init();
	MdelayKTask(500);
    while(1)
    {
        ;
    }

}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                CanTestRecv, CanTestRecv, CanTestRecv );
#endif
