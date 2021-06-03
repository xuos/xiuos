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
* @file sm9_util.c
* @brief the function called by SM9 function, including hash, KDF, produce random number, encrypt and decrypt algorithm, BiLinearPairing
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <sm9_util.h> 
#include <sm4.h>

/**
 * 
 * @brief generate random bytes
 * 
 * @param null
 * 
 * @return uint8_t
 * 
 */
uint8_t hw_get_random_byte(void)
{
  static uint32_t lcg_state;
  static uint32_t nb_soft = 9876543;
#define MAX_SOFT_RNG 1024
  static const uint32_t a = 1664525;
  static const uint32_t c = 1013904223;

  nb_soft = (nb_soft + 1) % MAX_SOFT_RNG;
  lcg_state = (a * lcg_state + c);
  return (uint8_t) (lcg_state >> 24);
}
/**
 * 
 * @brief generate random bytes stream
 * 
 * @param buf buffer to store the random bytes
 * @param blen buffer length
 * 
 * @return 1 if produce random bytes stream; 0 if buffer is null 
 */
int crypto_rng_read(uint8_t *buf, size_t blen)
{
  uint8_t *b = buf;
  size_t n;

  if (!b)
    return -1;

  for (n = 0; n < blen; n++)
    b[n] = hw_get_random_byte();

  return 1;
}
/**
 * 
 * @brief generate random big number based on crypto_rng_read, bad prng version , only for test. 
 * TODO: replace next version
 * 
 * @param null
 * 
 * @return ret, big number r
 * 
 */
big8w RandomNumGenerate()
{
	char i = 0, j = 0;
	unsigned char random_byte[4];
	big8w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++){
		for (j = 0; j < 4; j++)
			random_byte[j] = hw_get_random_byte();
		ret.word[i] = GETU32(random_byte);
	}

	if (Big8wBigThan(&ret, &curve.N))
		ret = Big8wMinusMod(ret, curve.N, curve.N);
	
	return ret;
}
/**
 * 
 * @brief HashTwice = Hash(funcflag||g1||(Hash(g2||g3||ID_A||ID_B||RA||RB)))
 *              t1 = (g2||g3||ID_A||ID_B||RA||RB), t2 = (funcflag||g1||hashvalue)
 * 
 * @param ID_A id of challenger
 * @param ID_A_len length of ID_A, byte size
 * @param ID_B id of responser
 * @param ID_B_len length of ID_B, byte size
 * @param RA a pointer of a point in group G1
 * @param RB a pointer of a point in group G1
 * @param g1 a pointer of a number in Fq12
 * @param g2 a pointer of a number in Fq12
 * @param g3 a pointer of a number in Fq12
 * @param funcflag  defined in SM9, funcflag = 0x82 or 0x83
 * @param ret unsigned char string, store the result
 * 
 * @result ret
 * 
 */
void HashTwice(uint8_t* ID_A, uint8_t ID_A_len, uint8_t* ID_B, uint8_t ID_B_len, 
G1point* RA, G1point* RB,
q12* g1, q12* g2, q12* g3, uint8_t funcflag, uint8_t* ret)
{
	uint8_t* t1, *t2;
	uint32_t i, temp, msglen;

	t1 = (uint8_t*)(malloc(ID_A_len + ID_B_len + BIG8W_BYTESIZE * 12 * 2 + BIG8W_BYTESIZE * 2 * 2));
	t2 = (uint8_t*)(malloc(1 + BIG8W_BYTESIZE * 12 + SM3OUT_32BYTES));

	i = 0;
	Q12Intou8string(g2, t1, i);
	i += BIG8W_BYTESIZE * 12;
	Q12Intou8string(g3, t1, i);
	i += BIG8W_BYTESIZE * 12;
	
	for (temp = 0; temp < ID_A_len; temp++)
		t1[i + temp] = ID_A[temp];
	
	i += temp;

	for (temp = 0; temp < ID_B_len; temp++)
		t1[i + temp] = ID_B[temp];
	i += temp;

	Big8wIntou8string(&RA->x, t1, i); // join RA and t1
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&RA->y, t1, i);
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&RB->x, t1, i); // join RB and t1
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&RB->y, t1, i);
	i += BIG8W_BYTESIZE;
	msglen = i;

	t2[0] = funcflag; 
	i = 1;

	Q12Intou8string(g1, t2, i);
	i += BIG8W_BYTESIZE * 12;

	sm3(t1, msglen, t2 + i);
	sm3(t2, 1 + BIG8W_BYTESIZE * 12 + SM3OUT_32BYTES, ret);

	free(t1);
	free(t2);
}
/**
 * @brief judge if elements in string are zero
 * 
 * @param string unsigned char string to be checked
 * @param stringlen length of string
 * 
 * @return true if all elements in string is zero; else false 
 * 
 */
bool StringEqualZero(uint8_t* string, uint32_t stringlen)
{
	uint32_t length = 0;

	while (length < stringlen) 
		if (string[length++]) 
			return false;

	return true;
}
/**
 * 
 * @brief defined in SM9:H(Z, n); n = curve.N, 255 bits, hlen = 2(therefore temp[i] = 0x01, 0x02)
 * 
 * @param Z unsigned char string
 * @param Zlen length of Z
 * @param funcflag defined in SM9, funcflag = 0x01 or 0x02; H1() or H2()
 * 
 * @return ret big number
 * 
 */
big8w H(uint8_t* Z, uint32_t Zlen, uint8_t funcflag)
{
	uint8_t* temp;
    uint8_t Ha[40];
    uint8_t Ha2[32];
    uint32_t ctr = 0x01, i = 0;
    uint32_t datalen;
	big8w tmp = curve.N;
    big16w ret;

    tmp.word[0] -= 1; // tmp = curve.N - 1

	datalen = 1 + Zlen + 4;
    temp = (uint8_t *)malloc(datalen);
    memset(temp, 0x00, datalen);
    memset(ret.word, 0x00, BIG8W_BYTESIZE * 2);

    temp[0] = funcflag; // temp = funcfalg||Z||ctr

	for (i = 0; i < Zlen; i++) 
		temp[i + 1] = Z[i];
	
	temp[++i] = 0;
	temp[++i] = 0;
	temp[++i] = 0;

	temp[++i] = 0x01;
	sm3(temp, datalen, Ha);

	temp[i] = 0x02;
	sm3(temp, datalen, Ha2);
    free(temp);

    for (i = 32; i < 40; i++)
		Ha[i] = Ha2[i - 32];
	
	for (i = 0; i < 10; i++) 
		ret.word[9 - i] = GETU32(Ha + i * 4);
	
	while (ret.word[i] == 0)
        i--;
	ret.length = i;

	tmp = Big16wmod8w(ret, tmp); // tmp = Ha mod (N-1)

	i = 0; // tmp += 1
	while (tmp.word[0] == 0xffffffff) // if overflow after plus one
		i++;
	tmp.word[i]++;
	if (i) while (i) { // if overflow
		i--;
		tmp.word[i] = 0;
	}

	return tmp;
}
/**
 * 
 * @brief key derive function defined in SM9, baseed of hash function(SM3).
 * 
 * @param Z unsigned char string
 * @param Zlen length of Z
 * @param klen length of key to be derived, bitsize!!!
 * @param ret unsigned char string to store the key derived.
 * 
 * @result ret
 * 
 */
void KDF(uint8_t* Z, uint32_t Zlen, uint32_t klen, uint8_t* ret)
{
	uint8_t* tmp, Ha[32];
	uint32_t i, round;
    uint32_t ctr = 0x00000001;

    round = klen >> 8;
	if (klen & 0xff)
		round++;

	tmp = (uint8_t*)(malloc(Zlen + sizeof(uint32_t))); // tmp = Z||ctr

	for (i = 0; i < Zlen; i++) 
		tmp[i] = Z[i];
	
	for (ctr = 1; ctr <= round; ctr++) {
		tmp[i] = (ctr >> 24) & 0xff;
		tmp[i+1] = (ctr >> 16) & 0xff;
		tmp[i+2] = (ctr >> 8) & 0xff;
		tmp[i+3] =  ctr & 0xff;
		sm3(tmp, Zlen + sizeof(uint32_t), ret + 32 * (ctr - 1));
	}
}
/**
 * @brief SM4 Encrypt Function With ecb mode
 * 
 * @param message data to be encrypted
 * @param msglen length of message, byte size
 * @param key key of SM4 algorithm
 * @param ciphertext unsigned char string to store the encrypted message
 * 
 * @result ciphertext
 * 
 */
void SM4EncryptWithEcbMode(uint8_t* message, uint32_t msglen, uint8_t* key, uint8_t* ciphertext)
{
	sms4_key_t renckey;
	uint32_t ciphertextlen;
	sms4_set_encrypt_key(&renckey, key);
	Sms4EcbEncryptPkcs7Padding(message, msglen, ciphertext, &ciphertextlen, &renckey);
}
/**
 * @brief SM4 Decrypt Function 
 * 
 * @param ciphertext encrypted message to be decrypted
 * @param ciphertextlen length of ciphertext,  % 128 == 0
 * @param message unsigned char string to store decrypted data
 * @param msglen length of message
 * @param key key of SM4 algorithm
 * 
 * @result message
 * 
 */
void SM4DecryptWithEcbMode(uint8_t* ciphertext, uint32_t ciphertextlen, uint8_t* message, int msglen, uint8_t* key)
{
	uint32_t i;

	sms4_key_t rdeckey;
	sms4_set_decrypt_key(&rdeckey, key);
	Sms4EcbDecryptPkcs7Padding(ciphertext, ciphertextlen, message, &msglen, &rdeckey);
}
/**
 * @brief function defined in SM9, e(P, Q)
 * 
 * @param P a point in group G1
 * @param Q a point in group G2
 * 
 * @return a number in Fq12
 * 
 */
q12 BiLinearPairing(G1point P, G2point Q)
{
	bool flag;
	uint32_t elem = 0x215d93e, i; // a = 6*t+2 = 0x2400000000215d93e, used in Miller round
	int bitindex = 25; // index of elem's hightest bit
	G2point T = Q, Q1, Q2;
	q12 f;
	
	// Miller round
	Q12Zero(&f);
	f.low.low.low.word[0] = 1;

	f = Q12MultMod(f, f); // 0
	Line(P, &T, T, 1, &f);

	f = Q12MultMod(f, f); // 0
	Line(P, &T, T, 1, &f);

	f = Q12MultMod(f, f); // 1
	Line(P, &T, T, 1, &f);
	Line(P, &T, Q, 0, &f);

	for (i = 0; i < 36; i++) { // 36 continuous 0
		f = Q12MultMod(f, f);
		Line(P, &T, T, 1, &f);
	} 

	for (; bitindex >= 0; bitindex--) { // scan 0x215d93e
		flag = (elem >> bitindex) & 1;
		f = Q12MultMod(f, f);
		Line(P, &T, T, 1, &f);
		if (flag)
			Line(P, &T, Q, 0, &f);
	}

	// Frobenius of Q
	G2pointFrobenius(Q, &Q1, 1);
	G2pointFrobenius(Q, &Q2, 2);

	Line(P, &T, Q1, 0, &f);

	Q2.y.high = Big8wMinusMod(curve.q, Q2.y.high, curve.q);
	Q2.y.low = Big8wMinusMod(curve.q, Q2.y.low, curve.q);
	Line(P, &T, Q2, 0, &f);

	LastPower(&f);

	return f;
}
