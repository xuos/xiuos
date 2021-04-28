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
 * @file xs_adapterAT_client.c
 * @brief AT proxy, auto receive AT reply and transparency data
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */


#include <xs_adapter_at_agent.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <user_api.h>
#include <bus.h>

#define AT_CMD_MAX_LEN 128
#define AT_AGENT_MAX 2
static char send_buf[AT_CMD_MAX_LEN];
static uint32 last_cmd_len = 0;

static struct ATAgent at_agent_table[AT_AGENT_MAX] = {0};

uint IpTint(char *ipstr){
    if (ipstr == NULL)
        return 0;

    char *token;
    uint i = 3, total = 0, cur;

    token = strtok(ipstr, ".");

    while (token != NULL){
        cur = atoi(token);
        if (cur >= 0 && cur <= 255){
            total += cur * pow(256, i);
        }
        i--;
        token = strtok(NULL, ".");
    }

    return total;
}

void SwapStr(char *str, int begin, int end)
{
    int i, j;

    for (i = begin, j = end; i <= j; i++, j--){
        if (str[i] != str[j]){
            str[i] = str[i] ^ str[j];
            str[j] = str[i] ^ str[j];
            str[i] = str[i] ^ str[j];
        }
    }
}

char *IpTstr(uint ipint)
{
    int LEN = 16;
    char *new = (char *)malloc(LEN);
    memset(new, '\0', LEN);
    new[0] = '.';
    char token[4];
    int bt, ed, len, cur;

    while (ipint){
        cur = ipint % 256;
        sprintf(token, "%d", cur);
        strcat(new, token);
        ipint /= 256;
        if (ipint)
            strcat(new, ".");
    }

    len = strlen(new);
    SwapStr(new, 0, len - 1);

    for (bt = ed = 0; ed < len;){
        while (ed < len && new[ed] != '.'){
            ed++;
        }
        SwapStr(new, bt, ed - 1);
        ed += 1;
        bt = ed;
    }

    new[len - 1] = '\0';

    return new;
}

int ParseATReply(char *str, const char *format, ...)
{
    va_list params;
    int counts = 0;

    va_start(params, format);
    counts = vsscanf(str, format, params);
    va_end(params);

    return counts;
}

uint32 ATSprintf(int fd, const char *format, va_list params)
{
    last_cmd_len = vsnprintf(send_buf, sizeof(send_buf), format, params);
	return write(fd, send_buf, last_cmd_len);
}

int ATOrderSend(ATAgentType agent, uint32 timeout, ATReplyType reply, const char *cmd_expr, ...)
{
    if (agent == NULL){
        printf("ATAgent is null");
        return -ERROR;
    }

    agent->receive_mode = AT_MODE;

    memset(agent->maintain_buffer, 0x00, agent->maintain_max);
    agent->maintain_len = 0;

    memset(agent->entm_recv_buf, 0, ENTM_RECV_MAX);
    agent->entm_recv_len = 0;

    va_list params;
    uint32 cmd_size = 0;
    uint32 result = EOK;
    const char *cmd = NULL;

    UserMutexObtain(agent->lock, WAITING_FOREVER); 

    agent->reply = reply;

    if(agent->reply != NULL){
        reply->reply_len = 0;
        va_start(params, cmd_expr);
        ATSprintf(agent->fd, cmd_expr, params);
        va_end(params);

        if (UserSemaphoreObtain(agent->rsp_sem, timeout) != EOK){
            result = -ETIMEOUT;
            goto __out;
        }
    }else{
        va_start(params, cmd_expr);
        ATSprintf(agent->fd, cmd_expr, params);
        va_end(params);
        goto __out;
    }

__out:
    agent->reply = NULL;
    UserMutexAbandon(agent->lock); 
    agent->receive_mode = ENTM_MODE;

    return result;
}

char *GetReplyText(ATReplyType reply)
{
    return reply->reply_buffer;
}

int EntmSend(ATAgentType agent, const char *data, int len)
{
    char send_buf[128];
    memset(send_buf, 0, 128);
    agent->receive_mode = ENTM_MODE;

    memcpy(send_buf, data, len);
    memcpy(send_buf + len, "!@", 2);

	write(agent->fd, send_buf, len + 2);
}

int EntmRecv(ATAgentType agent, char *rev_buffer, int buffer_len, int time_out)
{
    UserTaskDelay(1000);

    memset(agent->entm_recv_buf, 0, ENTM_RECV_MAX);
    agent->entm_recv_len = 0;

    UserSemaphoreSetValue(agent->entm_rx_notice, 0);

    if (UserSemaphoreObtain(agent->entm_rx_notice, time_out)){
        return ERROR;
    }

    if (buffer_len < agent->entm_recv_len){
        return ERROR;
    }

    printf("EntmRecv once .\n");

    agent->entm_recv_buf[agent->entm_recv_len - 2] = '\0';
    memcpy(rev_buffer, agent->entm_recv_buf, agent->entm_recv_len - 2);

    return EOK;
}

static int GetCompleteATReply(ATAgentType agent)
{
    uint32 read_len = 0;
    char ch = 0, last_ch = 0;
    bool is_full = false;

    memset(agent->maintain_buffer, 0x00, agent->maintain_max);
    agent->maintain_len = 0;

    while (1){
        read(agent->fd, &ch, 1);

        printf(" %c(0x%x)\n", ch, ch);

        if (agent->receive_mode == ENTM_MODE){
            if (agent->entm_recv_len < ENTM_RECV_MAX){
                agent->entm_recv_buf[agent->entm_recv_len++] = ch;

                if (last_ch == '!' && ch == '@'){
                    UserSemaphoreAbandon(agent->entm_rx_notice);
                }

                last_ch = ch;
            }
            else{
                printf("entm_recv_buf is_full ...\n");
            }
        }
        else if (agent->receive_mode == AT_MODE){
            if (read_len < agent->maintain_max){
                agent->maintain_buffer[read_len++] = ch;
                agent->maintain_len = read_len;
            }else{
                printf("maintain_len is_full ...\n");
                is_full = true;
            }

            if ((ch == '\n' && last_ch == '\r')){
                if (is_full){
                    printf("read line failed. The line data length is out of buffer size(%d)!", agent->maintain_max);
                    memset(agent->maintain_buffer, 0x00, agent->maintain_max);
                    agent->maintain_len = 0;
                    return -ERROR;
                }
                printf("GetCompleteATReply get n r ...\n");
                break;
            }
            last_ch = ch;
        }
    }

    return read_len;
}

ATAgentType GetATAgent(const char *agent_name)
{
    struct ATAgent* result = NULL;
    for (int i = 0; i < AT_AGENT_MAX; i++){
        if (strcmp(at_agent_table[i].agent_name, agent_name) == 0){
            result = &at_agent_table[i];
        }
    }

    return result;
}


static int DeleteATAgent(ATAgentType agent)
{
    if (agent->lock){
        UserMutexDelete(agent->lock);
    }

    if (agent->entm_rx_notice){
        UserSemaphoreDelete(agent->entm_rx_notice);
    }

    if (agent->fd > 0){
        close(agent->fd);
    }

    if (agent->rsp_sem){
        UserSemaphoreDelete(agent->rsp_sem);
    }

    if (agent->maintain_buffer){
        free(agent->maintain_buffer);
    }

    memset(agent, 0x00, sizeof(struct ATAgent));
}

static void ATAgentReceiveProcess(void *param)
{
    ATAgentType agent = (ATAgentType)param;
    const struct at_urc *urc;

    while (1){
        if (GetCompleteATReply(agent) > 0){
            if (agent->reply != NULL){
                ATReplyType reply = agent->reply;

                agent->maintain_buffer[agent->maintain_len - 1] = '\0';

                if (agent->maintain_len < reply->reply_max_len){
                    memcpy(reply->reply_buffer, agent->maintain_buffer, agent->maintain_len);

                    reply->reply_len = agent->maintain_len;
                }
                else{
                    printf("out of memory (%d)!", reply->reply_max_len);
                }

                agent->reply = NULL;
                UserSemaphoreAbandon(agent->rsp_sem);
            }
        }
    }
}

static int ATAgentInit(ATAgentType agent)
{
    int result = EOK;
	utask_x at_utask;
    do
    {
        agent->maintain_len = 0;
        agent->maintain_buffer = (char *)malloc(agent->maintain_max);

        if (agent->maintain_buffer == NONE){
            break;
        }

        agent->entm_rx_notice = UserSemaphoreCreate(0);
        if (agent->entm_rx_notice == 0){
            break;
        }

        agent->rsp_sem = UserSemaphoreCreate(0);
        if (agent->rsp_sem == 0){
            break;
        }

        agent->lock = UserMutexCreate();
        if (agent->lock == 0){
            break;
        }

        agent->receive_mode = ENTM_MODE;

		strncpy(at_utask.name, "recv_task", strlen("recv_task"));
        at_utask.func_entry = ATAgentReceiveProcess;
        at_utask.func_param = agent;
        at_utask.stack_size = 1024;
        at_utask.prio = 18;
		
        agent->at_handler = UserTaskCreate(at_utask);

        if (agent->at_handler == 0)        {
            break;
        }

        result = EOK;
        return result;
    } while (1);

    DeleteATAgent(agent);
    result = -ERROR;
    return result;
}



static int SerialBusCheck(struct ATAgent *agent, const char *device_name, struct SerialCfgParam *pserial_cfg)
{
    int ret = EOK;
	struct SerialDataCfg data_cfg;
    memset(&data_cfg, 0, sizeof(struct SerialDataCfg));

	agent->fd = open(device_name,O_RDWR);

	data_cfg.serial_baud_rate = 57600;
    ioctl(agent->fd, OPE_INT, &data_cfg);

}

int InitATAgent(const char *agent_name, const char *device_name, uint32 maintain_max, struct SerialCfgParam *pserial_cfg)
{
    int i = 0;
    int result = EOK;
    int open_result = EOK;
    struct ATAgent *agent = NONE;

    if (GetATAgent(agent_name) != NULL) {
        return result;
    }
    
    while (i < AT_AGENT_MAX && at_agent_table[i].fd > 0) {
        i++;
    }

    if (i >= AT_AGENT_MAX) {
        printf("agent buffer(%d) is full .", AT_AGENT_MAX);
        result = -ERROR;
        return result;
    }

    agent = &at_agent_table[i];

    SerialBusCheck(agent, device_name, pserial_cfg);

    strcpy(agent->agent_name, agent_name);

    agent->maintain_max = maintain_max;

    result = ATAgentInit(agent);
    if (result == EOK)
    {
        UserTaskStartup(agent->at_handler);
    }

    return result;
}

ATReplyType CreateATReply(uint32 reply_max_len)
{
    ATReplyType reply = NULL;

    reply = (ATReplyType)malloc(sizeof(struct ATReply));
    if (reply == NULL){
        printf("no more memory\n");
        return NULL;
    }

    reply->reply_max_len = reply_max_len;

    reply->reply_buffer = (char *)malloc(reply_max_len);
    if (reply->reply_buffer == NULL){
        printf("no more memory\n");
        free(reply);
        return NULL;
    }

    return reply;
}

void DeleteATReply(ATReplyType reply)
{
    if (reply){
        if (reply->reply_buffer){
            free(reply->reply_buffer);
            reply->reply_buffer = NULL;
        }
    }

    if (reply){
        free(reply);
        reply = NULL;
    }
}



