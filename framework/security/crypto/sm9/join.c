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
* @file join.c
* @brief convert data type and join string 
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/
#include <join.h>

/**
 * 
 * @brief convert big8w to char string and insert into string, start from startindex
 * 
 * @param bignum a big number
 * @param string a string of unsigned char
 * @param startindex the start index of string convert to bignum
 * 
 * @return string
 * 
 */
void Big8wIntou8string(big8w* bignum, uint8_t* string, uint32_t startindex)
{
	int index;
	uint32_t elem;

	for (index = BIGNUMBER_SIZE_8WORD - 1; index >= 0; index--) {
		elem = bignum->word[index];
		string[startindex++] = (uint8_t)(elem >> 24);
		string[startindex++] = (uint8_t)(elem >> 16);
		string[startindex++] = (uint8_t)(elem >> 8);
		string[startindex++] = (uint8_t)(elem);
	}
}
/**
 * 
 * @brief convert q12 to char string and insert into string, start from startindex
 * 
 * @param num a pointer of a number in Fq12
 * @param string a string of unsigned int
 * @param startindex the start index of string to insert.
 * 
 * @return string
 * 
 */
void Q12Intou8string(q12* num, uint8_t* string, uint32_t startindex)
{
	Big8wIntou8string(&num->high.high.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->high.high.low, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->high.low.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->high.low.low, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->mid.high.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->mid.high.low, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->mid.low.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->mid.low.low, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->low.high.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->low.high.low, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->low.low.high, string, startindex);
	startindex += BIG8W_BYTESIZE;

	Big8wIntou8string(&num->low.low.low, string, startindex);
	startindex += BIG8W_BYTESIZE;
}
/**
 * @brief convert unsigned char string to a point in group G1.
 * 
 * @param string unsinged char string 
 * @param ret pointer of a point in group G1
 * 
 * @return ret
 * 
 */
void U8stringToG1point(uint8_t* string, G1point *ret)
{
	uint32_t i;

	for (i = 1; i < BIGNUMBER_SIZE_8WORD + 1; i++)
		ret->x.word[BIGNUMBER_SIZE_8WORD - i] = GETU32((string + 4*(i-1)));

	for (i = 1; i < BIGNUMBER_SIZE_8WORD + 1; i++)
		ret->y.word[BIGNUMBER_SIZE_8WORD - i] = GETU32((string + BIG8W_BYTESIZE + 4*(i-1)));
}
/**
 * @brief join ID and hid
 * 
 * @param ID id, unsigned char string
 * @param hid hid(0x01, 0x02, 0x03), unsigned char, defined in SM9
 * @param ret unsigned char string, store the result of joined ID and hid
 * 
 * @return ret
 * 
 */
void JoinIDhid(uint8_t* ID, uint8_t IDlen, uint8_t hid, uint8_t* ret)
{
	uint8_t i;
	
	for (i = 0; i < IDlen; i++)
		ret[i] = ID[i];

	ret[i] = hid;
}
/**
 * @brief join message and w(q12 number).
 * 
 * @param message message, unsigned char string
 * @param msglen length of message, byte size
 * @param w pointer of a number in Fq12
 * @param ret unsigned char string, store the result
 * 
 * @return ret
 * 
 */
void JoinMsgW(uint8_t* message, uint32_t msglen, q12* w, uint8_t* ret)
{
	uint32_t i = 0;

	for (i = 0; i < msglen; i++) 
		ret[i] = message[i];

	Q12Intou8string(w, ret, msglen);
}
/**
 * @brief join IDA, IDB, RA, RB, g1, g2, g3, defined in SM9
 * 
 * @param ID_Challenger id of challenger, unsigned char string
 * @param ID_Challenger_len length of the id of challenger, byte size, unsigned char
 * @param ID_Responser id of responser, unsigned char string
 * @param ID_Responser_len length of the id of responser, byte size, unsigned char
 * @param R_Challenger pointer of a point in group G1
 * @param R_Responser pointer of a point in group G1
 * @param g1 pointer of a number in Fq12, defined in SM9
 * @param g2 pointer of a number in Fq12, defined in SM9
 * @param g3 pointer of a number in Fq12, defined in SM9
 * @param ret unsigned char string, store the result
 * 
 * @result ret
 * 
 */
void JoinIDAIDBRARBg123(
    uint8_t *ID_Challenger, uint8_t ID_Challenger_len,
    uint8_t *ID_Responser, uint8_t ID_Responser_len,
    G1point* R_Challenger, G1point* R_Responser,
    q12 *g1, q12 *g2, q12 *g3, 
    uint8_t* ret)
{
	uint32_t i = 0;
	uint32_t index = 0;

	index = 0;
	while (index < ID_Challenger_len)
		ret[i++] = ID_Challenger[index++];
	index = 0;
	while (index < ID_Responser_len)
		ret[i++] = ID_Responser[index++];

	Big8wIntou8string(&R_Challenger->x, ret, i);
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&R_Challenger->y, ret, i);
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&R_Responser->x, ret, i);
	i += BIG8W_BYTESIZE;

	Big8wIntou8string(&R_Responser->y, ret, i);
	i += BIG8W_BYTESIZE;

	Q12Intou8string(g1, ret, i);
	i += BIG8W_BYTESIZE * 12;

	Q12Intou8string(g2, ret, i);
	i += BIG8W_BYTESIZE * 12;

	Q12Intou8string(g3, ret, i);

}
/**
 * @brief join C, w, ID, defined in SM9
 * 
 * @param C a pointer of a point in group G1
 * @param w a pointer of a number in Fq12
 * @param ID unsinged char string
 * @param IDlen length of ID, byte size, unsinged char
 * @param ret unsinged char string , store the result
 * 
 * @result ret
 * 
 */
void JoinCwID(G1point* C, q12* w, uint8_t* ID, uint8_t IDlen, uint8_t* ret)
{
	uint8_t index = 0;
	uint32_t i = 0;

	Big8wIntou8string(&C->x, ret, i);
	i += BIG8W_BYTESIZE;
	Big8wIntou8string(&C->y, ret, i);
	i += BIG8W_BYTESIZE;

	Q12Intou8string(w, ret, i);
	i += BIG8W_BYTESIZE * 12;

	while (index < IDlen)
		ret[i++] = ID[index++];
}
/**
 * 
 * @brief msg xor K; u8string xor u8string
 * 
 * @param msg message, unsigned char string 
 * @param msglen length of message, byte size, unsigned int
 * @param K produced by KDF(), unsigned char string
 * @param ret unsigned char string, store the result
 * 
 * @result ret
 * 
 */
void XOR(unsigned char* msg, uint32_t msglen, unsigned char* K, unsigned char* ret)
{
	uint32_t i;
	for (i = 0; i < msglen; i++)
		ret[i] = msg[i] ^ K[i];
		
}