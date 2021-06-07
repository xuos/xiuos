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
* @file:    xs_AdaperBluetooth_register.c
* @brief:   register Bluetooth in initialization
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/30
*
*/
#include <xs_adapter_bluetooth.h>
#include <xs_adapter_manager.h>
#include <string.h>
/* initialize to the register list*/
int RegisterAdapterBluetooth(void)
{
    static struct AdapterBluetooth bluetooth_adapter; 
    memset(&bluetooth_adapter, 0, sizeof(bluetooth_adapter));

    static struct AdapterDone bluetooth_send_done = {
        .NetAiitOpen = BluetoothOpen,
        .NetAiitClose = BluetoothClose,
        .NetAiitSend = BluetoothSend,
        .NetAiitReceive = BluetoothReceive,
        .NetAiitJoin = NULL,
        .NetAiitIoctl = NULL,
    };
    bluetooth_adapter.parent.done = bluetooth_send_done; 
    bluetooth_adapter.name = "Bluetooth";     

    BluetoothAdapterInit();
    BluetoothAdapterRegister((adapter_t)&bluetooth_adapter);

    return EOK;
}