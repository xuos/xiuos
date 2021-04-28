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
* @file:    xs_ktask_stat.h
* @brief:   function declaration of task stat
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_KTASK_STAT_H
#define XS_KTASK_STAT_H

#include <xs_base.h>
#include<xs_ktask.h>
#include <stdbool.h>

void KTaskStateSet(KTaskDescriptorType task, uint8 stat);
void KTaskStatSetAsInit(KTaskDescriptorType task);
void KTaskStatSetAsReady(KTaskDescriptorType task);
void KTaskStatSetAsSuspend(KTaskDescriptorType task);
void KTaskStatSetAsRunning(KTaskDescriptorType task);
void KTaskStatSetAsClose(KTaskDescriptorType task);
uint8 KTaskStatGet(KTaskDescriptorType task);
bool JudgeKTaskStatIsInit(KTaskDescriptorType task);
bool JudgeKTaskStatIsReady(KTaskDescriptorType task);
bool JudgeKTaskStatIsSuspend(KTaskDescriptorType task);
bool JudgeKTaskStatIsRunning(KTaskDescriptorType task);
bool JudgeKTaskStatIsClose(KTaskDescriptorType task);

#endif


