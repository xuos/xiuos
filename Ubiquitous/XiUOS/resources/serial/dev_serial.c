/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-13     bernard      first version
 * 2012-05-15     lgnq         modified according bernard's implementation.
 * 2012-05-28     bernard      code cleanup
 * 2012-11-23     bernard      fix compiler warning.
 * 2013-02-20     bernard      use RT_SERIAL_RB_BUFSZ to define
 *                             the size of ring buffer.
 * 2014-07-10     bernard      rewrite serial framework
 * 2014-12-31     bernard      use open_flag for poll_tx stream mode.
 * 2015-05-19     Quintin      fix DMA tx mod tx_dma->activated flag !=RT_FALSE BUG
 *                             in open function.
 * 2015-11-10     bernard      fix the poll rx issue when there is no data.
 * 2016-05-10     armink       add fifo mode to DMA rx when serial->config.bufsz != 0.
 * 2017-01-19     aubr.cool    prevent change serial rx bufsz when serial is opened.
 * 2017-11-07     JasonJia     fix data bits error issue when using tcsetattr.
 * 2017-11-15     JasonJia     fix poll rx issue when data is full.
 *                             add TCFLSH and FIONREAD support.
 * 2018-12-08     Ernest Chen  add DMA choice
 * 2020-09-14     WillianChan  add a line feed to the carriage return character
 *                             when using interrupt tx
 */

/**
* @file dev_serial.c
* @brief register serial dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

/*************************************************
File name: dev_serial.c
Description: support serial dev INT and DMA configure、transfer data
Others: take RT-Thread v4.0.2/components/driver/serial/serial.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-24
Author: AIIT XUOS Lab
Modification: 
1. support serial dev register, configure, write and read
2. add bus driver framework support, include INT and DMA mode
*************************************************/

#include <bus_serial.h>
#include <dev_serial.h>

static DoubleLinklistType serialdev_linklist;

static int SerialWorkModeCheck(struct SerialDevParam *serial_dev_param)
{
    if (SIGN_OPER_INT_TX & serial_dev_param->serial_set_mode) {
        if (SIGN_OPER_INT_TX & serial_dev_param->serial_work_mode) {
            return EOK;
        } else {
            KPrintf("SerialWorkModeCheck set mode 0x%x work mode error 0x%x\n", 
                serial_dev_param->serial_set_mode, serial_dev_param->serial_work_mode);
            return ERROR;
        }
    } else if (SIGN_OPER_INT_RX & serial_dev_param->serial_set_mode) {
        if (SIGN_OPER_INT_RX & serial_dev_param->serial_work_mode) {
            return EOK;
        } else {
            KPrintf("SerialWorkModeCheck set mode 0x%x work mode error 0x%x\n", 
                serial_dev_param->serial_set_mode, serial_dev_param->serial_work_mode);
            return ERROR;
        }
    } else if (SIGN_OPER_DMA_TX & serial_dev_param->serial_set_mode) {
        if (SIGN_OPER_DMA_TX & serial_dev_param->serial_work_mode) {
            return EOK;
        } else {
            KPrintf("SerialWorkModeCheck set mode 0x%x work mode error 0x%x\n", 
                serial_dev_param->serial_set_mode, serial_dev_param->serial_work_mode);
            return ERROR;
        }
    } else if (SIGN_OPER_DMA_RX & serial_dev_param->serial_set_mode) {
        if (SIGN_OPER_DMA_RX & serial_dev_param->serial_work_mode) {
            return EOK;
        } else {
            KPrintf("SerialWorkModeCheck set mode 0x%x work mode error 0x%x\n", 
                serial_dev_param->serial_set_mode, serial_dev_param->serial_work_mode);
            return ERROR;
        }
    } else {
        serial_dev_param->serial_set_mode = serial_dev_param->serial_work_mode;
        return EOK;
    }
}

static inline int SerialDevIntWrite(struct SerialHardwareDevice *serial_dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(write_param);

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    const uint8 *write_data = (const uint8 *)write_param->buffer;
    x_size_t write_length = write_param->size;

    while (write_length)
    {
        if (EOK != hwdev_done->put_char(serial_dev, *(char *)write_data)) {
            KSemaphoreObtain(serial_dev->serial_fifo.serial_tx->serial_txfifo_sem, WAITING_FOREVER);
            continue;
        }

        KPrintf("SerialDevIntWrite data %d write_length %u\n", *(char *)write_data, write_length);

        write_data++; 
        write_length--;
    }

    return EOK;
}

static inline int SerialDevIntRead(struct SerialHardwareDevice *serial_dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(read_param);

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    uint8 *read_data = (uint8 *)read_param->buffer;
    x_size_t read_length = read_param->size;

    while (read_length)
    {
        uint8 get_char;
        x_base lock;

        lock = CriticalAreaLock();
     
        if (serial_dev->serial_fifo.serial_rx->serial_recv_num == serial_dev->serial_fifo.serial_rx->serial_send_num) {
            if (RET_FALSE == serial_dev->serial_fifo.serial_rx->serial_rx_full) {
                CriticalAreaUnLock(lock);
                break;
            }
        }
        
        get_char = serial_dev->serial_fifo.serial_rx->serial_rx_buffer[serial_dev->serial_fifo.serial_rx->serial_recv_num];
        serial_dev->serial_fifo.serial_rx->serial_recv_num += 1;
        if (serial_dev->serial_fifo.serial_rx->serial_recv_num >= serial_cfg->data_cfg.serial_buffer_size) {
            serial_dev->serial_fifo.serial_rx->serial_recv_num = 0;
        }

        if (RET_TRUE == serial_dev->serial_fifo.serial_rx->serial_rx_full) {
            serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_FALSE;
        }

        CriticalAreaUnLock(lock);

        *read_data = get_char;
        read_data++; 
        read_length--;
        read_param->read_length++;
    }

    return EOK;
}

#ifdef SERIAL_USING_DMA
static inline int SerialDevDMAWrite(struct SerialHardwareDevice *serial_dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(write_param);

    x_err_t ret = EOK;
    x_base lock;

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    const uint8 *write_data = (const uint8 *)write_param->buffer;
    x_size_t write_length = write_param->size;

    ret = ((DataQueueDoneType*)serial_dev->serial_fifo.serial_tx->serial_dma_queue.done)->PushDataqueue((DataQueueType *)serial_dev->serial_fifo.serial_tx->serial_dma_queue.property, write_param->buffer, write_param->size, WAITING_FOREVER);
    if (EOK != ret) {
        KUpdateExstatus(ret);
        return ERROR;
    }

    lock = CriticalAreaLock();
    if (RET_FALSE == serial_dev->serial_fifo.serial_tx->serial_dma_enable) {
        serial_dev->serial_fifo.serial_tx->serial_dma_enable = RET_TRUE;
        CriticalAreaUnLock(lock);

        hwdev_done->dmatransfer(serial_dev, (uint8 *)write_data, write_length, SERIAL_DMA_TX);
    } else {
        CriticalAreaUnLock(lock);
    }

    return EOK;
}

static x_size_t SerialGetRxFifoLength(struct SerialHardwareDevice *serial_dev)
{
    NULL_PARAM_CHECK(serial_dev);

    x_size_t length;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;  

    if (serial_dev->serial_fifo.serial_rx->serial_recv_num == serial_dev->serial_fifo.serial_rx->serial_send_num) {
        if (serial_dev->serial_fifo.serial_rx->serial_rx_full) {
            length = serial_cfg->data_cfg.serial_buffer_size;
        } else {
            length = 0;
        }
    } else {
        if (serial_dev->serial_fifo.serial_rx->serial_recv_num > serial_dev->serial_fifo.serial_rx->serial_send_num) {
            length = serial_cfg->data_cfg.serial_buffer_size - serial_dev->serial_fifo.serial_rx->serial_recv_num + serial_dev->serial_fifo.serial_rx->serial_send_num;
        } else {
            length = serial_dev->serial_fifo.serial_rx->serial_send_num - serial_dev->serial_fifo.serial_rx->serial_recv_num;
        }
    }
}

static void SerialDmaRxSetRecvLength(struct SerialHardwareDevice *serial_dev, x_size_t length)
{
    CHECK(length <= SerialGetRxFifoLength(serial_dev));

    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;

    if ((serial_dev->serial_fifo.serial_rx->serial_rx_full) && (length)) {
        serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_FALSE;
    }

    serial_dev->serial_fifo.serial_rx->serial_recv_num += length;

    if (serial_dev->serial_fifo.serial_rx->serial_recv_num >= serial_cfg->data_cfg.serial_buffer_size) {
        serial_dev->serial_fifo.serial_rx->serial_recv_num %= serial_cfg->data_cfg.serial_buffer_size;
    }
}

static void SerialDmaRxSetSendLength(struct SerialHardwareDevice *serial_dev, x_size_t length)
{
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
    
    if (serial_dev->serial_fifo.serial_rx->serial_recv_num > serial_dev->serial_fifo.serial_rx->serial_send_num) {
        serial_dev->serial_fifo.serial_rx->serial_send_num += length;
        if (serial_dev->serial_fifo.serial_rx->serial_recv_num <= serial_dev->serial_fifo.serial_rx->serial_send_num) {
            if (serial_dev->serial_fifo.serial_rx->serial_send_num >= serial_cfg->data_cfg.serial_buffer_size) {
                serial_dev->serial_fifo.serial_rx->serial_send_num %= serial_cfg->data_cfg.serial_buffer_size;
            }
        
            serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_TRUE;
        }
    } else {
        serial_dev->serial_fifo.serial_rx->serial_send_num += length;
        if (serial_dev->serial_fifo.serial_rx->serial_send_num >= serial_cfg->data_cfg.serial_buffer_size) {
            serial_dev->serial_fifo.serial_rx->serial_send_num %= serial_cfg->data_cfg.serial_buffer_size;

            if (serial_dev->serial_fifo.serial_rx->serial_send_num >= serial_dev->serial_fifo.serial_rx->serial_recv_num) {
                serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_TRUE;
            }
        }
    }
    
    if (RET_TRUE == serial_dev->serial_fifo.serial_rx->serial_rx_full) {
        serial_dev->serial_fifo.serial_rx->serial_recv_num = serial_dev->serial_fifo.serial_rx->serial_send_num;
    }
}

static inline int SerialDevDMARead(struct SerialHardwareDevice *serial_dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(read_param);

    x_err_t ret = EOK;
    x_base lock;

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;    
    uint8 *read_data = (uint8 *)read_param->buffer;
    x_size_t read_length = read_param->size;
    x_size_t read_dma_length;
    x_size_t read_dma_size = SerialGetRxFifoLength(serial_dev);
    
    if (serial_cfg->data_cfg.serial_buffer_size) {
        if(read_length < (int)read_dma_size)
            read_dma_length = read_length;
        else
            read_dma_length = read_dma_size;

        if (serial_dev->serial_fifo.serial_rx->serial_recv_num + read_dma_length < serial_cfg->data_cfg.serial_buffer_size) {
            memcpy(read_data, 
                serial_dev->serial_fifo.serial_rx->serial_rx_buffer + serial_dev->serial_fifo.serial_rx->serial_recv_num, read_dma_length);
        } else {
            memcpy(read_data, serial_dev->serial_fifo.serial_rx->serial_rx_buffer + serial_dev->serial_fifo.serial_rx->serial_recv_num,
                serial_cfg->data_cfg.serial_buffer_size - serial_dev->serial_fifo.serial_rx->serial_recv_num);
            memcpy(read_data + serial_cfg->data_cfg.serial_buffer_size - serial_dev->serial_fifo.serial_rx->serial_recv_num, 
                serial_dev->serial_fifo.serial_rx->serial_rx_buffer, read_dma_length + serial_dev->serial_fifo.serial_rx->serial_recv_num - serial_cfg->data_cfg.serial_buffer_size);
        }
        SerialDmaRxSetRecvLength(serial_dev, read_dma_length);
        CriticalAreaUnLock(lock);
        return EOK;
    } else {
        if (RET_FALSE == serial_dev->serial_fifo.serial_rx->serial_dma_enable) {
            serial_dev->serial_fifo.serial_rx->serial_dma_enable = RET_TRUE;
            hwdev_done->dmatransfer(serial_dev, read_data, read_length, SERIAL_DMA_RX);
        } else {
            ret = ERROR;
            KUpdateExstatus(ret);
        }

        CriticalAreaUnLock(lock);
        return ret;
    }
}
#endif

static inline int SerialDevPollingWrite(struct SerialHardwareDevice *serial_dev, struct BusBlockWriteParam *write_param, uint16 serial_stream_mode)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(write_param);

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    const uint8 *write_data = (const uint8 *)write_param->buffer;
    x_size_t write_length = write_param->size;
    
    while (write_length)
    {
        if ((*write_data == '\n') && (SIGN_OPER_STREAM == serial_stream_mode)) {
            hwdev_done->put_char(serial_dev, '\r');
        }

        hwdev_done->put_char(serial_dev, *write_data);

        ++write_data;
        --write_length;
    }

    return EOK;
}

static inline int SerialDevPollingRead(struct SerialHardwareDevice *serial_dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(serial_dev);
    NULL_PARAM_CHECK(read_param);

    struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
    uint8 *read_data = (uint8 *)read_param->buffer;
    x_size_t read_length = read_param->size;

    uint8 get_char;

    while (read_length)
    {
        get_char = hwdev_done->get_char(serial_dev);
        if (-ERROR == get_char) {
            break;
        }

        *read_data = get_char;
        read_data++; 
        read_length--;

        if ('\n' == get_char) {
            break;
        }
    }

    return EOK;
}

static uint32 SerialDevOpen(void *dev)
{
    NULL_PARAM_CHECK(dev);

    int serial_operation_cmd;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
    struct Driver *drv = serial_dev->haldev.owner_bus->owner_driver;
    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;

    if (EOK != SerialWorkModeCheck(serial_dev_param)) {
        KPrintf("SerialDevOpen error!\n");
        return ERROR;
    }
 
    if (NONE == serial_dev->serial_fifo.serial_rx) { 
        if (SIGN_OPER_INT_RX & serial_dev_param->serial_set_mode) {
            serial_dev->serial_fifo.serial_rx = (struct SerialRx *)malloc(sizeof(struct SerialRx));
            if (NONE == serial_dev->serial_fifo.serial_rx) {   
                KPrintf("SerialDevOpen malloc serial_rx error\n");
                free(serial_dev->serial_fifo.serial_rx);
                return ERROR;
            }

            serial_dev->serial_fifo.serial_rx->serial_rx_buffer = (uint8 *)malloc(serial_cfg->data_cfg.serial_buffer_size);
            if (NONE == serial_dev->serial_fifo.serial_rx->serial_rx_buffer) {   
                KPrintf("SerialDevOpen malloc serial_rx_buffer error\n");
                free(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
                free(serial_dev->serial_fifo.serial_rx);
                return ERROR;
            }

            memset(serial_dev->serial_fifo.serial_rx->serial_rx_buffer, 0, serial_cfg->data_cfg.serial_buffer_size);
            serial_dev->serial_fifo.serial_rx->serial_send_num = 0;
            serial_dev->serial_fifo.serial_rx->serial_recv_num = 0;
            serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_FALSE;
            serial_dev_param->serial_work_mode |= SIGN_OPER_INT_RX;
            
            serial_operation_cmd = OPER_SET_INT;
            serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);
        }
#ifdef SERIAL_USING_DMA        
        else if (SIGN_OPER_DMA_RX & serial_dev_param->serial_set_mode) {
            if (0 == serial_cfg->data_cfg.serial_buffer_size) {
                serial_dev->serial_fifo.serial_rx = (struct SerialRx *)malloc(sizeof(struct SerialRx));
                if (NONE == serial_dev->serial_fifo.serial_rx) {   
                    KPrintf("SerialDevOpen DMA buffer 0 malloc serial_rx error\n");
                    free(serial_dev->serial_fifo.serial_rx);
                    return ERROR;
                }
                serial_dev->serial_fifo.serial_rx->serial_dma_enable = RET_FALSE;
                serial_dev_param->serial_work_mode |= SIGN_OPER_DMA_RX;
            } else {
                serial_dev->serial_fifo.serial_rx = (struct SerialRx *)malloc(sizeof(struct SerialRx));
                if (NONE == serial_dev->serial_fifo.serial_rx) {   
                    KPrintf("SerialDevOpen DMA malloc serial_rx error\n");
                    free(serial_dev->serial_fifo.serial_rx);
                    return ERROR;
                }

                serial_dev->serial_fifo.serial_rx->serial_rx_buffer = (uint8 *)malloc(serial_cfg->data_cfg.serial_buffer_size);
                if (NONE == serial_dev->serial_fifo.serial_rx->serial_rx_buffer) {   
                    KPrintf("SerialDevOpen DMA malloc serial_rx_buffer error\n");
                    free(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
                    free(serial_dev->serial_fifo.serial_rx);
                    return ERROR;
                }

                memset(serial_dev->serial_fifo.serial_rx->serial_rx_buffer, 0, serial_cfg->data_cfg.serial_buffer_size);
                serial_dev->serial_fifo.serial_rx->serial_send_num = 0;
                serial_dev->serial_fifo.serial_rx->serial_recv_num = 0;
                serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_FALSE;
                serial_dev_param->serial_work_mode |= SIGN_OPER_DMA_RX;

                int serial_dma_operation = OPER_CONFIG;
                serial_drv->drv_done->configure(serial_drv, serial_dma_operation);
            }
        }
#endif 
        else {
            serial_dev->serial_fifo.serial_rx = NONE;
        }
    } else {
        if (SIGN_OPER_INT_RX & serial_dev_param->serial_set_mode) {
            serial_dev_param->serial_work_mode |= SIGN_OPER_INT_RX;
        }
#ifdef SERIAL_USING_DMA
        else if (SIGN_OPER_DMA_RX & serial_dev_param->serial_set_mode) {
            serial_dev_param->serial_work_mode |= SIGN_OPER_DMA_RX;
        }
#endif 
    }

    if (NONE == serial_dev->serial_fifo.serial_tx) {
        if (SIGN_OPER_INT_TX & serial_dev_param->serial_set_mode) {
            serial_dev->serial_fifo.serial_tx = (struct SerialTx *)malloc(sizeof(struct SerialTx));
            if (NONE == serial_dev->serial_fifo.serial_tx) {   
                KPrintf("SerialDevOpen malloc serial_tx error\n");
                free(serial_dev->serial_fifo.serial_tx);
                return ERROR;
            }

            serial_dev->serial_fifo.serial_tx->serial_txfifo_sem = KSemaphoreCreate(0);
            serial_dev_param->serial_work_mode |= SIGN_OPER_INT_TX;

            serial_operation_cmd = OPER_SET_INT;
            serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);
        }
#ifdef SERIAL_USING_DMA
        else if (SIGN_OPER_DMA_TX & serial_dev_param->serial_set_mode) {
            serial_dev->serial_fifo.serial_tx = (struct SerialTx *)malloc(sizeof(struct SerialTx));
            if (NONE == serial_dev->serial_fifo.serial_tx) {   
                KPrintf("SerialDevOpen DMA malloc serial_tx error\n");
                free(serial_dev->serial_fifo.serial_tx);
                return ERROR;
            }

            serial_dev->serial_fifo.serial_tx->serial_dma_enable = RET_FALSE;
            serial_dev->serial_fifo.serial_tx->serial_dma_queue.done = g_queue_done[DATA_QUEUE];
            serial_dev->serial_fifo.serial_tx->serial_dma_queue.property = x_malloc(sizeof(DataQueueType));
            ((DataQueueDoneType*)serial_dev->serial_fifo.serial_tx->serial_dma_queue.done)->InitDataqueue((DataQueueType *)serial_dev->serial_fifo.serial_tx->serial_dma_queue.property, 8);
        
            serial_dev_param->serial_work_mode |= SIGN_OPER_DMA_TX;
            serial_operation_cmd = OPER_CONFIG;
            serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);        
        }
#endif 
        else {
            serial_dev->serial_fifo.serial_tx = NONE;
        }
    } else {
        if (SIGN_OPER_INT_TX & serial_dev_param->serial_set_mode) {
            serial_dev_param->serial_work_mode |= SIGN_OPER_INT_TX;
        }
#ifdef SERIAL_USING_DMA
        else if (SIGN_OPER_DMA_TX & serial_dev_param->serial_set_mode) {
            serial_dev_param->serial_work_mode |= SIGN_OPER_DMA_TX;
        }
#endif 
    }

    serial_dev->haldev.dev_sem = KSemaphoreCreate(0);
	if (serial_dev->haldev.dev_sem < 0) {
		KPrintf("SerialDevOpen create sem failed .\n");

        if (serial_dev->serial_fifo.serial_rx->serial_rx_buffer) {
            free(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
        }
        if (serial_dev->serial_fifo.serial_rx) {
            free(serial_dev->serial_fifo.serial_rx);
        }
        if (serial_dev->serial_fifo.serial_tx) {
            free(serial_dev->serial_fifo.serial_tx);
        }
    
		return ERROR;
	}

    return EOK;
}

static uint32 SerialDevClose(void *dev)
{
    NULL_PARAM_CHECK(dev);

    int serial_operation_cmd = OPER_CLR_INT;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
    struct Driver *drv = serial_dev->haldev.owner_bus->owner_driver;
    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;
    struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;

    if (SIGN_OPER_INT_RX & serial_dev_param->serial_work_mode) {
        NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
        NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_rx);
        free(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
        free(serial_dev->serial_fifo.serial_rx);

        serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);
    }
#ifdef SERIAL_USING_DMA
    else if (SIGN_OPER_DMA_RX & serial_dev_param->serial_work_mode) {
        if(0 == serial_cfg->data_cfg.serial_buffer_size)
        {
            NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_rx);
            free(serial_dev->serial_fifo.serial_rx);
        } else {
            NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
            NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_rx);
            free(serial_dev->serial_fifo.serial_rx->serial_rx_buffer);
            free(serial_dev->serial_fifo.serial_rx);
        }

        serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);
    }
#endif 
    
    if (SIGN_OPER_INT_TX & serial_dev_param->serial_work_mode) {
        NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_tx);
        free(serial_dev->serial_fifo.serial_tx);

        serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);    
    }
#ifdef SERIAL_USING_DMA
    else if (SIGN_OPER_DMA_TX & serial_dev_param->serial_work_mode) {
        NULL_PARAM_CHECK(serial_dev->serial_fifo.serial_tx);
        free(serial_dev->serial_fifo.serial_tx);

        serial_drv->drv_done->configure(serial_drv, serial_operation_cmd);
    }
#endif 

    KSemaphoreDelete(serial_dev->haldev.dev_sem);
    return EOK;
}

static uint32 SerialDevWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);

    x_err_t ret = EOK;

    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;

    if (serial_dev_param->serial_work_mode & SIGN_OPER_INT_TX) {
        ret = SerialDevIntWrite(serial_dev, write_param);
        if (EOK != ret) {
            KPrintf("SerialDevIntWrite error %d\n", ret);
            return ERROR;
        }
    }
#ifdef SERIAL_USING_DMA
    else if (serial_dev_param->serial_work_mode & SIGN_OPER_DMA_TX) {
        ret = SerialDevDMAWrite(serial_dev, write_param);
        if (EOK != ret) {
            KPrintf("SerialDevDMAWrite error %d\n", ret);
            return ERROR;
        }
    }
#endif        
    else {
        ret = SerialDevPollingWrite(serial_dev, write_param, serial_dev_param->serial_stream_mode);
        if (EOK != ret) {
            KPrintf("SerialDevPollingWrite error %d\n", ret);
            return ERROR;
        }
    }

    return EOK;
}

static uint32 SerialDevRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);

    x_err_t ret = EOK;

    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;

    if (EOK == KSemaphoreObtain(serial_dev->haldev.dev_sem, WAITING_FOREVER)) {
        if (serial_dev_param->serial_work_mode & SIGN_OPER_INT_RX) {
            ret = SerialDevIntRead(serial_dev, read_param);
            if (EOK != ret) {
                KPrintf("SerialDevIntRead error %d\n", ret);
                return ERROR;
            }
        }
    #ifdef SERIAL_USING_DMA
        else if (serial_dev_param->serial_work_mode & SIGN_OPER_DMA_RX) {
            ret = SerialDevDMARead(serial_dev, read_param);
            if (EOK != ret) {
                KPrintf("SerialDevDMARead error %d\n", ret);
                return ERROR;
            }
        }
    #endif        
        else {
            ret = SerialDevPollingRead(serial_dev, read_param);
            if (EOK != ret) {
                KPrintf("SerialDevPollingRead error %d\n", ret);
                return ERROR;
            }
        }
    }
    return EOK;
}

void SerialSetIsr(struct SerialHardwareDevice *serial_dev, int event)
{
    switch (event & 0xff)
    {
        case SERIAL_EVENT_RX_IND:
        {
            int get_char;
            x_base lock;

            struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;
            struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;

            while (1)
            {
                get_char = hwdev_done->get_char(serial_dev);
                if (-ERROR == get_char) {
                    break;
                }

                lock = CriticalAreaLock();

                serial_dev->serial_fifo.serial_rx->serial_rx_buffer[serial_dev->serial_fifo.serial_rx->serial_send_num] = (uint8)get_char;
                serial_dev->serial_fifo.serial_rx->serial_send_num += 1;
                if (serial_dev->serial_fifo.serial_rx->serial_send_num >= serial_cfg->data_cfg.serial_buffer_size) {
                    serial_dev->serial_fifo.serial_rx->serial_send_num = 0;
                }
             
                if (serial_dev->serial_fifo.serial_rx->serial_send_num == serial_dev->serial_fifo.serial_rx->serial_recv_num) {
                    serial_dev->serial_fifo.serial_rx->serial_recv_num += 1;
                    serial_dev->serial_fifo.serial_rx->serial_rx_full = RET_TRUE;
                    if (serial_dev->serial_fifo.serial_rx->serial_recv_num >= serial_cfg->data_cfg.serial_buffer_size) {
                        serial_dev->serial_fifo.serial_rx->serial_recv_num = 0;
                    }
                }
                CriticalAreaUnLock(lock);
            }

            x_size_t serial_rx_length;
                
            lock = CriticalAreaLock();
            if (serial_dev->serial_fifo.serial_rx->serial_recv_num > serial_dev->serial_fifo.serial_rx->serial_send_num) {
               serial_rx_length = serial_cfg->data_cfg.serial_buffer_size - serial_dev->serial_fifo.serial_rx->serial_recv_num + serial_dev->serial_fifo.serial_rx->serial_send_num;
            } else {
                serial_rx_length = serial_dev->serial_fifo.serial_rx->serial_send_num - serial_dev->serial_fifo.serial_rx->serial_recv_num;
            }
            CriticalAreaUnLock(lock);

            if (serial_rx_length) {
                if (serial_dev->haldev.dev_recv_callback) {
                    serial_dev->haldev.dev_recv_callback((void *)serial_dev, serial_rx_length);
                }
                
                KSemaphoreAbandon(serial_dev->haldev.dev_sem);
            }
            break;
        }
        case SERIAL_event_id_tX_DONE:
        {
            KSemaphoreAbandon(serial_dev->serial_fifo.serial_tx->serial_txfifo_sem);
            break;
        }
#ifdef SERIAL_USING_DMA
        case SERIAL_event_id_tX_DMADONE:
        {
            const void *data_ptr;
            x_size_t DataSize;
            const void *last_data_ptr;

            struct SerialHwDevDone *hwdev_done = serial_dev->hwdev_done;

            ((DataQueueDoneType*)serial_dev->serial_fifo.serial_tx->serial_dma_queue.done)->PopDataqueue((DataQueueType *)serial_dev->serial_fifo.serial_tx->serial_dma_queue.property, &last_data_ptr, &DataSize, 0);
            if (EOK == ((DataQueueDoneType*)serial_dev->serial_fifo.serial_tx->serial_dma_queue.done)->DataqueuePeak((DataQueueType *)serial_dev->serial_fifo.serial_tx->serial_dma_queue.property, &data_ptr, &DataSize)) {
                serial_dev->serial_fifo.serial_tx->serial_dma_enable = RET_TRUE;
                hwdev_done->dmatransfer(serial_dev, (uint8 *)data_ptr, DataSize, SERIAL_DMA_TX);
            } else {
                serial_dev->serial_fifo.serial_tx->serial_dma_enable = RET_FALSE;
            }
            break;
        }
        case SERIAL_EVENT_RX_DMADONE:
        {
            int length;
            x_base lock;

            struct SerialCfgParam *serial_cfg = (struct SerialCfgParam *)serial_dev->private_data;
          
            length = (event & (~0xff)) >> 8;

            if (serial_cfg->data_cfg.serial_buffer_size) {
                lock = CriticalAreaLock();
            
                SerialDmaRxSetSendLength(serial_dev, length);
         
                length = SerialGetRxFifoLength(serial_dev);
           
                CriticalAreaUnLock(lock);
                if (serial_dev->haldev.dev_recv_callback) {
                    serial_dev->haldev.dev_recv_callback((void *)serial_dev, length);
                }
                KSemaphoreAbandon(serial_dev->haldev.dev_sem);
            } else {
                serial_dev->serial_fifo.serial_rx->serial_dma_enable = RET_FALSE;
                if (serial_dev->haldev.dev_recv_callback) {
                    serial_dev->haldev.dev_recv_callback((void *)serial_dev, length);
                }
                KSemaphoreAbandon(serial_dev->haldev.dev_sem);
            }
            break;
        }
#endif 
    }
}

const struct SerialDevDone dev_done =
{
    .open = SerialDevOpen,
    .close = SerialDevClose,
    .write = SerialDevWrite,
    .read = SerialDevRead,
};

/*Create the serial device linklist*/
static void SerialDeviceLinkInit()
{
    InitDoubleLinkList(&serialdev_linklist);
}

HardwareDevType SerialDeviceFind(const char *dev_name, enum DevType dev_type)
{
    NULL_PARAM_CHECK(dev_name);
    
    struct HardwareDev *device = NONE;

    DoubleLinklistType *node = NONE;
    DoubleLinklistType *head = &serialdev_linklist;

    for (node = head->node_next; node != head; node = node->node_next) {
        device = SYS_DOUBLE_LINKLIST_ENTRY(node, struct HardwareDev, dev_link);
        if ((!strcmp(device->dev_name, dev_name)) && (dev_type == device->dev_type)) {
            return device;
        }
    }

    KPrintf("SerialDeviceFind cannot find the %s device.return NULL\n", dev_name);
    return NONE;
}

int SerialDeviceRegister(struct SerialHardwareDevice *serial_device, void *serial_param, const char *device_name)
{
    NULL_PARAM_CHECK(serial_device);
    NULL_PARAM_CHECK(device_name);

    x_err_t ret = EOK;    
    static x_bool dev_link_flag = RET_FALSE;

    if (!dev_link_flag) {
        SerialDeviceLinkInit();
        dev_link_flag = RET_TRUE;
    }

    if (DEV_INSTALL != serial_device->haldev.dev_state) {
        strncpy(serial_device->haldev.dev_name, device_name, NAME_NUM_MAX);
        serial_device->haldev.dev_type = TYPE_SERIAL_DEV;
        serial_device->haldev.dev_state = DEV_INSTALL;

        if (serial_device->ext_serial_mode) {
            serial_device->haldev.dev_done = (struct HalDevDone *)serial_device->dev_done;
        } else {
            serial_device->haldev.dev_done = (struct HalDevDone *)&dev_done;
        }
        
        serial_device->private_data = serial_param;

        DoubleLinkListInsertNodeAfter(&serialdev_linklist, &(serial_device->haldev.dev_link));
    } else {
        KPrintf("SerialDeviceRegister device has been register state%u\n", serial_device->haldev.dev_state);        
    }

    return ret;
}

int SerialDeviceAttachToBus(const char *dev_name, const char *bus_name)
{
    NULL_PARAM_CHECK(dev_name);
    NULL_PARAM_CHECK(bus_name);
    
    x_err_t ret = EOK;

    struct Bus *bus;
    struct HardwareDev *device;

    bus = BusFind(bus_name);
    if (NONE == bus) {
        KPrintf("SerialDeviceAttachToBus find serial bus error!name %s\n", bus_name);
        return ERROR;
    }
    
    if (TYPE_SERIAL_BUS == bus->bus_type) {
        device = SerialDeviceFind(dev_name, TYPE_SERIAL_DEV);
        if (NONE == device) {
            KPrintf("SerialDeviceAttachToBus find serial device error!name %s\n", dev_name);
            return ERROR;
        }

        if (TYPE_SERIAL_DEV == device->dev_type) {
            ret = DeviceRegisterToBus(bus, device);
            if (EOK != ret) {
                KPrintf("SerialDeviceAttachToBus DeviceRegisterToBus error %u\n", ret);
                return ERROR;
            }
        }
    }

    return EOK;
}
