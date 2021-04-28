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
#include <string.h>
#include <stdio.h>
#include <gzip.h>
#include <stdlib.h>
#include <stdint.h>
#include "utility.h"


#define USE_HEAP_MEM

static const uint8_t extra_lbits[LENGTH_CODES] = {
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 2, 2,
    2, 2, 3, 3, 3, 3, 4,
    4, 4, 4, 5, 5, 5, 5,
    0,
};

static const uint8_t extra_dbits[D_CODES] = {
    0, 0, 0, 0, 1, 1, 2,
    2, 3, 3, 4, 4, 5, 5,
    6, 6, 7, 7, 8, 8, 9,
    9, 10, 10, 11, 11, 12,
    12, 13, 13,
};

static const uint8_t extra_blbits[BL_CODES] = {
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 3, 7,
};

static const uint8_t bl_order[BL_CODES] = {
    16, 17, 18, 0, 8, 7, 9,
    6, 10, 5, 11, 4, 12, 3,
    13, 2, 14, 1, 15,
};

struct globals {
    unsigned char window[2 * WSIZE];
    unsigned char l_buf[LIT_BUFSIZE];
    uint16_t d_buf[DIST_BUFSIZE];
    unsigned char flag_buf[LIT_BUFSIZE / 8];
    unsigned char outbuf[OUTBUF_SIZE];
    uint16_t prev[2 * WSIZE];
#define HEAD (GLB.prev + WSIZE)

    uint32_t bi_buf;
    uint32_t bi_valid;
#define BUF_SIZE 32

    unsigned char flags;
    unsigned char flag_bit;

    int last_lit;
    int last_dist;
    int last_flags;

    int in_fd, out_fd;
    uint32_t crc32;
    int block_start;
    int ins_h;
    int prev_length;
    int strstart;
    int match_start;
    int lookahead;
    int incnt;
    int outcnt;
    int eofile;

    uint16_t heap[HEAP_SIZE];
    int heap_len;
    int heap_max;


    CtDate_t dyn_ltree[HEAP_SIZE];
    CtDate_t dyn_dtree[2 * D_CODES + 1];
    CtDate_t static_ltree[L_CODES + 2];
    CtDate_t static_dtree[D_CODES];
    CtDate_t bl_tree[2 * BL_CODES + 1];

    TreeDesc_t l_desc;
    TreeDesc_t d_desc;
    TreeDesc_t bl_desc;

    uint16_t bl_count[MAX_BITS + 1];

    unsigned char depth[2 * L_CODES + 1];

    unsigned char length_code[MAX_MATCH - MIN_MATCH + 1];
    int base_length[LENGTH_CODES];

    unsigned char dist_code[512];
    int base_dist[D_CODES];

    int opt_len;
    int static_len;
};
#ifdef USE_HEAP_MEM
static struct globals *glbp;
#define GLB (*glbp)
#else
struct globals GLB;
#endif

static void FlushOutbuf()
{
    write(GLB.out_fd, GLB.outbuf, GLB.outcnt);
    GLB.outcnt = 0;
}

static inline void output8(unsigned char u8)
{
    GLB.outbuf[GLB.outcnt++] = u8;
    if (GLB.outcnt == OUTBUF_SIZE)
        FlushOutbuf();
}

static inline void output16(uint32_t u16)
{
    output8(u16);
    output8(u16 >> 8);
}

static inline void output32(uint32_t u32)
{
    output16(u32);
    output16(u32 >> 16);
}

static void UpdateCrc32(unsigned char *buf, size_t len)
{
    for (int i = 0; i < len; i++) {
        GLB.crc32 = GLB.crc32 ^ buf[i];
        for (int j = 0; j < 8; j++)
            GLB.crc32 = (GLB.crc32 >> 1) ^
                    (0xedb88320 & -(GLB.crc32 & 1));
    }
}

static size_t ReadFile(void *buf, size_t size)
{
    size_t ReadSize;

    ReadSize = read(GLB.in_fd, buf, size);
    GLB.incnt += ReadSize;
    UpdateCrc32(buf, ReadSize);


    return ReadSize;
}

static int LongestMatch(int curr_match)
{
    int chain_length = MAX_CHAIN_LEN;
    unsigned char *str = &GLB.window[GLB.strstart];
    unsigned char *match;
    int length;
    int best_length = GLB.prev_length;
    int limit = GLB.strstart > MAX_DIST ? GLB.strstart - MAX_DIST : 0;

    for (chain_length = MAX_CHAIN_LEN; curr_match > limit && chain_length > 0;
            curr_match = GLB.prev[curr_match & WMASK], chain_length--) {
        match = &GLB.window[curr_match];

        if (match[best_length] != str[best_length] || match[0] != str[0]
                || match[1] != str[1])
            continue;

        length = 2;
        while (length < MAX_MATCH && match[length] == str[length])
            length++;

        if (length > best_length) {
            GLB.match_start = curr_match;
            best_length = length;
            if (length >= MAX_MATCH)
                break;
        }
    }

    return best_length;
}

#define D_CODE(dist)           ((dist) < 256 ? GLB.dist_code[dist] : \
                               GLB.dist_code[256 + ((dist) >> 7)])

static int CtTally(int dist, int lc)
{
    GLB.l_buf[GLB.last_lit++] = lc;

    if (dist == 0) {
        GLB.dyn_ltree[lc].freq++;
    } else {
        dist--;

        GLB.dyn_ltree[GLB.length_code[lc] + LITERALS + 1].freq++;
        GLB.dyn_dtree[D_CODE(dist)].freq++;

        GLB.d_buf[GLB.last_dist++] = dist;
        GLB.flags |= GLB.flag_bit;
    }
    GLB.flag_bit <<= 1;

    if ((GLB.last_lit & 0x7) == 0) {
        GLB.flag_buf[GLB.last_flags++] = GLB.flags;
        GLB.flags = 0;
        GLB.flag_bit = 1;
    }

    return (GLB.last_lit == LIT_BUFSIZE - 1 ||
            GLB.last_dist == DIST_BUFSIZE);
}

static void SendBits(uint32_t value, int length)
{
    uint32_t new_buf;

    new_buf = GLB.bi_buf | (value << GLB.bi_valid);

    length += GLB.bi_valid;

    if (length >= BUF_SIZE) {
        value >>= (BUF_SIZE - GLB.bi_valid);
        output32(new_buf);
        new_buf = value;
        length -= BUF_SIZE;
    }
    GLB.bi_buf = new_buf;
    GLB.bi_valid = length;
}

#define SEND_CODE(c, tree)           SendBits(tree[c].code, tree[c].len)

static void CompressBlock(CtDate_t *ltree, CtDate_t *dtree)
{
    int dist;
    int lc;
    int lx = 0;
    int dx = 0;
    int fx = 0;
    unsigned char flag = 0;
    uint32_t code;
    int extra;

    if (GLB.last_lit != 0) {
        do {
            if ((lx & 0x7) == 0)
                flag = GLB.flag_buf[fx++];
            lc = GLB.l_buf[lx++];
            if ((flag & 1) == 0) {
                SEND_CODE(lc, ltree);
            } else {
                code = GLB.length_code[lc];
                SEND_CODE(code + LITERALS + 1, ltree);
                extra = extra_lbits[code];
                if (extra != 0) {
                    lc -= GLB.base_length[code];
                    SendBits(lc, extra);
                }

                dist = GLB.d_buf[dx++];
                code = D_CODE(dist);
                SEND_CODE(code, dtree);
                extra = extra_dbits[code];
                if (extra != 0) {
                    dist -= GLB.base_dist[code];
                    SendBits(dist, extra);
                }
            }
            flag >>= 1;
        } while (lx < GLB.last_lit);
    }

    SEND_CODE(END_BLOCK, ltree);
}

static void SendTree(CtDate_t *tree, int max_code)
{
    int prevlen = -1;
    int currlen;
    int nextlen = tree[0].len;
    int count = 0;
    int max_count = 7;
    int min_count = 4;

    if (nextlen == 0) {
        max_count = 138;
        min_count = 3;
    }

    for (int n = 0; n <= max_code; n++) {
        currlen = nextlen;
        nextlen = tree[n + 1].len;
        if (++count < max_count && currlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do {
                SEND_CODE(currlen, GLB.bl_tree);
            } while (--count);
        } else if (currlen != 0) {
            if (currlen != prevlen) {
                SEND_CODE(currlen, GLB.bl_tree);
                count--;
            }
            SEND_CODE(REP_3_6, GLB.bl_tree);
            SendBits(count - 3, 2);
        } else if (count <= 10) {
            SEND_CODE(REPZ_3_10, GLB.bl_tree);
            SendBits(count - 3, 3);
        } else {
            SEND_CODE(REPZ_11_138, GLB.bl_tree);
            SendBits(count - 11, 7);
        }
        count = 0;
        prevlen = currlen;
        if (nextlen == 0) {
            max_count = 138;
            min_count = 3;
        } else if (currlen == nextlen) {
            max_count = 6;
            min_count = 3;
        } else {
            max_count = 7;
            min_count = 4;
        }
    }
}


static void SendAllTrees(int lcodes, int dcodes, int blcodes)
{
    SendBits(lcodes - 257, 5);
    SendBits(dcodes - 1, 5);
    SendBits(blcodes - 4, 4);

    for (int rank = 0; rank < blcodes; rank++)
        SendBits(GLB.bl_tree[bl_order[rank]].len, 3);

    SendTree(GLB.dyn_ltree, lcodes - 1);
    SendTree(GLB.dyn_dtree, dcodes - 1);
}

static void BiWindup()
{
    uint32_t bits = GLB.bi_buf;
    int cnt = GLB.bi_valid;

    while (cnt > 0) {
        output8(bits);
        bits >>= 8;
        cnt -= 8;
    }
    GLB.bi_buf = 0;
    GLB.bi_valid = 0;
}

static void CopyBlock(unsigned char *buf, size_t len, int with_header)
{
    BiWindup();

    if (with_header) {
        uint32_t v = ((uint16_t)len | ((~len) << 16));
        output32(v);
    }

    for (int i = 0; i < len; i++)
        output8(buf[i]);
}

static void ScanTree(CtDate_t *tree, int max_code)
{
    int n;
    int prevlen = -1;
    int currlen;
    int nextlen = tree[0].len;
    int count = 0;
    int max_count = 7;
    int min_count = 4;

    if (nextlen == 0) {
        max_count = 138;
        min_count = 3;
    }
    tree[max_code + 1].len = 0xffff;

    for (n = 0; n <= max_code; n++) {
        currlen = nextlen;
        nextlen = tree[n + 1].len;
        if (++count < max_count && currlen == nextlen)
            continue;

        if (count < min_count) {
            GLB.bl_tree[currlen].freq += count;
        } else if (currlen != 0) {
            if (currlen != prevlen)
                GLB.bl_tree[currlen].freq++;
            GLB.bl_tree[REP_3_6].freq++;
        } else if (count <= 10) {
            GLB.bl_tree[REPZ_3_10].freq++;
        } else {
            GLB.bl_tree[REPZ_11_138].freq++;
        }
        count = 0;
        prevlen = currlen;

        max_count = 7;
        min_count = 4;
        if (nextlen == 0) {
            max_count = 138;
            min_count = 3;
        } else if (currlen == nextlen) {
            max_count = 6;
            min_count = 3;
        }
    }
}

#define NODE_SMALLER(tree, n, m)             (tree[n].freq < tree[m].freq || \
                                             (tree[n].freq == tree[m].freq && \
                                             GLB.depth[n] <= GLB.depth[m]))

static void HeapAdjust(CtDate_t *tree, int k)
{
    int v = GLB.heap[k];
    int j = k * 2;

    while (j <= GLB.heap_len) {
        if (j < GLB.heap_len &&
                NODE_SMALLER(tree, GLB.heap[j + 1], GLB.heap[j]))
            j++;

        if (NODE_SMALLER(tree, v, GLB.heap[j]))
            break;

        GLB.heap[k] = GLB.heap[j];
        k = j;
        j *= 2;
    }
    GLB.heap[k] = v;
}

#define HEAP_POP(tree, top)        do {                                       \
                                      top = GLB.heap[1];                      \
                                      GLB.heap[1] = GLB.heap[GLB.heap_len--]; \
                                       HeapAdjust(tree, 1);                  \
                                      } while (0)

static void GenBitlen(TreeDesc_t *desc)
{
    CtDate_t *tree = desc->dyn_tree;
    CtDate_t *stree = desc->static_tree;
    const uint8_t *extra = desc->extra_bits;
    int base = desc->extra_base;
    int max_code = desc->max_code;
    int max_length = desc->max_length;
    int h;
    int n, m;
    int bits;
    int xbits;
    uint16_t f;
    int overflow = 0;

    for (bits = 0; bits <= MAX_BITS; bits++)
        GLB.bl_count[bits] = 0;

    tree[GLB.heap[GLB.heap_max]].len = 0;
    for (h = GLB.heap_max + 1; h < HEAP_SIZE; h++) {
        n = GLB.heap[h];
        bits = tree[tree[n].father].len + 1;
        if (bits > max_length) {
            bits = max_length;
            overflow++;
        }
        tree[n].len = bits;

        if (n > max_code)
            continue;

        GLB.bl_count[bits]++;
        xbits = 0;
        if (n >= base)
            xbits = extra[n - base];
        f = tree[n].freq;
        GLB.opt_len += f * (bits + xbits);

        if (stree)
            GLB.static_len += f * (stree[n].len + xbits);
    }
    if (overflow == 0)
        return;

    do {
        bits = max_length - 1;
        while (GLB.bl_count[bits] == 0)
            bits--;

        GLB.bl_count[bits]--;
        GLB.bl_count[bits + 1] += 2;
        GLB.bl_count[max_length]--;

        overflow -= 2;
    } while (overflow > 0);

    for (bits = max_length; bits > 0; bits--) {
        n = GLB.bl_count[bits];
        while (n > 0) {
            m = GLB.heap[--h];
            if (m > max_code)
                continue;
            if (tree[m].len != bits) {
                GLB.opt_len += (int)(bits - tree[m].len) *
                        tree[m].freq;
                tree[m].len = bits;
            }
            n--;
        }
    }
}

static unsigned BiReverse (unsigned code, int len)
{
    unsigned ret = 0;

    while (1) {
        ret |= code & 1;
        if (--len == 0)
            return ret;
        code >>= 1;
        ret <<= 1;
    }
}

static void GenCodes(CtDate_t * tree, int max_code)
{
    uint16_t next_code[MAX_BITS + 1];
    uint16_t code = 0;
    int bits;
    int n;

    for (bits = 1; bits <= MAX_BITS; bits++)
        next_code[bits] = code = (code + GLB.bl_count[bits - 1]) << 1;

    for (n = 0; n <= max_code; n++) {
        int len = tree[n].len;

        if (len == 0)
            continue;
        tree[n].code = BiReverse (next_code[len]++, len);
    }
}

static void BuildTree(TreeDesc_t *desc)
{
    CtDate_t *tree = desc->dyn_tree;
    CtDate_t *stree = desc->static_tree;
    int elems = desc->elems;
    int m, n;
    int max_code = -1;
    int node = elems;

    GLB.heap_len = 0;
    GLB.heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].freq != 0) {
            GLB.heap[++GLB.heap_len] = max_code = n;
            GLB.depth[n] = 0;
        } else {
            tree[n].len = 0;
        }
    }

    while (GLB.heap_len < 2) {
        int new = GLB.heap[++GLB.heap_len] = (max_code < 2 ?
                ++max_code : 0);

        tree[new].freq = 1;
        GLB.depth[new] = 0;
        GLB.opt_len--;
        if (stree)
            GLB.static_len -= stree[new].len;
    }
    desc->max_code = max_code;

    for (n = GLB.heap_len / 2; n >= 1; n--)
        HeapAdjust(tree, n);

    do {
        HEAP_POP(tree, n);
        m = GLB.heap[1];

        GLB.heap[--GLB.heap_max] = n;
        GLB.heap[--GLB.heap_max] = m;

        tree[node].freq = tree[n].freq + tree[m].freq;
        GLB.depth[node] = MAX(GLB.depth[n], GLB.depth[m]) + 1;
        tree[n].father = tree[m].father = node;

        GLB.heap[1] = node++;
        HeapAdjust(tree, 1);
    } while (GLB.heap_len >= 2);

    GLB.heap[--GLB.heap_max] = GLB.heap[1];

    GenBitlen(desc);
    GenCodes(tree, max_code);
}

static int BuildBlTree()
{
    int max_blindex;

    ScanTree(GLB.dyn_ltree, GLB.l_desc.max_code);
    ScanTree(GLB.dyn_dtree, GLB.d_desc.max_code);

    BuildTree(&GLB.bl_desc);

    for (max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex--)
        if (GLB.bl_tree[bl_order[max_blindex]].len != 0)
            break;

    GLB.opt_len += 3 * (max_blindex + 1) + 5 + 5 + 4;

    return max_blindex;
}

static void InitBlock(void)
{
    int n;

    for (n = 0; n < L_CODES; n++)
        GLB.dyn_ltree[n].freq = 0;
    for (n = 0; n < D_CODES; n++)
        GLB.dyn_dtree[n].freq = 0;
    for (n = 0; n < BL_CODES; n++)
        GLB.bl_tree[n].freq = 0;

    GLB.dyn_ltree[END_BLOCK].freq = 1;
    GLB.opt_len = GLB.static_len = 0;
    GLB.last_lit = GLB.last_dist = GLB.last_flags = 0;
    GLB.flags = 0;
    GLB.flag_bit = 1;
}

static void FlushBlock(unsigned char *buf, uint32_t stored_len, int eof)
{
    uint32_t opt_lenb, static_lenb;
    int max_blindex;

    GLB.flag_buf[GLB.last_flags] = GLB.flags;

    BuildTree(&GLB.l_desc);
    BuildTree(&GLB.d_desc);

    max_blindex = BuildBlTree();

    opt_lenb = (GLB.opt_len + 3 + 7) >> 3;
    static_lenb = (GLB.static_len + 3 + 7) >> 3;
    if (static_lenb < opt_lenb)
        opt_lenb = static_lenb;
    if (stored_len + 4 <= opt_lenb && buf != NULL) {
        SendBits((STORED_BLOCK << 1) + eof, 3);
        CopyBlock(buf, stored_len, 1);
    } else if (static_lenb == opt_lenb) {
        SendBits((STATIC_TREES << 1) + eof, 3);
        CompressBlock(GLB.static_ltree, GLB.static_dtree);
    } else {
        SendBits((DYN_TREES << 1) + eof, 3);
        SendAllTrees(GLB.l_desc.max_code + 1,
                GLB.d_desc.max_code + 1, max_blindex + 1);
        CompressBlock(GLB.dyn_ltree, GLB.dyn_dtree);
    }

    InitBlock();

    if (eof)
        BiWindup();
}

#define FLUSH_BLOCK(eof)    FlushBlock(                                           \
                                         GLB.block_start >= 0                      \
                                         ? &GLB.window[GLB.block_start]            \
                                         : NULL,                                   \
                                         (uint32_t)GLB.strstart - GLB.block_start, \
                                         (eof)                                     \
                                        )

static void FillWindow()
{
    uint32_t n, m;
    uint32_t more = 2 * WSIZE - GLB.lookahead - GLB.strstart;

    if (more == (uint32_t)-1) {
        more--;
    } else if (GLB.strstart >= WSIZE + MAX_DIST) {
        memcpy(GLB.window, &GLB.window[WSIZE], WSIZE);

        GLB.match_start -= WSIZE;
        GLB.strstart -= WSIZE;
        GLB.block_start -= WSIZE;
        for (n = 0; n < HASH_SIZE; n++) {
            m = HEAD[n];
            HEAD[n] = m >= WSIZE ? m - WSIZE : 0;
        }
        for (n = 0; n < WSIZE; n++) {
            m = GLB.prev[n];
            GLB.prev[n] = m >= WSIZE ? m - WSIZE : 0;
        }
        more += WSIZE;
    }
    if (!GLB.eofile) {
        n = ReadFile(GLB.window + GLB.strstart +
                GLB.lookahead, more);
        if (n == 0 || n == (unsigned) -1)
            GLB.eofile = 1;
        else
            GLB.lookahead += n;
    }
}

static void FillWindowIfNeeded(void)
{
    while (GLB.lookahead < MIN_LOOKAHEAD && !GLB.eofile)
        FillWindow();
}

#define UPDATE_HASH(h, c)             (h = (((h) << HASH_SHIFT) ^ (c)) & HASH_MASK)

#define INSERT_STRING(s, match_head)    do {                                                          \
                                             UPDATE_HASH(GLB.ins_h, GLB.window[(s) + MIN_MATCH - 1]); \
                                             GLB.prev[(s) & WMASK] = HEAD[GLB.ins_h];                 \
                                             match_head = HEAD[GLB.ins_h];                            \
                                             HEAD[GLB.ins_h] = (s);                                   \
                                            } while (0)

static void deflate()
{
    int hash_head;
    int prev_match;
    int flush;
    int match_available = 0;
    int match_length = MIN_MATCH - 1;

    while (GLB.lookahead) {
        INSERT_STRING(GLB.strstart, hash_head);

        GLB.prev_length = match_length;
        prev_match = GLB.match_start;
        match_length = MIN_MATCH - 1;

        if (hash_head && GLB.prev_length < MAX_LAZY_MATCH &&
                GLB.strstart - hash_head <= MAX_DIST) {
            match_length = LongestMatch(hash_head);
            if (match_length > GLB.lookahead)
                match_length = GLB.lookahead;

            if (match_length == MIN_MATCH &&
                    GLB.strstart - GLB.match_start > 4096)
                match_length--;
        }

        if (GLB.prev_length >= MIN_MATCH &&
                match_length <= GLB.prev_length) {
            flush = CtTally(GLB.strstart - 1 - prev_match,
                    GLB.prev_length - MIN_MATCH);

            GLB.lookahead -= GLB.prev_length - 1;
            for (GLB.prev_length -= 2; GLB.prev_length > 0;
                    GLB.prev_length--) {
                GLB.strstart++;
                INSERT_STRING(GLB.strstart, hash_head);
            }
            match_available = 0;
            match_length = MIN_MATCH - 1;
            GLB.strstart++;
            if (flush) {
                FLUSH_BLOCK(0);
                GLB.block_start = GLB.strstart;
            }
        } else if (match_available) {
            if (CtTally(0, GLB.window[GLB.strstart - 1])) {
                FLUSH_BLOCK(0);
                GLB.block_start = GLB.strstart;
            }
            GLB.strstart++;
            GLB.lookahead--;
        } else {
            match_available = 1;
            GLB.strstart++;
            GLB.lookahead--;
        }

        FillWindowIfNeeded();
    }
    if (match_available)
        CtTally(0, GLB.window[GLB.strstart - 1]);

    FLUSH_BLOCK(1);
}

static void CtInit()
{
    int n;
    int length;
    int code;
    int dist;

    length = 0;
    for (code = 0; code < LENGTH_CODES - 1; code++) {
        GLB.base_length[code] = length;
        for (n = 0; n < (1 << extra_lbits[code]); n++) {
            GLB.length_code[length++] = code;
        }
    }

    GLB.length_code[length - 1] = code;

    dist = 0;
    for (code = 0; code < 16; code++) {
        GLB.base_dist[code] = dist;
        for (n = 0; n < (1 << extra_dbits[code]); n++) {
            GLB.dist_code[dist++] = code;
        }
    }
    dist >>= 7;
    for (; code < D_CODES; code++) {
        GLB.base_dist[code] = dist << 7;
        for (n = 0; n < (1 << (extra_dbits[code] - 7)); n++) {
            GLB.dist_code[256 + dist++] = code;
        }
    }

    n = 0;
    while (n <= 143)
        GLB.static_ltree[n++].len = 8;
    while (n <= 255)
        GLB.static_ltree[n++].len = 9;
    while (n <= 279)
        GLB.static_ltree[n++].len = 7;
    while (n <= 287)
        GLB.static_ltree[n++].len = 8;
    GLB.bl_count[7] = 279 - 255;
    GLB.bl_count[8] = (143 + 1) + (287 - 279);
    GLB.bl_count[9] = 255 - 143;

    GenCodes((CtDate_t *)GLB.static_ltree, L_CODES + 1);

    for (n = 0; n < D_CODES; n++) {
        GLB.static_dtree[n].len = 5;
        GLB.static_dtree[n].code = BiReverse (n, 5);
    }

    InitBlock();
}

static void LmInit()
{
    unsigned j;

    memset(HEAD, 0, (1 << HASH_BITS) * sizeof(*HEAD));

    GLB.lookahead = ReadFile(GLB.window,
            sizeof(int) <= 2 ? (unsigned) WSIZE : 2 * WSIZE);

    if (GLB.lookahead == 0 || GLB.lookahead == (unsigned) -1) {
        GLB.eofile = 1;
        GLB.lookahead = 0;
        return;
    }

    FillWindowIfNeeded();

    for (j = 0; j < MIN_MATCH - 1; j++)
        UPDATE_HASH(GLB.ins_h, GLB.window[j]);
}

int GzipCompress(char *file_name)
{
    int ret = 0;
    char *gz_file_name;
    struct stat statbuf;

#ifdef USE_HEAP_MEM
    glbp = malloc(sizeof(struct globals));
    if (glbp == NULL) {
        KPrintf("Memory too small\n");
        return -1;
    }
#endif

    memset(&GLB, 0, sizeof(struct globals));

    if ((GLB.in_fd = open(file_name, O_RDONLY)) < 0) {
        ret = -1;
        goto free_glbp;
    }

    fstat(GLB.in_fd, &statbuf);
    if (!S_ISREG(statbuf.st_mode)) {
        ret = -1;
        goto free_glbp;
    }

    if ((gz_file_name = malloc(strlen(file_name) + 4)) == NULL) {
        ret = -1;
        goto close_in_fd;
    }
    strcpy(gz_file_name, file_name);
    strcat(gz_file_name, ".gz");

    if ((GLB.out_fd =
            open(gz_file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        ret = -1;
        goto free_gz_file_name;
    }

    GLB.crc32 = ~0;
    GLB.l_desc.dyn_tree = GLB.dyn_ltree;
    GLB.l_desc.static_tree = GLB.static_ltree;
    GLB.l_desc.extra_bits = extra_lbits;
    GLB.l_desc.extra_base = LITERALS + 1;
    GLB.l_desc.elems = L_CODES;
    GLB.l_desc.max_length = MAX_BITS;
    GLB.d_desc.dyn_tree = GLB.dyn_dtree;
    GLB.d_desc.static_tree = GLB.static_dtree;
    GLB.d_desc.extra_bits = extra_dbits;
    GLB.d_desc.elems = D_CODES;
    GLB.d_desc.max_length = MAX_BITS;
    GLB.bl_desc.dyn_tree = GLB.bl_tree;
    GLB.bl_desc.extra_bits = extra_blbits,
    GLB.bl_desc.elems = BL_CODES;
    GLB.bl_desc.max_length = MAX_BL_BITS;
    CtInit();
    LmInit();

    output32(0x00088b1f);
    output32(0x0);
    output16(0x2 | 0x300);
    FlushOutbuf();

    deflate();

    output32(~GLB.crc32);
    output32(GLB.incnt);
    FlushOutbuf();

    if (GLB.incnt != statbuf.st_size)
        ret = -1;

    close(GLB.out_fd);
free_gz_file_name:
    free(gz_file_name);
close_in_fd:
    close(GLB.in_fd);
free_glbp:
#ifdef USE_HEAP_MEM
    free(glbp);
#endif

    return ret;
}

extern int GzipDecompress(char *file_name);

static void GzipPrintUsage()
{
    KPrintf("Usage: gzip [OPTIONS]... [FILES]...\n");
    KPrintf("Supported option flags:\n");
    KPrintf("    d   decompress gzip file(s)\n");
}

int gzip(int argc, char **argv)
{
    int (*gzip_func)(char *file_name) = GzipCompress;

    if (argc < 2) {
        GzipPrintUsage();
        return 0;
    }

    if (argv[1][0] == '-') {
        if (strlen(argv[1]) != 2 || argv[1][1] != 'd') {
            KPrintf("Bad options\n");
            GzipPrintUsage();
            return 0;
        }
        gzip_func = GzipDecompress;
        argv++;
        argc--;
    }

    for (int i = 1; i < argc; i++)
        if (gzip_func(argv[i]) < 0) {
            KPrintf("Operation failed\n");
            break;
        }

    return 0;
}


#endif
