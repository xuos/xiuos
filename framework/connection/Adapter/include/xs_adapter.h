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
 * @file xs_adapter.h
 * @brief Adapter interface
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_N
#define XS_ADAPTER_N

#include <stdbool.h>
#include <xs_klist.h>
#include <xs_adapter_def.h>



enum AdapterType {
    ADAPTER_LORA = 0,       /* Lora */
    ADAPTER_4G ,            /* 4G */
    ADAPTER_NBIOT ,         /* _NBIot */
    ADAPTER_WIFI ,          /* WIFI */
    ADAPTER_ETHERNET ,      /* ETHERNET */
    ADAPTER_BLUETOOTH ,     /* BLUETOOTH */
    ADAPTER_ZIGBEE ,        /* ZIGBEE */
    ADAPTER_5G ,            /* 5G */
};
enum MeshType{
    NET_P2P = 0,
    NET_ADHOC_SINGLE_GROUP,
    NET_ADHOC_MULTI_GROUP,
};

enum NetRoleType{
    ROLE_TYPE_SLAVE = 1, 
    ROLE_TYPE_MASTER,
    ROLE_TYPE_NONE,
};

struct Adapter;
struct AdapterDone {
    void (*NetAiitClose)(struct Adapter *padapter);
    int (*NetAiitOpen)(struct Adapter *padapter);
    int (*NetAiitJoin)(struct Adapter *padapter, int dev_type, char* net_id);
    int (*NetAiitSend)(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved);
    int (*NetAiitReceive)(struct Adapter *padapter, char* rev_buffer, int buffer_len,int time_out, bool block, void* reserved);
    int (*NetAiitIoctl)(struct Adapter *padapter, int cmd, void *arg);
};

struct Adapter
{
    enum AdapterType type;      /* type of adapter, such as lora adapter */
    enum NetRoleType net_role_type;
    enum MeshType mesh_type;
    struct AdapterDone done; /* socket-like APIs for data transferring */
    struct SysDoubleLinklistNode link; /* link list node */
};
typedef struct Adapter *adapter_t;

#endif