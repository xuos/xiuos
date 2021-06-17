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

static DoublelistType adapter_list;

static int adapter_list_lock;

/**
 * @description: Init adapter framework
 * @return 0
 */
int AdapterFrameworkInit(void)
{
    int ret = 0;
    AppInitDoubleList(&adapter_list);

    ret = PrivMutexCreate(&adapter_list_lock, 0);
    if(ret < 0) {
        printf("AdapterFrameworkInit mutex create failed.\n");
    }

    return 0;
}

/**
 * @description: Find adapter device by name
 * @param name - name string
 * @return adapter device pointer
 */
struct Adapter *AdapterDeviceFindByName(const char *name)
{
    struct Adapter *ret = NULL;
    struct DoublelistNode *node;

    if (NULL == name)
        return NULL;

    PrivMutexObtain(&adapter_list_lock);
    DOUBLE_LIST_FOR_EACH(node, &adapter_list) {
        struct Adapter *adapter = CONTAINER_OF(node,
                struct Adapter, link);
        if (0 == strncmp(adapter->name, name, NAME_NUM_MAX)) {
            ret = adapter;
            break;
        }
    }
    PrivMutexAbandon(&adapter_list_lock);

    return ret;
}

/**
 * @description: Register the adapter to the linked list
 * @param adapter - adapter device pointer
 * @return success: 0 , failure: -1
 */
int AdapterDeviceRegister(struct Adapter *adapter)
{
    if (NULL == adapter )
        return -1;

    if (NULL != AdapterDeviceFindByName(adapter->name)) {
        printf("%s: sensor with the same name already registered\n", __func__);
        return -1;
    }

    PrivMutexObtain(&adapter_list_lock);
    AppDoubleListInsertNodeAfter(&adapter_list, &adapter->link);
    PrivMutexAbandon(&adapter_list_lock);

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
    PrivMutexObtain(&adapter_list_lock);
    AppDoubleListRmNode(&adapter->link);
    PrivMutexAbandon(&adapter_list_lock);

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
        if (NULL == priv_done->open)
            return 0;
        
        result = priv_done->open(adapter);
        if (0 == result) {
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
        if (NULL == ip_done->open)
            return 0;
        
        result = ip_done->open(adapter);
        if (0 == result) {
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
        if (NULL == priv_done->close)
            return 0;
        
        result = priv_done->close(adapter);
        if (0 == result)
            printf("%s successfully closed.\n", adapter->name);
        else
            printf("Closed %s failure.\n", adapter->name);

        break;
    
    case IP_PROTOCOL:
        ip_done = (struct IpProtocolDone *)adapter->done;
        if (NULL == ip_done->close)
            return 0;
        
        result = ip_done->close(adapter);
        if (0 == result)
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

    if (PRIVATE_PROTOCOL == adapter->net_protocol) {
        struct PrivProtocolDone *priv_done = (struct PrivProtocolDone *)adapter->done; 
        
        if (NULL == priv_done->recv)
            return -1;
    
        return priv_done->recv(adapter, dst, len);
    } else if (IP_PROTOCOL == adapter->net_protocol) {
        struct IpProtocolDone *ip_done = (struct IpProtocolDone *)adapter->done;
    
        if (NULL == ip_done->recv)
            return -1;
    
        return ip_done->recv(adapter->socket, dst, len);
    } else {
        printf("AdapterDeviceRead net_protocol %d not support\n", adapter->net_protocol);
        return -1;
    }
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

    if (PRIVATE_PROTOCOL == adapter->net_protocol) {
        struct PrivProtocolDone *priv_done = (struct PrivProtocolDone *)adapter->done; 
        
        if (NULL == priv_done->send)
            return -1;
    
        return priv_done->send(adapter, src, len);
    } else if (IP_PROTOCOL == adapter->net_protocol) {
        struct IpProtocolDone *ip_done = (struct IpProtocolDone *)adapter->done;
    
        if (NULL == ip_done->send)
            return -1;
    
        return ip_done->send(adapter->socket, src, len);
    } else {
        printf("AdapterDeviceWrite net_protocol %d not support\n", adapter->net_protocol);
        return -1;
    }
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
        
    if (PRIVATE_PROTOCOL == adapter->net_protocol) {
        struct PrivProtocolDone *priv_done = (struct PrivProtocolDone *)adapter->done; 
        
        if (NULL == priv_done->ioctl)
            return -1;
    
        return priv_done->ioctl(adapter, cmd, args);
    } else if (IP_PROTOCOL == adapter->net_protocol) {
        struct IpProtocolDone *ip_done = (struct IpProtocolDone *)adapter->done;
    
        if (NULL == ip_done->ioctl)
            return -1;
    
        return ip_done->ioctl(adapter, cmd, args);
    } else {
        printf("AdapterDeviceControl net_protocol %d not support\n", adapter->net_protocol);
        return -1;
    }
}
