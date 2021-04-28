/*
 * Copyright (c) 2020 AIIT XUOS Lab
 * XiUOS  is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *        http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

/**
* @file:    user_fs.c
* @brief:   the priviate user api of fs for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"
#include <stdarg.h>

#define stdio 1
#define CONSOLEBUF_SIZE 128

int open(const char *path, int flags, ...){
    va_list ap;
    mode_t mode;

    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);
    return (int)(KSwitch3(KS_USER_OPEN,(uintptr_t)path,(uintptr_t)flags,(uintptr_t)mode));
}

int read(int fd, void *buf, size_t len){
    return (int)(KSwitch3(KS_USER_READ,(uintptr_t)fd,(uintptr_t)buf,(uintptr_t)len));
}

int write(int fd, const void *buf, size_t len){
    return (int)(KSwitch3(KS_USER_WRITE,(uintptr_t)fd,(uintptr_t)buf,(uintptr_t)len));
}

int close(int fd){
    return (int)(KSwitch1(KS_USER_CLOSE,(uintptr_t)fd));
}

int ioctl(int fd, int cmd, void *args){
    return (int)(KSwitch3(KS_USER_IOCTL,(uintptr_t)fd,(uintptr_t)cmd,(uintptr_t)args));
}

off_t lseek(int fd, off_t offset, int whence){
    return (off_t)(KSwitch3(KS_USER_LSEEK,(uintptr_t)fd,(uintptr_t)offset,(uintptr_t)whence));
}

int rename(const char *from, const char *to){
    return (int)(KSwitch2(KS_USER_RENAME,(uintptr_t)from,(uintptr_t)to));
}

int unlink(const char *path){
    return (int)(KSwitch1(KS_USER_UNLINK,(uintptr_t)path));
}

int stat(const char *path, struct stat *buf){
    return (int)(KSwitch2(KS_USER_STAT,(uintptr_t)path,(uintptr_t)buf));
}

int fstat(int fd, struct stat *buf){
    return (int)(KSwitch2(KS_USER_FS_STAT,(uintptr_t)fd,(uintptr_t)buf));
}

int fsync(int fd){
    return (int)(KSwitch1(KS_USER_FS_SYNC,(uintptr_t)fd));
}

int ftruncate(int fd, off_t length){
    return (int)(KSwitch2(KS_USER_FTRUNCATE,(uintptr_t)fd,(uintptr_t)length));
}

int mkdir(const char *path, mode_t mode){
    return (int)(KSwitch2(KS_USER_MKDIR,(uintptr_t)path,(uintptr_t)mode));
}

DIR *opendir(const char *path){
    return (DIR *)(KSwitch1(KS_USER_OPENDIR,(uintptr_t)path));
}

int closedir(DIR *dirp){
    return (int)(KSwitch1(KS_USER_CLOSEDIR,(uintptr_t)dirp));
}

struct dirent *readdir(DIR *dirp){
    return (struct dirent *)(KSwitch1(KS_USER_READDIR,(uintptr_t)dirp));
}

int rmdir(const char *path){
    return (int)(KSwitch1(KS_USER_RMDIR,(uintptr_t)path));
}

int chdir(const char *path){
    return (int)(KSwitch1(KS_USER_CHDIR,(uintptr_t)path));
}

char *getcwd(char *buf, size_t size){
     return (char *)(KSwitch2(KS_USER_GETCWD,(uintptr_t)buf,(uintptr_t)size ));
}

long telldir(DIR *dirp){
    return (long)(KSwitch1(KS_USER_TELLDIR,(uintptr_t)dirp));
}

void seekdir(DIR *dirp, off_t offset){
     return (void)(KSwitch2(KS_USER_SEEKDIR,(uintptr_t)dirp,(uintptr_t)offset));
}

void rewinddir(DIR *dirp){
     return (void)(KSwitch1(KS_USER_REWIND_DIR,(uintptr_t)dirp));
}

int statfs(const char *path, struct statfs *buf){
    return (int)(KSwitch2(KS_USER_STAT_FS,(uintptr_t)path,(uintptr_t)buf));
}

void Userprintf(const char *fmt, ...)
{
    if(stdio != NONE)
    {
        va_list args;
        size_t length;
        static char logbuf[CONSOLEBUF_SIZE];

        va_start(args, fmt);

        length = vsnprintf(logbuf, sizeof(logbuf) - 1, fmt, args);
        if (length > CONSOLEBUF_SIZE - 1)
            length = CONSOLEBUF_SIZE - 1;

        write(stdio, logbuf, length);        
        va_end(args);
    }
}

