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
 * @file xs_adapter_manager.c
 * @brief manage adapter list
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include "xs_adapter_manager.h"
#include <xs_klist.h>
#include <string.h>
#include <xs_adapter_at.h>
#include <xs_adapter.h>
#ifdef CONNECTION_COMMUNICATION_ZIGBEE
#include <xs_adapter_zigbee.h>
#endif
#ifdef CONNECTION_COMMUNICATION_LORA
#include <xs_adapter_lora.h>
#endif


// Zigbee Adapter List
#ifdef CONNECTION_COMMUNICATION_ZIGBEE
static DoubleLinklistType zigbee_adapter_list;

void ZigbeeAdapterInit()
{
    InitDoubleLinkList(&zigbee_adapter_list);
}

void ZigbeeAdapterRegister(adapter_t padapter)
{
    DoubleLinkListInsertNodeAfter(&zigbee_adapter_list, &(padapter->link));
}

void* ZigbeeAdapterFind(char* name)
{
    DoubleLinklistType* pnode = NONE;
    DoubleLinklistType* phead = &zigbee_adapter_list;
    struct AdapterZigbee* padapter = NONE;
    
    for(pnode = phead->node_next; pnode != phead; pnode = pnode->node_next) {
        padapter = (struct AdapterZigbee*)SYS_DOUBLE_LINKLIST_ENTRY(pnode, struct Adapter, link);
        // KPrintf("ZigbeeReceiveDemo bbb\n"); 
        if (0 == strcmp(padapter->name, name)){
            return padapter;
        }
    }

    return padapter;
}
#endif

// Lora Adapter List
#ifdef CONNECTION_COMMUNICATION_LORA
static DoubleLinklistType lora_adapter_list;

void LoraAdapterInit()
{
    InitDoubleLinkList(&lora_adapter_list);
}

void LoraAdapterRegister(adapter_t padapter)
{
    DoubleLinkListInsertNodeAfter(&lora_adapter_list, &(padapter->link));
}

void* LoraAdapterFind(char* name)
{
    DoubleLinklistType* pnode = NONE;
    DoubleLinklistType* phead = &lora_adapter_list;
    struct AdapterLora* padapter = NONE;

    for(pnode = phead->node_next; pnode != phead; pnode = pnode->node_next) {
        padapter = (struct AdapterLora*)SYS_DOUBLE_LINKLIST_ENTRY(pnode, struct Adapter, link);

        if (0 == strcmp(padapter->name, name)) {
            return padapter;
        }
    }

    return padapter;
}
#endif

// AT Adapter List
static DoubleLinklistType at_adapter_list;
static int at_adapter_list_inited = 0;
void ATAdapterInit()
{
    if(!at_adapter_list_inited){
        InitDoubleLinkList(&at_adapter_list);
        at_adapter_list_inited = 1;
    }
}

void ATAdapterRegister(struct AdapterAT* at_adapter)
{
    DoubleLinkListInsertNodeAfter(&at_adapter_list, &(at_adapter->link));
}

void* ATAdapterFind(uint32 adapter_id)
{
    DoubleLinklistType* pnode = NONE;
    DoubleLinklistType* phead = &at_adapter_list;
    struct AdapterAT* padapter = NONE;

    
    for(pnode = phead->node_next; pnode != phead; pnode = pnode->node_next) {
        padapter = (struct AdapterAT*)SYS_DOUBLE_LINKLIST_ENTRY(pnode, struct AdapterAT, link);

        if (padapter->at_adapter_id == adapter_id){
            return padapter;
        }
    }

    return NULL;
}

