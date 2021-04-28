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

#ifndef __INC_GUNZIP_H__
#define __INC_GUNZIP_H__

typedef struct GzipHdr {
    uint8_t gz_method;
    uint8_t flags;
    uint32_t mtime;
    uint8_t extra_flags;
    uint8_t os_flags;
} __attribute__((packed)) GzipHdr_t;


typedef struct huft {
    unsigned char e;
    unsigned char b;
    union {
        uint16_t n;
        struct huft *t;
    };
} huft_t;

#define GUNZIP_WSIZE              (1 << 15)
#define GUNZIP_BYTEBUFFER_MAX     (1 << 14)

#define BMAX                      16
#define NMAX                      288

enum methods {
    STORED = 1,
    CODES,
};

int InflateUnzip(int zip_fd, int dst_fd, uint32_t cmpsize,
        uint32_t ucmpsize, uint32_t crc32);

#endif
