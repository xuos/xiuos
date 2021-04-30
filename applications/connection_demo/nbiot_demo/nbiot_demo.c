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
 * @file nbiot_demo.c
 * @brief Demo for NBIoT function
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <string.h>
#include <user_api.h>
#include <xs_adapter_at_agent.h>
#include <xs_adapter_manager.h>
#include <xs_adapter_at_nbiot.h>

void NbiotEnable(void)
{
    RegisterAdapterNBIoT();

    struct AdapterAT* at_adapter =  ATAdapterFind(NBIOT_ADAPTER_ID);

    UserTaskDelay(5000);
    
    at_adapter->parent.done.NetAiitOpen(&at_adapter->parent);

    printf("Waiting for msg...\n");

    at_adapter->atdone.ATSocketCreate(at_adapter, 1, SOCKET_TYPE_STREAM, NET_TYPE_AF_INET);
    UserTaskDelay(1000);

    struct ADDRESS_IPV4 addr;
    addr.ipv4 = IpTint("115.236.53.226");
    at_adapter->atdone.ATSocketConnect(at_adapter, 1, addr, 8989, 0);

    int socket_fd = 1;
    int count = 0;

    while (1) {
        UserTaskDelay(1000);
        at_adapter->parent.done.NetAiitSend((struct Adapter *)at_adapter, "AB30313233", 5, 0, 0, 0, 0, 0, &socket_fd);
        count++;
        if (count == 10)
            break;
    }
}
