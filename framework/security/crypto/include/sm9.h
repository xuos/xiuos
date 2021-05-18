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
* @file sm9.h
* @brief API of SM9
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef SM9_H
#define SM9_H

#include <sm9_util.h>
#include <sm9_para.h>

typedef struct SM9Signature {
    big8w h;
    G1point S;
} Signature;

typedef struct SM9_Key_Package {
    unsigned char* K;
    G1point C;
} Key_Package;

void SM9Init();
Signature SM9Sign(unsigned char *message, uint32_t msglen, G1point ds, G2point Ppub_s);
bool SM9VerifySignature(
	unsigned char *ID, unsigned char ID_len, unsigned char hid,
	unsigned char *message, uint32_t msglen,
	Signature signature, G2point Ppub_s);

void SM9KeyExchangeProduceR(unsigned char* ID, unsigned char IDlen, big8w* r, G1point* R, G1point encrypt_publickey);
bool SM9KeyExchangeProduceKey(G1point* RA, G1point* RB, big8w* r, uint32_t klen_bitsize, 
                            unsigned char* challengerID, unsigned char challengerIDlen,
                            unsigned char* responserID, unsigned char responserIDlen,
                            q12 *g1, q12* g2, q12* g3, char* resultkey, bool sponsor,
                            G1point encrypt_publickey, G2point encrypt_secretkey);
bool SM9KeyExchangeVerifyKey(q12 *g1, q12 *g2, q12 *g3, G1point *RA, G1point *RB,
                             unsigned char *challengerID, unsigned char challengerIDlen,
                             unsigned char *responserID, unsigned char responserIDlen,
                             unsigned char *S1, unsigned char *SA);
                             
void SM9KeyPackage(unsigned char* ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e, uint32_t klen_bitsize, unsigned char* K, G1point* C);
bool SM9KeyDepackage(G1point C,  G2point de, unsigned char* ID, unsigned char IDlen, unsigned int klen_bitsize, unsigned char* K);

bool SM9EncryptWithKDF(unsigned char *message, unsigned int msglen_bitsize, unsigned int K2_len_bitsize,
                       unsigned char *ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e, unsigned char *C);
bool SM9DecryptWithKDF(unsigned char *ID, unsigned char IDlen,
                       unsigned char *message, unsigned int msglen_bitsize, unsigned int K2_len_bitsize,
                       unsigned char *C, G2point encrypt_secretkey);

bool SM9EncryptWithSM4(unsigned char *message, unsigned int msglen_bytesize,
                       unsigned int K1_len_bitsize, unsigned int K2_len_bitsize,
                       unsigned char *ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e,
                       unsigned char *C);
bool SM9DecryptWithSM4(unsigned char *ID, unsigned char IDlen,
                       unsigned char *message, unsigned int msglen, unsigned int K1_len_bitsize, unsigned int K2_len_bitsize,
                       unsigned char *C, unsigned int Cbyteslen, G2point encrypt_secretkey);

#endif