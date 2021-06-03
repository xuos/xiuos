/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @brief       Universal Asynchronous Receiver/Transmitter (UART)
 *
 *              The UART peripheral supports the following features:
 *
 *              - 8-N-1 and 8-N-2 formats: 8 data bits, no parity bit, 1 start
 *                bit, 1 or 2 stop bits
 *
 *              - 8-entry transmit and receive FIFO buffers with programmable
 *                watermark interrupts
 *
 *              - 16× Rx oversampling with 2/3 majority voting per bit
 *
 *              The UART peripheral does not support hardware flow control or
 *              other modem control signals, or synchronous serial data
 *              tranfesrs.
 *
 *
 */

/**
* @file hardware_uart.h
* @brief add from Canaan k210 SDK
*                https://canaan-creative.com/developer
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef __HARDWARE_UART_H__
#define __HARDWARE_UART_H__

#include "dmac.h"
#include <platform.h>
#include "plic.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _uart_dev
{
    UART_DEV1 = 0,
    UART_DEV2,
    UART_DEV3,
} uart_dev_t;

typedef struct _uart
{
    union
    {
        volatile uint32_t RBR;
        volatile uint32_t DLL;
        volatile uint32_t THR;
    };

    union
    {
        volatile uint32_t DLH;
        volatile uint32_t IER;
    };

    union
    {
        volatile uint32_t FCR;
        volatile uint32_t IIR;
    };

    volatile uint32_t LCR;
    volatile uint32_t MCR;
    volatile uint32_t LSR;
    volatile uint32_t MSR;

    volatile uint32_t SCR;
    volatile uint32_t LPDLL;
    volatile uint32_t LPDLH;

    volatile uint32_t reserved1[2];

    union
    {
        volatile uint32_t SRBR[16];
        volatile uint32_t STHR[16];
    };

    volatile uint32_t FAR;
    volatile uint32_t TFR;
    volatile uint32_t RFW;
    volatile uint32_t USR;
    volatile uint32_t TFL;
    volatile uint32_t RFL;
    volatile uint32_t SRR;
    volatile uint32_t SRTS;
    volatile uint32_t SBCR;
    volatile uint32_t SDMAM;
    volatile uint32_t SFE;
    volatile uint32_t SRT;
    volatile uint32_t STET;
    volatile uint32_t HTX;
    volatile uint32_t DMASA;
    volatile uint32_t TCR;
    volatile uint32_t DE_EN;
    volatile uint32_t RE_EN;
    volatile uint32_t DET;
    volatile uint32_t TAT;
    volatile uint32_t DLF;
    volatile uint32_t RAR;
    volatile uint32_t TAR;
    volatile uint32_t LCR_EXT;
    volatile uint32_t reserved2[9];
    volatile uint32_t CPR;
    volatile uint32_t UCV;
    volatile uint32_t CTR;
} UartT;

typedef enum _uart_device_number
{
    UART_DEVICE_1,
    UART_DEVICE_2,
    UART_DEVICE_3,
    UART_DEVICE_MAX,
} UartDeviceNumberT;

typedef enum _uart_bitwidth
{
    UART_BITWIDTH_5BIT = 5,
    UART_BITWIDTH_6BIT,
    UART_BITWIDTH_7BIT,
    UART_BITWIDTH_8BIT,
} UartBitwidthPointer;

typedef enum _uart_stopbit
{
    UART_STOP_1,
    UART_STOP_1_5,
    UART_STOP_2
} UartStopbitT;

typedef enum _uart_rede_sel
{
    DISABLE = 0,
    ENABLE,
} uart_rede_sel_t;

typedef enum _uart_parity
{
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN
} UartParityT;

typedef enum _uart_interrupt_mode
{
    UART_SEND = 1,
    UART_RECEIVE = 2,
} UartInterruptModeT;

typedef enum _uart_send_trigger
{
    UART_SEND_FIFO_0,
    UART_SEND_FIFO_2,
    UART_SEND_FIFO_4,
    UART_SEND_FIFO_8,
} uart_send_trigger_t;

typedef enum _uart_receive_trigger
{
    UART_RECEIVE_FIFO_1,
    UART_RECEIVE_FIFO_4,
    UART_RECEIVE_FIFO_8,
    UART_RECEIVE_FIFO_14,
} uart_receive_trigger_t;

typedef struct _uart_data_t
{
    dmac_channel_number_t tx_channel;
    dmac_channel_number_t rx_channel;
    uint32_t *tx_buf;
    size_t tx_len;
    uint32_t *rx_buf;
    size_t rx_len;
    UartInterruptModeT TransferMode;
} uart_data_t;

/**
 * @brief       Send data from uart
 *
 * @param[in]   channel     Uart index
 * @param[in]   buffer      The data be transfer
 * @param[in]   len         The data length
 *
 * @return      Transfer length
 */
int UartSendData(UartDeviceNumberT channel, const char *buffer, size_t BufLen);

/**
 * @brief       Read data from uart
 *
 * @param[in]   channel     Uart index
 * @param[in]   buffer      The Data received
 * @param[in]   len         Receive length
 *
 * @return      Receive length
 */
int UartReceiveData(UartDeviceNumberT channel, char *buffer, size_t BufLen);

/**
 * @brief       Init uart
 *
 * @param[in]   channel     Uart index
 *
 */
void UartInit(UartDeviceNumberT channel);

/**
 * @brief       Set uart param
 *
 * @param[in]   channel         Uart index
 * @param[in]   BaudRate       Baudrate
 * @param[in]   DataWidth      Data width
 * @param[in]   stopbit         Stop bit
 * @param[in]   parity          Odd Even parity
 *
 */
void uart_config(UartDeviceNumberT channel, uint32_t BaudRate, UartBitwidthPointer DataWidth, UartStopbitT stopbit, UartParityT parity);

/**
 * @brief       Set uart param
 *
 * @param[in]   channel         Uart index
 * @param[in]   BaudRate       Baudrate
 * @param[in]   DataWidth      Data width
 * @param[in]   stopbit         Stop bit
 * @param[in]   parity          Odd Even parity
 *
 */
void uart_configure(UartDeviceNumberT channel, uint32_t BaudRate, UartBitwidthPointer DataWidth, UartStopbitT stopbit, UartParityT parity);

/**
 * @brief       Register uart interrupt
 *
 * @param[in]   channel             Uart index
 * @param[in]   interrupt_mode      Interrupt Mode receive or send
 * @param[in]   uart_callback       Call back
 * @param[in]   ctx                 Param of call back
 * @param[in]   priority            Interrupt priority
 *
 */
void uart_irq_register(UartDeviceNumberT channel, UartInterruptModeT interrupt_mode, plic_irq_callback_t uart_callback, void *ctx, uint32_t priority);

/**
 * @brief       Deregister uart interrupt
 *
 * @param[in]   channel             Uart index
 * @param[in]   interrupt_mode      Interrupt Mode receive or send
 *
 */
void uart_irq_unregister(UartDeviceNumberT channel, UartInterruptModeT interrupt_mode);

/**
 * @brief       Set send interrupt threshold
 *
 * @param[in]   channel             Uart index
 * @param[in]   trigger             Threshold of send interrupt
 *
 */
void UartSetSendTrigger(UartDeviceNumberT channel, uart_send_trigger_t trigger);

/**
 * @brief       Set receive interrupt threshold
 *
 * @param[in]   channel             Uart index
 * @param[in]   trigger             Threshold of receive interrupt
 *
 */
void uart_set_receive_trigger(UartDeviceNumberT channel, uart_receive_trigger_t trigger);

/**
 * @brief       Send data by dma
 *
 * @param[in]   channel             Uart index
 * @param[in]   dmac_channel        Dmac channel
 * @param[in]   buffer              Send data
 * @param[in]   BufLen             Data length
 *
 */
void UartSendDataDma(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel, const uint8_t *buffer, size_t BufLen);

/**
 * @brief       Receive data by dma
 *
 * @param[in]   channel             Uart index
 * @param[in]   dmac_channel        Dmac channel
 * @param[in]   buffer              Receive data
 * @param[in]   BufLen             Data length
 *
 */
void UartReceiveDataDma(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel, uint8_t *buffer, size_t BufLen);


/**
 * @brief       Send data by dma
 *
 * @param[in]   uart_channel        Uart index
 * @param[in]   dmac_channel        Dmac channel
 * @param[in]   buffer              Send data
 * @param[in]   BufLen             Data length
 * @param[in]   uart_callback       Call back
 * @param[in]   ctx                 Param of call back
 * @param[in]   priority            Interrupt priority
 *
 */
void UartSendDataDmaIrq(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel,
                            const uint8_t *buffer, size_t BufLen, plic_irq_callback_t uart_callback,
                            void *ctx, uint32_t priority);

/**
 * @brief       Receive data by dma
 *
 * @param[in]   uart_channel        Uart index
 * @param[in]   dmac_channel        Dmac channel
 * @param[in]   buffer              Receive data
 * @param[in]   BufLen             Data length
 * @param[in]   uart_callback       Call back
 * @param[in]   ctx                 Param of call back
 * @param[in]   priority            Interrupt priority
 *
 */
void UartReceiveDataDmaIrq(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel,
                               uint8_t *buffer, size_t BufLen, plic_irq_callback_t uart_callback,
                               void *ctx, uint32_t priority);

/**
 * @brief       Uart handle transfer data operations
 *
 * @param[in]   uart_channel        Uart index
 * @param[in]   data                Uart data information
 * @param[in]   buffer              Uart DMA callback
 *
 */
void uart_handle_data_dma(UartDeviceNumberT uart_channel ,uart_data_t data, plic_interrupt_t *cb);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H__ */
