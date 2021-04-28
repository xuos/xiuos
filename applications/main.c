/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

#include <stdio.h>
#include <user_api.h>
#include <string.h>

extern int FrameworkInit();

#ifdef SEPARATE_COMPILE
int main(void)
{
	UserPrintInfo(1);
	FrameworkInit();
	return 0;
}
#else

int main(void)
{
	UserPrintf("Hello, world!\n");
	FrameworkInit();
    return 0;
}
// int cppmain(void);

#endif