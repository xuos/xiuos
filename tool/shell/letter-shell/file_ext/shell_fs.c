/**
 * @file shell_fs.c
 * @author Letter (nevermindzzt@gmail.com)
 * @brief shell file system support
 * @version 0.1
 * @date 2020-07-22
 * 
 * @copyright (c) 2020 Letter
 * 
 */
#include "shell_fs.h"
#include "shell.h"
#include "stdio.h"



/**
 * @brief init shell filesystem-suport component
 * 
 * @param shellFs filesystem shell object
 * @param pathBuffer  path
 * @param pathLen length of path
 */
void shellFsInit(ShellFs *shellFs, char *pathBuffer, size_t pathLen)
{
    shellFs->info.path = pathBuffer;
    shellFs->info.pathLen = pathLen;
    shellFs->getcwd(shellFs->info.path, pathLen);
}
