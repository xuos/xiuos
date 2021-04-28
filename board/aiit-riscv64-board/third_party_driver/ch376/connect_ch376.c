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
* @file connect_ch436.c
* @brief support to register ch436 pointer and function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <ch376fs.h>
#include <connect_ch376.h>

static struct HwCh376 sdio;

static uint32 Ch376Configure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    uint8 temp[2];
    temp[0] = 0x57;
    temp[1] = 0xab;
    MdelayKTask(100);

    struct BusBlockWriteParam write_param;
    write_param.pos = 0;
    write_param.buffer = (void *)temp;
    write_param.size = 2;
    BusDevWriteData(sdio.dev, &write_param);

    write_param.pos = 0;
    write_param.buffer = &configure_info->configure_cmd;
    write_param.size = 1;
    BusDevWriteData(sdio.dev, &write_param);

    return EOK;
}

static int HwCh376RxInd(void *dev, x_size_t length)
{
    sdio.MsgLen += length;
    KSemaphoreAbandon(sdio.sem);

    return EOK;
}

static uint32 Ch376Open(void *dev)
{
    NULL_PARAM_CHECK(dev);

    sdio.sem = KSemaphoreCreate(0);
    if (sdio.sem < 0) {
        KPrintf("CH376 open fail\n");
        return -ERROR;
    }

    struct Bus *bus = BusFind(SERIAL_BUS_NAME_2);
    struct Driver *bus_driver = BusFindDriver(bus, SERIAL_DRV_NAME_2);
    sdio.dev = BusFindDevice(bus, SERIAL_2_DEVICE_NAME_0);
    if (!sdio.dev) {
        KPrintf("CH376 open fail\n");
        return -ERROR;
    }

    struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
    serial_cfg.data_cfg.serial_buffer_size = 512;
    struct BusConfigureInfo cfg;
    cfg.configure_cmd = OPE_INT;
    cfg.private_data = &serial_cfg;

    BusDrvConfigure(bus_driver, &cfg);

    bus->match(bus_driver, sdio.dev);

    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)sdio.dev->private_data;
    serial_dev_param->serial_set_mode = SIGN_OPER_INT_RX;
    BusDevOpen(sdio.dev);

    BusDevRecvCallback(sdio.dev, HwCh376RxInd);

    KPrintf("CH376 open done\n");

    return EOK;
}

static uint32 Ch376Close(void *dev)
{
    BusDevClose(sdio.dev);
    KSemaphoreDelete(sdio.sem);

    return EOK;
}

static uint32 Ch376Write(void *dev, struct BusBlockWriteParam *write_param)
{
    if (EOK == BusDevWriteData(sdio.dev, write_param))
        return EOK;

    return -ERROR;
}

static uint32 Ch376Read(void *dev, struct BusBlockReadParam *read_param)
{
    if (KSemaphoreObtain(sdio.sem, WAITING_FOREVER) == EOK) {
        while(KSemaphoreObtain(sdio.sem, TICK_PER_SECOND) != -ETIMEOUT);
        read_param->size = sdio.MsgLen;
        BusDevReadData(sdio.dev, read_param);
        sdio.MsgLen = 0;
    }
    
    return read_param->read_length;
}

#ifdef BSP_USING_SDIO
static struct SdioDevDone sdio_done = 
{
    Ch376Open,
    Ch376Close,
    Ch376Write,
    Ch376Read,
};
#endif

#ifdef BSP_USING_USB
static struct UsbDevDone usb_done = 
{
    Ch376Open,
    Ch376Close,
    Ch376Write,
    Ch376Read,
};
#endif

int HwCh376Init(void)
{
    x_err_t ret = EOK;

#ifdef BSP_USING_SDIO
    static struct SdioBus sdio_bus;
    static struct SdioDriver sdio_drv;
    static struct SdioHardwareDevice sdio_dev;
    
    ret = SdioBusInit(&sdio_bus, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio bus init error %d\n", ret);
        return -ERROR;
    }

    sdio_drv.configure = &Ch376Configure;
    ret = SdioDriverInit(&sdio_drv, SDIO_DRIVER_NAME);
    if (ret != EOK) {
        KPrintf("Sdio driver init error %d\n", ret);
        return -ERROR;
    }

    ret = SdioDriverAttachToBus(SDIO_DRIVER_NAME, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio driver attach error %d\n", ret);
        return -ERROR;
    }

    sdio_dev.dev_done = &sdio_done;
    ret = SdioDeviceRegister(&sdio_dev, SDIO_DEVICE_NAME);
    if (ret != EOK) {
        KPrintf("Sdio device register error %d\n", ret);
        return -ERROR;
    }

    ret = SdioDeviceAttachToBus(SDIO_DEVICE_NAME, SDIO_BUS_NAME);
    if (ret != EOK) {
        KPrintf("Sdio device register error %d\n", ret);
        return -ERROR;
    }
#endif

#ifdef BSP_USING_USB
    static struct UsbBus usb_bus;
    static struct UsbDriver usb_drv;
    static struct UsbHardwareDevice usb_dev;
    
    ret = UsbBusInit(&usb_bus, USB_BUS_NAME);
    if (ret != EOK) {
        KPrintf("USB bus init error %d\n", ret);
        return -ERROR;
    }

    usb_drv.configure = &Ch376Configure;
    ret = UsbDriverInit(&usb_drv, USB_DRIVER_NAME);
    if (ret != EOK) {
        KPrintf("USB driver init error %d\n", ret);
        return -ERROR;
    }
    ret = UsbDriverAttachToBus(USB_DRIVER_NAME, USB_BUS_NAME);
    if (ret != EOK) {
        KPrintf("USB driver attach error %d\n", ret);
        return -ERROR;
    }

    usb_dev.dev_done = &usb_done;
    ret = USBDeviceRegister(&usb_dev, NONE, USB_DEVICE_NAME);
    if (ret != EOK) {
        KPrintf("USB device register error %d\n", ret);
        return -ERROR;
    }
    ret = USBDeviceAttachToBus(USB_DEVICE_NAME, USB_BUS_NAME);
    if (ret != EOK) {
        KPrintf("USB device register error %d\n", ret);
        return -ERROR;
    }
#endif
    return ret;
}