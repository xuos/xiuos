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
* @file:    xs_Adapterxs_adapter_bluetooth.c
* @brief:   bluetooth open close function
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2021/4/30
*
*/
#include <xs_adapter_manager.h>
#include "xs_adapter_bluetooth.h"
#include <user_api.h>
#include <bus_serial.h>
#include <dev_serial.h>
#include <string.h>

#ifdef BOARD_K210_EVB
#define SAMPLE_UART_NAME       "/dev/uart3_dev3"  
#endif
#ifdef BOARD_STM32F407_EVB
#define SAMPLE_UART_NAME       "/dev/usart3_dev3"  
#endif

static int serial_fd;
static int32_t bluetooth_receive;
static int rx_sem;
char bluetooth_buffer[NAME_NUM_MAX ]={0}; 

/* initialize srial port to open bluetooth*/
int BluetoothOpen(struct Adapter *padapter)
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

    ioctl(serial_fd, 0, &cfg);
    UserTaskDelay(1000);
    printf("Bluetooth ready\n");

    return 0;
}

/* send message to srial port*/
int BluetoothSend(struct Adapter *padapter, const char* data, int len, bool block, int time_out, int delay, send_success cb, void* param, void* reserved)
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
            bluetooth_buffer[i++] = ch;
        } 
        if (run ==1)
            break;
    }
}

int BluetoothReceive(struct Adapter *padapter, char* rev_buffer, int buffer_len,int time_out, bool block, void* reserved)
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
    memset(bluetooth_buffer, 0, sizeof(bluetooth_buffer));
    
    /* Initialize semaphore */
    rx_sem = UserSemaphoreCreate(0);

    bluetooth_receive = UserTaskCreate(recv);   
    UserTaskStartup(bluetooth_receive);

    /*copy to the receive buffer*/ 
    UserSemaphoreObtain(rx_sem,-1);
    memcpy(rev_buffer,bluetooth_buffer,strlen(bluetooth_buffer)+1 );

    return ret;    

}

void BluetoothSettingDemo(int argc, char *argv[])
{
    
    adapter_t padapter = BluetoothAdapterFind("Bluetooth");
    if (NONE == padapter){
        KPrintf("adapter find failed!\n");
        return;
    }
    /*Open adapter*/
    if (0 != padapter->done.NetAiitOpen(padapter)){
        KPrintf("adapter open failed!\n");
        return;
    }

    BluetoothOpen(padapter);
    /*Bluetooth communication settings*/
    /*it can be changed if needed*/
    char* set5 = "AT";
    write(serial_fd,set5,strlen(set5));
    UserTaskDelay(1000);
    printf("bluetooth setting success!\n");
}
#ifndef SEPARATE_COMPILE
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
BluetoothSettingDemo, BluetoothSettingDemo,  bluetooth send function );
#endif

void BluetoothClose(struct Adapter *padapter)
{
    UserTaskDelete(bluetooth_receive);
    close(serial_fd);
}
