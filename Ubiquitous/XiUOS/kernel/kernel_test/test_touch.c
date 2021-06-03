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
* @file test_touch.c
* @brief support to test touch function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include "bus_touch.h"
#include "dev_touch.h"

int TestTouch(const char *bus_name, const char *driver_name, const char *device_name0)
{
    struct Bus *bus;
    struct Driver *driver, *bus_driver;
    struct HardwareDev *device0;
    struct HardwareDev *bus_device0;
    struct TouchDataStandard   datacfg;

    struct TouchDataStandard  data ={0,0};

    struct BusConfigureInfo configure_info = {0,&data};

    struct BusBlockReadParam read_param;
    memset(&read_param,0,sizeof(struct BusBlockReadParam ));

    read_param.buffer = &data;

    if(bus_name)
    {
        KPrintf("##test find bus %s\n", bus_name);
        bus = BusFind(bus_name);
        KPrintf("##test bus %p####\n", bus);
    }

    if(driver_name)
    {
        KPrintf("##test find driver %s\n", driver_name);
        driver = TouchDriverFind(driver_name, TYPE_TOUCH_DRV);
        bus_driver = BusFindDriver(bus, driver_name);
        KPrintf("##test driver %p bus_driver %p##\n", driver, bus_driver);
    }

    if(device_name0)
    {
        KPrintf("##test find device0 %s\n", device_name0);
        device0 = TouchDeviceFind(device_name0, TYPE_TOUCH_DEV);
        KPrintf("device0 :%p\n", device0);
        bus_device0 = BusFindDevice(bus, device_name0);
        KPrintf("bus_device0 :%p\n", bus_device0);
    }
       KPrintf("ttt\n");
     BusDrvConfigure(bus_driver, &configure_info);
       KPrintf("yyy\n");
    while(1)
    {
        BusDevReadData(bus_device0,&read_param);
		KPrintf("dev.x= %8d    ***  dev.y= %8d \r\n",data.x,data.y);
		MdelayKTask(100);
    }

}

void TouchTest(void)
{
     TestTouch(TOUCH_BUS_NAME_1,TOUCH_DRV_NAME_1, TOUCH_1_DEVICE_NAME_0);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),TouchTest, TouchTest,  Close AC task );