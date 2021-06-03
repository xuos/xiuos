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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <xiuos.h>
#include <iot-vfs.h>
#include <ff.h>
#include <ffconf.h>
#include <diskio.h>

/* copy from xs_base.h */
#define DK_UNKNOWN           0x00
#define DK_REG               0x01
#define DK_DIR               0x02

struct ff_disk ff_disks[FF_VOLUMES];

static int GetErrno(FRESULT error)
{
    if (error == FR_OK)
        return EOK;
    else if (error == FR_NO_FILE || error == FR_NO_PATH || error == FR_NO_FILESYSTEM)
        return -ENOENT;
    else if (error == FR_INVALID_NAME)
        return -EINVAL;
    else if (error == FR_EXIST || error == FR_INVALID_OBJECT)
        return -EEXIST;
    else if (error == FR_DISK_ERR || error == FR_NOT_READY || error == FR_INT_ERR)
        return -EIO;
    else if (error == FR_WRITE_PROTECTED || error == FR_DENIED)
        return -EROFS;
    else if (error == FR_MKFS_ABORTED)
        return -EINVAL;
    else
        return -1;
}

static int GetDisk(HardwareDevType dev)
{
    for (int i = 0; i < FF_VOLUMES; i++)
        if (ff_disks[i].dev == dev)
            return i;
    return -1;
}

static char *MakeFatfsPath(const struct MountPoint *mp,
        const char *relpath)
{
    int i = GetDisk(mp->dev);
    char *ff_path = malloc(strlen(relpath) + 8);

    sprintf(ff_path, "%d:/%s", i, relpath);

    return ff_path;
}

static int FatfsOpen(struct FileDescriptor *fdp, const char *path)
{
    FRESULT res;
    uint8_t fs_mode;
    FIL *file;
    char *ff_path;

    file = malloc(sizeof(FIL));
    if (file == NULL)
        return -ENOMEM;
    memset(file, 0, sizeof(FIL));
    fdp->data = file;

    fs_mode = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

    ff_path = MakeFatfsPath(fdp->mntp, path);
    res = f_open(fdp->data, ff_path, fs_mode);
    free(ff_path);

    return GetErrno(res);
}

static int FatfsClose(struct FileDescriptor *fdp)
{
    FRESULT res;

    res = f_close(fdp->data);

    /* Free file ptr memory */
    if (res == FR_OK) {
        free(fdp->data);
        fdp->data = NULL;
    }

    return GetErrno(res);
}

static ssize_t FatfsRead(struct FileDescriptor *fdp, void *dst, size_t len)
{
    FRESULT res;
    unsigned int br;

    res = f_read(fdp->data, dst, len, &br);
    if (res != FR_OK)
        return GetErrno(res);

    return br;
}

static ssize_t FatfsWrite(struct FileDescriptor *fdp, const void *src, size_t len)
{
    FRESULT res;
    unsigned int bw;

    res = f_write(fdp->data, src, len, &bw);
    if (res != FR_OK)
        return GetErrno(res);

    return bw;
}

static int FatfsSeek(struct FileDescriptor *fdp, off_t offset, int whence,
        off_t *new_offset)
{
    FRESULT res = FR_OK;
    off_t pos;

    switch (whence) {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = f_tell((FIL *)fdp->data) + offset;
        break;
    case SEEK_END:
        pos = f_size((FIL *)fdp->data) + offset;
        break;
    default:
        return -EINVAL;
    }

    if ((pos < 0) || (pos > f_size((FIL *)fdp->data)))
        return -EINVAL;

    res = f_lseek(fdp->data, pos);
    *new_offset = (off_t)f_tell((FIL *)fdp->data);

    return GetErrno(res);
}

static off_t FatfsTell(struct FileDescriptor *fdp)
{
    return f_tell((FIL *)fdp->data);
}

static int FatfsTruncate(struct FileDescriptor *fdp, off_t length)
{
    FRESULT res = FR_OK;
    off_t curr_len = f_size((FIL *)fdp->data);

    /* expand file if new position is larger than file size */
    res = f_lseek(fdp->data, length);
    if (res != FR_OK)
        return GetErrno(res);

    if (length < curr_len)
        res = f_truncate(fdp->data);
    else {
        /* get actual length */
        length = f_tell((FIL *)fdp->data);

        res = f_lseek(fdp->data, curr_len);
        if (res != FR_OK)
            return GetErrno(res);

        unsigned int bw;
        uint8_t c = 0;

        for (int i = curr_len; i < length; i++) {
            res = f_write(fdp->data, &c, 1, &bw);
            if (res != FR_OK)
                break;
        }
    }

    return GetErrno(res);
}

static int FatfsSync(struct FileDescriptor *fdp)
{
    FRESULT res = FR_OK;

    res = f_sync(fdp->data);

    return GetErrno(res);
}

static int FatfsOpendir(struct FileDescriptor *fdp, const char *path)
{
    FRESULT res;
    DIR *dir;
    char *ff_path;

    dir = malloc(sizeof(DIR));
    if (dir == NULL)
        return -ENOMEM;
    memset(dir, 0, sizeof(DIR));
    fdp->data = dir;

    ff_path = MakeFatfsPath(fdp->mntp, path);
    res = f_opendir((DIR *)fdp->data, ff_path);
    free(ff_path);

    return GetErrno(res);
}

static int FatfsClosedir(struct FileDescriptor *fdp)
{
    FRESULT res;

    res = f_closedir(fdp->data);

    if (res == FR_OK) {
        free(fdp->data);
        fdp->data = NULL;
    }

    return GetErrno(res);
}

static int FatfsReaddir(struct FileDescriptor *fdp, struct dirent *dirent)
{
    FRESULT res;
    FILINFO fno;

    res = f_readdir(fdp->data, &fno);
    if (res == FR_OK) {
        dirent->d_kind = (fno.fattrib & AM_DIR) ? DK_DIR : DK_REG;
        strcpy(dirent->d_name, fno.fname);
    }

    return GetErrno(res);
}

static int FatfsMount(struct MountPoint *mp)
{
    FRESULT res;
    char ff_mnt_point[16];
    int i = GetDisk(NULL);

    if (i < 0)
        return -ENOENT;
    ff_disks[i].dev = mp->dev;
    sprintf(ff_mnt_point, "%d:", i);

    mp->fs_data = malloc(sizeof(FATFS));
    if (mp->fs_data == NULL)
        return -ENOMEM;

    res = f_mount((FATFS *)mp->fs_data, ff_mnt_point, 1);

    if (res == FR_NO_FILESYSTEM) {
        /* create filesystem if not found */
        uint8_t work[FF_MAX_SS];
        MKFS_PARM parm = {FM_FAT | FM_SFD, 0, 0, 0, 0};

        res = f_mkfs(ff_mnt_point, &parm, work, sizeof(work));
        if (res == FR_OK)
            res = f_mount((FATFS *)mp->fs_data, ff_mnt_point, 1);
    }

    CHECK(res == FR_OK);

    return GetErrno(res);
}

static int FatfsUnmount(struct MountPoint *mp)
{
    FRESULT res;
    char ff_mnt_point[16];
    int i = GetDisk(mp->dev);

    if (i < 0)
        return -ENOENT;
    sprintf(ff_mnt_point, "%d:", i);

    res = f_mount(NULL, ff_mnt_point, 0);
    if (res == 0) {
        ff_disks[i].dev = NULL;
    }

    return GetErrno(res);
}

static int FatfsUnlink(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    char *ff_path;

    ff_path = MakeFatfsPath(mp, path);
    res = f_unlink(ff_path);
    free(ff_path);

    return GetErrno(res);
}

static int FatfsRename(struct MountPoint *mp, const char *from,
        const char *to)
{
    FRESULT res;
    FILINFO fno;
    char *ff_path_from, *ff_path_to;

    ff_path_from = MakeFatfsPath(mp, from);
    ff_path_to = MakeFatfsPath(mp, to);

    /* remove if "to" already exists */
    res = f_stat(ff_path_to, &fno);
    if (FR_OK == res) {
        res = f_unlink(ff_path_to);
        if (FR_OK != res)
            goto err;
    }

    res = f_rename(ff_path_from, ff_path_to);

err:
    free(ff_path_from);
    free(ff_path_to);

    return GetErrno(res);
}

static int FatfsMkdir(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    char *ff_path;

    ff_path = MakeFatfsPath(mp, path);
    res = f_mkdir(ff_path);
    free(ff_path);

    return GetErrno(res);
}

static int FatfsStat(struct MountPoint *mp, const char *path,
        struct FileStat *buf)
{
    FRESULT res;
    FILINFO fno;
    char *ff_path;
    struct tm tm;
    int year, mon, day, hour, min, sec;
    WORD tmp;

    ff_path = MakeFatfsPath(mp, path);
    res = f_stat(ff_path, &fno);
    if (res != FR_OK)
        goto err;

    buf->type = (fno.fattrib & AM_DIR) ? FTYPE_DIR : FTYPE_FILE;
    strcpy(buf->name, fno.fname);
    buf->size = fno.fsize;

    // tmp = fno.fdate;
    // day = tmp & 0x1f;
    // tmp >>= 5;
    // mon = tmp & 0x0f;
    // tmp >>= 4;
    // year = (tmp & 0x7f) + 1980;

    // tmp = fno.ftime;
    // sec = (tmp & 0x1f) * 2;
    // tmp >>= 5;
    // min = tmp & 0x3f;
    // tmp >>= 6;
    // hour = tmp & 0x1f;

    // memset(&tm, 0, sizeof(tm));
    // tm.tm_year = year - 1900;
    // tm.tm_mon = mon - 1;
    // tm.tm_mday = day;
    // tm.tm_hour = hour;
    // tm.tm_min = min;
    // tm.tm_sec = sec;

    // buf->mtime = mktime(&tm);
    buf->mtime = 1580486400; /* 2020/01/01 00:00:00 */

err:
    free(ff_path);

    return GetErrno(res);
}

static int FatfsStatvfs(struct MountPoint *mp, const char *path,
        struct statfs *buf)
{
    FATFS *fs;
    FRESULT res;
    char *ff_mnt_point;
    DWORD fre_clust, fre_sect, tot_sect;

    fs = mp->fs_data;

    ff_mnt_point = MakeFatfsPath(mp, "");
    res = f_getfree(ff_mnt_point, &fre_clust, &fs);
    free(ff_mnt_point);
    if (res != FR_OK)
        return -EIO;

    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    buf->f_bfree = fre_sect;
    buf->f_blocks = tot_sect;
#if FF_MAX_SS != 512
    buf->f_bsize = fs->ssize;
#else
    buf->f_bsize = 512;
#endif

    return GetErrno(res);
}

/* fatfs filesystem interface */
static struct Filesystem fatfs_fs = {
    .open = FatfsOpen,
    .close = FatfsClose,
    .read = FatfsRead,
    .write = FatfsWrite,
    .seek = FatfsSeek,
    .tell = FatfsTell,
    .truncate = FatfsTruncate,
    .sync = FatfsSync,
    .ioctl = NULL,
    .poll = NULL,
    .opendir = FatfsOpendir,
    .closedir = FatfsClosedir,
    .readdir = FatfsReaddir,
    .seekdir = NULL,
    .mount = FatfsMount,
    .unmount = FatfsUnmount,
    .unlink = FatfsUnlink,
    .rename = FatfsRename,
    .mkdir = FatfsMkdir,
    .stat = FatfsStat,
    .statvfs = FatfsStatvfs,
};

int FatfsInit()
{
    return RegisterFilesystem(FSTYPE_FATFS, &fatfs_fs);
}

DWORD GetFatTime(void)
{
    DWORD fat_time = 0;

#ifdef LIB
    time_t now;
    struct tm *p_tm;
    struct tm tm_now;
    x_base lock;

    /* get current time */
    now = time(NONE);

    /* lock scheduler. */
    lock = CriticalAreaLock();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_now, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    CriticalAreaUnLock(lock);

    fat_time = (DWORD)(tm_now.tm_year - 80) << 25 |
               (DWORD)(tm_now.tm_mon + 1)   << 21 |
               (DWORD)tm_now.tm_mday        << 16 |
               (DWORD)tm_now.tm_hour        << 11 |
               (DWORD)tm_now.tm_min         <<  5 |
               (DWORD)tm_now.tm_sec / 2 ;
#endif /* LIB  */

    return fat_time;
}
