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
* @file bus.h
* @brief define bus driver framework function and common API
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef BUS_H
#define BUS_H

#include <xiuos.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPE_INT                  0x0000
#define OPE_CFG                 0x0001

#define OPER_WDT_SET_TIMEOUT    0x0002
#define OPER_WDT_KEEPALIVE           0x0003

typedef struct Bus *BusType;
typedef struct HardwareDev *HardwareDevType;
typedef struct Driver *DriverType;

/* need to add new bus type in ../tool/shell/letter-shell/cmd.c, ensure ShowBus cmd supported*/
enum BusType
{
    TYPE_I2C_BUS = 0,
    TYPE_SPI_BUS,
    TYPE_HWTIMER_BUS,
    TYPE_USB_BUS,
    TYPE_CAN_BUS,
    TYPE_WDT_BUS,
    TYPE_SDIO_BUS,
    TYPE_TOUCH_BUS,
    TYPE_LCD_BUS,
    TYPE_PIN_BUS,
    TYPE_RTC_BUS,
    TYPE_SERIAL_BUS,
    TYPE_BUS_END,
};

enum BusState
{
    BUS_INIT = 0,
    BUS_INSTALL,
    BUS_UNINSTALL,
};

enum DevType
{
    TYPE_I2C_DEV = 0,
    TYPE_SPI_DEV,
    TYPE_HWTIMER_DEV,
    TYPE_USB_DEV,
    TYPE_CAN_DEV,
    TYPE_WDT_DEV,
    TYPE_SDIO_DEV,
    TYPE_TOUCH_DEV,
    TYPE_LCD_DEV,
    TYPE_PIN_DEV,
    TYPE_RTC_DEV,
    TYPE_SERIAL_DEV,
    TYPE_DEV_END,
};

enum DevState
{
    DEV_INIT = 0,
    DEV_INSTALL,
    DEV_UNINSTALL,
};

enum DriverType
{
    TYPE_I2C_DRV = 0,
    TYPE_SPI_DRV,
    TYPE_HWTIMER_DRV,
    TYPE_USB_DRV,
    TYPE_CAN_DRV,
    TYPE_WDT_DRV,
    TYPE_SDIO_DRV,
    TYPE_TOUCH_DRV,
    TYPE_LCD_DRV,
    TYPE_PIN_DRV,
    TYPE_RTC_DRV,
    TYPE_SERIAL_DRV,
    TYPE_DRV_END,
};

enum DriverState
{
    DRV_INIT = 0,
    DRV_INSTALL,
    DRV_UNINSTALL,
};

struct BusConfigureInfo
{
    int configure_cmd;
    void *private_data;
};

struct BusBlockReadParam
{
    x_OffPos pos;
    void* buffer;
    x_size_t size;
    x_size_t read_length;
};

struct BusBlockWriteParam
{
    x_OffPos pos;
    const void* buffer;
    x_size_t size;
};

struct HalDevBlockParam
{
    uint32 cmd;
    struct DeviceBlockArrange dev_block;
    struct DeviceBlockAddr *dev_addr;
};

struct HalDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);
};

struct HardwareDev
{
    int8 dev_name[NAME_NUM_MAX];
    enum DevType dev_type;
    enum DevState dev_state;
    
    const struct HalDevDone *dev_done;

    int (*dev_recv_callback) (void *dev, x_size_t length);
    int (*dev_block_control) (struct HardwareDev *dev, struct HalDevBlockParam *block_param);

    struct Bus *owner_bus;
    void *private_data;

    int32 dev_sem;

    DoubleLinklistType  dev_link;    
};

struct Driver
{
    int8 drv_name[NAME_NUM_MAX];
    enum DriverType driver_type;
    enum DriverState driver_state;

    uint32 (*configure)(void *drv, struct BusConfigureInfo *configure_info);

    struct Bus *owner_bus;
    void *private_data;

    DoubleLinklistType  driver_link;    
};

struct Bus
{
    int8 bus_name[NAME_NUM_MAX];
    enum BusType bus_type;
    enum BusState bus_state;

    int32 (*match)(struct Driver *driver, struct HardwareDev *device);

    int bus_lock;

    struct HardwareDev *owner_haldev;
    struct Driver *owner_driver;
    
    void *private_data;

    /*manage the drv of the bus*/
    uint8 driver_cnt;
    uint8 bus_drvlink_flag;
    DoubleLinklistType bus_drvlink;

    /*manage the dev of the bus*/
    uint8 haldev_cnt;
    uint8 bus_devlink_flag;
    DoubleLinklistType bus_devlink;

    uint8 bus_cnt;
    uint8 bus_link_flag;
    DoubleLinklistType  bus_link;    
};

/*Register the BUS,manage with the double linklist*/
int BusRegister(struct Bus *bus);

/*Release the BUS framework*/
int BusRelease(struct Bus *bus);

/*Unregister a certain kind of BUS*/
int BusUnregister(struct Bus *bus);

/*Register the driver to the bus*/
int DriverRegisterToBus(struct Bus *bus, struct Driver *driver);

/*Register the device to the bus*/
int DeviceRegisterToBus(struct Bus *bus, struct HardwareDev *device);

/*Delete the driver from the bus*/
int DriverDeleteFromBus(struct Bus *bus, struct Driver *driver);

/*Delete the device from the bus*/
int DeviceDeleteFromBus(struct Bus *bus, struct HardwareDev *device);

/*Find the bus with bus name*/
BusType BusFind(const char *bus_name);

/*Find the driver of cetain bus*/
DriverType BusFindDriver(struct Bus *bus, const char *driver_name);

/*Find the device of certain bus*/
HardwareDevType BusFindDevice(struct Bus *bus, const char *device_name);

/*Dev receive data callback function*/
uint32 BusDevRecvCallback(struct HardwareDev *dev, int (*dev_recv_callback) (void *dev, x_size_t length));

/*Open the device of the bus*/
uint32 BusDevOpen(struct HardwareDev *dev);

/*Close the device of the bus*/
uint32 BusDevClose(struct HardwareDev *dev);

/*Write data to the device*/
uint32 BusDevWriteData(struct HardwareDev *dev, struct BusBlockWriteParam *write_param);

/*Read data from the device*/
uint32 BusDevReadData(struct HardwareDev *dev, struct BusBlockReadParam *read_param);

/*Configure the driver of the bus*/
uint32 BusDrvConfigure(struct Driver *drv, struct BusConfigureInfo *configure_info);

/*Obtain the bus using a certain dev*/
int DeviceObtainBus(struct Bus *bus, struct HardwareDev *dev, const char *drv_name, struct BusConfigureInfo *configure_info);

#ifdef __cplusplus
}
#endif

#endif
