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
* @file tcp_echo_socket_demo.c
* @brief One UDP demo based on LwIP
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-29
*/

#include <xiuos.h>
#include "udp_echo.h"
#include <connect_ethernet.h>

static void NetStackTaskCreate(void* param);

extern void TcpIpInit(void);

extern char* send_msg;

int UdpEchoSocketDemo(int argc, char *argv[])
{	
  if (argc == 2)
  {
    send_msg = argv[1];
  }

  ETH_BSP_Config();

  int32 thr_id = KTaskCreate(
                        (const char*    )"NetStackTaskCreate",
                        NetStackTaskCreate,
                        (void*          )NULL,
                        (uint16_t       )512,  
                        15);

  if(thr_id >= 0)
    StartupKTask(thr_id);  
  else{
    KPrintf("NetStackTaskCreate create failed !\n");
    return -1;  
  }
}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
UdpEchoSocketDemo, UdpEchoSocketDemo,  tcp_echo_socket function );
#endif

static void NetStackTaskCreate(void* param)
{
  TcpIpInit();

  UdpEchoInit();
}




