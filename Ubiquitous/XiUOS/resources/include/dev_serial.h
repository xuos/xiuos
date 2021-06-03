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
* @file dev_serial.h
* @brief define serial dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_SERIAL_H
#define DEV_SERIAL_H

#include <bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BAUD_RATE_2400                       2400
#define BAUD_RATE_4800                       4800
#define BAUD_RATE_9600                       9600
#define BAUD_RATE_19200                  19200
#define BAUD_RATE_38400                  38400
#define BAUD_RATE_57600                  57600
#define BAUD_RATE_115200             115200
#define BAUD_RATE_230400             230400
#define BAUD_RATE_460800             460800
#define BAUD_RATE_921600             921600
#define BAUD_RATE_2000000        2000000
#define BAUD_RATE_3000000        3000000

#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

#define STOP_BITS_1                     1
#define STOP_BITS_2                     2
#define STOP_BITS_3                     3
#define STOP_BITS_4                     4

#define PARITY_NONE                     1
#define PARITY_ODD                        2
#define PARITY_EVEN                      3

#define BIT_ORDER_LSB                   1
#define BIT_ORDER_MSB                  2

#define NRZ_NORMAL                      1     
#define NRZ_INVERTED                   2    

#ifndef SERIAL_RB_BUFSZ
#define SERIAL_RB_BUFSZ              128
#endif

#define SERIAL_EVENT_RX_IND                     0x01 
#define SERIAL_event_id_tX_DONE             0x02 
#define SERIAL_EVENT_RX_DMADONE       0x03   
#define SERIAL_event_id_tX_DMADONE    0x04  
#define SERIAL_EVENT_RX_TIMEOUT         0x05   

#define SERIAL_DMA_RX                0x01
#define SERIAL_DMA_TX                0x02

struct SerialTx
{
    int32 serial_txfifo_sem;

    x_bool serial_dma_enable;
    queue serial_dma_queue;
};

struct SerialRx
{
    uint8 *serial_rx_buffer;
    uint16 serial_send_num;
    uint16 serial_recv_num;
    x_bool serial_rx_full;

    x_bool serial_dma_enable;
};

struct SerialDataTransferParam
{
    struct SerialTx *serial_tx;

    struct SerialRx *serial_rx;
};

struct SerialDevParam
{
    uint8 ext_uart_no;
    
    uint16 serial_work_mode;
    uint16 serial_set_mode;
    uint16 serial_stream_mode;
};

struct SerialHardwareDevice;

struct SerialHwDevDone
{
    int (*put_char) (struct SerialHardwareDevice *serial_dev, char c);
    int (*get_char) (struct SerialHardwareDevice *serial_dev);
    int (*dmatransfer) (struct SerialHardwareDevice *serial_dev, uint8 *buf, x_size_t size, int direction);
};

struct SerialDevDone
{
    uint32 (*open) (void *dev);
    uint32 (*close) (void *dev);
    uint32 (*write) (void *dev, struct BusBlockWriteParam *datacfg);
    uint32 (*read) (void *dev, struct BusBlockReadParam *datacfg);
};

struct SerialHardwareDevice
{
    struct HardwareDev haldev;
    struct SerialHwDevDone *hwdev_done;

    struct SerialDataTransferParam serial_fifo;

    uint32 ext_serial_mode;
    const struct SerialDevDone *dev_done;

    void *private_data;
};

/*Register the serial device*/
int SerialDeviceRegister(struct SerialHardwareDevice *serial_device, void *serial_param, const char *device_name);

/*Register the serial device to the serial bus*/
int SerialDeviceAttachToBus(const char *dev_name, const char *bus_name);

/*Find the register serial device*/
HardwareDevType SerialDeviceFind(const char *dev_name, enum DevType dev_type);

/*Set serial isr function*/
void SerialSetIsr(struct SerialHardwareDevice *serial_dev, int event);

#ifdef __cplusplus
}
#endif

#endif
