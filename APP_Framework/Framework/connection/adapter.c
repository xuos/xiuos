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
 * @file adapter.c
 * @brief Implement the communication adapter framework management and API
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.05.10
 */

#include <adapter.h>

static DoubleLinklistType adapter_list;

static int adapter_list_lock;

/**
 * @description: Init adapter framework
 * @return 0
 */
int AdapterFrameworkInit(void)
{
    InitDoubleLinkList(&adapter_list);

    adapter_list_lock = KMutexCreate();

    return 0;
}

/**
 * @description: Find adapter device by name
 * @param name - name string
 * @return adapter device pointer
 */
struct Adapter *AdapterDeviceFind(const char *name)
{
    struct Adapter *ret = NULL;
    struct SysDoubleLinklistNode *node;

    if (name == NULL)
        return NULL;

    UserMutexObtain(adapter_list_lock, -1);
    DOUBLE_LINKLIST_FOR_EACH(node, &adapter_list) {
        struct Adapter *adapter =CONTAINER_OF(node,
                struct Adapter, link);
        if (strncmp(adapter->name, name, NAME_NUM_MAX) == 0) {
            ret = adapter;
            break;
        }
    }
    UserMutexAbandon(adapter_list_lock);

    return ret;
}

/**
 * @description: Register the adapter to the linked list
 * @param adapter - adapter device pointer
 * @return success: 0 , failure: -1
 */
int AdapterDeviceRegister(struct Adapter *adapter)
{
    if (adapter == NULL)
        return -1;

    if (AdapterDeviceFindByName(adapter->name) != NULL) {
        printf("%s: sensor with the same name already registered\n", __func__);
        return -1;
    }

    UserMutexObtain(adapter_list_lock, -1);
    DoubleLinkListInsertNodeAfter(&adapter_list, &adapter->link);
    UserMutexAbandon(adapter_list_lock);

    return 0;
}

/**
 * @description: Unregister the adapter from the linked list
 * @param adapter - adapter device pointer
 * @return 0
 */
int AdapterDeviceUnregister(struct Adapter *adapter)
{
    if (!adapter)
        return -1;
    UserMutexObtain(adapter_list, -1);
    DoubleLinkListRmNode(&adapter->link);
    UserMutexAbandon(adapter_list);

    return 0;
}

/**
 * @description: Open adapter device
 * @param adapter - adapter device pointer
 * @return success: 0 , failure: other
 */
int AdapterDeviceOpen(struct Adapter *adapter)
{
    if (!adapter)
        return -1;

    int result = 0;

    struct IpProtocolDone *ip_done = NULL;
    struct PrivProtocolDone *priv_done = NULL;

    switch (adapter->net_protocol)
    {
    case PRIVATE_PROTOCOL:
        priv_done = (struct PrivProtocolDone *)adapter->done;
        if (priv_done->open == NULL)
            return 0;
        
        result = priv_done->open(adapter);
        if (result == 0) {
            printf("Device %s open success.\n", adapter->name);
        }else{
            if (adapter->fd) {
                PrivClose(adapter->fd);
                adapter->fd = 0;
            }
            printf("Device %s open failed(%d).\n", adapter->name, result);
        }
        break;
    
    case IP_PROTOCOL:
        ip_done = (struct IpProtocolDone *)adapter->done;
        if (ip_done->open == NULL)
            return 0;
        
        result = ip_done->open(adapter);
        if (result == 0) {
            printf("Device %s open success.\n", adapter->name);
        }else{
            if (adapter->fd) {
                PrivClose(adapter->fd);
                adapter->fd = 0;
            }
            printf("Device %s open failed(%d).\n", adapter->name, result);
        }
        break;

    default:
        break;
    }

    return result;
}

/**
 * @description: Close adapter device
 * @param adapter - adapter device pointer
 * @return success: 0 , failure: other
 */
int AdapterDeviceClose(struct Adapter *adapter)
{
    if (!adapter)
        return -1;

    int result = 0;

    struct IpProtocolDone *ip_done = NULL;
    struct PrivProtocolDone *priv_done = NULL;

    switch (adapter->net_protocol)
    {
    case PRIVATE_PROTOCOL:
        priv_done = (struct PrivProtocolDone *)adapter->done;
        if (priv_done->close == NULL)
            return 0;
        
        result = priv_done->close(adapter);
        if (result == 0)
            printf("%s successfully closed.\n", adapter->name);
        else
            printf("Closed %s failure.\n", adapter->name);

        break;
    
    case IP_PROTOCOL:
        ip_done = (struct IpProtocolDone *)adapter->done;
        if (ip_done->close == NULL)
            return 0;
        
        result = ip_done->close(adapter);
        if (result == 0)
            printf("%s successfully closed.\n", adapter->name);
        else
            printf("Closed %s failure.\n", adapter->name);
        break;

    default:
        break;
    }
    
    return result;
}

/**
 * @description: Read data from adapter
 * @param adapter - adapter device pointer
 * @param dst - buffer to save data
 * @param len - buffer length
 * @return gotten data length
 */
ssize_t AdapterDeviceRead(struct Adapter *adapter, void *dst, size_t len)
{
    if (!adapter)
        return -1;

    if (adapter->done->read == NULL)
        return -1;
    
    return adapter->done->read(adapter, dst, len);
}

/**
 * @description: Write data to adapter
 * @param adapter - adapter device pointer
 * @param src - data buffer
 * @param len - data length
 * @return length of data written
 */
ssize_t AdapterDeviceWrite(struct Adapter *adapter, const void *src, size_t len)
{
    if (!adapter)
        return -1;

    if (adapter->done->write == NULL)
        return -1;
    
    return adapter->done->write(adapter, src, len);
}

/**
 * @description: Configure adapter
 * @param adapter - adapter device pointer
 * @param cmd - command
 * @param args - command parameter
 * @return success: 0 , failure: other
 */
int AdapterDeviceControl(struct Adapter *adapter, int cmd, void *args)
{
    if (!adapter)
        return -1;

    if (adapter->done->ioctl == NULL)
        return -1;
    
    return adapter->done->ioctl(adapter, cmd, args);
}
