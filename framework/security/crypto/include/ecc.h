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
* @file ecc.h
* @brief arithmetic in ecc, included by qn.h
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef ECC_H
#define ECC_H

#include <bignum.h>

typedef struct G1_base_group_point {
    big8w x;
    big8w y;
} G1point;

typedef struct SM9ecn{
    big8w x;
    big8w y;
    big8w z;
} ecn;

void G1pointPrint(G1point *point);
bool PointInG1(G1point point);
G1point G1pointAdd(G1point point1, G1point point2);
G1point G1pointMult(big8w bignum, G1point point);

#endif