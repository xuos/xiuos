/* ------------------------------------------------------------------
This file is part of bzip2/libbzip2, a program and library for
lossless, block-sorting data compression.

bzip2/libbzip2 version 1.0.4 of 20 December 2006
Copyright (C) 1996-2006 Julian Seward <jseward@bzip.org>

Please read the WARNING, DISCLAIMER and PATENTS sections in the
README file.

This program is released under the terms of the license contained
in the file LICENSE.
------------------------------------------------------------------ */

#include <xiuos.h>

#if defined(FS_VFS) && defined(TOOL_SHELL)

#include "bzlib_private.h"

#define WEIGHTOF(zz0)                      ((zz0) & 0xffffff00)
#define DEPTHOF(zz1)                       ((zz1) & 0x000000ff)
#define MYMAX(zz2,zz3)                     ((zz2) > (zz3) ? (zz2) : (zz3))

#define ADDWEIGHTS(zw1,zw2)                                                                    \
	                                       (WEIGHTOF(zw1)+WEIGHTOF(zw2)) |                     \
	                                       (1 + MYMAX(DEPTHOF(zw1),DEPTHOF(zw2)))

#define UPHEAP(z)                          {                                                   \
	                                          int32_t zz, tmp;                                 \
	                                          zz = z;                                          \
	                                          tmp = HEAP[zz];                                  \
	                                          while (WEIGHT[tmp] < WEIGHT[HEAP[zz >> 1]]) {    \
		                                         HEAP[zz] = HEAP[zz >> 1];                     \
		                                         zz >>= 1;                                     \
	                                          }                                                \
	                                          HEAP[zz] = tmp;                                  \
                                            }

#if BZIP2_SPEED >= 1

#define DOWNHEAP1(heap, weight, Heap)       {                                                  \
	                                           int32_t zz, yy, tmp;                            \
	                                           zz = 1;                                         \
	                                           tmp = heap[zz];                                 \
	                                           while (1) {                                     \
		                                          yy = zz << 1;                                \
		                                          if (yy > nHeap)                              \
			                                         break;                                    \
		                                          if (yy < nHeap                               \
		                                          && weight[heap[yy+1]] < weight[heap[yy]])    \
			                                         yy++;                                     \
		                                          if (weight[tmp] < weight[heap[yy]])          \
			                                         break;                                    \
		                                          heap[zz] = heap[yy];                         \
		                                          zz = yy;                                     \
	                                           }                                               \
	                                           heap[zz] = tmp;                                 \
                                             }

#else

static void DOWNHEAP1(int32_t *heap, int32_t *weight, int32_t nHeap)
{
	int32_t zz, yy, tmp;
	zz = 1;
	tmp = heap[zz];
	while (1) {
		yy = zz << 1;
		if (yy > nHeap)
			break;
		if (yy < nHeap
		 && weight[heap[yy + 1]] < weight[heap[yy]])
			yy++;
		if (weight[tmp] < weight[heap[yy]])
			break;
		heap[zz] = heap[yy];
		zz = yy;
	}
	heap[zz] = tmp;
}

#endif

void BZ2HbMakeCodeLengths(EState *s,
		uint8_t *len,
		int32_t *freq,
		int32_t alphaSize,
		int32_t maxLen)
{
	int32_t nNodes, nHeap, n1, n2, i, j, k;
	Bool  tooLong;

#define HEAP                       (s->BZ2_hbMakeCodeLengths__heap)
#define WEIGHT                     (s->BZ2_hbMakeCodeLengths__weight)
#define PARENT                     (s->BZ2_hbMakeCodeLengths__parent)

	for (i = 0; i < alphaSize; i++)
		WEIGHT[i+1] = (freq[i] == 0 ? 1 : freq[i]) << 8;

	while (1) {
		nNodes = alphaSize;
		nHeap = 0;

		HEAP[0] = 0;
		WEIGHT[0] = 0;
		PARENT[0] = -2;

		for (i = 1; i <= alphaSize; i++) {
			PARENT[i] = -1;
			nHeap++;
			HEAP[nHeap] = i;
			UPHEAP(nHeap);
		}

		ASSERTH(nHeap < (BZ_MAX_ALPHA_SIZE+2), 2001);

		while (nHeap > 1) {
			n1 = HEAP[1]; HEAP[1] = HEAP[nHeap]; nHeap--; DOWNHEAP1(HEAP, WEIGHT, nHeap);
			n2 = HEAP[1]; HEAP[1] = HEAP[nHeap]; nHeap--; DOWNHEAP1(HEAP, WEIGHT, nHeap);
			nNodes++;
			PARENT[n1] = PARENT[n2] = nNodes;
			WEIGHT[nNodes] = ADDWEIGHTS(WEIGHT[n1], WEIGHT[n2]);
			PARENT[nNodes] = -1;
			nHeap++;
			HEAP[nHeap] = nNodes;
			UPHEAP(nHeap);
		}

		ASSERTH(nNodes < (BZ_MAX_ALPHA_SIZE * 2), 2002);

		tooLong = FALSE;
		for (i = 1; i <= alphaSize; i++) {
			j = 0;
			k = i;
			while (PARENT[k] >= 0) {
				k = PARENT[k];
				j++;
			}
			len[i-1] = j;
			if (j > maxLen)
				tooLong = TRUE;
		}

		if (!tooLong)
			break;

		for (i = 1; i <= alphaSize; i++) {
			j = WEIGHT[i] >> 8;
			j = 1 + (j / 2);
			WEIGHT[i] = j << 8;
		}
	}
#undef HEAP
#undef WEIGHT
#undef PARENT
}

void BZ2HbAssignCodes(int32_t *code,
		uint8_t *length,
		int32_t minLen,
		int32_t maxLen,
		int32_t alphaSize)
{
	int32_t n, vec, i;

	vec = 0;
	for (n = minLen; n <= maxLen; n++) {
		for (i = 0; i < alphaSize; i++) {
			if (length[i] == n) {
				code[i] = vec;
				vec++;
			}
		}
		vec <<= 1;
	}
}

#endif
