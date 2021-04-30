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
 * @file xs_adapterLora.h
 * @brief lora adhoc logic
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_LORA_H
#define XS_ADAPTER_LORA_H

#include "xs_adapter.h"

#define DEVNAME_LEN_MAX 32
#define NETNAME_LEN_MAX 32
#define CONNECTED_CLIENTS_MAX 512

#define CLIENT_SEND_CELL_LEN 120


struct AdapterLora {
    struct Adapter parent; /* inherit from Adapter */
    const char * name; /* name of the adapter instance */

    const char *deve_ui; /* Lora specific value */
    const char *app_key; /* Lora specific value */

    int spi_lora_fd;
};
typedef struct AdapterLora *AdapterLoraT;

int LoraAdapterOpen(adapter_t padapter);
void LoraAdapterCose(struct Adapter *padapter);
int LoraAdapterSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, void *p);
int LoraAdapterReceive(adapter_t padapter, char* rev_buffer, int buffer_len,int time_out, bool block, void *p);
int LoraAdapterJoin(adapter_t sadapter, int dev_type, char* net_id);
int LoraAdapterSendc2g(adapter_t padapter, const char *data, int len, bool block, int time_out, int delay, send_success cb, void* param, void *p);

// Client state machine
enum LoraClientStatus
{
    LORA_CLIENT_IDLE = 0,
    LORA_CLIENT_LOOKING4GATEWAY,
    LORA_CLIENT_CONNECTING2GATEWAY,
    LORA_CLIENT_CONNECTED,
    LORA_CLIENT_WAITTING_FOR_DISCONNECTED,
    LORA_CLIENT_DISCONNECTED,
};

struct LoraClientStatusInfo
{
    enum LoraClientStatus status; 
    int user_id;
    char gateway_name[DEVNAME_LEN_MAX];
    char client_name[DEVNAME_LEN_MAX];
};

enum LoraOpcode
{
    LORA_OPCODE_START = 0xFFF0,
    LORA_JOIN_REQ = 0xFFF1,
    LORA_JOIN_RSP = 0xFFF2,
    LORA_HANDSHAKE_REQ = 0xFFF3,
    LORA_HANDSHAKE_RSP = 0xFFF4,
    LORA_C2G_DATA_REQ = 0xFFF5,
    LORA_C2G_DATA_RSP = 0xFFF6,
    LORA_CLOSE_REQ = 0xFFF7,
    LORA_CLOSE_RSP = 0xFFF8,
    LORA_OPCODE_END,
};

enum WorkThreadStatus
{
    WORK_THREAD_RX = 1,
    WORK_THREAD_TX,
};

// pkg header
typedef struct  
{
    int op_code;
    int length;
}LoraHeader;

// Network access, handshake function protocol
typedef struct 
{
    char net_id[NETNAME_LEN_MAX];
}LoraProtoJoinReq;

typedef struct 
{
    char net_id[NETNAME_LEN_MAX];
    char gateway_name[DEVNAME_LEN_MAX];
    int signal_strength;
    int error_code;
}LoraProtoJoinRsp;

typedef struct 
{
    char client_name[DEVNAME_LEN_MAX];
    char gateway_name[DEVNAME_LEN_MAX];
}LoraProtoHandshakeReq;

typedef struct 
{
    char client_name[DEVNAME_LEN_MAX];
    char gateway_name[DEVNAME_LEN_MAX];
    int user_id;
    int error_code;
}LoraProtoHandshakeRsp;


// Data transmission protocol
typedef struct 
{
    int user_id;
    int pkg_id;
    int data_len;
    int crc;
}LoraProtoC2GDataReq;

typedef struct 
{
    int user_id;
    int ack_id;
    int data_len;
    int crc;
}LoraProtoC2GDataRsp;

// Client active disconnect
typedef struct 
{
    int user_id;
}LoraProtoCloseReq;

typedef struct 
{
    int user_id;
    int error_code;
}LoraProtoCloseRsp;

typedef struct 
{
    char* data;
    int len;
    bool* prsp_flag;
    adapter_t padapter;    
}CheckRspParam;

typedef void(*send_success)(void* param);
typedef struct 
{
    bool has_data;
    int crc;
    int pkg_id;
    char data[CLIENT_SEND_CELL_LEN];
    int data_len;
    send_success callback;
    void* param;
}ClientSendCell;

typedef struct 
{
    char user_name[DEVNAME_LEN_MAX];
    int user_id;
    DoubleLinklistType link;
}OnlineUser;


#endif

