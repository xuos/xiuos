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
#include <string.h>
#include <stdlib.h>
#include <iot-vfs_posix.h>
#include "utility.h"


#define MAX_GROUPS          6
#define GROUP_SIZE          50
#define MAX_HUFCODE_BITS    20
#define MAX_SYMBOLS         258
#define SYMBOL_RUNA         0
#define SYMBOL_RUNB         1

#define BZIP2_MAGIC         ('B' + 'Z' * 256)

#define IOBUF_SIZE           4096

#define RET_ERROR            -1
#define RET_LASTBLOCK        -2


struct GroupData {
    int32_t limit[MAX_HUFCODE_BITS+1], base[MAX_HUFCODE_BITS], permute[MAX_SYMBOLS];
    int minLen, maxLen;
};


typedef struct BunZipData {

    uint32_t inbufBitCount, inbufBits;
    int in_fd, out_fd, inbufCount, inbufPos;
    uint8_t *inbuf;

    int writeCopies, writePos, writeRunCountdown, writeCount;
    int writeCurrent;

    uint32_t headerCRC, totalCRC, writeCRC;

    uint32_t *dbuf;
    uint32_t dbufSize;

    uint32_t crc32Table[256];
    uint8_t selectors[32768];
    struct GroupData groups[MAX_GROUPS];
} BunZipData_t;


static int GetBits(BunZipData_t *bd, int bits_wanted, uint32_t *bits)
{
    uint32_t b = 0;

    int bit_count = bd->inbufBitCount;

    while (bit_count < bits_wanted) {

        if (bd->inbufPos == bd->inbufCount) {
            bd->inbufCount = read(bd->in_fd, bd->inbuf, IOBUF_SIZE);
            if (bd->inbufCount <= 0)
                return -1;
            bd->inbufPos = 0;
        }

        if (bit_count >= 24) {
            b = bd->inbufBits & ((1U << bit_count) - 1);
            bits_wanted -= bit_count;
            b <<= bits_wanted;
            bit_count = 0;
        }

        bd->inbufBits = (bd->inbufBits << 8) | bd->inbuf[bd->inbufPos++];
        bit_count += 8;
    }

    bit_count -= bits_wanted;
    bd->inbufBitCount = bit_count;
    b |= (bd->inbufBits >> bit_count) & ((1 << bits_wanted) - 1);
    *bits = b;

    return 0;
}

static int GetNextBlock(BunZipData_t *bd)
{
    int groupCount, selector,
        i, j, symCount, symTotal, nSelectors, byteCount[256];
    uint8_t uc, symToByte[256], mtfSymbol[256], *selectors;
    uint32_t *dbuf;
    uint32_t origPtr, t;
    uint32_t dbufCount, runPos;
    uint32_t runCnt = runCnt;
    uint32_t tmp;

    dbuf = bd->dbuf;
    selectors = bd->selectors;

    if (GetBits(bd, 24, (uint32_t *)&i) < 0)
        return RET_ERROR;
    if (GetBits(bd, 24, (uint32_t *)&j) < 0)
        return RET_ERROR;
    if (GetBits(bd, 32, &bd->headerCRC) < 0)
        return RET_ERROR;
    if ((i == 0x177245) && (j == 0x385090))
        return RET_LASTBLOCK;
    if ((i != 0x314159) || (j != 0x265359))
        return RET_ERROR;

    if (GetBits(bd, 1, &tmp) < 0)
        return RET_ERROR;
    if (tmp)
        return RET_ERROR;
    if (GetBits(bd, 24, (uint32_t *)&origPtr) < 0)
        return RET_ERROR;
    if (origPtr > bd->dbufSize)
        return RET_ERROR;

    symTotal = 0;
    i = 0;
    if (GetBits(bd, 16, &t) < 0)
        return RET_ERROR;
    do {
        if (t & (1 << 15)) {
            uint32_t inner_map;
            if (GetBits(bd, 16, &inner_map) < 0)
                return RET_ERROR;
            do {
                if (inner_map & (1 << 15))
                    symToByte[symTotal++] = i;
                inner_map <<= 1;
                i++;
            } while (i & 15);
            i -= 16;
        }
        t <<= 1;
        i += 16;
    } while (i < 256);

    if (GetBits(bd, 3, (uint32_t *)&groupCount) < 0)
        return RET_ERROR;
    if (groupCount < 2 || groupCount > MAX_GROUPS)
        return RET_ERROR;


    for (i = 0; i < groupCount; i++)
        mtfSymbol[i] = i;
    if  (GetBits(bd, 15, (uint32_t *)&nSelectors) < 0)
        return RET_ERROR;
    if (!nSelectors)
        return RET_ERROR;
    for (i = 0; i < nSelectors; i++) {
        uint8_t tmp_byte;

        int n = 0;
        while (GetBits(bd, 1, &tmp) == 0 && tmp) {
            n++;
            if (n >= groupCount)
                return RET_ERROR;
        }

        tmp_byte = mtfSymbol[n];
        while (--n >= 0)
            mtfSymbol[n + 1] = mtfSymbol[n];
        mtfSymbol[0] = selectors[i] = tmp_byte;
    }


    symCount = symTotal + 2;
    for (j = 0; j < groupCount; j++) {
        uint8_t length[MAX_SYMBOLS];

        uint32_t temp[MAX_HUFCODE_BITS+1];
        struct GroupData *hufGroup;
        int32_t *base, *limit;
        int minLen, maxLen, pp, len_m1;


        if (GetBits(bd, 5, (uint32_t *)&len_m1) < 0)
            return RET_ERROR;
        len_m1 -= 1;
        for (i = 0; i < symCount; i++) {
            for (;;) {
                int two_bits;
                if ((uint32_t)len_m1 > (MAX_HUFCODE_BITS - 1))
                    return RET_ERROR;

                if  (GetBits(bd, 2, (uint32_t *)&two_bits) < 0)
                    return RET_ERROR;
                if (two_bits < 2) {
                    bd->inbufBitCount++;
                    break;
                }

                len_m1 += (((two_bits+1) & 2) - 1);
            }
            length[i] = len_m1 + 1;
        }

        minLen = maxLen = length[0];
        for (i = 1; i < symCount; i++) {
            if (length[i] > maxLen)
                maxLen = length[i];
            else if (length[i] < minLen)
                minLen = length[i];
        }

        hufGroup = bd->groups + j;
        hufGroup->minLen = minLen;
        hufGroup->maxLen = maxLen;

        base = &hufGroup->base[0] - 1;
        limit = &hufGroup->limit[0] - 1;

        pp = 0;
        for (i = minLen; i <= maxLen; i++) {
            int k;
            temp[i] = limit[i] = 0;
            for (k = 0; k < symCount; k++)
                if (length[k] == i)
                    hufGroup->permute[pp++] = k;
        }

        for (i = 0; i < symCount; i++)
            temp[length[i]]++;

        pp = t = 0;
        for (i = minLen; i < maxLen;) {
            uint32_t temp_i = temp[i];

            pp += temp_i;

            limit[i] = (pp << (maxLen - i)) - 1;
            pp <<= 1;
            t += temp_i;
            base[++i] = pp - t;
        }
        limit[maxLen] = pp + temp[maxLen] - 1;
        limit[maxLen+1] = INT32_MAX;
        base[minLen] = 0;
    }

    for (i = 0; i < 256; i++) {
        byteCount[i] = 0;
        mtfSymbol[i] = (uint8_t)i;
    }

    runPos = dbufCount = selector = 0;
    for (;;) {
        struct GroupData *hufGroup;
        int *base, *limit;
        int nextSym;
        uint8_t ngrp;

        symCount = GROUP_SIZE - 1;
        if (selector >= nSelectors)
            return RET_ERROR;
        ngrp = selectors[selector++];
        if (ngrp >= groupCount)
            return RET_ERROR;
        hufGroup = bd->groups + ngrp;
        base = hufGroup->base - 1;
        limit = hufGroup->limit - 1;

 continue_this_group:

        if (1) {

            int new_cnt;
            while ((new_cnt = bd->inbufBitCount - hufGroup->maxLen) < 0) {

                if (bd->inbufPos == bd->inbufCount) {
                    if (GetBits(bd, hufGroup->maxLen, (uint32_t *)&nextSym) < 0)
                        return RET_ERROR;
                    goto got_huff_bits;
                }
                bd->inbufBits = (bd->inbufBits << 8) | bd->inbuf[bd->inbufPos++];
                bd->inbufBitCount += 8;
            };
            bd->inbufBitCount = new_cnt;
            nextSym = (bd->inbufBits >> new_cnt) & ((1 << hufGroup->maxLen) - 1);
 got_huff_bits: ;
        } else {
            if  (GetBits(bd, hufGroup->maxLen, (uint32_t *)&nextSym) < 0)
                return RET_ERROR;
        }

        i = hufGroup->minLen;
        while (nextSym > limit[i])
            ++i;
        j = hufGroup->maxLen - i;
        if (j < 0)
            return RET_ERROR;
        bd->inbufBitCount += j;


        nextSym = (nextSym >> j) - base[i];
        if ((unsigned)nextSym >= MAX_SYMBOLS)
            return RET_ERROR;
        nextSym = hufGroup->permute[nextSym];


        if ((unsigned)nextSym <= SYMBOL_RUNB) {


            if (runPos == 0) {
                runPos = 1;
                runCnt = 0;
            }

            runCnt += (runPos << nextSym);
            if (runPos < bd->dbufSize) runPos <<= 1;
            goto end_of_huffman_loop;
        }

        if (runPos != 0) {
            uint8_t tmp_byte;
            if (dbufCount + runCnt > bd->dbufSize)
                return RET_ERROR;
            tmp_byte = symToByte[mtfSymbol[0]];
            byteCount[tmp_byte] += runCnt;
            while ((int)--runCnt >= 0)
                dbuf[dbufCount++] = (uint32_t)tmp_byte;
            runPos = 0;
        }

        if (nextSym > symTotal)
            break;
        if (dbufCount >= bd->dbufSize)
            return RET_ERROR;
        i = nextSym - 1;
        uc = mtfSymbol[i];

        do {
            mtfSymbol[i] = mtfSymbol[i - 1];
        } while (--i);
        mtfSymbol[0] = uc;
        uc = symToByte[uc];

        byteCount[uc]++;
        dbuf[dbufCount++] = (uint32_t)uc;

 end_of_huffman_loop:
        if (--symCount >= 0) goto continue_this_group;
    }

    j = 0;
    for (i = 0; i < 256; i++) {
        int tmp_count = j + byteCount[i];
        byteCount[i] = j;
        j = tmp_count;
    }

    for (i = 0; i < dbufCount; i++) {
        uint8_t tmp_byte = (uint8_t)dbuf[i];
        int tmp_count = byteCount[tmp_byte];
        dbuf[tmp_count] |= (i << 8);
        byteCount[tmp_byte] = tmp_count + 1;
    }

    if (dbufCount) {
        uint32_t tmp;
        if ((int)origPtr >= dbufCount)
            return RET_ERROR;
        tmp = dbuf[origPtr];
        bd->writeCurrent = (uint8_t)tmp;
        bd->writePos = (tmp >> 8);
        bd->writeRunCountdown = 5;
    }
    bd->writeCount = dbufCount;

    return 0;
}

static int ReadBunzip(BunZipData_t *bd, char *outbuf, int len)
{
    const uint32_t *dbuf;
    int pos, current, previous;
    uint32_t CRC;


    if (bd->writeCount < 0)
        return bd->writeCount;

    dbuf = bd->dbuf;


    pos = bd->writePos;
    current = bd->writeCurrent;
    CRC = bd->writeCRC;


    if (bd->writeCopies) {

 dec_writeCopies:

        --bd->writeCopies;

        for (;;) {


            if (--len < 0) {

                goto outbuf_full;
            }


            *outbuf++ = current;
            CRC = (CRC << 8) ^ bd->crc32Table[(CRC >> 24) ^ current];


            if (bd->writeCopies) {

                goto dec_writeCopies;
            }
 decode_next_byte:
            if (--bd->writeCount < 0)
                break;

            previous = current;
            pos = dbuf[pos];
            current = (uint8_t)pos;
            pos >>= 8;

            if (--bd->writeRunCountdown != 0) {
                if (current != previous)
                    bd->writeRunCountdown = 4;
            } else {

                bd->writeCopies = current;
                current = previous;
                bd->writeRunCountdown = 5;


                if (!bd->writeCopies) goto decode_next_byte;


                --bd->writeCopies;
            }
        }


        bd->writeCRC = CRC = ~CRC;
        bd->totalCRC = ((bd->totalCRC << 1) | (bd->totalCRC >> 31)) ^ CRC;

        if (CRC != bd->headerCRC) {
            bd->totalCRC = bd->headerCRC + 1;
            return RET_LASTBLOCK;
        }
    }

    {
        int r = GetNextBlock(bd);
        if (r) {
            bd->writeCount = r;
            return (r != RET_LASTBLOCK) ? r : len;
        }
    }

    CRC = ~0;
    pos = bd->writePos;
    current = bd->writeCurrent;
    goto decode_next_byte;

 outbuf_full:
    bd->writePos = pos;
    bd->writeCurrent = current;
    bd->writeCRC = CRC;

    bd->writeCopies++;

    return 0;
}

static void Crc32FilltableBigEndian(uint32_t *crc_table)
{
    uint32_t polynomial = 0x04c11db7;
    uint32_t c;
    unsigned i, j;

    for (i = 0; i < 256; i++) {
        c = i << 24;
        for (j = 8; j; j--) {
            c = (c & 0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
        }
        *crc_table++ = c;
    }
}

static int StartBunzip(BunZipData_t **bdp, int in_fd,
        const void *inbuf, int len)
{
    BunZipData_t *bd;
    uint32_t i;
    enum {
        BZh0 = ('B' << 24) + ('Z' << 16) + ('h' << 8) + '0',
        h0 = ('h' << 8) + '0',
    };

    i = sizeof(BunZipData_t);
    if (in_fd != -1)
        i += IOBUF_SIZE;

    bd = *bdp = malloc(i);
    if (bd == NULL)
        return RET_ERROR;
    memset(bd, 0, sizeof(*bd));

    bd->in_fd = in_fd;
    if (in_fd == -1) {
        bd->inbuf = (void *)inbuf;
    } else {
        bd->inbuf = (uint8_t *)(bd + 1);
        memcpy(bd->inbuf, inbuf, len);
    }
    bd->inbufCount = len;

    Crc32FilltableBigEndian(bd->crc32Table);

    if (GetBits(bd, 16, (uint32_t *)&i) < 0)
        return RET_ERROR;
    if ((unsigned)(i - h0 - 1) >= 9)
        return RET_ERROR;

    bd->dbufSize = 100000 * (i - h0);

    bd->dbuf = malloc(bd->dbufSize * sizeof(bd->dbuf[0]));
    if (!bd->dbuf) {
        free(bd);
        return RET_ERROR;
    }
    return 0;
}

int Bzip2Decompress(char *file_name)
{
    BunZipData_t *bd;
    char *out_buf;
    int i;
    uint32_t len;
    int in_fd, out_fd;
    uint16_t magic;
    int ret = 0;

    if ((in_fd = open(file_name, O_RDONLY)) < 0)
        return RET_ERROR;

    if (TruncateExtension(file_name, ".bz2") < 0) {
        ret = RET_ERROR;
        goto close_in_fd;
    }
    if ((out_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        ret = RET_ERROR;
        goto close_in_fd;
    }

    if ((out_buf = malloc(IOBUF_SIZE)) == NULL) {
        ret = RET_ERROR;
        goto close_out_fd;
    }

    if (read(in_fd, &magic, 2) != 2 || magic != BZIP2_MAGIC) {
        ret = -1;
        goto free_out_buf;
    }

    len = 0;
    while (1) {
        i = StartBunzip(&bd, in_fd, out_buf + 2, len);
        if (i == 0) {
            while (1) {
                i = ReadBunzip(bd, out_buf, IOBUF_SIZE);
                if (i < 0)
                    break;
                i = IOBUF_SIZE - i;
                if (i == 0)
                    break;
                if (write(out_fd, out_buf, i) != i) {
                    ret = RET_ERROR;
                    goto free_bd;
                }
            }
        }

        if (i != 0 && i != RET_LASTBLOCK)
            break;
        if (bd->headerCRC != bd->totalCRC)
            break;

        i = 0;

        len = bd->inbufCount - bd->inbufPos;
        memcpy(out_buf, &bd->inbuf[bd->inbufPos], len);
        if (len < 2) {
            if (read(in_fd, out_buf + len, 2 - len) != 2 - len)
                break;
            len = 2;
        }
        if (*(uint16_t *)out_buf != BZIP2_MAGIC)
            break;
        free(bd->dbuf);
        free(bd);
        len -=2;
    }

free_bd:
    free(bd->dbuf);
    free(bd);
free_out_buf:
    free(out_buf);
close_out_fd:
    close(out_fd);
close_in_fd:
    close(in_fd);

    return ret;
}

void Bunzip2PrintUsage()
{
    KPrintf("Usage: bunzip2 [FILES]....\n");
    KPrintf("File names MUST end with the .bz2 extension\n");
}

int bunzip2(int argc, char **argv)
{
    if (argc < 2) {
        Bunzip2PrintUsage();
        return 0;
    }

    for (int i = 1; i < argc; i++)
        if (Bzip2Decompress(argv[i]) < 0)
            KPrintf("Operation failed: %s\n", argv[i]);

    return 0;
}



#endif
