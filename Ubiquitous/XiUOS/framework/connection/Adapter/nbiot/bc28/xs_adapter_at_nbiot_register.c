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
 * @file xs_adapter_at_nbiot_register.c
 * @brief BC28 nbiot driver register to connection framework
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.22
 */

#include <xs_adapter_manager.h>
#include <xs_adapter_at_nbiot.h>

const struct AdapterDone bc28_done =
{
    NONE,
    NbiotOpen,
    NONE,
    NbiotSend,
    NONE,
    NONE,
};

const struct ATDone bc28_at_done =
{
    NONE,
    NONE,
    NONE,
    NONE,
    NONE,
    NONE,
    NONE,
    NONE,
    NBIoTSocketCreate,
    NBIoTSocketConnect,
    NBIoTSocketClose,
};

/**
 * @description: Register nbiot device
 * @return success: EOK, failure: ERROR
 */
int RegisterAdapterNBIoT(void)
{
    static struct AdapterNBIoT nbiot_adapter;

    struct AdapterAT *nbiot_at_adapter = (struct AdapterAT *)&nbiot_adapter;
    struct Adapter *adapter = (struct Adapter *)&nbiot_adapter;

    nbiot_adapter.parent.atdone = bc28_at_done;
    nbiot_adapter.parent.parent.done = bc28_done;

    nbiot_at_adapter->at_adapter_id = NBIOT_ADAPTER_ID;

    ATAdapterInit();
    ATAdapterRegister(nbiot_at_adapter);

    return 0;
}