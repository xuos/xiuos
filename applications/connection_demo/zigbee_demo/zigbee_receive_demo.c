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
* @file:    zigbee_receive_demo.c
* @brief:   using zigbee to receive message
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/25
*
*/
#include <xs_adapter_zigbee.h>   
#include <string.h>
#include <xs_klist.h>
#include <xs_adapter_manager.h>
#include <user_api.h>
#include <string.h>
static int re_sem;

static int buff_sem;


/*Critical zone protection function for*/
void ZigbeeWait(char *rev_buffer)
{
    while(1){  
        if (strlen(rev_buffer)>1){
            UserSemaphoreAbandon(re_sem);
            break;
        }
    }

}


/* receive message from another zigbee device*/    
void ZigbeeReceiveDemo(int argc, char *argv[])
{
    adapter_t padapter = ZigbeeAdapterFind("zigbee");
    if (NONE == padapter){
        KPrintf("adapter find failed!\n");
        return;
    }
    /*Open adapter*/
    if (0 != padapter->done.NetAiitOpen(padapter)){
        KPrintf("adapter open failed!\n");
        return;
    }


    char rev_buffer[NAME_NUM_MAX];    
    /* Initialize semaphore */
    re_sem = UserSemaphoreCreate(0);
    /* receive buffer from serial port */
    padapter->done.NetAiitReceive(padapter,rev_buffer,strlen(rev_buffer),10000,false,NULL);
    ZigbeeWait(rev_buffer);
    UserSemaphoreObtain(re_sem,-1);


    printf("\n");
    for (int i=0;i<strlen(rev_buffer);i++)
    {     
        if(rev_buffer[i] != 0Xff)
            printf("%c",rev_buffer[i]);    
              
    }
    printf("\n");
    
    
    
}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
ZigbeeReceiveDemo, ZigbeeReceiveDemo,  zigbee receive function );
#endif




