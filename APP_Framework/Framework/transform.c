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
 * @file transform.c
 * @brief support to transform the application interface from private api to posix api
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.06.07
 */

#include "transform.h"

//for test
#define XIUOS_OS

#ifdef XIUOS_OS
/************************Kernel Posix Transform***********************/


/************************Driver Posix Transform***********************/
int PrivOpen(const char *path, int flags, ...)
{
    return open(path, flags, ...);
}

int PrivClose(int fd, void *buf, size_t len)
{
    return close(fd);
}

int PrivRead(int fd, void *buf, size_t len)
{    
    return read(fd, buf, len);
}

int PrivWrite(int fd, const void *buf, size_t len)
{   
    return write(fd, buf, len);
}

static int PrivSerialIoctl(int fd, void *args)
{
    struct SerialDataCfg *serial_cfg = (struct SerialDataCfg *)args;

    return ioctl(fd, OPE_INT, &serial_cfg);
}

int PrivIoctl(int fd, int cmd, void *args)
{
    struct PrivIoctlCfg *ioctl_cfg = (struct PrivIoctlCfg *)args;
    
    switch (ioctl_cfg->ioctl_driver_type)
    {
    case SERIAL_TYPE:
        PrivSerialIoctl(fd, ioctl_cfg->args);
        break;
    default:
        break;
    }
}

#endif

