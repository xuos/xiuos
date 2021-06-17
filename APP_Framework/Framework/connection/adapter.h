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
 * @file adapter.h
 * @brief Structure and function declarations of the communication adapter framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.05.10
 */

#ifndef ADAPTER_H
#define ADAPTER_H

#include <list.h>
#include <transform.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#define ADAPTER_BUFFSIZE 64

#define ADAPTER_LORA_FUNC       ((uint32_t)(1 << ATAPTER_LORA))
#define ADAPTER_4G_FUNC         ((uint32_t)(1 << ADAPTER_4G))
#define ADAPTER_NBIOT_FUNC      ((uint32_t)(1 << ADAPTER_NBIOT))
#define ADAPTER_WIFI_FUNC       ((uint32_t)(1 << ADAPTER_WIFI))
#define ADAPTER_ETHERNET_FUNC   ((uint32_t)(1 << ADAPTER_ETHERNET))
#define ADAPTER_BLUETOOTH_FUNC  ((uint32_t)(1 << ADAPTER_BLUETOOTH))
#define ADAPTER_ZIGBEE_FUNC     ((uint32_t)(1 << ADAPTER_ZIGBEE))
#define ADAPTER_5G_FUNC         ((uint32_t)(1 << ADAPTER_5G))

struct Adapter;

struct Socket
{
    int id;
    struct Adapter *adapter;
};

enum AdapterType
{
    ADAPTER_LORA = 0,
    ADAPTER_4G ,
    ADAPTER_NBIOT ,
    ADAPTER_WIFI ,
    ADAPTER_ETHERNET ,
    ADAPTER_BLUETOOTH ,
    ADAPTER_ZIGBEE ,
    ADAPTER_5G ,
};

enum NetProtocolType
{
    PRIVATE_PROTOCOL = 1,
    IP_PROTOCOL,
    PROTOCOL_NONE,
};

enum NetRoleType
{
    CLIENT = 1, 
    SERVER,
    ROLE_NONE,
};

struct AdapterProductInfo
{
    uint32_t functions;
    const char *vendor_name;
    const char *model_name;
};

struct IpProtocolDone
{
    int (*open)(struct Adapter *adapter);
    int (*close)(struct Adapter *adapter);
    int (*ioctl)(struct Adapter *adapter, int cmd, void *args);
    int (*connect)(struct Adapter *adapter, const char *ip, const char *port, uint8_t ip_type);
    int (*send)(struct Socket *socket, const void *buf, size_t len);
    int (*recv)(struct Socket *socket, void *buf, size_t len);
    int (*disconnect)(struct Socket *socket);
};

struct PrivProtocolDone
{
    int (*open)(struct Adapter *adapter);
    int (*close)(struct Adapter *adapter);
    int (*ioctl)(struct Adapter *adapter, int cmd, void *args);
    int (*join)(struct Adapter *adapter, const char *priv_net_group);
    int (*send)(struct Adapter *adapter, const void *buf, size_t len);
    int (*recv)(struct Adapter *adapter, void *buf, size_t len);
    int (*disconnect)(struct Adapter *adapter);
};

struct Adapter
{
    char *name;
    int fd;

    struct AdapterProductInfo *info;
    struct Socket *socket;

    enum NetProtocolType net_protocol;
    enum NetRoleType net_role;

    char buffer[ADAPTER_BUFFSIZE];
    
    void *done;

    struct DoublelistNode link;
};

#endif