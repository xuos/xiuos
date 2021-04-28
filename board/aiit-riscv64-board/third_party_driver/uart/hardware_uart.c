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
* @file hardware_uart.c
* @brief add from Canaan k210 SDK
*                https://canaan-creative.com/developer
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#include <atomic.h>
#include <hardware_uart.h>
#include <plic.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysctl.h>
#include <utils.h>

#define __UART_BRATE_CONST  16

volatile UartT* const  uart[3] =
{
    (volatile UartT*)UART1_BASE_ADDR,
    (volatile UartT*)UART2_BASE_ADDR,
    (volatile UartT*)UART3_BASE_ADDR
};

#define UART_INTERRUPT_SEND                                         0x02U
#define UART_INTERRUPT_RECEIVE                                   0x04U
#define UART_INTERRUPT_CHARACTER_TIMEOUT     0x0CU

typedef struct UartInterruptInstance
{
    plic_irq_callback_t callback;
    void *ctx;
} UartInterruptInstanceT;

typedef struct UartInstance
{
    UartInterruptInstanceT UartReceiveInstance;
    UartInterruptInstanceT UartSendInstance;
    uint32_t UartNum;
} UartInstancePointer;

UartInstancePointer GUartInstance[3];

typedef struct UartDmaInstance
{
    uint8_t *buffer;
    size_t BufLen;
    uint32_t *MallocBuffer;
    UartInterruptModeT IntMode;
    dmac_channel_number_t dmac_channel;
    UartDeviceNumberT UartNum;
    UartInterruptInstanceT UartIntInstance;
} UartDmaInstanceT;

UartDmaInstanceT uart_send_dma_instance[3];
UartDmaInstanceT UartRecvDmaInstance[3];

typedef struct UartInstanceDma
{
    UartDeviceNumberT UartNum;
    UartInterruptModeT TransferMode;
    dmac_channel_number_t dmac_channel;
    plic_instance_t UartIntInstance;
    spinlock_t lock;
} UartInstanceDmaT;

static UartInstanceDmaT GUartSendInstanceDma[3];
static UartInstanceDmaT GUartRecvInstanceDma[3];

volatile int GWriteCount = 0;

static int UartIrqCallback(void *param)
{
    UartInstancePointer *uart_instance = (UartInstancePointer *)param;
    uint32_t v_channel = uart_instance->UartNum;
    uint8_t VIntStatus = uart[v_channel]->IIR & 0xF;

    if(VIntStatus == UART_INTERRUPT_SEND && GWriteCount != 0)
    {
        if(uart_instance->UartSendInstance.callback != NULL)
            uart_instance->UartSendInstance.callback(uart_instance->UartSendInstance.ctx);
    }
    else if(VIntStatus == UART_INTERRUPT_RECEIVE || VIntStatus == UART_INTERRUPT_CHARACTER_TIMEOUT)
    {
        if(uart_instance->UartReceiveInstance.callback != NULL)
            uart_instance->UartReceiveInstance.callback(uart_instance->UartReceiveInstance.ctx);
    }
    return 0;
}

static int UartapbPutc(UartDeviceNumberT channel, char c)
{
    while (uart[channel]->LSR & (1u << 5))
        continue;
    uart[channel]->THR = c;
    return 0;
}

int UartapbGetc(UartDeviceNumberT channel)
{
    while (!(uart[channel]->LSR & 1))
        continue;

    return (char)(uart[channel]->RBR & 0xff);
}

static int UartDmaCallback(void *ctx)
{
    UartDmaInstanceT *VUartDmaInstance = (UartDmaInstanceT *)ctx;
    dmac_channel_number_t dmac_channel = VUartDmaInstance->dmac_channel;
    dmac_irq_unregister(dmac_channel);

    if(VUartDmaInstance->IntMode == UART_RECEIVE)
    {
        size_t VBufLen = VUartDmaInstance->BufLen;
        uint8_t *VBuffer = VUartDmaInstance->buffer;
        uint32_t *VRecvBuffer = VUartDmaInstance->MallocBuffer;
        for(size_t i = 0; i < VBufLen; i++)
        {
            VBuffer[i] = VRecvBuffer[i];
        }
    }
    free(VUartDmaInstance->MallocBuffer);
    if(VUartDmaInstance->UartIntInstance.callback)
        VUartDmaInstance->UartIntInstance.callback(VUartDmaInstance->UartIntInstance.ctx);
    return 0;
}

int UartReceiveData(UartDeviceNumberT channel, char *buffer, size_t BufLen)
{
    size_t i = 0;
    for(i = 0;i < BufLen; i++)
    {
        if(uart[channel]->LSR & 1)
            buffer[i] = (char)(uart[channel]->RBR & 0xff);
        else
            break;
    }
    return i;
}

void UartReceiveDataDma(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel, uint8_t *buffer, size_t BufLen)
{
    uint32_t *VRecvBuf = malloc(BufLen * sizeof(uint32_t));
        configASSERT(VRecvBuf!=NULL);

    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_RX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, (void *)(&uart[uart_channel]->RBR), VRecvBuf, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, BufLen);
    dmac_wait_done(dmac_channel);
    for(uint32_t i = 0; i < BufLen; i++)
    {
        buffer[i] = (uint8_t)(VRecvBuf[i] & 0xff);
    }
    free(VRecvBuf);
}

void UartReceiveDataDmaIrq(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel,
                                     uint8_t *buffer, size_t BufLen, plic_irq_callback_t uart_callback,
                                     void *ctx, uint32_t priority)
{
    uint32_t *VRecvBuf = malloc(BufLen * sizeof(uint32_t));
        configASSERT(VRecvBuf!=NULL);

    UartRecvDmaInstance[uart_channel].dmac_channel = dmac_channel;
    UartRecvDmaInstance[uart_channel].UartNum = uart_channel;
    UartRecvDmaInstance[uart_channel].MallocBuffer = VRecvBuf;
    UartRecvDmaInstance[uart_channel].buffer = buffer;
    UartRecvDmaInstance[uart_channel].BufLen = BufLen;
    UartRecvDmaInstance[uart_channel].IntMode = UART_RECEIVE;
    UartRecvDmaInstance[uart_channel].UartIntInstance.callback = uart_callback;
    UartRecvDmaInstance[uart_channel].UartIntInstance.ctx = ctx;

    dmac_irq_register(dmac_channel, UartDmaCallback, &UartRecvDmaInstance[uart_channel], priority);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_RX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, (void *)(&uart[uart_channel]->RBR), VRecvBuf, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, BufLen);
}

int UartSendData(UartDeviceNumberT channel, const char *buffer, size_t BufLen)
{
    GWriteCount = 0;
    while (GWriteCount < BufLen)
    {
        UartapbPutc(channel, *buffer++);
        GWriteCount++;
    }
    return GWriteCount;
}

void UartSendDataDma(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel, const uint8_t *buffer, size_t BufLen)
{
    uint32_t *VSendBuf = malloc(BufLen * sizeof(uint32_t));
    configASSERT(VSendBuf!=NULL);
    for(uint32_t i = 0; i < BufLen; i++)
        VSendBuf[i] = buffer[i];
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_TX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, VSendBuf, (void *)(&uart[uart_channel]->THR), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, BufLen);
    dmac_wait_done(dmac_channel);
    free((void *)VSendBuf);
}

void UartSendDataDmaIrq(UartDeviceNumberT uart_channel, dmac_channel_number_t dmac_channel,
                            const uint8_t *buffer, size_t BufLen, plic_irq_callback_t uart_callback,
                            void *ctx, uint32_t priority)
{
    uint32_t *VSendBuf = malloc(BufLen * sizeof(uint32_t));
    configASSERT(VSendBuf!=NULL);

    uart_send_dma_instance[uart_channel] = (UartDmaInstanceT) {
        .dmac_channel = dmac_channel,
        .UartNum = uart_channel,
        .MallocBuffer = VSendBuf,
        .buffer = (uint8_t *)buffer,
        .BufLen = BufLen,
        .IntMode = UART_SEND,
        .UartIntInstance.callback = uart_callback,
        .UartIntInstance.ctx = ctx,
    };

    for(uint32_t i = 0; i < BufLen; i++)
        VSendBuf[i] = buffer[i];
    dmac_irq_register(dmac_channel, UartDmaCallback, &uart_send_dma_instance[uart_channel], priority);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_TX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, VSendBuf, (void *)(&uart[uart_channel]->THR), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, BufLen);

}

void uart_configure(UartDeviceNumberT channel, uint32_t BaudRate, UartBitwidthPointer DataWidth, UartStopbitT stopbit, UartParityT parity)
{
    configASSERT(DataWidth >= 5 && DataWidth <= 8);
    if (DataWidth == 5)
    {
        configASSERT(stopbit != UART_STOP_2);
    }
    else
    {
        configASSERT(stopbit != UART_STOP_1_5);
    }

    uint32_t stopbit_val = stopbit == UART_STOP_1 ? 0 : 1;
    uint32_t ParityVal;
    switch (parity)
    {
        case UART_PARITY_NONE:
            ParityVal = 0;
            break;
        case UART_PARITY_ODD:
            ParityVal = 1;
            break;
        case UART_PARITY_EVEN:
            ParityVal = 3;
            break;
        default:
            configASSERT(!"Invalid parity");
            break;
    }

    uint32_t freq = SysctlClockGetFreq(SYSCTL_CLOCK_APB0);
    uint32_t divisor = freq / BaudRate;
    uint8_t dlh = divisor >> 12;
    uint8_t dll = (divisor - (dlh << 12)) / __UART_BRATE_CONST;
    uint8_t dlf = divisor - (dlh << 12) - dll * __UART_BRATE_CONST;

    /* Set UART registers */
    uart[channel]->TCR &= ~(1u);
    uart[channel]->TCR &= ~(1u << 3);
    uart[channel]->TCR &= ~(1u << 4);
    uart[channel]->TCR |= (1u << 2);
    uart[channel]->TCR &= ~(1u << 1);
    uart[channel]->DE_EN &= ~(1u);

    uart[channel]->LCR |= 1u << 7;
    uart[channel]->DLH = dlh;
    uart[channel]->DLL = dll;
    uart[channel]->DLF = dlf;
    uart[channel]->LCR = 0;
    uart[channel]->LCR = (DataWidth - 5) | (stopbit_val << 2) | (ParityVal << 3);
    uart[channel]->LCR &= ~(1u << 7);
    uart[channel]->MCR &= ~3;
    uart[channel]->IER |= 0x80; /* THRE */
    uart[channel]->FCR = UART_RECEIVE_FIFO_1 << 6 | UART_SEND_FIFO_8 << 4 | 0x1 << 3 | 0x1;
}

void __attribute__((weak, alias("uart_configure")))
uart_config(UartDeviceNumberT channel, uint32_t BaudRate, UartBitwidthPointer DataWidth, UartStopbitT stopbit, UartParityT parity);

void UartInit(UartDeviceNumberT channel)
{
    sysctl_clock_enable(SYSCTL_CLOCK_UART1 + channel);
}

void UartSetSendTrigger(UartDeviceNumberT channel, uart_send_trigger_t trigger)
{
    uart[channel]->STET = trigger;
}

void uart_set_receive_trigger(UartDeviceNumberT channel, uart_receive_trigger_t trigger)
{
    uart[channel]->SRT = trigger;
}

void uart_irq_register(UartDeviceNumberT channel, UartInterruptModeT interrupt_mode, plic_irq_callback_t uart_callback, void *ctx, uint32_t priority)
{
    if(interrupt_mode == UART_SEND)
    {
        uart[channel]->IER |= 0x2;
        GUartInstance[channel].UartSendInstance.callback = uart_callback;
        GUartInstance[channel].UartSendInstance.ctx = ctx;
    }
    else if(interrupt_mode == UART_RECEIVE)
    {
        uart[channel]->IER |= 0x1;
        GUartInstance[channel].UartReceiveInstance.callback = uart_callback;
        GUartInstance[channel].UartReceiveInstance.ctx = ctx;
    }
    GUartInstance[channel].UartNum = channel;
    plic_set_priority(IRQN_UART1_INTERRUPT + channel, priority);
    plic_irq_register(IRQN_UART1_INTERRUPT + channel, UartIrqCallback, &GUartInstance[channel]);
    plic_irq_enable(IRQN_UART1_INTERRUPT + channel);
}

void uart_irq_unregister(UartDeviceNumberT channel, UartInterruptModeT interrupt_mode)
{
    if(interrupt_mode == UART_SEND)
    {
        uart[channel]->IER &= ~(0x2);
        GUartInstance[channel].UartSendInstance.callback = NULL;
        GUartInstance[channel].UartSendInstance.ctx = NULL;
    }
    else if(interrupt_mode == UART_RECEIVE)
    {
        uart[channel]->IER &= ~(0x1);
        GUartInstance[channel].UartReceiveInstance.callback = NULL;
        GUartInstance[channel].UartReceiveInstance.ctx = NULL;
    }
    if(uart[channel]->IER == 0)
    {
        plic_irq_unregister(IRQN_UART1_INTERRUPT + channel);
    }
}

int uart_dma_irq(void *ctx)
{
    UartInstanceDmaT *v_instance = (UartInstanceDmaT *)ctx;
    dmac_irq_unregister(v_instance->dmac_channel);
    if(v_instance->TransferMode == UART_SEND)
    {
        while(!(uart[v_instance->UartNum]->LSR & (1u << 6)));
    }
    spinlock_unlock(&v_instance->lock);
    if(v_instance->UartIntInstance.callback)
    {
        v_instance->UartIntInstance.callback(v_instance->UartIntInstance.ctx);
    }
    return 0;
}

void uart_handle_data_dma(UartDeviceNumberT uart_channel ,uart_data_t data, plic_interrupt_t *cb)
{
    configASSERT(uart_channel < UART_DEVICE_MAX);
    if(data.TransferMode == UART_SEND)
    {
        configASSERT(data.tx_buf && data.tx_len && data.tx_channel < DMAC_CHANNEL_MAX);
        spinlock_lock(&GUartSendInstanceDma[uart_channel].lock);
        if(cb)
        {
            GUartSendInstanceDma[uart_channel].UartIntInstance.callback = cb->callback;
            GUartSendInstanceDma[uart_channel].UartIntInstance.ctx = cb->ctx;
            GUartSendInstanceDma[uart_channel].dmac_channel = data.tx_channel;
            GUartSendInstanceDma[uart_channel].TransferMode = UART_SEND;
            dmac_irq_register(data.tx_channel, uart_dma_irq, &GUartSendInstanceDma[uart_channel], cb->priority);
        }
        sysctl_dma_select((sysctl_dma_channel_t)data.tx_channel, SYSCTL_DMA_SELECT_UART1_TX_REQ + uart_channel * 2);
        dmac_set_single_mode(data.tx_channel, data.tx_buf, (void *)(&uart[uart_channel]->THR), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
            DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, data.tx_len);
        if(!cb)
        {
            dmac_wait_done(data.tx_channel);
            while(!(uart[uart_channel]->LSR & (1u << 6)));
            spinlock_unlock(&GUartSendInstanceDma[uart_channel].lock);
        }
    }
    else
    {
        configASSERT(data.rx_buf && data.rx_len && data.rx_channel < DMAC_CHANNEL_MAX);
        spinlock_lock(&GUartRecvInstanceDma[uart_channel].lock);
        if(cb)
        {
            GUartRecvInstanceDma[uart_channel].UartIntInstance.callback = cb->callback;
            GUartRecvInstanceDma[uart_channel].UartIntInstance.ctx = cb->ctx;
            GUartRecvInstanceDma[uart_channel].dmac_channel = data.rx_channel;
            GUartRecvInstanceDma[uart_channel].TransferMode = UART_RECEIVE;
            dmac_irq_register(data.rx_channel, uart_dma_irq, &GUartRecvInstanceDma[uart_channel], cb->priority);
        }
        sysctl_dma_select((sysctl_dma_channel_t)data.rx_channel, SYSCTL_DMA_SELECT_UART1_RX_REQ + uart_channel * 2);
        dmac_set_single_mode(data.rx_channel, (void *)(&uart[uart_channel]->RBR), data.rx_buf, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
                DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, data.rx_len);
        if(!cb)
        {
            dmac_wait_done(data.rx_channel);
            spinlock_unlock(&GUartRecvInstanceDma[uart_channel].lock);
        }
    }
}
