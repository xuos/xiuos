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

/**
* @file:    banner.c
* @brief:   system banner
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/15
*
*/

#include <xs_banner.h>

/**
 * This function will show the version of XiUOS 
 */
void ShowBanner(void)
{
	KPrintf("*********************************************************************************************\n");
	KPrintf("AAAAAAA       AAAAAAA             IIIIIIII    IIIIIIII     IIIIIIIII        TTTTTTTTTTTTTTT  \n");
	KPrintf("A:::::A       A:::::A             I::::::I    I::::::I   II:::::::::II    TT:::::::::::::::T \n");
	KPrintf("A:::::A       A:::::A       iiii  I::::::I    I::::::I II:::::::::::::II T:::::TTTTTT::::::T \n");
	KPrintf("A::::::A     A::::::A      i::::i II:::::I     I:::::III:::::::III:::::::IT:::::T     TTTTTTT\n");
	KPrintf("AAA:::::A   A:::::AAA       iiii   I:::::I     I:::::I I::::::I   I::::::IT:::::T            \n");
	KPrintf("   A:::::A A:::::A                 I:::::I     I:::::I I:::::I     I:::::IT:::::T            \n");
	KPrintf("    A:::::A:::::A         iiiiiii  I:::::I     I:::::I I:::::I     I:::::I T::::TTTT         \n");
	KPrintf("     A:::::::::A          i:::::i  I:::::I     I:::::I I:::::I     I:::::I  TT::::::TTTTT    \n");
	KPrintf("     A:::::::::A           i::::i  I:::::I     I:::::I I:::::I     I:::::I    TTT::::::::TT  \n");
	KPrintf("    A:::::A:::::A          i::::i  I:::::I     I:::::I I:::::I     I:::::I       TTTTTT::::T \n");
	KPrintf("   A:::::A A:::::A         i::::i  I:::::I     I:::::I I:::::I     I:::::I            T:::::T\n");
	KPrintf("AAA:::::A   A:::::AAA      i::::i  I::::::I   I::::::I I::::::I   I::::::I            T:::::T\n");
	KPrintf("A::::::A     A::::::A      i::::i  I:::::::III:::::::I I:::::::III:::::::ITTTTTTT     T:::::T\n");
	KPrintf("A:::::A       A:::::A      i::::i  II:::::::::::::II   II:::::::::::::II T::::::TTTTTT:::::T \n");
	KPrintf("A:::::A       A:::::A     i::::::i   II:::::::::II       II:::::::::II   T:::::::::::::::TT  \n");
	KPrintf("AAAAAAA       AAAAAAA     iiiiiiii     IIIIIIIII           IIIIIIIII      TTTTTTTTTTTTTTT    \n");                                                
	KPrintf("*********************************************************************************************\n");
	KPrintf("*********************-----X Industrial Ubiquitous Operating System-----**********************\n");
	KPrintf("***************************2021 Copyright AIIT Ubiquitous-OS Team****************************\n");
	KPrintf("*********************************************************************************************\n");
}