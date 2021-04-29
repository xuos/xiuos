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
 * @file wifi_demo.c
 * @brief Demo for wifi function
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <string.h>
#include <user_api.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_wifi.h>

void SendWiftMsg(int argc, char *argv[])
{
    char wifi_msg[128];
    if (argc >= 1) {
        memset(wifi_msg, 0, 128);
        strncpy(wifi_msg, argv[1], strlen(argv[1]));
        printf("SendWiftMsg(%s).\n", wifi_msg);
    }

    struct AdapterAT *at_adapter = ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }
    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    at_adapter->parent.done.NetAiitSend(&at_adapter->parent, wifi_msg, strlen(wifi_msg), true, 1000, 0, NULL, NULL, NULL);
}

void RecvWifiMsg()
{
    char wifi_recv_msg[128];
    memset(wifi_recv_msg, 0, sizeof(wifi_recv_msg));
    struct AdapterAT *at_adapter = ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    while (1) {
        memset(wifi_recv_msg, 0, sizeof(wifi_recv_msg));
        if (EOK == at_adapter->parent.done.NetAiitReceive(&at_adapter->parent, wifi_recv_msg, 128, 40000, true, NULL)) {
            printf("wifi_recv_msg (%s)\n", wifi_recv_msg);
        } else {
            printf("wifi_recv_msg failed .\n");
        }
    }
}

void SetAddrWifi()
{
    struct AdapterAT* at_adapter =  ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    printf("Waiting for msg...\n");

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    if(EOK != at_adapter->atdone.ATOperateAddr(at_adapter, IpTint("10.10.100.222"), IpTint("255.255.255.0"), IpTint("255.255.255.0")))
        printf("WifiSetAddr failed \n");
}

void DhcpWifi()
{
    struct AdapterAT* at_adapter =  ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    printf("Waiting for msg...\n");

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    if(EOK != at_adapter->atdone.ATOperateDHCP(at_adapter,1))
        printf("WifiDHCP failed \n");
}

void PingWifi()
{
    char wifi_recv_msg[128];
    memset(wifi_recv_msg, 0, sizeof(wifi_recv_msg));

    struct AdapterAT *at_adapter = ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    printf("Waiting for msg...\n");
    struct PingResult result;
    //www.baidu.com
    char *ip_str = "36.152.44.95";

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    at_adapter->atdone.ATPing(at_adapter, ip_str, &result);
}

void SetUpWifi()
{
    char wifi_recv_msg[128];
    memset(wifi_recv_msg, 0, sizeof(wifi_recv_msg));
    
    struct AdapterAT* at_adapter =  ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    printf("Waiting for msg...\n");
    struct PingResult result; 

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    if (EOK == at_adapter->atdone.ATOperateUp(at_adapter)) {
        printf("WifiSetUp success (%s)\n", result.ip_addr.ipv4);
    } else {
        printf("WifiSetUp failed \n");
    }
}

void NetstatWifi()
{
    struct AdapterAT* at_adapter =  ATAdapterFind(WIFI_ADAPTER_ID);
    if (!at_adapter) {
        printf("ATAdapterFind failed .\n");
    }

    printf("Waiting for msg...\n");

    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    if (EOK != at_adapter->atdone.ATNetstat(at_adapter))
        printf("WifiNetstat failed \n");
}

void AtTestCmdWifi(int argc, char *argv[])
{
    char cmd[64];
    if (argc >= 1) {
        memset(cmd, 0, sizeof(cmd));
        strncpy(cmd, argv[1], strlen(argv[1]));
        printf("AT cmd send(%s).\n", cmd);
    }

    strcat(cmd,"\r");
    struct AdapterAT* at_adapter =  ATAdapterFind(WIFI_ADAPTER_ID);
    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    printf("Waiting for msg...\n");

    ATOrderSend(at_adapter->agent, REPLY_TIME_OUT, NULL, "+++");
    UserTaskDelay(100);

    ATOrderSend(at_adapter->agent, REPLY_TIME_OUT, NULL, "a");

    UserTaskDelay(2500);

    ATOrderSend(at_adapter->agent,REPLY_TIME_OUT, NULL,cmd);
    UserTaskDelay(2500);

    ATOrderSend(at_adapter->agent,REPLY_TIME_OUT, NULL,"AT+Z\r");
    UserTaskDelay(5000);
}
