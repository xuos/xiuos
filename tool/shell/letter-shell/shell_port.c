/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#include "xiuos.h"
#include "shell.h"
#include "shell_fs.h"
#include <stdio.h>
#ifdef FS_VFS
#include <iot-vfs_posix.h>
#endif
#include <stddef.h>
#include <string.h>
#include "xsconfig.h"
#include <device.h>

Shell shell;
char *shellBuffer;
ShellFs shellFs;
char *shellPathBuffer;

HardwareDevType console; 

/**
 * @brief Shell write 
 * 
 * @param data write data
 */
void userShellWrite(char data)
{
    KPrintf("%c",data);
}

/**
 * @brief shell read
 * 
 * @param data read data
 * @return char  read status
 */
signed char userShellRead(char *data)
{
    struct BusBlockReadParam read_param;
    read_param.size = 1;
    read_param.buffer = data;

    BusDevReadData(console, &read_param);

    return 0;
}

#ifdef SHELL_ENABLE_FILESYSTEM
/**
 * @brief list file
 * 
 * @param path  path
 * @param buffer result buffer
 * @param maxLen the maximum buffer size
 * @return size_t 0
 */
size_t userShellListDir(char *path, char *buffer, size_t maxLen)
{
    DIR *dir;
    struct dirent *ptr;
    int i;
    dir = opendir(path);
    memset(buffer, 0, maxLen);
    while((ptr = readdir(dir)) != NULL)
    {
        strcat(buffer, ptr->d_name);
        strcat(buffer, "\t");
    }
    closedir(dir);
    return 0;
}
#endif

/**
 * @brief Initialize the shell 
 * 
 */
int userShellInit(void)
{
    shellBuffer = x_malloc(512*sizeof(char));
    
  
#ifdef SHELL_ENABLE_FILESYSTEM
    shellPathBuffer = x_malloc(512*sizeof(char));
    shellPathBuffer[0] = '/';
    shellFs.getcwd = getcwd;
    shellFs.chdir = chdir;
    shellFs.listdir = userShellListDir;
    shellFsInit(&shellFs, shellPathBuffer, 512);
    shellSetPath(&shell, shellPathBuffer);
#endif    

    shell.write = userShellWrite;
    shell.read = userShellRead;

    console = ObtainConsole();

    /*Open the serial device in interrupt receiving and polling sending mode */
    struct SerialDevParam *serial_dev_param = (struct SerialDevParam *)console->private_data;
    serial_dev_param->serial_set_mode = SIGN_OPER_INT_RX;
    serial_dev_param->serial_stream_mode = SIGN_OPER_STREAM;
    BusDevOpen(console);
    
    shellInit(&shell, shellBuffer, 512);
    int32 tid;
    tid = KTaskCreate("letter-shell",
                           shellTask, &shell,
                           SHELL_TASK_STACK_SIZE, SHELL_TASK_PRIORITY);
        
        StartupKTask(tid);
        return 0;
}




