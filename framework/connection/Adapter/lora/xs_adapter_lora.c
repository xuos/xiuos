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
 * @file xs_AdapterLora.c
 * @brief lora adhoc logic
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <xs_adapter_lora.h>
#include "spi_lora_sx12xx.h"
#include "radio.h"
#include "sx1276-LoRa.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <user_api.h>

#define SLEEP_MS 50

enum NetRoleType _role_type = ROLE_TYPE_NONE;

bool gateway_listening = false;
bool client_work_thread_running = false;

char gateway_net_id[NETNAME_LEN_MAX];
char client_net_id[NETNAME_LEN_MAX];

struct LoraClientStatusInfo g_client_status_info;
int pkg_id_c2g;

#define BUFFER_LEN_MAX 120
char rec_buffer[BUFFER_LEN_MAX] = {0};
char client_rec_buffer[BUFFER_LEN_MAX] = {0};

char send_buffer[BUFFER_LEN_MAX] = {0};

DoubleLinklistType online_user_head = {&online_user_head, &online_user_head};

// Client send buffer. Only one element is set temporarily
ClientSendCell client_send_buffer;
int pclient_send_buffer_mutex = -1;
enum WorkThreadStatus g_work_thread_status;
bool need_send_data = false;

int32 ServerTask = NONE;
int32 ClientTask = NONE;

static int spi_lora_fd_intern = -1;

extern tRadioDriver *Radio;

/**
 * @description: open lora adapter
 * @param padapter - lora adapter pointer
 */
int LoraAdapterOpen(adapter_t padapter)
{
    int fd = ((AdapterLoraT)padapter)->spi_lora_fd;
    if (fd < 0){
        fd = open(LORA_SPI_NAME,O_RDWR);
    }

    if(fd < 0){
        printf("LoRa check failed!\n!");
        return ERROR;
    } else {
        ((AdapterLoraT)padapter)->spi_lora_fd = fd;
        spi_lora_fd_intern = fd;

        memset(&g_client_status_info, 0, sizeof(g_client_status_info));
        g_client_status_info.status = LORA_CLIENT_IDLE;
        pkg_id_c2g = 1;
        _role_type = ROLE_TYPE_NONE;
        printf("LoRa check ok!\nNote: The length of the message that can be sent in a single time is 120 characters\n");
    }

    return 0;
}

/**
 * @description: close lora adapter
 * @param padapter - lora adapter pointer
 */
void LoraAdapterCose(struct Adapter *padapter)
{
    if (((AdapterLoraT)padapter)->spi_lora_fd < 0){
        printf("LoRa device not found! close failed!\n");
    } else {
        // Disconnect
        int count = 10;
        g_client_status_info.status = LORA_CLIENT_WAITTING_FOR_DISCONNECTED;
        need_send_data = true;
        while (count > 0 && g_client_status_info.status == LORA_CLIENT_WAITTING_FOR_DISCONNECTED)
        {
            printf("lora client waitting for rsp, count(%d)\n", count);
            UserTaskDelay(1000);
            count--;
        }
        if (g_client_status_info.status != LORA_CLIENT_DISCONNECTED){
            printf("lora send close pkg failed!\n");
        }

        gateway_listening = false;
        client_work_thread_running = false;

        // Release thread
        if (NONE != ServerTask){
            UserTaskDelete(ServerTask);
            ServerTask = NONE;
        }

        if (NONE != ClientTask){
            UserTaskDelete(ClientTask);
            ClientTask = NONE;
        }

        // Release the lock
        if (pclient_send_buffer_mutex != NONE) {
            UserMutexDelete(pclient_send_buffer_mutex);
            pclient_send_buffer_mutex = NONE;
        }

        // Driver
        ((AdapterLoraT)padapter)->spi_lora_fd = NONE;
    }
}

bool lora_channel_avtive()
{
    return Radio->ChannelEmpty() == 0;
}

int LoraAdapterSend(struct Adapter *padapter, const char *data, int len, bool block, int time_out, int delay, void *p)
{
    if (len > 120){
        printf("ERROR:The message is too long!\n");
        return ERROR;
    }

    if (((AdapterLoraT)padapter)->spi_lora_fd == NONE){
        printf("LoRa device not found!\n");
        return ERROR;
    } else {
        int time_counter = 0;
        int time_sleep_gap = 500;
        while (false == lora_channel_avtive()) {
            
            if (time_counter * time_sleep_gap > time_out) {
                printf("LoRa_adapter_send failed! time_out!\n");
                return ERROR;
            }

            
            UserTaskDelay(time_sleep_gap);
            time_counter++;
        }

        char Msg[120] = {0};
        memcpy(Msg, data, len);

        Radio->SetTxPacket(Msg, len);
        while (Radio->Process() != RF_TX_DONE)
            ;
    }
}

int lora_send(const char *data, int len)
{
    if (len > 120) {
        printf("ERROR:The message is too long!\n");
        return ERROR;
    }

    int time_counter = 0;
    int time_sleep_gap = 500;
    int time_out = 10000;
    while (false == lora_channel_avtive()) {
        if (time_counter * time_sleep_gap > time_out) {
            printf("LoRa send failed! time_out!\n");
            return ERROR;
        }

        UserTaskDelay(time_sleep_gap);
        time_counter++;
    }

    char Msg[120] = {0};
    memcpy(Msg, data, len);

    Radio->SetTxPacket(Msg, len);
    while (Radio->Process() != RF_TX_DONE)
        ;
}

int LoraAdapterSendc2g(adapter_t padapter, const char *data, int len, bool block, int time_out, int delay, send_success cb, void *param, void *p)
{
    if (_role_type == ROLE_TYPE_SLAVE && g_client_status_info.status == LORA_CLIENT_CONNECTED) {
        LoraHeader header;
        memset(&header, 0, sizeof(header));
        header.op_code = LORA_C2G_DATA_REQ;
        header.length = sizeof(LoraHeader) + sizeof(LoraProtoC2GDataReq) + len;

        if (header.length > 120) {
            printf("Send failed, LoRa send max length is 120.\n");
            return ERROR;
        }

        if (client_send_buffer.has_data == true){
            printf("Send failed, last pakage is resending\n");
            return ERROR;
        }

        LoraProtoC2GDataReq req;
        req.user_id = g_client_status_info.user_id;
        req.pkg_id = pkg_id_c2g;
        req.data_len = len;
        req.crc = 0xFFFF; 

        memcpy(send_buffer, &header, sizeof(header));
        memcpy(send_buffer + sizeof(header), &req, sizeof(req));
        memcpy(send_buffer + sizeof(header) + sizeof(req), data, len);

        // Write data to buffer (lock)
        UserMutexObtain(pclient_send_buffer_mutex, WAITING_FOREVER);
        client_send_buffer.has_data = true;
        client_send_buffer.pkg_id = pkg_id_c2g;
        client_send_buffer.crc = 0xFFFF;
        memcpy(client_send_buffer.data, send_buffer, header.length);
        client_send_buffer.data_len = header.length;
        client_send_buffer.callback = cb;
        client_send_buffer.param = param;

        UserMutexAbandon(pclient_send_buffer_mutex);
        // printf("copy to send buffer. len = %d\n", header.length);

        // Switch worker thread state
        need_send_data = true;
        return ERROR;
    } else {
        printf("LoRa client is unconnected! can not send data!\n");
    }
}

OnlineUser *find_user_by_name(char *name)
{
    DoubleLinklistType *pLink;
    DOUBLE_LINKLIST_FOR_EACH(pLink, &online_user_head)
    {
        OnlineUser *pUer =CONTAINER_OF(pLink, OnlineUser, link);
        if (strcmp(pUer->user_name, name) == 0) {
            return pUer;
        }
    }
    return NONE;
}

OnlineUser *find_user_by_id(int id)
{
    DoubleLinklistType *pLink;
    DOUBLE_LINKLIST_FOR_EACH(pLink, &online_user_head)
    {
        OnlineUser *pUer =CONTAINER_OF(pLink, OnlineUser, link);
        if (pUer->user_id == id) {
            return pUer;
        }
    }
    return NONE;
}

int insert_connected_clients(char *name, int user_id)
{
    OnlineUser *pUser = malloc(sizeof(OnlineUser));
    if (NONE == pUser)
        return ERROR;

    pUser->user_id = user_id;
    strcpy(pUser->user_name, name);
    DoubleLinkListInsertNodeAfter(&online_user_head, &pUser->link);

    return EOK;
}




void CheckSendBuffer()
{
    UserMutexObtain(pclient_send_buffer_mutex, WAITING_FOREVER);
    if (client_send_buffer.has_data == true)
    {
        //Sending or packet loss retransmission
        lora_send(client_send_buffer.data, client_send_buffer.data_len);
        // printf("client check and send. len = %d\n", client_send_buffer.data_len);
    }
    UserMutexAbandon(pclient_send_buffer_mutex);
}

int LoraAdapterReceive(adapter_t padapter, char *rev_buffer, int buffer_len, int time_out, bool block, void *p)
{
    uint16 BufferSize = buffer_len;

    memset(rev_buffer, 0, buffer_len);

    if (((AdapterLoraT)padapter)->spi_lora_fd == NONE)
    {
        printf("LoRa device not found!\n");
    }
    else
    {
        Radio->StartRx();
        printf("Ready!\n");

        while (Radio->Process() != RF_RX_DONE)
        {
            // if (time_out < 0)
            // {
            //     printf("LoRa device receive failed! Timeout!\n");
            //     return ERROR;
            // }
            // UserTaskDelay(SLEEP_MS);
            // time_out -= SLEEP_MS;
        }

        Radio->GetRxPacket(rev_buffer, (uint16 *)&BufferSize);
        if (BufferSize == buffer_len)
        {
            printf("rev_buffer is too small!\n");
        }

        printf("RX : %s\n", rev_buffer);
        return BufferSize;
    }
}

int LoraGatewayProcess(adapter_t padapter, char *pkg, int rec_len)
{
    struct AdapterLora* lora_adapter = (struct AdapterLora*)padapter;

    if (rec_len > 0)
    {
        LoraHeader header;
        memset(&header, 0, sizeof(header));
        char *start = (char *)pkg;
        LoraHeader *pheader = (LoraHeader *)start;

        char *req = start + sizeof(LoraHeader);

        if (pheader->op_code < LORA_OPCODE_START || pheader->op_code > LORA_OPCODE_END)
        {
            printf("Illegal opcode, discard data!\n");
            return ERROR;
        }

        if (rec_len != pheader->length)
        {
            printf("pkg is not complete!, discard data!\n");
            return ERROR;
        }

        printf("gateway receive pkg opcode(%4x)\n", pheader->op_code);
        switch (pheader->op_code)
        {
        case LORA_JOIN_REQ:                                                        // There is a client request to join the network segment
            if (strcmp(((LoraProtoJoinReq *)req)->net_id, gateway_net_id) == 0) // The request is really this network segment
            {
                LoraProtoJoinReq *req = (LoraProtoJoinReq *)(start + sizeof(LoraHeader));

                header.op_code = LORA_JOIN_RSP;
                header.length = sizeof(LoraHeader) + sizeof(LoraProtoJoinRsp);

                LoraProtoJoinRsp rsp;
                memset(&rsp, 0, sizeof(rsp));
                rsp.error_code = 0;
                rsp.signal_strength = 0; // signal intensity

                printf(" 11aa strlen(lora_adapter->name) = %d\n", strlen(lora_adapter->name));
                printf(" 11aa lora_adapter->name = %s\n", lora_adapter->name);

                strcpy(rsp.gateway_name, lora_adapter->name);
                strcpy(rsp.net_id, gateway_net_id);

                memcpy(send_buffer, &header, sizeof(header));
                memcpy(send_buffer + sizeof(header), &rsp, sizeof(rsp));

                printf("LoraProtoJoinRsp rsp.gateway_name = %s rsp.net_id = %s\n", rsp.gateway_name, rsp.net_id);
                lora_send(send_buffer, header.length);
                printf("gateway send pkg opcode(%4x)\n", header.op_code);
            }
            break;
        case LORA_HANDSHAKE_REQ:
            if (strcmp(((LoraProtoHandshakeReq *)req)->gateway_name, lora_adapter->name) == 0) //The other party really wants to connect themselves
            {
                // Construction reply, connection confirmation
                LoraProtoHandshakeReq *req = (LoraProtoHandshakeReq *)(start + sizeof(LoraHeader));
                int user_id = 0;
                // If it can be found, it will prove that it is connected. It may be packet loss, so normal retransmission is good
                OnlineUser *pUser = find_user_by_name(req->client_name);
                if (NONE == pUser)
                {
                    // New virtual connection
                    user_id = rand() % UINT32_SIZE_MAX;
                    if (ERROR == insert_connected_clients(req->client_name, user_id))
                    {
                        return ERROR;
                    }
                }
                else
                {
                    // Packet loss
                    user_id = pUser->user_id;
                }

                header.op_code = LORA_HANDSHAKE_RSP;
                header.length = sizeof(LoraHeader) + sizeof(LoraProtoHandshakeRsp);

                LoraProtoHandshakeRsp rsp;
                memset(&rsp, 0, sizeof(rsp));
                strcpy(rsp.client_name, req->client_name);
                strcpy(rsp.gateway_name, req->gateway_name);
                rsp.user_id = user_id;
                rsp.error_code = 0;

                memcpy(send_buffer, &header, sizeof(header));
                memcpy(send_buffer + sizeof(header), &rsp, sizeof(rsp));

                printf("LoraProtoHandshakeRsp, rsp.client_name = %s, rsp.gateway_name = %s\n", rsp.client_name, rsp.gateway_name);
                lora_send(send_buffer, header.length);
                printf("gateway send pkg opcode(%4x)\n", header.op_code);
            }
            break;
        case LORA_C2G_DATA_REQ:
        {
            OnlineUser *pUser = find_user_by_id(((LoraProtoC2GDataReq *)req)->user_id);
            if (pUser) //What the other party wants to send is really himself
            {
                LoraProtoC2GDataReq *req = (LoraProtoC2GDataReq *)(start + sizeof(LoraHeader));

                char *data = start + sizeof(LoraHeader) + sizeof(LoraProtoC2GDataReq);
                // printf("receive data from client(%s), content(%s)", pUser->user_name, data);
                printf(" receive data from \033[0;31m client(%s)\033[0m \n, content(%s). ", pUser->user_name, data);
                // Logic layer to deal with

                // CRC calculation for data and reply to client
                header.op_code = LORA_C2G_DATA_RSP;
                header.length = sizeof(LoraHeader) + sizeof(LoraProtoC2GDataRsp);

                LoraProtoC2GDataRsp rsp;
                memset(&rsp, 0, sizeof(rsp));
                rsp.user_id = req->user_id;
                rsp.crc = 0xFFFF;
                rsp.data_len = rec_len;
                rsp.ack_id = req->pkg_id;

                memcpy(send_buffer, &header, sizeof(header));
                memcpy(send_buffer + sizeof(header), &rsp, sizeof(rsp));

                lora_send(send_buffer, header.length);
                printf("gateway send pkg opcode(%4x), ack_id(%d)\n", header.op_code, rsp.ack_id);
            }
            else
            {
                printf("user unconnected, discard data.\n");
                return ERROR;
            }

            break;
        }

        case LORA_CLOSE_REQ:
        {
            LoraProtoCloseReq *req = (LoraProtoCloseReq *)(start + sizeof(LoraHeader));

            //log
            DoubleLinklistType *pNode;
            printf("******** connected users *********\n");
            DOUBLE_LINKLIST_FOR_EACH(pNode, &online_user_head)
            {
                OnlineUser *pUser =CONTAINER_OF(pNode, OnlineUser, link);
                printf("pUser->user_name %s\n", pUser->user_name);
                printf("pUser->user_id   %d\n", pUser->user_id);
            }
            printf("*********************************\n");

            printf("req->user_id = %d\n", req->user_id);

            OnlineUser *pUser = find_user_by_id(req->user_id);
            if (pUser)
            {
                DoubleLinkListRmNode(&pUser->link);

                header.op_code = LORA_CLOSE_RSP;
                header.length = sizeof(LoraHeader) + sizeof(LoraProtoCloseRsp);
                LoraProtoCloseRsp rsp;
                rsp.error_code = 0;
                rsp.user_id = req->user_id;
                memcpy(send_buffer, &header, sizeof(header));
                memcpy(send_buffer + sizeof(header), &rsp, sizeof(rsp));

                lora_send(send_buffer, header.length);
            }
            else
            {
                printf("find user failed!\n");
            }
        }

        default:
            break;
        }
    }
}

// Gateway receives requests from clients in a loop
static void LoraGateWayListen(void *parameter)
{
    // adapter_t padapter = (adapter_t)malloc(sizeof(struct Adapter));
    adapter_t padapter = parameter;

    while (gateway_listening)
    {
        printf("444x(0x%x), %d, %d \n", ((AdapterLoraT)padapter)->spi_lora_fd, sizeof(rec_buffer), BUFFER_LEN_MAX);

        int rec_len = LoraAdapterReceive(padapter, rec_buffer, BUFFER_LEN_MAX, 10000, true, NULL);
        UserTaskDelay(50); //Afraid the other party hasn't entered reception mode yet
        LoraGatewayProcess(padapter, rec_buffer, rec_len);
    }
}

//  All client packets received are processed here
int LoraClientProcess(adapter_t padapter, char *pkg, int pkg_len)
{
    struct AdapterLora* lora_adapter = (struct AdapterLora*)padapter;

    LoraHeader *pheader = (LoraHeader *)pkg;

    LoraHeader header;
    memset(&header, 0, sizeof(header));

    char *data_start = pkg + sizeof(LoraHeader);

    switch (pheader->op_code)
    {
    case LORA_JOIN_RSP:
        if (g_client_status_info.status == LORA_CLIENT_LOOKING4GATEWAY)
        {
            LoraProtoJoinRsp *data = (LoraProtoJoinRsp *)data_start;

            if (strcmp(data->net_id, client_net_id) != 0)
                break;

            strcpy(g_client_status_info.gateway_name, data->gateway_name);
            strcpy(g_client_status_info.client_name, lora_adapter->name);
            g_client_status_info.status = LORA_CLIENT_CONNECTING2GATEWAY;
            break;
        }
    case LORA_HANDSHAKE_RSP:
        if (strcmp(((LoraProtoHandshakeRsp *)data_start)->client_name, lora_adapter->name) == 0) // Confirm that it was sent to yourself
        {
            LoraProtoHandshakeRsp *data = (LoraProtoHandshakeRsp *)data_start;
            printf("get LoraProtoHandshakeRsp data->client_name = %s, data->gateway_name = %s\n", data->client_name, data->gateway_name);
            // Confirm the connection and record the status information
            g_client_status_info.status = LORA_CLIENT_CONNECTED;
            g_client_status_info.user_id = data->user_id;

            strcpy(g_client_status_info.gateway_name, data->gateway_name);

            printf("client is connected to gateway(%s)\n", g_client_status_info.gateway_name);
        }
        break;

    case LORA_C2G_DATA_RSP:
        if (((LoraProtoC2GDataRsp *)data_start)->user_id == g_client_status_info.user_id) // Confirm that it was sent to yourself
        {
            LoraProtoC2GDataRsp *data = (LoraProtoC2GDataRsp *)data_start;

            UserMutexObtain(pclient_send_buffer_mutex, WAITING_FOREVER);
            if (data->ack_id == client_send_buffer.pkg_id)
            {
                if (data->crc == client_send_buffer.crc)
                {
                    // Send successfully, execute external callback
                    if (client_send_buffer.callback)
                    {
                        client_send_buffer.callback(client_send_buffer.param);
                    }

                    //Reset buffer
                    memset(&client_send_buffer, 0, sizeof(client_send_buffer));
                    client_send_buffer.has_data = false;
                    // printf("pkg_id(%d) get ack. send success. send buffer clear.\n", data->ack_id);

                    pkg_id_c2g++;
                }
                else
                {
                    // Then do nothing and wait for the retransmission
                    printf("client send failed.crc check failed.\n");
                }
            }
            UserMutexAbandon(pclient_send_buffer_mutex);
        }
        break;

    case LORA_CLOSE_RSP:
    {
        LoraProtoCloseRsp *rsp = (LoraProtoCloseRsp *)data_start;
        printf("case LORA_CLOSE_RSP                 rsp->user_id(%d)\n", rsp->user_id);
        printf("case LORA_CLOSE_RSP g_client_status_info.user_id(%d)\n", g_client_status_info.user_id);

        if (rsp->user_id == g_client_status_info.user_id)
        {
            g_client_status_info.status = LORA_CLIENT_DISCONNECTED;
            printf("client close success.\n");
        }
    }
    break;
    default:
        break;
    }
}

// The client requests the gateway to disconnect
int SendCloseReq()
{
    LoraHeader header;
    memset(&header, 0, sizeof(header));
    header.op_code = LORA_CLOSE_REQ;
    header.length = sizeof(LoraHeader) + sizeof(LoraProtoCloseReq);

    LoraProtoCloseReq req;
    req.user_id = g_client_status_info.user_id;
    memcpy(send_buffer, &header, sizeof(header));
    memcpy(send_buffer + sizeof(header), &req, sizeof(req));
    printf("client send close req");
    lora_send(send_buffer, header.length);
}

// The client broadcasts the name of the network segment that it wants to join
int SendJoinReq(char *net_id)
{
    LoraHeader header;
    memset(&header, 0, sizeof(header));
    header.op_code = LORA_JOIN_REQ;
    header.length = sizeof(LoraHeader) + sizeof(LoraProtoJoinReq);

    LoraProtoJoinReq req;
    memcpy(req.net_id, net_id, strlen(net_id) + 1);
    memcpy(send_buffer, &header, sizeof(header));
    memcpy(send_buffer + sizeof(header), &req, sizeof(req));

    lora_send(send_buffer, header.length);
}

// Client requests connection from gateway
int SendHandShakeReq(char *client_name, char *gateway_name)
{
    LoraHeader header;
    header.op_code = LORA_HANDSHAKE_REQ;
    header.length = sizeof(LoraHeader) + sizeof(LoraProtoHandshakeReq);

    LoraProtoHandshakeReq req;
    memset(&req, 0, sizeof(req));
    strcpy(req.client_name, client_name);
    strcpy(req.gateway_name, gateway_name);

    memcpy(send_buffer, &header, sizeof(header));
    memcpy(send_buffer + sizeof(header), &req, sizeof(req));

    printf("LoraProtoHandshakeReq, req.client_name = %s req.gateway_name = %s\n", req.client_name, req.gateway_name);
    lora_send(send_buffer, header.length);
}

void work_thread_process(adapter_t padapter)
{
    if (g_work_thread_status == WORK_THREAD_RX)
    {
        // printf("client start receiving \n");
        uint16 len;
        int counter = 0;
        Radio->StartRx();
        while (Radio->Process() != RF_RX_DONE)
        {
            // Receive external signal, check buffer
            if (need_send_data)
            {
                g_work_thread_status = WORK_THREAD_TX;
                need_send_data = false;
                return;
            }

            //Jump out of the monitoring cycle regularly to see if there is anything that needs to be retransmitted
            if (counter > 100000)
            {
                if (g_client_status_info.status == LORA_CLIENT_CONNECTED)
                {
                    //printf("check send buffer.\n");
                }
                if (g_client_status_info.status >= LORA_CLIENT_LOOKING4GATEWAY &&
                    g_client_status_info.status <= LORA_CLIENT_CONNECTING2GATEWAY)
                {
                    printf("retry to handshake.\n");
                }
                g_work_thread_status = WORK_THREAD_TX;
                // printf("client end receiving.\n");
                return;
            }
            counter++;
        }

        enum LoraClientStatus old = g_client_status_info.status;
        Radio->GetRxPacket(rec_buffer, (uint16 *)&len);
        LoraClientProcess(padapter, rec_buffer, len);

        //If the client state machine changes, there is something to send to the other party, and it will be switched to the sending mode immediately
        if (old != g_client_status_info.status) 
        {
            g_work_thread_status = WORK_THREAD_TX;
            UserTaskDelay(50); //Afraid the other party hasn't entered reception mode yet
        }
    }
    else if (g_work_thread_status == WORK_THREAD_TX)
    {
        // Temporarily designed not to be interrupted
        switch (g_client_status_info.status)
        {
        case LORA_CLIENT_LOOKING4GATEWAY:
            SendJoinReq(client_net_id);
            break;
        case LORA_CLIENT_CONNECTING2GATEWAY:
            SendHandShakeReq(g_client_status_info.client_name, g_client_status_info.gateway_name);
            break;
        case LORA_CLIENT_CONNECTED:
            CheckSendBuffer();
            break;
        case LORA_CLIENT_WAITTING_FOR_DISCONNECTED:
            SendCloseReq(g_client_status_info.user_id);
            break;
        default:
            break;
        }
        g_work_thread_status = WORK_THREAD_RX;
    }
}

void static work_thread_func(void *parameter)
{
    adapter_t padapter = (adapter_t)parameter;
    while (client_work_thread_running)
    {
        work_thread_process(padapter);
    }
}
int LoraAdapterJoin(adapter_t padapter, int net_role_type, char *net_id)
{
    UtaskType lora_utask_master;
    UtaskType lora_utask_slave;
    do
    {
        _role_type = net_role_type;
        x_err_t err;
        if (_role_type == ROLE_TYPE_MASTER)
        {
            strcpy(gateway_net_id, net_id);

            //Single child thread gateway loop monitoring req
            gateway_listening = true;
            printf("GateWayListen thread create...");

            strncpy(lora_utask_master.name,"GateWayListen",strlen("GateWayListen"));
            lora_utask_master.func_entry = LoraGateWayListen;
            lora_utask_master.func_param = padapter;
            lora_utask_master.prio = 10;
            lora_utask_master.stack_size = 2048;

            ServerTask = UserTaskCreate(lora_utask_master);
            err = UserTaskStartup(ServerTask);
            if (err != EOK)
            {
                break;
            }
        }
        else if (_role_type == ROLE_TYPE_SLAVE)
        {
            strcpy(client_net_id, net_id);

            // Create lock
            if (pclient_send_buffer_mutex < 0)
            {
                pclient_send_buffer_mutex = UserMutexCreate();
            }

            // Update state machine
            g_client_status_info.status = LORA_CLIENT_LOOKING4GATEWAY;

            // Start single child thread client loop to listen for RSP
            client_send_buffer.has_data = false;
            memset(&client_send_buffer, 0, sizeof(client_send_buffer));
            client_work_thread_running = true;
            g_work_thread_status = WORK_THREAD_TX;

            strncpy(lora_utask_slave.name,"ClientWorkThread",strlen("ClientWorkThread"));
            lora_utask_slave.func_entry = work_thread_func;
            lora_utask_slave.func_param = padapter;
            lora_utask_slave.prio = 10;
            lora_utask_slave.stack_size = 2048;

            ClientTask = UserTaskCreate(lora_utask_slave);

            err = UserTaskStartup(ClientTask);
            if (err != EOK)
            {
                break;
            }
            // Block detection for a period of time
            int counter = 100;
            while (counter > 0)
            {
                if (g_client_status_info.status == LORA_CLIENT_CONNECTED)
                {
                    break; // Successful connection 
                }
                else
                {
                    UserTaskDelay(300);
                }
                counter--;
            }
            return (counter > 0) ? 0 : ERROR;
        }
        return EOK;
    } while (false);

    // Exception handling, releasing resources
    LoraAdapterCose(padapter);
    return ERROR;
}
