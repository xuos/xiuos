/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2016-04-23
 */

/**
* @file sfud_cfg.h
* @brief define SFUD_DEBUG_MODE and SFUD_FLASH_DEVICE_TABLE
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

/*************************************************
File name: sfud_cfg.h
Description: support sfud debug log and device table configure
Others: add sfud_cfg.h to support SPI Flash function based on SFUD LIB
                https://github.com/armink/SFUD
History: 
1. Date: 2021-04-24
Author: AIIT XUOS Lab
Modification: 
1. add xsconfig.h support
*************************************************/

#ifndef _SFUD_CFG_H_
#define _SFUD_CFG_H_

#include <xsconfig.h>

#ifdef SFUD_DEBUG_LOG
#define SFUD_DEBUG_MODE
#endif

enum {
    SFUD_XXXX_DEVICE_INDEX = 0,
};

#define SFUD_FLASH_DEVICE_TABLE                                                \
{                                                                              \
    [SFUD_XXXX_DEVICE_INDEX] = {.name = "XXXX", .spi.name = "SPIX"},           \
}

//#define SFUD_USING_QSPI

#endif /* _SFUD_CFG_H_ */
