
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
* @file:    kservicetask.c
* @brief:   create service task for system 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>

extern void ZombieTaskRecycleInit(void);
extern void InitIdleKTask(void);
void KSerciveKTaskIdle(void)
{
    InitIdleKTask();
}

void xz_KServiceKTaskRecycle()
{
    ZombieTaskRecycleInit();
}

void CreateKServiceKTask(void)
{
    /* create zombie recycle task */
    xz_KServiceKTaskRecycle();

    /* create idle task */
    KSerciveKTaskIdle();

}

