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

#include <stdint.h>
#include "bzlib.h"

typedef unsigned char Bool;

#define TRUE                                   ((Bool)1)
#define FALSE                                  ((Bool)0)

#if BZ_LIGHT_DEBUG
static void BzAssertFail(int errcode) NORETURN;
#define ASSERTH(cond, errcode)                 do {                                                                     \
	                                                 if (!(cond))                                                       \
		                                                 BzAssertFail(errcode);                                       \
                                               } while (0)
#else
#define ASSERTH(cond, msg)                     do { } while (0)
#endif

#if BZ_DEBUG
#define ASSERTD(cond, msg)                     do {                                                                     \
                                                     if (!(cond))                                                       \
		                                                 bb_error_msg_and_die("(debug build): internal error %s", msg); \
                                               } while (0)
#else
#define ASSERTD(cond, msg)                     do { } while (0)
#endif

#define BZ_HDR_B                               0x42
#define BZ_HDR_Z                               0x5a
#define BZ_HDR_H                               0x68
#define BZ_HDR_0                               0x30

#define BZ_HDR_BZH0                            0x425a6830


#define BZ_MAX_ALPHA_SIZE                      258
#define BZ_MAX_CODE_LEN                        23

#define BZ_RUNA                                0
#define BZ_RUNB                                1

#define BZ_N_GROUPS                            6
#define BZ_G_SIZE                              50
#define BZ_N_ITERS                             4

#define BZ_MAX_SELECTORS                       (2 + (900000 / BZ_G_SIZE))


extern uint32_t BZ2_crc32Table[256];

#define BZ_INITIALISE_CRC(crcVar)              {                                              \
                                                  crcVar = 0xffffffffL;                       \
                                               }

#define BZ_FINALISE_CRC(crcVar)                {                                              \
                                                  crcVar = ~(crcVar);                         \
                                               }

#define BZ_UPDATE_CRC(crcVar,cha)              {                                              \
                                                  crcVar = (crcVar << 8) ^                    \
                                                  BZ2_crc32Table[(crcVar >> 24) ^             \
                                                  ((uint8_t)cha)];                            \
                                               }


#define BZ_M_IDLE                              1
#define BZ_M_RUNNING                           2
#define BZ_M_FLUSHING                          3
#define BZ_M_FINISHING                         4

#define BZ_S_OUTPUT                            1
#define BZ_S_INPUT                             2

#define BZ_N_RADIX                             2
#define BZ_N_QSORT                             12
#define BZ_N_SHELL                             18
#define BZ_N_OVERSHOOT                         (BZ_N_RADIX + BZ_N_QSORT + BZ_N_SHELL + 2)



typedef struct EState {
    BzStream *strm;

	uint8_t mode;
	uint8_t state;

	uint8_t blockSize100k;

	uint32_t *arr1;
	uint32_t *arr2;
	uint32_t *ftab;

	uint16_t *quadrant;
	int32_t  budget;

	uint32_t *ptr;
	uint8_t  *block;
	uint16_t *mtfv;
	uint8_t  *zbits;

	uint32_t state_in_ch;
	int32_t  state_in_len;

	int32_t  nblock;
	int32_t  nblockMAX;
	uint8_t  *posZ;
	uint8_t  *state_out_pos;

	uint32_t bsBuff;
	int32_t  bsLive;

	uint32_t blockCRC;
	uint32_t combinedCRC;

	int32_t  blockNo;

	int32_t  nMTF;

	int32_t  nInUse;
	Bool     inUse[256] __attribute__((__aligned__(sizeof(long))));
	uint8_t  unseqToSeq[256];

	int32_t  mtfFreq    [BZ_MAX_ALPHA_SIZE];
	uint8_t  selector   [BZ_MAX_SELECTORS];
	uint8_t  selectorMtf[BZ_MAX_SELECTORS];

	uint8_t  len[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];

	int32_t  sendMTFValues__code [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
	int32_t  sendMTFValues__rfreq[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
#if BZIP2_SPEED >= 5
	uint32_t sendMTFValues__len_pack[BZ_MAX_ALPHA_SIZE][4];
#endif
	int32_t  BZ2_hbMakeCodeLengths__heap  [BZ_MAX_ALPHA_SIZE + 2];
	int32_t  BZ2_hbMakeCodeLengths__weight[BZ_MAX_ALPHA_SIZE * 2];
	int32_t  BZ2_hbMakeCodeLengths__parent[BZ_MAX_ALPHA_SIZE * 2];

	int32_t  mainSort__copyStart[256];
	int32_t  mainSort__copyEnd[256];
} EState;

int32_t BZ2BlockSort(EState*);

void BZ2CompressBlock(EState*, int);

void BZ2BsInitWrite(EState*);

void BZ2HbAssignCodes(int32_t*, uint8_t*, int32_t, int32_t, int32_t);

void BZ2HbMakeCodeLengths(EState*, uint8_t*, int32_t*, int32_t, int32_t);

