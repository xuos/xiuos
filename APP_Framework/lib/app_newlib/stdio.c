/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017/10/15     bernard      the first version
 */

/**
* @file stdio.c
* @brief support newlib stdio
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: stdio.c
Description: support newlib stdio
Others: take RT-Thread v4.0.2/components/libc/compilers/newlib/stdio.c for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: Use set and get console functions
*************************************************/

#include <libc.h>
#include <stdio.h>
#include <stdlib.h>

#define STDIO_DEVICE_NAME_MAX   32

static FILE* std_console = NULL;

/**
 * This function will set system console device.
 *
 * @param device_name  the name of device
 * @param mode the mode
 *
 * @return file number on success; or -1 on failure
 */
int LibcStdioSetConsole(const char* device_name, int mode)
{
    FILE *fp;
    char name[STDIO_DEVICE_NAME_MAX];
    char *file_mode;

    snprintf(name, sizeof(name) - 1, "/dev/%s", device_name);
    name[STDIO_DEVICE_NAME_MAX - 1] = '\0';

    switch (mode)
    {
    case O_RDWR:
        file_mode = "r+";
        break;
    
    case O_WRONLY:
        file_mode = "wb";
        break;

    default:
        file_mode = "rb";
        break;
    }

    /* try to open file */
    fp = fopen(name, file_mode);
    if (fp)
    {
        /* set the fp buffer */
        setvbuf(fp, NULL, _IONBF, 0);

        if (std_console)
            /* try to close console device */
            fclose(std_console);
        std_console = fp;

        if (mode == O_RDWR)
        {
            /* set _stdin as std_console */
            _GLOBAL_REENT->_stdin  = std_console;
        }
        else 
        {
            /* set NULL */
            _GLOBAL_REENT->_stdin  = NULL;
        }

        if (mode == O_RDONLY)
        {
            /* set the _stdout as NULL */
            _GLOBAL_REENT->_stdout = NULL;
            /* set the _stderr as NULL */
            _GLOBAL_REENT->_stderr = NULL;
        }
        else
        {
            /* set the _stdout as std_console */
            _GLOBAL_REENT->_stdout = std_console;
            /* set the _stderr as std_console */
            _GLOBAL_REENT->_stderr = std_console;
        }
        /* set the __sdidinit as 1 */
        _GLOBAL_REENT->__sdidinit = 1;
    }

    if (std_console)
        /* return the file number */
        return fileno(std_console);
    /* failure and return -1 */
    return -1;
}

/**
 * This function will get system console device.
 *
 * @return file number on success; or -1 on failure
 */
int LibcStdioGetConsole(void) {
    if (std_console)
        /* return the file number */
        return fileno(std_console);
    else
        /* failure and return -1 */
        return -1;
}

/**
 * This function will initialize the c library system.
 *
 * @return 0
 */
int LibcSystemInit(void)
{
#if defined(KERNEL_CONSOLE)
    HardwareDevType console;
    /* try to get console device */
    console = ObtainConsole();
    if (console)
    {
#if defined(LIB_POSIX)
        /* set console device mode */
        LibcStdioSetConsole(console->dev_name, O_RDWR);
#else
        /* set console device mode */
        LibcStdioSetConsole(console->dev_name, O_WRONLY);
#endif
    }
#endif
    return 0;
}
