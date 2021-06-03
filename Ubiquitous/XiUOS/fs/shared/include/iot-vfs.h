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

#ifndef _INC_IOT_VFS_H_
#define _INC_IOT_VFS_H_

#include <xiuos.h>
#include <bus.h>

#define MAX_FILE_NAME 255
#define MAX_PATH MAX_FILE_NAME

enum FilesystemType
{
    FSTYPE_FATFS = 0,
    FSTYPE_IOTDEVICEFILE,
    FSTYPE_CH376,
    FSTYPE_END,
};

enum x_FileType
{
    FTYPE_FILE = 0,
    FTYPE_DIR,
    FTYPE_SOCKET,
    FTYPE_END,
};

struct Filesystem;

struct MountPoint
{
    struct SysDoubleLinklistNode node;
    enum FilesystemType fs_type;
    char *mnt_point;
    size_t mnt_point_len;
    void *fs_data;
    BusType bus;
    HardwareDevType dev;
    DriverType drv;
    struct Filesystem *fs;
};

struct FileStat
{
    enum x_FileType type;
    char name[MAX_FILE_NAME + 1];
    size_t size;
    time_t mtime;
};

struct statfs
{
    size_t f_bsize;
    size_t f_blocks;
    size_t f_bfree;
};

#define FD_MAX 64
#define FD_OFFSET 3

struct FileDescriptor
{
    char *path;
    enum x_FileType type;
    void *data; 
    struct MountPoint *mntp;
    off_t pos;
};

int NewFileDescriptor(struct FileDescriptor **fdptr);
struct FileDescriptor *GetFileDescriptor(int fd);
void FreeFileDescriptor(struct FileDescriptor *fdp);

struct dirent
{
    int d_kind;
    char d_name[MAX_PATH + 1];
};

/* NOTE: all path arguments must be relative path under mount point */
struct Filesystem
{
    int (*open)(struct FileDescriptor *fdp, const char *path);
    int (*close)(struct FileDescriptor *fdp);
    ssize_t (*read)(struct FileDescriptor *fdp, void *dst, size_t len);
    ssize_t (*write)(struct FileDescriptor *fdp, const void *src, size_t len);
    int (*seek)(struct FileDescriptor *fdp, off_t offset, int whence, off_t *new_offset);
    off_t (*tell)(struct FileDescriptor *fdp);
    int (*truncate)(struct FileDescriptor *fdp, off_t length);
    int (*sync)(struct FileDescriptor *fdp);

    int (*ioctl)(struct FileDescriptor *fdp, int cmd, void *args);
    int (*poll)(struct FileDescriptor *fdp, struct Pollreq *req);

    int (*opendir)(struct FileDescriptor *fdp, const char *path);
    int (*closedir)(struct FileDescriptor *fdp);
    int (*readdir)(struct FileDescriptor *fdp, struct dirent *dirent);
    int (*seekdir)(struct FileDescriptor *fdp, off_t offset);

    int (*mount)(struct MountPoint *mp);
    int (*unmount)(struct MountPoint *mp);
    int (*unlink)(struct MountPoint *mp, const char *path);
    int (*rename)(struct MountPoint *mp, const char *from,
            const char *to);
    int (*mkdir)(struct MountPoint *mp, const char *path);
    int (*stat)(struct MountPoint *mp, const char *path,
            struct FileStat *buf);
    int (*statvfs)(struct MountPoint *mp, const char *path,
            struct statfs *buf);
};

int RegisterFilesystem(enum FilesystemType fs_type,
        struct Filesystem *fs);

int MountFilesystem(const char *bus_name,
        const char *dev_name, const char *drv_name,
        enum FilesystemType fs_type, const char *path);

int UnmountFileSystem(const char *path);

void SyncOpenedFiles();

#endif /* _INC_IOT_VFS_H_ */
