/**
 * @file shell_ext.h
 * @author Letter (NevermindZZT@gmail.com)
 * @brief shell extensions
 * @version 3.0.0
 * @date 2019-12-31
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#ifndef __SHELL_EXT_H__
#define __SHELL_EXT_H__

#include "shell.h"

/**
 * @brief type of number
 * 
 */
typedef enum
{
    NUM_TYPE_INT,                                           /**< Decimal integer  */
    NUM_TYPE_BIN,                                           /**< Binary integer  */
    NUM_TYPE_OCT,                                           /**< Octal integer  */
    NUM_TYPE_HEX,                                           /**< Hexadecimal integer  */
    NUM_TYPE_FLOAT                                          /**< Floating point  */
} NUM_Type;

unsigned int shellExtParsePara(Shell *shell, char *string);
int shellExtRun(Shell *shell, ShellCommand *command, int argc, char *argv[]);

#endif
