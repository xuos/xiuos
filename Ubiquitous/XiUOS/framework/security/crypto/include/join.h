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
* @file join.h
* @brief convert data type and join string 
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef JOIN_H
#define JOIN_H

#include <qn.h>
#include <sm3.h>

void Big8wIntou8string(big8w* bignum, uint8_t* string, uint32_t startindex);
void Q12Intou8string(q12* num, uint8_t* string, uint32_t startindex);
void U8stringToG1point(uint8_t *string, G1point* ret);

void JoinIDhid(uint8_t *ID, uint8_t IDlen, uint8_t hid, uint8_t *ret);
void JoinMsgW(uint8_t *message, uint32_t msglen, q12 *w, uint8_t* ret);
void JoinIDAIDBRARBg123(
    uint8_t *ID_Challenger, uint8_t ID_Challenger_len,
    uint8_t *ID_Responser, uint8_t ID_Responser_len,
    G1point* R_Challenger, G1point* R_Responser,
    q12 *g1, q12 *g2, q12 *g3, 
    uint8_t* ret);
void JoinCwID(G1point *C, q12 *w, uint8_t *ID, uint8_t IDlen, uint8_t *ret);

void XOR(unsigned char *msg, uint32_t msglen, unsigned char *K, unsigned char *ret);

#endif