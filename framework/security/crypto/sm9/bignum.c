/**
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
* @file bignum.c
* @brief arithmetic of big number
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <bignum.h>

sm9curve curve;
// used in Montgomery Mult
uint32_t qlow_reverse = 0x2f2ee42b; // power(2, 32) - (curve.q.word[0] 's reverse under power(2, 32))
uint32_t Nlow_reverse = 0x51974b53; // power(2, 32) - (curve.N.word[0] 's reverse under power(2, 32))
big8w q_2k; // (2^(256*2)) mod curve.q; used in big numbers' mult(Montgomery Mult)
big8w N_2k; // (2^(256*2)) mod curve.N; used in big numbers' mult(Montgomery Mult)

/**
 * @brief This function is to print the big number in hex.
 * 
 * @param bignum pointer of a big number
 * 
 * @return null
 * 
 */
void Big8wPrint(big8w* bignum)
{
	int i = BIGNUMBER_SIZE_8WORD - 1;
	while (bignum->word[i] == 0 && i >= 0)
		i--;
	if (i < 0) {
		KPrintf("0x00\n");
		return;
	}

	KPrintf("0x %08x", bignum->word[i]);

	if(i--) // i > 0
		for (; i>=0; i--) 
			KPrintf(" %08x", bignum->word[i]);
		
	KPrintf("\n");
} 
/**
 * @brief This function is to get the index of highest bit of highest word.
 * 
 * @param bignum pointer of a big number 
 * 
 * @return null
 * 
*/
uint8_t Big8wHighestbit(big8w* bignum)
{
	uint8_t i = BIGNUMBER_SIZE_8WORD - 1;
	uint32_t elem;

	while (bignum->word[i] == 0 && i >= 0) 
		i--;
	elem = bignum->word[i];

	i = 32;
	while(--i) 
		if ((elem >> i) & 1) 
			break;

	return i;
}
/**
 * 
 * @brief This function is to judge if a big number is zero
 * 
 * @param bignum pointer of a big number
 * 
 * @return true if bignum == 0; else false
 * 
 */
bool Big8wIsZero(big8w* bignum)
{
	char i = 0;

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++) 
		if (bignum->word[i]) 
			return false;

	return true;
}
/**
 * 
 * @brief return bn1 >= bn2
 * 
 * @param bn1 the first big number
 * @param bn2 the second big number
 * 
 * @return true if bn1 >= bn2; false if bn1 < bn2
 * 
 */
bool Big8wBigThan(big8w* bn1, big8w* bn2)
{
	uint8_t i = BIGNUMBER_SIZE_8WORD - 1;

	for (; i; i--) {
		if (bn1->word[i] > bn2->word[i])
			return true;

		else if (bn1->word[i] < bn2->word[i]) 
			return false;
	} 

	return bn1->word[i] >= bn2->word[i];
}
/**
 * @brief reutrn bn1 == bn2
 * 
 * @param bn1 the first big number
 * @param bn2 the second big number
 * 
 * @return true if bn1 == bn2; else false
 * 
 */
bool Big8wEqual(big8w* bn1, big8w* bn2)
{
	uint8_t i = BIGNUMBER_SIZE_8WORD - 1;

	for (; i; i--) 
		if (bn1->word[i] != bn2->word[i])
			return false; 

	return bn1->word[i] == bn2->word[i];
}
/**
 * @brief compute (bn1 - bn2)%p
 * 
 * @param bn1 the first big number, smaller than p
 * @param bn2 the second big number, smaller than p
 * @param p a big number, the module number
 * 
 * @return ret, a big number
 */
big8w Big8wMinusMod(big8w bn1, big8w bn2, big8w p)
{
	bool borrow = 0;
	char i = 0;
	big8w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	if (Big8wEqual(&bn2, &bn1))   
		return ret;

	else if (Big8wBigThan(&bn2, &bn1)) { // p - (bn2 - bn1)
		ret = Big8wMinusMod(bn2, bn1, p);
		ret = Big8wMinusMod(p, ret, p);
		return ret;
	}

	borrow = 0;
	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++){
		ret.word[i] = bn1.word[i] - bn2.word[i] - borrow;
		borrow = (ret.word[i] < bn1.word[i] || ((ret.word[i] == bn1.word[i]) && borrow == 0)) ? 0 : 1;
	}

	return ret;
}
/**
 * @brief compute (bn1 + bn2)%p
 * 
 * @param bn1 the first big number 
 * @param bn2 the second big number
 * @param p a big number, the module number
 * 
 * @return ret, a big number
 * 
 */
big8w Big8wAddMod(big8w bn1, big8w bn2, big8w p)
{
	bool flag = 0;
	uint8_t i = 0;
	big8w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++){
		ret.word[i] = bn1.word[i] + bn2.word[i] + flag;
		flag = (
			(ret.word[i] > bn1.word[i] && ret.word[i] > bn2.word[i] ) 
			|| ((ret.word[i]==bn1.word[i] ||ret.word[i]==bn2.word[i]) && flag == 0)
			) ? 0 : 1;
	}

	if (flag) {
		//  (2^(32*8)) + ret - p = (2^(32*8)-1) - (p - ret) + 1
		// ret = p - ret
		ret = Big8wMinusMod(p, ret, p);

		// ret = (2^(32*8)-1) - (p - ret)
		for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++) 
			ret.word[i] = 0xffffffff - ret.word[i];

		// ret++
		i = 0;
		while (i < BIGNUMBER_SIZE_8WORD && ret.word[i] == 0xffffffff) 
			i++;
		ret.word[i]++; // plus one
		if (i) 
			while (--i)
				ret.word[i] = 0;		
	}

	if (Big8wBigThan(&ret, &p))
		ret = Big8wMinusMod(ret, p, p);

	return ret;
}
/**
 * @brief big number << (kround * 32 + rest), result store in big16w
 * 
 * @param bignum a big number
 * @param kround (left shift bits) // 32
 * @param rest (left shift bits) % 32
 * @param length length of bignum, size = unsigned int 
 * @param highestbit index of the highest bit of highest word of bignum, 0 <= highest <= 31
 * 
 * @return ret, 16word big number
 * 
 */
big16w Big16wLeftShift(big8w bignum, uint8_t kround, uint8_t rest, uint8_t length, uint8_t highestbit)
{
	char i = 0;
	big16w ret;
	memset(ret.word, 0x00, BIG8W_BYTESIZE * 2);

	for (i = 0; i <= length; i++) 
		ret.word[i + kround] = bignum.word[i];
	ret.length = length + kround;

	if (rest) {
		if (rest + highestbit > 31) 
			ret.length++;
		for (i = ret.length; i >kround; i--) 
			ret.word[i] = ret.word[i] << rest | ret.word[i - 1] >> (32 - rest);
		ret.word[i] <<= rest;
	}

	return ret;
}
/**
 * @brief This function is to get the index of highest bit of highest word of a big number of 16word size.
 * 
 * @param bignum16w pointer of a big number of 16word size.
 * 
 * @return ret, unsigned char
 * 
 */
uint8_t Big16wHighestbit(big16w bignum16w) 
{
	uint8_t ret = 31;
	uint32_t elem = bignum16w.word[bignum16w.length];

	if (bignum16w.length == 0 && bignum16w.word[bignum16w.length] == 0) 
		return 0;

	while (true) {
		if (((elem >> ret) & 1) == 0)
			ret--;
		else
			return ret;
	} // end while

}
/**
 * @brief return bn1 >= bn2
 * 
 * @param bn1 the first big number of 16word size.
 * @param bn2 the second big number of 16word size.
 * 
 * @return true if bn1 >= bn2; else false
 * 
 */
bool Big16wBigThan(big16w bn1, big16w bn2)
{
	uint8_t i;

	if (bn1.length > bn2.length) 
		return true;
	else if (bn1.length < bn2.length) 
		return false;

	for (i = bn1.length; i > 0; i--){
		if (bn1.word[i] > bn2.word[i]) 
			return true;
		else if (bn1.word[i] < bn2.word[i]) 
			return false;
	}

	return bn1.word[0] >= bn2.word[0];
}
/**
 * @brief return (bn1 - bn2)
 * 
 * @param bn1 the first big number of 16word size.
 * @param bn2 the second big number of 16word size.
 * 
 * @return (bn1 - bn2), a big numbe of 16word size
 * 
 */
big16w Big16wMinus(big16w bn1, big16w bn2)
{
	bool borrow;
	char len = bn1.length;
	int i = 0;
	big16w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE * 2);

	borrow = 0;
	for (i = 0; i <= bn1.length; i++){
		ret.word[i] = bn1.word[i] - bn2.word[i] - borrow;
		borrow = (ret.word[i] < bn1.word[i] || ((ret.word[i] == bn1.word[i]) && borrow == 0)) ? 0 : 1;
	}

	i = bn1.length;
	while (ret.word[i] == 0) 
		i--;
	ret.length = i;
	if (i < 0) 
		ret.length = 0;

	return ret;
}
/**
 * 
 * @brief This function is only called by function H(), which is in topfunc.h
 * 		while(bignum16w.length > 7) // bignum16w > p
 * 				bignum16w = bignum16w - (p << gap)  or bignum16w = (p << gap) - bignum16w, turn = !turn
 * 
 * 		if (turn) // bignum16w == (p - bignum16w) mod p,  bignum16w == (- ret) mod p
 * 			bignum16w = p - bignum16w 
 * 
 * @param bignum16w big number of 16word size.
 * @param p big number, the module number.
 * 
 * @return bignum16w % p.
 *
 */
big8w Big16wmod8w(big16w bignum16w, big8w p)
{
	bool turn = false;
	char plen = 7;
	char pbit = Big8wHighestbit(&p);
	int gap;
	big8w ret;
	big16w temp;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	while (p.word[plen] == 0) 
		plen--;

	while (bignum16w.length > 7){
		gap = bignum16w.length * 32 + Big16wHighestbit(bignum16w) - 255; // 255 = bitlen of p
		temp = Big16wLeftShift(p, gap >> 5, gap & 0x1f, plen, pbit);
		if (Big16wBigThan(bignum16w, temp)) 
			bignum16w = Big16wMinus(bignum16w, temp);
		else {
			bignum16w = Big16wMinus(temp, bignum16w);
			turn = !turn;
		}// end else
	}

	for (gap = 7; gap >= 0; gap--) 
		ret.word[gap] = bignum16w.word[gap];
	while (Big8wBigThan(&ret, &p)) 
		ret = Big8wMinusMod(ret, p, p);
	if (turn) 
		ret = Big8wMinusMod(p, ret, p);
	
	return ret;
}
/**
 * @brief big number right shift
 * 
 * @param bignum big number
 * @param round bit length of big number to right shift
 * 
 * @return big number = (bignum >> round)
 * 
 */
big8w Big8wRightShift(big8w bignum, uint8_t round)
{
	uint8_t kround = round >> 5;
	uint8_t rest = round & 0x1f;
	char i;
	big8w ret;
	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	for (i = 0; i < BIGNUMBER_SIZE_8WORD - kround; i++) 
		ret.word[i] = bignum.word[i + kround];

	if (rest) {
		if (kround) {
			for (i = 0; i < BIGNUMBER_SIZE_8WORD - kround; i++) {
				ret.word[i] = (ret.word[i] >> rest) | (ret.word[i + 1] << (32 - rest));
			} // end for
		}else {
			for (i = 0; i < BIGNUMBER_SIZE_8WORD - 1; i++) {
				ret.word[i] = (ret.word[i] >> rest) | (ret.word[i + 1] << (32 - rest));
			} // end for
			ret.word[7] >>= rest;
		}
	} // end if 

	return ret;
}
/**
 * 
 * @brief return (bignum + N)>>1. (bignum + N) is even. only called by Big8wReverse, more in Big8wReverse
 * 
 * @param bignum the first big number
 * @param N the second big number
 * 
 * @return a big number = (bignum + N) >> 1. 
 * 
 */
big8w PlusAndRightShiftOne(big8w bignum, big8w N)
{
	bool flag = 0;
	uint8_t i = 0;
	big8w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++) {
		ret.word[i] = bignum.word[i] + N.word[i] + flag;

		flag = (
			(ret.word[i] > bignum.word[i] && ret.word[i] > N.word[i]) 
			|| ((ret.word[i] == bignum.word[i] || ret.word[i] == N.word[i]) && flag == 0)
			) ? 0 : 1;
	}

	ret = Big8wRightShift(ret, 1);
	if (flag) 
		ret.word[7] |= 0x80000000;
	
	return ret;
}
/**
 * 
 * @brief get reverse of bignum under N; implemented with Stein algorithm.  
 * Calls: Big8wRightShift, PlusAndRightShiftOne, Big8wEqual, Big8wMinusMod
 * 
 * @param bignum a big number
 * @param N a big prime number
 * 
 * @return a big number = (bignum)^(-1) mod N
 * 
 */
big8w Big8wReverse(big8w bignum, big8w N)
{
	bool flag1, flag2;
	big8w ret, zero, one, x1, y1, x2, y2, temp;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);
	memset(zero.word, 0x00, BIG8W_BYTESIZE);
	memset(one.word, 0x00, BIG8W_BYTESIZE);

	one.word[0] = 1;

	x1 = bignum, y1 = one;
	x2 = N, y2 = zero;

	while (true){
		flag1 = ((x1.word[0]&1) == 0), flag2 = ((y1.word[0]&1) == 0);
		if (flag1 && flag2) {
			x1 = Big8wRightShift(x1, 1);
			y1 = Big8wRightShift(y1, 1);
		}
		else if (flag1 && !flag2) {
			x1 = Big8wRightShift(x1, 1);
			y1 = PlusAndRightShiftOne(y1, N);
		}
		if (Big8wEqual(&x1, &one))
			return y1;

		flag1 = ((x2.word[0]&1) == 0), flag2 = ((y2.word[0]&1) == 0);
		if (flag1 && flag2) {
			x2 = Big8wRightShift(x2, 1);
			y2 = Big8wRightShift(y2, 1);
		}
		else if (flag1 && !flag2) {
			x2 = Big8wRightShift(x2, 1);
			y2 = PlusAndRightShiftOne(y2, N);
		}
		if (Big8wEqual(&x2, &one))
			return y2;

		if (Big8wBigThan(&x1, &x2)) {
			x1 = Big8wMinusMod(x1, x2, N);
			y1 = Big8wMinusMod(y1, y2, N);
			if (Big8wEqual(&x1, &one)) 
				return y1;
		}
		else {
			x2 = Big8wMinusMod(x2, x1, N);
			y2 = Big8wMinusMod(y2, y1, N);
			if (Big8wEqual(&x2, &one))
				return y2;
		}

	} // end while
}
/**
 * 
 * @brief return bn1 >= bn2
 * 
 * @param bn1 string of unsigned int, length <= BIGNUMBER_SIZE + 1.
 * @param bn2  string of unsigned int, length <= BIGNUMBER_SIZE + 1.
 * 
 * @return true if bn1 >= bn2; else false
 * 
 */
bool U32CMP(uint32_t* bn1, uint32_t* bn2)
{
	int i;

	for (i = BIGNUMBER_SIZE_8WORD + 1; i; i--) {
		if (bn1[i] > bn2[i]) 
			return true;
		else if (bn1[i] < bn2[i]) 
			return false;
	}

	return bn1[0] >= bn2[0];
}
/**
 * @brief This function is to compute a big number multiply a unsinged int number.
 * 
 * @param bignum  big number
 * @param elem unsigned int
 * @param ret  pointer of a string of unsigned int, length <= BIGNUMBER_SIZE_WORD + 2. store the result.
 * 
 * @result ret = bignum * elem,
 * 
 */
void Big8wMultNum(big8w bignum, uint32_t elem, uint32_t* ret)
{
	char i = 0;
	uint32_t overflow = 0;
	uint64_t temp;

	memset(ret, 0x00, sizeof(uint32_t) * (BIGNUMBER_SIZE_8WORD + 2));

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++) {
		temp = ((uint64_t)elem * (uint64_t)bignum.word[i]) + (uint64_t)overflow;
		ret[i] = temp;
		overflow = temp >> 32;
	}

	ret[BIGNUMBER_SIZE_8WORD] = overflow;
}
/**
 * @brief add two unsigned int strings.
 * 
 * @param bn1 string of unsigned int, lenght < BIGNUMBER_SIZE_8WORD + 2
 * @param bn2 string of unsigned int, lenght < BIGNUMBER_SIZE_8WORD + 2
 * @param ret string of unsigned int, lenght < BIGNUMBER_SIZE_8WORD + 2, store the result
 * 
 * @result ret, string of unsigned int
 */
void U32Add(uint32_t* bn1, uint32_t* bn2, uint32_t* ret)
{
	char i;
	bool overflow = 0;
	uint64_t temp;

	for (i = 0; i < BIGNUMBER_SIZE_8WORD + 2; i++){
		temp = (uint64_t)bn1[i] + (uint64_t)bn2[i] + (uint64_t)overflow;
		ret[i] = temp;
		overflow = temp >> 32;
	}
}
/**
 * @brief two unsigned int strings run minus.
 * 
 * @param bn1 the first string of unsigned int, lenght <= BIGNUMBER_SIZE_8WORD + 2
 * @param bn2 the second string of unsigned int, lenght <= BIGNUMBER_SIZE_8WORD + 2
 * @param ret the result string of unsigned int, lenght <= BIGNUMBER_SIZE_8WORD + 2, store the result
 * 
 * @result ret
 */
void U32Minus(uint32_t* bn1, uint32_t* bn2, uint32_t* ret)
{
	char i;
	bool borrow = 0, newborrow;

	for (i = 0; i < BIGNUMBER_SIZE_8WORD + 2; i++){
		newborrow = (uint64_t)bn1[i] < ((uint64_t)bn2[i] + borrow);
		ret[i] = bn1[i] - bn2[i] - borrow;
		borrow = newborrow;
	}
}
/**
 *
 * @brief Montogery multyply algorithm; Calls: Big8wMultNum, U32CMP, U32Add, U32Minus; Called By: Big8wMultMod
 * 		  montmult(bn1, bn2, p) = bn1 * bn2 * (2^(32*8)) mod p
 * 
 * @param bn1 the first big number 
 * @param bn2 the second big number
 * @param p big number, the module number
 * @param fill unsigned int, precomputed number
 * @param ret pointer of a string of unsigned int, store the result
 * 
 * @result ret
 * 
 */
void Big8wMontMult(big8w bn1, big8w bn2, big8w p, uint32_t fill, uint32_t* ret)
{
	int i;
	int numindex = BIGNUMBER_SIZE_8WORD - 1;
	uint32_t temp[BIGNUMBER_SIZE_8WORD + 1 + 1]; // big8w mult uint32_t  and add overflow
	uint32_t elem, time;

	memset(temp, 0x00, sizeof(uint32_t) * (BIGNUMBER_SIZE_8WORD + 1 + 1));
	memset(ret, 0x00, sizeof(uint32_t) * (BIGNUMBER_SIZE_8WORD + 1 + 1));

	while (bn2.word[numindex] == 0)
		numindex--;
	if (numindex < 0)
		return;

	for (numindex = 0; numindex < BIGNUMBER_SIZE_8WORD; numindex++){
		elem = bn2.word[numindex];
		Big8wMultNum(bn1, elem, temp);
		U32Add(temp, ret, ret);
		Big8wMultNum(p, fill*ret[0], temp);
		U32Add(temp, ret, ret);

		for (i = 0; i < BIGNUMBER_SIZE_8WORD + 1; i++) 
		{ // ret.word[0] = 0,  (ret >> 32) == ret * (2^(-32)) mod p
			ret[i] = ret[i + 1];
		}
		ret[i] = 0;
	}

	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++)
		temp[i] = p.word[i];
	temp[i] = 0, temp[i + 1] = 0;

	if (U32CMP(ret, temp)) 
		U32Minus(ret, temp, ret);
}
/**
 * 
 * @brief  return (bn1*bn2 mod p); call twice Montogery multiply algorithm. Only suitable for sm9, the input big8w p is q or N.
 * montmult(A, B, p) = A * B * (2^(-32*8)) mod p, which can be computed fastly.
 * so multmod(A, B, p) can be computed by:
 * ret = montmult(A, B, p) = A*B*(2^(-256)) mod p
 * ret = montmult(ret, 2^(256*2), p) = ret * 2^(256*2) * (2^(-256)) mod p    (computed fastly)
 *     = A * B * (2^(-256)) * (2^(256*2)) * (2^(-256)) mod p = A * B mod p   (verify the algorithm)
 * N_2k = (2^(256*2)) mod curve.N; q_2k = (2^(256*2)) mod curve.q
 * fill = (2^32 - ((p.word[0])^(-1) mod (2^32))).
 * fill is precalculated, module number p could be prime number curve.q or curve.N 
 * more details see the theory of Montogery multiply algorithm
 * 
 * @param bn1 the first big number
 * @param bn2 the second big number
 * @param p big number, the module number.
 * 
 * @return ret, big number, ret = bn1 * bn2 mod p.
 * 
 */
big8w Big8wMultMod(big8w bn1, big8w bn2, big8w p)
{
	bool flag; // to decide use N_2k or q_2k
	char i;
	uint32_t res[BIGNUMBER_SIZE_8WORD + 1 + 1];
	uint32_t fill;
	big8w ret;

	memset(ret.word, 0x00, BIG8W_BYTESIZE);

	if (Big8wEqual(&p, &curve.q)){
		fill = qlow_reverse; 
		flag = 1;
	}
	else {
		fill = Nlow_reverse;
		flag = 0;
	}

	if (Big8wIsZero(&bn1) || Big8wIsZero(&bn2))
		return ret;

	Big8wMontMult(bn1, bn2, p, fill, res);
	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++) 
		ret.word[i] = res[i];
	
	if (flag) 
		Big8wMontMult(ret, q_2k, p, fill, res);
	else 
		Big8wMontMult(ret, N_2k, p, fill, res);
	
	for (i = 0; i < BIGNUMBER_SIZE_8WORD; i++)
		ret.word[i] = res[i];

	return ret;
}
