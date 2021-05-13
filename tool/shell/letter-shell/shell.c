/**
 * @file shell.c
 * @author Letter (NevermindZZT@gmail.com)
 * @version 3.0.0
 * @date 2019-12-30
 * 
 * @copyright (c) 2020 Letter
 * 
 */
/*change log:
    Change Chinese comment to English comment 

*/

#include "shell.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "shell_ext.h"
#include <xiuos.h>

#ifdef FS_VFS
#include "iot-vfs.h"
#endif

#ifndef CommandDescSize
#define CommandDescSize 36
#endif

/**
 * @brief default user
 */
const char shellCmdDefaultUser[] = SHELL_DEFAULT_USER;
const char shellPasswordDefaultUser[] = SHELL_DEFAULT_USER_PASSWORD;
const char shellDesDefaultUser[] = "default user";
const ShellCommand shellUserDefault SECTION("shellCommand") =
{
    .attr.value = SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_USER),
    .data.user.name = shellCmdDefaultUser,
    .data.user.password = shellPasswordDefaultUser,
    .data.user.desc = shellDesDefaultUser
};


   
    #if defined(__GNUC__)
        extern const unsigned long _shell_command_start;
        extern const unsigned long _shell_command_end;
    #endif


/**
 * @brief shell Constant text index 
 */
enum
{
    SHELL_TEXT_INFO,                                    /**<shell information  */
    SHELL_TEXT_CMD_TOO_LONG,                            /**< Command is too long  */
    SHELL_TEXT_CMD_LIST,                                /**<  Command list   */
    SHELL_TEXT_VAR_LIST,                                /**< Variable List */
    SHELL_TEXT_USER_LIST,                               /**<User List*/
    SHELL_TEXT_KEY_LIST,                                /**< Key List */
    SHELL_TEXT_CMD_NOT_FOUND,                           /**< Commmand not fuond */
    SHELL_TEXT_POINT_CANNOT_MODIFY,                     /**< Pointer variables are not allowed to be modified  */
    SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY,             /**< Read-only variables are not allowed to be modified  */
    SHELL_TEXT_NOT_VAR,                                 /**< Command is not a variable  */
    SHELL_TEXT_VAR_NOT_FOUND,                           /**< Variable not found  */
    SHELL_TEXT_HELP_HEADER,                             /**< help  */
    SHELL_TEXT_PASSWORD_HINT,                           /**< Password input hint */
    SHELL_TEXT_PASSWORD_ERROR,                          /**< wrong password  */
    SHELL_TEXT_CLEAR_CONSOLE,                           /**< Clear the console  */
    SHELL_TEXT_CLEAR_LINE,                              /**< Clear current line  */
    SHELL_TEXT_TYPE_CMD,                                /**< Command type  */
    SHELL_TEXT_TYPE_VAR,                                /**< Variable type  */
    SHELL_TEXT_TYPE_USER,                               /**< User type  */
    SHELL_TEXT_TYPE_KEY,                                /**< Key type  */
    SHELL_TEXT_TYPE_NONE,                               /**< Illegal type  */

};


static const char *shellText[] =
{
        [SHELL_TEXT_INFO] =
        "\r\n"
        " _         _   _                  _          _ _ \r\n"
        "| |    ___| |_| |_ ___ _ __   ___| |__   ___| | |\r\n"
        "| |   / _ \\ __| __/ _ \\ '__| / __| '_ \\ / _ \\ | |\r\n"
        "| |__|  __/ |_| ||  __/ |    \\__ \\ | | |  __/ | |\r\n"
        "|_____\\___|\\__|\\__\\___|_|    |___/_| |_|\\___|_|_|\r\n"
        "\r\n"
        "Build:       "__DATE__" "__TIME__"\r\n"
        "Version:     "SHELL_VERSION"\r\n"
        "Copyright:   (c) 2020 Letter\r\n",
    [SHELL_TEXT_CMD_TOO_LONG] = 
        "\r\nWarning: Command is too long\r\n",
    [SHELL_TEXT_CMD_LIST] = 
        "\r\nCommand List:\r\n",
    [SHELL_TEXT_VAR_LIST] = 
        "\r\nVar List:\r\n",
    [SHELL_TEXT_USER_LIST] = 
        "\r\nUser List:\r\n",
    [SHELL_TEXT_KEY_LIST] =
        "\r\nKey List:\r\n",
    [SHELL_TEXT_CMD_NOT_FOUND] = 
        "Command not Found\r\n",
    [SHELL_TEXT_POINT_CANNOT_MODIFY] = 
        "can't set pointer\r\n",
    [SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY] = 
        "can't set read only var\r\n",
    [SHELL_TEXT_NOT_VAR] =
        " is not a var\r\n",
    [SHELL_TEXT_VAR_NOT_FOUND] = 
        "Var not Fount\r\n",
    [SHELL_TEXT_HELP_HEADER] =
        "command help of ",
    [SHELL_TEXT_PASSWORD_HINT] = 
        "\r\nPlease input password:",
    [SHELL_TEXT_PASSWORD_ERROR] = 
        "\r\npassword error\r\n",
    [SHELL_TEXT_CLEAR_CONSOLE] = 
        "\033[2J\033[1H",
    [SHELL_TEXT_CLEAR_LINE] = 
        "\033[2K\r",
    [SHELL_TEXT_TYPE_CMD] = 
        "CMD ",
    [SHELL_TEXT_TYPE_VAR] = 
        "VAR ",
    [SHELL_TEXT_TYPE_USER] = 
        "USER",
    [SHELL_TEXT_TYPE_KEY] = 
        "KEY ",
    [SHELL_TEXT_TYPE_NONE] = 
        "NONE",
};


/**
 * @brief shell object list
 */
static Shell *shellList[SHELL_MAX_NUMBER] = {NULL};


static void shellAdd(Shell *shell);
static void shellWriteCommandLine(Shell *shell, unsigned char newline);
static void shellWriteReturnValue(Shell *shell, int value);
static int shellShowVar(Shell *shell, ShellCommand *command);
static void shellSetUser(Shell *shell, const ShellCommand *user);
ShellCommand* shellSeekCommand(Shell *shell,
                               const char *cmd,
                               ShellCommand *base,
                               unsigned short compareLength);

/**
 * @brief shell initialization 
 * 
 * @param shell shell
 * @param buffer buffer,used to record history
 * @param size the size of buffer
 */
void shellInit(Shell *shell, char *buffer, unsigned short size)
{
    shell->parser.length = 0;
    shell->parser.cursor = 0;
    shell->history.offset = 0;
    shell->history.number = 0;
    shell->history.record = 0;
    shell->info.user = NULL;
    shell->status.isChecked = 1;

    shell->parser.buffer = buffer;
    shell->parser.bufferSize = size / (SHELL_HISTORY_MAX_NUMBER + 1);
    for (short i = 0; i < SHELL_HISTORY_MAX_NUMBER; i++)
    {
        shell->history.item[i] = buffer + shell->parser.bufferSize * (i + 1);
    }



    #if defined(__GNUC__)
        shell->commandList.base = (ShellCommand *)(&_shell_command_start);
        shell->commandList.count = ((unsigned long)(&_shell_command_end)
                                - (unsigned long)(&_shell_command_start))
                                / sizeof(ShellCommand);
    #else
        #error not supported compiler, please use command table mode
    #endif


    shellAdd(shell);

    shellSetUser(shell, shellSeekCommand(shell,
                                         SHELL_DEFAULT_USER,
                                         shell->commandList.base,
                                         0));
    shellWriteCommandLine(shell, 1);
}


/**
 * @brief  Add shell
 * 
 * @param shell shell
 */
static void shellAdd(Shell *shell)
{
    for (short i = 0; i < SHELL_MAX_NUMBER; i++)
    {
        if (shellList[i] == NULL)
        {
            shellList[i] = shell;
            return;
        }
    }
}


/**
 * @brief Get the current active shell 
 * 
 * @return Shell* Currently active shell object 
 */
Shell* shellGetCurrent(void)
{
    for (short i = 0; i < SHELL_MAX_NUMBER; i++)
    {
        if (shellList[i] && shellList[i]->status.isActive)
        {
            return shellList[i];
        }
    }
    return NULL;
}


/**
 * @brief Shell write characters 
 * 
 * @param shell shell
 * @param data Character data 
 */
static void shellWriteByte(Shell *shell, const char data)
{
    shell->write(data);
}


/**
 * @brief shell Write string 
 * 
 * @param shell shell
 * @param string String data 
 * 
 * @return unsigned short Number of characters written (include '\n')
 */
unsigned short shellWriteString(Shell *shell, const char *string)
{
    unsigned short count = 0;
    CHECK(shell->write);
    while(*string)
    {
        shell->write(*string ++);
        count ++;
    }
    return count;
}


/**
 * @brief Shell write command description string 
 * 
 * @param shell shell 
 * @param string String data 
 * 
 * @return unsigned short Number of characters written 
 */
static unsigned short shellWriteCommandDesc(Shell *shell, const char *string)
{
    unsigned short count = 0;
    CHECK(shell->write);
    while(*string
        && *string != '\r'
        && *string != '\n'
        && count < CommandDescSize)
    {
        shell->write(*string ++);
        count ++;
        if (count >= CommandDescSize && *(string + 1))
        {
            shell->write('.');
            shell->write('.');
            shell->write('.');
        }
    }
    return count;
}

#ifdef FS_VFS
extern char working_dir[];
#endif

/**
 * @brief Shell write new command line
 * 
 * @param shell shell
 * @param newline Bool ,Whether to write a line 
 * 
 */
static void shellWriteCommandLine(Shell *shell, unsigned char newline)
{
    if (shell->status.isChecked)
    {
        if (newline)
        {
            shellWriteString(shell, "\r\n");
        }
        shellWriteString(shell, shell->info.user->data.user.name);
        shellWriteString(shell, ":");
#ifdef FS_VFS
        shellWriteString(shell,working_dir);
#endif
        shellWriteString(shell, "$ ");
    }
    else
    {
        shellWriteString(shell, shellText[SHELL_TEXT_PASSWORD_HINT]);
    }
}




/**
 * @brief shell Check command permissions 
 * 
 * @param shell shell
 * @param command ShellCommand
 * 
 * @return signed char 0 The current user has the command authority 
 * @return signec char -1 The current user does not have the command authority 
 */
signed char shellCheckPermission(Shell *shell, ShellCommand *command)
{
    return ((!command->attr.attrs.permission
                || command->attr.attrs.type == SHELL_TYPE_USER
                || (command->attr.attrs.permission 
                    & shell->info.user->attr.attrs.permission))
            && (shell->status.isChecked
                || command->attr.attrs.enableUnchecked))
            ? 0 : -1;
}


/**
 * @brief int to hexadecimal string 
 * 
 * @param value int value
 * @param buffer  hexadecimal result
 * 
 * @return signed char, data length after conversion 
 */
signed char shellToHex(unsigned int value, char *buffer)
{
    char byte;
    unsigned char i = 8;
    buffer[8] = 0;
    while (value)
    {
        byte = value & 0x0000000F;
        buffer[--i] = (byte > 9) ? (byte + 87) : (byte + 48);
        value >>= 4;
    }
    return 8 - i;
}


/**
* @brief int to decimal string 
 * 
 * @param value int value
 * @param buffer decimal result
 * 
 * @return  signed char, data length after conversion 
 */
signed char shellToDec(int value, char *buffer)
{
    unsigned char i = 11;
    int v = value;
    if (value < 0)
    {
        v = -value;
    }
    buffer[11] = 0;
    while (v)
    {
        buffer[--i] = v % 10 + 48;
        v /= 10;
    }
    if (value < 0)
    {
        buffer[--i] = '-';
    }
    if (value == 0) {
        buffer[--i] = '0';
    }
    return 11 - i;
}


/**
 * @brief shell string copy 
 * 
 * @param dest destination string 
 * @param src Source string 
 * @return Unsigned short ,String length 
 */
static unsigned short shellStringCopy(char *dest, char* src)
{
    unsigned short count = 0;
    while (*(src + count))
    {
        *(dest + count) = *(src + count);
        count++;
    }
    *(dest + count) = 0;
    return count;
}


/**
 * @brief Shell string comparison 
 * 
 * @param dest destination string 
 * @param src Source string 
 * @return unsigned short, Match length 
 */
static unsigned short shellStringCompare(char* dest, char *src)
{
    unsigned short match = 0;
    unsigned short i = 0;

    while (*(dest +i) && *(src + i))
    {
        if (*(dest + i) != *(src +i))
        {
            break;
        }
        match ++;
        i++;
    }
    return match;
}


/**
 * @brief shell get command name 
 * 
 * @param command command
 * @return const char* ,Command name 
 */
static const char* shellGetCommandName(ShellCommand *command)
{
    static char buffer[9];
    for (unsigned char i = 0; i < 9; i++)
    {
        buffer[i] = '0';
    }
    if (command->attr.attrs.type <= SHELL_TYPE_CMD_FUNC)
    {
        return command->data.cmd.name;
    }
    else if (command->attr.attrs.type <= SHELL_TYPE_VAR_NODE)
    {
        return command->data.var.name;
    }
    else if (command->attr.attrs.type <= SHELL_TYPE_USER)
    {
        return command->data.user.name;
    }
    else
    {
        shellToHex(command->data.key.value, buffer);
        return buffer;
    }
}


/**
 * @brief shell get command description
 * 
 * @param command command
 * @return const char* ,Command description
 */
static const char* shellGetCommandDesc(ShellCommand *command)
{
    if (command->attr.attrs.type <= SHELL_TYPE_CMD_FUNC)
    {
        return command->data.cmd.desc;
    }
    else if (command->attr.attrs.type <= SHELL_TYPE_VAR_NODE)
    {
        return command->data.var.desc;
    }
    else if (command->attr.attrs.type <= SHELL_TYPE_USER)
    {
        return command->data.user.desc;
    }
    else
    {
        return command->data.key.desc;
    }
}

/**
 * @brief shell list items
 * 
 * @param shell shell ojbect
 * @param item ShellCommand item(func,user,key,variable...)
 */
void shellListItem(Shell *shell, ShellCommand *item)
{
    short spaceLength;

    spaceLength = 22 - shellWriteString(shell, shellGetCommandName(item));
    spaceLength = (spaceLength > 0) ? spaceLength : 4;
    do {
        shellWriteByte(shell, ' ');
    } while (--spaceLength);
    if (item->attr.attrs.type <= SHELL_TYPE_CMD_FUNC)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_TYPE_CMD]);
    }
    else if (item->attr.attrs.type <= SHELL_TYPE_VAR_NODE)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_TYPE_VAR]);
    }
    else if (item->attr.attrs.type <= SHELL_TYPE_USER)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_TYPE_USER]);
    }
    else if (item->attr.attrs.type <= SHELL_TYPE_KEY)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_TYPE_KEY]);
    }
    else
    {
        shellWriteString(shell, shellText[SHELL_TEXT_TYPE_NONE]);
    }
#ifdef SHELL_HELP_SHOW_PERMISSION
    shellWriteString(shell, "  ");
    for (signed char i = 7; i >= 0; i--)
    {
        shellWriteByte(shell, item->attr.attrs.permission & (1 << i) ? 'x' : '-');
    }
#endif
    shellWriteString(shell, "  ");
    shellWriteCommandDesc(shell, shellGetCommandDesc(item));
    shellWriteString(shell, "\r\n");
}


/**
 * @brief shell lists all function commands 
 * 
 * @param shell shell
 */
void shellListCommand(Shell *shell)
{
    ShellCommand *base = (ShellCommand *)shell->commandList.base;
    shellWriteString(shell, shellText[SHELL_TEXT_CMD_LIST]);
    for (short i = 0; i < shell->commandList.count; i++)
    {
        if (base[i].attr.attrs.type <= SHELL_TYPE_CMD_FUNC
            && shellCheckPermission(shell, &base[i]) == 0)
        {
            shellListItem(shell, &base[i]);
        }
    }
}


/**
 * @brief shell list all variable
 * 
 * @param shell shell
 */
void shellListVar(Shell *shell)
{
    ShellCommand *base = (ShellCommand *)shell->commandList.base;
    shellWriteString(shell, shellText[SHELL_TEXT_VAR_LIST]);
    for (short i = 0; i < shell->commandList.count; i++)
    {
        if (base[i].attr.attrs.type > SHELL_TYPE_CMD_FUNC
            && base[i].attr.attrs.type <= SHELL_TYPE_VAR_NODE
            && shellCheckPermission(shell, &base[i]) == 0)
        {
            shellListItem(shell, &base[i]);
        }
    }
}


/**
 * @brief shell list all users
 * 
 * @param shell shell 
 */
void shellListUser(Shell *shell)
{
    ShellCommand *base = (ShellCommand *)shell->commandList.base;
    shellWriteString(shell, shellText[SHELL_TEXT_USER_LIST]);
    for (short i = 0; i < shell->commandList.count; i++)
    {
        if (base[i].attr.attrs.type > SHELL_TYPE_VAR_NODE
            && base[i].attr.attrs.type <= SHELL_TYPE_USER
            && shellCheckPermission(shell, &base[i]) == 0)
        {
            shellListItem(shell, &base[i]);
        }
    }
}


/**
 * @brief shell list all keys
 * 
 * @param shell shell 
 */
void shellListKey(Shell *shell)
{
    ShellCommand *base = (ShellCommand *)shell->commandList.base;
    shellWriteString(shell, shellText[SHELL_TEXT_KEY_LIST]);
    for (short i = 0; i < shell->commandList.count; i++)
    {
        if (base[i].attr.attrs.type > SHELL_TYPE_USER
            && base[i].attr.attrs.type <= SHELL_TYPE_KEY
            && shellCheckPermission(shell, &base[i]) == 0)
        {
            shellListItem(shell, &base[i]);
        }
    }
}


/**
 * @brief shell delete command line data 
 * 
 * @param shell shell
 * @param length  length of delete data
 */
void shellDeleteCommandLine(Shell *shell, unsigned char length)
{
    while (length--)
    {
        shellWriteString(shell, "\b \b");
    }
}


/**
 * @brief shell clear command line data
 * 
 * @param shell shell
 */
void shellClearCommandLine(Shell *shell)
{
    for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--)
    {
        shellWriteByte(shell, ' ');
    }
    shellDeleteCommandLine(shell, shell->parser.length);
}


/**
 * @brief shell Insert a character at the cursor position 
 * 
 * @param shell shell
 * @param data charactoer
 */
void shellInsertByte(Shell *shell, char data)
{
    /* if data is too long to insert a character */
    if (shell->parser.length >= shell->parser.bufferSize - 1)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_CMD_TOO_LONG]);
        shellWriteCommandLine(shell, 1);
        shellWriteString(shell, shell->parser.buffer);
        return;
    }

    /* insert character */
    if (shell->parser.cursor == shell->parser.length)
    {
        shell->parser.buffer[shell->parser.length++] = data;
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor++;
        shellWriteByte(shell, data);
    }
    else if (shell->parser.cursor < shell->parser.length)
    {
        for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shell->parser.buffer[shell->parser.cursor + i] = 
                shell->parser.buffer[shell->parser.cursor + i - 1];
        }
        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.buffer[++shell->parser.length] = 0;
        for (short i = shell->parser.cursor - 1; i < shell->parser.length; i++)
        {
            shellWriteByte(shell, shell->parser.buffer[i]);
        }
        for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shellWriteByte(shell, '\b');
        }
    }
}


/**
 * @brief shell delete character
 * 
 * @param shell shell
 * @param direction delete a character:
 *                                          {@code 1} Delete the character before the cursor 
 *                                          {@code -1}Delete the character after the cursor
 */
void shellDeleteByte(Shell *shell, signed char direction)
{
    char offset = (direction == -1) ? 1 : 0;

    if ((shell->parser.cursor == 0 && direction == 1)
        || (shell->parser.cursor == shell->parser.length && direction == -1))
    {
        return;
    }
    if (shell->parser.cursor == shell->parser.length && direction == 1)
    {
        shell->parser.cursor--;
        shell->parser.length--;
        shell->parser.buffer[shell->parser.length] = 0;
        shellDeleteCommandLine(shell, 1);
    }
    else
    {
        for (short i = offset; i < shell->parser.length - shell->parser.cursor; i++)
        {
            shell->parser.buffer[shell->parser.cursor + i - 1] = 
                shell->parser.buffer[shell->parser.cursor + i];
        }
        shell->parser.length--;
        if (!offset)
        {
            shell->parser.cursor--;
            shellWriteByte(shell, '\b');
        }
        shell->parser.buffer[shell->parser.length] = 0;
        for (short i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shellWriteByte(shell, shell->parser.buffer[i]);
        }
        shellWriteByte(shell, ' ');
        for (short i = shell->parser.length - shell->parser.cursor + 1; i > 0; i--)
        {
            shellWriteByte(shell, '\b');
        }
    }
}


/**
 * @brief shell Parsing parameters 
 * 
 * @param shell shell
 */
static void shellParserParam(Shell *shell)
{
    unsigned char quotes = 0;
    unsigned char record = 1;

    for (short i = 0; i < SHELL_PARAMETER_MAX_NUMBER; i++)
    {
        shell->parser.param[i] = NULL;
    }

    shell->parser.paramCount = 0;
    for (unsigned short i = 0; i < shell->parser.length; i++)
    {
        if (quotes != 0
            || (shell->parser.buffer[i] != ' '
                && shell->parser.buffer[i] != 0))
        {
            if (shell->parser.buffer[i] == '\"')
            {
                quotes = quotes ? 0 : 1;
            }
            if (record == 1)
            {
                if (shell->parser.paramCount < SHELL_PARAMETER_MAX_NUMBER)
                {
                    shell->parser.param[shell->parser.paramCount++] =
                        &(shell->parser.buffer[i]);
                }
                record = 0;
            }
            if (shell->parser.buffer[i] == '\\'
                && shell->parser.buffer[i + 1] != 0)
            {
                i++;
            }
        }
        else
        {
            shell->parser.buffer[i] = 0;
            record = 1;
        }
    }
}


/**
 * @brief shell Remove the double quotes at the beginning and end of string parameters 
 * 
 * @param shell shell
 */
static void shellRemoveParamQuotes(Shell *shell)
{
    unsigned short paramLength;
    for (unsigned short i = 0; i < shell->parser.paramCount; i++)
    {
        if (shell->parser.param[i][0] == '\"')
        {
            shell->parser.param[i][0] = 0;
            shell->parser.param[i] = &shell->parser.param[i][1];
        }
        paramLength = strlen(shell->parser.param[i]);
        if (shell->parser.param[i][paramLength - 1] == '\"')
        {
            shell->parser.param[i][paramLength - 1] = 0;
        }
    }
}


/**
 * @brief shell seeking command 
 * 
 * @param shell shell
 * @param cmd command
 * @param base command table base address 
 * @param compareLength Match string length 
 * @return ShellCommand* :find command 
 */
ShellCommand* shellSeekCommand(Shell *shell,
                               const char *cmd,
                               ShellCommand *base,
                               unsigned short compareLength)
{
    const char *name;
    unsigned short count = shell->commandList.count -
        ((long)base - (long)shell->commandList.base) / sizeof(ShellCommand);
    for (unsigned short i = 0; i < count; i++)
    {
        if (base[i].attr.attrs.type == SHELL_TYPE_KEY
            || shellCheckPermission(shell, &base[i]) != 0)
        {
            continue;
        }
        name = shellGetCommandName(&base[i]);
        if (!compareLength)
        {
            if (strcmp(cmd, name) == 0)
            {
                return &base[i];
            }
        }
        else
        {
            if (strncmp(cmd, name, compareLength) == 0)
            {
                return &base[i];
            }
        }
    }
    return NULL;
}


/**
 * @brief shell Get variable value 
 * 
 * @param shell shell
 * @param command command
 * @return int value
 */
int shellGetVarValue(Shell *shell, ShellCommand *command)
{
    int value = 0;
    switch (command->attr.attrs.type)
    {
    case SHELL_TYPE_VAR_INT:
        value = *((int *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_SHORT:
        value = *((short *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_CHAR:
        value = *((char *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_STRING:
    case SHELL_TYPE_VAR_POINT:
        value = (long)(command->data.var.value);
        break;
    case SHELL_TYPE_VAR_NODE:
        value = ((ShellNodeVarAttr *)command->data.var.value)->get ?
                    ((ShellNodeVarAttr *)command->data.var.value)
                        ->get(((ShellNodeVarAttr *)command->data.var.value)->var) : 0;
        break;
    default:
        break;
    }
    return value;
}


/**
 * @brief shell set variable value
 * 
 * @param shell shell
 * @param command command
 * @param value value
 * @return int value
 */
int shellSetVarValue(Shell *shell, ShellCommand *command, int value)
{
    if (command->attr.attrs.readOnly)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY]);
    }
    else
    {
        switch (command->attr.attrs.type)
        {
        case SHELL_TYPE_VAR_INT:
            *((int *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_SHORT:
            *((short *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_CHAR:
            *((char *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_STRING:
            shellStringCopy(((char *)(command->data.var.value)), (char *)(long)value);
            break;
        case SHELL_TYPE_VAR_POINT:
            shellWriteString(shell, shellText[SHELL_TEXT_POINT_CANNOT_MODIFY]);
            break;
        case SHELL_TYPE_VAR_NODE:
            if (((ShellNodeVarAttr *)command->data.var.value)->set)
            {
                if (((ShellNodeVarAttr *)command->data.var.value)->var)
                {
                    ((ShellNodeVarAttr *)command->data.var.value)
                        ->set(((ShellNodeVarAttr *)command->data.var.value)->var, value);
                }
                else
                {
                    ((ShellNodeVarAttr *)command->data.var.value)->set(value);
                }
            }
            break;
        default:
            break;
        }
    }
    return shellShowVar(shell, command);
}


/**
 * @brief shell print variable value
 * 
 * @param shell shell
 * @param command command
 * @return int value
 */
static int shellShowVar(Shell *shell, ShellCommand *command)
{
    char buffer[12] = "00000000000";
    int value = shellGetVarValue(shell, command);
    
    shellWriteString(shell, command->data.var.name);
    shellWriteString(shell, " = ");

    switch (command->attr.attrs.type)
    {
    case SHELL_TYPE_VAR_STRING:
        shellWriteString(shell, "\"");
        shellWriteString(shell, (char *)(long)value);
        shellWriteString(shell, "\"");
        break;
    // case SHELL_TYPE_VAR_INT:
    // case SHELL_TYPE_VAR_SHORT:
    // case SHELL_TYPE_VAR_CHAR:
    // case SHELL_TYPE_VAR_POINT:
    default:
        shellWriteString(shell, &buffer[11 - shellToDec(value, buffer)]);
        shellWriteString(shell, ", 0x");
        for (short i = 0; i < 11; i++)
        {
            buffer[i] = '0';
        }
        shellToHex(value, buffer);
        shellWriteString(shell, buffer);
        break;
    }

    shellWriteString(shell, "\r\n");
    return value;
}


/**
 * @brief shell set variable value
 * 
 * @param name value name
 * @param value value
 * @return int value
 */
int shellSetVar(char *name, int value)
{
    Shell *shell = shellGetCurrent();
    if (shell == NULL)
    {
        return 0;
    }
    ShellCommand *command = shellSeekCommand(shell,
                                             name,
                                             shell->commandList.base,
                                             0);
    if (!command)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_VAR_NOT_FOUND]);
        return 0;
    }
    if (command->attr.attrs.type < SHELL_TYPE_VAR_INT
        || command->attr.attrs.type > SHELL_TYPE_VAR_NODE)
    {
        shellWriteString(shell, name);
        shellWriteString(shell, shellText[SHELL_TEXT_NOT_VAR]);
        return 0;
    }
    return shellSetVarValue(shell, command, value);
}
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
setVar, shellSetVar, set var);


/**
 * @brief shell run command
 * 
 * @param shell shell
 * @param command command
 * 
 * @return unsigned int command return value
 */
unsigned int shellRunCommand(Shell *shell, ShellCommand *command)
{
    int returnValue = 0;
    shell->status.isActive = 1;
    if (command->attr.attrs.type == SHELL_TYPE_CMD_MAIN)
    {
        shellRemoveParamQuotes(shell);
        returnValue = command->data.cmd.function(shell->parser.paramCount,
                                                 shell->parser.param);
        if (!command->attr.attrs.disableReturn)
        {
            shellWriteReturnValue(shell, returnValue);
        }
    }
    else if (command->attr.attrs.type == SHELL_TYPE_CMD_FUNC)
    {
        returnValue = shellExtRun(shell,
                                  command,
                                  shell->parser.paramCount,
                                  shell->parser.param);
        if (!command->attr.attrs.disableReturn)
        {
            shellWriteReturnValue(shell, returnValue);
        }
    }
    else if (command->attr.attrs.type >= SHELL_TYPE_VAR_INT
        && command->attr.attrs.type <= SHELL_TYPE_VAR_NODE)
    {
        shellShowVar(shell, command);
    }
    else if (command->attr.attrs.type == SHELL_TYPE_USER)
    {
        shellSetUser(shell, command);
    }
    shell->status.isActive = 0;

    return returnValue;
}


/**
 * @brief shell check password
 * 
 * @param shell shell
 */
static void shellCheckPassword(Shell *shell)
{
    if (strcmp(shell->parser.buffer, shell->info.user->data.user.password) == 0)
    {
        shell->status.isChecked = 1;
        shellWriteString(shell, shellText[SHELL_TEXT_INFO]);
    }
    else
    {
        shellWriteString(shell, shellText[SHELL_TEXT_PASSWORD_ERROR]);
    }
    shell->parser.length = 0;
    shell->parser.cursor = 0;
}


/**
 * @brief shell set user 
 * 
 * @param shell shell
 * @param user user
 */
static void shellSetUser(Shell *shell, const ShellCommand *user)
{
    shell->info.user = user;
    shell->status.isChecked = 
        ((user->data.user.password && strlen(user->data.user.password) != 0)
            && (shell->parser.paramCount < 2
                || strcmp(user->data.user.password, shell->parser.param[1]) != 0))
         ? 0 : 1;
        
    if (shell->status.isChecked)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_INFO]);
    }
}


/**
 * @brief shell write return value
 * 
 * @param shell shell
 * @param value value
 */
static void shellWriteReturnValue(Shell *shell, int value)
{
    char buffer[12] = "00000000000";
    shellWriteString(shell, "Return: ");
    shellWriteString(shell, &buffer[11 - shellToDec(value, buffer)]);
    shellWriteString(shell, ", 0x");
    for (short i = 0; i < 11; i++)
    {
        buffer[i] = '0';
    }
    shellToHex(value, buffer);
    shellWriteString(shell, buffer);
    shellWriteString(shell, "\r\n");
}


/**
 * @brief shell add history
 * 
 * @param shell shell
 */
static void shellHistoryAdd(Shell *shell)
{
    shell->history.offset = 0;
    if (shell->history.number > 0
        && strcmp(shell->history.item[(shell->history.record == 0 ? 
                SHELL_HISTORY_MAX_NUMBER : shell->history.record) - 1],
                shell->parser.buffer) == 0)
    {
        return;
    }
    if (shellStringCopy(shell->history.item[shell->history.record],
                        shell->parser.buffer) != 0)
    {
        shell->history.record++;
    }
    if (++shell->history.number > SHELL_HISTORY_MAX_NUMBER)
    {
        shell->history.number = SHELL_HISTORY_MAX_NUMBER;
    }
    if (shell->history.record >= SHELL_HISTORY_MAX_NUMBER)
    {
        shell->history.record = 0;
    }
}


/**
 * @brief shell history find
 * 
 * @param shell shell
 * @param dir directory:{@code <0}UP, {@code >0}Down
 */
static void shellHistory(Shell *shell, signed char dir)
{
    if (dir > 0)
    {
        if (shell->history.offset-- <= 
            -((shell->history.number > shell->history.record) ?
                shell->history.number : shell->history.record))
        {
            shell->history.offset = -((shell->history.number > shell->history.record)
                                    ? shell->history.number : shell->history.record);
        }
    }
    else if (dir < 0)
    {
        if (++shell->history.offset > 0)
        {
            shell->history.offset = 0;
            return;
        }
    }
    else
    {
        return;
    }
    shellClearCommandLine(shell);
    if (shell->history.offset == 0)
    {
        shell->parser.cursor = shell->parser.length = 0;
    }
    else
    {
        if ((shell->parser.length = shellStringCopy(shell->parser.buffer,
                shell->history.item[(shell->history.record + SHELL_HISTORY_MAX_NUMBER
                    + shell->history.offset) % SHELL_HISTORY_MAX_NUMBER])) == 0)
        {
            return;
        }
        shell->parser.cursor = shell->parser.length;
        shellWriteString(shell, shell->parser.buffer);
    }
    
}


/**
 * @brief shell normalinput
 * 
 * @param shell shell 
 * @param data input character
 */
void shellNormalInput(Shell *shell, char data)
{
    shell->status.tabFlag = 0;
    shellInsertByte(shell, data);
}


/**
 * @brief shell run command
 * 
 * @param shell shell
 */
void shellExec(Shell *shell)
{
    
    if (shell->parser.length == 0)
    {
        return;
    }

    shell->parser.buffer[shell->parser.length] = 0;

    if (shell->status.isChecked)
    {
        shellHistoryAdd(shell);
        shellParserParam(shell);
        shell->parser.length = shell->parser.cursor = 0;
        if (shell->parser.paramCount == 0)
        {
            return;
        }
        shellWriteString(shell, "\r\n");

        ShellCommand *command = shellSeekCommand(shell,
                                                 shell->parser.param[0],
                                                 shell->commandList.base,
                                                 0);
        if (command != NULL)
        {
            shellRunCommand(shell, command);
        }
        else
        {
            shellWriteString(shell, shellText[SHELL_TEXT_CMD_NOT_FOUND]);
        }
    }
    else
    {
        shellCheckPassword(shell);
    }
}


/**
 * @brief shell Previous history 
 * 
 * @param shell shell
 */
void shellUp(Shell *shell)
{
    shellHistory(shell, 1);
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x1B5B4100, shellUp, up);


/**
 * @brief shell Next
 * 
 * @param shell shell
 */
void shellDown(Shell *shell)
{
    shellHistory(shell, -1);
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x1B5B4200, shellDown, down);


/**
 * @brief shell input right key
 * 
 * @param shell shell
 */
void shellRight(Shell *shell)
{
    if (shell->parser.cursor < shell->parser.length)
    {
        shellWriteByte(shell, shell->parser.buffer[shell->parser.cursor++]);
    }
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x1B5B4300, shellRight, right);


/**
 * @brief shell input left key
 * 
 * @param shell shell
 */
void shellLeft(Shell *shell)
{
    if (shell->parser.cursor > 0)
    {
        shellWriteByte(shell, '\b');
        shell->parser.cursor--;
    }
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x1B5B4400, shellLeft, left);


/**
 * @brief shell Tab key function
 * 
 * @param shell shell
 */
void shellTab(Shell *shell)
{
    unsigned short maxMatch = shell->parser.bufferSize;
    unsigned short lastMatchIndex = 0;
    unsigned short matchNum = 0;
    unsigned short length;

    if (shell->parser.length == 0)
    {
        shellListCommand(shell);
        shellWriteCommandLine(shell, 1);
    }
    else if (shell->parser.length > 0)
    {
        shell->parser.buffer[shell->parser.length] = 0;
        ShellCommand *base = (ShellCommand *)shell->commandList.base;
        for (short i = 0; i < shell->commandList.count; i++)
        {
            if (shellCheckPermission(shell, &base[i]) == 0
                && shellStringCompare(shell->parser.buffer,
                                   (char *)shellGetCommandName(&base[i]))
                        == shell->parser.length)
            {
                if (matchNum != 0)
                {
                    if (matchNum == 1)
                    {
                        shellWriteString(shell, "\r\n");
                    }
                    shellListItem(shell, &base[lastMatchIndex]);
                    length = 
                        shellStringCompare((char *)shellGetCommandName(&base[lastMatchIndex]),
                                           (char *)shellGetCommandName(&base[i]));
                    maxMatch = (maxMatch > length) ? length : maxMatch;
                }
                lastMatchIndex = i;
                matchNum++;
            }
        }
        if (matchNum == 0)
        {
            return;
        }
        if (matchNum == 1)
        {
            shellClearCommandLine(shell);
        }
        if (matchNum != 0)
        {
            shell->parser.length = 
                shellStringCopy(shell->parser.buffer,
                                (char *)shellGetCommandName(&base[lastMatchIndex]));
        }
        if (matchNum > 1)
        {
            shellListItem(shell, &base[lastMatchIndex]);
            shellWriteCommandLine(shell, 1);
            shell->parser.length = maxMatch;
        }
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor = shell->parser.length;
        shellWriteString(shell, shell->parser.buffer);
    }

    if (SHELL_GET_TICK())
    {
        if (matchNum == 1
            && shell->status.tabFlag
            && SHELL_GET_TICK() - shell->info.activeTime < 200)
        {
            shellClearCommandLine(shell);
            for (short i = shell->parser.length; i >= 0; i--)
            {
                shell->parser.buffer[i + 5] = shell->parser.buffer[i];
            }
            shellStringCopy(shell->parser.buffer, "help");
            shell->parser.buffer[4] = ' ';
            shell->parser.length += 5;
            shell->parser.cursor = shell->parser.length;
            shellWriteString(shell, shell->parser.buffer);
        }
        else
        {
            shell->status.tabFlag = 1;
        }
    }
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x09000000, shellTab, tab);


/**
 * @brief shell backspace 
 * 
 * @param shell shell
 */
void shellBackspace(Shell *shell)
{
    shellDeleteByte(shell, 1);
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x08000000, shellBackspace, backspace);
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x7F000000, shellBackspace, backspace);


/**
 * @brief shell delet
 * 
 * @param shell shell
 */
void shellDelete(Shell *shell)
{
    shellDeleteByte(shell, -1);
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x1B5B337E, shellDelete, delete);


/**
 * @brief shell Enter key
 * 
 * @param shell shell
 */
void shellEnter(Shell *shell)
{
    shellExec(shell);
    shellWriteCommandLine(shell, 1);
}
#ifdef SHELL_ENTER_LF 
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x0A000000, shellEnter, enter);
#endif
#ifdef SHELL_ENTER_CR 
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x0D000000, shellEnter, enter);
#endif
#ifdef SHELL_ENTER_CRLF 
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0)|SHELL_CMD_ENABLE_UNCHECKED,
0x0D0A0000, shellEnter, enter);
#endif


/**
 * @brief shell help
 * 
 * @param argc Number of parameters 
 * @param argv parameter 
 */
void shellHelp(int argc, char *argv[])
{
    Shell *shell = shellGetCurrent();
    CHECK(shell);
    if (argc == 1)
    {
        shellListCommand(shell);
    }
    else if (argc > 1)
    {
        ShellCommand *command = shellSeekCommand(shell,
                                                 argv[1],
                                                 shell->commandList.base,
                                                 0);
        shellWriteString(shell, shellText[SHELL_TEXT_HELP_HEADER]);
        shellWriteString(shell, shellGetCommandName(command));
        shellWriteString(shell, "\r\n");
        shellWriteString(shell, shellGetCommandDesc(command));
        shellWriteString(shell, "\r\n");
    }
}
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
Help, shellHelp, show command info\r\nhelp [cmd]);

/**
 * @brief shell Input processing 
 * 
 * @param shell shell
 * @param data input data
 */
void shellHandler(Shell *shell, char data)
{
    CHECK(data);
    

#if SHELL_LOCK_TIMEOUT > 0
    if (shell->info.user->data.user.password
        && strlen(shell->info.user->data.user.password) != 0
        && SHELL_GET_TICK())
    {
        if (SHELL_GET_TICK() - shell->info.activeTime > SHELL_LOCK_TIMEOUT)
        {
            shell->status.isChecked = 0;
        }
    }
#endif

    /* Calculate the offset of the current byte in the key value according to the recorded key value  */
    char keyByteOffset = 24;
    int keyFilter = 0x00000000;
    if ((shell->parser.keyValue & 0x0000FF00) != 0x00000000)
    {
        keyByteOffset = 0;
        keyFilter = 0xFFFFFF00;
    }
    else if ((shell->parser.keyValue & 0x00FF0000) != 0x00000000)
    {
        keyByteOffset = 8;
        keyFilter = 0xFFFF0000;
    }
    else if ((shell->parser.keyValue & 0xFF000000) != 0x00000000)
    {
        keyByteOffset = 16;
        keyFilter = 0xFF000000;
    }

    /* Traverse the Shell Command list and try to match the key value  */
    ShellCommand *base = (ShellCommand *)shell->commandList.base;
    for (short i = 0; i < shell->commandList.count; i++)
    {
        /* Determine whether it is a key definition and verify permissions  */
        if (base[i].attr.attrs.type == SHELL_TYPE_KEY
            && shellCheckPermission(shell, &(base[i])) == 0)
        {
            /* Match the entered byte with the key value  */
            if ((base[i].data.key.value & keyFilter) == shell->parser.keyValue
                && (base[i].data.key.value & (0xFF << keyByteOffset))
                    == (data << keyByteOffset))
            {
                shell->parser.keyValue |= data << keyByteOffset;
                data = 0x00;
                if (keyByteOffset == 0 
                    || (base[i].data.key.value & (0xFF << (keyByteOffset - 8)))
                        == 0x00000000)
                {
                    if (base[i].data.key.function)
                    {
                        base[i].data.key.function(shell);
                    }
                    shell->parser.keyValue = 0x00000000;
                    break;
                }
            }
        }
    }

    if (data != 0x00)
    {
        shell->parser.keyValue = 0x00000000;
        shellNormalInput(shell, data);
    }

    if (SHELL_GET_TICK())
    {
        shell->info.activeTime = SHELL_GET_TICK();
    }
}





/**
 * @brief shell task
 * 
 * @param param parameter(shell)
 * 
 */
void shellTask(void *param)
{
    // KPrintf("this is in 1733");
    Shell *shell = (Shell *)param;
    char data;
    while(RET_TRUE)
    {
        if (shell->read && shell->read(&data) == 0)
        {
            //   KPrintf("in 1741 the char is:                    '%c' and ascii code is %d.\n\n",data,data);
            // KPrintf("the buffer is:'%s'\n\n",shell->parser.);
           shellHandler(shell, data);
        }
    }
}


/**
 * @brief Output user list (shell call) 
 */
void shellUsers(void)
{
    Shell *shell = shellGetCurrent();
    if (shell)
    {
        shellListUser(shell);
    }
}
#ifdef SHELL_HELP_LIST_USER
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
users, shellUsers, list all user);
#endif



/**
 * @brief Output variable list (shell call) 
 */
void shellVars(void)
{
    Shell *shell = shellGetCurrent();
    if (shell)
    {
        shellListVar(shell);
    }
}
#ifdef SHELL_HELP_LIST_VAR
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
vars, shellVars, list all var);
#endif


/**
 * @brief Output key list (shell call) 
 */
void shellKeys(void)
{
    Shell *shell = shellGetCurrent();
    if (shell)
    {
        shellListKey(shell);
    }
}
#ifdef SHELL_HELP_LIST_KEY
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
keys, shellKeys, list all key);
#endif

/**
 * @brief Clear the console (shell call) 
 */
void shellClear(void)
{
    Shell *shell = shellGetCurrent();
    if (shell)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_CLEAR_CONSOLE]);
    }
}
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
clear, shellClear, clear console);

