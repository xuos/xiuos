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
* @file connect_sdio.h
* @brief define stm32f407-st-discovery-board sdio function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/
 
#ifndef CONNECT_SDIO_H
#define CONNECT_SDIO_H

#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDIO_DMA_MODE 1

int HwSdioInit(void);

#ifdef __cplusplus
}
#endif

#endif
