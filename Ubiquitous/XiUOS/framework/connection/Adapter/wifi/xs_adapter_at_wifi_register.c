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
 * @file xs_adapter_at_wifi_register.c
 * @brief HFA21 wifi driver register to connection framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <stdlib.h>
#include <user_api.h>
#include <xs_adapter_at_wifi.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_agent.h>

const struct AdapterDone wifi_adapter_done =
{
    WifiClose,
    WifiOpen,
    NULL,
    WifiSend,
    WifiReceive,
    NULL,
};

const struct ATDone wifi_at_done =
{
    WifiSetUp,
    WifiSetDown,
    WifiSetAddr,
    NULL,
    WifiDHCP,
    WifiPing,
    WifiNetstat,
    NULL,
};

/**
 * @description: Register wifi device
 * @return success: EOK, failure: ERROR
 */
int RegisterAdapterWifi(void)
{
    struct Adapterwifi *wifi_adapter = malloc(sizeof(struct Adapterwifi));
    if (wifi_adapter == NULL) {
        printf("out of memory\n");
        return ERROR;
    }

    struct AdapterAT *wifi_at_adapter = (struct AdapterAT *)wifi_adapter;
    struct Adapter *adapter = (struct Adapter *)wifi_adapter;

    wifi_adapter->parent.atdone = wifi_at_done;
    wifi_adapter->parent.parent.done = wifi_adapter_done;

    wifi_at_adapter->at_adapter_id = WIFI_ADAPTER_ID;

    ATAdapterInit();
    ATAdapterRegister(wifi_at_adapter);

    return EOK;
}