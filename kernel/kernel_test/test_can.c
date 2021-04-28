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
* @file test_can.c
* @brief support to test can function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include "bus_can.h"
#include "dev_can.h"


uint8  buf[8]  = {0x33,0x55,0x77,0x99,0x66,0x44,0x88,0x11};
uint8  rx_buf[8];
struct BusBlockWriteParam   cfx  = {
     .pos  = 0,
      .buffer  = buf,
       .size  = 8
};


uint8  rx_buf[8]  = {0};
struct BusBlockReadParam read_param  = {
    .pos = 0,
    .buffer =rx_buf ,
    .read_length =0 
};


  static struct  CanDriverConfigure  can_defconfig  = {
        .tsjw  =  0,
        .tbs2 =  5 ,
        .tbs1 =  6,
         .brp  =  6,
        .mode  = 0
};

struct BusConfigureInfo configure_info ={
    .configure_cmd = 0,
    .private_data = &can_defconfig
};



  static int _CanTestSend(const char *bus_name, const char *driver_name, const char *device_name0)
{
    int i;
    struct Bus *bus;
    struct Driver *driver, *bus_driver;
    struct HardwareDev *device0, *device1;
    struct HardwareDev *bus_device0;

    if(bus_name)
    {
        bus = BusFind(bus_name);
        KPrintf("##test bus %p##\n", bus);
    }

    if(driver_name)
    {
        driver = CanDriverFind(driver_name, TYPE_CAN_DRV);
        bus_driver = BusFindDriver(bus, driver_name);
        KPrintf("##test driver %p bus_driver %p##\n", driver, bus_driver);
    }

    if(device_name0)
    {
        device0 = CanDeviceFind(device_name0, TYPE_CAN_DEV);
        bus_device0 = BusFindDevice(bus, device_name0);
        KPrintf("####test device0 %p bus_device0 %p####\n", device0, bus_device0);
        KPrintf("####test device0 private %p\n", device0->private_data);
    }

     BusDrvConfigure(bus_driver, &configure_info);

    while(1)
    {
         BusDevWriteData(device0, &cfx);
         uint8  * buf  =(uint8 *) cfx.buffer;
        KPrintf("send:");
          for(i = 0;i<cfx.size;i++)
                   KPrintf("0x%2x    ", buf[i]);
        KPrintf("\n");
         MdelayKTask(2000);

    }

}
void  CanTestSend(void)
{
       _CanTestSend(CAN_BUS_NAME_1,CAN_DRIVER_NAME,CAN_1_DEVICE_NAME_1);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),CanTestSend, CanTestSend,  Close AC task );


   

static int   _CanTestRecv(const char *bus_name, const char *driver_name, const char *device_name0)
{
    int len ,i;
    struct Bus *bus;
    struct Driver *driver, *bus_driver;
    struct HardwareDev *device0, *device1;
    struct HardwareDev *bus_device0;

    if(bus_name)
    {
        KPrintf("####test find bus %s\n", bus_name);
        bus = BusFind(bus_name);
        KPrintf("####test bus %p####\n", bus);
    }

    if(driver_name)
    {
        KPrintf("####test find driver %s\n", driver_name);
        driver = CanDriverFind(driver_name, TYPE_CAN_DRV);
        bus_driver = BusFindDriver(bus, driver_name);
        KPrintf("####test driver %p bus_driver %p####\n", driver, bus_driver);
    }

    if(device_name0)
    {
        KPrintf("####test find device0 %s\n", device_name0);
        device0 = CanDeviceFind(device_name0, TYPE_CAN_DEV);
        bus_device0 = BusFindDevice(bus, device_name0);
        KPrintf("####test device0 %p bus_device0 %p####\n", device0, bus_device0);
        KPrintf("####test device0 private %p\n", device0->private_data);
    }

      BusDrvConfigure(bus_driver, &configure_info);
        
    while(1)
    {
        len =  BusDevReadData(bus_device0,&read_param);
             if(0 != len) 
             {
                     for(i = 0; i<len;i++)
                           KPrintf("0x%02x   ", rx_buf[i]);
                           KPrintf("\n");
             }
            else
            {
                      MdelayKTask(100);
            }
    

    }

}
void  CanTestRecv(void)
{
    _CanTestRecv(CAN_BUS_NAME_1,CAN_DRIVER_NAME,CAN_1_DEVICE_NAME_1);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),CanTestRecv, CanTestRecv,  Close AC task );

