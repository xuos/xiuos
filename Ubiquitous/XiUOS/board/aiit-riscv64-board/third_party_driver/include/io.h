/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file io.h
* @brief add from Canaan k210 SDK
*                https://canaan-creative.com/developer
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define readb(addr) (*(volatile uint8_t *)(addr))
#define readw(addr) (*(volatile uint16_t *)(addr))
#define readl(addr) (*(volatile uint32_t *)(addr))
#define readq(addr) (*(volatile uint64_t *)(addr))

#define writeb(v, addr)                                                        \
    {                                                                      \
        (*(volatile uint8_t *)(addr)) = (v);                           \
    }
#define writew(v, addr)                                                        \
    {                                                                      \
        (*(volatile uint16_t *)(addr)) = (v);                          \
    }
#define writel(v, addr)                                                        \
    {                                                                      \
        (*(volatile uint32_t *)(addr)) = (v);                          \
    }
#define writeq(v, addr)                                                        \
    {                                                                      \
        (*(volatile uint64_t *)(addr)) = (v);                          \
    }

#ifdef __cplusplus
}
#endif

#endif /* __IO_H__ */
