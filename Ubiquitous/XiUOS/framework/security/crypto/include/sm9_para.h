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
* @file sm9_para.h
* @brief initialize paramters of SM9
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

 #ifndef SM9_PARA_H
 #define SM9_PARA_H

 #include <join.h>

//extern char *device_id;
//extern char *platform_id;

extern G1point P1;
extern G2point P2;
//extern G2point sign_publickey, encrypt_secretkey;
//extern G1point sign_secretkey, encrypt_publickey;

extern const uint32_t sm9_q[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_N[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P1_x[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P1_y[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P2_x_high[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P2_x_low[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P2_y_high[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_P2_y_low[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_1[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_2[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_3[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_4[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_5[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_6[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_7[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_8[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_9[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_10[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc1_11[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_2[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_3[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_4[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_5[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_6[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_7[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_8[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_9[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_10[BIGNUMBER_SIZE_8WORD];
extern const uint32_t fc2_11[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_qnr[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_q_2k[BIGNUMBER_SIZE_8WORD];
extern const uint32_t sm9_N_2k[BIGNUMBER_SIZE_8WORD];
#endif