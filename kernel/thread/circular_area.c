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
* @file:    circular_area.c
* @brief:   circular area file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include "xs_circular_area.h"
#include <string.h>
#include <xs_kdbg.h>
#include <xs_klist.h>
#include <xs_memory.h>

/**
 * This function will return whether the circular_area is full or not
 *
 * @param circular_area CircularArea descriptor
 */
x_bool CircularAreaIsFull(CircularAreaType circular_area)
{
    NULL_PARAM_CHECK(circular_area);

    if((circular_area->readidx == circular_area->writeidx) && (circular_area->b_status)) {
        KPrintf("the circular area is full\n");
        return RET_TRUE;
    } else {
        return RET_FALSE;
    }
}

/**
 * This function will return whether the circular_area is empty or not
 *
 * @param circular_area CircularArea descriptor
 */
x_bool CircularAreaIsEmpty(CircularAreaType circular_area)
{
    NULL_PARAM_CHECK(circular_area);

    if((circular_area->readidx == circular_area->writeidx) && (!circular_area->b_status)) {
        KPrintf("the circular area is empty\n");
        return RET_TRUE;
    } else {
        return RET_FALSE;
    }
}

/**
 * This function will reset the circular_area and set the descriptor to default
 *
 * @param circular_area CircularArea descriptor
 */
void CircularAreaReset(CircularAreaType circular_area)
{
    circular_area->writeidx = 0;
    circular_area->readidx = 0;
    circular_area->b_status = RET_FALSE;
}

/**
 * This function will release the circular_area descriptor and free the memory
 *
 * @param circular_area CircularArea descriptor
 */
void CircularAreaRelease(CircularAreaType circular_area)
{
    circular_area->readidx = 0;
    circular_area->writeidx = 0;
    circular_area->p_head = NONE;
    circular_area->p_tail = NONE;
    circular_area->b_status = RET_FALSE;
    circular_area->area_length = 0;

    x_free(circular_area->data_buffer);
    x_free(circular_area);
}

/**
 * This function will get the circual_area max length
 *
 * @param circular_area CircularArea descriptor
 */
uint32 CircularAreaGetMaxLength(CircularAreaType circular_area)
{
    NULL_PARAM_CHECK(circular_area);

    return circular_area->area_length;
}

/**
 * This function will get the data length of the circular_area
 *
 * @param circular_area CircularArea descriptor
 */
uint32 CircularAreaGetDataLength(CircularAreaType circular_area)
{
    NULL_PARAM_CHECK(circular_area);

    if(CircularAreaIsFull(circular_area)) {
        return circular_area->area_length;
    } else {
        return (circular_area->writeidx - circular_area->readidx + circular_area->area_length) % circular_area->area_length;
    }
}

/**
 * This function will return whether it is need to divide the read data into two parts  or not
 *
 * @param circular_area CircularArea descriptor
 * @param data_length output data length
 */
static uint32 CircularAreaDivideRdData(CircularAreaType circular_area, uint32 data_length)
{
    NULL_PARAM_CHECK(circular_area);

    if(circular_area->readidx + data_length <= circular_area->area_length) {
        return RET_FALSE;
    } else {
        return RET_TRUE;
    }
}

/**
 * This function will return whether it is need to divide the write data into two parts  or not
 *
 * @param circular_area CircularArea descriptor
 * @param data_length input data length
 */
static uint32 CircularAreaDivideWrData(CircularAreaType circular_area, uint32 data_length)
{
    NULL_PARAM_CHECK(circular_area);

    if(circular_area->writeidx + data_length <= circular_area->area_length) {
        return RET_FALSE;
    } else {
        return RET_TRUE;
    }
}

/**
 * This function will read data from the circular_area
 *
 * @param circular_area CircularArea descriptor
 * @param output_buffer output data buffer poniter
 * @param data_length output data length
 */
uint32 CircularAreaRead(CircularAreaType circular_area, uint8 *output_buffer, uint32 data_length)
{
    NULL_PARAM_CHECK(circular_area);
    NULL_PARAM_CHECK(output_buffer);
    CHECK(data_length > 0);

    if(CircularAreaIsEmpty(circular_area)) {
        return ERROR;
    }

    data_length = (data_length > CircularAreaGetDataLength(circular_area)) ? CircularAreaGetDataLength(circular_area) : data_length;

    if(CircularAreaDivideRdData(circular_area, data_length)) {
        uint32 read_len_up = circular_area->area_length - circular_area->readidx;
        uint32 read_len_down = data_length - read_len_up;

        memcpy(output_buffer, &circular_area->data_buffer[circular_area->readidx], read_len_up);
        memcpy(output_buffer + read_len_up, circular_area->p_head, read_len_down);

        circular_area->readidx = read_len_down;
    } else {
        memcpy(output_buffer, &circular_area->data_buffer[circular_area->readidx], data_length);
        circular_area->readidx = (circular_area->readidx + data_length) % circular_area->area_length;
    }

    circular_area->b_status = RET_FALSE;

    return EOK;
}

/**
 * This function will write data to the circular_area
 *
 * @param circular_area CircularArea descriptor
 * @param input_buffer input data buffer poniter
 * @param data_length input data length
 * @param b_force whether to force to write data disregard the length limit
 */
uint32 CircularAreaWrite(CircularAreaType circular_area, uint8 *input_buffer, uint32 data_length, x_bool b_force)
{
    NULL_PARAM_CHECK(circular_area);
    NULL_PARAM_CHECK(input_buffer);
    CHECK(data_length > 0);

    if(CircularAreaIsFull(circular_area) && (!b_force)) {
        return ERROR;
    }

    uint32 write_data_length = circular_area->area_length - CircularAreaGetDataLength(circular_area);
    data_length = (data_length > write_data_length) ? write_data_length : data_length;

    if(CircularAreaDivideWrData(circular_area, data_length)) {
        uint32 write_len_up = circular_area->area_length - circular_area->writeidx;
        uint32 write_len_down = data_length - write_len_up;

        memcpy(&circular_area->data_buffer[circular_area->writeidx], input_buffer, write_len_up);
        memcpy(circular_area->p_head, input_buffer + write_len_up, write_len_down);

        circular_area->writeidx = write_len_down;
    } else {
        memcpy(&circular_area->data_buffer[circular_area->writeidx], input_buffer, data_length);
        circular_area->writeidx = (circular_area->writeidx + data_length) % circular_area->area_length;
    }

    circular_area->b_status = RET_TRUE;

    if(b_force) {
        circular_area->readidx = circular_area->writeidx;
    }

    return EOK;
}

static struct CircularAreaOps CircularAreaOperations =
{
    CircularAreaRead,
    CircularAreaWrite,
    CircularAreaRelease,
    CircularAreaReset,
};

/**
 * This function will initialize the circular_area
 *
 * @param circular_area_length circular_area length
 */
CircularAreaType CircularAreaInit(uint32 circular_area_length)
{
    CHECK(circular_area_length > 0);

    circular_area_length = ALIGN_MEN_DOWN(circular_area_length, MEM_ALIGN_SIZE);

    CircularAreaType circular_area = x_malloc(sizeof(struct CircularArea));
    if(NONE == circular_area) {
        KPrintf("CircularAreaInit malloc struct circular_area failed\n");
        x_free(circular_area);
        return NONE;
    }

    CircularAreaReset(circular_area);

    circular_area->data_buffer = x_malloc(circular_area_length);
    if(NONE == circular_area->data_buffer) {
        KPrintf("CircularAreaInit malloc circular_area data_buffer failed\n");
        x_free(circular_area->data_buffer);
        return NONE;
    }

    circular_area->p_head = circular_area->data_buffer;
    circular_area->p_tail = circular_area->data_buffer + circular_area_length;
    circular_area->area_length = circular_area_length;

    KPrintf("CircularAreaInit done p_head %8p p_tail %8p length %u\n",
        circular_area->p_head, circular_area->p_tail, circular_area->area_length);

    circular_area->CircularAreaOperations = &CircularAreaOperations;

    return circular_area;
}
