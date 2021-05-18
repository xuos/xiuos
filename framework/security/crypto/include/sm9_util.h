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
* @file sm9_util.h
* @brief the function called by SM9 function, including hash, KDF, produce random number, encrypt and decrypt algorithm, BiLinearPairing
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef SM9_UTIL_H
#define SM9_UTIL_H

#include <join.h>
#include <sm4.h>

#define SM3OUT_32BYTES 32 // (256 / 8) 

void HashTwice(uint8_t *ID_A, uint8_t ID_A_len, uint8_t *ID_B, uint8_t ID_B_len,
               G1point *RA, G1point *RB,
               q12 *g1, q12 *g2, q12 *g3, uint8_t funcflag, uint8_t *ret);
big8w RandomNumGenerate();
bool StringEqualZero(uint8_t* string, uint32_t stringlen);
big8w H(uint8_t *Z, uint32_t Zlen, uint8_t funcflag);
void KDF(uint8_t *Z, uint32_t Zlen, uint32_t klen, uint8_t *ret);
void SM4EncryptWithEcbMode(uint8_t* message, uint32_t msglen, uint8_t* key, uint8_t* ciphertext);
void SM4DecryptWithEcbMode(uint8_t* ciphertext, uint32_t ciphertextlen, uint8_t* message, int msglen, uint8_t* key);
q12 BiLinearPairing(G1point P, G2point Q);

#endif
