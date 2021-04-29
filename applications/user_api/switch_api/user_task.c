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
* @file:    user_task.c
* @brief:   the priviate user api of task for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"

/**
 *
 * This function init a user task in dynamic way .
 *
 * @param name task name
 * @param parameter task process function
 * @param parameter task arg
 * @param stack_size task stack size
 * @param priority task priority
 * @param tick task time slice
 * 
 * @return EOK on success; ENOMEMORY/EEMPTY on failure
 */
int32_t UserTaskCreate(UtaskType utask){
    return (int32_t) KSwitch5(KS_USER_TASK_CREATE,(uintptr_t)utask.name,(uintptr_t)utask.func_entry,(uintptr_t)utask.func_param,(uintptr_t)utask.stack_size,(uintptr_t)utask.prio);
}

/**
 * This function will insert task to ready queue then schedule
 *
 * @param id task id
 *
 * @return EOK on success; EINVALED on failure
 */
x_err_t UserTaskStartup(int32_t id){
    return (x_err_t)KSwitch1(KS_USER_TASK_STARTUP,(uintptr_t)id);
}

/**
 * This function will remove a dynamic task out of the task manage list.
 *
 * @param id task id
 *
 * @return EOK on success; EINVALED on failure
 */
x_err_t UserTaskDelete(int32_t id){
    return (x_err_t)KSwitch1(KS_USER_TASK_DELETE,(uintptr_t)id);
}

void UserTaskQuit(void){
    KSwitch0(KS_USER_TASK_EXECEXIT);
    return ;
}
/**
 * This function will delay current task running with some ticks.
 *
 * @param tick delay ticks
 * @return EOK on success; EINVALED/EEMPTY on failure
 */
x_err_t UserTaskDelay(int32_t ms){
    return (x_err_t)KSwitch1(KS_USER_TASK_DELAY,(uintptr_t)ms);
}

x_err_t UserGetTaskName( int32_t id, char *name){
    return (x_err_t)KSwitch2(KS_USER_GET_TASK_NAME,(uintptr_t)id, (uintptr_t)name);
}

int32_t UserGetTaskID(void){
    return (int32_t)KSwitch0(KS_USER_GET_TASK_ID);
}

uint8_t UserGetTaskStat(int32_t id){
    return (uint8_t)KSwitch1(KS_USER_GET_TASK_STAT,(uintptr_t)id );
}

#ifdef ARCH_SMP
/**
 * This function binds a task to cpu core.
 *
 * @param id task id
 * @param coreid cpu core id 
 *
 * @return EOK
 */
x_err_t UserTaskCoreCombine(int32_t id,uint8_t core_id){
    return (x_err_t)KSwitch2(KS_USER_TASK_CORE_COMBINE,(uintptr_t)id,core_id);
}
/**
 * This function unbinds a task with cpu core.
 *
 * @param id task id
 *
 * @return EOK
 */
x_err_t UserTaskCoreUnCombine(int32_t id){
    return (x_err_t)KSwitch1(KS_USER_TASK_CORE_UNCOMBINE,(uintptr_t)id);
}

uint8_t UserGetTaskCombinedCore(int32_t id){
    return (uint8_t)KSwitch1(KS_USER_GET_TASK_COMBINEED_CORE,(uintptr_t)id );
}

uint8 UserGetTaskRunningCore(int32_t id){
    return (uint8_t)KSwitch1(KS_USER_GET_TASK_RUNNING_CORE,(uintptr_t)id );
}

#endif

x_err_t UserGetTaskErrorstatus(int32_t id){
    return (x_err_t)KSwitch1(KS_USER_GET_TASK_ERROR_STATUS,(uintptr_t)id );
}

uint8_t UserGetTaskPriority(int32_t id){
    return (uint8_t)KSwitch1(KS_USER_GET_TASK_PRIORITY,(uintptr_t)id );
}
