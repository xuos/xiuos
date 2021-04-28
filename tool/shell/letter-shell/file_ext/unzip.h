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

#ifndef __INC_UNZIP_H__
#define __INC_UNZIP_H__

#include <stdint.h>

typedef struct cde {
    uint16_t this_disk_no;
    uint16_t disk_with_cdf_no;
    uint16_t cdf_entries_on_this_disk;
    uint16_t cdf_entries_total;
    uint32_t cdf_size;
    uint32_t cdf_offset;
} cde_t;
char BUILD_BUG_CDE_SIZE[sizeof(cde_t) == 16 ? 0 : -1];

typedef struct ZipHdr {
    uint16_t version;
    uint16_t zip_flags;
    uint16_t method;
    uint16_t modtime;
    uint16_t moddate;
    uint32_t crc32 __attribute__ ((__packed__));
    uint32_t cmpsize __attribute__ ((__packed__));
    uint32_t ucmpsize __attribute__ ((__packed__));
    uint16_t filename_len;
    uint16_t extra_len;
} __attribute__ ((__packed__)) ZipHdr_t;
char BUILD_BUG_ZIP_HDR_SIZE[sizeof(ZipHdr_t) == 26 ? 0 : -1];

typedef struct CdfHdr {
        uint16_t version_made_by;
        uint16_t version_needed;
        uint16_t cdf_flags;
        uint16_t method;
        uint16_t modtime;
        uint16_t moddate;
        uint32_t crc32 __attribute__ ((__packed__));
        uint32_t cmpsize __attribute__ ((__packed__));
        uint32_t ucmpsize __attribute__ ((__packed__));
        uint16_t filename_len; 
        uint16_t extra_len;
        uint16_t file_comment_len;
        uint16_t disk_number_start;
        uint16_t internal_attributes;
        uint32_t external_attributes __attribute__ ((__packed__));
        uint32_t local_header_offset
                __attribute__ ((__packed__));
} __attribute__ ((__packed__)) CdfHdr_t;
char BUILD_BUG_CDF_HDR_SIZE[sizeof(CdfHdr_t) == 42 ? 0 : -1];

#define FIND_CDF_BUF_SIZE       1024

#define ZIP_FILEHEADER_MAGIC    0x04034b50
#define ZIP_CDF_MAGIC           0x02014b50
#define ZIP_CDE_MAGIC           0x06054b50
#define ZIP_DD_MAGIC            0x08074b50

#endif
