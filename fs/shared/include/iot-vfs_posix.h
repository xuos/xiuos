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

#ifndef _IOT_VFS_POSIX_H_
#define _IOT_VFS_POSIX_H_

#include <iot-vfs.h>

typedef void DIR;

#if defined(LIB_NEWLIB) && defined(_EXFUN)
_READ_WRITE_RETURN_TYPE _EXFUN(read, (int __fd, void *__buf, size_t __nbyte));
_READ_WRITE_RETURN_TYPE _EXFUN(write, (int __fd, const void *__buf, size_t __nbyte));
#else
int read(int fd, void *buf, size_t len);
int write(int fd, const void *buf, size_t len);
#endif

int open(const char *path, int flags, ...);
int close(int fd);
int ioctl(int fd, int cmd, void *args);
off_t lseek(int fd, off_t offset, int whence);
int rename(const char *from, const char *to);
int unlink(const char *path);
int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int fsync(int fd);
int ftruncate(int fd, off_t length);

int mkdir(const char *path, mode_t mode);
DIR *opendir(const char *path);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
int rmdir(const char *path);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, off_t offset);
void rewinddir(DIR *dirp);

int statfs(const char *path, struct statfs *buf);

#endif
