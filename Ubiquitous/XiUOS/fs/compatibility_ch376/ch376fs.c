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
 * @file ch376fs.c
 * @brief CH376 file system implementation
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.03.18
 */

#include <ch376fs.h>

#define	CMD_RESET_ALL		0x05
#define	CMD_CHECK_EXIST		0x06
#define	CMD_GET_FILE_SIZE	0x0C
#define	CMD_SET_FILE_SIZE	0x0D
#define	CMD_SET_USB_MODE	0x15
#define	CMD_RD_USB_DATA0	0x27
#define	CMD_WR_REQ_DATA		0x2D
#define	CMD_SET_FILE_NAME	0x2F
#define	CMD_DISK_CONNECT	0x30
#define	CMD_DISK_MOUNT		0x31
#define	CMD_FILE_OPEN		0x32
#define	CMD_FILE_ENUM_GO	0x33
#define	CMD_FILE_CREATE		0x34
#define	CMD_FILE_ERASE		0x35
#define	CMD_FILE_CLOSE		0x36
#define CMD_DIR_INFO_READ   0x37
#define CMD_BYTE_LOCATE     0x39
#define	CMD_BYTE_READ		0x3A
#define	CMD_BYTE_RD_GO		0x3B
#define	CMD_BYTE_WRITE		0x3C
#define	CMD_BYTE_WR_GO		0x3D
#define	CMD_DIR_CREATE		0x40
#define	CMD_WRITE_VAR32		0x0D

#define USB_INT_SUCCESS     0x14
#define USB_INT_DISK_READ   0x1D
#define USB_INT_DISK_WRITE  0x1E

#define ERR_OPEN_DIR        0x41
#define ERR_MISS_FILE       0x42
#define ERR_FOUND_NAME      0x43
#define ERR_FILE_CLOSE      0xB4

/* The flag bit for whether the folder was first opened */
static uint8 dir_flag = 0;

/* Save the read directory information */
static struct Ch376FatDirInfo dir;

/* Send the command */
static struct BusConfigureInfo cfg;

/* Command following parameters */
static uint8 cmd;
static struct BusBlockWriteParam write_cmd = {
    0,
    &cmd,
    1,
};

/* Reply to command */
static uint8 reply;
static struct BusBlockReadParam read_reply = {
    0,
    &reply,
    1,
    1,
};

/**
 * @description: Set file or directory path
 * @param bus - CH376 device bus
 * @param name - path name
 */
static void SetFileName(BusType bus, uint8* name)
{
    cfg.configure_cmd =  CMD_SET_FILE_NAME;
    BusDrvConfigure(bus->owner_driver, &cfg);

    cmd = *name;
    BusDevWriteData(bus->owner_haldev, &write_cmd);
    while (cmd) {
        name++;
        cmd = *name;
        if((cmd == 0x5c) || (cmd == 0x2f)) 
            cmd = 0;
        BusDevWriteData(bus->owner_haldev, &write_cmd);
    }
}

/**
 * @description: Back to the root directory
 * @param bus - CH376 device bus
 */
static void OpenRoot(BusType bus)
{
    SetFileName(bus, "/");
    cfg.configure_cmd =  CMD_FILE_OPEN;
    BusDrvConfigure(bus->owner_driver, &cfg);
    BusDevReadData(bus->owner_haldev, &read_reply);
}

/**
 * @description: Open file
 * @param fdp - file descriptor
 * @param path - path name
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsOpen(struct FileDescriptor *fdp, const char *path)
{
    uint32 len, idx;
    idx = 0;
    len = strlen(path);
    char buff[len + 1];
    strncpy(buff, path, len);
    buff[len] = 0;

    for (int i = 0; i < len; i++) {
        if (buff[i] == '/') {
            buff[i] = 0;
            SetFileName(fdp->mntp->bus, &buff[idx]);
            idx = i + 1;
            cfg.configure_cmd =  CMD_FILE_OPEN;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
            switch (reply) {
            case USB_INT_SUCCESS:
            case ERR_OPEN_DIR:
                break;
            
            case ERR_MISS_FILE:
                OpenRoot(fdp->mntp->bus);
                return -ERROR;

            default:
                return -ERROR;
            }
        }
    }
    SetFileName(fdp->mntp->bus, &buff[idx]);
    cfg.configure_cmd =  CMD_FILE_OPEN;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    switch (reply) {
    case USB_INT_SUCCESS:
    case ERR_OPEN_DIR:
        return EOK;
    
    case ERR_MISS_FILE:
        cfg.configure_cmd =  CMD_FILE_CREATE;
        BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
        BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
        return EOK;

    default:
        return -ERROR;
    }
}

/**
 * @description: Close file
 * @param fdp - file descriptor
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsClose(struct FileDescriptor *fdp)
{
    cfg.configure_cmd =  CMD_FILE_CLOSE;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    cmd =  0x01;
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);
    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    if (reply == USB_INT_SUCCESS) {
        OpenRoot(fdp->mntp->bus);
        return EOK;
    }

    return -ERROR;
}

/**
 * @description: Read file
 * @param fdp - file descriptor
 * @param dst - store the read data
 * @param len - need data length
 * @return length of data
 */
static ssize_t Ch376fsRead(struct FileDescriptor *fdp, void *dst, size_t len)
{
    ssize_t size = 0;
    uint8 buff[PARAM_LEN];
    struct BusBlockReadParam read_param;
    read_param.pos = 0;
    read_param.buffer = buff;
    
    cfg.configure_cmd =  CMD_BYTE_READ;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    cmd =  (uint8)len;
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);
    cmd =  (uint8)(len >> 8);
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);

    while (1) {
        BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);

        switch (reply) {
        case USB_INT_SUCCESS:
            return size;

        case USB_INT_DISK_READ:
            cfg.configure_cmd =  CMD_RD_USB_DATA0;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            BusDevReadData(fdp->mntp->bus->owner_haldev, &read_param);
            memcpy(dst + size, &buff[1], buff[0]);
            size += buff[0];
            cfg.configure_cmd =  CMD_BYTE_RD_GO;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            break;
        
        default:
            return -ERROR;
        }
    }
}

/**
 * @description: Write file
 * @param fdp - file descriptor
 * @param src - store the write data
 * @param len - write data length
 * @return success: EOK, failure: -ERROR
 */
static ssize_t Ch376fsWrite(struct FileDescriptor *fdp, const void *src, size_t len)
{
    struct BusBlockWriteParam write_param;
    char WriteBuff[len], temp[len];
    write_param.pos = 0;
    write_param.buffer = WriteBuff;
    size_t size = 0;

    memcpy(temp, src, len);

    cfg.configure_cmd =  CMD_BYTE_WRITE;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    cmd =  (uint8)len;
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);
    cmd =  (uint8)(len >> 8);
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);

    while (1) {
        BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);

        switch (reply) {
        case USB_INT_SUCCESS:
            return EOK;

        case USB_INT_DISK_WRITE:
            cfg.configure_cmd =  CMD_WR_REQ_DATA;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
            strncpy(WriteBuff, &temp[size], reply);
            write_param.size = reply;
            BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_param);
            size += reply;

            cfg.configure_cmd =  CMD_BYTE_WR_GO;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            break;
        
        default:
            return -ERROR;
        }
    }
}

/**
 * @description: Moves pointer to end of file
 * @param fdp - file descriptor
 * @param offset - the old offset
 * @param whence - moving position
 * @param new_offset - the new offset
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsSeek(struct FileDescriptor *fdp, off_t offset, int whence, off_t *new_offset)
{
    cfg.configure_cmd =  CMD_BYTE_LOCATE;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);

    uint8 i = 4;
    cmd = 0xFF;
    while (i--)
        BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);
    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    if(reply == USB_INT_SUCCESS)
        return EOK;

    return -ERROR;
}

/**
 * @description: Open directory
 * @param fdp - file descriptor
 * @param path - directory path name
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsOpenDir(struct FileDescriptor *fdp, const char *path)
{
    uint32 len, idx;
    idx = 0;
    len = strlen(path);
    char buff[len + 1];
    strncpy(buff, path, len);
    buff[len] = 0;

    dir_flag = 0;

    for (int i = 0; i < len; i++) {
        if(buff[i] == '/') {
            buff[i] = 0;
            SetFileName(fdp->mntp->bus, &buff[idx]);
            idx = i + 1;
            cfg.configure_cmd =  CMD_FILE_OPEN;
            BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
            BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
            switch (reply) {
            case USB_INT_SUCCESS:
            case ERR_OPEN_DIR:
                break;
            
            case ERR_MISS_FILE:
                OpenRoot(fdp->mntp->bus);
                return -ERROR;

            default:
                return -ERROR;
            }
        }
    }
    SetFileName(fdp->mntp->bus, &buff[idx]);
    cfg.configure_cmd =  CMD_FILE_OPEN;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    switch (reply) {
    case USB_INT_SUCCESS:
    case ERR_OPEN_DIR:
        return EOK;
    
    case ERR_MISS_FILE:
        OpenRoot(fdp->mntp->bus);
        return -ERROR;

    default:
        return -ERROR;
    }
}

/**
 * @description: Close directory
 * @param fdp - file descriptor
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsCloseDir(struct FileDescriptor *fdp)
{
    cfg.configure_cmd =  CMD_FILE_CLOSE;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
    cmd = 0x01;
    BusDevWriteData(fdp->mntp->bus->owner_haldev, &write_cmd);
    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    dir_flag = 0;
    if ((reply == ERR_FILE_CLOSE) || (reply == USB_INT_SUCCESS)) {
        OpenRoot(fdp->mntp->bus);
        return EOK;
    }

    return -ERROR;
}

/**
 * @description: Read directory
 * @param fdp - file descriptor
 * @param dirent - transmit directory info
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsReadDir(struct FileDescriptor *fdp, struct dirent *dirent)
{
    memset(&dir, 0, sizeof(dir));
    struct BusBlockReadParam read_param;
    read_param.pos = 0;
    read_param.buffer = &dir;
    
    if (dir_flag == 0) {
        char seek[2];
        seek[0] = '*';
        seek[1] = 0x00;

        dir_flag = 1;

        SetFileName(fdp->mntp->bus, seek);
    
        cfg.configure_cmd =  CMD_FILE_OPEN;
        BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
        goto __exit;
    }
    
    cfg.configure_cmd =  CMD_FILE_ENUM_GO;
    BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);

__exit:

    BusDevReadData(fdp->mntp->bus->owner_haldev, &read_reply);
    switch (reply) {
    case USB_INT_DISK_READ:
        cfg.configure_cmd =  CMD_RD_USB_DATA0;
        BusDrvConfigure(fdp->mntp->bus->owner_driver, &cfg);
        BusDevReadData(fdp->mntp->bus->owner_haldev, &read_param);
        dir.dir_attr = 0;
        strncpy(dirent->d_name, dir.dir_name, 11);
        return EOK;
    
    case ERR_MISS_FILE:
        memset(dirent->d_name, 0, strlen(dirent->d_name));
        return EOK;

    default:
        return -ERROR;
    }
}

/**
 * @description: Create directory
 * @param mp - mount point
 * @param path - directory path name
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsCreateDir(struct MountPoint *mp, const char *path)
{
    uint32 len, idx;
    len = strlen(path);
    idx = 0;
    char buff[len + 1];
    strncpy(buff, path, len);
    buff[len] = 0;

    for (int i = 0; i < len; i++) {
        if (buff[i] == '/') {
            buff[i] = 0;
            SetFileName(mp->bus, &buff[idx]);
            idx = i + 1;
            cfg.configure_cmd =  CMD_FILE_OPEN;
            BusDrvConfigure(mp->bus->owner_driver, &cfg);
            BusDevReadData(mp->bus->owner_haldev, &read_reply);
            switch (reply) {
            case USB_INT_SUCCESS:
            case ERR_OPEN_DIR:
                break;
            
            case ERR_MISS_FILE:
                cfg.configure_cmd =  CMD_DIR_CREATE;
                BusDrvConfigure(mp->bus->owner_driver, &cfg);
                BusDevReadData(mp->bus->owner_haldev, &read_reply);
                break;

            default:
                return ERROR;
            }
        }
    }

    SetFileName(mp->bus, &buff[idx]);
    cfg.configure_cmd =  CMD_DIR_CREATE;
    BusDrvConfigure(mp->bus->owner_driver, &cfg);
    BusDevReadData(mp->bus->owner_haldev, &read_reply);
    switch (reply) {
    case USB_INT_SUCCESS:
        break;
    
    case ERR_FOUND_NAME:
        KPrintf("File of the same name exists\n");
        break;

    default:
        return ERROR;
    }

    OpenRoot(mp->bus);

    return EOK;
}

/**
 * @description: Create directory
 * @param mp - mount point
 * @param path - file path name
 * @param buf - file state info
 * @return EOK
 */
static int Ch376fsStat(struct MountPoint *mp, const char *path, struct FileStat *buf)
{
    strncpy(buf->name, dir.dir_name, strlen(dir.dir_name));
    if(dir.dir_ntres == 0x10)
        buf->type = FTYPE_DIR;
    else if(dir.dir_ntres == 0x20)
        buf->type = FTYPE_FILE;
    buf->size = dir.dir_file_size >> 8;
    buf->mtime = 1580486400;
    return EOK;
}

/**
 * @description: Mount CH376 file system
 * @param mp - mount point
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsMount(struct MountPoint *mp)
{
    cfg.configure_cmd =  CMD_CHECK_EXIST;
    BusDrvConfigure(mp->bus->owner_driver, &cfg);
    cmd =  0x55;
    BusDevWriteData(mp->bus->owner_haldev, &write_cmd);
    BusDevReadData(mp->bus->owner_haldev, &read_reply);
    if(reply != 0xAA)
        goto __exit;

    cfg.configure_cmd =  CMD_SET_USB_MODE;
    BusDrvConfigure(mp->bus->owner_driver, &cfg);
    if (mp->bus->bus_type == TYPE_SDIO_BUS)
        cmd =  0x03;
    else if (mp->bus->bus_type == TYPE_USB_BUS)
        cmd =  0x06;
    BusDevWriteData(mp->bus->owner_haldev, &write_cmd);
    BusDevReadData(mp->bus->owner_haldev, &read_reply);
    if(reply != 0x51)
        goto __exit;

    cfg.configure_cmd =  CMD_DISK_MOUNT;
    BusDrvConfigure(mp->bus->owner_driver, &cfg);
    BusDevReadData(mp->bus->owner_haldev, &read_reply);
    if(reply != 0x14)
        goto __exit;

    return EOK;

__exit:
    return -ERROR;
}

/**
 * @description: Unlink file
 * @param mp - mount point
 * @param path - file path name
 * @return success: EOK, failure: -ERROR
 */
static int Ch376fsUnlink(struct MountPoint *mp, const char *path)
{
    uint32 len, idx;
    idx = 0;
    len = strlen(path);
    char buff[len + 1];
    strncpy(buff, path, len);
    buff[len] = 0;

    for (int i = 0; i < len; i++) {
        if (buff[i] == '/') {
            buff[i] = 0;
            SetFileName(mp->bus, &buff[idx]);
            idx = i + 1;
            cfg.configure_cmd =  CMD_FILE_OPEN;
            BusDrvConfigure(mp->bus->owner_driver, &cfg);
            BusDevReadData(mp->bus->owner_haldev, &read_reply);
            switch (reply) {
            case USB_INT_SUCCESS:
            case ERR_OPEN_DIR:
                break;
            
            case ERR_MISS_FILE:
            default:
                OpenRoot(mp->bus);
                return -ERROR;
            }
        }
    }
    SetFileName(mp->bus, &buff[idx]);
    cfg.configure_cmd =  CMD_FILE_OPEN;
    BusDrvConfigure(mp->bus->owner_driver, &cfg);
    BusDevReadData(mp->bus->owner_haldev, &read_reply);
    switch (reply) {
    case USB_INT_SUCCESS:
    case ERR_OPEN_DIR:
        cfg.configure_cmd =  CMD_FILE_ERASE;
        BusDrvConfigure(mp->bus->owner_driver, &cfg);
        BusDevReadData(mp->bus->owner_haldev, &read_reply);
        OpenRoot(mp->bus);
        return EOK;
    
    case ERR_MISS_FILE:
    default:
        OpenRoot(mp->bus);
        return -ERROR;
    }
}

static struct Filesystem ch376fs = {
    .open = Ch376fsOpen,
    .close = Ch376fsClose,
    .read = Ch376fsRead,
    .write = Ch376fsWrite,
    .seek = Ch376fsSeek,
    .tell = NONE,
    .truncate = NONE,
    .sync = NONE,
    .ioctl = NONE,
    .poll = NONE,
    .opendir = Ch376fsOpenDir,
    .closedir = Ch376fsCloseDir,
    .readdir = Ch376fsReadDir,
    .seekdir = NULL,
    .mount = Ch376fsMount,
    .unmount = NONE,
    .unlink = Ch376fsUnlink,
    .rename = NONE,
    .mkdir = Ch376fsCreateDir,
    .stat = Ch376fsStat,
    .statvfs = NONE,
};

/**
 * @description: Init CH376 file system
 * @return success: EOK, failure: other
 */
int Ch376fsInit()
{
    return RegisterFilesystem(FSTYPE_CH376, &ch376fs);
}
