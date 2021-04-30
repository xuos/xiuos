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
* @file:    xs_AdaperZigbee_register.c
* @brief:   register zigbee in initialization
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/30
*
*/
#include <xs_adapter_zigbee.h>
#include <xs_adapter_manager.h>
#include <string.h>
/* initialize to the register list*/
int RegisterAdapterZigbee(void)
{
    static struct AdapterZigbee zigbee_adapter; 
    memset(&zigbee_adapter, 0, sizeof(zigbee_adapter));

    static struct AdapterDone zigbee_send_done = {
        .NetAiitOpen = ZigbeeOpen,
        .NetAiitClose = ZigbeeClose,
        .NetAiitSend = ZigbeeSend,
        .NetAiitReceive = ZigbeeReceive,
        .NetAiitJoin = NULL,
        .NetAiitIoctl = NULL,
    };
    zigbee_adapter.parent.done = zigbee_send_done; 
    zigbee_adapter.name = "zigbee";     

    ZigbeeAdapterInit();
    ZigbeeAdapterRegister((adapter_t)&zigbee_adapter);

    return EOK;
}