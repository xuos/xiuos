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
* @file device.h
* @brief support to include RESOURCES of all drivers
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEVICE_H
#define DEVICE_H

#include <xiuos.h>

#include <xs_avltree.h>
#include <xs_circular_area.h>
#include <xs_dataqueue.h>
#include <xs_workqueue.h>
#include <xs_waitqueue.h>

#ifdef RESOURCES_RTC
#include <bus_rtc.h>
#include <dev_rtc.h>
#endif

#ifdef RESOURCES_SPI
#include <bus_spi.h>
#include <dev_spi.h>
#endif

#ifdef RESOURCES_TOUCH
#include <bus_touch.h>
#include <dev_touch.h>
#endif

#ifdef RESOURCES_LCD
#include <bus_lcd.h>
#include <dev_lcd.h>
#endif

#ifdef RESOURCES_USB
#include <bus_usb.h>
#include <dev_usb.h>
#ifdef RESOURCES_USB_HOST
#include <usb_host.h>
#endif
#endif

#ifdef RESOURCES_SERIAL
#include <bus_serial.h>
#include <dev_serial.h>
HardwareDevType InstallConsole(const char *bus_name, const char *drv_name, const char *dev_name);
HardwareDevType ObtainConsole(void);
#endif

#ifdef RESOURCES_I2C
#include <bus_i2c.h>
#include <dev_i2c.h>
#endif 

#ifdef RESOURCES_WDT
#include <bus_wdt.h>
#include <dev_wdt.h>
#endif

#ifdef RESOURCES_SDIO
#include <bus_sdio.h>
#include <dev_sdio.h>
#endif

#ifdef RESOURCES_PIN
#include <bus_pin.h>
#include <dev_pin.h>
#endif

#ifdef RESOURCES_CAN
#include <bus_can.h>
#include <dev_can.h>
#endif

#ifdef RESOURCES_HWTIMER
#include <bus_hwtimer.h>
#include <dev_hwtimer.h>
#endif

#endif
