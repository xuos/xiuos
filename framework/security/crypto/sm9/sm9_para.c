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
* @file sm9_para.c
* @brief SM9 parameters
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <stdint.h>
#include <ecc.h>
#include <qn.h>

 //extern char *device_id;
 //extern char *platform_id;

 G1point P1;
 G2point P2;
 //G2point sign_publickey, encrypt_secretkey;
 //G1point sign_secretkey, encrypt_publickey;


const uint32_t sm9_q[BIGNUMBER_SIZE_8WORD] =  {
         0xE351457D, 0xE56F9B27, 0x1A7AEEDB, 0x21F2934B,
         0xF58EC745, 0xD603AB4F, 0x02A3A6F1, 0xB6400000,
     };

const uint32_t sm9_N[BIGNUMBER_SIZE_8WORD] =  {
         0xD69ECF25, 0xE56EE19C, 0x18EA8BEE, 0x49F2934B,
         0xF58EC744, 0xD603AB4F, 0x02A3A6F1, 0xB6400000,
     };

const uint32_t sm9_P1_x[BIGNUMBER_SIZE_8WORD] = {
         0x7C66DDDD, 0xE8C4E481, 0x09DC3280, 0xE1E40869,
         0x487D01D6, 0xF5ED0704, 0x62BF718F, 0x93DE051D,
     };

const uint32_t sm9_P1_y[BIGNUMBER_SIZE_8WORD] = {
         0x0A3EA616, 0x0C464CD7, 0xFA602435, 0x1C1C00CB,
         0x5C395BBC, 0x63106512, 0x4F21E607, 0x21FE8DDA,
     };

const uint32_t sm9_P2_x_high[BIGNUMBER_SIZE_8WORD] = {
         0xD8806141, 0x54806C11, 0x0F5E93C4, 0xF1DD2C19,
         0xB441A01F, 0x597B6027, 0x78640C98, 0x85AEF3D0,
     };

const uint32_t sm9_P2_x_low[BIGNUMBER_SIZE_8WORD] = {
         0xAF82D65B, 0xF9B7213B, 0xD19C17AB, 0xEE265948,
         0xD34EC120, 0xD2AAB97F, 0x92130B08, 0x37227552,
     };

const uint32_t sm9_P2_y_high[BIGNUMBER_SIZE_8WORD] = {
         0x84EBEB96, 0x856DC76B, 0xA347C8BD, 0x0736A96F,
         0x2CBEE6ED, 0x66BA0D26, 0x2E845C12, 0x17509B09,
     };

const uint32_t sm9_P2_y_low[BIGNUMBER_SIZE_8WORD] = {
         0xC999A7C7, 0x6215BBA5, 0xA71A0811, 0x47EFBA98,
         0x3D278FF2, 0x5F317015, 0x19BE3DA6, 0xA7CF28D5,
     };

const uint32_t fc1_1[BIGNUMBER_SIZE_8WORD] = {
         0xE351457C, 0xE56F9B27, 0x1A7AEEDB, 0x21F2934B, 
         0xF58EC745, 0xD603AB4F, 0x02A3A6F1, 0xB6400000,
     };

const uint32_t fc1_2[BIGNUMBER_SIZE_8WORD] = {
         0xDA24D011, 0xF5B21FD3, 0x06DC5177, 0x9F9D4118, 
         0xEE0BAF15, 0xF55ACC93, 0xDC0A3F2C, 0x6C648DE5,
     };

const uint32_t fc1_3[BIGNUMBER_SIZE_8WORD] = {
         0x092c756c, 0xefbd7b54, 0x139e9d63, 0x82555233, 
         0x0783182f, 0xe0a8debc, 0x269967c4, 0x49db721a,
     };

const uint32_t fc1_4[BIGNUMBER_SIZE_8WORD] = {
         0x377b698b, 0xa91d8354, 0x0ddd04ed, 0x47c5c86e, 
         0x9c086749, 0x843c6cfa, 0xe5720bdb, 0x3f23ea58,
     };

const uint32_t fc1_5[BIGNUMBER_SIZE_8WORD] = {
         0xabd5dbf2, 0x3c5217d3, 0x0c9de9ee, 0xda2ccadd, 
         0x59865ffb, 0x51c73e55, 0x1d319b16, 0x771c15a7,
     };

const uint32_t fc1_6[BIGNUMBER_SIZE_8WORD] = {
         0x7be65333, 0xd5fc1196, 0x4f8b78f4, 0x78027235, 
         0x02a3a6f2, 0xf3000000, 0x0, 0x0,
     };

const uint32_t fc1_7[BIGNUMBER_SIZE_8WORD] = {
         0x676af24a, 0x0f738991, 0xcaef75e7, 0xa9f02115, 
         0xf2eb2052, 0xe303ab4f, 0x02a3a6f0, 0xb6400000,
     };

const uint32_t fc1_8[BIGNUMBER_SIZE_8WORD] = {
         0x7be65334, 0xd5fc1196, 0x4f8b78f4, 0x78027235,
         0x02a3a6f2, 0xf3000000, 0x0, 0x0,
     };

const uint32_t fc1_9[BIGNUMBER_SIZE_8WORD] = {
         0x676af249, 0x0f738991, 0xcaef75e7, 0xa9f02115,
         0xf2eb2052, 0xe303ab4f, 0x02a3a6f0, 0xb6400000,
     };

const uint32_t fc1_10[BIGNUMBER_SIZE_8WORD] = {
         0xa2a96686, 0x4c949c7f, 0xf8ff4c8a, 0x57d778a9,
         0x520347cc, 0x711e5f99, 0xf6983351, 0x2d40a38c,
     };

const uint32_t fc1_11[BIGNUMBER_SIZE_8WORD] = {
         0x40a7def7, 0x98dafea8, 0x217ba251, 0xca1b1aa1,
         0xa38b7f78, 0x64e54bb6, 0x0c0b73a0, 0x88ff5c73,
     };


const uint32_t fc2_2[BIGNUMBER_SIZE_8WORD] = {
         0xE351457C, 0xE56F9B27, 0x1A7AEEDB, 0x21F2934B,
         0xF58EC745, 0xD603AB4F, 0x02A3A6F1, 0xB6400000,
     };

const uint32_t fc2_3[BIGNUMBER_SIZE_8WORD] = {
         0xE351457C, 0xE56F9B27, 0x1A7AEEDB, 0x21F2934B,
         0xF58EC745, 0xD603AB4F, 0x02A3A6F1, 0xB6400000,
     };

const uint32_t fc2_4[BIGNUMBER_SIZE_8WORD] = {
         0x7BE65334, 0xD5FC1196, 0x4F8B78F4, 0x78027235,
         0x02A3A6F2, 0xF3000000, 0x0, 0x0,
     };

const uint32_t fc2_5[BIGNUMBER_SIZE_8WORD] = {
         0x7BE65334, 0xD5FC1196, 0x4F8B78F4, 0x78027235,
         0x02A3A6F2, 0xF3000000, 0x0, 0x0,
     };

const uint32_t fc2_6[BIGNUMBER_SIZE_8WORD] = {
         0x676AF249, 0x0F738991, 0xCAEF75E7, 0xA9F02115,
         0xF2EB2052, 0xE303AB4F, 0x02A3A6F0, 0xB6400000,
     };

const uint32_t fc2_7[BIGNUMBER_SIZE_8WORD] = {
         0x676AF249, 0x0F738991, 0xCAEF75E7, 0xA9F02115,
         0xF2EB2052, 0xE303AB4F, 0x02A3A6F0, 0xB6400000,
     };

const uint32_t fc2_8[BIGNUMBER_SIZE_8WORD] = {
         0x7BE65333, 0xD5FC1196, 0x4F8B78F4, 0x78027235,
         0x02A3A6F2, 0xF3000000, 0x0, 0x0,
     };

const uint32_t fc2_9[BIGNUMBER_SIZE_8WORD] = {
         0x7BE65333, 0xD5FC1196, 0x4F8B78F4, 0x78027235,
         0x02A3A6F2, 0xF3000000, 0x0, 0x0,
     };

const uint32_t fc2_10[BIGNUMBER_SIZE_8WORD] = {
         0x676AF24A, 0x0F738991, 0xCAEF75E7, 0xA9F02115,
         0xF2EB2052, 0xE303AB4F, 0x02A3A6F0, 0xB6400000,
     };

const uint32_t fc2_11[BIGNUMBER_SIZE_8WORD] = {
         0x676AF24A, 0x0F738991, 0xCAEF75E7, 0xA9F02115,
         0xF2EB2052, 0xE303AB4F, 0x02A3A6F0, 0xB6400000,
     };

const uint32_t sm9_qnr[BIGNUMBER_SIZE_8WORD] = {
         0xf1a8a2be, 0xf2b7cd93, 0x8d3d776d, 0x90f949a5,
         0xfac763a2, 0xeb01d5a7, 0x0151d378, 0x5b200000,
     };

const uint32_t sm9_q_2k[BIGNUMBER_SIZE_8WORD] = {
         0xb417e2d2, 0x27dea312, 0xae1a5d3f, 0x88f8105f,
         0xd6706e7b, 0xe479b522, 0x56f62fbd, 0x2ea795a6,
     };

const uint32_t sm9_N_2k[BIGNUMBER_SIZE_8WORD] = {
         0xcd750c35, 0x7598cd79, 0xbb6daeab, 0xe4a08110,
         0x7d78a1f9, 0xbfee4bae, 0x63695d0e, 0x8894f5d1,
     };

