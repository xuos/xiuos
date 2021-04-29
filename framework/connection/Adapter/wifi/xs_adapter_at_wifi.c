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
 * @file xs_adapter_at_wifi.c
 * @brief HFA21 wifi driver base connection framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <string.h>
#include <user_api.h>
#include <xs_adapter_at_wifi.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_agent.h>
#include <xs_adapter_def.h>

int WifiSetDown(struct AdapterAT *adapter_at);

/**
 * @description: Close wifi
 * @param padapter - wifi device pointer
 */
void WifiClose(struct Adapter *padapter)
{
    WifiSetDown((struct AdapterAT *)padapter);
}

/**
 * @description: Open wifi
 * @param padapter - wifi device pointer
 * @return success: EOK, failure: -ERROR
 */
int WifiOpen(struct Adapter *padapter)
{
    uint32 result;
    char *agent_name = "uart3_client";
    const char *device_name = WIFI_UART_NAME;

    if (InitATAgent(agent_name, device_name, 512, NULL)) {
        printf("at_client_init failed ! \n");
        result = -ERROR;
        return result;
    }

    ATAgentType at_agent = GetATAgent(agent_name);
    if (NULL == at_agent) {
        printf("GetATAgent failed ! \n");
        return -ERROR;
    }
    UserTaskDelay(5000);
    struct AdapterAT *wifi_at_adapter = (struct AdapterAT *)padapter;
    wifi_at_adapter->agent = at_agent;
}

int WifiSend(struct Adapter *padapter, const char *data, int len, bool block, int time_out, int delay, send_success cb, void *param, void* p)
{
    struct AdapterAT *wifi_at_adapter = (struct AdapterAT *)padapter;

    if (wifi_at_adapter->agent) {
        EntmSend(wifi_at_adapter->agent, data, len);
    }
}

int WifiReceive(struct Adapter *padapter, char *rev_buffer, int buffer_len, int time_out, bool block, void* p)
{
    printf("wifi receive waiting ... \n");

    struct AdapterAT *wifi_at_adapter = (struct AdapterAT *)padapter;
    if (wifi_at_adapter->agent) {
        return EntmRecv(wifi_at_adapter->agent, rev_buffer, buffer_len, 40000);
    } else {
        printf("Can not find agent \n");
	}
}

uint32 WifiInitAtCmd(ATAgentType at_agent)
{
    uint32 result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at_create_resp failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "+++");
    UserTaskDelay(100);

    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "a");

    UserTaskDelay(500);

    return result;

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

static void WifiSetUpAdapter(void *parameter)
{
    #define INIT_RETRY    5
    #define	LEN_PARA_BUF	128
    uint8 server_addr_wifi[LEN_PARA_BUF] = "192.168.1.183";
    uint8 server_port_wifi[LEN_PARA_BUF] = "12345";
    uint8 wifi_ssid[LEN_PARA_BUF] = "AIIT-Guest";
    uint8 wifi_pwd[LEN_PARA_BUF] = "";
    char cmd[LEN_PARA_BUF];

    struct AdapterAT *adapter_at = (struct AdapterAT *) parameter;

    //struct at_device_esp8266 *esp8266 = (struct at_device_esp8266 *) device->UserData;
    struct ATAgent *agent = adapter_at->agent;
    ATReplyType reply = NONE;
    x_err_t result = EOK;
    x_size_t retry_num = INIT_RETRY;

    /* wait hfa21 device startup finish */
    UserTaskDelay(5000);

    reply = CreateATReply(64);
    if (reply == NONE) {
        printf("no memory for reply create.");
        return;
    }

    while (retry_num--) {
        WifiInitAtCmd(agent);

        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "AT+FCLR\r");
        UserTaskDelay(20000);

        WifiInitAtCmd(agent);
			
        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WANN\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WSSSID=");
        strcat(cmd,wifi_ssid);
        strcat(cmd,"\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);
        
        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WSKEY=WPA2PSK,AES,");
        strcat(cmd,wifi_pwd);
        strcat(cmd,"\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WMODE=sta\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        memset(cmd,0,sizeof(cmd));
        strcat(cmd,"AT+Z\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(5000);

        /* initialize successfully  */
        result = EOK;
        break;

    __exit:
        if (result != EOK) {
            UserTaskDelay(1000);
        }
    }

    if (reply) {
        DeleteATReply(reply);
    }
}

int WifiSetDown(struct AdapterAT *adapter_at)
{
    WifiInitAtCmd(adapter_at->agent);

    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+FCLR\r");
    UserTaskDelay(20000);
}

int WifiSetUp(struct AdapterAT *adapter_at)
{
    WifiSetUpAdapter(adapter_at);

    return EOK;
}

int WifiSetAddr(struct AdapterAT *adapter_at, uint ip, uint gateway, uint netmask)
{
    #define HFA21_SET_ADDR_EXPRESSION        "+ok=%[^,],%[^,],%[^,],%[^,]\r"
    char *dhcp_mode =NULL;
    char *ip_str = NULL;
    /* role type(server/agent) */
    char *gw_str = NULL;
    char *mask_str = NULL;

    dhcp_mode = (char *) UserCalloc(1, 8);
    ip_str = (char *) UserCalloc(1, 17);
    gw_str = (char *) UserCalloc(1, 17);
    mask_str = (char *) UserCalloc(1, 17);

    WifiInitAtCmd(adapter_at->agent);

    uint32 result = EOK;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at_create_resp failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    uint32 err = ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+WANN=%s,%s,%s,%s\r", "static", IpTstr(ip), IpTstr(netmask), IpTstr(gateway));
    if (err) {
        printf("ATOrderSend（AT+PING）failed ! err = %d\n", err);
        result = -ERROR;
        goto __exit;
    }

    UserTaskDelay(2500);

    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, reply, "AT+WANN\r");
    UserTaskDelay(2500);
    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(5000);

    const char * result_buf = GetReplyText(reply);
    if (!result_buf) {
        printf("send_dhcp_at_cmd AT+ result_buf = NULL");
        result = -ERROR;
        goto __exit;
    }

    char* str = strstr(result_buf, "+ok=");
    printf("str is:%s\n",str);

    ParseATReply(str, HFA21_SET_ADDR_EXPRESSION, dhcp_mode,ip_str,mask_str,gw_str);
    printf("after configure:\n mode:%s\n ip:%s\n netmask:%s\n gateway:%s\n", dhcp_mode, ip_str, mask_str, gw_str);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

int WifiDHCP(struct AdapterAT *adapter_at, bool enable)
{
    int result = EOK;
    ATReplyType reply = NONE;
    char dhcp_status[4];
    memset(dhcp_status,0,sizeof(dhcp_status));
    if(enable)
        strcpy(dhcp_status,"on");
    else 
        strcpy(dhcp_status,"off");

    reply = CreateATReply(64);
    if (reply == NONE) {
        printf("no memory for reply struct.");
        return -ENOMEMORY;
    }

    WifiInitAtCmd(adapter_at->agent);

    char cmd[LEN_PARA_BUF];
    memset(cmd, 0, sizeof(cmd));
    strcpy(cmd, "AT+DHCPDEN=");
    strcat(cmd, dhcp_status);
    strcat(cmd, "\r");
    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, reply, cmd);
    UserTaskDelay(2500);

    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(2500);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

int WifiPing(struct AdapterAT *adapter_at, const char *destination,struct PingResult *ping_resp)
{
    char *ping_result = NONE;
    char *dst = NONE;
    ping_result = (char *) UserCalloc(1, 17);
    dst = (char *) UserCalloc(1, 17);
    strcpy(dst, destination);
    strcat(dst, "\r");

    WifiInitAtCmd(adapter_at->agent);

    uint32 result = EOK;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at_create_resp failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    printf("\\r = 0x%x, \\n = 0x%x\n", '\r', '\n');

    //ping baidu.com
    uint32 err = ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, reply, "AT+PING=%s", dst);
    if (err) {
        printf("ATOrderSend（AT+PING）failed ! err = %d\n", err);
        result = -ERROR;
        goto __exit;
    }

    UserTaskDelay(2500);

    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(2000);

    const char * result_buf = GetReplyText(reply);
    if (!result_buf) {
        printf("send_dhcp_at_cmd AT+ result_buf = NULL");
        result = -ERROR;
        goto __exit;
    }

    char* str = strstr(result_buf, "+ok=");
    printf("str is:%s\n",str);

    ParseATReply(str, "+ok=%s\r", ping_result);

    printf("ping www.baidu.com(36.152.44.95) result is:%s\n", ping_result);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

int WifiNetstat(struct AdapterAT *adapter_at)
{
    #define HFA21_NETSTAT_RESP_SIZE         320
    #define HFA21_NETSTAT_TYPE_SIZE         10
    #define HFA21_NETSTAT_IPADDR_SIZE       17
    #define HFA21_WANN_EXPRESSION        "+ok=%[^,],%[^,],%[^,],%[^,]\r"
    #define HFA21_LANN_EXPRESSION        "+ok=%[^,],%[^,]\r"
    #define HFA21_WMODE_EXPRESSION       "+ok=%s\r"

    ATReplyType reply = NULL;
    struct ATAgent *agent = adapter_at->agent;
    uint32 result;
    char * result_buf = NULL;
    char * str = NULL;

    /* sta/ap */
    char *work_mode = NULL;
    /* dhcp/static */
    char *ip_mode = NULL;
    char *local_ipaddr = NULL;
    char *gateway = NULL;
    char *netmask = NULL;
    local_ipaddr = (char *) UserCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    gateway = (char *) UserCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    netmask = (char *) UserCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    work_mode = (char *) UserCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    ip_mode = (char *) UserCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);

    reply = CreateATReply(HFA21_NETSTAT_RESP_SIZE);
    if (reply == NULL) {
        //printf("no memory for reply create.", device->name);
        goto __exit;
    }

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "+++");
    UserTaskDelay(100);

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "a");
    UserTaskDelay(2500);

    if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WMODE\r") < 0) {
        goto __exit;
    }

    UserTaskDelay(2500);

    result_buf = GetReplyText(reply);
    if(!result_buf) {
        printf("send_dhcp_at_cmd AT+ result_buf = NULL");
        result = -ERROR;
        goto __exit;
    }

    str = strstr(result_buf, "+ok=");
    printf("str is:%s\n",str);
    /* parse the third line of response data, get the network connection information */
    ParseATReply(str, HFA21_WMODE_EXPRESSION, work_mode);

    if (work_mode[0]=='S') {
        if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WANN\r") < 0) {
            goto __exit;
        }

        UserTaskDelay(2500);

        result_buf = GetReplyText(reply);
        if (!result_buf) {
            printf("send_dhcp_at_cmd AT+ result_buf = NULL");
            result = -ERROR;
            goto __exit;
        }

        str = strstr(result_buf, "+ok=");
        printf("str is:%s\n",str);
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_WANN_EXPRESSION, ip_mode, local_ipaddr, netmask, gateway);
    } else {
        if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+LANN\r") < 0) {
            goto __exit;
        }

        UserTaskDelay(2500);

        result_buf = GetReplyText(reply);
        if (!result_buf) {
            printf("send_dhcp_at_cmd AT+ result_buf = NULL");
            result = -ERROR;
            goto __exit;
        }

        str = strstr(result_buf, "+ok=");
        printf("str is:%s\n",str);
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_LANN_EXPRESSION, local_ipaddr, netmask);
    }

    ATOrderSend(adapter_at->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(2500);

    printf("work mode: %s\n", work_mode);
    if (work_mode[0]=='S')
        printf("ip mode: %s\nlocal ip: %s\nnetmask: %s\ngateway: %s\n", ip_mode, local_ipaddr, netmask, gateway);
    else
        printf("local ip: %s\nnetmask: %s\n", local_ipaddr, netmask);

    return EOK;

__exit:
    if (reply)
        DeleteATReply(reply);
    if (local_ipaddr)
        UserFree(local_ipaddr);
    if (netmask)
        UserFree(netmask);
    if (gateway)
        UserFree(gateway);
    if (work_mode)
        UserFree(work_mode);
}
