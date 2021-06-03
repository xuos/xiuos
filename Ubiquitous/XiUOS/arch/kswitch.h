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

#ifndef __KSWITCH_H__
#define __KSWITCH_H__

#ifdef SEPARATE_COMPILE

#if defined(ARCH_RISCV)
#include "risc-v/shared/kswitch.h"
#endif

#if defined(ARCH_ARM)
#include "arm/cortex-m4/kswitch.h"
#endif

#endif

#endif