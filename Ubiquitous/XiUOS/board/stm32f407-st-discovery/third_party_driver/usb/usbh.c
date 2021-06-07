/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2017-10-30     ZYH            the first version
 * 2019-12-19     tyustli           port to stm32 series
 */

/**
* @file usbh.c
* @brief support stm32f407-st-discovery-board usb function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: usbh.c
Description: support stm32f407-st-discovery-board usb configure
Others: take RT-Thread v4.0.2/bsp/stm32/libraries/HAL_Drivers/drv_usbh.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. support stm32f407-st-discovery-board usb irq configure
2. support stm32f407-st-discovery-board usb host register
*************************************************/

#include <xiuos.h>
#include <usb_host.h>
#include <hardware_gpio.h>
#include <hardware_rcc.h>
#include <stm32f4xx.h>
#include <misc.h>
#include "usb_bsp.h"
#include "usb_hcd.h"
#include "usb_hcd_int.h"

static USB_OTG_CORE_HANDLE USB_OTG_Core;
static int UrbCompletionSem;

void OTG_FS_IRQHandler(int irq_num, void *arg)
{
    USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
DECLARE_HW_IRQ(OTG_FS_IRQn, OTG_FS_IRQHandler, NONE);

static uint8_t SOF_cb(USB_OTG_CORE_HANDLE *pdev)
{
    return 0;
}

static uint8_t DevConnected_cb(USB_OTG_CORE_HANDLE *pdev)
{
    UhcdPointer hcd = pdev->data;

    if (!pdev->host.ConnSts) {
        pdev->host.ConnSts = 1;
        UsbhRootHubConnectHandler(hcd, 1, RET_FALSE);
    }

    return 0;
}

static uint8_t DevDisconnected_cb(USB_OTG_CORE_HANDLE *pdev)
{
    UhcdPointer hcd = pdev->data;

    if (pdev->host.ConnSts) {
        pdev->host.ConnSts = 0;
        UsbhRootHubDisconnectHandler(hcd, 1);
    }

    return 0;
}

static uint8_t URBChangeNotify_cb(USB_OTG_CORE_HANDLE *pdev)
{
    KSemaphoreAbandon(UrbCompletionSem);

    return 0;
}

static uint8_t DevPortEnabled_cb(USB_OTG_CORE_HANDLE *pdev)
{
    pdev->host.PortEnabled = 1;

    return 0;
}

static uint8_t DevPortDisabled_cb(USB_OTG_CORE_HANDLE *pdev)
{
    pdev->host.PortEnabled = 0;

    return 0;
}

static USBH_HCD_INT_cb_TypeDef USBH_HCD_INT_cb = {
    .SOF = SOF_cb,
    .DevConnected = DevConnected_cb,
    .DevDisconnected = DevDisconnected_cb,
    .DevPortEnabled = DevPortEnabled_cb,
    .DevPortDisabled = DevPortDisabled_cb,
    .URBChangeNotify = URBChangeNotify_cb,
};
USBH_HCD_INT_cb_TypeDef *USBH_HCD_INT_fops = &USBH_HCD_INT_cb;

static void STM32USBHostChannelOpen(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num, uint8_t epnum,
        uint8_t dev_address, uint8_t speed, uint8_t ep_type, uint16_t mps)
{
    pdev->host.hc[hc_num].ep_num = epnum & 0x7F;
    pdev->host.hc[hc_num].ep_is_in = (epnum & 0x80 ) == 0x80;
    pdev->host.hc[hc_num].DevAddr = dev_address;
    pdev->host.hc[hc_num].ep_type = ep_type;
    pdev->host.hc[hc_num].max_packet = mps;
    pdev->host.hc[hc_num].speed = speed;
    pdev->host.hc[hc_num].toggle_in = 0;
    pdev->host.hc[hc_num].toggle_out = 0;
    if(speed == HPRT0_PRTSPD_HIGH_SPEED) {
        pdev->host.hc[hc_num].do_ping = 1;
    }

    USB_OTG_HC_Init(pdev, hc_num) ;
}

static x_err_t STM32USBHostResetPort(uint8 port)
{
    SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("reset port\n"));
    HCD_ResetPort(&USB_OTG_Core);

    return EOK;
}

x_err_t STM32USBHostChannelSubmitRequest(USB_OTG_CORE_HANDLE *hhcd,
                                  uint8_t ch_num,
                                  uint8_t direction,
                                  uint8_t ep_type,
                                  uint8_t token,
                                  uint8_t *pbuff,
                                  uint16_t length,
                                  uint8_t do_ping)
{
    SYS_KDEBUG_LOG(SYS_DEBUG_USB,
            ("%s: ch_num: %d, direction: %d, ", __func__, ch_num, direction));
    hhcd->host.hc[ch_num].ep_is_in = direction;
    hhcd->host.hc[ch_num].ep_type  = ep_type;

    if (token == 0U)
        hhcd->host.hc[ch_num].data_pid = HC_PID_SETUP;
    else
        hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;

    /* Manage Data Toggle */
    switch (ep_type) {
    case EP_TYPE_CTRL:
        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("EP_TYPE_CTRL\n"));
        if ((token == 1U) && (direction == 0U)) {
            if (length == 0U)
                hhcd->host.hc[ch_num].toggle_out = 1U;

            if (hhcd->host.hc[ch_num].toggle_out == 0U)
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
            else
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;
        }
        break;

    case EP_TYPE_BULK:
        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("EP_TYPE_BULK\n"));
        if (direction == 0U) {
            if (hhcd->host.hc[ch_num].toggle_out == 0U)
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
            else
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;
        } else {
            if (hhcd->host.hc[ch_num].toggle_in == 0U)
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
            else
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;
        }
        break;

    case EP_TYPE_INTR:
        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("EP_TYPE_INTR\n"));
        if (direction == 0U) {
            if (hhcd->host.hc[ch_num].toggle_out == 0U)
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
            else
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;
        } else {
            if (hhcd->host.hc[ch_num].toggle_in == 0U)
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
            else
                hhcd->host.hc[ch_num].data_pid = HC_PID_DATA1;
        }
        break;

    case EP_TYPE_ISOC:
        SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("EP_TYPE_ISOC\n"));
        hhcd->host.hc[ch_num].data_pid = HC_PID_DATA0;
        break;

    default:
        break;
    }

    hhcd->host.hc[ch_num].xfer_buff = pbuff;
    hhcd->host.hc[ch_num].XferLen  = length;
    hhcd->host.HC_Status[ch_num] = HC_IDLE;

    return HCD_SubmitRequest(hhcd, ch_num);
}

static int STM32USBHostPipeXfer(upipe_t pipe, uint8 token, void *buffer, int nbytes, int timeouts)
{
    int timeout = timeouts;
    int interval = 1;
    int retry = 0;

    while (1) {
        if (!USB_OTG_Core.host.ConnSts)
            return -1;

        // UrbCompletionSem = KSemaphoreCreate( 0);
        KSemaphoreSetValue(UrbCompletionSem, 0);
        STM32USBHostChannelSubmitRequest(
            &USB_OTG_Core,
            pipe->pipe_index,
            (pipe->ep.bEndpointAddress & 0x80) >> 7,
            pipe->ep.bmAttributes,
            token,
            buffer,
            nbytes,
            0
        );

        MdelayKTask(interval);
        KSemaphoreObtain(UrbCompletionSem, timeout);

        if (HCD_GetHCState(&USB_OTG_Core, pipe->pipe_index) == HC_NAK) {
            SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("nak\n"));
            if (pipe->ep.bmAttributes == USB_EP_ATTR_INT)
                DelayKTask((pipe->ep.bInterval * TICK_PER_SECOND / 1000) > 0 ? (pipe->ep.bInterval * TICK_PER_SECOND / 1000) : 1);
            if (interval < 10) {
                interval += 1;
                continue;
            }
            USB_OTG_HC_Halt(&USB_OTG_Core, pipe->pipe_index);
            STM32USBHostChannelOpen(
                &USB_OTG_Core,
                pipe->pipe_index,
                pipe->ep.bEndpointAddress,
                pipe->inst->address,
                USB_OTG_SPEED_FULL,
                pipe->ep.bmAttributes,
                pipe->ep.wMaxPacketSize
            );
            if (++retry >= 10) {
                SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("NAK retry limit exceeded\n"));
                return -1;
            }
            continue;
        } else if (HCD_GetHCState(&USB_OTG_Core, pipe->pipe_index) == HC_STALL) {
            SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("stall\n"));
            pipe->status = UPIPE_STATUS_STALL;
            if (pipe->callback != NONE)
                pipe->callback(pipe);
            return -1;
        } else if (HCD_GetHCState(&USB_OTG_Core, pipe->pipe_index) == URB_ERROR) {
            SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("error\n"));
            pipe->status = UPIPE_STATUS_ERROR;
            if (pipe->callback != NONE)
                pipe->callback(pipe);
            return -1;
        } else if (URB_DONE == HCD_GetHCState(&USB_OTG_Core, pipe->pipe_index)) {
            SYS_KDEBUG_LOG(SYS_DEBUG_USB, ("ok\n"));
            pipe->status = UPIPE_STATUS_OK;
            if (pipe->callback != NONE)
                pipe->callback(pipe);
            // size_t size = HAL_HCD_HC_GetXferCount(&USB_OTG_Core, pipe->pipe_index);
            size_t size = USB_OTG_Core.host.XferCnt[pipe->pipe_index];
            if (pipe->ep.bEndpointAddress & 0x80)
                return size;
            else if (pipe->ep.bEndpointAddress & 0x00)
                return size;
            return nbytes;
        }

        continue;
    }
}

static uint16 pipe_bitmap;

static uint8 STM32USBHostAllocPipe()
{
    for (int i = 1; i < 16; i++)
        if ((pipe_bitmap & (1 << i)) == 0) {
            pipe_bitmap |= (1 << i);
            return i;
        }

    return 0xff;
}

static void STM32USBHostFreePipe(int i)
{
    pipe_bitmap &= ~(1 << i);
}

static x_err_t STM32USBHostOpenPipe(upipe_t pipe)
{
    pipe->pipe_index = STM32USBHostAllocPipe();

    STM32USBHostChannelOpen(&USB_OTG_Core, pipe->pipe_index, pipe->ep.bEndpointAddress,
            pipe->inst->address, USB_OTG_SPEED_FULL, pipe->ep.bmAttributes,
            pipe->ep.wMaxPacketSize);

    if (USB_OTG_Core.host.hc[pipe->pipe_index].ep_is_in)
        USB_OTG_Core.host.hc[pipe->pipe_index].toggle_in = 0;
    else
        USB_OTG_Core.host.hc[pipe->pipe_index].toggle_out = 0;

    return EOK;
}

static x_err_t STM32USBHostClosePipe(upipe_t pipe)
{
    USB_OTG_HC_Halt(&USB_OTG_Core, pipe->pipe_index);

    STM32USBHostFreePipe(pipe->pipe_index);

    return EOK;
}

/* implement these */
static struct uhcd_ops USBHostOps = {
    .reset_port = STM32USBHostResetPort,
    .pipe_xfer = STM32USBHostPipeXfer,
    .open_pipe = STM32USBHostOpenPipe,
    .close_pipe = STM32USBHostClosePipe
};

static x_err_t STM32USBHostInit()
{
    KPrintf("\nINITIALIZING HOST......\n");

    USB_OTG_BSP_Init(&USB_OTG_Core);

    HCD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID);

    // RegisterHwIrq(OTG_FS_IRQn, OTG_FS_IRQHandler, NONE);
    USB_OTG_BSP_ENABLE_INTERRUPT(&USB_OTG_Core);

    UrbCompletionSem = KSemaphoreCreate( 0);

    return EOK;
}

int STM32USBHostRegister()
{
    UhcdPointer uhcd = (UhcdPointer)x_malloc(sizeof(struct uhcd));

    memset(uhcd, 0, sizeof(struct uhcd));

    uhcd->ops = &USBHostOps;
    uhcd->NumPorts = 1;

    USB_OTG_Core.data = uhcd;

    UsbHostInit(uhcd);
    STM32USBHostInit();

    return EOK;
}

long dump_usb_regs()
{
    volatile uint32_t *p = (volatile uint32_t *)0x50000000;

    while ((uint32_t)p < 0x50000000 + 80 * 4) {
        KPrintf("0x%03X: 0x%08X\n", (uint32_t)p - 0x50000000, *p);
        p++;
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),dump_usb_regs, dump_usb_regs,  dump USB registers );

long dump_hc_regs()
{
    for (int i = 0; i < USB_OTG_MAX_TX_FIFOS; i++) {
        USB_OTG_HC_REGS *regs = USB_OTG_Core.regs.HC_REGS[i];
        KPrintf("EP No.%d:\n", i);
        KPrintf("    HCCHAR: 0x%08X\n", regs->HCCHAR);
        KPrintf("    HCSPLT: 0x%08X\n", regs->HCSPLT);
        KPrintf("    HCINT: 0x%08X\n", regs->HCINT);
        KPrintf("    HCINTMSK: 0x%08X\n", regs->HCINTMSK);
        KPrintf("    HCTSIZ: 0x%08X\n", regs->HCTSIZ);
        KPrintf("    HCDMA: 0x%08X\n", regs->HCDMA);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),dump_hc_regs, dump_hc_regs,  dump_hc_regs );
