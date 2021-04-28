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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <xiuos.h>
#include <iot-vfs.h>
#include <iot-vfs_posix.h>

DoubleLinklistType mnt_list;
static int mnt_list_lock;

char working_dir[MAX_PATH + 1];
static int working_dir_lock;

static struct Filesystem *fstable[FSTYPE_END];

static struct FileDescriptor *fdtable[FD_MAX - FD_OFFSET];
static int fdtable_lock;

static struct MountPoint *GetMountPoint(const char *path)
{
    struct MountPoint *mp = NULL, *itr;
    size_t longest_match = 0;
    size_t len, name_len = strlen(path);
    struct SysDoubleLinklistNode *node;

    KMutexObtain(mnt_list_lock, WAITING_FOREVER);
    DOUBLE_LINKLIST_FOR_EACH(node, &mnt_list) {
        itr =CONTAINER_OF(node, struct MountPoint, node);
        len = itr->mnt_point_len;

        if ((len < longest_match) || (len > name_len)) {
            continue;
        }

        if ((len > 1) && (path[len] != '/') && (path[len] != '\0')) {
            continue;
        }

        if (strncmp(path, itr->mnt_point, len) == 0) {
            mp = itr;
            longest_match = len;
        }
    }
    KMutexAbandon(mnt_list_lock);

    return mp;
}

char *GetAbsolutePath(const char *parent, const char *filename)
{
    char *abspath;
    char *dst0, *dst, *src;

    NULL_PARAM_CHECK(filename != NULL);

    if (parent == NULL)
        parent = working_dir;

    if (filename[0] != '/') {
        abspath = (char *)malloc(strlen(parent) + strlen(filename) + 2);
        if (abspath == NULL)
            return NULL;
        snprintf(abspath, strlen(parent) + strlen(filename) + 2,
                "%s/%s", parent, filename);
    } else {
        abspath = strdup(filename);
        if (abspath == NULL)
            return NULL;
    }

    src = abspath;
    dst = abspath;

    dst0 = dst;
    while (1) {
        char c = *src;

        if (c == '.') {
            if (!src[1]) {
                src++;
            } else if (src[1] == '/') {
                src += 2;

                while ((*src == '/') && (*src != '\0'))
                    src ++;
                continue;
            } else if (src[1] == '.') {
                if (!src[2]) {
                    src += 2;
                    goto up_one;
                } else if (src[2] == '/') {
                    src += 3;

                    while ((*src == '/') && (*src != '\0'))
                        src ++;
                    goto up_one;
                }
            }
        }

        while ((c = *src++) != '\0' && c != '/')
            *dst++ = c;

        if (c == '/') {
            *dst ++ = '/';
            while (c == '/')
                c = *src++;

            src --;
        } else if (!c) {
            break;
        }

        continue;

up_one:
        dst--;
        if (dst < dst0) {
            free(abspath);
            return NULL;
        }
        while (dst0 < dst && dst[-1] != '/')
            dst --;
    }

    *dst = '\0';

    dst --;
    if ((dst != abspath) && (*dst == '/'))
        *dst = '\0';

    if ('\0' == abspath[0]) {
        abspath[0] = '/';
        abspath[1] = '\0';
    }

    return abspath;
}

static char *GetRelativePath(const char *prefix, const char *abspath)
{
    char *relpath;
    int prefix_len, abspath_len;

    prefix_len = strlen(prefix);
    abspath_len = strlen(abspath);
    if (prefix_len == abspath_len)
        return "";

    relpath = (char *)abspath + prefix_len;
    while (*relpath == '/')
        relpath++;

    return relpath;
}

int NewFileDescriptor(struct FileDescriptor **fdptr)
{
    int fd;
    struct FileDescriptor *fdp;

    KMutexObtain(fdtable_lock, WAITING_FOREVER);

    if (fdptr)
        *fdptr = NULL;

    for (fd = 0; fd < FD_MAX - FD_OFFSET; fd++)
        if (fdtable[fd] == NULL)
            break;
    if (fd >= FD_MAX - FD_OFFSET) {
        fd = -1;
        goto err;
    }

    fdp = malloc(sizeof(struct FileDescriptor));
    if (fdp == NULL) {
        fd = -1;
        goto err;
    }

    fdtable[fd] = fdp;
    fd += FD_OFFSET;
    if (fdptr)
        *fdptr = fdp;

err:
    KMutexAbandon(fdtable_lock);

    return fd;
}

struct FileDescriptor *GetFileDescriptor(int fd)
{
    if (fd < 0 || fd >= FD_MAX)
        return NULL;

#if defined(FS_VFS_DEVFS) && defined(LIB_POSIX)
    extern int LibcStdioGetConsole();
    if (fd <= 2)
        fd = LibcStdioGetConsole();
#endif

    fd -= FD_OFFSET;
    if (fd < 0)
        return NULL;
    
    return fdtable[fd];
}

void FreeFileDescriptor(struct FileDescriptor *fdp)
{
    KMutexObtain(fdtable_lock, WAITING_FOREVER);
    for (int i = 0; i < FD_MAX - FD_OFFSET; i++)
        if (fdtable[i] == fdp) {
            free(fdp->path);
            free(fdp);
            fdtable[i] = NULL;
            break;
        }
    KMutexAbandon(fdtable_lock);
}

int MountFilesystem(const char *bus_name,
        const char *dev_name, const char *drv_name,
        enum FilesystemType fs_type, const char *path)
{
    struct MountPoint *mp = NULL, *itr;
    struct Bus *bus;
    HardwareDevType dev;
    DriverType drv;
    struct SysDoubleLinklistNode *node;
    int ret = -EINVAL;

    if (bus_name != NULL && dev_name != NULL && drv_name != NULL) {

        bus = BusFind(bus_name);
        if (bus == NULL) {
            SYS_ERR("%s: bus %s not found\n", __func__, bus_name);
            return -ENXIO;
        }
        dev = BusFindDevice(bus, dev_name);
        if (dev == NULL) {
            SYS_ERR("%s: dev %s on bus %s not found\n", __func__, dev_name, bus_name);
            return -ENXIO;
        }
        bus->owner_haldev = dev;
        drv = BusFindDriver(bus, drv_name);
        if (drv == NULL) {
            SYS_ERR("%s: drv %s on bus %s not found\n", __func__, drv_name, bus_name);
            return -ENXIO;
        }
        bus->owner_driver = drv;
    } else {
        dev = NULL;
    }

    if (fs_type >= FSTYPE_END) {
        SYS_ERR("%s: invalid filesystem\n", __func__);
        return -EINVAL;
    }

    if (fstable[fs_type] == NULL) {
        SYS_ERR("%s: specified filesystem not registered\n", __func__);
        return -EINVAL;
    }

    if (path == NULL || path[0] != '/') {
        SYS_ERR("%s: invalid mount point\n", __func__);
        return -EINVAL;
    }

    mp = malloc(sizeof(struct MountPoint));
    if (mp == NULL) {
        SYS_ERR("%s: memory not enough\n", __func__);
        return -ENOMEM;
    }

    KMutexObtain(mnt_list_lock, WAITING_FOREVER);

    mp->fs_type = fs_type;
    mp->fs = fstable[fs_type];
    mp->bus = bus;
    mp->dev = dev;
    mp->drv = drv;
    mp->fs_data = NULL;
    mp->mnt_point_len = strlen(path);
    mp->mnt_point = strdup(path);
    if (mp->mnt_point == NULL) {
        SYS_ERR("%s: memory not enough\n", __func__);
        ret = -ENOMEM;
        goto err;
    }

    if (mp->fs->mount == NULL) {
        SYS_ERR("%s: specified filesystem does not have mount function\n",
                __func__);
        ret = -EINVAL;
        goto err;
    }

    DOUBLE_LINKLIST_FOR_EACH(node, &mnt_list) {
        itr =CONTAINER_OF(node, struct MountPoint, node);

        if (mp->mnt_point_len != itr->mnt_point_len)
            continue;
        if (strncmp(mp->mnt_point, itr->mnt_point,
                mp->mnt_point_len) == 0) {
            SYS_ERR("%s: mount point already exists\n", __func__);
            ret = -EINVAL;
            goto err;
        }
    }

    if (dev != NULL)
        if (BusDevOpen(dev) != 0) {
            SYS_ERR("%s: failed to open device %s on bus %s\n",
                    __func__, dev_name, bus_name);
            ret = -EINVAL;
            goto err;
        }

    ret = mp->fs->mount(mp);
    if (ret < 0) {
        SYS_ERR("%s: filesystem mount failed\n", __func__);
        if (dev)
            BusDevClose(dev);
        goto err;
    }

    DoubleLinkListInsertNodeBefore(&mnt_list, &mp->node);
    DBG("%s: filesystem mounted at %s\n", __func__, path);

err:
    KMutexAbandon(mnt_list_lock);

    if (ret < 0) {
        free(mp->mnt_point);
        free(mp);
    }

    return ret;
}

int UnmountFileSystem(const char *path)
{
    size_t mnt_point_len;
    struct MountPoint *mp = NULL, *itr;
    struct SysDoubleLinklistNode *node;
    int ret = -EINVAL;

    if (path == NULL || path[0] != '/') {
        SYS_ERR("%s: invalid mount point\n", __func__);
        return -EINVAL;
    }
    mnt_point_len = strlen(path);

    KMutexObtain(mnt_list_lock, WAITING_FOREVER);

    DOUBLE_LINKLIST_FOR_EACH(node, &mnt_list) {
        itr =CONTAINER_OF(node, struct MountPoint, node);

        if (itr->mnt_point_len != mnt_point_len)
            continue;
        if (strncmp(itr->mnt_point, path, mnt_point_len) == 0) {
            mp = itr;
            break;
        }
    }

    if (mp == NULL) {
        SYS_ERR("%s: no mount point found on %s\n", __func__, path);
        ret = -EINVAL;
        goto err;
    }

    if (mp->fs->unmount == NULL) {
        SYS_ERR("%s: specified file system does not have unmount function",
                __func__);
        ret = -EINVAL;
        goto err;
    }

    ret = mp->fs->unmount(mp);
    if (ret < 0) {
        SYS_ERR("%s: filesystem unmount failed\n", __func__);
        goto err;
    }

    DoubleLinkListRmNode(&mp->node);
    if (mp->dev != NULL)
        BusDevClose(mp->dev);
    free(mp->mnt_point);
    free(mp);
    DBG("%s: filesystem unmounted from %s\n", __func__, path);

err:
    KMutexAbandon(mnt_list_lock);

    return ret;
}

int RegisterFilesystem(enum FilesystemType fs_type,
        struct Filesystem *fs)
{
    if (fs_type >= FSTYPE_END)
        return -EINVAL;

    fstable[fs_type] = fs;
    return 0;
}

void SyncOpenedFiles()
{
    struct FileDescriptor *fdp;

    for (int i = 0; i < FD_MAX - FD_OFFSET; i++) {
        fdp = fdtable[i];
        if (fdp != NULL)
            if (fdp->mntp->fs->sync != NULL)
                fdp->mntp->fs->sync(fdp);
    }
}

int open(const char *path, int flags, ...)
{
    int fd, ret;
    struct FileDescriptor *fdp;
    struct MountPoint *mp;
    char *abspath, *relpath;

    if (path == NULL) {
        SYS_ERR("%s: invalid file name\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    fd = NewFileDescriptor(&fdp);
    if (fd < 0) {
        KUpdateExstatus(ENOMEM);
        return -1;
    }

    abspath = GetAbsolutePath(NULL, path);
    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        SYS_ERR("%s: mount point not found\n", __func__);
        ret = -EINVAL;
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    fdp->type = FTYPE_FILE;
    fdp->mntp = mp;
    fdp->pos = 0;
    fdp->path = strdup(abspath);
    if (fdp->path == NULL) {
        SYS_ERR("%s: memory not enough\n", __func__);
        ret = -ENOMEM;
        goto err;
    }

    if (mp->fs->open == NULL) {
        SYS_ERR("%s: no open function found\n", __func__);
        ret = -EINVAL;
        goto err;
    }

    ret = mp->fs->open(fdp, relpath);
    if (ret < 0) {
        SYS_ERR("%s: file open failed\n", __func__);
        goto err;
    }

    if (flags & O_TRUNC)
        ftruncate(fd, 0);
    if (flags & O_APPEND)
        lseek(fd, 0, SEEK_END);

err:
    free(abspath);
    if (ret < 0) {
        FreeFileDescriptor(fdp);
        KUpdateExstatus(ret);
        return -1;
    }

    return fd;
}

int close(int fd)
{
    int ret;
    struct FileDescriptor *fdp;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->close == NULL) {
        SYS_ERR("%s: no close function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->close(fdp);
    if (ret < 0) {
        SYS_ERR("%s: close file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }
    FreeFileDescriptor(fdp);

    return 0;
}

int read(int fd, void *buf, size_t len)
{
    int ret;
    struct FileDescriptor *fdp;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->read == NULL) {
        SYS_ERR("%s: no read function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->read(fdp, buf, len);
    if (ret < 0) {
        SYS_ERR("%s: read file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return ret;
}

int write(int fd, const void *buf, size_t len)
{
    int ret;
    struct FileDescriptor *fdp;
    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->write == NULL) {
        SYS_ERR("%s: no write function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->write(fdp, buf, len);
    if (ret < 0) {
        SYS_ERR("%s: write file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return ret;
}

int ioctl(int fd, int cmd, void *args)
{
    int ret;
    struct FileDescriptor *fdp;
    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->ioctl == NULL) {
        SYS_ERR("%s: no ioctl function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->ioctl(fdp, cmd, args);
    if (ret < 0) {
        SYS_ERR("%s: ioctl file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return ret;
}

off_t lseek(int fd, off_t offset, int whence)
{
    int ret;
    off_t new_offset;
    struct FileDescriptor *fdp;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->seek == NULL) {
        SYS_ERR("%s: no seek function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->seek(fdp, offset, whence, &new_offset);
    if (ret < 0) {
        SYS_ERR("%s: seek file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return new_offset;
}

int rename(const char *from, const char *to)
{
    struct MountPoint *mp_from, *mp_to;
    char *abs_from, *abs_to, *rel_from, *rel_to;
    int ret = -EINVAL;

    if (from == NULL || to == NULL) {
        SYS_ERR("%s: invalid file name\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    abs_from = GetAbsolutePath(NULL, from);
    abs_to = GetAbsolutePath(NULL, to);
    mp_from = GetMountPoint(abs_from);
    mp_to = GetMountPoint(abs_to);
    if (mp_from == NULL || mp_from != mp_to) {
        SYS_ERR("%s: paths not under the same mount point\n", __func__);
        goto err;
    }
    rel_from = GetRelativePath(mp_from->mnt_point, abs_from);
    rel_to = GetRelativePath(mp_to->mnt_point, abs_to);

    if (mp_from->fs->rename == NULL) {
        SYS_ERR("%s: no rename function found\n", __func__);
        goto err;
    }

    ret = mp_from->fs->rename(mp_from, rel_from, rel_to);
    if (ret < 0) {
        SYS_ERR("%s: rename file failed\n", __func__);
        goto err;
    }

err:
    free(abs_from);
    free(abs_to);

    if (ret < 0) {
        KUpdateExstatus(ret);
        return -1;
    }
    return 0;
}

int unlink(const char *path)
{
    struct MountPoint *mp;
    char *abspath, *relpath;
    int ret = -EINVAL;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    abspath = GetAbsolutePath(NULL, path);
    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        SYS_ERR("%s: no mount point found\n", __func__);
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    if (mp->fs->unlink == NULL) {
        SYS_ERR("%s: no unlink function found\n", __func__);
        goto err;
    }

    ret = mp->fs->unlink(mp, relpath);
    if (ret < 0) {
        SYS_ERR("%s: unlink file failed\n", ret);
        goto err;
    }

err:
    free(abspath);

    if (ret < 0) {
        KUpdateExstatus(ret);
        return -1;
    }
    return 0;
}

int stat(const char *path, struct stat *buf)
{
    struct FileStat vfs_statbuf;
    struct MountPoint *mp;
    char *abspath, *relpath;
    int ret = -EINVAL;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    abspath = GetAbsolutePath(NULL, path);

    if (strcmp(abspath, "/") == 0) {
        buf->st_dev = 0;
        buf->st_mode = S_IRUSR | S_IRGRP | S_IROTH |
                S_IWUSR | S_IWGRP | S_IWOTH |
                S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        buf->st_size = 0;
        buf->st_mtime = 0;

        ret = 0;
        goto err;
    }

    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        // if (strcmp(abspath, "/") == 0) {
        //     /* vfs root directory, no fs mounted */
        //     buf->st_dev = 0;
        //     buf->st_mode = S_IRUSR | S_IRGRP | S_IROTH |
        //             S_IWUSR | S_IWGRP | S_IWOTH |
        //             S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        //     buf->st_size = 0;
        //     buf->st_mtime = 0;

        //     ret = 0;
        //     goto err;
        // }
        SYS_ERR("%s: no mount point found\n", __func__);
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    if (mp->fs->stat == NULL) {
        SYS_ERR("%s: no stat function found\n", __func__);
        goto err;
    }

    ret = mp->fs->stat(mp, relpath, &vfs_statbuf);
    if (ret < 0) {
        SYS_ERR("%s: stat file failed\n", __func__);
        goto err;
    }

    buf->st_dev = 0;
    buf->st_mode = S_IRUSR | S_IRGRP | S_IROTH |
            S_IWUSR | S_IWGRP | S_IWOTH;
    if (vfs_statbuf.type == FTYPE_DIR)
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    else
        buf->st_mode |= S_IFREG;
    buf->st_size = vfs_statbuf.size;
    buf->st_mtime = vfs_statbuf.mtime;

err:
    free(abspath);

    if (ret < 0) {
        KUpdateExstatus(ret);
        return -1;
    }
    return 0;
}

int fstat(int fd, struct stat *buf)
{
    int ret;
    struct FileDescriptor *fdp;
    char *relpath;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    return stat(fdp->path, buf);
}

int fsync(int fd)
{
    int ret;
    struct FileDescriptor *fdp;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->sync == NULL) {
        SYS_ERR("%s: no sync function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->sync(fdp);
    if (ret < 0) {
        SYS_ERR("%s: sync file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return ret;
}

int ftruncate(int fd, off_t length)
{
    int ret;
    struct FileDescriptor *fdp;

    fdp = GetFileDescriptor(fd);
    if (fdp == NULL) {
        KUpdateExstatus(EBADF);
        return -1;
    }

    if (fdp->mntp->fs->truncate == NULL) {
        SYS_ERR("%s: no truncate function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->truncate(fdp, length);
    if (ret < 0) {
        SYS_ERR("%s: truncate file failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    return ret;
}

int mkdir(const char *path, mode_t mode)
{
    struct MountPoint *mp;
    char *abspath, *relpath;
    int ret = -EINVAL;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    abspath = GetAbsolutePath(NULL, path);
    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        SYS_ERR("%s: no mount point found\n", __func__);
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    if (mp->fs->mkdir == NULL) {
        SYS_ERR("%s: no mkdir function found\n", __func__);
        goto err;
    }

    ret = mp->fs->mkdir(mp, relpath);
    if (ret < 0) {
        SYS_ERR("%s: mkdir failed\n", ret);
        goto err;
    }

err:
    free(abspath);

    if (ret < 0) {
        KUpdateExstatus(ret);
        return -1;
    }
    return 0;
}

DIR *opendir(const char *path)
{
    struct FileDescriptor *fdp;
    struct MountPoint *mp;
    char *abspath, *relpath;
    int ret = -EINVAL;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return NULL;
    }

    fdp = malloc(sizeof(struct FileDescriptor));
    if (fdp == NULL) {
        SYS_ERR("%s: memory not enough\n", __func__);
        KUpdateExstatus(ENOMEM);
        return NULL;
    }
    memset(fdp, 0, sizeof(struct FileDescriptor));

    abspath = GetAbsolutePath(NULL, path);
    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        SYS_ERR("%s: no mount point found\n", __func__);
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    fdp->type = FTYPE_DIR;
    fdp->mntp = mp;
    fdp->pos = 0;
    fdp->path = strdup(abspath);
    if (fdp->path == NULL) {
        SYS_ERR("%s: memory not enough\n", __func__);
        ret = -ENOMEM;
        goto err;
    }

    if (mp->fs->opendir == NULL) {
        SYS_ERR("%s: no opendir function found\n", __func__);
        goto err;
    }

    ret = mp->fs->opendir(fdp, relpath);
    if (ret < 0) {
        SYS_ERR("%s: opendir failed\n", __func__);
        goto err;
    }

err:
     free(abspath);

    if (ret < 0) {
        free(fdp->path);
        free(fdp);
        KUpdateExstatus(ret);
        return NULL;
    }
    return fdp;
}

int closedir(DIR *dirp)
{
    struct FileDescriptor *fdp = dirp;
    int ret;

    if (fdp == NULL) {
        SYS_ERR("%s: invalid directory pointer\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    if (fdp->mntp->fs->closedir == NULL) {
        SYS_ERR("%s: no closedir function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    ret = fdp->mntp->fs->closedir(fdp);
    if (ret < 0) {
        SYS_ERR("%s: closedir failed\n", __func__);
        KUpdateExstatus(ret);
        return -1;
    }

    free(fdp->path);
    free(fdp);

    return 0;
}

static struct dirent dirent;
struct dirent *readdir(DIR *dirp)
{
    struct FileDescriptor *fdp = dirp;
    int ret;

    if (fdp == NULL) {
        SYS_ERR("%s: invalid directory pointer\n", __func__);
        KUpdateExstatus(EINVAL);
        return NULL;
    }

    if (fdp->mntp->fs->readdir == NULL) {
        SYS_ERR("%s: no readdir function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return NULL;
    }

    ret = fdp->mntp->fs->readdir(fdp, &dirent);
    if (ret < 0) {
        SYS_ERR("%s: readdir failed\n", __func__);
        KUpdateExstatus(ret);
        return NULL;
    }

    if (dirent.d_name[0] == '\0')
        return NULL;
    return &dirent;
}

int rmdir(const char *path)
{
    return unlink(path);
}

int chdir(const char *path)
{
    char *abspath;
    DIR *dirp;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    dirp = opendir(path);
    if (dirp == NULL) {
        SYS_ERR("%s: chdir failed\n", __func__);
        KUpdateExstatus(ENOTDIR);
        return -1;
    }
    closedir(dirp);

    abspath = GetAbsolutePath(NULL, path);

    KMutexObtain(working_dir_lock, WAITING_FOREVER);
    strcpy(working_dir, abspath);
    KMutexAbandon(working_dir_lock);

    free(abspath);

    return 0;
}

char *getcwd(char *buf, size_t size)
{
    KMutexObtain(working_dir_lock, WAITING_FOREVER);
    strncpy(buf, working_dir, size - 1);
    KMutexAbandon(working_dir_lock);

    buf[size - 1] = '\0';

    return buf;
}

void seekdir(DIR *dirp, off_t offset)
{
    struct FileDescriptor *fdp = dirp;

    if (fdp == NULL) {
        SYS_ERR("%s: invalid directory pointer\n", __func__);
        KUpdateExstatus(EINVAL);
        return;
    }

    if (fdp->mntp->fs->seekdir == NULL) {
        SYS_ERR("%s: no seekdir function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return;
    }

    fdp->mntp->fs->seekdir(fdp, offset);
}

void rewinddir(DIR *dirp)
{
    struct FileDescriptor *fdp = dirp;

    if (fdp == NULL) {
        SYS_ERR("%s: invalid directory pointer\n", __func__);
        KUpdateExstatus(EINVAL);
        return;
    }

    if (fdp->mntp->fs->seekdir == NULL) {
        SYS_ERR("%s: no seekdir function found\n", __func__);
        KUpdateExstatus(EINVAL);
        return;
    }

    fdp->mntp->fs->seekdir(fdp, 0);
}

int statfs(const char *path, struct statfs *buf)
{
    struct MountPoint *mp;
    char *abspath, *relpath;
    int ret = -EINVAL;

    if (path == NULL) {
        SYS_ERR("%s: invalid path\n", __func__);
        KUpdateExstatus(EINVAL);
        return -1;
    }

    abspath = GetAbsolutePath(NULL, path);
    mp = GetMountPoint(abspath);
    if (mp == NULL) {
        SYS_ERR("%s: no mount point found\n", __func__);
        goto err;
    }
    relpath = GetRelativePath(mp->mnt_point, abspath);

    if (mp->fs->statvfs == NULL) {
        SYS_ERR("%s: no statvfs function found\n", __func__);
        goto err;
    }

    ret = mp->fs->statvfs(mp, relpath, buf);
    if (ret < 0) {
        SYS_ERR("%s: statfs failed\n", __func__);
        goto err;
    }

err:
    free(abspath);

    if (ret < 0) {
        KUpdateExstatus(ret);
        return -1;
    }
    return 0;
}

int VfsInit()
{
    InitDoubleLinkList(&mnt_list);

    mnt_list_lock = KMutexCreate();
    working_dir_lock = KMutexCreate();
    fdtable_lock = KMutexCreate();

    strcpy(working_dir, "/");

    extern int DevicefileInit();
    DevicefileInit();
    MountFilesystem(NULL, NULL, NULL, FSTYPE_IOTDEVICEFILE, "/dev");

    return 0;
}

int showfd()
{
    for (int i = 0; i < FD_MAX - FD_OFFSET; i++) {
        struct FileDescriptor *fdp = fdtable[i];
        if (fdp == NULL)
            continue;
        KPrintf("%d ", i + FD_OFFSET);
        if (fdp->type == FTYPE_DIR)
            KPrintf("DIR ");
        else
            KPrintf("FILE ");
        KPrintf("%s\n", fdp->path);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),showfd, showfd,  list file descriptor );
