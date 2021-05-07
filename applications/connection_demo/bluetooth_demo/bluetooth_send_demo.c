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
* @file:    bluetooth_send_demo.c
* @brief:   using bluetooth to send message
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/30
*
*/
#include <xs_adapter_bluetooth.h>   
#include <string.h>
#include <xs_klist.h>
#include <xs_adapter_manager.h>
#include <string.h>
#include <user_api.h>
adapter_t padapter;
/* a demo function to send message through command line using bluetooth*/
/* first open bluetooth to start demo*/
void BluetoothOpenDemo()
{
    /*Find from the list of registered adapters*/
    // adapter_t padapter = BluetoothAdapterFind("Bluetoot");
    padapter = BluetoothAdapterFind("Bluetooth");
    if (NONE == padapter){
        printf("adapter find failed!\n");
        return;
    }

    /*Open adapter*/
    if (0 != padapter->done.NetAiitOpen(padapter)){
        printf("adapter open failed!\n");
        return;
    }

}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
BluetoothOpenDemo, BluetoothOpenDemo,  bluetooth send function );
#endif

void BluetoothSendDemo(int argc, char *argv[])
{
    /*Find from the list of registered adapters*/
    bool v = false;
    padapter->done.NetAiitSend(padapter, argv[1], strlen(argv[1]) ,true,10000,0, NULL,&v,NULL);
        

}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
BluetoothSendDemo, BluetoothSendDemo,  bluetooth send function );
#endif


