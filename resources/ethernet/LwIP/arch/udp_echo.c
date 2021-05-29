/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

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
* @file tcpecho.c
* @brief Add UDP client function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-05-29
*/

#include "udp_echo.h"

#include "lwip/opt.h"

#if LWIP_SOCKET
#include <lwip/sockets.h>

#include "lwip/sys.h"
#include "lwip/api.h"
#include <xs_ktask.h>

#define RECV_DATA         (1024)

char* send_msg = "\n\nThis one is UDP pkg. Congratulations on you.\n\n";

static void UdpEchoThreadServer(void *arg)
{
  KPrintf("UdpEchoThreadServer start.\n");

  int sock = -1;
  char *recv_data;
  struct sockaddr_in udp_addr,seraddr;
  int recv_data_len;
  socklen_t addrlen;
  
  while(1)
  {
    recv_data = (char *)malloc(RECV_DATA);
    if (recv_data == NULL)
    {
        KPrintf("No memory\n");
        goto __exit;
    }
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
      KPrintf("Socket error\n");
      goto __exit;
    }
    
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(LOCAL_PORT_SERVER);
    memset(&(udp_addr.sin_zero), 0, sizeof(udp_addr.sin_zero));
    
    if (bind(sock, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr)) == -1)
    {
      KPrintf("Unable to bind\n");
      goto __exit;
    }

    KPrintf("UDP bind sucess, start to receive.\n");
    KPrintf("\n\nLocal Port:%d\n\n", LOCAL_PORT_SERVER);

    while(1)
    {
      recv_data_len=recvfrom(sock,recv_data,
                             RECV_DATA,0,
                             (struct sockaddr*)&seraddr,
                             &addrlen);
                             
      KPrintf("Receive from : %s\n",inet_ntoa(seraddr.sin_addr));
                             
      KPrintf("Recevce data : %s\n\n",recv_data);
                             
      sendto(sock,recv_data,
             recv_data_len,0,
             (struct sockaddr*)&seraddr,
             addrlen);
    }
    
__exit:
    if (sock >= 0) closesocket(sock);
    if (recv_data) free(recv_data);
  }
}

static void UdpEchoThreadClient(void *arg)
{
  KPrintf("UdpEchoThreadClient start.\n");
  MdelayKTask(5000);
  int sock_udp_send_once = -1;
  sock_udp_send_once = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_udp_send_once < 0)
  {
    KPrintf("Socket error\n");
    goto __exit;
  }
  
  struct sockaddr_in udp_sock;
  udp_sock.sin_family = AF_INET;
  udp_sock.sin_port = htons(TARGET_PORT_CLIENT);
  udp_sock.sin_addr.s_addr = PP_HTONL(LWIP_MAKEU32(IP_ADDR0_SERVER,IP_ADDR1_SERVER,IP_ADDR2_SERVER,IP_ADDR3_SERVER));
  memset(&(udp_sock.sin_zero), 0, sizeof(udp_sock.sin_zero));
  
  if (connect(sock_udp_send_once, (struct sockaddr *)&udp_sock, sizeof(struct sockaddr)))
  {
    KPrintf("Unable to connect\n");
    goto __exit;
  }

  KPrintf("UDP connect sucess, start to send.\n");
  KPrintf("\n\nTarget Port:%d\n\n", udp_sock.sin_port);

  sendto(sock_udp_send_once,send_msg,
     strlen(send_msg),0,
     (struct sockaddr*)&udp_sock,
     sizeof(struct sockaddr));

  KPrintf("Send UDP msg: %s ", send_msg);

  while (1)
  {
    KPrintf("Lwip client is running.\n");
    MdelayKTask(3000);
  }

__exit:
  if (sock_udp_send_once >= 0) closesocket(sock_udp_send_once);
  
  return;
}

void
UdpEchoInit(void)
{
  #ifdef SET_AS_SERVER
    sys_thread_new("UdpEchoThreadServer", UdpEchoThreadServer, NULL, 2048, 4);
  #else
    sys_thread_new("UdpEchoThreadClient", UdpEchoThreadClient, NULL, 2048, 4);
  #endif
}

#endif /* LWIP_NETCONN */
