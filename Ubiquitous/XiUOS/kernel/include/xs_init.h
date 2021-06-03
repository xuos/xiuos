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
* @file:    xs_init.h
* @brief:   init function declaration of components
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/10
*
*/

#ifndef XS_INIT_H
#define XS_INIT_H

typedef int (*InitFnType)(void);
struct InitSequenceDesc
{
	const char* fn_name;
	const InitFnType fn;
};

#ifdef KERNEL_COMPONENTS_INIT
void InitCmpts(void);
#endif


extern int VfsInit(void);
extern int WorkSysWorkQueueInit(void);
extern int FlashW25qxxSpiDeviceInit(void);
extern int FatfsInit(void);
extern int Ch376fsInit(void);
extern int LibcSystemInit(void);
extern int RtcNtpSyncInit(void);
extern int MountSDCard(void);
extern int DfsMountTable(void);
extern int userShellInit(void);
extern int Stm32SdcardMount(void);
extern int STM32USBHostRegister(void);
extern int WorkSysWorkQueueInit(void);

#endif
