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
* @file bus.c
* @brief 1、support bus driver framework；2、provide bus API。
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus.h>
#include <stdlib.h>

DoubleLinklistType bus_linklist;

/*Create the bus linklist*/
static void BusLinkInit(struct Bus *bus)
{
    static uint8 bus_link_flag = RET_FALSE;

    if(!bus_link_flag) {
        InitDoubleLinkList(&bus_linklist);
        bus_link_flag = RET_TRUE;
        bus->bus_link_flag = RET_TRUE;
    }

    /*Create the driver of the bus linklist*/
    if(!bus->bus_drvlink_flag) {
        InitDoubleLinkList(&bus->bus_drvlink);
        bus->bus_drvlink_flag = RET_TRUE;
    }

    /*Create the hardware device of the bus linklist*/
    if(!bus->bus_devlink_flag) {
        InitDoubleLinkList(&bus->bus_devlink);
        bus->bus_devlink_flag = RET_TRUE;
    }
}

static int BusMatchDrvDev(struct Driver  *driver, struct HardwareDev *device)
{
    NULL_PARAM_CHECK(driver);
    NULL_PARAM_CHECK(device);

    if(!strncmp(driver->owner_bus->bus_name, device->owner_bus->bus_name, NAME_NUM_MAX)) {
        KPrintf("BusMatchDrvDev match successfully, bus name %s\n", driver->owner_bus->bus_name);

        driver->private_data = device->private_data;//driver get the device param
        device->owner_bus->owner_driver = driver;
        driver->owner_bus->owner_haldev = device;

        return EOK;
    }

    return ERROR;
}

/**
* @Description: support to obtain bus for a certain dev if necessary, then configure and init its drv
* @param bus - bus pointer
* @param dev - dev pointer
* @param drv_name - drv name
* @param configure_info - BusConfigureInfo pointer
* @return successful：EOK，failed：ERROR
*/
int DeviceObtainBus(struct Bus *bus, struct HardwareDev *dev, const char *drv_name, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(bus);
    NULL_PARAM_CHECK(dev);

    int32 ret = EOK;

    ret = KMutexObtain(bus->bus_lock, WAITING_FOREVER);
    if(EOK != ret) {
        KPrintf("DevObtainBus bus_lock error %d!\n", ret);
        return ret;
    }

    if(bus->owner_haldev != dev) {
        struct Driver *drv = BusFindDriver(bus, drv_name);
        
        configure_info->configure_cmd = OPE_CFG;
        drv->configure(drv, configure_info);

        configure_info->configure_cmd = OPE_INT;
        drv->configure(drv, configure_info);

        bus->owner_haldev = dev;
    }

    return ret;
}

/**
* @Description: support to register bus pointer with linklist
* @param bus - bus pointer
* @return successful：EOK，failed：NONE
*/
int BusRegister(struct Bus *bus)
{
    x_err_t ret = EOK;
    NULL_PARAM_CHECK(bus);

    bus->match = BusMatchDrvDev;

    BusLinkInit(bus);

    bus->bus_lock = KMutexCreate();

    DoubleLinkListInsertNodeAfter(&bus_linklist, &(bus->bus_link));

    return ret;
}

/**
* @Description: support to release bus pointer in linklist
* @param bus - bus pointer
* @return successful：EOK，failed：NONE
*/
int BusRelease(struct Bus *bus)
{
    NULL_PARAM_CHECK(bus);

    KMutexAbandon(bus->bus_lock);

    bus->bus_cnt = 0;
    bus->driver_cnt = 0;
    bus->haldev_cnt = 0;

    bus->bus_link_flag = RET_FALSE;
    bus->bus_drvlink_flag = RET_FALSE;
    bus->bus_devlink_flag = RET_FALSE;

    return EOK;
}

/**
* @Description: support to unregister bus pointer and delete its linklist node
* @param bus - bus pointer
* @return successful：EOK，failed：NONE
*/
int BusUnregister(struct Bus *bus)
{
    NULL_PARAM_CHECK(bus);

    bus->bus_cnt--;

    DoubleLinkListRmNode(&(bus->bus_link));

    return EOK;
}

/**
* @Description: support to register driver pointer to bus pointer
* @param bus - bus pointer
* @param driver - driver pointer
* @return successful：EOK，failed：NONE
*/
int DriverRegisterToBus(struct Bus *bus, struct Driver *driver)
{
    NULL_PARAM_CHECK(bus);
    NULL_PARAM_CHECK(driver);

    driver->owner_bus = bus;
    bus->driver_cnt++;

    DoubleLinkListInsertNodeAfter(&bus->bus_drvlink, &(driver->driver_link));

    return EOK;
}

/**
* @Description: support to register dev pointer to bus pointer
* @param bus - bus pointer
* @param device - device pointer
* @return successful：EOK，failed：NONE
*/
int DeviceRegisterToBus(struct Bus *bus, struct HardwareDev *device)
{
    NULL_PARAM_CHECK(bus);
    NULL_PARAM_CHECK(device); 

    device->owner_bus = bus;
    bus->haldev_cnt++;

    DoubleLinkListInsertNodeAfter(&bus->bus_devlink, &(device->dev_link));

    return EOK;
}

/**
* @Description: support to delete driver pointer from bus pointer
* @param bus - bus pointer
* @param driver - driver pointer
* @return successful：EOK，failed：NONE
*/
int DriverDeleteFromBus(struct Bus *bus, struct Driver *driver)
{
    NULL_PARAM_CHECK(bus);
    NULL_PARAM_CHECK(driver);

    bus->driver_cnt--;

    DoubleLinkListRmNode(&(driver->driver_link));

    free(driver);

    return EOK;
}

/**
* @Description: support to delete dev pointer from bus pointer
* @param bus - bus pointer
* @param device - device pointer
* @return successful：EOK，failed：NONE
*/
int DeviceDeleteFromBus(struct Bus *bus, struct HardwareDev *device)
{
    NULL_PARAM_CHECK(bus);
    NULL_PARAM_CHECK(device); 

    bus->haldev_cnt--;

    DoubleLinkListRmNode(&(device->dev_link));

    free(device);

    return EOK;
}

/**
* @Description: support to find bus pointer by bus name
* @param bus_name - bus name
* @return successful：bus pointer，failed：NONE
*/
BusType BusFind(const char *bus_name)
{
    struct Bus *bus = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &bus_linklist;

    for (node = head->node_next; node != head; node = node->node_next)
    {
        bus = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Bus, bus_link);
        if(!strcmp(bus->bus_name, bus_name)) {
            return bus;
        }
    }

    KPrintf("BusFind cannot find the %s bus.return NULL\n", bus_name);
    return NONE;
}

/**
* @Description: support to find driver pointer of certain bus by driver name
* @param bus - bus pointer
* @param driver_name - driver name
* @return successful：EOK，failed：NONE
*/
DriverType BusFindDriver(struct Bus *bus, const char *driver_name)
{
    NULL_PARAM_CHECK(bus);
    struct Driver *driver = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &bus->bus_drvlink;

    for (node = head->node_next; node != head; node = node->node_next)
    {
        driver = SYS_DOUBLE_LINKLIST_ENTRY(node, struct Driver, driver_link);
        if(!strcmp(driver->drv_name, driver_name)) {
            return driver;
        }
    }

    KPrintf("BusFindDriver cannot find the %s driver.return NULL\n", driver_name);
    return NONE;
}

/**
* @Description: support to find device pointer of certain bus by device name
* @param bus - bus pointer
* @param device_name - device name
* @return successful：EOK，failed：NONE
*/
HardwareDevType BusFindDevice(struct Bus *bus, const char *device_name)
{
    NULL_PARAM_CHECK(bus);
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &bus->bus_devlink;

    for (node = head->node_next; node != head; node = node->node_next)
    {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);

        if(!strcmp(device->dev_name, device_name)) {
            return device;
        }
    }

    KPrintf("BusFindDevice cannot find the %s device.return NULL\n", device_name);
    return NONE;
}

/**
* @Description: support to set dev receive function callback
* @param dev - dev pointer
* @param dev_recv_callback - callback function
* @return successful：EOK，failed：ERROR
*/
uint32 BusDevRecvCallback(struct HardwareDev *dev, int (*dev_recv_callback) (void *dev, x_size_t length))
{
    NULL_PARAM_CHECK(dev );

    dev->dev_recv_callback = dev_recv_callback;

    return EOK;
}

/**
* @Description: support to open dev
* @param dev - dev pointer
* @return successful：EOK，failed：ERROR
*/
uint32 BusDevOpen(struct HardwareDev *dev)
{
    NULL_PARAM_CHECK(dev);

    x_err_t ret = EOK;

    if (dev->dev_done->open) {
        ret = dev->dev_done->open(dev);
        if(ret) {
            KPrintf("BusDevOpen error ret %u\n", ret);
            return ERROR;
        }
    }

    return ret;
}

/**
* @Description: support to close dev
* @param dev - dev pointer
* @return successful：EOK，failed：ERROR
*/
uint32 BusDevClose(struct HardwareDev *dev)
{
    NULL_PARAM_CHECK(dev);

    x_err_t ret = EOK;

    if (dev->dev_done->close) {
        ret = dev->dev_done->close(dev);
        if(ret) {
            KPrintf("BusDevClose error ret %u\n", ret);
            return ERROR;
        }
    }

    return ret;
}

/**
* @Description: support to write data to dev
* @param dev - dev pointer
* @param write_param - BusBlockWriteParam
* @return successful：EOK，failed：NONE
*/
uint32 BusDevWriteData(struct HardwareDev *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    
    if (dev->dev_done->write) {
        return dev->dev_done->write(dev, write_param);
    }

    return EOK;
}

/**
* @Description: support to read data from dev
* @param dev - dev pointer
* @param read_param - BusBlockReadParam
* @return successful：EOK，failed：NONE
*/
uint32 BusDevReadData(struct HardwareDev *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    
    if (dev->dev_done->read) {
        return dev->dev_done->read(dev, read_param);
    }

    return EOK;
}

/**
* @Description: support to configure drv, include OPE_CFG and OPE_INT
* @param drv - drv pointer
* @param configure_info - BusConfigureInfo
* @return successful：EOK，failed：NONE
*/
uint32 BusDrvConfigure(struct Driver *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;

    if (drv->configure) {
        ret = drv->configure(drv, configure_info);
        if(ret) {
            KPrintf("BusDrvCfg error, ret %u\n", ret);
            return ERROR;
        }
    }

    return ret;
} 
