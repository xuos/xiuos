/*
* Copyright (c) 2020 AIIT Ubiquitous Team
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file qn.h
* @brief arithmetic in extention field, and arithmetic in group G2, frobenius and LastPower in BiLinearPairing
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef QN_H
#define QN_H

#include <ecc.h>

typedef struct q2_num {
    big8w high;
    big8w low;
} q2;

typedef struct G2_q2group_point {
    q2 x;
    q2 y;
} G2point;

typedef struct q4_num {
    q2 high;
    q2 low;
} q4;

typedef struct q12_num {
    
    q4 high;
    q4 mid;
    q4 low;

} q12;

typedef struct big_12bignum {
    big8w word[12];
} big_12big;

extern big8w t; // sm9 ecc parameter
extern big8w qnr; // (-1/2) mod curve.q
extern big8w frobenius_constant_1[12];
extern big8w frobenius_constant_2[12];

void G2pointPrint(G2point *point);
void Q12Print(q12* number);
void Q12To12big(q12 *num, big_12big *ret);
G2point G2PointAdd(G2point point1, G2point point2);
G2point G2PointMult(big8w num, G2point point);
void Q12Zero(q12 *num);
q12 Q12MultMod(q12 a, q12 b);
q12 Q12PowerMod(q12 g, big8w r);
void Q12Frobenius(q12 *f, uint8_t flag);
void G2pointFrobenius(G2point Q, G2point* Q1, uint8_t flag);
void Line(G1point P, G2point *T, G2point Q, bool doubleflag, q12 *f);
void LastPower(q12 *f);

#endif