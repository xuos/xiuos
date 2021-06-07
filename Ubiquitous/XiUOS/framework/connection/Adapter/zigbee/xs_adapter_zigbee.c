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
* @file:    xs_AdapterZigbee.c
* @brief:   zigbee open close function
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/30
*
*/
#include <xs_adapter_manager.h>
#include "xs_adapter_zigbee.h"
#include <user_api.h>
#include <bus_serial.h>
#include <dev_serial.h>
#include <string.h>
#ifdef CONNECTION_COMMUNICATION_ZIGBEE_AIIT
#define SAMPLE_UART_NAME       "/dev/extuart_dev0"
int use_aiit = 1;    
#endif
#ifdef CONNECTION_COMMUNICATION_ZIGBEE_KD233
#define SAMPLE_UART_NAME       "/dev/uart3_dev3"  
int use_aiit = 0; 
#endif
#ifdef CONNECTION_COMMUNICATION_ZIGBEE_STM32
#define SAMPLE_UART_NAME       "/dev/usart3_dev3"  
int use_aiit = 0; 
#endif

static int serial_fd;
static int32_t zigbeereceive;
static int rx_sem;
char zigbee_buffer[NAME_NUM_MAX ]={0}; 

/* initialize srial port to open zigbee*/
int ZigbeeOpen(struct Adapter *padapter)
{
    
    /* Open device in read-write mode */
    serial_fd = open(SAMPLE_UART_NAME,O_RDWR);

    /* set serial config, serial_baud_rate = 115200 */

    struct SerialDataCfg cfg;
    cfg.serial_baud_rate = BAUD_RATE_115200;
    cfg.serial_data_bits = DATA_BITS_8;
    cfg.serial_stop_bits = STOP_BITS_1;
    cfg.serial_parity_mode = PARITY_NONE;
    cfg.serial_bit_order = 0;
    cfg.serial_invert_mode = 0;
    cfg.serial_buffer_size = 128;

    /*aiit board use ch438, so it nees more serial configuration*/
    if (use_aiit==1){
        cfg.ext_uart_no         = 0;
        cfg.port_configure      = 0;
    }

    ioctl(serial_fd, 0, &cfg);
    UserTaskDelay(1000);
    printf("Zigbee ready\n");

    return 0;
}

/* send message to srial port*/
int ZigbeeSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved)
{   
    write(serial_fd,data,strlen(data));


    return 0;
}


/*thread to read message from srial port*/
void SerialThreadEntry(void *parameter)
{
    char ch;
    int i = 0;    
    int n;
    int run = 0;
    while (1){   
        n = read(serial_fd,&ch,1); 
        if (n>0){
            if (ch == '~'){
                UserSemaphoreAbandon(rx_sem); 
                run = 1;
                break;
            }
            zigbee_buffer[i++] = ch;
        } 
        if (run ==1)
            break;
    }
}

int ZigbeeReceive(struct Adapter *padapter, char* rev_buffer, int buffer_len,int time_out, bool block, void* reserved)
{

    x_err_t ret = EOK;
    /* Set callback function */
    /* Create thread serial */
    UtaskType recv;
    recv.name[0] = 'z';
    recv.func_entry = SerialThreadEntry;
    recv.func_param = NONE;
    recv.stack_size = 1024;
    recv.prio = 25;
    memset(zigbee_buffer, 0, sizeof(zigbee_buffer));
    
    /* Initialize semaphore */
    rx_sem = UserSemaphoreCreate(0);

    zigbeereceive = UserTaskCreate(recv);   
    UserTaskStartup(zigbeereceive);

    /*copy to the receive buffer*/ 
    UserSemaphoreObtain(rx_sem,-1);
    memcpy(rev_buffer,zigbee_buffer,strlen(zigbee_buffer)+1 );

    return ret;    

}

void ZigbeeSettingDemo(int argc, char *argv[])
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

    ZigbeeOpen(padapter);
    /*zigbee communication settings*/
    /*it can be changed if needed*/
    char *set0 = "+++";
    char *set1_1 = "AT+DEV=C"; /*set device type for coordinater*/
    char *set1_2 = "AT+DEV=E"; /*set device type for end device*/
    char *set2 = "AT+MODE=1"; /*device mode 1 : passthrough */
    char *set3 = "AT+PANID=A1B2"; /* set PANID*/
    char *set4 = "AT+CH=11"; /* set channel*/
    char *set5 = "AT+EXIT";  /* exit AT mode*/
    write(serial_fd,set0,strlen(set0));
    UserTaskDelay(1000);
    /*type something in the command line to set this zigbee as coordinater*/
    /*otherwise it is an end device*/
    if (argc == 2){
        write(serial_fd,set1_1,strlen(set1_1));
        UserTaskDelay(1000); /*zigbee needs some time to process input message*/ 
    }else{
        write(serial_fd,set1_2,strlen(set1_2));
        UserTaskDelay(1000);
    }
    write(serial_fd,set2,strlen(set2));
    UserTaskDelay(1000);
    write(serial_fd,set3,strlen(set3));
    UserTaskDelay(1000);
    write(serial_fd,set4,strlen(set4));
    UserTaskDelay(1000);
    write(serial_fd,set5,strlen(set5));
    UserTaskDelay(1000);
    printf("zigbee setting success!\n");
}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
ZigbeeSettingDemo, ZigbeeSettingDemo,  zigbee send function );
#endif

void ZigbeeClose(struct Adapter *padapter)
{
    UserTaskDelete(zigbeereceive);
    close(serial_fd);
}
