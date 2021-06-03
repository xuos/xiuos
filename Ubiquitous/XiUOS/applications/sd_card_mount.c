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
 * @file sd_card_mount.c
 * @brief Mount SD card when opened SDIO
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.04.19
 */

#include "user_api/switch_api/user_api.h"
#include <stdio.h>

#if defined(FS_VFS)
#include <iot-vfs.h>

/**
 * @description: Mount SD card
 * @return 0
 */
int  MountSDCard(void)
{
    if (MountFilesystem(SDIO_BUS_NAME, SDIO_DEVICE_NAME, SDIO_DRIVER_NAME, FSTYPE_FATFS, "/") == 0)
        DBG("sd card mount to '/'");
    else
        SYS_WARN("sd card mount to '/' failed!");
    
    return 0;
}
#endif
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),MountSDCard, MountSDCard,  MountSDCard );
