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

#ifndef CPUPORT_H__
#define CPUPORT_H__

#include <xsconfig.h>

#ifdef ARCH_CPU_64BIT
#define StoreD                   sd
#define LoadD                    ld
#define FSubDS                   fsub.d
#define RegLength                8
#define StoreDS                  "sd"
#define LoadDS                   "ld"
#define RegLengthS               "8"
#else
#define StoreD                   sw
#define LoadD                    lw
#define FSubDS                   fsub.s
#define RegLength                4
#define StoreDS                  "sw"
#define LoadDS                   "lw"
#define RegLengthS               "4"
#endif

#endif
