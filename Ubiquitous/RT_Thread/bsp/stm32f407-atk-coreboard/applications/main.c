/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <board.h>
#include <stdio.h>

#define LED0_PIN    GET_PIN(F, 9)

int main(void)
{
     int count = 1;
     rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
     rt_thread_mdelay(100);
     printf("XIUOS stm32f4 build %s %s\n",__DATE__,__TIME__);          
     while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
