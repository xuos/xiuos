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
* @file sm9.c
* @brief API of SM9
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/
#include <sm9.h>

#define SM4OUT 16 // 128 / 8

/**
 * @brief initialization function, to set value of SM9 curve elements and other essential parameters.
 * 
 * @param null
 * 
 * @result null
 * 
 */

void SM9Init()
 {
     big8w one;
     memset(one.word, 0x00, BIG8W_BYTESIZE);
     one.word[0] = 1;
     memset(curve.b.word, 0x00, BIG8W_BYTESIZE);
     curve.b.word[0] = 0x05;
     memcpy(curve.q.word, sm9_q, BIG8W_BYTESIZE);
     memcpy(curve.N.word, sm9_N, BIG8W_BYTESIZE);
     memcpy(P1.x.word, sm9_P1_x, BIG8W_BYTESIZE);
     memcpy(P1.y.word, sm9_P1_y, BIG8W_BYTESIZE);
     memcpy(P2.x.high.word, sm9_P2_x_high, BIG8W_BYTESIZE);
     memcpy(P2.x.low.word, sm9_P2_x_low, BIG8W_BYTESIZE);
     memcpy(P2.y.high.word, sm9_P2_y_high, BIG8W_BYTESIZE);
     memcpy(P2.y.low.word, sm9_P2_y_low, BIG8W_BYTESIZE);
     memset(t.word, 0x00, BIG8W_BYTESIZE);
	 t.word[0] = 0x0058F98A, t.word[1] = 0x60000000;
     frobenius_constant_1[0] = one;
     memcpy(frobenius_constant_1[1].word, fc1_1, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[2].word, fc1_2, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[3].word, fc1_3, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[4].word, fc1_4, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[5].word, fc1_5, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[6].word, fc1_6, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[7].word, fc1_7, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[8].word, fc1_8, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[9].word, fc1_9, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[10].word, fc1_10, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_1[11].word, fc1_11, BIG8W_BYTESIZE);
     frobenius_constant_2[0] = one;
     frobenius_constant_2[1] = one;
     memcpy(frobenius_constant_2[2].word, fc2_2, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[3].word, fc2_3, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[4].word, fc2_4, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[5].word, fc2_5, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[6].word, fc2_6, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[7].word, fc2_7, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[8].word, fc2_8, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[9].word, fc2_9, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[10].word, fc2_10, BIG8W_BYTESIZE);
     memcpy(frobenius_constant_2[11].word, fc2_11, BIG8W_BYTESIZE);
     memcpy(qnr.word, sm9_qnr, BIG8W_BYTESIZE);
     memcpy(q_2k.word, sm9_q_2k, BIG8W_BYTESIZE);
     memcpy(N_2k.word, sm9_N_2k, BIG8W_BYTESIZE);
}


/**
 * 
 * @brief sign algorithm of SM9
 * 
 * @param message data need to sign with sign_secretkey
 * @param msgbytelen length of message, byte size
 * @param ds sign_secretkey
 * @param Ppub_s sign_publickey
 * 
 * @return Signature
 * 
 */
Signature SM9Sign(unsigned char *message, uint32_t msgbytelen, G1point ds, G2point Ppub_s)
{
    uint8_t *msg_w;
    big8w r, L_temp, zero;
    q12 g, w;
	Signature sig;

    msg_w = (uint8_t *)(malloc(msgbytelen + BIG8W_BYTESIZE * 12));

    memset(zero.word, 0x00, BIG8W_BYTESIZE);

	g = BiLinearPairing(P1, Ppub_s); //e(P1, Ppub_s)

	do
	{
		r = RandomNumGenerate();

		w = Q12PowerMod(g, r); // w = g^r

        JoinMsgW(message, msgbytelen, &w, msg_w);
        sig.h = H(msg_w, msgbytelen + BIG8W_BYTESIZE * 12, 0x02);

		L_temp = Big8wMinusMod(r, sig.h, curve.N);
	} while (Big8wEqual(&L_temp, &zero));

	sig.S = G1pointMult(L_temp, ds);

    free(msg_w);

    return sig;
}
/**
 * 
 * @brief signature verify algorithm of SM9
 * 
 * @param ID id of user who generate the signature
 * @param IDlen length of ID
 * @param hid function id defined in SM9
 * @param message message that received from counterpart
 * @param msgbytelen length of message, byte size
 * @param signature signature received from counterpart
 * @param Ppub_s sign_publickey
 * 
 * @return true if signature verified successfully, else false
 * 
 */
bool SM9VerifySignature(
	unsigned char* ID, unsigned char IDlen, unsigned char hid, 
	unsigned char* message, unsigned int msgbytelen,
	Signature signature, G2point Ppub_s)
{
    unsigned char *msg_w;
    big8w zero, h;
    G2point P;
    q12 g, t, w;

    msg_w = (unsigned char *)(malloc(msgbytelen + BIG8W_BYTESIZE * 12));
	memset(zero.word, 0x00, BIG8W_BYTESIZE);

	if (Big8wBigThan(&signature.h, &curve.N)
		|| Big8wEqual(&signature.h, &zero))
		return false;

	if (!PointInG1(signature.S))
        return false;

	g = BiLinearPairing(P1, Ppub_s);

	t = Q12PowerMod(g, signature.h);

	unsigned char* id_hid;
	id_hid = (unsigned char*)malloc((IDlen + 1));
	JoinIDhid(ID, IDlen, hid, id_hid);

	h = H(id_hid, IDlen + 1, 0x01);

	P = G2PointMult(h, P2);

	P = G2PointAdd(P, Ppub_s);

	g = BiLinearPairing(signature.S, P);

	w = Q12MultMod(g, t);

    JoinMsgW(message, msgbytelen, &w, msg_w);
	h = H(msg_w, msgbytelen + BIG8W_BYTESIZE * 12, 0x02);

    free(id_hid);
    free(msg_w);

    return Big8wEqual(&h, &signature.h);
}
/**
 * 
 * @brief the first step of key exchange of SM9, produce R(the data send to the counterpart)
 * 
 * @param ID ID of counterpart
 * @param IDlen length of ID
 * @param r a pointer of a big number, random big number
 * @param R  a pointer of a point in group G1, store the result
 * @param encrypt_publickey encrypt_publickey
 * 
 * @result R
 * 
 */
void SM9KeyExchangeProduceR(unsigned char* ID, unsigned char IDlen, big8w* r, G1point* R, G1point encrypt_publickey)
{
	unsigned char hid = 0x02;
    unsigned char *id_hid;

    id_hid = (unsigned char *)(malloc(IDlen + 1));

    JoinIDhid(ID, IDlen, hid, id_hid);

    *R = G1pointMult(H(id_hid, IDlen + 1, 0x01), P1);
	*R = G1pointAdd(*R, encrypt_publickey);
	*r = RandomNumGenerate();
	*R = G1pointMult(*r, *R);

    free(id_hid);
}
/**
 * 
 * @brief the second step of key exchange of SM9, produce key
 * 
 * @param RA a pointer of a point in group G1, produced in function SM9KeyExchangeProduceR
 * @param RB a pointer of a point in group G1, received from counterpart 
 * @param r a pointer of a big number, random big number, produced in function SM9KeyExchangeProduceR.
 * @param klen_bitsize length of key to be generated, bit size!!!
 * @param challengerID ID of the challenger of key exchange
 * @param challengerIDlen length of chalengerID
 * @param responserID ID of the responser of key exchange
 * @param responserIDlen length of responserID
 * @param resultkey unsigned char string, store the key generated in key exchange
 * @param encrypt_publickey encrypt public key
 * @param encrypt_secretkey encrypt secret key
 * 
 * @result resultkey
 * 
 */
bool SM9KeyExchangeProduceKey(G1point* RA, G1point* RB, big8w* r, uint32_t klen_bitsize, 
                            unsigned char* challengerID, unsigned char challengerIDlen,
                            unsigned char* responserID, unsigned char responserIDlen,
                            q12 *g1, q12* g2, q12* g3, char* resultkey, bool sponsor,
                            G1point encrypt_publickey, G2point encrypt_secretkey)
{

    unsigned char *string;
    int i, stringlen;

    stringlen = challengerIDlen + responserIDlen + BIG8W_BYTESIZE * (2 * 2 + 12 * 3);
    string = (uint8_t *)(malloc(stringlen));
    
	if (!PointInG1(*RB)){
        printf("Point error, point is not in G1\n");
        return false;
    }
		
	*g1 = BiLinearPairing(encrypt_publickey, P2);
	*g1 = Q12PowerMod(*g1, *r);

	*g2 = BiLinearPairing(*RB, encrypt_secretkey);

    if (!sponsor){ // notice that the format of challenger and responser is similar.
        *g3 = *g1;
        *g1 = *g2;
        *g2 = *g3;
    }
    
    *g3 = Q12PowerMod(*g2, *r);

    JoinIDAIDBRARBg123(challengerID, challengerIDlen, responserID, responserIDlen, RA, RB, g1, g2, g3, string);

    KDF(string, stringlen, klen_bitsize, (uint8_t*)resultkey);

    free(string);

    return true;
}
/**
 * @brief the third step of SM9 key exchange, to verify the key is right.
 * 
 * @param g1 pointer of the first q12 number
 * @param g2 pointer of the second q12 number
 * @param g3 pointer of the second q12 number
 * @param RA pointer of a G1point of the challenger
 * @param RB pointer of a G1point of the responser
 * @param challengerID ID of the challenger of key exchange
 * @param challengerIDlen length of chalengerID
 * @param responserID ID of the responser of key exchange
 * @param responserIDlen length of responserID
 * @param S1 hash value, function flag = 0x82
 * @param SA hash value, function flag = 0x83
 * 
 * @result two hash values: S1, SA
 * 
 */
bool SM9KeyExchangeVerifyKey(q12* g1, q12 *g2, q12* g3, G1point* RA, G1point* RB, 
                            unsigned char* challengerID, unsigned char challengerIDlen,
                            unsigned char* responserID, unsigned char responserIDlen,
                            unsigned char *S1, unsigned char* SA)
{
    HashTwice(challengerID, challengerIDlen, responserID, responserIDlen, RA, RB, g1, g2, g3, 0x82, S1);

	HashTwice(challengerID, challengerIDlen, responserID, responserIDlen, RA, RB, g1, g2, g3, 0x83, SA);
}
/**
 * 
 * @brief key encapsulation define in SM9, called by encrypt of SM9.
 * 
 * @param ID counterpart's ID
 * @param IDlen length of ID
 * @param hid function flag defined in SM9, 
 * @param Ppub_e encrypt_publickey
 * @param klen_bitsize length of key to be generated by KDF(), bit size!!!
 * @param K key generated by KDF()
 * @param C packaged K
 * 
 * @result K, C
 * 
 */
void SM9KeyPackage(unsigned char* ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e, uint32_t klen_bitsize, unsigned char* K, G1point* C)
{
	unsigned char *c_w_id, * id_hid;
    big8w r;
	G1point temp, QB;
	q12 g;

    c_w_id = (unsigned char*)(malloc(BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + IDlen));
	id_hid = (unsigned char*)(malloc(IDlen + 1));

	JoinIDhid(ID, IDlen, hid, id_hid);

	QB = G1pointMult(
		H(id_hid, IDlen + 1, 0x01),
		P1);
	QB = G1pointAdd(QB, Ppub_e);

	do {
		r = RandomNumGenerate();

		temp = G1pointMult(r,QB);

		g = BiLinearPairing(Ppub_e, P2);

		g = Q12PowerMod(g, r);

		JoinCwID(&temp, &g, ID, IDlen, c_w_id);

		KDF(c_w_id, BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + IDlen, klen_bitsize, K);

	} while (StringEqualZero(K, klen_bitsize));

    free(c_w_id);
    free(id_hid);

    *C = temp;
}
/**
 * 
 * @brief key depackage of SM9
 * 
 * @param C a point in group G1, received from the counterpart
 * @param de encrypt_secretkey
 * @param ID ID of self
 * @param IDlen length of ID
 * @param klen_bitsize length of key to be generated, bit size!!!
 * @param K key generated
 * 
 * @result K
 * 
 */
bool SM9KeyDepackage(G1point C,  G2point de, unsigned char* ID, unsigned char IDlen, unsigned int klen_bitsize, unsigned char* K)
{
	unsigned char* c_w_id;
	q12 w;

    c_w_id = (unsigned char*)(malloc(BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + IDlen));

	if (!PointInG1(C)){
        printf("point not in G1\n");
        free(c_w_id);
        return false;
    }

	w = BiLinearPairing(C, de);

    JoinCwID(&C, &w, ID, IDlen, c_w_id);


    KDF(c_w_id, BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + IDlen, klen_bitsize, K);

    free(c_w_id);

    if (StringEqualZero(K, klen_bitsize))
		return false;

    return true;
}
/**
 * 
 * @brief Encrypt of SM9 with KDF
 * 
 * @param message data to be encrypted 
 * @param msglen_bitsieze length of message, bit size!!!
 * @param K2_len_bitsize K2_len, defined in SM9
 * @param ID ID of counterpart
 * @param IDlen length of ID
 * @param hid function flag defined in SM9
 * @param Ppub_e encrypt_publickey
 * @param C ciphertext of message
 * 
 * @result C
 * 
 */
bool SM9EncryptWithKDF(unsigned char* message, unsigned int msglen_bitsize, unsigned int K2_len_bitsize,
	unsigned char* ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e, unsigned char *C)
{
    unsigned int i;
    unsigned int klen_bitsize = msglen_bitsize + K2_len_bitsize;
    unsigned char *K, *C2, *C3, *C2K2;
    G1point C1;

    K = (uint8_t *)(malloc( ((klen_bitsize >> 8) + 1) * (SM3OUT_32BYTES)) );
    C2 = (uint8_t *)(malloc(msglen_bitsize >> 3)); // msglen_bitsize / 8
    C3 = (uint8_t *)(malloc(BIG8W_BYTESIZE));
    C2K2 = (uint8_t *)(malloc((msglen_bitsize >> 3) + (K2_len_bitsize >> 3))); // msglen_bitsize / 8 + K2_len_bitsize / 8

    do{
       SM9KeyPackage(ID, IDlen, hid, Ppub_e, klen_bitsize, K, &C1);
    } while (StringEqualZero(K, msglen_bitsize >> 3)); 

    XOR(message, msglen_bitsize >> 3, K, C2);

    for (i = 0; i < msglen_bitsize >> 3 ; i++)
		C2K2[i] = C2[i];
	for (i; i < (klen_bitsize >> 3); i++)
		C2K2[i] = K[i];

    sm3(C2K2, (msglen_bitsize >> 3) + (K2_len_bitsize >> 3), C3);

    // C = C1||C3||C2
    i = 0;
    Big8wIntou8string(&C1.x, C, i);
	i += BIG8W_BYTESIZE;
	Big8wIntou8string(&C1.y, C, i);
	i += BIG8W_BYTESIZE;

	for (i = BIG8W_BYTESIZE * 2; i < BIG8W_BYTESIZE * 2 + (SM3OUT_32BYTES); i++)
		C[i] = C3[i - BIG8W_BYTESIZE * 2];

	for (; i < BIG8W_BYTESIZE * 2 + (SM3OUT_32BYTES) + (msglen_bitsize >> 3); i++)
		C[i] = C2[i - (BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES)];

    free(C2);
	free(C3);
    free(C2K2);
    free(K);

    return true;
}
/**
 * 
 * @brief Decrypt function of SM9 with KDF
 * 
 * @param ID ID of self
 * @param IDlen length of ID
 * @param message decrypted date
 * @param msglen_bitsize length of message, bit size!!!
 * @param K2_len_bitsize K2_len defined in SM9
 * @param C ciphertext, received from counterpart
 * @param encrypt_secretkey encrypt_secretkey of self
 * 
 * @result message
 * 
 */
bool SM9DecryptWithKDF(unsigned char* ID, unsigned char IDlen, 
    unsigned char* message, unsigned int msglen_bitsize, unsigned int K2_len_bitsize, 
    unsigned char *C, G2point encrypt_secretkey)
{
    G1point C1;

    unsigned char *K;
    unsigned char *u;
    unsigned char *C2K2;
    unsigned int i;
    unsigned int klen_bitsize = msglen_bitsize + K2_len_bitsize;

    K = (uint8_t *)(malloc( ((klen_bitsize >> 8) + 1) * (SM3OUT_32BYTES)) );
    u = (unsigned char *)(malloc(SM3OUT_32BYTES));
    C2K2 = (unsigned char *)(malloc((msglen_bitsize >> 3) + (K2_len_bitsize >> 3)));

    U8stringToG1point(C, &C1);

    SM9KeyDepackage(C1, encrypt_secretkey, ID, IDlen, msglen_bitsize + K2_len_bitsize, K);

    if (StringEqualZero(K, msglen_bitsize >> 3)){
        free(K);
        free(u);
        free(C2K2);
        return false;
    }
        
    XOR(K, msglen_bitsize >> 3, C + BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE, (unsigned char*)message);

    for (i = 0; i < msglen_bitsize >> 3; i++)
		C2K2[i] = (C + BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE)[i];
	for (; i < (msglen_bitsize >> 3) + (K2_len_bitsize >> 3); i++)
		C2K2[i] = K[i];

    sm3(C2K2, (msglen_bitsize >> 3) + (K2_len_bitsize >> 3), u);

    free(K);
    free(C2K2);

    for (i = 0; i < SM3OUT_32BYTES; i++)
        if (u[i] != (C + BIG8W_BYTESIZE * 2)[i])
        {
            printf("hash value error!\n");
            free(u);;
            return false;
        }
    
    free(u);

    return true;
}
/**
 * 
 * @brief Encrypt function of SM9 with SM4
 * 
 * @param message data to be encrypted
 * @param msglen_bitsize length of message, bit size !!!
 * @param K1_len_bitsize K1_len defined in SM9, length of the key of SM4
 * @param K2_len_bitsize K2_len defined in SM9
 * @param ID ID of counterpart
 * @param IDlen length of ID
 * @param hid function flag defined in SM9
 * @param Ppub_e encrypt_publickey
 * @param C ciphertext of message
 * 
 * @result C
 * 
 */
bool SM9EncryptWithSM4(unsigned char* message, unsigned int msglen_bitsize, 
    unsigned int K1_len_bitsize, unsigned int K2_len_bitsize,
	unsigned char* ID, unsigned char IDlen, unsigned char hid, G1point Ppub_e, 
    unsigned char *C)
{
    unsigned int i;
    unsigned int klen_bitsize = K1_len_bitsize + K2_len_bitsize;
    unsigned int C2byteslen = ((K1_len_bitsize >> 7) + 1) * (SM4OUT);
    unsigned char *K, *C2, *C3, *C2K2;
    G1point C1;

    K = (uint8_t *)(malloc(((klen_bitsize >> 8) + 1) * SM3OUT_32BYTES));
    C2 = (uint8_t *)(malloc(C2byteslen));
    C3 = (uint8_t *)(malloc(256 / 8));
    C2K2 = (uint8_t *)(malloc(C2byteslen + (K2_len_bitsize >> 3)));

    do{
        SM9KeyPackage(ID, IDlen, hid, Ppub_e, klen_bitsize, K, &C1);
    } while (StringEqualZero(K, K1_len_bitsize >> 3));

    SM4EncryptWithEcbMode(message, (msglen_bitsize >> 3), K, C2);

    for (i = 0; i < C2byteslen; i++) // join C2
		C2K2[i] = C2[i];
	for (; i < C2byteslen + (K2_len_bitsize >> 3); i++) // join K2
		C2K2[i] = K[i - C2byteslen + (K1_len_bitsize >> 3)];

    sm3(C2K2, C2byteslen + (K2_len_bitsize >> 3), C3);

    // C = C1||C3||C2
    i = 0;
	Big8wIntou8string(&C1.x, C, i);
	i += BIG8W_BYTESIZE;
	Big8wIntou8string(&C1.y, C, i);
	i += BIG8W_BYTESIZE;

	for (i = BIG8W_BYTESIZE * 2; i < BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES; i++)
        C[i] = C3[i - (BIG8W_BYTESIZE * 2)];

    for (i = BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES; i < BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES + C2byteslen; i++)
		C[i] = C2[i - (BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES)];

    free(C2);
	free(C3);
    free(C2K2);
    free(K);

    return true;
}
/**
 * 
 * @brief Decrypt of SM9 with SM4
 * 
 * @param ID ID of self
 * @param IDlen length of ID
 * @param message  message to be decrypted, store the result
 * @param msglen length of message, byte size
 * @param K1_len_bitsize K1_len defined in SM9, bit size, length of key of SM4
 * @param K2_len_bitsize K2_len defined in SM9, bit size
 * @param C ciphertext received from counterpart
 * @param Cbyteslen length of C, byte size
 * @param encrypt_secretkey encrypt_secretkey of self
 * 
 * @result message
 * 
 */
bool SM9DecryptWithSM4(unsigned char* ID, unsigned char IDlen, 
    unsigned char* message, unsigned int msglen, unsigned int K1_len_bitsize, unsigned int K2_len_bitsize, 
    unsigned char *C, unsigned int Cbyteslen, G2point encrypt_secretkey)
{
    G1point C1;
    unsigned int i;
    unsigned char *K;
    unsigned char *u;
    unsigned char *C2K2;
    unsigned int c_w_id_len;
    unsigned int C2byteslen = Cbyteslen - (BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES);
    unsigned int klen_bitsize = K1_len_bitsize + K2_len_bitsize;

    K = (uint8_t *)(malloc(((klen_bitsize >> 8) + 1) * SM3OUT_32BYTES));
    u = (unsigned char *)(malloc(SM3OUT_32BYTES));
    C2K2 = (unsigned char *)(malloc(C2byteslen + (K2_len_bitsize >> 3)));

    U8stringToG1point(C, &C1);

    if (!SM9KeyDepackage(C1, encrypt_secretkey, ID, IDlen, (K1_len_bitsize + K2_len_bitsize), K))
        return false;

    SM4DecryptWithEcbMode((C + BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES), C2byteslen, message, msglen, K);

    for (i = 0; i < C2byteslen; i++)
		C2K2[i] = (C + BIG8W_BYTESIZE * 2 + SM3OUT_32BYTES)[i];
	for (i = C2byteslen; i < C2byteslen + (K2_len_bitsize >> 3); i++)
		C2K2[i] = K[i - C2byteslen + (K1_len_bitsize >> 3)];
    
    sm3(C2K2, C2byteslen + (K2_len_bitsize >> 3), u);

    free(K);
    free(C2K2);

    for (i = 0; i < SM3OUT_32BYTES; i++)
        if (u[i] != (C + BIG8W_BYTESIZE * 2)[i]){
            printf("u != C3, decrypt failed\n");
            free(u);
            return false;
        }

    free(u);

    return true;
}