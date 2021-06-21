#include <rtthread.h>
#include <rtdevice.h>
#include "stdio.h"
#include "string.h"
#include "dvp.h"
#include "fpioa.h"
#include "plic.h"
#include "sysctl.h"
#if(defined DRV_USING_OV2640 && defined BSP_USING_LCD)
#include<drv_ov2640.h>
#define RGB_BUF_SIZE (320*240*2)
#ifdef RT_USING_POSIX
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <dfs_poll.h>
#ifdef RT_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif
#endif
static int g_fd = 0;
static _ioctl_shoot_para shoot_para_t = {0};


void ov2640_test(int argc, char **argv)
{
    extern void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr);
 
    g_fd = open("/dev/ov2640",O_RDONLY);
    if(g_fd < 0)
    {
        printf("open ov2640 fail !!");
        return;
    }
    if (argc < 2)
    {
        printf("Usage:ov2640 display images in real time or take photos \n");
        printf("Like: ov2640_test 1(take photos )\n");
        printf("Like: ov2640_test 0(display images in real time )\n");
        close(g_fd);
        return ;
    }
    uint32_t* rgbbuffer = rt_malloc(RGB_BUF_SIZE);
    rt_thread_t tid;
    rt_err_t ret = 0;
    int temf = 0;
    if(NULL == rgbbuffer)
    {
        printf("malloc rgbbuffer failed ! \n");
        close(g_fd);
        return;
    }
    temf = strtoul(argv[1],0, 10);
    printf("ov2640_test choose %d mode \n",temf);
    shoot_para_t.pdata = (uint32_t)(rgbbuffer);
    shoot_para_t.length = RGB_BUF_SIZE;
    if(temf == 0)
    {
        void lcd_show_ov2640_thread(uint32_t* rgbbuffer);
        tid = rt_thread_create("lcdshow", lcd_show_ov2640_thread, rgbbuffer,3000, 9, 20);
        rt_thread_startup(tid);
    }
    else
    {
        memset(rgbbuffer,0,320*240*2);
        ret = ioctl(g_fd,IOCTRL_CAMERA_START_SHOT,&shoot_para_t);
//      ret = rt_ov2640_start_shoot((uint32_t)(rgbbuffer), RGB_BUF_SIZE);
        if(RT_ERROR == ret)
        {
            printf("ov2640 can't wait event flag");
            close(g_fd);
            return;
        }
        lcd_draw_picture(0, 0, 320, 240, rgbbuffer);
        rt_thread_mdelay(100);
        printf("the lcd has shown the image \n");
        rt_free(rgbbuffer);
        close(g_fd);
    }
}
MSH_CMD_EXPORT(ov2640_test,lcd show camera shot image);

void lcd_show_ov2640_thread(uint32_t* rgbbuffer)
{
    extern void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr);
    rt_err_t ret = 0;
    while(1)
    {
    
        ret = ioctl(g_fd,IOCTRL_CAMERA_START_SHOT,&shoot_para_t);
        if(RT_ERROR == ret)
        {
            printf("ov2640 can't wait event flag");
            rt_free(rgbbuffer);
            return;
        }
        lcd_draw_picture(0, 0, 320, 240, rgbbuffer); 
        rt_thread_mdelay(2);
    }
    
}
     

#endif


