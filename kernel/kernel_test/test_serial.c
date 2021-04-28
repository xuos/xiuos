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
* @file test_serial.c
* @brief support to test serial function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <device.h>
#if defined (ARCH_RISCV)   
#include <connect_uart.h>
#elif defined(ARCH_ARM)
#include <connect_usart.h>
#endif

//#define TEST_POSIX

#ifdef TEST_POSIX
#include "user_api.h"
#endif

#ifdef RESOURCES_SERIAL

static char test_str[] = "Hello AIIT!\r\n";

struct Bus *bus = NONE;
struct Driver *bus_driver = NONE;
struct HardwareDev *bus_device = NONE;

#ifdef TEST_POSIX
static int uart3_fd = 0;
#endif
static int32 test_serial_task = 0;

static void TestSerialRecvTask(void *parameter)
{
    int16 i = 0;
    char recv_data = 0;
#ifdef TEST_POSIX
    char data_buffer[128] = {0};
    char data_size = 0;
#endif
    struct BusBlockReadParam read_param;
    struct BusBlockWriteParam write_param;
    memset(&read_param, 0, sizeof(struct BusBlockReadParam));
    memset(&write_param, 0, sizeof(struct BusBlockWriteParam));



    while(RET_TRUE) {
#ifndef TEST_POSIX
        read_param.size = 1;
        read_param.buffer = &recv_data;
        read_param.read_length = 0;
        
        BusDevReadData(bus_device, &read_param);
        for (i = 0; i < read_param.read_length; i ++) {
            KPrintf("TestSerialRecvTask i %d char 0x%x\n", i, recv_data);
        }

        write_param.buffer = &recv_data;
        write_param.size = 1;
        BusDevWriteData(bus_device, &write_param);
#else
        memset(data_buffer, 0, 128);
		data_size = read(uart3_fd, data_buffer, 128);
		KPrintf("uart 3 data size %d data %s\n", data_size, data_buffer);
#endif
    }
}

static int SerialBusCheck(const char *bus_name, const char *driver_name, const char *device_name)
{
    int ret = EOK;
#ifndef TEST_POSIX
    struct SerialBus *serial_bus = NONE;
    struct SerialDriver *serial_driver = NONE;
    struct SerialHardwareDevice *serial_device = NONE;
    struct SerialCfgParam *serial_cfg_default = NONE;
    struct SerialDevParam *serial_dev_param = NONE;

    struct BusConfigureInfo configure_info;

    if(bus_name)
    {
        KPrintf("####test find bus %s\n", bus_name);
        bus = BusFind(bus_name);
        serial_bus = (struct SerialBus *)bus;
        KPrintf("####test find bus %8p serial_bus %8p\n", bus, serial_bus);
    }

    if(driver_name)
    {
        KPrintf("####test find driver %s\n", driver_name);
        bus_driver = BusFindDriver(bus, driver_name);
        serial_driver = (struct SerialDriver *)bus_driver;
        serial_cfg_default = (struct SerialCfgParam *)serial_driver->private_data;
        KPrintf("####test bus_driver %8p serial_driver %8p done %8p serial_cfg_default %8p####\n", 
            bus_driver, serial_driver, serial_driver->drv_done, serial_cfg_default);
        KPrintf("####hw cfg base 0x%x irq %d####\n", serial_cfg_default->hw_cfg.serial_register_base, serial_cfg_default->hw_cfg.serial_irq_interrupt);
        KPrintf("####data cfg rate %u order %u size %u bits %u invert %u parity %u stop %u####\n", serial_cfg_default->data_cfg.serial_baud_rate, serial_cfg_default->data_cfg.serial_bit_order, serial_cfg_default->data_cfg.serial_buffer_size,
            serial_cfg_default->data_cfg.serial_data_bits, serial_cfg_default->data_cfg.serial_invert_mode, serial_cfg_default->data_cfg.serial_parity_mode, serial_cfg_default->data_cfg.serial_stop_bits);
    }

    if(device_name)
    {
        KPrintf("####test find device0 %s\n", device_name);
        bus_device = BusFindDevice(bus, device_name);
        serial_device = (struct SerialHardwareDevice *)bus_device;
        serial_dev_param = (struct SerialDevParam *)bus_device->private_data;
        KPrintf("####test bus_device %8p serial_dev %8p hwdone %8p devdone %8p####\n", 
            bus_device, serial_device, serial_device->hwdev_done, serial_device->haldev.dev_done);
        KPrintf("####dev_param %8p work mode 0x%x set mode 0x%x stream mode 0x%x\n", 
            serial_dev_param, serial_dev_param->serial_work_mode, serial_dev_param->serial_set_mode, serial_dev_param->serial_stream_mode);
    }

    //BusDevRecvCallback(bus_device, test_serial_callback);

    /*step 1: init bus_driver, change struct SerialCfgParam if necessary*/
    struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
    configure_info.configure_cmd = OPE_INT;
    configure_info.private_data = &serial_cfg;
    BusDrvConfigure(bus_driver, &configure_info);
    KPrintf("BusDrvConfigure OPE_INT done\n");

    /*step 2: match bus_driver with bus_device*/
    bus->match(bus_driver, bus_device);

    /*step 3: open bus_device, configure struct SerialDevParam if necessary*/
    serial_dev_param->serial_set_mode = SIGN_OPER_INT_RX;
    serial_dev_param->serial_stream_mode = SIGN_OPER_STREAM;
    BusDevOpen(bus_device);
    KPrintf("BusDevOpen done\n");

    /*step 4: write serial data, configure struct BusBlockWriteParam*/
    struct BusBlockWriteParam write_param;
    write_param.pos = 0;
    write_param.buffer = (void *)test_str;
    write_param.size = sizeof(test_str) - 1;
    BusDevWriteData(bus_device, &write_param);

    //BusDevClose(bus_device);
    //KPrintf("BusDevClose done\n");
#else
    uart3_fd = open("/dev/uart3_dev3", O_RDWR);
	if (uart3_fd < 0) {
		KPrintf("open fd error %d\n", uart3_fd);
	}
    KPrintf("open fd %d\n", uart3_fd);

    struct SerialDataCfg cfg;
    cfg.serial_baud_rate    = BAUD_RATE_115200;
    cfg.serial_data_bits    = DATA_BITS_8;
    cfg.serial_stop_bits    = STOP_BITS_1;
    cfg.serial_buffer_size  = 128;
    cfg.serial_parity_mode  = PARITY_NONE;
    cfg.serial_bit_order    = 0;
    cfg.serial_invert_mode  = 0;
    cfg.ext_uart_no         = 0;
    cfg.port_configure      = 0;

    if (ret != ioctl(uart3_fd, OPE_INT, &cfg)) {
        KPrintf("ioctl fd error %d\n", ret);
    }
#endif
}

int TestSerial(void)
{    
    /*use serial device 3 to test serial function*/
    SerialBusCheck(SERIAL_BUS_NAME_3, SERIAL_DRV_NAME_3, SERIAL_3_DEVICE_NAME_0);

	test_serial_task = KTaskCreate("TestSerialRecvTask",
										TestSerialRecvTask,
										NONE,
										2048,
										18);
	StartupKTask(test_serial_task);

    return EOK;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                                                TestSerial, TestSerial, TestSerial );

#endif
