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
* @file:    crt.h
* @brief:   head file for crt.c
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/
#ifndef CRT_H_
#define CRT_H_

void *operator new(size_t size);
void *operator new[](size_t size);

void operator delete(void * ptr);
void operator delete[](void *ptr);

#endif