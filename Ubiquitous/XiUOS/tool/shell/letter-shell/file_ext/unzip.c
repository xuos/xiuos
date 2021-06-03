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

#include <xiuos.h>

#if defined(FS_VFS) && defined(TOOL_SHELL)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iot-vfs_posix.h>
#include <unzip.h>
#include <gunzip.h>
#include "utility.h"
#include <string.h>

int zip_fd;

static void ReadZipHdr(ZipHdr_t *zip_hdr)
{
    zip_hdr->version = read16Le(zip_fd);
    zip_hdr->zip_flags = read16Le(zip_fd);
    zip_hdr->method = read16Le(zip_fd);
    zip_hdr->modtime = read16Le(zip_fd);
    zip_hdr->moddate = read16Le(zip_fd);
    zip_hdr->crc32 = read32Le(zip_fd);
    zip_hdr->cmpsize = read32Le(zip_fd);
    zip_hdr->ucmpsize = read32Le(zip_fd);
    zip_hdr->filename_len = read16Le(zip_fd);
    zip_hdr->extra_len = read16Le(zip_fd);
}

static void ReadCdfHdr(CdfHdr_t *cdf_hdr)
{
    cdf_hdr->version_made_by = read16Le(zip_fd);
    cdf_hdr->version_needed = read16Le(zip_fd);
    cdf_hdr->cdf_flags = read16Le(zip_fd);
    cdf_hdr->method = read16Le(zip_fd);
    cdf_hdr->modtime = read16Le(zip_fd);
    cdf_hdr->moddate = read16Le(zip_fd);
    cdf_hdr->crc32 = read32Le(zip_fd);
    cdf_hdr->cmpsize = read32Le(zip_fd);
    cdf_hdr->ucmpsize = read32Le(zip_fd);
    cdf_hdr->filename_len = read16Le(zip_fd);
    cdf_hdr->extra_len = read16Le(zip_fd);
    cdf_hdr->file_comment_len = read16Le(zip_fd);
    cdf_hdr->disk_number_start = read16Le(zip_fd);
    cdf_hdr->internal_attributes = read16Le(zip_fd);
    cdf_hdr->external_attributes = read32Le(zip_fd);
    cdf_hdr->local_header_offset = read32Le(zip_fd);
}

static uint32_t FindCdfOffset()
{
    cde_t cde;
    unsigned char *buf;
    unsigned char *p;
    size_t ReadSize;
    off_t end;
    uint32_t found = (uint32_t)-1;

    end = lseek(zip_fd, 0, SEEK_END);
    if (end == -1) {
        return (uint32_t)-1;
    }

    buf = malloc(FIND_CDF_BUF_SIZE);
    end = MAX(0, end - FIND_CDF_BUF_SIZE);
    lseek(zip_fd, end, SEEK_SET);
    ReadSize = read(zip_fd, buf, FIND_CDF_BUF_SIZE);

    p = buf;
    while (p <= buf + ReadSize - sizeof(cde_t) - 4) {
        if (*p != 'P') {
            p++;
            continue;
        }
        if (*(++p) != 'K')
            continue;
        if (*(++p) != 5)
            continue;
        if (*(++p) != 6)
            continue;
        memcpy(&cde, p + 1, sizeof(cde_t));
        char *tmp = (char *)&cde.cdf_offset;
        uint32_t cdf_offset = 0;
        for (int i = 0; i < 4; i++)
            cdf_offset |= (uint32_t)tmp[i] << (i * 8);
        if (cdf_offset < end + (p - buf))
            found = cdf_offset;
    }
    free(buf);

    return found;
}

static uint32_t ReadNextCdf(uint32_t cdf_offset, CdfHdr_t *cdf_hdr)
{
    uint32_t magic;

    if (cdf_offset == (uint32_t)-1)
        return (uint32_t)-1;

    lseek(zip_fd, cdf_offset, SEEK_SET);

    magic = read32Le(zip_fd);
    if (magic == ZIP_CDE_MAGIC)
        return 0;

    ReadCdfHdr(cdf_hdr);
    cdf_offset += 4 + sizeof(CdfHdr_t) +
            cdf_hdr->filename_len +
            cdf_hdr->extra_len +
            cdf_hdr->file_comment_len;

    return cdf_offset;
}

static int CreateLeadingDir(char *path) {
    int len = strlen(path);
    char tmp;
    struct stat statbuf;

    for (int i = 1; i < len; i++)
        if (path[i] == '/') {
            tmp = path[i];
            path[i] = '\0';
            if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                path[i] = tmp;
                continue;
            }
            if (mkdir(path, 0777) < 0) {
                path[i] = tmp;
                return -1;
            }
            path[i] = tmp;
        }

    return 0;
}

static int ExtractUncompressedFile(int in_fd, int out_fd, uint32_t size)
{
    uint32_t size_read = 0;
    uint32_t size_written = 0;
    char *buf = malloc(512);

    if (buf == NULL)
        return -1;

    while (size > 0) {
        size_read = read(in_fd, buf, MIN(size, 512));
        size_written = write(out_fd, buf, size_read);
        if (size_read != size_written) {
            free(buf);
            return -1;
        }
        size -= size_written;
    }
    free(buf);

    return 0;
}

static int ExtractFile(ZipHdr_t *zip_hdr, int dst_fd)
{
    switch (zip_hdr->method) {
    case 0:
        return ExtractUncompressedFile(zip_fd,
                dst_fd, zip_hdr->cmpsize);
    case 8:
        return InflateUnzip(zip_fd, dst_fd, zip_hdr->cmpsize,
                zip_hdr->ucmpsize, zip_hdr->crc32);
    default:
        return -1;
    }
}

static int UnzipFile(char *path)
{
    int ret = 0;
    uint32_t cdf_offset;
    char *filename_buf = NULL;

    if ((zip_fd = open(path, O_RDONLY)) < 0)
        return -1;

    cdf_offset = FindCdfOffset();
    if (cdf_offset == (uint32_t)-1) {
        ret = -1;
    }
    while (1) {
        if (cdf_offset == (uint32_t)-1)
            break;

        CdfHdr_t cdf_hdr;
        ZipHdr_t zip_hdr;

        cdf_offset = ReadNextCdf(cdf_offset, &cdf_hdr);
        if (cdf_offset == 0)
            break;
        lseek(zip_fd, cdf_hdr.local_header_offset + 4, SEEK_SET);
        ReadZipHdr(&zip_hdr);
        if (zip_hdr.zip_flags & 0x8 || zip_hdr.zip_flags & 0x1) {
            ret = -1;
            break;
        }

        if (filename_buf)
            free(filename_buf);
        filename_buf = malloc(zip_hdr.filename_len + 1);
        if (filename_buf == NULL) {
            ret = -1;
            break;
        }
        read(zip_fd, filename_buf, zip_hdr.filename_len);
        filename_buf[zip_hdr.filename_len] = '\0';
        lseek(zip_fd, zip_hdr.extra_len, SEEK_CUR);

        if (filename_buf[zip_hdr.filename_len - 1] == '/') {
            KPrintf("   creating: %s\n", filename_buf);
            if (CreateLeadingDir(filename_buf) < 0) {
                ret = -1;
                break;
            }
            lseek(zip_fd, zip_hdr.cmpsize, SEEK_SET);
        } else {
            if (CreateLeadingDir(filename_buf) < 0) {
                ret = -1;
                break;
            }
            int dst_fd = open(filename_buf, O_WRONLY | O_CREAT | O_TRUNC);
            if (dst_fd < 0) {
                ret = -1;
                break;
            }
            KPrintf("   inflating: %s\n", filename_buf);
            ExtractFile(&zip_hdr, dst_fd);
            close(dst_fd);
        }
    }
    if (filename_buf)
        free(filename_buf);

    return ret;
}

static void UnzipPrintUsage()
{
    KPrintf("Usage: unzip [FILES]...\n");
}

int unzip(int argc, char **argv)
{
    if (argc < 2) {
        UnzipPrintUsage();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (!IsSuffixedWith(argv[i], ".zip")) {
            KPrintf("Unvalid extension: %s\n", argv[i]);
            continue;
        }
        KPrintf("Decompressing zip file: %s\n", argv[i]);
        if (UnzipFile(argv[i]) < 0)
            KPrintf("Unable to extract zip file: %s\n", argv[i]);
    }

    return 0;
}


#endif
