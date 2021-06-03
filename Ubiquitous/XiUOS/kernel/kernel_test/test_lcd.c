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
* @file EnableLcd.c
* @brief support to test lcd function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include "bus_lcd.h"
#include "dev_lcd.h"

uint16 color  = RED;

char *addr  ="LCD Testing ";

/**
 * @description: Enable LCD function
 * @param bus_name - LCD bus name
 * @param driver_name - LCD driver name
 * @param device_name - LCD device name
 */
int EnableLcd(const char *bus_name, const char *driver_name, const char *device_name)
{
    struct Bus *bus;
    struct Driver *driver, *bus_driver;
    struct HardwareDev *device;
    struct HardwareDev *bus_device;


    if (bus_name) {
        KPrintf("##test find bus %s\n", bus_name);
        bus = BusFind(bus_name);
        KPrintf("##test bus %p##\n", bus);
    }

    if (driver_name) {
        KPrintf("##test find driver %s\n", driver_name);
        driver = LcdDriverFind(driver_name, TYPE_LCD_DRV);
        bus_driver = BusFindDriver(bus, driver_name);
        KPrintf("##test driver %p bus_driver %p####\n", driver, bus_driver);
    }

    if (device_name) {
        KPrintf("##test find device %s\n", device_name);
        device =  LcdDeviceFind(device_name, TYPE_LCD_DEV);
        bus_device = BusFindDevice(bus, device_name);
        KPrintf("##test device %p bus_device %p##\n", device, bus_device);
    }

    LcdStringParam   str;
    str.x_pos  = 60  ;
    str.y_pos  = 40;
    str.width  =  250;
    str.height = 24;
    str.font_size = 24;
    str.addr  =  addr;
    str.font_color = WHITE;
    str.back_color  =  RED;

    struct BusBlockWriteParam write_param;
    memset(&write_param,0,sizeof(struct BusBlockWriteParam ));

    struct  BusConfigureInfo  pdata = {RED,NONE};

    write_param.pos  = 0;
    write_param.buffer = &str ;

    while (1) {
     
        BusDrvConfigure(bus_driver, &pdata );
        MdelayKTask(1000);

        BusDevWriteData(device,&write_param);
        MdelayKTask(1000);

    }
}

/**
 * @description: LCD test function
 */
void  TestLcd(void)
{
    EnableLcd(LCD_BUS_NAME_1,LCD_DRV_NAME_1,LCD_1_DEVICE_NAME_0);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),TestLcd, TestLcd, Test LCD  );
