/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-01-27     tianchunyu        the first version
 */


#include <rtthread.h>
#include <stdio.h>

#ifdef BSP_USING_DVP 
#include <drv_dvp.h>
#define DRV_DEBUG
#define LOG_TAG     "drv.dvp"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>

static struct kendryte_dvp rt_dvp = {0};
static void (*dvp_irq_callback)(void) = NULL;
/*
the camera starts transfering photos
*/

static int on_irq_dvp(void* ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        rt_dvp_stop();
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);        
        (*dvp_irq_callback)();
    }
    return 0;
}


void rt_dvp_start(uint32_t pData, uint32_t Length)
{   
    dvp_set_display_addr(pData);
    dvp_config_interrupt(DVP_CFG_FINISH_INT_ENABLE, 1);
    dvp_start_convert();
}

/*
the camera stops transfering photos
*/
void rt_dvp_stop(void)
{
    dvp_config_interrupt(DVP_CFG_FINISH_INT_ENABLE, 0);
}


static rt_err_t rt_dvp_init(rt_device_t dev)
{
    //sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    RT_ASSERT(dev != RT_NULL);
    rt_err_t result = RT_EOK;
    /* Init DVP IO map and function settings  io pin serial number depends on schematic diagram 
    initialize io in io_config_init function*/
    /*ov2640 dvp interface initialize*/
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);////////////////
    dvp_set_image_size(320, 240);
    dvp_config_interrupt(DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);
    dvp_clear_interrupt(DVP_STS_FRAME_FINISH); 
    LOG_I("dvp initialize success");
    return result;
}

static rt_err_t rt_dvp_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_dvp_close(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_dvp_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_dvp_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_dvp_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

rt_err_t rt_set_irq_dvp_callback_hander(void (*p)(void))
{
    if(NULL == p)
    {
        LOG_E("set irq dcmi callback hander is NULL");
        return RT_ERROR;
    }
    dvp_irq_callback = p;
    return RT_EOK;
    
}
int kendryte_dvp_init(void)
{
    int ret = 0;
    rt_device_t dvp_dev = RT_NULL;
    rt_dvp.dev.parent.type      = RT_Device_Class_Miscellaneous;
    rt_dvp.dev.parent.init      = rt_dvp_init;
    rt_dvp.dev.parent.open      = rt_dvp_open;
    rt_dvp.dev.parent.close     = rt_dvp_close;
    rt_dvp.dev.parent.read      = rt_dvp_read;
    rt_dvp.dev.parent.write     = rt_dvp_write;
    rt_dvp.dev.parent.control   = rt_dvp_control;
    rt_dvp.dev.parent.user_data = RT_NULL;
    ret = rt_device_register(&rt_dvp.dev.parent, "dvp", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
    if(ret != RT_EOK)
    {
        LOG_E("dvp register fail!!\n\r");
        return -RT_ERROR;
    }
    LOG_I("dvp register successfully");
     dvp_dev = rt_device_find("dvp");
    if (dvp_dev == RT_NULL)
    {
        LOG_E("can't find dvp device!");
        return RT_ERROR;
    }
    ret = rt_device_open(dvp_dev, RT_DEVICE_FLAG_RDWR);
    if(ret != RT_EOK)
    {
        LOG_E("can't open dvp device!");
        return RT_ERROR;
    }
    LOG_I("dvp open successfully");
    return RT_EOK;
}
INIT_BOARD_EXPORT(kendryte_dvp_init);
#endif





