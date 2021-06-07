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
 * @file xs_AdapterAT_ethernet_register.c
 * @brief Structure and function declarations of the ethernet register
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <xs_adapter_at_ethernet.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_agent.h>
#include <user_api.h>
#include <stdlib.h>


const struct AdapterDone EthernetAdapterDone =
{
    EthernetClose,
    EthernetOpen,
    NULL,
    EthernetSend,
    EthernetReceive,
    NULL,
};

const struct ATDone EthernetATDone =
{
    EthernetSetUp,
    NULL,
    NULL,
    NULL,
    EthernetDHCP,
    EthernetPing,
    EthernetNetstat,
    NULL,
};

int RegisterAdapterEthernet(void)
{
    struct AdapterEthernet *ethernet_adapter = malloc(sizeof(struct AdapterEthernet));
    if (ethernet_adapter == NULL){
        printf("out of memory\n");
        return ERROR;
    }

    struct AdapterAT *ethernetAT_adapter = (struct AdapterAT *)ethernet_adapter;
    struct Adapter *adapter = (struct Adapter *)ethernet_adapter;

    ethernet_adapter->parent.atdone = EthernetATDone;
    ethernet_adapter->parent.parent.done = EthernetAdapterDone;

    ethernetAT_adapter->at_adapter_id = ETHERNET_ADAPTER_ID;

    ATAdapterInit();
    ATAdapterRegister(ethernetAT_adapter);

    return EOK;
}