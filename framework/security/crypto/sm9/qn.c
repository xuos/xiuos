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
* @file qn.c
* @brief arithmetic in extention field, and arithmetic in group G2, frobenius and LastPower in BiLinearPairing
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/
#include <qn.h>

big8w t; // sm9 ecc parameter
big8w qnr; // (-1/2) mod curve.q
big8w frobenius_constant_1[12];
big8w frobenius_constant_2[12];
/**
 * @brief print the point in group G2
 * 
 * @param point a pointer of point in group G2
 * 
 * @return null
 * 
 */
void G2pointPrint(G2point* point) 
{
	Big8wPrint(&point->x.high);
	Big8wPrint(&point->x.low);
	Big8wPrint(&point->y.high);
	Big8wPrint(&point->y.low);
}
/**
 * @brief print a number in Fq12
 * 
 * @param number a pointer of number in Fq12
 * 
 * @return null
 * 
 */
void Q12Print(q12* number)
{
	Big8wPrint(&number->high.high.high);
	Big8wPrint(&number->high.high.low);
	Big8wPrint(&number->high.low.high);
	Big8wPrint(&number->high.low.low);

	Big8wPrint(&number->mid.high.high);
	Big8wPrint(&number->mid.high.low);
	Big8wPrint(&number->mid.low.high);
	Big8wPrint(&number->mid.low.low);

	Big8wPrint(&number->low.high.high);
	Big8wPrint(&number->low.high.low);
	Big8wPrint(&number->low.low.high);
	Big8wPrint(&number->low.low.low);
}
/**
 * @brief convert q12 to big_12big
 * 
 * @param num pointer of number in Fq12
 * @param ret pointer of big_12big, store the result
 * 
 * @result ret
 * 
 */
void Q12To12big(q12* num, big_12big* ret)
{
	ret->word[0] = num->high.high.high;
	ret->word[1] = num->high.high.low;
	ret->word[2] = num->high.low.high;
	ret->word[3] = num->high.low.low;

	ret->word[4] = num->mid.high.high;
	ret->word[5] = num->mid.high.low;
	ret->word[6] = num->mid.low.high;
	ret->word[7] = num->mid.low.low;

	ret->word[8] = num->low.high.high;
	ret->word[9] = num->low.high.low;
	ret->word[10] = num->low.low.high;
	ret->word[11] = num->low.low.low;
}
/**
 * @brief set a number in Fq12 to 0
 * 
 * @param num pointer of a number in Fq12
 * 
 * @resulr num
 * 
 */
void Q2Zero(q2* num)
{
	memset(num->high.word, 0x00, BIG8W_BYTESIZE);
	memset(num->low.word, 0x00, BIG8W_BYTESIZE);
}
/**
 * @brief set a number in Fq4 to 0
 * 
 * @param num number in Fq4
 * 
 * @result num
 * 
 */
void Q4Zero(q4* num)
{
	Q2Zero(&num->high);
	Q2Zero(&num->low);
}
/**
 * @brief set a number in Fq12 to 0
 * 
 * @param num pointer of a number in Fq12
 * 
 * @result num
 * 
 */
void Q12Zero(q12* num)
{
	Q4Zero(&num->high);
	Q4Zero(&num->mid);
	Q4Zero(&num->low);
}
/**
 * @brief return num1 == num2
 * 
 * @param num1 pointer of the first number in Fq2
 * @param num2 pointer of the second number in Fq2
 * 
 * @return true if num1 == num2; else false
 * 
 */
bool Q2Equal(q2* num1, q2* num2)
{
	return Big8wEqual(&num1->high, &num2->high) 
		&& Big8wEqual(&num1->low, &num2->low);
}
/**
 * @brief return (num1 + num2)
 * 
 * @param num1 the first number in Fq2
 * @param num2 the second number in Fq2
 * 
 * @return ret, a number in Fq2, ret = num1 + num2.
 */
q2 Q2Add(q2 num1, q2 num2)
{
	q2 ret;

	ret.high = Big8wAddMod(num1.high, num2.high, curve.q);
	ret.low = Big8wAddMod(num1.low, num2.low, curve.q);

	return ret;
}
/**
 * @brief return (num1 - num2)
 * 
 * @param num1 the first number in Fq2
 * @param num2 the second number in Fq2
 * 
 * @return ret, a number in Fq2, ret = num1 - num2.
 */
q2 Q2Minus(q2 num1, q2 num2)
{
	q2 ret;

	ret.high = Big8wMinusMod(num1.high, num2.high, curve.q);
	ret.low = Big8wMinusMod(num1.low, num2.low, curve.q);

	return ret;
}
/**
 * @brief 
 * 		set num1 = (a, b), num2 = (c, d), num1*num2 = (a*d + b*c, b*d - 2*a*c)......(high, low)
 * 		Toom-Cook: (a*d + b*c) = (a + b)(c + d) - a*c - b*d
 * 		Calls: Big8wMultMod, Big8wAddMod, Big8wMinusMod
 * 		Called By: G2pointAdd, Q4Mult, Q2Reverse and more
 * @param num1 the first number in Fq2
 * @param num2 the second number in Fq2
 * 
 * @return ret, a number in Fq2, ret = num1 * num2
 * 
 */
q2 Q2Mult(q2 num1, q2 num2) 
{ // num1 = (a1, a0), num2 = (b1, b0).....(high, low)
	q2 ret;
	big8w a0b0, a1b1;
	// Toom-Cook
	a0b0 = Big8wMultMod(num1.low, num2.low, curve.q);
	a1b1 = Big8wMultMod(num1.high, num2.high, curve.q);

	ret.high = Big8wMultMod(
		Big8wAddMod(num1.high, num1.low, curve.q),
		Big8wAddMod(num2.high, num2.low, curve.q), 
		curve.q);

	ret.high = Big8wMinusMod(
		ret.high,
		Big8wAddMod(a0b0, a1b1, curve.q),
		curve.q);

	ret.low = Big8wMinusMod(
		a0b0,
		Big8wAddMod(a1b1, a1b1, curve.q),
		curve.q);

	return ret;
}
/**
 * 
 * @brief num1 = (a, b), num2 = (c, d), num1*num2*u = (b*d - 2*a*c, -2*(a*d +b*c)). compare Q2Mult
 *              
 * @param num1 the first number in Fq2
 * @param num2 the second number in Fq2
 * 
 * @return ret, a number in Fq2; ret = a * b * u
 * 
 */
q2 Q2MultFlag(q2 num1, q2 num2)
{
	q2 ret = Q2Mult(num1, num2);
	big8w temp = ret.high;

	ret.high = ret.low;
	ret.low = Big8wMinusMod(
		curve.q,
		Big8wAddMod(temp, temp, curve.q), curve.q);

	return ret;
}
/**
 * 
 * @brief num = (a, b), num^(-1) = ((-a)/(b*b +2*a*a), b/(b*b, 2*a*a))
 * 
 * @param num number in Fq2
 * 
 * @return ret, number in Fq2, ret = num^(-1).
 * 
 */
q2 Q2Reverse(q2 num)
{
	big8w temp;
	q2 ret;

	temp = Big8wMultMod(num.high, num.high, curve.q); // a*a
	temp = Big8wAddMod(temp, temp, curve.q); // 2*a*a
	temp = Big8wAddMod(temp, 
		Big8wMultMod(num.low, num.low, curve.q), curve.q); // (b*b +2*a*a)
	temp = Big8wReverse(temp, curve.q); // (1/(b*b + 2*a*a))

	ret.high = Big8wMinusMod(curve.q, num.high, curve.q); // (-a)
	ret.high = Big8wMultMod(ret.high, temp, curve.q); 
	ret.low = Big8wMultMod(num.low, temp, curve.q);

	return ret;
}
/**
 * @brief return (-num);(-num.high, -num.low) mod curve.q = (curve.q - num.high, curve.q - num.low)
 * 
 * @param num number in Fq2
 * 
 * @return ret, a number in Fq2
 * 
 */
q2 Q2Negate(q2 num)
{
	num.high = Big8wMinusMod(curve.q, num.high, curve.q);
	num.low = Big8wMinusMod(curve.q, num.low, curve.q);

	return num;
}
/**
 * @brief 
 * 
 * @param num number in Fq2
 * 
 * @return num
 * 
 */
q2 Q2Txx(q2 num)
{
	big8w temp;

	temp = num.high;
	num.high = num.low;
	num.low = Big8wMinusMod(curve.q, temp, curve.q);
	num.low = Big8wAddMod(num.low, num.low, curve.q);

	return num;
}
/**
 * @brief add of two points in group G2
 * 
 * @param point1 the first point in group G2
 * @param point2 the second point in group G2
 * 
 * @return ret, a point in group G2
 * 
 */
G2point G2PointAdd(G2point point1, G2point point2)
{
	q2 temp, lambda;
	G2point ret;

	memset(&temp, 0x00, BIG8W_BYTESIZE * 2);

	// temp = zero
	// infinite point judge
	if (Q2Equal(&point1.x, &temp) && Q2Equal(&point1.y, &temp))
		return point2;
	else if (Q2Equal(&point2.x, &temp) && Q2Equal(&point2.y, &temp))
		return point1;

	if (Big8wEqual(&point1.x.high, &point2.x.high)
		&& Big8wEqual(&point1.x.low, &point2.x.low)) { // x1 = x2

		if (!Big8wEqual(&point1.y.high, &point2.y.high)
			|| !Big8wEqual(&point1.y.low, &point2.y.low)) // y1 != y2 (y1 = -y2)
		{
			memset(&ret, 0x00, BIG8W_BYTESIZE * 4);
			return ret; // ret = O
		}

		temp = Q2Mult(point1.x, point1.x);

		lambda = Q2Add(temp, temp);
		lambda = Q2Add(lambda, temp);
		lambda = Q2Mult(lambda, Q2Reverse(Q2Add(point1.y, point1.y)));
	}

	else {
		lambda = Q2Mult(
			Q2Minus(point1.y, point2.y),
			Q2Reverse(Q2Minus(point1.x, point2.x)));
	}

	ret.x = Q2Mult(lambda, lambda);
	ret.x = Q2Minus(ret.x, point1.x);
	ret.x = Q2Minus(ret.x, point2.x);

	ret.y = Q2Minus(point1.x, ret.x);
	ret.y = Q2Mult(lambda, ret.y);
	ret.y = Q2Minus(ret.y, point1.y);

	return ret;
}
/**
 * @brief mult point in group G2. (num * point)
 * 
 * @param num big number, num > 0
 * @param point point in group G2
 * 
 * @return ret, a point in group G2
 * 
 */
G2point G2PointMult(big8w num, G2point point)
{
	bool flag = 0;
	int i = BIGNUMBER_SIZE_8WORD - 1;
	int index = Big8wHighestbit(&num);
	uint32_t elem;
	G2point ret = point, temp = point;

	while (num.word[i] == 0)
		i--;
	elem = num.word[i];

	index--;
	while (index >= 0) {
		flag = (elem >> (index--)) & 1;
		ret = G2PointAdd(temp, temp); 
		if (flag) 
			ret = G2PointAdd(ret, point);
		temp = ret;
	}

	i--; 
	for (; i >= 0; i--) {
		elem = num.word[i];
		index = 31;

		while (index >= 0) {
			flag = (elem >> (index--)) & 1;
			ret = G2PointAdd(temp, temp);
			if (flag)
				ret = G2PointAdd(ret, point);
			temp = ret;
		}
	}

	return ret;
}
/**
 * @brief return (num1 + num2)
 * 
 * @param num1 the first number in Fq4
 * @param num2 the second number in Fq4
 * 
 * @return ret, a number in Fq4, ret = (num1 + num2)
 * 
 */
q4 Q4Add(q4 num1, q4 num2)
{
	q4 ret;

	ret.high = Q2Add(num1.high, num2.high);
	ret.low = Q2Add(num1.low, num2.low);

	return ret;
}
/**
 * @brief return (num1 - num2)
 * 
 * @param num1 the first number in Fq4
 * @param num2 the second number in Fq4
 * 
 * @return ret, a number in Fq4, ret = (num1 - num2)
 * 
 */
q4 Q4Minus(q4 num1, q4 num2)
{
	q4 ret;

	ret.high = Q2Minus(num1.high, num2.high);
	ret.low = Q2Minus(num1.low, num2.low);

	return ret;
}
/**
 * @brief num1 * num2; num1, num2 are numbers in Fq4, the way to compute is similar to Q2Mult
 * 			q4 num = (num1, num2), similar to Q2Mult, except num1 = (a1, a0), a0, a1 is big8w
 * 
 * @param num1 the first number in Fq4
 * @param num2 the second number in Fq4
 * 
 * @return ret, a number in Fq4
 */
q4 Q4Mult(q4 num1, q4 num2)
{// num1 = (a1, a0), num2 = (b1, b0). left high value
	q4 ret;
	q2 a0b0, a1b1;

	// Toom-Cook
	a1b1 = Q2Mult(num1.high, num2.high);
	a0b0 = Q2Mult(num1.low, num2.low);

	ret.high = Q2Mult(
		Q2Add(num1.high, num1.low),
		Q2Add(num2.high, num2.low));

	ret.high = Q2Minus(
		ret.high,
		Q2Add(a0b0, a1b1));

	ret.low.high = Big8wAddMod(a0b0.high, a1b1.low, curve.q);
	ret.low.low = Big8wMinusMod(
		a0b0.low,
		Big8wAddMod(a1b1.high, a1b1.high, curve.q),
		curve.q);

	return ret;
}
/**
 * @brief return num1 * num2 * u; num1, num2 are numbers in Fq4
 * 
 * @param num1 the first number in Fq4
 * @param num2 the second number in Fq4
 * 
 * @return ret, a number in Fq4
 * 
 */
q4 Q4MultFlag(q4 num1, q4 num2)
{
	q4 ret;

	ret.high = Q2Add(
		Q2Mult(num1.low, num2.low),
		Q2MultFlag(num1.high, num2.high));
	ret.low = Q2Add(
		Q2MultFlag(num1.high, num2.low),
		Q2MultFlag(num1.low, num2.high));

	return ret;
}
/**
 * @brief 
 * 
 * @param num number in Fq4
 * 
 * @return ret, a number in Fq4
 * 
 */
q4 Q4Txx(q4 num)
{
	q2 temp;

	temp = num.high;
	num.high = num.low;
	num.low = Q2Txx(temp);
	
	return num;
}
/**
 * @brief return (-num), similar to Q2Negate
 * 
 * @param num a number in Fq4
 * 
 * @return num, a number in Fq4
 * 
 */
q4 Q4Negate(q4 num)
{
	num.high = Q2Negate(num.high);
	num.low = Q2Negate(num.low);

	return num;
}
/**
 * @brief return num^(-1)
 * 
 * @param num number in Fq4
 * 
 * @return num, number in Fq4
 * 
 */
q4 Q4Reverse(q4 num)
{
	q2 t1, t2;
	
	t1 = Q2Mult(num.low, num.low);
	t2 = Q2Mult(num.high, num.high);
	t2 = Q2Txx(t2);
	t1 = Q2Minus(t1, t2);
	t1 = Q2Reverse(t1);
	num.low = Q2Mult(num.low, t1);
	t1 = Q2Negate(t1);
	num.high = Q2Mult(num.high, t1);

	return num;
}
/**
 * @brief return (num1 * num2);
 * 
 * @param num1 the first number in Fq12
 * @param num2 the second number in Fq12
 * 
 * @return a number in Fq12
 * 
 */
q12 Q12MultMod(q12 num1, q12 num2)
{
	q12 ret;

	ret.high = Q4Add(
		Q4Mult(num1.high, num2.low),
		Q4Mult(num1.mid, num2.mid));

	ret.high = Q4Add(
		ret.high, 
		Q4Mult(num1.low, num2.high));

	ret.mid = Q4Add(
		Q4Mult(num1.low, num2.mid),
		Q4Mult(num1.mid, num2.low));

	ret.mid = Q4Add(
		ret.mid, 
		Q4MultFlag(num1.high, num2.high));

	ret.low = Q4Add(
		Q4MultFlag(num1.high, num2.mid),
		Q4MultFlag(num1.mid, num2.high));

	ret.low = Q4Add(
		ret.low, 
		Q4Mult(num1.low, num2.low));

	return ret;
}
/**
 * @brief return (num1 - num2)
 * 
 * @param num1 the first number in Fq12
 * @param num2 the second number in Fq12
 * 
 * @return ret, a number in Fq12.
 * 
 */
q12 Q12MinusMod(q12 num1, q12 num2)
{
	q12 ret;

	ret.high = Q4Minus(num1.high, num2.high);
	ret.mid = Q4Minus(num1.mid, num2.mid);
	ret.low = Q4Minus(num1.low, num2.low);

	return ret;
}
/**
 * @brief return (num1 + num2)
 * 
 * @param num1 the first number in Fq12
 * @param num2 the second number in Fq12
 * 
 * @return ret, a number in Fq12
 * 
 */
q12 Q12AddMod(q12 num1, q12 num2)
{
	q12 ret;

	ret.high = Q4Add(num1.high, num2.high);
	ret.mid = Q4Add(num1.mid, num2.mid);
	ret.low = Q4Add(num1.low, num2.low);

	return ret;
}
/**
 * @brief return num^(-1)
 * 
 * @param num number in Fq12
 * 
 * @return ret, number in Fq12
 * 
 */
q12 Q12Reverse(q12 num)
{
	q12 ret;
	q4 temp1, temp2;

	ret.low = Q4Mult(num.low, num.low);
	ret.mid = Q4Mult(num.mid, num.high);
	ret.mid = Q4Txx(ret.mid);
	ret.low = Q4Minus(ret.low, ret.mid);

	ret.high = Q4Mult(num.high, num.high);
	ret.high = Q4Txx(ret.high);
	ret.mid = Q4Mult(num.low, num.mid);
	ret.mid = Q4Minus(ret.high, ret.mid);

	ret.high = Q4Mult(num.mid, num.mid);
	temp1 = Q4Mult(num.low, num.high);
	ret.high = Q4Minus(ret.high, temp1);

	temp1 = Q4Mult(num.mid, ret.high);
	temp1 = Q4Txx(temp1);

	temp2 = Q4Mult(num.low, ret.low);
	temp1 = Q4Add(temp1, temp2);

	temp2 = Q4Mult(num.high, ret.mid);
	temp2 = Q4Txx(temp2);
	temp1 = Q4Add(temp1, temp2);

	temp1 = Q4Reverse(temp1);

	ret.low = Q4Mult(ret.low, temp1);
	ret.mid = Q4Mult(ret.mid, temp1);
	ret.high = Q4Mult(ret.high, temp1);

	return ret;
}
/**
 * @brief compute g^r. scan every bit of r
 * 
 * @param g a number in Fq12
 * @param r power, a big number, r >= 0
 * 
 * @return ret, a number in Fq12
 * 
 */
q12 Q12PowerMod(q12 g, big8w r)
{
	bool flag;
	int bitindex = Big8wHighestbit(&r);
	int i = BIGNUMBER_SIZE_8WORD - 1;
	uint32_t elem;
	q12 ret = g, temp = g;
	q12 one;

	memset(&one, 0x00, BIG8W_BYTESIZE * 12);
	one.low.low.low.word[0] = 1;

	while (i && r.word[i] == 0)
		i--;
	if (i < 0)
		return one;
	elem = r.word[i];

	bitindex--;
	while (bitindex >= 0) {
		flag = (elem >> (bitindex--)) & 1;
		ret = Q12MultMod(temp, temp); 
		if (flag)
			ret = Q12MultMod(ret, g);
		temp = ret;
	}

	i--; 
	for (; i >= 0; i--) {
		elem = r.word[i];
		bitindex = 31;
		while (bitindex >= 0) {
			flag = (elem >> (bitindex--)) & 1;
			ret = Q12MultMod(temp, temp); 
			if (flag)
				ret = Q12MultMod(ret, g); 
			temp = ret;
		}
	}

	return ret;
}
/**
 * @brief compute f^(curve.q^(flag)); f = (f11, f10, ... f0), f^(curve.q^(flag)) = (f11*c11, f10*c10, ... f0*c0)
 * 
 * @param f pointer of a number in Fq12
 * @param flag 1, 2, 6;
 * 
 * @return null
 */
void Q12Frobenius(q12* f, uint8_t flag)
{
	if (flag == 1) {
		f->high.high.high = Big8wMultMod(f->high.high.high, frobenius_constant_1[11], curve.q);
		f->high.high.low = Big8wMultMod(f->high.high.low, frobenius_constant_1[10], curve.q);
		f->high.low.high = Big8wMultMod(f->high.low.high, frobenius_constant_1[9], curve.q);
		f->high.low.low = Big8wMultMod(f->high.low.low, frobenius_constant_1[8], curve.q);

		f->mid.high.high = Big8wMultMod(f->mid.high.high, frobenius_constant_1[7], curve.q);
		f->mid.high.low = Big8wMultMod(f->mid.high.low, frobenius_constant_1[6], curve.q);
		f->mid.low.high = Big8wMultMod(f->mid.low.high, frobenius_constant_1[5], curve.q);
		f->mid.low.low = Big8wMultMod(f->mid.low.low, frobenius_constant_1[4], curve.q);

		f->low.high.high = Big8wMultMod(f->low.high.high, frobenius_constant_1[3], curve.q);
		f->low.high.low = Big8wMultMod(f->low.high.low, frobenius_constant_1[2], curve.q);
		f->low.low.high = Big8wMultMod(f->low.low.high, frobenius_constant_1[1], curve.q);
		f->low.low.low = Big8wMultMod(f->low.low.low, frobenius_constant_1[0], curve.q);
	}

	else if (flag == 2) {

		f->high.high.high = Big8wMultMod(f->high.high.high, frobenius_constant_2[11], curve.q);
		f->high.high.low = Big8wMultMod(f->high.high.low, frobenius_constant_2[10], curve.q);
		f->high.low.high = Big8wMultMod(f->high.low.high, frobenius_constant_2[9], curve.q);
		f->high.low.low = Big8wMultMod(f->high.low.low, frobenius_constant_2[8], curve.q);

		f->mid.high.high = Big8wMultMod(f->mid.high.high, frobenius_constant_2[7], curve.q);
		f->mid.high.low = Big8wMultMod(f->mid.high.low, frobenius_constant_2[6], curve.q);
		f->mid.low.high = Big8wMultMod(f->mid.low.high, frobenius_constant_2[5], curve.q);
		f->mid.low.low = Big8wMultMod(f->mid.low.low, frobenius_constant_2[4], curve.q);

		f->low.high.high = Big8wMultMod(f->low.high.high, frobenius_constant_2[3], curve.q);
		f->low.high.low = Big8wMultMod(f->low.high.low, frobenius_constant_2[2], curve.q);
		f->low.low.high = Big8wMultMod(f->low.low.high, frobenius_constant_2[1], curve.q);
		f->low.low.low = Big8wMultMod(f->low.low.low, frobenius_constant_2[0], curve.q);

	}

	else if (flag == 6) {
		f->high.high.high = Big8wMinusMod(curve.q, f->high.high.high, curve.q);
		f->high.high.low = Big8wMinusMod(curve.q, f->high.high.low, curve.q);

		f->mid.low.high = Big8wMinusMod(curve.q, f->mid.low.high, curve.q);
		f->mid.low.low = Big8wMinusMod(curve.q, f->mid.low.low, curve.q);

		f->low.high.high = Big8wMinusMod(curve.q, f->low.high.high, curve.q);
		f->low.high.low = Big8wMinusMod(curve.q, f->low.high.low, curve.q);
	}
}
/**
 * 
 * @brief compute Frobenius(Q)
 *		  Q = Q(x, y), point in group G2;first convert x, y to q12 number:(x*beta^(-1/2), y*beta(-1/3))
 *        then run Q12Frobenius, last convert back to G2point(ret.x*(beta^(1/2)), ret.y*(beta^(1/3)))
 * 
 * @param Q point in group G2
 * @param Q1 pointer of a point in group G2, store the result
 * @param flag power of frobenius
 * 
 * @result Q1
 * 
 */
void G2pointFrobenius(G2point Q, G2point* Q1, uint8_t flag)
{
	q12 temp, ret;
	
	Q12Zero(&temp);
	Q12Zero(&ret);

	ret.low.low = Q.x;
	temp.mid.high.high = qnr;
	ret = Q12MultMod(ret, temp);
	Q12Frobenius(&ret, flag);
	memset(temp.mid.high.high.word, 0x00, BIG8W_BYTESIZE);
	temp.high.low.low.word[0] = 1;
	ret = Q12MultMod(ret, temp);
	Q1->x = ret.low.low;

	Q12Zero(&ret);
	Q12Zero(&temp);

	ret.low.low = Q.y;
	temp.low.high.high = qnr;
	ret = Q12MultMod(ret, temp);
	Q12Frobenius(&ret, flag);
	memset(temp.low.high.high.word, 0x00, BIG8W_BYTESIZE);
	temp.low.high.low.word[0] = 1;
	ret = Q12MultMod(ret, temp);
	Q1->y = ret.low.low;
}
/**
 * @brief line function defined in SM9 and a Q12MultMod
 * 
 * @param P a point in group G1
 * @param T a pointer of a point in group G2
 * @param Q a point in group G2
 * @param doubleflag if (*T) == Q
 * @param f a pointer of a number in Fq12, (*f) = (*f) * g(T, Q, P), g(T, Q, P) is the line function defined in SM9
 * 
 * @return null
 * 
 */
void Line(G1point P, G2point* T, G2point Q, bool doubleflag, q12* f)
{
	q2 lambda, temp;
	q12 ret;

	Q12Zero(&ret);
	Q2Zero(&temp);

	if (doubleflag) {

		if (Q2Equal(&Q.x, &temp) && Q2Equal(&Q.y, &temp)) // Q = T = O
			return;

		lambda = Q2Mult(Q.x, Q.x);
		lambda = Q2Add(lambda, Q2Add(lambda, lambda));
		lambda = Q2Mult(
			lambda,
			Q2Reverse(Q2Add(Q.y, Q.y)));
	}
	else {
		if (Q2Equal(&T->x, &Q.x)){
			// T = -Q => T = T + Q = O
			Q2Zero(&T->x), Q2Zero(&T->y);
			// g(T, Q, P) = xP - xQ
			temp.high = qnr;
			temp = Q2Mult(temp, Q.x);
			temp = Q2Negate(temp);
			ret.mid.high = temp;
			ret.low.low.low = P.x;

			*f = Q12MultMod(*f, ret);
		}
		else if (Q2Equal(&T->x, &temp) && Q2Equal(&T->y, &temp)){ // T = O
			// g(U, V, O) = 1 => f * g(U, V, O) = f; *T = T + Q = Q
			*T = Q;
			return;
		}
		else if (Q2Equal(&Q.x, &temp) && Q2Equal(&Q.y, &temp)){ // Q = O
			// g(U, V, O) = 1 => f * g(U, V, O) = f; *T = T + Q = T
			return;
		}
		
		lambda = Q2Mult(
			Q2Minus(T->y, Q.y),
			Q2Reverse(Q2Minus(T->x, Q.x)));
	}

	temp.high = qnr;

	ret.high.high = lambda;
	ret.high.high.high = Big8wMultMod(ret.high.high.high, P.x, curve.q);
	ret.high.high.low = Big8wMultMod(ret.high.high.low, P.x, curve.q);
	ret.high.high = Q2Mult(ret.high.high, temp);

	ret.low.high = Q2Minus(
		T->y,
		Q2Mult(lambda, T->x));
	ret.low.high = Q2Mult(temp, ret.low.high);

	ret.low.low.low = Big8wMinusMod(curve.q, P.y, curve.q);

	*T = G2PointAdd(Q, *T);
	*f = Q12MultMod(ret, *f);
}
/**
 * @brief compute (f^(curve.q^(12) - 1)), called by BiLinearParing function(in topfunc.h).
 * 
 * @param f a pointer of a number in Fq12
 * 
 * @return null, result stored in *f.
 * 
 */
// t = 0x60000000 0058F98A
void LastPower(q12* f)
{
	q12 m, g, s, temp;

	m = Q12Reverse(*f);
	Q12Frobenius(f, 6);
	// f = f^(q^6 - 1)
	*f = Q12MultMod(*f, m);
	m = *f;
	Q12Frobenius(f, 2);
	*f = Q12MultMod(*f, m);  // f = f^(q^6 - 1)(q^2+1)
	s = *f;  

	// hard part
	s = *f;
	m = s, temp = m;
	Q12Frobenius(&temp, 1);
	s = temp;
	Q12Frobenius(&temp, 1);
	s = Q12MultMod(s, temp);
	Q12Frobenius(&temp, 1);
	s = Q12MultMod(s, temp);

	temp = Q12PowerMod(m, t);
	temp = Q12PowerMod(temp, t);

	g = Q12MultMod(temp, temp);
	temp = Q12MultMod(g, g);
	temp = Q12MultMod(temp, g);
	Q12Frobenius(&temp, 2);

	s = Q12MultMod(s, temp);

	temp = m;
	Q12Frobenius(&temp, 6);
	temp = Q12MultMod(temp, temp);
	s = Q12MultMod(s, temp);
	g = temp;

	temp = Q12MultMod(g, g);
	g = Q12MultMod(temp, g);
	g = Q12PowerMod(g, t);
	
	temp = Q12MultMod(g, g);
	s = Q12MultMod(s, Q12MultMod(temp, g));
	Q12Frobenius(&temp, 1);
	s = Q12MultMod(s, temp);

	g = Q12PowerMod(g, t);
	temp = Q12MultMod(g, g);
	m = Q12MultMod(g, temp);
	s = Q12MultMod(s, m);
	s = Q12MultMod(s, temp); 
	g = m;
	Q12Frobenius(&m, 1);
	s = Q12MultMod(s, m);

	g = Q12MultMod(g, g);
	g = Q12PowerMod(g, t);

	s = Q12MultMod(s, g);
	Q12Frobenius(&g, 1);
	s = Q12MultMod(s, g);

	*f = s;;
}