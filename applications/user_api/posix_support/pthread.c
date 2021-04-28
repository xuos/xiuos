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
* @file:    pthread.c
* @brief:   posix api of pthread
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "include/pthread.h"

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg)
{
    int ret ;
    int pid ;
    utask_x task ;
    task.func_entry = start_routine ;
    task.func_param = arg ;
    memcpy(task.name , "utask", 6);
    task.prio = 20 ;
    task.stack_size = 1024 ;

    pid = UserTaskCreate(task);
    if (pid < 0)
      return -1 ;
    
    ret = UserTaskStartup(pid);
    return ret;

}

void pthread_exit(void *value_ptr){
    //todo add exit value
    UserTaskQuit();
}

pthread_t pthread_self(void){
    
    pthread_t pthread ;
    pthread = UserGetTaskID();
    return pthread;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
    //add syscall
    return 0;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return (int)(t1 == t2);
}

int pthread_cancel(pthread_t thread)
{
    return -1;
}

void pthread_testcancel(void)
{
    return;
}

int pthread_setcancelstate(int state, int *oldstate)
{
    return -1;
}

int pthread_setcanceltype(int type, int *oldtype)
{
    return -1;
}

int pthread_kill(pthread_t thread, int sig)
{
    /* This api should not be used, and will not be supported */
    return -1;
}


