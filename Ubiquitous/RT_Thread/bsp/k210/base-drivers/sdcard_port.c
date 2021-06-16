/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#ifdef BSP_USING_SDCARD
#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>

#define DBG_TAG "sdcard"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

int sd_mount(void)
{
    int ret = 0;
    ret = msd_init("sd0", "spi10");
    if(RT_EOK == ret)
    {
         if(dfs_mount("sd0", "/", "elm", 0, 0) == 0)
        {
            LOG_I("Mount /sd0  successfully"); 
            return RT_EOK;
        }
        else
        { 
            LOG_E("Mount fail !!1");
            return -1;
        }
    }
    LOG_E("msd_init fail !!!");
    return -2;    
}
INIT_ENV_EXPORT(sd_mount);
#endif
#endif
