/*
* Copyright (c) 2020 AIIT XUOS Lab
* xiuOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file extmem.h
* @brief support extmem function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef EXTMEM_H
#define EXTMEM_H

#if defined (DATA_IN_ExtSRAM) || defined (DATA_IN_ExtSDRAM)
  void SystemInitExtMemCtl(void); 
#endif

#endif