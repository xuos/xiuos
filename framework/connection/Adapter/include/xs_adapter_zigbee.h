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
* @file:    xs_adapter_zigbee.h
* @brief:   head file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/25
*
*/
#ifndef XS_ADAPTER_ZIGBEE_H
#define XS_ADAPTER_ZIGBEE_H
#include "xs_adapter.h"

struct AdapterZigbee {
    struct Adapter parent; /* inherit from Adapter */
    const char * name; /* name of the adapter instance */

    const char * device_type; /* type of the adapter instance */


};
typedef struct AdapterZigbee *adapterZigbee_t;

int ZigbeeOpen(struct Adapter *padapter);
int ZigbeeSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved);
int ZigbeeReceive(struct Adapter *padapter, char* rev_buffer, int buffer_len,int time_out, bool block, void* reserved);
void ZigbeeClose(struct Adapter *padapter);


#endif
