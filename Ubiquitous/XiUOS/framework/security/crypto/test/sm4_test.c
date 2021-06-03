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
* @file sm4_test.c
* @brief test for SM4
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <sm4.h>
#include <xiuos.h>

void sm4_test_case(){
    unsigned char ukey[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe ,0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
    unsigned char input[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe ,0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
    uint8_t res[16] = {0} ;
	uint8_t plaintext[16] = {1};
	uint8_t ciphertext[16] = {2};
    int olen = 0;
    sms4_key_t key;

    //test case 1
    KPrintf("\n#################### sm4 test ##########################\n");
    KPrintf("\n####sm4 test case1:\n");
    KPrintf("plaintext:  ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x ",input[i]);
    }
    KPrintf("\n");
    KPrintf("key:        ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",ukey[i]);
    }
    KPrintf("\n");

    KPrintf("encryption:\n");
    sms4_set_encrypt_key(&key, ukey);
    Sms4EcbEncryptNoPadding(input,16,res,&olen,&key);
    KPrintf("ciphertext: ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",res[i]);
    }
    KPrintf("\n");
    KPrintf("decryption:\n");
    sms4_set_decrypt_key(&key, ukey);
	KPrintf("round key in sms4_set_decrypt_key:\n");
	for (int i = 0; i < 32; i++){
		KPrintf("rk%d:%08x\n", i, key.rk[i]);
	}
    Sms4EcbDecryptNoPadding(res,16,res,&olen,&key);
    printf("plaintext:  ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",res[i]);
    }
    printf("\n");

    ////test case 2
    KPrintf("\n####sm4 test case2:\n");
    KPrintf("plaintext:  ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",input[i]);
    }
    KPrintf("\n");
    KPrintf("key:        ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",ukey[i]);
    }
    KPrintf("\n");
    KPrintf("encrypt 1000000 times:\n");
    sms4_set_encrypt_key(&key, ukey);
	memcpy(plaintext, input, 16);
    for (int i = 0;i< 1000000; i++){
        Sms4EcbEncryptNoPadding(plaintext,16,ciphertext,&olen,&key);
		memcpy(plaintext, ciphertext, 16);
    }
    KPrintf("ciphertext: ");
    for (int i = 0; i< 16; i++){
        KPrintf("%02x",ciphertext[i]);
    }
    KPrintf("\n");
    KPrintf("\n########################################################\n");
}
