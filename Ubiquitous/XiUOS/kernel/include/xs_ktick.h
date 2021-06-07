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
* @file:    xs_ktick.h
* @brief:   function declaration of kernel tick
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/2
*
*/

#ifndef XS_KTICK_H
#define XS_KTICK_H

#include <xs_base.h>

x_ticks_t CurrentTicksGain(void);
void TickAndTaskTimesliceUpdate(void);
x_ticks_t  CalculteTickFromTimeMs(uint32 ms);
uint32 CalculteTimeMsFromTick(x_ticks_t ticks);

#endif
