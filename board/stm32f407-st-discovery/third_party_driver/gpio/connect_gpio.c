/*
 * Copyright (c) 2020 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-06     balanceTWK        first version
 * 2019-04-23     WillianChan       Fix GPIO serial number disorder
*/

/**
* @file connect_gpio.c
* @brief support gpio function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: connect_gpio.c
Description: support  gpio configure and register to bus framework
Others: take RT-Thread v4.0.2/bsp/stm32/libraries/HAL_Drivers/drv_gpio.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: add bus driver framework support for gpio
*************************************************/

#include <device.h>
#include <board.h>
#include "misc.h"
#include "hardware_rcc.h"
#include "hardware_gpio.h"
#include "hardware_exti.h"
#include "hardware_syscfg.h"
#include "connect_gpio.h"

#define STM32_PIN_NUMBERS 100 // [48, 64, 100, 144]

#define __STM32_PIN(index, rcc, gpio, gpio_index) { 0, RCC_##rcc##Periph_GPIO##gpio, GPIO##gpio, GPIO_Pin_##gpio_index}
#define __STM32_PIN_DEFAULT {-1, 0, 0, 0}

#define ITEM_NUM(items) sizeof(items)/sizeof(items[0])

struct PinIndex
{
    int index;
    uint32_t rcc;
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

struct PinIrq
{
    uint8 port_source;
    uint8 pin_source;
    enum IRQn irq_exti_channel;
    uint32 exti_line;
};

static const struct PinIndex pins[] =
{
#if (STM32_PIN_NUMBERS == 48)
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(2, AHB1, C, 13),
    __STM32_PIN(3, AHB1, C, 14),
    __STM32_PIN(4, AHB1, C, 15),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(10, AHB1, A, 0),
    __STM32_PIN(11, AHB1, A, 1),
    __STM32_PIN(12, AHB1, A, 2),
    __STM32_PIN(13, AHB1, A, 3),
    __STM32_PIN(14, AHB1, A, 4),
    __STM32_PIN(15, AHB1, A, 5),
    __STM32_PIN(16, AHB1, A, 6),
    __STM32_PIN(17, AHB1, A, 7),
    __STM32_PIN(18, AHB1, B, 0),
    __STM32_PIN(19, AHB1, B, 1),
    __STM32_PIN(20, AHB1, B, 2),
    __STM32_PIN(21, AHB1, B, 10),
    __STM32_PIN(22, AHB1, B, 11),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(25, AHB1, B, 12),
    __STM32_PIN(26, AHB1, B, 13),
    __STM32_PIN(27, AHB1, B, 14),
    __STM32_PIN(28, AHB1, B, 15),
    __STM32_PIN(29, AHB1, A, 8),
    __STM32_PIN(30, AHB1, A, 9),
    __STM32_PIN(31, AHB1, A, 10),
    __STM32_PIN(32, AHB1, A, 11),
    __STM32_PIN(33, AHB1, A, 12),
    __STM32_PIN(34, AHB1, A, 13),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(37, AHB1, A, 14),
    __STM32_PIN(38, AHB1, A, 15),
    __STM32_PIN(39, AHB1, B, 3),
    __STM32_PIN(40, AHB1, B, 4),
    __STM32_PIN(41, AHB1, B, 5),
    __STM32_PIN(42, AHB1, B, 6),
    __STM32_PIN(43, AHB1, B, 7),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(45, AHB1, B, 8),
    __STM32_PIN(46, AHB1, B, 9),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,

#endif
#if (STM32_PIN_NUMBERS == 64)
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(2, AHB1, C, 13),
    __STM32_PIN(3, AHB1, C, 14),
    __STM32_PIN(4, AHB1, C, 15),
    __STM32_PIN(5, AHB1, D, 0),
    __STM32_PIN(6, AHB1, D, 1),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(8, AHB1, C, 0),
    __STM32_PIN(9, AHB1, C, 1),
    __STM32_PIN(10, AHB1, C, 2),
    __STM32_PIN(11, AHB1, C, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(14, AHB1, A, 0),
    __STM32_PIN(15, AHB1, A, 1),
    __STM32_PIN(16, AHB1, A, 2),
    __STM32_PIN(17, AHB1, A, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(20, AHB1, A, 4),
    __STM32_PIN(21, AHB1, A, 5),
    __STM32_PIN(22, AHB1, A, 6),
    __STM32_PIN(23, AHB1, A, 7),
    __STM32_PIN(24, AHB1, C, 4),
    __STM32_PIN(25, AHB1, C, 5),
    __STM32_PIN(26, AHB1, B, 0),
    __STM32_PIN(27, AHB1, B, 1),
    __STM32_PIN(28, AHB1, B, 2),
    __STM32_PIN(29, AHB1, B, 10),
    __STM32_PIN(30, AHB1, B, 11),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(33, AHB1, B, 12),
    __STM32_PIN(34, AHB1, B, 13),
    __STM32_PIN(35, AHB1, B, 14),
    __STM32_PIN(36, AHB1, B, 15),
    __STM32_PIN(37, AHB1, C, 6),
    __STM32_PIN(38, AHB1, C, 7),
    __STM32_PIN(39, AHB1, C, 8),
    __STM32_PIN(40, AHB1, C, 9),
    __STM32_PIN(41, AHB1, A, 8),
    __STM32_PIN(42, AHB1, A, 9),
    __STM32_PIN(43, AHB1, A, 10),
    __STM32_PIN(44, AHB1, A, 11),
    __STM32_PIN(45, AHB1, A, 12),
    __STM32_PIN(46, AHB1, A, 13),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(49, AHB1, A, 14),
    __STM32_PIN(50, AHB1, A, 15),
    __STM32_PIN(51, AHB1, C, 10),
    __STM32_PIN(52, AHB1, C, 11),
    __STM32_PIN(53, AHB1, C, 12),
    __STM32_PIN(54, AHB1, D, 2),
    __STM32_PIN(55, AHB1, B, 3),
    __STM32_PIN(56, AHB1, B, 4),
    __STM32_PIN(57, AHB1, B, 5),
    __STM32_PIN(58, AHB1, B, 6),
    __STM32_PIN(59, AHB1, B, 7),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(61, AHB1, B, 8),
    __STM32_PIN(62, AHB1, B, 9),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
#endif
#if (STM32_PIN_NUMBERS == 100)
    __STM32_PIN_DEFAULT,
    __STM32_PIN(1, AHB1, E, 2),
    __STM32_PIN(2, AHB1, E, 3),
    __STM32_PIN(3, AHB1, E, 4),
    __STM32_PIN(4, AHB1, E, 5),
    __STM32_PIN(5, AHB1, E, 6),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(7, AHB1, C, 13),
    __STM32_PIN(8, AHB1, C, 14),
    __STM32_PIN(9, AHB1, C, 15),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(15, AHB1, C, 0),
    __STM32_PIN(16, AHB1, C, 1),
    __STM32_PIN(17, AHB1, C, 2),
    __STM32_PIN(18, AHB1, C, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(23, AHB1, A, 0),
    __STM32_PIN(24, AHB1, A, 1),
    __STM32_PIN(25, AHB1, A, 2),
    __STM32_PIN(26, AHB1, A, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(29, AHB1, A, 4),
    __STM32_PIN(30, AHB1, A, 5),
    __STM32_PIN(31, AHB1, A, 6),
    __STM32_PIN(32, AHB1, A, 7),
    __STM32_PIN(33, AHB1, C, 4),
    __STM32_PIN(34, AHB1, C, 5),
    __STM32_PIN(35, AHB1, B, 0),
    __STM32_PIN(36, AHB1, B, 1),
    __STM32_PIN(37, AHB1, B, 2),
    __STM32_PIN(38, AHB1, E, 7),
    __STM32_PIN(39, AHB1, E, 8),
    __STM32_PIN(40, AHB1, E, 9),
    __STM32_PIN(41, AHB1, E, 10),
    __STM32_PIN(42, AHB1, E, 11),
    __STM32_PIN(43, AHB1, E, 12),
    __STM32_PIN(44, AHB1, E, 13),
    __STM32_PIN(45, AHB1, E, 14),
    __STM32_PIN(46, AHB1, E, 15),
    __STM32_PIN(47, AHB1, B, 10),
    __STM32_PIN(48, AHB1, B, 11),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(51, AHB1, B, 12),
    __STM32_PIN(52, AHB1, B, 13),
    __STM32_PIN(53, AHB1, B, 14),
    __STM32_PIN(54, AHB1, B, 15),
    __STM32_PIN(55, AHB1, D, 8),
    __STM32_PIN(56, AHB1, D, 9),
    __STM32_PIN(57, AHB1, D, 10),
    __STM32_PIN(58, AHB1, D, 11),
    __STM32_PIN(59, AHB1, D, 12),
    __STM32_PIN(60, AHB1, D, 13),
    __STM32_PIN(61, AHB1, D, 14),
    __STM32_PIN(62, AHB1, D, 15),
    __STM32_PIN(63, AHB1, C, 6),
    __STM32_PIN(64, AHB1, C, 7),
    __STM32_PIN(65, AHB1, C, 8),
    __STM32_PIN(66, AHB1, C, 9),
    __STM32_PIN(67, AHB1, A, 8),
    __STM32_PIN(68, AHB1, A, 9),
    __STM32_PIN(69, AHB1, A, 10),
    __STM32_PIN(70, AHB1, A, 11),
    __STM32_PIN(71, AHB1, A, 12),
    __STM32_PIN(72, AHB1, A, 13),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(76, AHB1, A, 14),
    __STM32_PIN(77, AHB1, A, 15),
    __STM32_PIN(78, AHB1, C, 10),
    __STM32_PIN(79, AHB1, C, 11),
    __STM32_PIN(80, AHB1, C, 12),
    __STM32_PIN(81, AHB1, D, 0),
    __STM32_PIN(82, AHB1, D, 1),
    __STM32_PIN(83, AHB1, D, 2),
    __STM32_PIN(84, AHB1, D, 3),
    __STM32_PIN(85, AHB1, D, 4),
    __STM32_PIN(86, AHB1, D, 5),
    __STM32_PIN(87, AHB1, D, 6),
    __STM32_PIN(88, AHB1, D, 7),
    __STM32_PIN(89, AHB1, B, 3),
    __STM32_PIN(90, AHB1, B, 4),
    __STM32_PIN(91, AHB1, B, 5),
    __STM32_PIN(92, AHB1, B, 6),
    __STM32_PIN(93, AHB1, B, 7),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(95, AHB1, B, 8),
    __STM32_PIN(96, AHB1, B, 9),
    __STM32_PIN(97, AHB1, E, 0),
    __STM32_PIN(98, AHB1, E, 1),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
#endif
#if (STM32_PIN_NUMBERS == 144)
    __STM32_PIN_DEFAULT,
    __STM32_PIN(1, AHB1, E, 2),
    __STM32_PIN(2, AHB1, E, 3),
    __STM32_PIN(3, AHB1, E, 4),
    __STM32_PIN(4, AHB1, E, 5),
    __STM32_PIN(5, AHB1, E, 6),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(7, AHB1, C, 13),
    __STM32_PIN(8, AHB1, C, 14),
    __STM32_PIN(9, AHB1, C, 15),

    __STM32_PIN(10, AHB1, F, 0),
    __STM32_PIN(11, AHB1, F, 1),
    __STM32_PIN(12, AHB1, F, 2),
    __STM32_PIN(13, AHB1, F, 3),
    __STM32_PIN(14, AHB1, F, 4),
    __STM32_PIN(15, AHB1, F, 5),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(18, AHB1, F, 6),
    __STM32_PIN(19, AHB1, F, 7),
    __STM32_PIN(20, AHB1, F, 8),
    __STM32_PIN(21, AHB1, F, 9),
    __STM32_PIN(22, AHB1, F, 10),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(26, AHB1, C, 0),
    __STM32_PIN(27, AHB1, C, 1),
    __STM32_PIN(28, AHB1, C, 2),
    __STM32_PIN(29, AHB1, C, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(34, AHB1, A, 0),
    __STM32_PIN(35, AHB1, A, 1),
    __STM32_PIN(36, AHB1, A, 2),
    __STM32_PIN(37, AHB1, A, 3),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(40, AHB1, A, 4),
    __STM32_PIN(41, AHB1, A, 5),
    __STM32_PIN(42, AHB1, A, 6),
    __STM32_PIN(43, AHB1, A, 7),
    __STM32_PIN(44, AHB1, C, 4),
    __STM32_PIN(45, AHB1, C, 5),
    __STM32_PIN(46, AHB1, B, 0),
    __STM32_PIN(47, AHB1, B, 1),
    __STM32_PIN(48, AHB1, B, 2),
    __STM32_PIN(49, AHB1, F, 11),
    __STM32_PIN(50, AHB1, F, 12),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(53, AHB1, F, 13),
    __STM32_PIN(54, AHB1, F, 14),
    __STM32_PIN(55, AHB1, F, 15),
    __STM32_PIN(56, AHB1, G, 0),
    __STM32_PIN(57, AHB1, G, 1),
    __STM32_PIN(58, AHB1, E, 7),
    __STM32_PIN(59, AHB1, E, 8),
    __STM32_PIN(60, AHB1, E, 9),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(63, AHB1, E, 10),
    __STM32_PIN(64, AHB1, E, 11),
    __STM32_PIN(65, AHB1, E, 12),
    __STM32_PIN(66, AHB1, E, 13),
    __STM32_PIN(67, AHB1, E, 14),
    __STM32_PIN(68, AHB1, E, 15),
    __STM32_PIN(69, AHB1, B, 10),
    __STM32_PIN(70, AHB1, B, 11),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(73, AHB1, B, 12),
    __STM32_PIN(74, AHB1, B, 13),
    __STM32_PIN(75, AHB1, B, 14),
    __STM32_PIN(76, AHB1, B, 15),
    __STM32_PIN(77, AHB1, D, 8),
    __STM32_PIN(78, AHB1, D, 9),
    __STM32_PIN(79, AHB1, D, 10),
    __STM32_PIN(80, AHB1, D, 11),
    __STM32_PIN(81, AHB1, D, 12),
    __STM32_PIN(82, AHB1, D, 13),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(85, AHB1, D, 14),
    __STM32_PIN(86, AHB1, D, 15),
    __STM32_PIN(87, AHB1, G, 2),
    __STM32_PIN(88, AHB1, G, 3),
    __STM32_PIN(89, AHB1, G, 4),
    __STM32_PIN(90, AHB1, G, 5),
    __STM32_PIN(91, AHB1, G, 6),
    __STM32_PIN(92, AHB1, G, 7),
    __STM32_PIN(93, AHB1, G, 8),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(96, AHB1, C, 6),
    __STM32_PIN(97, AHB1, C, 7),
    __STM32_PIN(98, AHB1, C, 8),
    __STM32_PIN(99, AHB1, C, 9),
    __STM32_PIN(100, AHB1, A, 8),
    __STM32_PIN(101, AHB1, A, 9),
    __STM32_PIN(102, AHB1, A, 10),
    __STM32_PIN(103, AHB1, A, 11),
    __STM32_PIN(104, AHB1, A, 12),
    __STM32_PIN(105, AHB1, A, 13),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(109, AHB1, A, 14),
    __STM32_PIN(110, AHB1, A, 15),
    __STM32_PIN(111, AHB1, C, 10),
    __STM32_PIN(112, AHB1, C, 11),
    __STM32_PIN(113, AHB1, C, 12),
    __STM32_PIN(114, AHB1, D, 0),
    __STM32_PIN(115, AHB1, D, 1),
    __STM32_PIN(116, AHB1, D, 2),
    __STM32_PIN(117, AHB1, D, 3),
    __STM32_PIN(118, AHB1, D, 4),
    __STM32_PIN(119, AHB1, D, 5),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(122, AHB1, D, 6),
    __STM32_PIN(123, AHB1, D, 7),
    __STM32_PIN(124, AHB1, G, 9),
    __STM32_PIN(125, AHB1, G, 10),
    __STM32_PIN(126, AHB1, G, 11),
    __STM32_PIN(127, AHB1, G, 12),
    __STM32_PIN(128, AHB1, G, 13),
    __STM32_PIN(129, AHB1, G, 14),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
    __STM32_PIN(132, AHB1, G, 15),
    __STM32_PIN(133, AHB1, B, 3),
    __STM32_PIN(134, AHB1, B, 4),
    __STM32_PIN(135, AHB1, B, 5),
    __STM32_PIN(136, AHB1, B, 6),
    __STM32_PIN(137, AHB1, B, 7),
    __STM32_PIN_DEFAULT,
    __STM32_PIN(139, AHB1, B, 8),
    __STM32_PIN(140, AHB1, B, 9),
    __STM32_PIN(141, AHB1, E, 0),
    __STM32_PIN(142, AHB1, E, 1),
    __STM32_PIN_DEFAULT,
    __STM32_PIN_DEFAULT,
#endif
};

struct PinIrqHdr pin_irq_hdr_tab[] =
{
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},
    {-1, 0, NONE, NONE},                  
};
const struct PinIndex *GetPin(uint8_t pin)
{
    const struct PinIndex *index;

    if (pin < ITEM_NUM(pins)){
        index = &pins[pin];
        if (index->index == -1)
            index = NONE;
    }
    else{
        index = NONE;
    }

    return index;
};

static int32 GpioConfigMode(int mode, const struct PinIndex* index)
{
    GPIO_InitTypeDef gpio_initstructure;
    NULL_PARAM_CHECK(index);

    RCC_AHB1PeriphClockCmd(index->rcc, ENABLE);

    gpio_initstructure.GPIO_Pin   = index->pin;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    switch (mode)
    {
        case GPIO_CFG_OUTPUT:
            gpio_initstructure.GPIO_Mode  = GPIO_Mode_OUT;
            gpio_initstructure.GPIO_OType = GPIO_OType_PP;
            break;
        case GPIO_CFG_INPUT:
            gpio_initstructure.GPIO_Mode = GPIO_Mode_IN;
            gpio_initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            break;
        case GPIO_CFG_INPUT_PULLUP:
            gpio_initstructure.GPIO_Mode = GPIO_Mode_IN;
            gpio_initstructure.GPIO_PuPd = GPIO_PuPd_UP;
            break;
        case GPIO_CFG_INPUT_PULLDOWN:
            gpio_initstructure.GPIO_Mode = GPIO_Mode_IN;
            gpio_initstructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            break;
        case GPIO_CFG_OUTPUT_OD:
            gpio_initstructure.GPIO_Mode  = GPIO_Mode_OUT;
            gpio_initstructure.GPIO_OType = GPIO_OType_OD;
            break;
        default:
            break;
    }
    GPIO_Init(index->gpio, &gpio_initstructure);

    return EOK;
}

static __inline int32 Bit2Bitnum(uint32 bit)
{
    for (int i = 0; i < 32; i++){
        if ((1UL << i) == bit){
            return i;
        }
    }
    return -1;
}

static __inline int32 Bitno2Bit(uint32 bitno)
{
    if (bitno <= 32) {
        return 1UL << bitno;
    }
    else {
        return 0;
    }
}
static const struct PinIrq *GetPinIrq(uint16_t pin)
{
    static struct PinIrq irq;
    const struct PinIndex* index = GetPin(pin);

    if (index == NONE) {
        return NONE;
    }

    irq.exti_line = index->pin;
    irq.pin_source = Bit2Bitnum(index->pin);
    irq.port_source = ((uint32_t)index->gpio - GPIOA_BASE) / (GPIOB_BASE - GPIOA_BASE);
    switch (irq.pin_source)
    {
    case 0 : irq.irq_exti_channel = EXTI0_IRQn;break;
    case 1 : irq.irq_exti_channel = EXTI1_IRQn;break;
    case 2 : irq.irq_exti_channel = EXTI2_IRQn;break;
    case 3 : irq.irq_exti_channel = EXTI3_IRQn;break;
    case 4 : irq.irq_exti_channel = EXTI4_IRQn;break;
    case 5 :
    case 6 :
    case 7 :
    case 8 :
    case 9 : irq.irq_exti_channel = EXTI9_5_IRQn;break;
    case 10 :
    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 : irq.irq_exti_channel = EXTI15_10_IRQn;break;
    default : return NONE;
    }

    return &irq;
};
static int32 GpioIrqRegister(int32 pin, int32 mode, void (*hdr)(void *args), void *args)
{
    const struct PinIndex* index = GetPin(pin);
    int32 irqindex = -1;

    irqindex = Bit2Bitnum(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_hdr_tab)){
        return -ENONESYS;
    }

    x_base level = CriticalAreaLock();
    if (pin_irq_hdr_tab[irqindex].pin == pin   &&
        pin_irq_hdr_tab[irqindex].hdr == hdr   &&
        pin_irq_hdr_tab[irqindex].mode == mode &&
        pin_irq_hdr_tab[irqindex].args == args
    )
    {
        CriticalAreaUnLock(level);
        return EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1) {
        CriticalAreaUnLock(level);
        return -EDEV_BUSY;
    }
    pin_irq_hdr_tab[irqindex].pin = pin;
    pin_irq_hdr_tab[irqindex].hdr = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;
    CriticalAreaUnLock(level);
 
    return EOK;
}

static uint32 GpioIrqFree(int32 pin)
{
    const struct PinIndex* index = GetPin(pin);
    int32 irqindex = -1;

    irqindex = Bit2Bitnum(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_hdr_tab)){
        return -ENONESYS;
    }

    x_base level = CriticalAreaLock();
    if (pin_irq_hdr_tab[irqindex].pin == -1){
        CriticalAreaUnLock(level);
        return EOK;
    }
    pin_irq_hdr_tab[irqindex].pin  = -1;
    pin_irq_hdr_tab[irqindex].hdr  = NONE;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = NONE;
    CriticalAreaUnLock(level);
 
    return EOK;
}

static int32 GpioIrqEnable(x_base pin)
{
    const struct PinIndex* index = GetPin(pin);
    int32 irqindex = -1;
    const struct PinIrq *irq;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    irqindex = Bit2Bitnum(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_hdr_tab)){
        return -ENONESYS;
    }
    x_base level = CriticalAreaLock();
    if (pin_irq_hdr_tab[irqindex].pin == -1){
        CriticalAreaUnLock(level);
        return -ENONESYS;
    }

    irq = GetPinIrq(pin);
    if (irq == NONE){
        CriticalAreaUnLock(level);
        return -ENONESYS;
    }
    SYSCFG_EXTILineConfig(irq->port_source, irq->pin_source);
    EXTI_InitStructure.EXTI_Line = irq->exti_line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    switch (pin_irq_hdr_tab[irqindex].mode)
    {
    case GPIO_IRQ_EDGE_RISING:
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        break;
    case GPIO_IRQ_EDGE_FALLING:
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        break;
    case GPIO_IRQ_EDGE_BOTH:
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        break;
    }
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = irq->irq_exti_channel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    CriticalAreaUnLock(level);
    return EOK;
}

static int32 GpioIrqDisable(x_base pin)
{
    const struct PinIndex* index = GetPin(pin);
    const struct PinIrq *irq;
    EXTI_InitTypeDef EXTI_InitStructure;
    
    irq = GetPinIrq(index->pin);
    NULL_PARAM_CHECK(irq);

    EXTI_InitStructure.EXTI_Line = irq->exti_line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;  
    EXTI_Init(&EXTI_InitStructure); 
    return EOK;
}

static uint32 Stm32PinConfigure(struct PinParam *param)
{
    NULL_PARAM_CHECK(param);    
    int ret = EOK;

    const struct PinIndex *index = GetPin(param->pin);
    switch(param->cmd)
    {
        case GPIO_CONFIG_MODE:
            GpioConfigMode(param->mode, index);
            break;
        case GPIO_IRQ_REGISTER:
            ret = GpioIrqRegister(param->pin,param->irq_set.irq_mode,param->irq_set.hdr,param->irq_set.args);
            break;
        case GPIO_IRQ_FREE:
            ret = GpioIrqFree(param->pin);
            break;
        case GPIO_IRQ_ENABLE:
            ret = GpioIrqEnable(param->pin);
            break;
        case GPIO_IRQ_DISABLE:
            ret = GpioIrqDisable(param->pin);
            break;
        default:
            ret = -EINVALED;
            break;
    }

    return ret;
}

static uint32 Stm32PinInit(void)
{
    static x_bool PinInitFlag = RET_FALSE;

    if(!PinInitFlag){
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        PinInitFlag = RET_TRUE;
    }
    
    return EOK;
}

static uint32 Stm32GpioDrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;
    struct PinParam *param;

    switch (configure_info->configure_cmd)
    {
    case OPE_INT:
        ret = Stm32PinInit();
        break;    
    case OPE_CFG:
        param = (struct PinParam *)configure_info->private_data;
        ret = Stm32PinConfigure(param);
        break;
    default:
        break;
    }

    return ret;
}

uint32 Stm32PinWrite(void *dev, struct BusBlockWriteParam *write_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(write_param);
    struct PinStat *pinstat = (struct PinStat *)write_param->buffer;
    const struct PinIndex* index = GetPin(pinstat->pin);
    NULL_PARAM_CHECK(index);

    if (GPIO_LOW == pinstat->val){
        GPIO_ResetBits(index->gpio, index->pin);
    }
    else{
        GPIO_SetBits(index->gpio, index->pin);
    }
    return EOK;
}

uint32 Stm32PinRead(void *dev, struct BusBlockReadParam *read_param)
{
    NULL_PARAM_CHECK(dev);
    NULL_PARAM_CHECK(read_param);
    struct PinStat *pinstat = (struct PinStat *)read_param->buffer;
    const struct PinIndex* index = GetPin(pinstat->pin);
    NULL_PARAM_CHECK(index);
    
    if(GPIO_ReadInputDataBit(index->gpio, index->pin) == Bit_RESET) {
        pinstat->val = GPIO_LOW;
    } else {
        pinstat->val = GPIO_HIGH;
    }
    return pinstat->val;
}

static const struct PinDevDone dev_done =
{
    .open  = NONE,
    .close = NONE,
    .write = Stm32PinWrite,
    .read  = Stm32PinRead,
};

int Stm32HwGpioInit(void)
{
    x_err_t ret = EOK;

    static struct PinBus pin;

    ret = PinBusInit(&pin, PIN_BUS_NAME);
    if(ret != EOK) {
        KPrintf("gpio bus init error %d\n", ret);
        return ERROR;
    }

    static struct PinDriver drv;
    drv.configure = &Stm32GpioDrvConfigure;
    
    ret = PinDriverInit(&drv, PIN_DRIVER_NAME, NONE);
    if(ret != EOK){
        KPrintf("pin driver init error %d\n", ret);
        return ERROR;
    }
    ret = PinDriverAttachToBus(PIN_DRIVER_NAME, PIN_BUS_NAME);
    if(ret != EOK){
        KPrintf("pin driver attach error %d\n", ret);
        return ERROR;
    }

    static struct PinHardwareDevice dev;
    dev.dev_done = &dev_done;

    ret = PinDeviceRegister(&dev, NONE, PIN_DEVICE_NAME);
    if(ret != EOK){
        KPrintf("pin device register error %d\n", ret);
        return ERROR;
    }
    ret = PinDeviceAttachToBus(PIN_DEVICE_NAME, PIN_BUS_NAME);
    if(ret != EOK) {
        KPrintf("pin device register error %d\n", ret);
        return ERROR;
    }

    return ret;
}

static __inline void PinIrqHdr(int irqno)
{
    EXTI_ClearITPendingBit(Bitno2Bit(irqno));
    if (pin_irq_hdr_tab[irqno].hdr){
       pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

void EXTI0_IRQHandler(int irq_num, void *arg)
{
    PinIrqHdr(0);
}
DECLARE_HW_IRQ(EXTI0_IRQn, EXTI0_IRQHandler, NONE);

void EXTI1_IRQHandler(int irq_num, void *arg)
{
    PinIrqHdr(1);
}
DECLARE_HW_IRQ(EXTI1_IRQn, EXTI1_IRQHandler, NONE);

void EXTI2_IRQHandler(int irq_num, void *arg)
{
    PinIrqHdr(2);
}
DECLARE_HW_IRQ(EXTI2_IRQn, EXTI2_IRQHandler, NONE);

void EXTI3_IRQHandler(int irq_num, void *arg)
{
    PinIrqHdr(3);
}
DECLARE_HW_IRQ(EXTI3_IRQn, EXTI3_IRQHandler, NONE);

void EXTI4_IRQHandler(int irq_num, void *arg)
{
    PinIrqHdr(4);
}
DECLARE_HW_IRQ(EXTI4_IRQn, EXTI4_IRQHandler, NONE);

void EXTI9_5_IRQHandler(int irq_num, void *arg)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET){
        PinIrqHdr(5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET){
        PinIrqHdr(6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) != RESET){
        PinIrqHdr(7);
    }
    if (EXTI_GetITStatus(EXTI_Line8) != RESET){
        PinIrqHdr(8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET){
        PinIrqHdr(9);
    }
}
DECLARE_HW_IRQ(EXTI9_5_IRQn, EXTI9_5_IRQHandler, NONE);

void EXTI15_10_IRQHandler(int irq_num, void *arg)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET){
        PinIrqHdr(10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET){
        PinIrqHdr(11);
    }
    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        PinIrqHdr(12);
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET){
        PinIrqHdr(13);
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET){
        PinIrqHdr(14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET){
        PinIrqHdr(15);
    }
}
DECLARE_HW_IRQ(EXTI15_10_IRQn, EXTI15_10_IRQHandler, NONE);
