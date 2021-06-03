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

#ifndef __INC_GZIP_H__
#define __INC_GZIP_H__

#include <stdint.h>

#define WSIZE               (1 << 15)
#define OUTBUF_SIZE         (1 << 14)
#define WMASK               (WSIZE - 1)

#define LIT_BUFSIZE         WSIZE
#define DIST_BUFSIZE        LIT_BUFSIZE

#define MIN_MATCH           3
#define MAX_MATCH           258

#define MIN_LOOKAHEAD       (MAX_MATCH + MIN_MATCH + 1)
#define MAX_CHAIN_LEN       4096
#define MAX_LAZY_MATCH      MAX_MATCH
#define MAX_DIST            (WSIZE - MIN_LOOKAHEAD)

#define HASH_BITS           15
#define HASH_SIZE           (1 << HASH_BITS)
#define HASH_MASK           (HASH_SIZE - 1)
#define HASH_SHIFT          ((HASH_BITS + MIN_MATCH - 1) / MIN_MATCH)

#define MAX_BITS         15

#define MAX_BL_BITS      7

#define LENGTH_CODES     29

#define LITERALS         256

#define END_BLOCK        256

#define L_CODES          (LITERALS + 1 + LENGTH_CODES)

#define D_CODES          30

#define BL_CODES         19

#define STORED_BLOCK     0
#define STATIC_TREES     1
#define DYN_TREES        2

typedef struct CtDate
{
    union {
        uint16_t freq;
        uint16_t code;
    };
    union {
        uint16_t father;
        uint16_t len;
    };
} CtDate_t;


typedef struct TreeDesc {
    CtDate_t *dyn_tree;
    CtDate_t *static_tree;
    const uint8_t *extra_bits;
    int extra_base;
    int elems;
    int max_length;
    int max_code;
} TreeDesc_t;

#define HEAP_SIZE      (2 * L_CODES + 1)


#define REP_3_6        16

#define REPZ_3_10      17

#define REPZ_11_138    18

#endif
