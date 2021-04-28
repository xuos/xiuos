/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

/* 
 * Modified by:     AIIT XUOS Lab
 * Modified date:   2020-08-25
 * Description:     implement disk I/O interfaces with XiUOS driver APIs
 */

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include <xiuos.h>
#include <bus.h>
#include <bus_usb.h>
#include <dev_usb.h>

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

extern struct ff_disk ff_disks[FF_VOLUMES];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    struct ff_disk *disk = &ff_disks[pdrv];

    if (disk->dev) {
        struct BusBlockReadParam param; 
        param.pos = sector;
        param.buffer = buff;
        param.size = count;

        if (BusDevReadData(disk->dev, &param) == count)
        {
           return RES_OK;
        }
        return RES_ERROR;
        
    } else {
        return RES_ERROR;
    }
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    struct ff_disk *disk = &ff_disks[pdrv];

    if (disk->dev) {
        struct BusBlockWriteParam param; 
        param.pos = sector;
        param.buffer = buff;
        param.size = count;

        if (BusDevWriteData(disk->dev, &param) == count)
        {
            return RES_OK;
        }
        return RES_ERROR;
        
    } else {
        return RES_ERROR;
    }
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    struct ff_disk *disk = &ff_disks[pdrv];

    if (disk->dev == NONE)
        return RES_ERROR;

    if (cmd == GET_SECTOR_COUNT) {
        struct HalDevBlockParam block_param;

        if (disk->dev->dev_block_control == NONE)
            return RES_OK;

        memset(&block_param, 0, sizeof(block_param));
        block_param.cmd = OPER_BLK_GETGEOME;
        disk->dev->dev_block_control(disk->dev, &block_param);

        *(DWORD *)buff = block_param.dev_block.bank_num;
        if (block_param.dev_block.bank_num == 0)
            return RES_ERROR;
    } else if (cmd == GET_SECTOR_SIZE) {
        struct HalDevBlockParam block_param;

        if (disk->dev->dev_block_control == NONE)
            return RES_OK;

        memset(&block_param, 0, sizeof(block_param));
        block_param.cmd = OPER_BLK_GETGEOME;
        disk->dev->dev_block_control(disk->dev, &block_param);

        *(WORD *)buff = (WORD)(block_param.dev_block.size_perbank);
    } else if (cmd == GET_BLOCK_SIZE) {
        struct HalDevBlockParam block_param;

        if (disk->dev->dev_block_control == NONE)
            return RES_OK;

        memset(&block_param, 0, sizeof(block_param));
        block_param.cmd = OPER_BLK_GETGEOME;
        disk->dev->dev_block_control(disk->dev, &block_param);

        *(DWORD *)buff = block_param.dev_block.block_size / block_param.dev_block.size_perbank;
    } else if (cmd == CTRL_SYNC) {
        struct HalDevBlockParam block_param;

        if (disk->dev->dev_block_control == NONE)
            return RES_OK;

        memset(&block_param, 0, sizeof(block_param));
        block_param.cmd = OPER_BLK_SYNC;
        disk->dev->dev_block_control(disk->dev, &block_param);
    } else if (cmd == CTRL_TRIM) {
        struct HalDevBlockParam block_param;

        if (disk->dev->dev_block_control == NONE)
            return RES_OK;

        memset(&block_param, 0, sizeof(block_param));
        block_param.cmd = OPER_BLK_ERASE;
        disk->dev->dev_block_control(disk->dev, &block_param);        
    }

    return RES_OK;
}

