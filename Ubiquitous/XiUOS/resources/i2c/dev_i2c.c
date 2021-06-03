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
* @file dev_i2c.c
* @brief register i2c dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_i2c.h>
#include <dev_i2c.h>

static DoubleLinklistType i2cdev_linklist;

/*Create the I2C device linklist*/
static void I2cDeviceLinkInit()
{
    InitDoubleLinkList(&i2cdev_linklist);
}

static uint32 I2cDeviceWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);

    struct I2cHardwareDevice *i2c_dev = (struct I2cHardwareDevice *)dev;
    struct I2cDataStandard i2c_msg;

    i2c_msg.addr = I2C_SLAVE_ADDR;
    i2c_msg.flags = I2C_WR;
    i2c_msg.buf    = NONE;
    i2c_msg.len    = 0;
    i2c_msg.retries = 1;
    i2c_msg.next = NONE;

    return i2c_dev->i2c_dev_done->dev_write(i2c_dev, &i2c_msg);
}

static uint32 I2cDeviceRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);

    struct I2cHardwareDevice *i2c_dev = (struct I2cHardwareDevice *)dev;
    struct I2cDataStandard i2c_msg;

    i2c_msg.addr = I2C_SLAVE_ADDR;
    i2c_msg.flags = I2C_RD;
    i2c_msg.buf = read_param->buffer;
    i2c_msg.len = read_param->size;
    i2c_msg.retries = 1;
    i2c_msg.next = NONE;

    return i2c_dev->i2c_dev_done->dev_read(i2c_dev, &i2c_msg);
}

static const struct HalDevDone dev_done =
{
    .open = NONE,
    .close = NONE,
    .write = I2cDeviceWrite,
    .read = I2cDeviceRead,
};

/*Find the register I2C device*/
HardwareDevType I2cDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &i2cdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("I2cDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

/*Register the I2C device*/
int I2cDeviceRegister(struct I2cHardwareDevice *i2c_device, void *i2c_param, const char *device_name)
{
    NULL_PARAM_CHECK(i2c_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        I2cDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != i2c_device->haldev.dev_state) {
        strncpy(i2c_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        i2c_device->haldev.dev_type = TYPE_I2C_DEV;
        i2c_device->haldev.dev_state = DEV_INSTALL;

        i2c_device->haldev.dev_done = &dev_done;

        i2c_device->haldev.private_data = i2c_param;

        DoubleLinkListInsertNodeAfter(&i2cdev_linklist, &(i2c_device->haldev.dev_link));
    } else {
        KPrintf("I2cDeviceRegister device has been register state%u\n", i2c_device->haldev.dev_state);        
    }

    return ret;
}

/*Register the I2C Device to the I2C BUS*/
int I2cDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("I2cDeviceAttachToBus find i2c bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_I2C_BUS == bus->bus_type) {
        device = I2cDeviceFind(dev_name, TYPE_I2C_DEV);
        if (NONE == device) {
            KPrintf("I2cDeviceAttachToBus find i2c device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_I2C_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("I2cDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
