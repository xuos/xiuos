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
* @file TestCirculararea.c
* @brief support to test circular area function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <device.h>
#include <string.h>
#ifdef KERNEL_CIRCULAR_AREA

#define READ_LENGTH 2
#define Write_LENGTH 3
#define CIRCULARAREA_LENGTH 8

static struct CircularArea *g_circular_area;
static int8 test_string[] = "Aiit-XiuOs";

static int32 task_caread;
static int32 task_cawrite;

int data_lock = -1;

static void CircularArea_ReadTsk_Entry(void *parameter)
{
    uint8 i;
    uint8 read_string[8] = {0};
    uint8 read_cnt = 0;

    while(read_cnt < 30)
    {
        KMutexObtain(data_lock, WAITING_FOREVER);
        
        if(EOK == g_circular_area->CircularAreaOperations->read(g_circular_area, read_string, READ_LENGTH)){
            for(i = 0; i < READ_LENGTH; i ++){
                KPrintf("Read  data i %u ch %c rdidx %u wridx %u\n", i, read_string[i], g_circular_area->readidx, g_circular_area->writeidx);
            }

            #ifdef Test_Dbg
            KPrintf("Read TSK writeidx %u readidx %u status %u len %u\n", 
                g_circular_area->writeidx, 
                g_circular_area->readidx, 
                g_circular_area->b_status,
                CircularAreaGetDataLength(g_circular_area));
            #endif
        }

        read_cnt++;
        MdelayKTask(500);

        KMutexAbandon(data_lock);
    }

    KMutexDelete(data_lock);
    KTaskDelete(task_caread);
    g_circular_area->CircularAreaOperations->release(g_circular_area);
}

static void CircularAreaWriteTskEntry(void *parameter)
{
    uint8 i;
    uint8 write_cnt = 0;
    uint8 string_length = strlen(test_string);
    static uint32 single_write_length = 0;

    while(write_cnt < 30)
    {
        KMutexObtain(data_lock, WAITING_FOREVER);

        uint8 write_string[CIRCULARAREA_LENGTH] = {0};
        if((write_cnt % string_length + Write_LENGTH) > string_length){
            memcpy(write_string, &test_string[write_cnt % string_length], string_length - write_cnt % string_length);
            memcpy(&write_string[string_length - write_cnt % string_length], test_string, Write_LENGTH - string_length + write_cnt % string_length);
        }
        else{
            memcpy(write_string, &test_string[write_cnt % string_length], Write_LENGTH);
        }

        if(EOK == g_circular_area->CircularAreaOperations->write(g_circular_area, write_string, Write_LENGTH, 0))
        {
            for(i = 0; i < Write_LENGTH; i ++) {
                KPrintf("Write data i %d ch %c rdidx %u wridx %u\n", i, write_string[i], g_circular_area->readidx, g_circular_area->writeidx);
            }
            write_cnt += (g_circular_area->writeidx - single_write_length + CIRCULARAREA_LENGTH) % CIRCULARAREA_LENGTH;
            single_write_length = g_circular_area->writeidx;
        }

        #ifdef Test_Dbg
        KPrintf("Write TSK writeidx %u readidx %u status %u len %u\n", 
            g_circular_area->writeidx, 
            g_circular_area->readidx, 
            g_circular_area->b_status,
            CircularAreaGetDataLength(g_circular_area));
        #endif
        MdelayKTask(500);

        KMutexAbandon(data_lock);
    }

    KTaskDelete(task_cawrite);
}

static uint32 TestCirculararea(void)
{
    x_err_t flag;

    g_circular_area = CircularAreaInit(CIRCULARAREA_LENGTH);
    if(g_circular_area){
        KPrintf("CircularAreaInit done buffer 0x%8p length %u phead 0x%8p ptail 0x%8p wridx %u rdidx %u\n", 
            g_circular_area->data_buffer, g_circular_area->area_length,
            g_circular_area->p_head, g_circular_area->p_tail,
            g_circular_area->writeidx, g_circular_area->readidx);
    }
    else{
        KPrintf("CircularAreaInit failed!Just return\n");
        CircularAreaRelease(g_circular_area);
        return ERROR;
    }

	data_lock = KMutexCreate();
	if (data_lock < 0){
		KPrintf("data_lock creat failed.\n");
        g_circular_area->CircularAreaOperations->release(g_circular_area);
		return ERROR;
	}    

    task_caread = KTaskCreate("task_caread", CircularArea_ReadTsk_Entry, NONE, 2048, 10); 
	flag = StartupKTask(task_caread);
    if (EOK != flag){
		KPrintf("CircularArea_Test StartupKTask task_caread failed!\n");
        g_circular_area->CircularAreaOperations->release(g_circular_area);
		return ERROR;
	}

    task_cawrite = KTaskCreate("task_cawrite", CircularAreaWriteTskEntry, NONE, 2048, 10); 
	flag = StartupKTask(task_cawrite);
    if (EOK != flag){
		KPrintf("CircularArea_Test StartupKTask task_cawrite failed!\n");
        g_circular_area->CircularAreaOperations->release(g_circular_area);
		return ERROR;
	}

    return EOK;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
                                                TestCirculararea, TestCirculararea, Test the Circular Area Function);
#endif