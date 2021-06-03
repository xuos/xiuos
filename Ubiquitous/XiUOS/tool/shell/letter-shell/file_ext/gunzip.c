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
#include <string.h>
#include <iot-vfs_posix.h>
#include <gunzip.h>
#include "utility.h"

#define      USE_HEAP_MEM

struct globals {
    uint32_t crc32;

    int in_fd;
    int out_fd;
    size_t outcnt;
    size_t outbuf_cnt;

    unsigned char window[GUNZIP_WSIZE];
    unsigned char bytebuffer[GUNZIP_BYTEBUFFER_MAX];
    uint32_t bytebuffer_offset;
    uint32_t bytebuffer_size;
    uint32_t to_read;

    uint32_t bi_buf;
    uint32_t bi_valid;


    uint32_t inflate_codes_ml;
    uint32_t inflate_codes_md;
    uint32_t inflate_codes_bb;
    uint32_t inflate_codes_k;
    uint32_t inflate_codes_w;
    uint32_t inflate_codes_bl;
    uint32_t inflate_codes_bd;
    uint32_t inflate_codes_nn;
    uint32_t inflate_codes_dd;
    huft_t *inflate_codes_tl;
    huft_t *inflate_codes_td;

    int resume_copy;


    int method;
    int need_another_block;
    int eof;

    uint32_t inflate_stored_n;
    uint32_t inflate_stored_b;
    uint32_t inflate_stored_k;
    uint32_t inflate_stored_w;
};

#ifdef USE_HEAP_MEM
static struct globals *glbp;
#define GLB          (*glbp)
#else
struct globals GLB;
#endif

static const uint16_t mask_bits[] = {
    0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

static const uint16_t cplens[] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
    67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

static const uint8_t cplext[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5,
    5, 5, 5, 0, 99, 99
};

static const uint16_t cpdist[] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
    769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};


static const uint8_t cpdext[] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
    11, 11, 12, 12, 13, 13
};

static const uint8_t border[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static void InitInflateCodes(uint32_t bl, uint32_t bd)
{
    GLB.inflate_codes_bl = bl;
    GLB.inflate_codes_bd = bd;
    GLB.inflate_codes_bb = GLB.bi_buf;
    GLB.inflate_codes_k = GLB.bi_valid;
    GLB.inflate_codes_w = GLB.outbuf_cnt;
    GLB.inflate_codes_ml = mask_bits[bl];
    GLB.inflate_codes_md = mask_bits[bd];
}

static int InflateCodes();

static void HuftFree(huft_t *p)
{
    huft_t *q;

    while (p) {
        q = (--p)->t;
        free(p);
        p = q;
    }
}

static int HuftBuild(const uint32_t *b,  const uint32_t n,
        const uint32_t s, const uint16_t *d,
        const unsigned char *e, huft_t **t, uint32_t *m)
{
    uint32_t a;
    uint32_t c[BMAX + 1];
    uint32_t eob_len;
    uint32_t f;
    int g;
    int htl;
    uint32_t i;
    uint32_t j;
    int k;
    const uint32_t *p;
    huft_t *q;
    huft_t r;
    huft_t *u[BMAX];
    uint32_t v[NMAX + 1];
    int ws[BMAX + 1];
    int w;
    uint32_t x[BMAX + 1];
    uint32_t *xp;
    int y;
    uint32_t z;

    eob_len = n > 256 ? b[256] : BMAX;

    *t = NULL;

    memset(c, 0, sizeof(c));
    p = b;
    i = n;
    do {
        c[*p]++;
        p++;
    } while (--i);
    if (c[0] == n) {
        q = malloc(3 * sizeof(*q));
        memset(q, 0, 3 * sizeof(*q));
        q[1].e = 99;
        q[1].b = 1;
        q[2].e = 99;
        q[2].b = 1;
        *t = q + 1;
        *m = 1;
        return 0;
    }

    for (j = 1; (j <= BMAX) && (c[j] == 0); j++)
        continue;
    k = j;
    for (i = BMAX; (c[i] == 0) && i; i--)
        continue;
    g = i;
    *m = (*m < j) ? j : ((*m > i) ? i : *m);

    for (y = 1 << j; j < i; j++, y <<= 1) {
        y -= c[j];
        if (y < 0)
            return -1;
    }
    y -= c[i];
    if (y < 0)
        return -1;
    c[i] += y;

    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i) {
        j += *p++;
        *xp++ = j;
    }

    memset(v, 0xff, sizeof(v));
    p = b;
    i = 0;
    do {
        j = *p++;
        if (j != 0) {
            v[x[j]++] = i;
        }
    } while (++i < n);

    x[0] = i = 0;
    p = v;
    htl = -1;
    w = ws[0] = 0;
    u[0] = NULL;
    q = NULL;
    z = 0;

    for (; k <= g; k++) {
        a = c[k];
        while (a--) {

            while (k > ws[htl + 1]) {
                w = ws[++htl];

                z = g - w;
                z = z > *m ? *m : z;
                j = k - w;
                f = 1 << j;
                if (f > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    while (++j < z) {
                        f <<= 1;
                        if (f <= *++xp) {
                            break;
                        }
                        f -= *xp;
                    }
                }
                j = (w + j > eob_len && w < eob_len) ? eob_len - w : j;
                z = 1 << j;
                ws[htl+1] = w + j;

                q = malloc((z + 1) * sizeof(huft_t));
                memset(q, 0, (z + 1) * sizeof(huft_t));
                *t = q + 1;
                t = &(q->t);
                u[htl] = ++q;


                if (htl) {
                    x[htl] = i;
                    r.b = (unsigned char)(w - ws[htl - 1]);
                    r.e = (unsigned char)(16 + j);
                    r.t = q;
                    j = (i & ((1 << w) - 1)) >> ws[htl - 1];
                    u[htl - 1][j] = r;
                }
            }

            r.b = (unsigned char)(k - w);
            if (*p == 0xffffffff) {
                r.e = 99;
            } else if (*p < s) {
                r.e = (unsigned char)(*p < 256 ? 16 : 15);
                r.n = (unsigned short)(*p++);
            } else {
                r.e = (unsigned char)e[*p - s];
                r.n = d[*p++ - s];
            }

            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f) {
                q[j] = r;
            }

            for (j = 1 << (k - 1); i & j; j >>= 1) {
                i ^= j;
            }
            i ^= j;

            while ((i & ((1 << w) - 1)) != x[htl]) {
                w = ws[--htl];
            }
        }
    }
    *m = ws[1];

    return y != 0 && g != 1 ? -1 : 0;
}

int FillBitBuffer(uint32_t *bitbuffer, uint32_t *current_len,
        uint32_t required_len)
{
    while (*current_len < required_len) {
        if (GLB.bytebuffer_offset >= GLB.bytebuffer_size) {

            uint32_t size = GUNZIP_BYTEBUFFER_MAX - 4;
            if (GLB.to_read >= 0 && GLB.to_read < size)
                size = GLB.to_read;

            GLB.bytebuffer_size = read(GLB.in_fd, &GLB.bytebuffer[4], size);
            if (GLB.bytebuffer_size == 0) {

                return -1;
            }
            if (GLB.to_read >= 0)
                GLB.to_read -= GLB.bytebuffer_size;
            GLB.bytebuffer_size += 4;
            GLB.bytebuffer_offset = 4;
        }
        *bitbuffer |= (uint32_t)GLB.bytebuffer[GLB.bytebuffer_offset]
                << *current_len;
        GLB.bytebuffer_offset++;
        *current_len += 8;
    }

    return 0;
}


static int InflateBlock()
{
    uint32_t ll[286 + 30];
    uint32_t t;
    uint32_t b;
    uint32_t k;

    b = GLB.bi_buf;
    k = GLB.bi_valid;

    if (FillBitBuffer(&b, &k, 1) < 0)
        return -1;
    GLB.eof = b & 1;
    b >>= 1;
    k -= 1;


    if (FillBitBuffer(&b, &k, 2) < 0)
        return -1;
    t = b & 0x3;
    b >>= 2;
    k -= 2;

    GLB.bi_buf = b;
    GLB.bi_valid = k;

    switch (t) {
    case 0: {

        uint32_t n;
        uint32_t b_stored;
        uint32_t k_stored;

        b_stored = GLB.bi_buf;
        k_stored = GLB.bi_valid;

        n = k_stored & 0x7;
        b_stored >>= n;
        k_stored -= n;

        if (FillBitBuffer(&b_stored, &k_stored, 16) < 0)
            return -1;
        n = b_stored & 0xffff;
        b_stored >>= 16;
        k_stored -= 16;

        if (FillBitBuffer(&b_stored, &k_stored, 16) < 0)
            return -1;
        if (n != ((~b_stored) & 0xffff))
            return -1;
        b_stored >>= 16;
        k_stored -= 16;

        GLB.inflate_stored_n = n;
        GLB.inflate_stored_b = b_stored;
        GLB.inflate_stored_k = k_stored;

        return STORED;
    }
    case 1: {

        int i;
        uint32_t bl;
        uint32_t bd;

        for (i = 0; i < 144; i++)
            ll[i] = 8;
        for (; i < 256; i++)
            ll[i] = 9;
        for (; i < 280; i++)
            ll[i] = 7;
        for (; i < 288; i++)
            ll[i] = 8;
        bl = 7;
        HuftBuild(ll, 288, 257, cplens, cplext,
                &GLB.inflate_codes_tl, &bl);

        for (i = 0; i < 30; i++)
            ll[i] = 5;
        bd = 5;
        HuftBuild(ll, 30, 0, cpdist, cpdext,
                &GLB.inflate_codes_td, &bd);
        InitInflateCodes(bl, bd);

        return CODES;
    }
    case 2: {

        int dbits = 6;
        int lbits = 9;

        huft_t *td;
        uint32_t i;
        uint32_t j;
        uint32_t l;
        uint32_t m;
        uint32_t n;
        uint32_t bl;
        uint32_t bd;
        uint32_t nb;
        uint32_t nl;
        uint32_t nd;
        uint32_t b_dynamic;
        uint32_t k_dynamic;

        b_dynamic = GLB.bi_buf;
        k_dynamic = GLB.bi_valid;


        if (FillBitBuffer(&b_dynamic, &k_dynamic, 5) < 0)
            return -1;
        nl = 257 + (b_dynamic & 0x1f);
        b_dynamic >>= 5;
        k_dynamic -= 5;

        if (FillBitBuffer(&b_dynamic, &k_dynamic, 5) < 0)
            return -1;
        nd = 1 + (b_dynamic & 0x1f);
        b_dynamic >>= 5;
        k_dynamic -= 5;

        if (FillBitBuffer(&b_dynamic, &k_dynamic, 4) < 0)
            return -1;
        nb = 4 + (b_dynamic & 0xf);
        b_dynamic >>= 4;
        k_dynamic -= 4;

        if (nl > 286 || nd > 30)
            return -1;

        for (j = 0; j < nb; j++) {
            if (FillBitBuffer(&b_dynamic, &k_dynamic, 3) < 0)
                return -1;
            ll[border[j]] = b_dynamic & 0x7;
            b_dynamic >>= 3;
            k_dynamic -= 3;
        }
        for (; j < 19; j++)
            ll[border[j]] = 0;

        bl = 7;
        i = HuftBuild(ll, 19, 19, NULL, NULL,
                &GLB.inflate_codes_tl, &bl);
        if (i < 0)
            return -1;

        n = nl + nd;
        m = mask_bits[bl];
        i = l = 0;
        while (i < n) {
            if (FillBitBuffer(&b_dynamic, &k_dynamic, bl) < 0)
                return -1;
            td = GLB.inflate_codes_tl + (b_dynamic & m);
            j = td->b;
            b_dynamic >>= j;
            k_dynamic -= j;
            j = td->n;
            if (j < 16) {
                ll[i++] = l = j;
            } else if (j == 16) {
                if (FillBitBuffer(&b_dynamic, &k_dynamic, 2) < 0)
                    return -1;
                j = 3 + (b_dynamic & 3);
                b_dynamic >>= 2;
                k_dynamic -= 2;
                if (i + j > n)
                    return -1;
                while (j--) {
                    ll[i++] = l;
                }
            } else if (j == 17) {
                if (FillBitBuffer(&b_dynamic, &k_dynamic, 3) < 0)
                    return -1;
                j = 3 + (b_dynamic & 7);
                b_dynamic >>= 3;
                k_dynamic -= 3;
                if (i + j > n)
                    return -1;
                while (j--) {
                    ll[i++] = 0;
                }
                l = 0;
            } else {
                if (FillBitBuffer(&b_dynamic, &k_dynamic, 7) < 0)
                    return -1;
                j = 11 + ((unsigned) b_dynamic & 0x7f);
                b_dynamic >>= 7;
                k_dynamic -= 7;
                if ((unsigned) i + j > n)
                    return -1;
                while (j--)
                    ll[i++] = 0;
                l = 0;
            }
        }

        HuftFree(GLB.inflate_codes_tl);

        GLB.bi_buf = b_dynamic;
        GLB.bi_valid = k_dynamic;

        bl = lbits;
        i = HuftBuild(ll, nl, 257, cplens, cplext,
                &GLB.inflate_codes_tl, &bl);
        if (i < 0)
            return -1;
        bd = dbits;
        i = HuftBuild(ll + nl, nd, 0, cpdist, cpdext,
                &GLB.inflate_codes_td, &bd);
        if (i < 0)
            return -1;

        InitInflateCodes(bl, bd);

        return CODES;
    }
    default:
        return -1;
    }
}

static int InflateStored()
{
    while (GLB.inflate_stored_n--) {
        if (FillBitBuffer(&GLB.inflate_stored_b,
                &GLB.inflate_stored_k, 8) < 0)
            return -1;
        GLB.window[GLB.inflate_stored_w++] =
                (unsigned char)GLB.inflate_stored_b;
        GLB.inflate_stored_b >>= 8;
        GLB.inflate_stored_k -= 8;
        if (GLB.inflate_stored_w == GUNZIP_WSIZE) {
            GLB.outbuf_cnt = GLB.inflate_stored_w;
            GLB.inflate_stored_w = 0;
            return 1;
        }
    }

    GLB.outbuf_cnt = GLB.inflate_stored_w;
    GLB.bi_buf = GLB.inflate_stored_b;
    GLB.bi_valid = GLB.inflate_stored_k;
    return 0;
}

static int InflateCodes()
{
    unsigned e;
    huft_t *t;

    if (GLB.resume_copy)
        goto do_copy;

    while (1) {
        if (FillBitBuffer(&GLB.inflate_codes_bb,
                &GLB.inflate_codes_k,GLB.inflate_codes_bl) < 0)
            return -1;
        t = GLB.inflate_codes_tl +
                (GLB.inflate_codes_bb & GLB.inflate_codes_ml);
        e = t->e;
        if (e > 16)
            do {
                if (e == 99)
                    return -1;
                GLB.inflate_codes_bb >>= t->b;
                GLB.inflate_codes_k -= t->b;
                e -= 16;
                if (FillBitBuffer(&GLB.inflate_codes_bb,
                        &GLB.inflate_codes_k, e) < 0)
                    return -1;
                t = t->t + (GLB.inflate_codes_bb & mask_bits[e]);
                e = t->e;
            } while (e > 16);
        GLB.inflate_codes_bb >>= t->b;
        GLB.inflate_codes_k -= t->b;
        if (e == 16) {
            GLB.window[GLB.inflate_codes_w++] = (unsigned char)t->n;
            if (GLB.inflate_codes_w == GUNZIP_WSIZE) {
                GLB.outbuf_cnt = GLB.inflate_codes_w;
                GLB.inflate_codes_w = 0;
                return 1;
            }
        } else {
            if (e == 15)
                break;

            if (FillBitBuffer(&GLB.inflate_codes_bb,
                    &GLB.inflate_codes_k, e) < 0)
                return -1;
            GLB.inflate_codes_nn = t->n +
                    (GLB.inflate_codes_bb & mask_bits[e]);
            GLB.inflate_codes_bb >>= e;
            GLB.inflate_codes_k -= e;

            if (FillBitBuffer(&GLB.inflate_codes_bb,
                    &GLB.inflate_codes_k, GLB.inflate_codes_bd) < 0)
                return -1;
            t = GLB.inflate_codes_td +
                    (GLB.inflate_codes_bb & GLB.inflate_codes_md);
            e = t->e;
            if (e > 16)
                do {
                    if (e == 99)
                        return -1;
                    GLB.inflate_codes_bb >>= t->b;
                    GLB.inflate_codes_k -= t->b;
                    e -= 16;
                    if (FillBitBuffer(&GLB.inflate_codes_bb,
                            &GLB.inflate_codes_k, e) < 0)
                        return -1;
                    t = t->t + (GLB.inflate_codes_bb & mask_bits[e]);
                    e = t->e;
                } while (e > 16);
            GLB.inflate_codes_bb >>= t->b;
            GLB.inflate_codes_k -= t->b;
            if (FillBitBuffer(&GLB.inflate_codes_bb,
                    &GLB.inflate_codes_k, e) < 0)
                return -1;
            GLB.inflate_codes_dd = GLB.inflate_codes_w - t->n -
                    (GLB.inflate_codes_bb & mask_bits[e]);
            GLB.inflate_codes_bb >>= e;
            GLB.inflate_codes_k -= e;

do_copy:
            do {
                uint32_t delta;

                GLB.inflate_codes_dd &= GUNZIP_WSIZE - 1;
                e = GUNZIP_WSIZE -
                        (GLB.inflate_codes_dd > GLB.inflate_codes_w ?
                        GLB.inflate_codes_dd : GLB.inflate_codes_w);
                delta = GLB.inflate_codes_w > GLB.inflate_codes_dd ?
                        GLB.inflate_codes_w - GLB.inflate_codes_dd :
                        GLB.inflate_codes_dd - GLB.inflate_codes_w;
                if (e > GLB.inflate_codes_nn)
                    e = GLB.inflate_codes_nn;
                GLB.inflate_codes_nn -= e;

                if (delta >= e) {
                    memcpy(GLB.window + GLB.inflate_codes_w,
                            GLB.window + GLB.inflate_codes_dd, e);
                    GLB.inflate_codes_w += e;
                    GLB.inflate_codes_dd += e;
                } else {
                    do {
                        GLB.window[GLB.inflate_codes_w++] =
                                GLB.window[GLB.inflate_codes_dd++];
                    } while (--e);
                }
                if (GLB.inflate_codes_w == GUNZIP_WSIZE) {
                    GLB.outbuf_cnt = GLB.inflate_codes_w;
                    GLB.resume_copy = (GLB.inflate_codes_nn != 0);

                    GLB.inflate_codes_w = 0;
                    return 1;
                }
            } while (GLB.inflate_codes_nn);
            GLB.resume_copy = 0;
        }
    }

    GLB.outbuf_cnt = GLB.inflate_codes_w;
    GLB.bi_buf = GLB.inflate_codes_bb;
    GLB.bi_valid = GLB.inflate_codes_k;

    HuftFree(GLB.inflate_codes_tl);
    HuftFree(GLB.inflate_codes_td);
    GLB.inflate_codes_tl = NULL;
    GLB.inflate_codes_td = NULL;

    return 0;
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

static int InflateGetNextWindow()
{
    while (1) {
        int ret;

        if (GLB.need_another_block) {
            if (GLB.eof) {
                UpdateCrc32(GLB.window, GLB.outbuf_cnt);
                GLB.eof = 0;
                return 0;
            }
            GLB.method = InflateBlock();
            GLB.need_another_block = 0;
        }

        switch (GLB.method) {
        case STORED:
            ret = InflateStored();
            break;
        case CODES:
            ret = InflateCodes();
            break;
        default:
            return -1;
        }
        if (ret < 0)
            return -1;
        if (ret == 1) {
            UpdateCrc32(GLB.window, GLB.outbuf_cnt);
            return 1;
        }
        GLB.need_another_block = 1;
    }
}

static inline int FlushOutbuf()
{
    int written_size = write(GLB.out_fd, GLB.window, GLB.outbuf_cnt);
    int ret = written_size == GLB.outbuf_cnt ? 0 : -1;
    GLB.outcnt += written_size;
    GLB.outbuf_cnt = 0;

    return ret;
}

static int inflate()
{

    GLB.crc32 = ~0;
    GLB.outcnt = 0;
    GLB.outbuf_cnt = 0;
    GLB.method = -1;
    GLB.need_another_block = 1;
    GLB.resume_copy = 0;
    GLB.bi_buf = 0;
    GLB.bi_valid = 0;

    while (1) {
        int ret = InflateGetNextWindow();
        if (ret < 0)
            return -1;
        if (FlushOutbuf() < 0)
            return -1;
        if (ret == 0)
            break;
    }

    if (GLB.bi_valid >= 8) {
        GLB.bytebuffer_offset--;
        GLB.bytebuffer[GLB.bytebuffer_offset] = GLB.bi_buf & 0xff;
        GLB.bi_buf >>= 8;
        GLB.bi_valid -= 8;
    }

    return 0;
}

static int FillByteBuffer(int n)
{
    int count = GLB.bytebuffer_size - GLB.bytebuffer_offset;

    if (count < n) {
        memmove(GLB.bytebuffer,
                &GLB.bytebuffer[GLB.bytebuffer_offset], count);
        GLB.bytebuffer_offset = 0;
        GLB.bytebuffer_size = read(GLB.in_fd,
                &GLB.bytebuffer[count], GUNZIP_BYTEBUFFER_MAX - count);
        if ((int)GLB.bytebuffer_size < 0)
            return -1;
        GLB.bytebuffer_size += count;
        if (GLB.bytebuffer_size < n)
            return -1;
    }

    return 0;
}

static uint16_t BufferRead16()
{
    uint16_t res;

    res = GLB.bytebuffer[GLB.bytebuffer_offset];
    res |= GLB.bytebuffer[GLB.bytebuffer_offset + 1] << 8;
    GLB.bytebuffer_offset += 2;

    return res;
}

static uint32_t BufferRead32()
{
    uint32_t res;

    res = GLB.bytebuffer[GLB.bytebuffer_offset];
    res |= (uint32_t)GLB.bytebuffer[GLB.bytebuffer_offset + 1] << 8;
    res |= (uint32_t)GLB.bytebuffer[GLB.bytebuffer_offset + 2] << 16;
    res |= (uint32_t)GLB.bytebuffer[GLB.bytebuffer_offset + 3] << 24;
    GLB.bytebuffer_offset += 4;

    return res;
}

int InflateUnzip(int zip_fd, int dst_fd, uint32_t cmpsize,
        uint32_t ucmpsize, uint32_t crc32) {
    int ret = 0;

#ifdef USE_HEAP_MEM
    glbp = malloc(sizeof(struct globals));
    if (glbp == NULL) {
        KPrintf("Memory too small\n");
        return -1;
    }
#endif
    memset(&GLB, 0, sizeof(struct globals));
    GLB.to_read = cmpsize;
    GLB.bytebuffer_offset = 4;
    GLB.in_fd = zip_fd;
    GLB.out_fd = dst_fd;

    if ((ret = inflate()) < 0)
        goto free_glbp;
    ret = ucmpsize == GLB.outcnt && ~crc32 == GLB.crc32;

free_glbp:
#ifdef USE_HEAP_MEM
    free(glbp);
#endif

    return ret;
}

static int GunzipCheckFooter()
{
    int crc32, length;

    if (FillByteBuffer(8) < 0)
        return -1;

    crc32 = BufferRead32();
    length = BufferRead32();
    if (~GLB.crc32 != crc32 || length != GLB.outcnt)
        return -1;

    return 0;
}

static int GunzipCheckHeader()
{
    GzipHdr_t hdr;

    if (FillByteBuffer(8) < 0)
        return -1;
    memcpy(&hdr, &GLB.bytebuffer[GLB.bytebuffer_offset], 8);
    GLB.bytebuffer_offset += 8;

    if (hdr.gz_method != 8)
        return -1;

    if (hdr.flags & 0x04) {
        uint16_t extra_len;

        extra_len = BufferRead16();
        if (FillByteBuffer(extra_len) < 0)
            return -1;
        GLB.bytebuffer_offset += extra_len;
    }

    if (hdr.flags & 0x18) {
        while (1) {
            do {
                if (FillByteBuffer(1) < 0)
                    return -1;
            } while (GLB.bytebuffer[GLB.bytebuffer_offset++] != 0);
            if ((hdr.flags & 0x18) != 0x18)
                break;
            hdr.flags &= ~0x18;
        }
    }

    if (hdr.flags & 0x02) {
        if (FillByteBuffer(2) < 0)
            return -1;
        GLB.bytebuffer_offset += 2;
    }

    return 0;
}

static int GunzipCheckMagic()
{
    uint16_t magic;

    if (FillByteBuffer(2) < 0)
        return -1;

    magic = BufferRead16();
    if (magic != 0x8b1f)
        return -1;

    return 0;
}

int GzipDecompress(char *file_name)
{
    int ret = 0;

#ifdef USE_HEAP_MEM
    glbp = malloc(sizeof(struct globals));
    if (glbp == NULL) {
        KPrintf("Memory too small\n");
        return -1;
    }
#endif
    memset(&GLB, 0, sizeof(struct globals));
    GLB.to_read = -1;

    if ((GLB.in_fd = open(file_name, O_RDONLY)) < 0) {
        ret = -1;
        goto free_glbp;
    }

    if (TruncateExtension(file_name, ".gz") < 0) {
        KPrintf("Invalid extension: %s\n", file_name);
        ret = -1;
        goto close_in_fd;
    }

    if ((GLB.out_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        ret = -1;
        goto close_in_fd;
    }
    strcat(file_name, ".gz");

    if (GunzipCheckMagic() < 0) {
        KPrintf("Invalid magic number: %s\n", file_name);
        ret = -1;
        goto close_out_fd;
    }

again:
    if (GunzipCheckHeader() < 0) {
        KPrintf("Invalid header: %s\n", file_name);
        ret = -1;
        goto close_out_fd;
    }

    if ((inflate()) < 0) {
        ret = -1;
        goto close_out_fd;
    }

    if (GunzipCheckFooter() < 0) {
        KPrintf("Invalid footer (checksum or length): %s\n", file_name);
        ret = -1;
    }

    if (GunzipCheckMagic() == 0)
        goto again;

close_out_fd:
    close(GLB.out_fd);
close_in_fd:
    close(GLB.in_fd);
free_glbp:
#ifdef USE_HEAP_MEM
    free(glbp);
#endif

    return ret;
}

static void GunzipPrintUsage()
{
    KPrintf("Usage: gunzip [FILEs]...\n");
    KPrintf("File names MUST end with the .gz extension\n");
}

int gunzip(int argc, char **argv)
{
    int ret = 0;

    if (argc < 2) {
        GunzipPrintUsage();
        return 0;
    }

    for (int i = 1; i < argc; i++)
        if (GzipDecompress(argv[i]) < 0)
            ret = -1;

    if (ret < 0)
        KPrintf("Failed to extract (some) file(s)\n");

    return 0;
}


#endif
