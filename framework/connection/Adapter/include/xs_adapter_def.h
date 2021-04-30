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
 * @file xs_adapter_def.h
 * @brief defines about adapter
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef __XS_ADAPTER_DEF_H__
#define __XS_ADAPTER_DEF_H__

#include "stdbool.h"

typedef void(*send_success)(void* param);

#define NAME_LEN_MAX 32

#define IPV6__ADDRESS_COUNT 3U
#define DNS_COUNT 2U
#define HW_MAX 8U

enum CbType
{
    CB_ADDR_IP,                 /* IP address */
    CB_ADDR_NETMASK,            /* subnet mask */
    CB_ADDR_GATEWAY,            /* netmask */
    CB_ADDR_DNS_SERVER,         /* dns server */
    CB_STATUS_UP,               /* changed to 'up' */
    CB_STATUS_DOWN,             /* changed to 'down' */
    CB_STATUS_LINK_UP,          /* changed to 'link up' */
    CB_STATUS_LINK_DOWN,        /* changed to 'link down' */
    CB_STATUS_INTERNET_UP,      /* changed to 'internet up' */
    CB_STATUS_INTERNET_DOWN,    /* changed to 'internet down' */
    CB_STATUS_DHCP_ENABLE,      /* enable DHCP capability */
    CB_STATUS_DHCP_DISABLE,     /* disable DHCP capability */
};

enum ChangeType
{
    ADDR_CHANGE,
    STATUS_CHANGE,
};

struct AddressIpv4
{
    uint32 ipv4;
};

struct PingResult
{
    struct AddressIpv4 ip_addr;                           /* response IP address */
    uint16 data_len;                           /* response data length */
    uint16 ttl;                                /* time to live */
    uint32 ticks;                              /* response time, unit tick */
    void *user_data;                             /* user-specific data */
};

#define NBIOT_ADAPTER_ID 0x02U
#define ETHERNET_ADAPTER_ID 0x03U
#define WIFI_ADAPTER_ID 0x04U
#define fourG_ADAPTER_ID 0x05U



#endif