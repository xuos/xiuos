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

/**
* @file:   xs_circular_area.h
* @brief:  function declaration and structure defintion of circular area
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef CIRCULAR_AREA_H
#define CIRCULAR_AREA_H

#include <xs_base.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef KERNEL_CIRCULAR_AREA
typedef struct CircularArea *CircularAreaType;

struct CircularAreaOps
{
    uint32 (*read) (CircularAreaType circular_area, uint8 *output_buffer, uint32 data_length);
    uint32 (*write) (CircularAreaType circular_area, uint8 *input_buffer, uint32 data_length, x_bool b_force);
    void (*release) (CircularAreaType circular_area);
    void (*reset) (CircularAreaType circular_area);
};

struct CircularArea
{
    uint8 *data_buffer;

    uint8 readidx;
    uint8 writeidx;

    uint8 *p_head;
    uint8 *p_tail;

    uint32 area_length;
    x_bool b_status;

    struct CircularAreaOps *CircularAreaOperations;
};

/*This function will return whether the circular_area is full or not*/
x_bool CircularAreaIsFull(CircularAreaType circular_area);

/*This function will return whether the circular_area is empty or not*/
x_bool CircularAreaIsEmpty(CircularAreaType circular_area);

/*This function will reset the circular_area and set the descriptor to default*/
void CircularAreaReset(CircularAreaType circular_area);

/*This function will release the circular_area descriptor and free the memory*/
void CircularAreaRelease(CircularAreaType circular_area);

/*This function will read data from the circular_area*/
uint32 CircularAreaRead(CircularAreaType circular_area, uint8 *output_buffer, uint32 data_length);

/*This function will write data to the circular_area*/
uint32 CircularAreaWrite(CircularAreaType circular_area, uint8 *input_buffer, uint32 data_length, x_bool b_force);

/*This function will get the circual_area max length*/
uint32 CircularAreaGetMaxLength(CircularAreaType circular_area);

/*This function will get the data length of the circular_area*/
uint32 CircularAreaGetDataLength(CircularAreaType circular_area);

/*This function will initialize the circular_area*/
CircularAreaType CircularAreaInit(uint32 circular_area_length);
#endif
#ifdef __cplusplus
}
#endif

#endif
