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
* @file ecc.c
* @brief arithmetic in ecc
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/
#include <ecc.h>

/**
 * @brief Print the point(x, y)
 * 
 * @param point pointer of a point in group G1.
 * 
 * @return null
 */
void G1pointPrint(G1point *point)
{
	Big8wPrint(&point->x);
	Big8wPrint(&point->y);
}
/**
 * @brief judge whether the point in group G1
 * 
 * @param point a point(x, y)
 * 
 * @return true if point in group G1; else false
 * 
 */
bool PointInG1(G1point point)
{
	big8w y_power2;
	big8w temp;

	y_power2 = Big8wMultMod(point.y, point.y, curve.q); // y^2 mod curve.q

	temp = Big8wMultMod(point.x, point.x, curve.q);
	temp = Big8wMultMod(temp, point.x, curve.q); // x^3

	temp = Big8wAddMod(temp, curve.b, curve.q); // x^3 + b

	return Big8wEqual(&y_power2, &temp);
}
/**
 * 
 * @brief compute the sum of two points in group G1; set infinite point O as (0, 0), tell if exist O before points add.
 *        Calls: big8wIszero, Big8wEqual, Big8wAddMod, Big8wMinusMod, Big8wReverse
 *        Called By: G1pointMult
 * 
 * @param point1 the first point in group G1
 * @param point2 the second point in group G1
 * 
 * @return a point in group
 * 
 */
G1point G1pointAdd(G1point point1, G1point point2)
{
	G1point ret;
	big8w lambda, temp;

	// infinite point
	if (Big8wIsZero(&point1.x) && Big8wIsZero(&point1.y))
		return point2;
	else if (Big8wIsZero(&point1.x) && Big8wIsZero(&point1.y))
		return point1;

	if (Big8wEqual(&point1.x, &point2.x)) {

		if (!Big8wEqual(&point1.y, &point2.y)){ // x1=x2, y1 != y2(y1 = -y2), ret = O (0, 0)
			memset(ret.x.word, 0x00, BIG8W_BYTESIZE);
			memset(ret.y.word, 0x00, BIG8W_BYTESIZE);
			return ret;
		}

		temp = Big8wAddMod(point1.y, point1.y, curve.q); // 2*y1
		temp = Big8wReverse(temp, curve.q); // 1/2*y1
		temp = Big8wMultMod(point1.x, temp, curve.q); // x1*(1/2*y1)
		temp = Big8wMultMod(point1.x, temp, curve.q); // x1*x1*(1/2*y1)
		lambda = Big8wAddMod(temp, Big8wAddMod(temp, temp, curve.q), curve.q); // 3*x1*x1*(1/2*y1)
	} 
	else {
		temp = Big8wMinusMod(point1.x, point2.x, curve.q);
		temp = Big8wReverse(temp, curve.q); // 1/(x2 - x1)

		lambda = Big8wMinusMod(point1.y, point2.y, curve.q); // y2 - y1
		lambda = Big8wMultMod(temp, lambda, curve.q);
	}

	ret.x = Big8wMultMod(lambda, lambda, curve.q); // k*k
	temp = Big8wAddMod(point1.x, point2.x, curve.q); // x1 + x2
	ret.x = Big8wMinusMod(ret.x, temp, curve.q); // x3 = lambda*lambda - x1 - x2

	ret.y = Big8wMinusMod(point1.x, ret.x, curve.q); // y3 = lambda*(x1 - x3) - y1
	ret.y = Big8wMultMod(lambda, ret.y, curve.q);
	ret.y = Big8wMinusMod(ret.y, point1.y, curve.q);

	return ret;
}
/**
 * 
 * @brief mult point; scan bits of bignum
 * 
 * @param bignum big number, bignum > 0
 * @param point point in group G1
 * 
 * @return a point in group G1
 * 
 */
// could optimized by scan the segment of continuous 1.
G1point G1pointMult(big8w bignum, G1point point)
{
	bool flag = 0;
	int i = BIGNUMBER_SIZE_8WORD - 1;
	int index = Big8wHighestbit(&bignum);
	uint32_t elem;
	G1point ret = point;
	G1point temp = point;

	while (bignum.word[i] == 0) 
		i--;
	elem = bignum.word[i];
	
	index--;
	while (index>=0) { 
		flag = (elem >> (index--)) & 1;
		ret = G1pointAdd(temp, temp); 
		if (flag)
			ret = G1pointAdd(ret, point); 
		temp = ret;
	}

	i--; 
	for (; i>=0; i--) {
		elem = bignum.word[i];
		index = 31;
		while (index>=0) {
			flag = (elem >> (index--)) & 1;
			ret = G1pointAdd(temp, temp); 
			if (flag)
				ret = G1pointAdd(ret, point); 
			temp = ret;
		}
	}

	return ret;
}