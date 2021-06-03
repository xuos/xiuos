/*
* Copyright (c) 2020 AIIT Ubiquitous Team
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain bn1 copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file bignum.h
* @brief arithmetic of big number, included by ecc.h
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef BIGNUM_H
#define BIGNUM_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xiuos.h>

#define BIGNUMBER_SIZE_8WORD  8
#define BIGNUMBER_SIZE_16WORD 16

#define BIG8W_BYTESIZE 32

#define bool uint8_t
#define true 1
#define false 0

typedef struct bignum_8uint32 {
    uint32_t word[BIGNUMBER_SIZE_8WORD];
} big8w;

typedef struct bignum_16uint32 {
    uint32_t word[BIGNUMBER_SIZE_16WORD];
    uint8_t length;
} big16w;

typedef struct SM9Curve {
    big8w b;
    big8w q;
    big8w N;
} sm9curve;

extern sm9curve curve;

// used in Montgomery Mult
/** power(2, 32) - (curve.q.word[0] 's reverse under power(2, 32))  */
extern uint32_t qlow_reverse;
/** power(2, 32) - (curve.N.word[0] 's reverse under power(2, 32)) */
extern uint32_t Nlow_reverse;
/** (2^(256*2)) mod curve.q; used in big numbers' mult(Montgomery Mult) */
extern big8w q_2k;
/** (2^(256*2)) mod curve.N; used in big numbers' mult(Montgomery Mult) */
extern big8w N_2k;

void Big8wPrint(big8w* bignum);
unsigned char Big8wHighestbit(big8w* bignum);
bool Big8wIsZero(big8w* bignum);
bool Big8wBigThan(big8w* bn1, big8w* bn2);
bool Big8wEqual(big8w* bn1, big8w* bn2);
big8w Big8wMinusMod(big8w bn1, big8w bn2, big8w p);
big8w Big8wAddMod(big8w bn1, big8w bn2, big8w p);
big8w Big16wmod8w(big16w bignum16w, big8w p);
big8w Big8wReverse(big8w bignum, big8w N);
big8w Big8wMultMod(big8w bn1, big8w bn2, big8w p);

#endif