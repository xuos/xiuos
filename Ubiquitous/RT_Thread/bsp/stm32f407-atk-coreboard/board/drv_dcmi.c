/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2020-07-27     thread-liu        the first version
 */

#include "board.h"
#ifdef BSP_USING_DCMI 
#include <drv_dcmi.h>
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#ifdef RT_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif
#endif

#define DRV_DEBUG
#define LOG_TAG             "drv.dcmi"
#include <drv_log.h>
static void (*dcmi_irq_callback)(void) = NULL;

static struct stm32_dcmi rt_dcmi = {0};
DMA_HandleTypeDef hdma_dcmi;
static void rt_hw_dcmi_dma_init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();
    hdma_dcmi.Instance                 = DMA2_Stream1;
    hdma_dcmi.Init.Channel             = DMA_CHANNEL_1;
    hdma_dcmi.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode                = DMA_NORMAL;
    hdma_dcmi.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_dcmi.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_dcmi.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_dcmi.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_dcmi);
    __HAL_LINKDMA(&rt_dcmi.DCMI_Handle, DMA_Handle, hdma_dcmi);
    __HAL_DMA_ENABLE_IT(&hdma_dcmi,DMA_IT_TC);   
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0x00, 0x00);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);;
}
/*
DMA  multi_buffer DMA Transfer.
*/
void rt_hw_dcmi_dma_config(rt_uint32_t dst_addr1, rt_uint32_t dst_addr2, rt_uint32_t len)
{
      HAL_DMAEx_MultiBufferStart(&hdma_dcmi, (rt_uint32_t)&DCMI->DR, dst_addr1, dst_addr2, len);

    __HAL_DMA_ENABLE_IT(&hdma_dcmi, DMA_IT_TC);
}

static rt_err_t rt_hw_dcmi_init(DCMI_HandleTypeDef *device)
{
    RT_ASSERT(device != RT_NULL);

    device->Instance               = DCMI;
    device->Init.SynchroMode       = DCMI_SYNCHRO_HARDWARE;
    device->Init.PCKPolarity       = DCMI_PCKPOLARITY_RISING;
    device->Init.VSPolarity        = DCMI_VSPOLARITY_LOW;
    device->Init.HSPolarity        = DCMI_HSPOLARITY_LOW;
    device->Init.CaptureRate       = DCMI_CR_ALL_FRAME;
    device->Init.ExtendedDataMode  = DCMI_EXTEND_DATA_8B;
    device->Init.JPEGMode          = DCMI_JPEG_ENABLE;
    #if defined(STM32F446xx) || defined(STM32F469xx) || defined(STM32F479xx)
    device->Init.ByteSelectMode    = DCMI_BSM_ALL;
    device->Init.ByteSelectStart   = DCMI_OEBS_ODD;
    device->Init.LineSelectMode    = DCMI_LSM_ALL;
    device->Init.LineSelectStart   = DCMI_OELS_ODD;
    #endif
    if(HAL_DCMI_Init(device) != HAL_OK)
    {
        LOG_E("dcmi init error!");
        return RT_ERROR;
    }
    else
    {
        LOG_I("dcmi HAL_DCMI_Init success");
    }
    DCMI->IER = 0x0;
    __HAL_DCMI_ENABLE_IT(device, DCMI_IT_FRAME);
    __HAL_DCMI_ENABLE(device);
    return RT_EOK;
}

void DCMI_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_DCMI_IRQHandler(&rt_dcmi.DCMI_Handle);
    /* leave interrupt */
    rt_interrupt_leave();
}
/*
the camera starts transfering photos
*/
void rt_dcmi_start(uint32_t pData, uint32_t Length)
{
    HAL_DCMI_Start_DMA(&rt_dcmi.DCMI_Handle,DCMI_MODE_SNAPSHOT,pData, Length);        
}

/*
the camera stops transfering photos
*/
void rt_dcmi_stop(void)
{
    HAL_DCMI_Stop(&rt_dcmi.DCMI_Handle);//
}


/* Capture a frame of the image */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    rt_interrupt_enter();
    __HAL_DCMI_ENABLE_IT(&rt_dcmi.DCMI_Handle, DCMI_IT_FRAME);
    rt_dcmi_stop();
    if(NULL != dcmi_irq_callback)
    {
       (*dcmi_irq_callback)();      
    }
    rt_interrupt_leave();
}

void DMA2_Stream1_IRQHandler(void)
{
    extern void camera_dma_data_process(void);
    /* enter interrupt */
    rt_interrupt_enter();

    if (__HAL_DMA_GET_FLAG(&hdma_dcmi, DMA_FLAG_TCIF1_5) != RESET)
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_dcmi, DMA_FLAG_TCIF1_5);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}

static rt_err_t rt_dcmi_init(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);
    rt_err_t result = RT_EOK;
    result = rt_hw_dcmi_init(&rt_dcmi.DCMI_Handle);
    if (result != RT_EOK)
    {
        return result;
    }

    rt_hw_dcmi_dma_init();

    return result;
}

static rt_err_t rt_dcmi_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_dcmi_close(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_dcmi_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_dcmi_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_dcmi_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

rt_err_t rt_set_irq_dcmi_callback_hander(void (*p)(void))
{
    if(NULL == p)
    {
        LOG_E("set irq dcmi callback hander is NULL");
        return RT_ERROR;
    }
    dcmi_irq_callback = p;
    return RT_EOK;
    
}
int dcmi_init(void)
{
    int ret = 0;
    rt_device_t dcmi_dev = RT_NULL;
    rt_dcmi.dev.parent.type      = RT_Device_Class_Miscellaneous;
    rt_dcmi.dev.parent.init      = rt_dcmi_init;
    rt_dcmi.dev.parent.open      = rt_dcmi_open;
    rt_dcmi.dev.parent.close     = rt_dcmi_close;
    rt_dcmi.dev.parent.read      = rt_dcmi_read;
    rt_dcmi.dev.parent.write     = rt_dcmi_write;
    rt_dcmi.dev.parent.control   = rt_dcmi_control;
    rt_dcmi.dev.parent.user_data = RT_NULL;
    ret = rt_device_register(&rt_dcmi.dev.parent, "dcmi", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
    if(ret != RT_EOK)
    {
        LOG_E("dcmi registered fail!!\n\r");
        return -RT_ERROR;
    }
    LOG_I("dcmi registered successfully!");
    dcmi_dev = rt_device_find("dcmi");
    if (dcmi_dev == RT_NULL)
    {
        LOG_E("can't find dcmi device!");
        return RT_ERROR;
    }
    ret = rt_device_open(dcmi_dev, RT_DEVICE_FLAG_RDWR);
    if(ret != RT_EOK)
    {
        LOG_E("can't open dcmi device!");
        return RT_ERROR;
    }
    LOG_I("dcmi open successfully");
    return RT_EOK;
}
INIT_BOARD_EXPORT(dcmi_init);
#endif
