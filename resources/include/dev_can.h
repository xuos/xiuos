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
* @file dev_can.h
* @brief define can dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_CAN_H
#define DEV_CAN_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CanDriverConfigure 
{
    uint8 tsjw;
    uint8 tbs2 ;
    uint8 tbs1;
    uint8 mode;
    uint16 brp;
};

struct CanSendConfigure
{
    uint32 stdid;
    uint32 exdid;
    uint8 ide;
    uint8 rtr;
    uint8 data_lenth;
    uint8 *data;
};

struct CanDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev,struct BusBlockWriteParam *write_param);
    uint32 (*read) (void *dev, struct BusBlockReadParam *read_param);

};

struct CanHardwareDevice
{
    struct HardwareDev haldev;
    const struct CanDevDone *dev_done;

    void *private_data;
};

/*Register the CAN device*/
int CanDeviceRegister(struct CanHardwareDevice *can_device, void *can_param, const char *device_name);

/*Register the CAN device to the CAN bus*/
int CanDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register CAN device*/
HardwareDevType CanDeviceFind(const char *dev_name, enum DevType dev_type);

#ifdef __cplusplus
}
#endif

#endif
