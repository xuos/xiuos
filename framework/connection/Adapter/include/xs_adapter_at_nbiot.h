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
 * @file xs_adapter_at_nbiot.h
 * @brief Structure and function declarations of the connection NBIoT
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_AT_NBIOT_H
#define XS_ADAPTER_AT_NBIOT_H

#include "xs_adapter.h"
#include "xs_adapter_at.h"
#include "xs_adapter_at_agent.h"
#include "xs_adapter_def.h"
#include "../../../applications/user_api/switch_api/user_api.h"

#define MAX_SOCKET_NUM 8 

#define SOCKET_STATUS_UNUSED    (0)
#define SOCKET_STATUS_INIT      (1)
#define SOCKET_STATUS_CONNECT   (2)

struct AdapterNBIoT {
    struct AdapterAT parent; /* inherit from Adapter */

    char vendor_name[NAME_LEN_MAX];        
    char product_ID_ethernet[NAME_LEN_MAX];      

    struct SingleLinklistNode link;
};

int NbiotOpen(struct Adapter *padapter);

int NbiotSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved);
int NBIotRecv(struct Adapter *padapter, char *rev_buffer, int buffer_len, int time_out, bool block);

int NBIoTSocketConnect(struct AdapterAT *adapterAT , uint8_t socket_fd , struct ADDRESS_IPV4 dst_ip , uint16_t dst_port, uint8 is_client);
int NBIoTSocketCreate(struct AdapterAT *adapterAT, uint8_t socket_fd, uint8_t type, uint8_t af_type );
int NBIoTSocketClose(struct AdapterAT *adapterAT, uint8_t socket_fd );

#endif