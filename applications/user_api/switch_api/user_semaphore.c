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
* @file:    user_semaphore.c
* @brief:   the priviate user api of semphore for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"

/**
 * Create a new semaphore with specified initial value.
 * 
 * @param val initial value
 * @return id of the semaphore
 */
sem_t UserSemaphoreCreate(uint16_t val){
   return  (sem_t)KSwitch1(KS_USER_SEMAPHORE_CREATE,(uintptr_t)val);
}

/**
 * Delete a semaphore and wakeup all pending tasks on it.
 * 
 * @param id id of the semaphore to be deleted
 */
x_err_t UserSemaphoreDelete(sem_t sem){
    return  (x_err_t)KSwitch1(KS_USER_SEMAPHORE_DELETE,(uintptr_t)sem);
}

/**
 * Obtain a semaphore when its value is greater than 0; pend on it otherwise.
 * 
 * @param id id of the semaphore to be obtained
 * @param msec wait time in millisecond
 * @return EOK on success, error code on failure
 */
x_err_t UserSemaphoreObtain(sem_t sem, int32_t wait_time){
    return  (x_err_t)KSwitch2(KS_USER_SEMAPHORE_OBTAIN,(uintptr_t)sem,(uintptr_t)wait_time);
}

/**
 * Abandon a semaphore and wakeup a pending task if any.
 * 
 * @param id id of the semaphore to be abandoned
 * @return EOK on success, error code on failure
 */
x_err_t UserSemaphoreAbandon(sem_t sem){
    return  (x_err_t)KSwitch1(KS_USER_SEMAPHORE_ABANDON,(uintptr_t)sem);
}

/**
 * Set the value of a semaphore, wakeup all pending tasks if new value is positive.
 * 
 * @param id id of the semaphore for which to set value
 * @param val new value
 * @return EOK on success, error code on failure
 */
x_err_t UserSemaphoreSetValue(sem_t sem, uint16_t val){
    return  (x_err_t)KSwitch2(KS_USER_SEMAPHORE_SETVALUE,(uintptr_t)sem,(uintptr_t)val);
}
