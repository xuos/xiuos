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
* @file:    appstartup.c
* @brief:   init application userspace and create main task
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xiuos.h>
#include <board.h>
#ifdef TASK_ISOLATION
#include <xs_isolation.h>
#endif

#ifdef APP_STARTUP_FROM_SDCARD
#include <iot-vfs_posix.h>
#endif

extern int main(void);
#ifdef USER_APPLICATION
#ifndef SEPARATE_COMPILE
void MainKTaskFunction(void *parameter)
{  
#if defined(__ICCARM__) || defined(__GNUC__)
    main();
#endif
}
#endif

/**
 * This function will create main application 
 * 
 * 
 */
void CreateMainTask(void)
{
    int32 main = 0;

#ifdef SEPARATE_COMPILE
	KPrintf("Tip!!! Kernel is separated with application. main entry : 0x%08x \n",USERSPACE->us_entrypoint);

	main = UTaskCreate("main", (void*)USERSPACE->us_entrypoint, NONE,
                           MAIN_KTASK_STACK_SIZE, MAIN_KTASK_PRIORITY);

#else
    main = KTaskCreate("main", MainKTaskFunction, NONE,
                           MAIN_KTASK_STACK_SIZE, MAIN_KTASK_PRIORITY);

#endif

    if(main < 0) {		
		KPrintf("main create failed ...%s %d.\n",__FUNCTION__,__LINE__);
		return;
	}

    StartupKTask(main);
}

#ifdef SEPARATE_COMPILE
int InitUserspace(void)
{
#ifdef APP_STARTUP_FROM_FLASH
    uint8_t *src = NONE;
    uint8_t *dest = NONE;
    uint8_t *end = NONE;
    
    dest = (uint8_t *)USERSPACE->us_bssstart;
    end  = (uint8_t *)USERSPACE->us_bssend;
    while (dest != end) {
        *dest++ = 0;
    }

    /* Initialize all of user-space .data */

    src  = (uint8_t *)USERSPACE->us_datasource;
    dest = (uint8_t *)USERSPACE->us_datastart;
    end  = (uint8_t *)USERSPACE->us_dataend;
    while (dest != end) {
        *dest++ = *src++;
    }
#ifndef  TASK_ISOLATION
    src  = (uint8_t *)&g_service_table_start;
    dest = (uint8_t *)SERVICE_TABLE_ADDRESS;
    end  = (uint8_t *)&g_service_table_end;
    while (src != end) {
        *dest++ = *src++;
    }
#endif 

    UserInitBoardMemory((void*)USER_MEMORY_START_ADDRESS, (void*)USER_MEMORY_END_ADDRESS);

#ifdef  MOMERY_PROTECT_ENABLE
         if ( mem_access.Init != NONE){
             if(mem_access.Init( (void **)(&isolation)) == EOK)
                 mem_access.Load(isolation);
         }
#endif
    return EOK;
#endif

#ifdef APP_STARTUP_FROM_SDCARD
    int fd = 0;
	char buf[1024] = {0};
	int len = 0;
    int len_check = 0;
    uint8_t *src = NONE;
    uint8_t *dest = NONE;
    uint8_t *end = NONE;
#ifndef FS_VFS
    KPrintf("fs not enable!%s %d\n",__func__,__LINE__);
    CHECK(0);
#endif
    
	fd = open(BOARD_APP_NAME,O_RDONLY );
	if(fd > 0) {
        KPrintf("open app bin %s success.\n",BOARD_APP_NAME);

        dest = (uint8_t *)USERSPACE;
        /* copy app to USERSPACE */
        while(RET_TRUE) {
            memset(buf, 0 , 1024); 
            len = read(fd, buf, 1024);
            KPrintf("read app bin len %d\n",len);
            if(len > 0) {
                 memcpy(dest, buf, len);
                 dest = dest + len;
                 len_check = len_check + len;
            } else {
                break;
            }
        }

        if(len_check <= 0){
            return -ERROR;
        }

        dest = (uint8_t *)USERSPACE->us_bssstart;
        end  = (uint8_t *)USERSPACE->us_bssend;
        while (dest != end) {
            *dest++ = 0;
        }

		src  = (uint8_t *)&g_service_table_start;
        dest = (uint8_t *)SERVICE_TABLE_ADDRESS;
        end  = (uint8_t *)&g_service_table_end;

        while (src != end) {
            *dest++ = *src++;
        }

		close(fd);

        UserInitBoardMemory((void*)USER_MEMORY_START_ADDRESS, (void*)USER_MEMORY_END_ADDRESS);

        return EOK;

	} else {
		KPrintf("open app bin %s failed.\n",BOARD_APP_NAME);
        return -EEMPTY;
	}
#endif
}
#endif
#endif