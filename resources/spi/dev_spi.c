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
* @file dev_spi.c
* @brief register spi dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <bus_spi.h>
#include <dev_spi.h>

static DoubleLinklistType spidev_linklist;

/*Create the spi device linklist*/
static void SpiDeviceLinkInit()
{
    InitDoubleLinkList(&spidev_linklist);
}

static uint32 SpiDeviceOpen(void *dev)
{
    NULL_PARAM_CHECK(dev);

    SpiDevConfigureCs(dev, 1, 0);

    return EOK;
} 

static uint32 SpiDeviceClose(void *dev)
{
    NULL_PARAM_CHECK(dev);

    SpiDevConfigureCs(dev, 0, 1);

    return EOK;
} 

static uint32 SpiDeviceWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);

    struct SpiHardwareDevice *spi_dev = (struct SpiHardwareDevice *)dev;
    struct SpiDataStandard spi_msg;

    spi_msg.tx_buff = (uint8 *)write_param->buffer;
    spi_msg.rx_buff = NONE;
    spi_msg.length = write_param->size;
    spi_msg.spi_chip_select = 0;
    spi_msg.spi_cs_release = 0;
    spi_msg.next = NONE;

    return spi_dev->spi_dev_done->write(spi_dev, &spi_msg);
}

static uint32 SpiDeviceRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);

    struct SpiHardwareDevice *spi_dev = (struct SpiHardwareDevice *)dev;
    struct SpiDataStandard spi_msg;

    spi_msg.tx_buff = NONE;
    spi_msg.rx_buff = (uint8 *)read_param->buffer;
    spi_msg.length = read_param->size;
    spi_msg.spi_chip_select = 0;
    spi_msg.spi_cs_release = 0;
    spi_msg.next = NONE;

    return spi_dev->spi_dev_done->read(spi_dev, &spi_msg);
}

static const struct HalDevDone dev_done =
{
    .open = SpiDeviceOpen,
    .close = SpiDeviceClose,
    .write = SpiDeviceWrite,
    .read = SpiDeviceRead,
};

HardwareDevType SpiDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &spidev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("SpiDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int SpiDeviceRegister(struct SpiHardwareDevice *spi_device, void *spi_param, const char *device_name)
{
    NULL_PARAM_CHECK(spi_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (dev_link_flag) {
        SpiDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != spi_device->haldev.dev_state) {
        strncpy(spi_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        spi_device->haldev.dev_type = TYPE_SPI_DEV;
        spi_device->haldev.dev_state = DEV_INSTALL;

        //only spi bus dev need to register dev_done
        if (RET_TRUE != spi_device->spi_dev_flag) {
            spi_device->haldev.dev_done = &dev_done;
        }

        spi_device->haldev.private_data = spi_param;

        DoubleLinkListInsertNodeAfter(&spidev_linklist, &(spi_device->haldev.dev_link));
    } else {
        KPrintf("SpiDeviceRegister device has been register state%u\n", spi_device->haldev.dev_state);        
    }

    return ret;
}

int SpiDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SpiDeviceAttachToBus find spi bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_SPI_BUS == bus->bus_type) {
        device = SpiDeviceFind(dev_name, TYPE_SPI_DEV);
        if (NONE == device) {
            KPrintf("SpiDeviceAttachToBus find spi device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_SPI_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("SpiDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}

int SpiDevConfigureCs(struct HardwareDev *dev, uint8 spi_chip_select, uint8 spi_cs_release)
{
    NULL_PARAM_CHECK(dev);

    struct SpiHardwareDevice *spi_dev = (struct SpiHardwareDevice *)dev;
    struct SpiDataStandard msg;

    memset(&msg, 0, sizeof(struct SpiDataStandard));
    msg.spi_chip_select = spi_chip_select;
    msg.spi_cs_release = spi_cs_release;

    return spi_dev->spi_dev_done->write(spi_dev, &msg);
}
