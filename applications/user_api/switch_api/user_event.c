/*
 * Copyright (c) 2020 AIIT XUOS Lab
 * XiUOS  is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *        http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

/**
* @file:    user_event.c
* @brief:   the priviate user api of event for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"

/**
 * This function will create a event.
 *
 * @param flag the trigger way of event.
 *
 * @return id
 */
EventIdType UserEventCreate(uint8_t flag){
    return  (EventIdType)KSwitch1(KS_USER_EVENT_CREATE,(uintptr_t)flag );
}

/**
 * This function will delete a event.
 *
 * @param event the id number of event.
 *
 * @return 
 */
void UserEventDelete(EventIdType event){
    KSwitch1(KS_USER_EVENT_DELETE,(uintptr_t)event);
}

/**
 * This function will trigger the event
 *
 * @param event the id number of event
 * @param set trigger way & events flag
 *
 * @return EOK on success.
 */
x_err_t UserEventTrigger(EventIdType event, uint32_t set){
    return  (x_err_t)KSwitch2(KS_USER_EVENT_TRIGGER,(uintptr_t)event, (uintptr_t)set );
}

/**
 * This function will get the event and process this event
 *
 * @param event the id number of event
 * @param set  events flag
 * @param option trigger way
 * @param wait_time timeout
 * @param Recved event processed flag
 *
 * @return EOK on success.
 */
x_err_t UserEventProcess(EventIdType event, uint32_t set, uint8_t option, 
                         int32_t   wait_time, uint32_t *Recved){
    return  (x_err_t)KSwitch5(KS_USER_EVENT_PROCESS,(uintptr_t)event, (uintptr_t)set,  (uintptr_t)option, (uintptr_t)wait_time, (uintptr_t)Recved);
}

