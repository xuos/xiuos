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
 * @file xs_adapter_manager.h
 * @brief manager adapter list
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#ifndef ADAPTER_MANAGER_H_
#define ADAPTER_MANAGER_H_
#include "xs_adapter.h"
#include <xs_adapter_at.h>

void LoraAdapterInit();
void LoraAdapterRegister(adapter_t padapter);
void* LoraAdapterFind(char* name);


void ATAdapterInit();
void ATAdapterRegister(struct AdapterAT* at_adapter);
void* ATAdapterFind(uint32 adapter_id);

void ZigbeeAdapterInit();
void ZigbeeAdapterRegister(adapter_t padapter);
void* ZigbeeAdapterFind(char* name);

void BluetoothAdapterInit();
void BluetoothAdapterRegister(adapter_t padapter);
void* BluetoothAdapterFind(char* name);

#endif