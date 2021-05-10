/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/**
* @file fs_syscalls.c
* @brief support newlib file system
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: fs_syscalls.c
Description: support newlib file system
Others: take RT-Thread v4.0.2/components/libc/compilers/newlib/syscalls.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: Use file system functions
*************************************************/

#include <sys/errno.h>
#include <stdio.h>
#include <user_api.h>

int _close_r(struct _reent *ptr, int fd)
{
    return close(fd);
}

int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _isatty_r(struct _reent *ptr, int fd)
{
    if (fd >=0 && fd < 3)
        return 1;
    
    ptr->_errno = ENOTSUP;
    return -1;
}

int _link_r(struct _reent *ptr, const char *old, const char *new)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence)
{
    return lseek(fd, pos, whence);
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode)
{
    return open(file, flags, mode);
}

_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes)
{
    return read(fd, buf, nbytes);
}

void * _sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    return NONE;
}

int _stat_r(struct _reent *ptr, const char *file, struct stat *pstat)
{
    return stat(file, pstat);
}

int _unlink_r(struct _reent *ptr, const char *file)
{
    return unlink(file);
}

int _wait_r(struct _reent *ptr, int *status)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
    return write(fd, buf, nbytes);
}
