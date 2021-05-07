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
 * @file xs_adapter_at_nbiot.c
 * @brief BC28 NBIoT driver base connection framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <dev_serial.h>
#include <xs_adapter_at_nbiot.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_def.h>

#define NBIOT_DEVICE_NAME "/dev/usart2_dev2"

#define SOCKET_TYPE_DGRAM   (0)
#define SOCKET_TYPE_STREAM  (1)

#define SOCKET_PROTOCOL_TCP  (6)
#define SOCKET_PROTOCOL_UDP  (17)

#define NET_TYPE_AF_INET   (0)
#define NET_TYPE_AF_INET6  (1)

/**
 * @description: Open NBIoT device
 * @param padapter - NBIoT adapter
 * @return success: EOK, failure: -ERROR
 */
int NbiotOpen(struct Adapter *padapter)
{
    char *agent_name = "urat2_client";
    const char *device_name = NBIOT_DEVICE_NAME;

    struct SerialCfgParam cfg;
    cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    cfg.data_cfg.serial_bit_order = 0;
    cfg.data_cfg.serial_buffer_size = SERIAL_RB_BUFSZ;
    cfg.data_cfg.serial_data_bits = DATA_BITS_8;
    cfg.data_cfg.serial_invert_mode = 0;
    cfg.data_cfg.serial_parity_mode = PARITY_NONE;
    cfg.data_cfg.serial_stop_bits = STOP_BITS_1;

    if (InitATAgent(agent_name, device_name, 512, &cfg)) {
        printf("NBIoT open failed!\n");
        return -ERROR;
    }

    ATAgentType at_agent = GetATAgent(agent_name);
    if (NULL == at_agent) {
        printf("Get AT agent failed!\n");
        return -ERROR;
    }

    UserTaskDelay(3000);
    struct AdapterAT *nbiot = (struct AdapterAT *)padapter;
    nbiot->agent = at_agent;

    return EOK;
}

/**
 * @description: NBIoT device create a socket connection
 * @param adapterAT - NBIoT adapter AT
 * @param socket_fd - socket file description
 * @param type - socket type
 * @param af_type - IPv4 or IPv6
 * @return success: EOK, failure: -ERROR
 */
int NBIoTSocketCreate(struct AdapterAT *adapterAT, uint8_t socket_fd, uint8_t type, uint8_t af_type )
{
    int32 result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at create failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    if ( af_type == NET_TYPE_AF_INET6) {
        printf("IPv6 not surport !\n");
        return -1;
    }

    char *str_af_type = "AF_INET";
    char *str_type;
    char str_fd[3] = {0};
    char *str_protocol ;
    char at_cmd[64] = {0};
    char *linsten_port = "0";
    struct Socket socket = {0};
    socket.status = SOCKET_STATUS_INIT;
    socket.af_type = NET_TYPE_AF_INET;

    if (type == SOCKET_TYPE_STREAM) {   //tcp = AT+NSOCR=STREAM,6,0,1,AF_INET
        socket.type = SOCKET_TYPE_STREAM;
        socket.protocal = SOCKET_PROTOCOL_TCP;
        str_type = "STREAM";
        char *str_protocol = "6";
        if (socket_fd > 0 && socket_fd < 8) {
            str_fd[0] = socket_fd + '0';
        } else
            str_fd[0] = '0';
        
        memcpy(at_cmd, "AT+NSOCR=", 9);
        strcat(at_cmd, str_type);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_protocol);
        strcat(at_cmd, ",");
        strcat(at_cmd, linsten_port);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_fd);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_af_type);
        strcat(at_cmd, "\n");

    }else if ( type == SOCKET_TYPE_DGRAM ){ //udp
        socket.type = SOCKET_TYPE_DGRAM;
        socket.protocal = SOCKET_PROTOCOL_UDP;
        str_type = "DGRAM";
        char *str_protocol = "17";
        if (socket_fd > 0 && socket_fd < 8){
            str_fd[0] = socket_fd + '0';
        }else
            str_fd[0] = '0';

        memcpy(at_cmd, "AT+NSOCR=", 9);
        strcat(at_cmd, str_type);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_protocol);
        strcat(at_cmd, ",");
        strcat(at_cmd, linsten_port);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_fd);
        strcat(at_cmd, ",");
        strcat(at_cmd, str_af_type);
        strcat(at_cmd, "\n");

    }else{
        printf("error socket type \n");
        return -1 ;
    }

    printf("cmd : %s\n", at_cmd);
    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, at_cmd);
    UserTaskDelay(3000);
    printf("bak : ");
    for(int i = 0; i < strlen(reply->reply_buffer); i++)
        printf(" 0x%02x", reply->reply_buffer[i]);
    printf("\n");

    struct Socket (*socketlist)[MAX_SOCKET_NUM] = &adapterAT->socket;
    memset(socketlist[socket_fd], 0, sizeof(struct Socket));
    memcpy(socketlist[socket_fd], &socket , sizeof(struct Socket));

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

/**
 * @description: NBIoT device create a socket connection
 * @param adapterAT - NBIoT adapter AT
 * @param socket_fd - socket file description
 * @param dst_ip - ip address
 * @param dst_port - ip port
 * @param is_client - whether it is a client
 * @return success: EOK, failure: -ERROR
 */
int NBIoTSocketConnect(struct AdapterAT *adapterAT , uint8_t socket_fd , struct AddressIpv4 dst_ip , uint16_t dst_port, uint8 is_client)
{
    int result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at create failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    if (socket_fd < 1 || socket_fd > 8) {
        printf("socket fd error \n");
        return -1 ;
    }
    struct Socket (*socketlist)[SOCKET_MAX] = &adapterAT->socket;
    
    if (socketlist[socket_fd]->status !=  SOCKET_STATUS_INIT || socketlist[socket_fd]->type != SOCKET_TYPE_STREAM) {
        printf("socket type error \n");
    }

    char at_cmd[64] = {0};
    char str_fd[2] = {0};
    char str_port[6] = {0};

    str_fd[0] = socket_fd + '0';
    char *str_ip = IpTstr(dst_ip.ipv4) ;
    sprintf(str_port, "%u", dst_port);

    memcpy(at_cmd, "AT+NSOCO=", 9);
    strcat(at_cmd, str_fd);
    strcat(at_cmd, ",");
    strcat(at_cmd, str_ip);
    strcat(at_cmd, ",");
    strcat(at_cmd, str_port);
    strcat(at_cmd, "\n");

    printf("cmd : %s\n", at_cmd);
    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, at_cmd);
    UserTaskDelay(300);
    
    socketlist[socket_fd]->dst_ip = dst_ip;
    socketlist[socket_fd]->dst_port = dst_port;
    socketlist[socket_fd]->status = SOCKET_STATUS_CONNECT;

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

/**
 * @description: NBIoT device close a socket connection
 * @param adapterAT - NBIoT adapter AT
 * @param socket_fd - socket file description
 * @return success: EOK, failure: -ERROR
 */
int NBIoTSocketClose(struct AdapterAT *adapterAT, uint8_t socket_fd )
{
    int result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at create failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    if (socket_fd < 1 || socket_fd > 7) {
        printf("socket fd error \n");
        return -1 ;
    }

    struct Socket (*socketlist)[SOCKET_MAX] = &adapterAT->socket;

    char str_fd[2] = {0};
    char at_cmd[16] = {0};
    str_fd[0] = socket_fd + '0';

    memcpy(at_cmd, "AT+NSOCL=", 9);
    strcat(at_cmd, str_fd);
    strcat(at_cmd, "\n");

    printf("cmd : %s\n", at_cmd);
    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, at_cmd);
    UserTaskDelay(300);

    memset(socketlist[socket_fd], 0, sizeof(struct Socket));

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

/**
 * @description: NBIoT socket send a message
 * @param padapter - NBIoT adapter
 * @param data - send data buffer
 * @param len - send data length
 * @param block - block
 * @param time_out - timeout
 * @param delay - delay time
 * @param cb - send callback function
 * @param param - send callback function parameter
 * @param reserved - reserved parameter
 * @return success: EOK, failure: -ERROR
 */
int NbiotSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved)
{
    uint32_t result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at create failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    struct AdapterAT *adapterAT = (struct AdapterAT *)padapter;
    int socket_fd = *(int*)reserved;

    struct Socket (*socketlist)[SOCKET_MAX] = &adapterAT->socket;
    
    char at_cmd[64] = {0};
    char str_fd[2] = {0};
    char size[2] = {0};

    str_fd[0] = socket_fd + '0';
    size[0] = len + '0';

    memcpy(at_cmd, "AT+NSOSD=", 9);
    strcat(at_cmd, str_fd);
    strcat(at_cmd, ",");
    strcat(at_cmd, size);
    strcat(at_cmd, ",");
    strcat(at_cmd, data);
    strcat(at_cmd, "\n");

    printf("cmd : %s\n", at_cmd);
    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, at_cmd);
    UserTaskDelay(300);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}