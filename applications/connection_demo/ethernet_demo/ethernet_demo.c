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
 * @file ethernet_demo.c
 * @brief Demo for ethernet function
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */
#include <string.h>
#include <user_api.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_ethernet.h>

static bool opened = false;

void OpenEthernetMsg()
{
    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }
}


void SendEthernetMsg(int argc, char *argv[])
{
    char ethernet_msg[128];
    if (argc >= 1){
        memset(ethernet_msg, 0, 128);
        strncpy(ethernet_msg, argv[1], strlen(argv[1]));
        printf("SendEthernetMsg(%s).\n", ethernet_msg);
    }

    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    at_adapter->parent.done.NetAiitSend(&at_adapter->parent, ethernet_msg, strlen(ethernet_msg), true, 1000, 0, NULL, NULL, NULL);
}

void RecvEthernetMsg()
{
    char ethernet_recv_msg[128];
    memset(ethernet_recv_msg, 0, sizeof(ethernet_recv_msg));
    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    while (1){
        memset(ethernet_recv_msg, 0, sizeof(ethernet_recv_msg));
        if (EOK == at_adapter->parent.done.NetAiitReceive(&at_adapter->parent, ethernet_recv_msg, 128, 40000, true, NULL))
            printf("ethernet_recv_msg (%s)\n", ethernet_recv_msg);
        else
            printf("ethernet_recv_msg failed .\n");
    }
}

void DhcpEthernet()
{
    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    printf("Waiting for msg...\n");

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    if (EOK != at_adapter->atdone.ATOperateDHCP(at_adapter, 1))
        printf("EthernetNetstat failed \n");
}

void PingEthernet()
{
    char ethernet_recv_msg[128];
    memset(ethernet_recv_msg, 0, sizeof(ethernet_recv_msg));

    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    printf("Waiting for msg...\n");
    struct ping_result result;
    char *ip_str = "192.168.250.250";

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    if (EOK == at_adapter->atdone.ATPing(at_adapter, ip_str, &result))
        printf("EthernetPing success (%s)\n", result.ip_addr.ipv4);
    else
        printf("EthernetPing failed \n");
}

void SetUpEthernet()
{
    char ethernet_recv_msg[128];
    memset(ethernet_recv_msg, 0, sizeof(ethernet_recv_msg));

    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    printf("Waiting for msg...\n");
    struct ping_result result;

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    if (EOK == at_adapter->atdone.ATOperateUp(at_adapter))
        printf("EthernetSetUp success (%s)\n", result.ip_addr.ipv4);
    else
        printf("EthernetSetUp failed \n");
}

void NetstatEthernet()
{
    struct AdapterAT *at_adapter = ATAdapterFind(ETHERNET_ADAPTER_ID);
    if (!at_adapter)
        printf("ATAdapterFind failed .\n");

    printf("Waiting for msg...\n");

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

    if (EOK != at_adapter->atdone.ATNetstat(at_adapter))
        printf("EthernetNetstat failed \n");
}

void AtTestCmdEthernet(int argc, char *argv[])
{
    char cmd[64];
    if (argc >= 1){
        memset(cmd, 0, sizeof(cmd));
        strncpy(cmd, argv[1], strlen(argv[1]));
        printf("AT cmd send(%s).\n", cmd);
    }

    strcat(cmd,"\r");
    struct AdapterAT* at_adapter =  ATAdapterFind(ETHERNET_ADAPTER_ID);

    if (!opened){
        opened = true;
        at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);
    }

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
