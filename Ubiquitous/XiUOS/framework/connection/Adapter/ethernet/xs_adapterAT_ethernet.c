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
 * @file xs_AdapterAT_ethernet.c
 * @brief Structure and function declarations of the connection ethernet
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */
#include <xs_adapter_at_ethernet.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_agent.h>
#include <xs_adapter_def.h>
#include <user_api.h>
#include <string.h>

/**
 * @description: Close ethernet
 * @param padapter - ethernet device pointer
 */
void EthernetClose(struct Adapter *padapter)
{
}

/**
 * @description: open ethernet
 * @param padapter - ethernet device pointer
 */
int EthernetOpen(struct Adapter *padapter)
{
    char *agent_name = "uart3_client";
    const char *device_name = ETHERNET_UART_NAME;

    uint32 result;
    if (InitATAgent(agent_name, device_name, 512, NULL)){
        printf("InitATAgent failed ! \n");
        result = -ERROR;
        return result;
    }

    ATAgentType at_agent = GetATAgent(agent_name);
    if (NULL == at_agent){
        printf("GetATAgent failed ! \n");
        return -ERROR;
    }
    UserTaskDelay(5000);
    struct AdapterAT *ethernetAT_adapter = (struct AdapterAT *)padapter;
    ethernetAT_adapter->agent = at_agent;
    return EOK;
}

int EthernetSend(struct Adapter *padapter, const char *data, int len, bool block, int time_out, int delay, send_success cb, void *param, void* p)
{
    struct AdapterAT *ethernetAT_adapter = (struct AdapterAT *)padapter;

    if (ethernetAT_adapter->agent){
        EntmSend(ethernetAT_adapter->agent, data, len);
    }
}

int EthernetReceive(struct Adapter *padapter, char *rev_buffer, int buffer_len, int time_out, bool block, void* p)
{
    printf("ethernet receive waiting ... \n");

    struct AdapterAT *ethernetAT_adapter = (struct AdapterAT *)padapter;
    if (ethernetAT_adapter->agent){
        if (EntmRecv(ethernetAT_adapter->agent, rev_buffer, buffer_len, 40000))
            printf("EntmRecv failed ! \n");
    }else{
        printf("Can not find agent \n");
	}
}

uint32 EthernetInitAtCmd(ATAgentType at_agent)
{
    uint32 result;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply){
        printf("CreateATReply failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "+++");
    UserTaskDelay(100);


    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "a");

    UserTaskDelay(500);

    return result;

__exit:
    if (reply)
        DeleteATReply(reply);

    return result;
}

static void EthernetSetUpAdapter(void *parameter)
{
    #define INIT_RETRY    5
    #define	LEN_PARA_BUF	128
    uint8	server_addr_wifi[LEN_PARA_BUF]="192.168.1.183";
    uint8	server_port_wifi[LEN_PARA_BUF]="12345";
    uint8	WIFI_ssid[LEN_PARA_BUF]="DDST 2";
    uint8	WIFI_pwd[LEN_PARA_BUF]="passw0rd";
    char	cmd[LEN_PARA_BUF];

    struct AdapterAT *adapterAT = (struct AdapterAT *) parameter;

    //struct at_device_esp8266 *esp8266 = (struct at_device_esp8266 *) device->UserData;
    struct ATAgent *agent = adapterAT->agent;
    ATReplyType reply = NONE;
    x_err_t result = EOK;
    x_size_t retry_num = INIT_RETRY;

    //DBG("%s device initialize start.", adapterAT->parent.);

    /* wait hfa21 device startup finish */
    UserTaskDelay(5000);

    reply = CreateATReply(64);
    if (reply == NONE){
        printf("no memory for reply create.");
        return;
    }

    while (retry_num--){
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "+++");
        UserTaskDelay(100);

        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "a");
        UserTaskDelay(2500);

        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "AT+FCLR\r");
        UserTaskDelay(30000);

        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "+++");
        UserTaskDelay(100);

        ATOrderSend(agent, REPLY_TIME_OUT, NULL, "a");
        UserTaskDelay(2500);
			
        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WSSSID=");
        strcat(cmd,WIFI_ssid);
        strcat(cmd,"\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);
        
        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WSKEY=WPA2PSK,AES,");
        strcat(cmd,WIFI_pwd);
        strcat(cmd,"\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+WMODE=sta\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);
        

        memset(cmd,0,sizeof(cmd));
        strcpy(cmd,"AT+NETP=TCP,CLIENT,");
        strcat(cmd,server_port_wifi);
        strcat(cmd,",");
        strcat(cmd,server_addr_wifi);
        strcat(cmd,"\r");
        // strcpy(cmd,"AT+NETP\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        memset(cmd,0,sizeof(cmd));
        strcat(cmd,"AT+Z\r");
        ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
        UserTaskDelay(2500);

        /* initialize successfully  */
        result = EOK;
        break;

    __exit:
        if (result != EOK)
            UserTaskDelay(1000);
    }

    if (reply)
        DeleteATReply(reply);
}

int EthernetSetUp(struct AdapterAT *adapterAT)
{
    EthernetSetUpAdapter(adapterAT);

    return EOK;
}

int EthernetDHCP(struct AdapterAT *adapterAT, bool enable)
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
    if (reply == NONE){
        printf("no memory for reply struct.");
        return -ENOMEMORY;
    }

    /* send dhcp set commond "AT+CWDHCP_CUR=<mode>,<en>" and wait response */
    if (ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, "AT+DHCPDEN=%s", dhcp_status) < 0){
        result = -ERROR;
        goto __exit;
    }


__exit:
    if (reply)
        DeleteATReply(reply);

    return result;
}

int EthernetPing(struct AdapterAT *adapterAT, const char *destination,struct PingResult *ping_resp)
{
    char *ping_result = NONE;
    ping_result = (char *) UserCalloc(1, 17);

    EthernetInitAtCmd(adapterAT->agent);

    uint32 result = EOK;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply){
        printf("CreateATReply failed ! \n");
        result = -ERROR;
        goto __exit;
    }

    printf("\\r = 0x%x, \\n = 0x%x\n", '\r', '\n');

    //ping baidu.com
    uint32 err = ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, reply, "AT+PING=%s", "192.168.250.240\r");
    if (err){
        printf("at_obj_exec_cmd（AT+PING）failed ! err = %d\n", err);
        result = -ERROR;
        goto __exit;
    }

    UserTaskDelay(2500);

    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(2000);


    const char * result_buf = GetReplyText(reply);
    if(!result_buf){
        printf("send_dhcp_at_cmd AT+ result_buf = NULL");
        result = -ERROR;
        goto __exit;
    }

    char* str = strstr(result_buf, "+ok=");
    printf("str is:%s\n",str);

    ParseATReply(str, "+ok=%s\r", ping_result);

    printf("ping result is:%s\n", ping_result);

__exit:
    if (reply)
        DeleteATReply(reply);
    return result;
}

int EthernetNetstat(struct AdapterAT *adapterAT)
{
#define HFA21_NETSTAT_RESP_SIZE         320
#define HFA21_NETSTAT_TYPE_SIZE         10
#define HFA21_NETSTAT_IPADDR_SIZE       17
#define HFA21_NETP_EXPRESSION        "+ok=%[^,],%[^,],%d,%s\r"
#define HFA21_WANN_EXPRESSION        "+ok=%[^,],%[^,],%[^,],%[^,]\r"
#define HFA21_LANN_EXPRESSION        "+ok=%[^,],%[^,]\r"
#define HFA21_WMODE_EXPRESSION       "+ok=%s\r"

    ATReplyType reply = NULL;
    struct ATAgent *agent = adapterAT->agent;
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
    if (reply == NULL)
        goto __exit;

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "+++");
    UserTaskDelay(100);

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "a");
    UserTaskDelay(2500);

    if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WMODE\r") < 0)
        goto __exit;

    UserTaskDelay(2500);

    result_buf = GetReplyText(reply);
    if(!result_buf){
        printf("send_dhcp_at_cmd AT+ result_buf = NULL");
        result = -ERROR;
        goto __exit;
    }

    str = strstr(result_buf, "+ok=");
    printf("str is:%s\n",str);
    /* parse the third line of response data, get the network connection information */
    ParseATReply(str, HFA21_WMODE_EXPRESSION, work_mode);

    if(work_mode[0]=='S'){
        if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WANN\r") < 0)
            goto __exit;

        UserTaskDelay(2500);

        result_buf = GetReplyText(reply);
        if(!result_buf){
            printf("send_dhcp_at_cmd AT+ result_buf = NULL");
            result = -ERROR;
            goto __exit;
        }

        str = strstr(result_buf, "+ok=");
        printf("str is:%s\n",str);
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_WANN_EXPRESSION, ip_mode, local_ipaddr, netmask, gateway);
    }else{
        if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+LANN\r") < 0)
            goto __exit;

        UserTaskDelay(2500);

        result_buf = GetReplyText(reply);
        if(!result_buf){
            printf("send_dhcp_at_cmd AT+ result_buf = NULL");
            result = -ERROR;
            goto __exit;
        }

        str = strstr(result_buf, "+ok=");
        printf("str is:%s\n",str);
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_LANN_EXPRESSION, local_ipaddr, netmask);
    }

    ATOrderSend(adapterAT->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    UserTaskDelay(2500);

    printf("work mode: %s\n", work_mode);
    if(work_mode[0]=='S')
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