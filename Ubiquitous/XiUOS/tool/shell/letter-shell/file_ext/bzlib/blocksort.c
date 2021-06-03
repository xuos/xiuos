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

#include <string.h>
#include <stdint.h>
#include "bzlib_private.h"

#define MSWAP(zz1, zz2)      {                         \
	                            int32_t zztmp = zz1;   \
	                            zz1 = zz2;             \
	                            zz2 = zztmp;           \
                             }

static void mvswap(uint32_t* ptr, int32_t zzp1, int32_t zzp2, int32_t zzn)
{
	while (zzn > 0) {
		MSWAP(ptr[zzp1], ptr[zzp2]);
		zzp1++;
		zzp2++;
		zzn--;
	}
}

static inline int32_t mmin(int32_t a, int32_t b)
{
	return (a < b) ? a : b;
}

static inline void FallbackSimpleSort(uint32_t* fmap,
		uint32_t* eclass,
		int32_t   lo,
		int32_t   hi)
{
	int32_t i, j, tmp;
	uint32_t ec_tmp;

	if (lo == hi) return;

	if (hi - lo > 3) {
		for (i = hi-4; i >= lo; i--) {
			tmp = fmap[i];
			ec_tmp = eclass[tmp];
			for (j = i+4; j <= hi && ec_tmp > eclass[fmap[j]]; j += 4)
				fmap[j-4] = fmap[j];
			fmap[j-4] = tmp;
		}
	}

	for (i = hi-1; i >= lo; i--) {
		tmp = fmap[i];
		ec_tmp = eclass[tmp];
		for (j = i+1; j <= hi && ec_tmp > eclass[fmap[j]]; j++)
			fmap[j-1] = fmap[j];
		fmap[j-1] = tmp;
	}
}


#define FPUSH(lz,hz)       {                     \
                             stackLo[sp] = lz;   \
	                         stackHi[sp] = hz;   \
	                         sp++;               \
                           }

#define FPOP(lz,hz)        {                     \
	                         sp--;               \
	                         lz = stackLo[sp];   \
	                         hz = stackHi[sp];   \
                           }

#define FALLBACK_QSORT_SMALL_THRESH          10
#define FALLBACK_QSORT_STACK_SIZE            100

static void FallbackQSort3(uint32_t* fmap,
		uint32_t* eclass,
		int32_t   loSt,
		int32_t   hiSt)
{
	int32_t sp;
	uint32_t r;
	int32_t stackLo[FALLBACK_QSORT_STACK_SIZE];
	int32_t stackHi[FALLBACK_QSORT_STACK_SIZE];

	r = 0;

	sp = 0;
	FPUSH(loSt, hiSt);

	while (sp > 0) {
		int32_t unLo, unHi, ltLo, gtHi, n, m;
		int32_t lo, hi;
		uint32_t med;
		uint32_t r3;

		ASSERTH(sp < FALLBACK_QSORT_STACK_SIZE - 1, 1004);

		FPOP(lo, hi);
		if (hi - lo < FALLBACK_QSORT_SMALL_THRESH) {
			FallbackSimpleSort(fmap, eclass, lo, hi);
			continue;
		}

		r = ((r * 7621) + 1) % 32768;
		r3 = r % 3;
		if (r3 == 0)
			med = eclass[fmap[lo]];
		else if (r3 == 1)
			med = eclass[fmap[(lo+hi)>>1]];
		else
			med = eclass[fmap[hi]];

		unLo = ltLo = lo;
		unHi = gtHi = hi;

		while (1) {
			while (1) {
				if (unLo > unHi) break;
				n = (int32_t)eclass[fmap[unLo]] - (int32_t)med;
				if (n == 0) {
					MSWAP(fmap[unLo], fmap[ltLo]);
					ltLo++;
					unLo++;
					continue;
				}
				if (n > 0) break;
				unLo++;
			}
			while (1) {
				if (unLo > unHi) break;
				n = (int32_t)eclass[fmap[unHi]] - (int32_t)med;
				if (n == 0) {
					MSWAP(fmap[unHi], fmap[gtHi]);
					gtHi--; unHi--;
					continue;
				}
				if (n < 0) break;
				unHi--;
			}
			if (unLo > unHi) break;
			MSWAP(fmap[unLo], fmap[unHi]); unLo++; unHi--;
		}

		ASSERTD(unHi == unLo-1, "fallbackQSort3(2)");

		if (gtHi < ltLo) continue;

		n = mmin(ltLo-lo, unLo-ltLo); mvswap(fmap, lo, unLo-n, n);
		m = mmin(hi-gtHi, gtHi-unHi); mvswap(fmap, unLo, hi-m+1, m);

		n = lo + unLo - ltLo - 1;
		m = hi - (gtHi - unHi) + 1;

		if (n - lo > hi - m) {
			FPUSH(lo, n);
			FPUSH(m, hi);
		} else {
			FPUSH(m, hi);
			FPUSH(lo, n);
		}
	}
}

#undef FPUSH
#undef FPOP
#undef FALLBACK_QSORT_SMALL_THRESH
#undef FALLBACK_QSORT_STACK_SIZE


#define       SET_BH(zz)         bhtab[(zz) >> 5] |= (1 << ((zz) & 31))
#define     CLEAR_BH(zz)         bhtab[(zz) >> 5] &= ~(1 << ((zz) & 31))
#define     ISSET_BH(zz)         (bhtab[(zz) >> 5] & (1 << ((zz) & 31)))
#define      WORD_BH(zz)         bhtab[(zz) >> 5]
#define UNALIGNED_BH(zz)         ((zz) & 0x01f)

static void FallbackSort(EState* state)
{
	int32_t ftab[257];
	int32_t ftabCopy[256];
	int32_t H, i, j, k, l, r, cc, cc1;
	int32_t nNotDone;
	int32_t nBhtab;

	uint32_t *const fmap    = state->arr1;
	uint32_t *const eclass  = state->arr2;
#define ECLASS8                   ((uint8_t*)eclass)
	uint32_t *const bhtab   = state->ftab;
	const int32_t   nblock  = state->nblock;


	for (i = 0; i < 257;    i++) ftab[i] = 0;
	for (i = 0; i < nblock; i++) ftab[ECLASS8[i]]++;
	for (i = 0; i < 256;    i++) ftabCopy[i] = ftab[i];

	j = ftab[0];
	for (i = 1; i < 257;    i++) {
		j += ftab[i];
		ftab[i] = j;
	}

	for (i = 0; i < nblock; i++) {
		j = ECLASS8[i];
		k = ftab[j] - 1;
		ftab[j] = k;
		fmap[k] = i;
	}

	nBhtab = 2 + ((uint32_t)nblock / 32);
	for (i = 0; i < nBhtab; i++) bhtab[i] = 0;
	for (i = 0; i < 256; i++) SET_BH(ftab[i]);

	for (i = 0; i < 32; i++) {
		SET_BH(nblock + 2*i);
		CLEAR_BH(nblock + 2*i + 1);
	}

	H = 1;
	while (1) {
		j = 0;
		for (i = 0; i < nblock; i++) {
			if (ISSET_BH(i))
				j = i;
			k = fmap[i] - H;
			if (k < 0)
				k += nblock;
			eclass[k] = j;
		}

		nNotDone = 0;
		r = -1;
		while (1) {

			k = r + 1;
			while (ISSET_BH(k) && UNALIGNED_BH(k))
				k++;
			if (ISSET_BH(k)) {
				while (WORD_BH(k) == 0xffffffff) k += 32;
				while (ISSET_BH(k)) k++;
			}
			l = k - 1;
			if (l >= nblock)
				break;
			while (!ISSET_BH(k) && UNALIGNED_BH(k))
				k++;
			if (!ISSET_BH(k)) {
				while (WORD_BH(k) == 0x00000000) k += 32;
				while (!ISSET_BH(k)) k++;
			}
			r = k - 1;
			if (r >= nblock)
				break;

			if (r > l) {
				nNotDone += (r - l + 1);
                FallbackQSort3(fmap, eclass, l, r);

				cc = -1;
				for (i = l; i <= r; i++) {
					cc1 = eclass[fmap[i]];
					if (cc != cc1) {
						SET_BH(i);
						cc = cc1;
					}
				}
			}
		}

		H *= 2;
		if (H > nblock || nNotDone == 0)
			break;
	}

	j = 0;
	for (i = 0; i < nblock; i++) {
		while (ftabCopy[j] == 0)
			j++;
		ftabCopy[j]--;
		ECLASS8[fmap[i]] = (uint8_t)j;
	}
	ASSERTH(j < 256, 1005);
#undef ECLASS8
}

#undef       SET_BH
#undef     CLEAR_BH
#undef     ISSET_BH
#undef      WORD_BH
#undef UNALIGNED_BH

static int MainGtU(EState* state,
		uint32_t  i1,
		uint32_t  i2)
{
	int32_t  k;
	uint8_t  c1, c2;
	uint16_t s1, s2;

	uint8_t  *const block    = state->block;
	uint16_t *const quadrant = state->quadrant;
	const int32_t   nblock   = state->nblock;


#if BZIP2_SPEED >= 1

#define TIMES_8(code)                              \
	                     code; code; code; code;   \
	                     code; code; code; code;
#define TIMES_12(code)                             \
	                     code; code; code; code;   \
	                     code; code; code; code;   \
	                     code; code; code; code;

#else

#define TIMES_8(code)    {                         \
	                       int nn = 8;             \
	                       do {                    \
		                        code;              \
	                        } while (--nn);        \
	                     }
#define TIMES_12(code)   {                         \
	                       int nn = 12;            \
	                       do {                    \
		                        code;              \
	                       } while (--nn);         \
                         }

#endif

	ASSERTD(i1 != i2, "mainGtU");
	TIMES_12(
		c1 = block[i1]; c2 = block[i2];
		if (c1 != c2) return (c1 > c2);
		i1++; i2++;
	)

	k = nblock + 8;

	do {
		TIMES_8(
			c1 = block[i1]; c2 = block[i2];
			if (c1 != c2) return (c1 > c2);
			s1 = quadrant[i1]; s2 = quadrant[i2];
			if (s1 != s2) return (s1 > s2);
			i1++; i2++;
		)

		if (i1 >= nblock) i1 -= nblock;
		if (i2 >= nblock) i2 -= nblock;

		state->budget--;
		k -= 8;
	} while (k >= 0);

	return FALSE;
}
#undef TIMES_8
#undef TIMES_12

static
const uint32_t incs[14] = {
	1, 4, 13, 40, 121, 364, 1093, 3280,
	9841, 29524, 88573, 265720,
	797161, 2391484
};

static void MainSimpleSort(EState* state,
		int32_t   lo,
		int32_t   hi,
		int32_t   d)
{
	uint32_t *const ptr = state->ptr;

	int hp = 0;
	{
		int bigN = hi - lo;
		if (bigN <= 0)
			return;
		while (incs[hp] <= bigN)
			hp++;
		hp--;
	}

	for (; hp >= 0; hp--) {
		int32_t i;
		unsigned h;

		h = incs[hp];
		i = lo + h;
		while (1) {
			unsigned j;
			unsigned v;

			if (i > hi) break;
			v = ptr[i];
			j = i;
			while (MainGtU(state, ptr[j-h]+d, v+d)) {
				ptr[j] = ptr[j-h];
				j = j - h;
				if (j <= (lo + h - 1)) break;
			}
			ptr[j] = v;
			i++;

#if BZIP2_SPEED >= 3
			if (i > hi) break;
			v = ptr[i];
			j = i;
			while (MainGtU(state, ptr[j-h]+d, v+d)) {
				ptr[j] = ptr[j-h];
				j = j - h;
				if (j <= (lo + h - 1)) break;
			}
			ptr[j] = v;
			i++;
			if (i > hi) break;
			v = ptr[i];
			j = i;
			while (MainGtU(state, ptr[j-h]+d, v+d)) {
				ptr[j] = ptr[j-h];
				j = j - h;
				if (j <= (lo + h - 1)) break;
			}
			ptr[j] = v;
			i++;
#endif
			if (state->budget < 0) return;
		}
	}
}


static inline uint8_t mmed3(uint8_t a, uint8_t b, uint8_t c)
{
	uint8_t t;
	if (a > b) {
		t = a;
		a = b;
		b = t;
	}
	if (b > c) {
		b = c;
		if (a > b)
			b = a;
	}
	return b;
}

#define MPUSH(lz,hz,dz)     {                      \
	                          stackLo[sp] = lz;    \
	                          stackHi[sp] = hz;    \
	                          stackD [sp] = dz;    \
	                          sp++;                \
	                        }

#define MPOP(lz,hz,dz)      {                      \
	                          sp--;                \
	                          lz = stackLo[sp];    \
	                          hz = stackHi[sp];    \
	                          dz = stackD [sp];    \
                            }

#define MNEXTSIZE(az)       (nextHi[az] - nextLo[az])

#define MNEXTSWAP(az,bz)     {                                                              \
	                           int32_t tz;                                                  \
	                           tz = nextLo[az]; nextLo[az] = nextLo[bz]; nextLo[bz] = tz;   \
	                           tz = nextHi[az]; nextHi[az] = nextHi[bz]; nextHi[bz] = tz;   \
	                           tz = nextD [az]; nextD [az] = nextD [bz]; nextD [bz] = tz;   \
	                          }

#define MAIN_QSORT_SMALL_THRESH                     20
#define MAIN_QSORT_DEPTH_THRESH                     (BZ_N_RADIX + BZ_N_QSORT)
#define MAIN_QSORT_STACK_SIZE                       100

static void MainQSort3(EState* state,
		int32_t   loSt,
		int32_t   hiSt
		)
{
	enum { dSt = BZ_N_RADIX };
	int32_t unLo, unHi, ltLo, gtHi, n, m, med;
	int32_t sp, lo, hi, d;

	int32_t stackLo[MAIN_QSORT_STACK_SIZE];
	int32_t stackHi[MAIN_QSORT_STACK_SIZE];
	int32_t stackD [MAIN_QSORT_STACK_SIZE];

	int32_t nextLo[3];
	int32_t nextHi[3];
	int32_t nextD [3];

	uint32_t *const ptr   = state->ptr;
	uint8_t  *const block = state->block;

	sp = 0;
	MPUSH(loSt, hiSt, dSt);

	while (sp > 0) {
		ASSERTH(sp < MAIN_QSORT_STACK_SIZE - 2, 1001);

		MPOP(lo, hi, d);
		if (hi - lo < MAIN_QSORT_SMALL_THRESH
		 || d > MAIN_QSORT_DEPTH_THRESH
		) {
            MainSimpleSort(state, lo, hi, d);
			if (state->budget < 0)
				return;
			continue;
		}
		med = (int32_t)	mmed3(block[ptr[lo          ] + d],
		                      block[ptr[hi          ] + d],
		                      block[ptr[(lo+hi) >> 1] + d]);

		unLo = ltLo = lo;
		unHi = gtHi = hi;

		while (1) {
			while (1) {
				if (unLo > unHi)
					break;
				n = ((int32_t)block[ptr[unLo]+d]) - med;
				if (n == 0) {
					MSWAP(ptr[unLo], ptr[ltLo]);
					ltLo++;
					unLo++;
					continue;
				}
				if (n > 0) break;
				unLo++;
			}
			while (1) {
				if (unLo > unHi)
					break;
				n = ((int32_t)block[ptr[unHi]+d]) - med;
				if (n == 0) {
					MSWAP(ptr[unHi], ptr[gtHi]);
					gtHi--;
					unHi--;
					continue;
				}
				if (n < 0) break;
				unHi--;
			}
			if (unLo > unHi)
				break;
			MSWAP(ptr[unLo], ptr[unHi]);
			unLo++;
			unHi--;
		}

		ASSERTD(unHi == unLo-1, "mainQSort3(2)");

		if (gtHi < ltLo) {
			MPUSH(lo, hi, d + 1);
			continue;
		}

		n = mmin(ltLo-lo, unLo-ltLo); mvswap(ptr, lo, unLo-n, n);
		m = mmin(hi-gtHi, gtHi-unHi); mvswap(ptr, unLo, hi-m+1, m);

		n = lo + unLo - ltLo - 1;
		m = hi - (gtHi - unHi) + 1;

		nextLo[0] = lo;  nextHi[0] = n;   nextD[0] = d;
		nextLo[1] = m;   nextHi[1] = hi;  nextD[1] = d;
		nextLo[2] = n+1; nextHi[2] = m-1; nextD[2] = d+1;

		if (MNEXTSIZE(0) < MNEXTSIZE(1)) MNEXTSWAP(0, 1);
		if (MNEXTSIZE(1) < MNEXTSIZE(2)) MNEXTSWAP(1, 2);
		if (MNEXTSIZE(0) < MNEXTSIZE(1)) MNEXTSWAP(0, 1);

		ASSERTD (MNEXTSIZE(0) >= MNEXTSIZE(1), "mainQSort3(8)");
		ASSERTD (MNEXTSIZE(1) >= MNEXTSIZE(2), "mainQSort3(9)");

		MPUSH(nextLo[0], nextHi[0], nextD[0]);
		MPUSH(nextLo[1], nextHi[1], nextD[1]);
		MPUSH(nextLo[2], nextHi[2], nextD[2]);
	}
}

#undef MPUSH
#undef MPOP
#undef MNEXTSIZE
#undef MNEXTSWAP
#undef MAIN_QSORT_SMALL_THRESH
#undef MAIN_QSORT_DEPTH_THRESH
#undef MAIN_QSORT_STACK_SIZE


#define BIGFREQ(b)           (ftab[((b)+1) << 8] - ftab[(b) << 8])
#define SETMASK              (1 << 21)
#define CLEARMASK            (~(SETMASK))

static void MainSort(EState* state)
{
	int32_t  i, j;
	Bool     bigDone[256];
	uint8_t  runningOrder[256];

#define COPYSTART             (state->mainSort__copyStart)
#define COPYEND               (state->mainSort__copyEnd)

	uint32_t *const ptr      = state->ptr;
	uint8_t  *const block    = state->block;
	uint32_t *const ftab     = state->ftab;
	const int32_t   nblock   = state->nblock;
	uint16_t *const quadrant = state->quadrant;

	memset(ftab, 0, 65537 * sizeof(ftab[0]));

	j = block[0] << 8;
	i = nblock - 1;

#if BZIP2_SPEED >= 2
	for (; i >= 3; i -= 4) {
		quadrant[i] = 0;
		j = (j >> 8) | (((unsigned)block[i]) << 8);
		ftab[j]++;
		quadrant[i-1] = 0;
		j = (j >> 8) | (((unsigned)block[i-1]) << 8);
		ftab[j]++;
		quadrant[i-2] = 0;
		j = (j >> 8) | (((unsigned)block[i-2]) << 8);
		ftab[j]++;
		quadrant[i-3] = 0;
		j = (j >> 8) | (((unsigned)block[i-3]) << 8);
		ftab[j]++;
	}
#endif
	for (; i >= 0; i--) {
		quadrant[i] = 0;
		j = (j >> 8) | (((unsigned)block[i]) << 8);
		ftab[j]++;
	}

	for (i = 0; i < BZ_N_OVERSHOOT; i++) {
		block   [nblock+i] = block[i];
		quadrant[nblock+i] = 0;
	}

	j = ftab[0];
	for (i = 1; i <= 65536; i++) {
		j += ftab[i];
		ftab[i] = j;
	}

	{
		unsigned s;
		s = block[0] << 8;
		i = nblock - 1;
#if BZIP2_SPEED >= 2
		for (; i >= 3; i -= 4) {
			s = (s >> 8) | (block[i] << 8);
			j = ftab[s] - 1;
			ftab[s] = j;
			ptr[j] = i;
			s = (s >> 8) | (block[i-1] << 8);
			j = ftab[s] - 1;
			ftab[s] = j;
			ptr[j] = i-1;
			s = (s >> 8) | (block[i-2] << 8);
			j = ftab[s] - 1;
			ftab[s] = j;
			ptr[j] = i-2;
			s = (s >> 8) | (block[i-3] << 8);
			j = ftab[s] - 1;
			ftab[s] = j;
			ptr[j] = i-3;
		}
#endif
		for (; i >= 0; i--) {
			s = (s >> 8) | (block[i] << 8);
			j = ftab[s] - 1;
			ftab[s] = j;
			ptr[j] = i;
		}
	}

	for (i = 0; i <= 255; i++) {
		bigDone     [i] = FALSE;
		runningOrder[i] = i;
	}

	{
		unsigned h = 364;

		do {

			h = (h * 171) >> 9;
			for (i = h; i <= 255; i++) {
				unsigned vv, jh;
				vv = runningOrder[i];
				j = i;
				while (jh = j - h, BIGFREQ(runningOrder[jh]) > BIGFREQ(vv)) {
					runningOrder[j] = runningOrder[jh];
					j = jh;
					if (j < h)
						break;
				}
				runningOrder[j] = vv;
			}
		} while (h != 1);
	}


	for (i = 0; ; i++) {
		unsigned ss;

		ss = runningOrder[i];


		for (j = 0; j <= 255; j++) {
			if (j != ss) {
				unsigned sb;
				sb = (ss << 8) + j;
				if (!(ftab[sb] & SETMASK)) {
					int32_t lo =  ftab[sb] ;
					int32_t hi = (ftab[sb+1] & CLEARMASK) - 1;
					if (hi > lo) {
                        MainQSort3(state, lo, hi );
						if (state->budget < 0) return;
					}
				}
				ftab[sb] |= SETMASK;
			}
		}

		ASSERTH(!bigDone[ss], 1006);

		{
			for (j = 0; j <= 255; j++) {
                COPYSTART[j] =  ftab[(j << 8) + ss]     & CLEARMASK;
                COPYEND  [j] = (ftab[(j << 8) + ss + 1] & CLEARMASK) - 1;
			}
			for (j = ftab[ss << 8] & CLEARMASK; j < COPYSTART[ss]; j++) {
				unsigned c1;
				int32_t k;
				k = ptr[j] - 1;
				if (k < 0)
					k += nblock;
				c1 = block[k];
				if (!bigDone[c1])
					ptr[COPYSTART[c1]++] = k;
			}
			for (j = (ftab[(ss+1) << 8] & CLEARMASK) - 1; j > COPYEND[ss]; j--) {
				unsigned c1;
				int32_t k;
				k = ptr[j]-1;
				if (k < 0)
					k += nblock;
				c1 = block[k];
				if (!bigDone[c1])
					ptr[COPYEND[c1]--] = k;
			}
		}


		ASSERTH((COPYSTART[ss]-1 == COPYEND[ss]) \
		     || (COPYSTART[ss] == 0 && COPYEND[ss] == nblock-1), 1007);

		for (j = 0; j <= 255; j++)
			ftab[(j << 8) + ss] |= SETMASK;

		if (i == 255)
			break;

		bigDone[ss] = TRUE;

		{
			unsigned bbStart = ftab[ss << 8] & CLEARMASK;
			unsigned bbSize  = (ftab[(ss+1) << 8] & CLEARMASK) - bbStart;
			unsigned shifts  = 0;

			while ((bbSize >> shifts) > 65534) shifts++;

			for (j = bbSize-1; j >= 0; j--) {
				unsigned a2update  = ptr[bbStart + j];
				uint16_t qVal      = (uint16_t)(j >> shifts);
				quadrant[a2update] = qVal;
				if (a2update < BZ_N_OVERSHOOT)
					quadrant[a2update + nblock] = qVal;
			}
			ASSERTH(((bbSize-1) >> shifts) <= 65535, 1002);
		}
	}
#undef runningOrder
#undef COPYSTART
#undef COPYEND
}

#undef BIGFREQ
#undef SETMASK
#undef CLEARMASK

int32_t BZ2BlockSort(EState* state)
{
	enum { wfact = 30 };
	unsigned i;
	int32_t origPtr = origPtr;

	if (state->nblock >= 10000) {
		i = state->nblock + BZ_N_OVERSHOOT;
		if (i & 1)
			i++;
		state->quadrant = (uint16_t*) &(state->block[i]);

		state->budget = state->nblock * ((wfact-1) / 3);
        MainSort(state);
		if (state->budget >= 0)
			goto good;
	}
    FallbackSort(state);
 good:

#if BZ_LIGHT_DEBUG
	origPtr = -1;
#endif
	for (i = 0; i < state->nblock; i++) {
		if (state->ptr[i] == 0) {
			origPtr = i;
			break;
		}
	}

	ASSERTH(origPtr != -1, 1003);
	return origPtr;
}
