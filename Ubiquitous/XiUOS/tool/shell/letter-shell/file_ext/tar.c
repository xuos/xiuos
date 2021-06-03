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

#include <iot-vfs_posix.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "tar.h"
#include "utility.h"

char *bytebuf;

static void PrintOctal(char *str, int len, uint64_t value)
{
    char buf[32];
    char *cp;
    int written_len;

    written_len = sprintf(buf, "%0*llo", len, value);
    cp = buf + written_len - len;

    if (*cp == '0')
        cp++;

    memcpy(str, cp, len);
}

static uint64_t ScanOctal(char *str, int len)
{
    uint64_t ret = 0;

    for (int i = 0; i < len; i++) {
        if (!isdigit(str[i]))
            break;
        if (str[i] > '7')
            break;
        ret = ret * 8 + str[i] - '0';
    }

    return ret;
}

static void TarChecksumHeader(TarHdr_t *hdr)
{
    int chksum;

    memcpy(hdr->magic, TAR_MAGIC_STR, 6);
    memcpy(hdr->version, "00", 2);

    memset(hdr->chksum, ' ', sizeof(hdr->chksum));
    chksum = 0;
    for (int i = 0; i < sizeof(TarHdr_t); i++)
        chksum += ((unsigned char *)hdr)[i];
    PrintOctal(hdr->chksum, sizeof(hdr->chksum), chksum);
}

static int TarWriteHeader(int tar_fd, struct stat *tar_statbuf,
        char *file_name, char *hdr_file_name, struct stat *statbuf)
{
    TarHdr_t hdr;
    int written_len;

    memset(&hdr, 0, sizeof(TarHdr_t));

    strncpy(hdr.name, hdr_file_name, sizeof(hdr.name));

    PrintOctal(hdr.mode, sizeof(hdr.mode), statbuf->st_mode & 07777);
    PrintOctal(hdr.uid, sizeof(hdr.uid), 1);
    PrintOctal(hdr.gid, sizeof(hdr.gid), 1);
    PrintOctal(hdr.mtime, sizeof(hdr.mtime),
            statbuf->st_mtime >= 0 ? statbuf->st_mtime : 0);
    
    strcpy(hdr.uname, "rtthread");
    strcpy(hdr.gname, "rtthread");

    if (S_ISDIR(statbuf->st_mode)) {
        hdr.typeflag = TYPE_DIR;
        if (strlen(hdr.name) < sizeof(hdr.name) - 1)
            hdr.name[strlen(hdr.name)] = '/';
    } else if (S_ISREG(statbuf->st_mode)) {
        hdr.typeflag = TYPE_REG;

        uint64_t size = statbuf->st_size;
        if (size <= 0777777777777)
            PrintOctal(hdr.size, sizeof(hdr.size), size);
        else {
            return -1;
        }
    } else {
        return -1;
    }

    TarChecksumHeader(&hdr);

    written_len = write(tar_fd, &hdr, sizeof(TarHdr_t));
    if (written_len != sizeof(TarHdr_t))
        return -1;

    return 0;
}

static int TarWriteFileRecursive(int tar_fd,
        struct stat *tar_statbuf, char *file_name, int verbose)
{
    char *hdr_file_name;
    struct stat statbuf;

    hdr_file_name = RemoveUnsafePrefix(file_name);
    if (hdr_file_name [0] == '\0')
        return 0;

    stat(file_name, &statbuf);


    if (TarWriteHeader(tar_fd, tar_statbuf, file_name,
            hdr_file_name, &statbuf) < 0)
        return -1;
    
    if (verbose)
        KPrintf("%s\n", file_name);

    if (S_ISREG(statbuf.st_mode)) {
        int fd;
        size_t written_size = 0, copied_size;

        fd = open(file_name, O_RDONLY);
        if (fd < 0)
            return -1;

        while ((copied_size = read(fd, bytebuf, TAR_BLOCK_SIZE)) > 0) {
            if (write(tar_fd, bytebuf, copied_size) != copied_size) {
                copied_size = -1;
                break;
            }
            written_size += copied_size;
        }
        if (copied_size < 0 || written_size != statbuf.st_size) {
            close(fd);
            return -1;
        }

        copied_size = TAR_BLOCK_SIZE - (written_size % TAR_BLOCK_SIZE);
        copied_size %= TAR_BLOCK_SIZE;
        memset(bytebuf, 0, copied_size);
        if (write(tar_fd, bytebuf, copied_size) != copied_size) {
            close(fd);
            return -1;
        }

        close(fd);
    } else {
        DIR *dir;
        struct dirent *dirent;

        dir = opendir(file_name);
        if (dir == NULL)
            return -1;

        while ((dirent = readdir(dir)) != NULL) {
            char *sub_file_name;
            int ret;
            
            sub_file_name = malloc(strlen(file_name) +
                    strlen(dirent->d_name) + 2);
            if (sub_file_name == NULL)
                return -1;

            sprintf(sub_file_name, "%s/%s", file_name, dirent->d_name);
            ret = TarWriteFileRecursive(tar_fd, tar_statbuf, sub_file_name, verbose);

            free(sub_file_name);

            if (ret < 0) {
                closedir(dir);
                return -1;
            }
        }
        closedir(dir);
    }

    return 0;
}

static int TarCreate(char *tar_name, int npaths, char **paths, int verbose)
{
    int tar_fd, written_size;
    struct stat tar_statbuf;

    if (npaths == 0)
        return 0;

    tar_fd = open(tar_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (tar_fd < 0)
        return -1;
    fstat(tar_fd, &tar_statbuf);

    for (int i = 0; i < npaths; i++) {
        int len = strlen(paths[i]);
        if (len > 1 && paths[i][len - 1] == '/')
            paths[len - 1] = '\0';

        if (TarWriteFileRecursive(tar_fd, &tar_statbuf, paths[i], verbose) < 0) {
            close(tar_fd);
            return -1;
        }
    }

    memset(bytebuf, 0, TAR_BLOCK_SIZE);
    written_size = 0;
    for (int i = 0; i < 2; i++)
        written_size += write(tar_fd, bytebuf, TAR_BLOCK_SIZE);

    close(tar_fd);
    
    return written_size == 2 * TAR_BLOCK_SIZE ? 0 : -1;
}

enum {
    RET_SUCC = 0,
    RET_EMPTY_HDR,
    RET_EOF,
};

static int TarParseHeader(int tar_fd, int verbose)
{
    int ReadSize;
    TarHdr_t hdr;


    ReadSize = read(tar_fd, bytebuf, TAR_BLOCK_SIZE);
    if (ReadSize == 0)
        return RET_EOF;
    if (ReadSize != TAR_BLOCK_SIZE)
        return -1;
    
    memcpy(&hdr, bytebuf, TAR_BLOCK_SIZE);

    if (hdr.name[0] == '\0')
        return RET_EMPTY_HDR;

    if (!IsPrefixedWith(hdr.magic, TAR_MAGIC_STR))
        return -1;

    int old_chksum = ScanOctal(hdr.chksum, sizeof(hdr.chksum));
    int correct_chksum = 0;
    memset(hdr.chksum, ' ', sizeof(hdr.chksum));
    for (int i = 0; i < sizeof(TarHdr_t); i++)
        correct_chksum += ((unsigned char *)&hdr)[i];
    if (old_chksum != correct_chksum)
        return -1;

    if (hdr.typeflag != TYPE_REG && hdr.typeflag != TYPE_DIR)
        return -1;

    char *file_name = RemoveUnsafePrefix(hdr.name);

    if (verbose)
        KPrintf("%s\n", file_name);

    if (hdr.typeflag == TYPE_DIR) {
        if (mkdir(file_name, 0777) < 0)
            return -1;
        return RET_SUCC;
    }

    int size_to_write = ScanOctal(hdr.size, sizeof(hdr.size));
    int written_size;

    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0)
        return -1;

    while (size_to_write > 0) {
        ReadSize = read(tar_fd, bytebuf, TAR_BLOCK_SIZE);
        if (ReadSize != TAR_BLOCK_SIZE) {
            close(fd);
            return -1;
        }

        written_size = write(fd, bytebuf, MIN(size_to_write, TAR_BLOCK_SIZE));
        if (written_size != MIN(size_to_write, TAR_BLOCK_SIZE)) {
            close(fd);
            return -1;
        }

        size_to_write -= TAR_BLOCK_SIZE;
    }

    close(fd);

    return RET_SUCC;
}

static int TarExtract(char *tar_name, int verbose)
{
    int tar_fd, ret, empty_hdrs = 0;
    struct stat tar_statbuf;

    if (stat(tar_name, &tar_statbuf) < 0)
        return -1;
    if (tar_statbuf.st_size % TAR_BLOCK_SIZE != 0)
        return -1;

    tar_fd = open(tar_name, O_RDONLY);
    if (tar_fd < 0)
        return -1;

    while ((ret = TarParseHeader(tar_fd, verbose)) >= 0) {
        if (ret == RET_EOF) {
            ret = 0;
            break;
        }
        if (ret == RET_EMPTY_HDR) {
            empty_hdrs++;
            if (empty_hdrs >= 2) {
                ret = 0;
                break;
            }
        }
        else
            empty_hdrs = 0;
    }
    close(tar_fd);

    return ret;
}

void TarTruncateCmpExtension(char *file_name, int filter)
{
    switch (filter) {
    case GZIP:
        TruncateExtension(file_name, ".gz");
        break;
    case BZIP2:
        TruncateExtension(file_name, ".bz2");
        break;
    }
}

extern int GzipCompress(char *file_name);
extern int GzipDecompress(char *file_name);
extern int Bzip2Compress(char *file_name);
extern int Bzip2Decompress(char *file_name);

static void TarPrintUsage()
{
    KPrintf("Usage: tar [OPTIONS]... [FILES]...\n");
    KPrintf("Supported option flags:\n");
    KPrintf("    c   create a new tarball\n");
    KPrintf("    x   extract files from an existing tarball\n");
    KPrintf("    f   specify name of the tarball\n");
    KPrintf("    v   show verbose status\n");
    KPrintf("    z   filter the tarball through gzip\n");
    KPrintf("    j   filter the tarball through bzip2\n");
}

int tar(int argc, char **argv)
{
    int flag_create = 0, flag_extract = 0, flag_file = 0, verbose = 0;
    int filter = 0;
    int ret = 0;
    char *tar_name;

    if (argc < 2 || argv[1][0] != '-') {
        TarPrintUsage();
        return 0;
    }

    for (int i = 1; i < strlen(argv[1]); i++)
        switch(argv[1][i]) {
        case 'c':
            flag_create = 1;
            break;
        case 'x':
            flag_extract = 1;
            break;
        case 'f':
            flag_file = 1;
            break;
        case 'z':
            if (filter != 0) {
                KPrintf("Bad options\n");
                return 0;
            }
            filter = GZIP;
            break;
        case 'j':
            if (filter != 0) {
                KPrintf("Bad options\n");
                return 0;
            }
            filter = BZIP2;
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            KPrintf("Unknown option: %c\n", argv[1][i]);
            return 0;
        }
    if (flag_create == flag_extract || flag_file == 0 || argc < 3) {
        KPrintf("Bad options\n");
        TarPrintUsage();
        return 0;
    }

    bytebuf = malloc(TAR_BLOCK_SIZE);

    tar_name = argv[2];
    if (flag_create) {
        TarTruncateCmpExtension(tar_name, filter);
        ret = TarCreate(tar_name, argc - 3, &argv[3], verbose);
        if (ret == 0) {
            switch (filter) {
            case GZIP:
                ret = GzipCompress(tar_name);
                break;
            case BZIP2:
                ret = Bzip2Compress(tar_name);
                break;
            }
        }
    } else if (flag_extract) {
        if (argc > 3) {
            KPrintf("Too many arguments\n");
            ret = -1;
        } else {
            switch (filter) {
            case GZIP:
                ret = GzipDecompress(tar_name);
                break;
            case BZIP2:
                ret = Bzip2Decompress(tar_name);
            }
            TarTruncateCmpExtension(tar_name, filter);
            if (ret == 0) {
                ret = TarExtract(tar_name, verbose);
            }
        }
    }

    if (ret < 0)
        KPrintf("Operation failed\n");

    free(bytebuf);

    return 0;
}



#endif
