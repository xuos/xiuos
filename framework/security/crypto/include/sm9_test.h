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
* @file sm9_test.h
* @brief tests of SM9
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#ifndef SM9_TEST_H
#define SM9_TEST_H

 #include <sm9.h>

 void SignAndVerifyTest();
 void SM9KeyExchangeTest();
 void SM9PackDepackTest();
 void SM9EncryptDecryptTest();

#endif