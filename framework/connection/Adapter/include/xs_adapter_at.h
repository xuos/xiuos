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
 * @file xs_adapter_at.h
 * @brief AdapterAT interface
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_AT_H
#define XS_ADAPTER_AT_H

#include "xs_adapter.h"
#include <xs_adapter_at_agent.h>

#define SOCKET_STATUS_UNUSED    (0)
#define SOCKET_STATUS_INIT      (1)
#define SOCKET_STATUS_CONNECT   (2)

#define SOCKET_TYPE_DGRAM   (0)
#define SOCKET_TYPE_STREAM  (1)

#define SOCKET_PROTOCOL_TCP  (6)
#define SOCKET_PROTOCOL_UDP  (17)

#define NET_TYPE_AF_INET   (0)
#define NET_TYPE_AF_INET6  (1)

#define SOCKET_MAX 8

struct Socket
{
    uint8_t fd;
    uint8_t status ;
    struct ADDRESS_IPV4 src_ip;
    uint16_t src_port;
    struct ADDRESS_IPV4 dst_ip;
    uint16_t dst_port;
    uint8_t type;
    uint8_t af_type;
    uint8_t protocal;
    uint8 is_client;
};

struct AdapterAT;
struct ATDone 
{
    int (*ATOperateUp)(struct AdapterAT *adapterAT);
    int (*ATOperateDown)(struct AdapterAT *adapterAT);
    int (*ATOperateAddr)(struct AdapterAT *adapterAT, uint ip, uint gateway, uint netmask);
    int (*ATOperateDns)(struct AdapterAT *adapterAT, struct ADDRESS_IPV4 *dns_addr, uint8 dns_count);
    int (*ATOperateDHCP)(struct AdapterAT *adapterAT, bool enable);
    int (*ATPing)(struct AdapterAT *adapterAT, const char *destination, struct ping_result *ping_resp);
    int (*ATNetstat)(struct AdapterAT *adapterAT);
    int (*ATOperateDefault)(struct AdapterAT *adapterAT);

    int (*ATSocketCreate)(struct AdapterAT *adapterAT , uint8_t socket_fd , uint8_t type ,  uint8_t af_type );
    int (*ATSocketConnect)(struct AdapterAT *adapterAT , uint8_t socket_fd , struct ADDRESS_IPV4 dst_ip , uint16_t dst_port, uint8 is_client);
    int (*ATSocketClose)(struct AdapterAT *adapterAT , uint8_t socket_fd );
};

struct AdapterAT {
    struct Adapter parent;                   
    struct ATAgent *agent;
    struct ADDRESS_IPV4 ip;                                
    struct ADDRESS_IPV4 netmask;                                 
    struct ADDRESS_IPV4 gateway;                                     
    struct ADDRESS_IPV4 dns[DNS_COUNT];           
    uint32 at_adapter_id;
    struct Socket socket[SOCKET_MAX];
    uint8 hardware_address_len;                                 
    uint8 hardware_address[HW_MAX];                              
    uint16 flags;                                   
    uint16 mtu;                                      

    void (*change_cb )(struct AdapterAT *adapterAT, enum cb_type type, enum change_type ch_type_);

    struct ATDone atdone;

    struct SysDoubleLinklistNode link;  
};

#endif