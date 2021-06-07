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
* @file:    user_mutex.c
* @brief:   the priviate user api of mutex for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"

/**
 * a mutex will be inited in static way,then this mutex will be inserted to the manage list
 *
 * @param mutex the mutex descriptor
 * @param name mutex name
 * @param flag mutex flag
 * 
 * @return EOK on success
 *
 */
int32_t UserMutexCreate(){
   return  (int32_t)KSwitch0(KS_USER_MUTEX_CREATE);
}

/**
 * a dynamic mutex will be deleted from the manage list
 *
 * @param mutex mutex descriptor
 *
 */
void UserMutexDelete(int32_t mutex){
    KSwitch1(KS_USER_MUTEX_DELETE,(uintptr_t)mutex);
}

/**
 * a mutex will be taken when mutex is available
 *
 * @param mutex mutex descriptor
 * @param msec the time needed waiting
 * 
 * @return EOK on success;ERROR on failure
 *
 */
int32_t UserMutexObtain(int32_t mutex, int32_t wait_time){
    return  (int32_t)KSwitch2(KS_USER_MUTEX_OBTAIN,(uintptr_t)mutex, (uintptr_t)wait_time);
}

/**
 * release the mutex and resume corresponding suspended task
 *
 * @param mutex mutex descriptor
 *
 * @return EOK on success;ERROR on failure
 */
int32_t UserMutexAbandon(int32_t mutex){
    return  (int32_t)KSwitch1(KS_USER_MUTEX_ABANDON,(uintptr_t)mutex);
}

