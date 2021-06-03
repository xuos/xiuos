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
 * @file xs_adapter_at_agent.h
 * @brief AT proxy, auto receive AT reply and transparency data
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef XS_ADAPTER_AT_CLIENT_H
#define XS_ADAPTER_AT_CLIENT_H

#include <stdarg.h>
#include <stddef.h>
#include <bus_serial.h>

enum ReceiveMode
{
    ENTM_MODE = 1,
    AT_MODE = 2,
};

struct ATReply
{
    char *reply_buffer;
    uint32 reply_max_len;
    uint32 reply_len;
};
typedef struct ATReply *ATReplyType;

struct ATAgent
{
    char agent_name[NAME_NUM_MAX];
    int fd;

    char *maintain_buffer;
    uint32 maintain_len;
    uint32 maintain_max;
    
    int32 lock;

    ATReplyType reply;
    int rsp_sem;

    int32 at_handler;

    #define ENTM_RECV_MAX 256
    char entm_recv_buf[ENTM_RECV_MAX];
    uint32 entm_recv_len;
    enum ReceiveMode receive_mode;
    int entm_rx_notice;
};
typedef struct ATAgent *ATAgentType;

int EntmSend(ATAgentType agent, const char *data, int len);
int EntmRecv(ATAgentType agent, char *rev_buffer, int buffer_len, int time_out);
char *GetReplyText(ATReplyType reply);
ATReplyType CreateATReply(uint32 reply_max_len);
uint IpTint(char *ipstr);
void SwapStr(char *str, int begin, int end);
char* IpTstr(uint ipint);
ATAgentType GetATAgent(const char *agent_name);
int InitATAgent(const char *agent_name, const char *device_name,  uint32 maintain_max, struct SerialCfgParam* pserial_cfg);
int ParseATReply(char* str, const char *format, ...);
void DeleteATReply(ATReplyType reply);
int ATOrderSend(ATAgentType agent, uint32 timeout, ATReplyType reply, const char *cmd_expr, ...);

#define REPLY_TIME_OUT 3000

#endif