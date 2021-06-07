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

#ifndef __INC_TAR_H__
#define __INC_TAR_H__

#define TAR_BLOCK_SIZE          512
#define TAR_FILE_NAME_LEN       100
#define TAR_MAGIC_STR           "ustar"

#define ROUNDUP(a, align)       (((a) + (align) - 1) & ~((align) - 1))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))

typedef struct TarHdr {
    char name[TAR_FILE_NAME_LEN];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[TAR_FILE_NAME_LEN];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} TarHdr_t;

enum FileType {
    TYPE_REG = '0',
    TYPE_LNK = '1',
    TYPE_SYM = '2',
    TYPE_CHR = '3',
    TYPE_BLK = '4',
    TYPE_DIR = '5',
    TYPE_FIFO = '6',
};

enum FilterType {
    GZIP = 1,
    BZIP2,
};

#endif
