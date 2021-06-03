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
* @file sm3_test.c
* @brief tests of SM3
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <sm3.h>
#include <xiuos.h>

void sm3_test_case(){
    uint8_t result[SM3_DIGEST_LENGTH] = { 0 };
    //test case 1
    KPrintf("\n#################### sm3 test ##########################\n");
    char *msg = "abc";
    KPrintf("\n####sm3 test case 1:\n");
    KPrintf( "%-15s %s\n", "digest message1:",msg);
    sm3(msg,3,result);
    KPrintf("%-15s ","digest result1: ");
    for ( int i = 0 ; i < SM3_DIGEST_LENGTH ; i++){
        KPrintf("%02x",result[i]);
    }
    KPrintf("\n");
    //test case 2
    KPrintf("\n####sm3 test case 2:\n");
    //msg = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
    char msg1[64] =  { 0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64 ,0x61, 0x62, 0x63, 0x64  };
    KPrintf("digest message2: ");
    for ( int i = 0 ; i < 64 ; i++){
        KPrintf("%02x",msg1[i]);
    }
    KPrintf("\n");
    sm3(msg1, 64,result);
    KPrintf("digest result2:  ");
    for ( int i = 0 ; i < SM3_DIGEST_LENGTH ; i++){
        KPrintf("%02x",result[i]);
    }
    KPrintf("\n");
    KPrintf("\n########################################################\n");
}