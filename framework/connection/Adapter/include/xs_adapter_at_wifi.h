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
 * @file xs_adapter_at_wifi.h
 * @brief Structure and function declarations of the connection wifi
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_AT_WIFI_H
#define XS_ADAPTER_AT_WIFI_H

#include "xs_adapter.h"
#include "xs_adapter_at.h"
#include "xs_adapter_def.h"
#include "xs_klist.h"

struct Adapterwifi {
    struct AdapterAT parent; /* inherit from Adapter */

    char vendor_name[NAME_LEN_MAX];        
    char product_id_wifi[NAME_LEN_MAX];      

    struct SingleLinklistNode link;
};

void WifiClose(struct Adapter *padapter);
int WifiOpen(struct Adapter *padapter);
int WifiSend(struct Adapter *padapter, const char *data, int len, bool block, int time_out, int delay, send_success cb, void *param, void *p);
int WifiReceive(struct Adapter *padapter, char *rev_buffer, int buffer_len, int time_out, bool block, void *p);

int WifiSetUp(struct AdapterAT *adapter_at);
int WifiSetDown(struct AdapterAT *adapter_at);
int WifiSetAddr(struct AdapterAT *adapter_at, uint ip, uint gateway, uint netmask);
int WifiDHCP(struct AdapterAT *adapter_at, bool enable);
int WifiPing(struct AdapterAT *adapter_at, const char *destination,struct ping_result *ping_resp);
int WifiNetstat(struct AdapterAT *adapter_at);

#endif