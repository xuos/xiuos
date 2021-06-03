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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iot-vfs_posix.h>
#include <bzip2.h>
#include "bzlib/bzlib.h"
#include "utility.h"

#define IOBUF_SIZE          8192

char *in_buf, *out_buf;


static int BzWrite(BzStream *stream, int ReadSize, int out_fd)

{
    stream->avail_in = ReadSize;
    stream->next_in = in_buf;

    while (1) {
        stream->avail_out = IOBUF_SIZE;
        stream->next_out = out_buf;

        int ret = BZ2BzCompress(stream,
                ReadSize > 0 ? BZ_RUN : BZ_FINISH);

        if (ret != BZ_RUN_OK && ret != BZ_FINISH_OK &&
                ret != BZ_STREAM_END)
            return -1;

        int out_buf_cnt = IOBUF_SIZE - stream->avail_out;
        if (out_buf_cnt) {
            int written_size = write(out_fd, out_buf, out_buf_cnt);
            if (written_size != out_buf_cnt)
                return -1;
        }

        if (ret == BZ_STREAM_END || (ReadSize > 0 && stream->avail_in == 0))
            break;
    }

    return 0;
}

int Bzip2Compress(char *file_name)
{
    int ret = 0;
    char *bz_file_name;
    struct stat statbuf;
    int in_fd, out_fd;
    BzStream stream;

    if ((in_fd = open(file_name, O_RDONLY)) < 0)
        return -1;
    if (fstat(in_fd, &statbuf) < 0 || !S_ISREG(statbuf.st_mode)) {
        ret = -1;
        goto close_in_fd;
    }

    if ((bz_file_name = malloc(strlen(file_name) + 5)) == NULL) {
        ret = -1;
        goto close_in_fd;
    }
    strcpy(bz_file_name, file_name);
    strcat(bz_file_name, ".bz2");

    if ((out_fd = open(bz_file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        ret = -1;
        goto free_bz_file_name;
    }

    if ((in_buf = malloc(2 * IOBUF_SIZE)) == NULL) {
        ret = -1;
        goto close_out_fd;
    }
    out_buf = in_buf + IOBUF_SIZE;

    memset(&stream, 0, sizeof(BzStream));
    BZ2BzCompressInit(&stream, 1);

    while (1) {
        int ReadSize = read(in_fd, in_buf, IOBUF_SIZE);
        if (ReadSize < 0) {
            ret = -1;
            break;
        }

        ret = BzWrite(&stream, ReadSize, out_fd);
        if (ReadSize == 0 || ret < 0)

            break;
    }

    BZ2BzCompressEnd(&stream);

    free(in_buf);
close_out_fd:
    close(out_fd);
free_bz_file_name:
    free(bz_file_name);
close_in_fd:
    close(in_fd);

    return ret;
}

static void Bzip2PrintUsage()
{
    KPrintf("Usage: bzip2 [OPTIONS]... [FILES]...\n");
    KPrintf("Supported option flags:\n");
    KPrintf("    d   decompress bzip2 file(s)\n");
}

extern int Bzip2Decompress(char *file_name);

int bzip2(int argc, char **argv)
{
    int (*bzip2_func)(char *file_name) = Bzip2Compress;
    if (argc == 1) {
        Bzip2PrintUsage();
        return 0;
    }
    
    if (argv[1][0] == '-') {
        if (strcmp(argv[1], "-b") != 0) {
            KPrintf("Bad options\n");
            Bzip2PrintUsage();
            return 0;
        }
        bzip2_func = Bzip2Decompress;
        argv++;
        argc--;
    }

    for (int i = 1; i < argc; i++)
        if (bzip2_func(argv[i]) < 0) {
            KPrintf("Operation failed\n");
            break;
        }

    return 0;
}


#endif
