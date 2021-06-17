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

#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#define LED_G   12

int main(void)
{
    rt_pin_mode(LED_G, PIN_MODE_OUTPUT);
    rt_thread_mdelay(100);
    char info1[25] ={0};
    char info2[25] ={0};
    sprintf(info1,"xuos-intelligence k210 build ");
    sprintf(info2,"%s %s",__DATE__,__TIME__);
    printf("%s %s \n",info1,info2); 
    #ifdef BSP_USING_LCD
    #include<drv_lcd.h>
    lcd_clear(PINK); 
    lcd_draw_string(70,100,info1,BLACK);
    lcd_draw_string(70,120,info2,BLACK);
    #endif
    while(1)
    {
        rt_pin_write(LED_G, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_G, PIN_LOW);
        rt_thread_mdelay(500);
    }
    return 0;
}
