/**
 * @file shell_ext.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief shell extensions
 * @version 3.0.0
 * @date 2019-12-31
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#include "shell.h"
#include "shell_ext.h"


extern ShellCommand* shellSeekCommand(Shell *shell,
                                      const char *cmd,
                                      ShellCommand *base,
                                      unsigned short compareLength);
extern int shellGetVarValue(Shell *shell, ShellCommand *command);

/**
 * @brief Judging the number base 
 * 
 * @param string Parameter string 
 * @return NUM_Type Base
 */
static NUM_Type shellExtNumType(char *string)
{
    char *p = string;
    NUM_Type type = NUM_TYPE_INT;

    if ((*p == '0') && ((*(p + 1) == 'x') || (*(p + 1) == 'X')))
    {
        type = NUM_TYPE_HEX;
    }
    else if ((*p == '0') && ((*(p + 1) == 'b') || (*(p + 1) == 'B')))
    {
        type = NUM_TYPE_BIN;
    }
    else if (*p == '0')
    {
        type = NUM_TYPE_OCT;
    }
    
    while (*p++)
    {
        if (*p == '.' && *(p + 1) != 0)
        {
            type = NUM_TYPE_FLOAT;
            break;
        }
    }

    return type;
}


/**
 * @brief character  to number 
 * 
 * @param code character 
 * @return char number
 */
static char shellExtToNum(char code)
{
    if ((code >= '0') && (code <= '9'))
    {
        return code -'0';
    }
    else if ((code >= 'a') && (code <= 'f'))
    {
        return code - 'a' + 10;
    }
    else if ((code >= 'A') && (code <= 'F'))
    {
        return code - 'A' + 10;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief Parse character parameters 
 * 
 * @param string String parameter 
 * @return char Parsed character 
 */
static char shellExtParseChar(char *string)
{
    char *p = string + 1;
    char value = 0;

    if (*p == '\\')
    {
        switch (*(p + 1))
        {
        case 'b':
            value = '\b';
            break;
        case 'r':
            value = '\r';
            break;
        case 'n':
            value = '\n';
            break;
        case 't':
            value = '\t';
            break;
        case '0':
            value = 0;
            break;
        default:
            value = *(p + 1);
            break;
        }
    }
    else
    {
        value = *p;
    }
    return value;
}


/**
 * @brief Parse string parameters 
 * 
 * @param string String parameter 
 * @return char* Parsed string 
 */
static char* shellExtParseString(char *string)
{
    char *p = string;
    unsigned short index = 0;

    if (*string == '\"')
    {
        p = ++string;
    }

    while (*p)
    {
        if (*p == '\\')
        {
            *(string + index) = shellExtParseChar(p - 1);
            p++;
        }
        else if (*p == '\"')
        {
            *(string + index) = 0;
        }
        else
        {
            *(string + index) = *p;
        }
        p++;
        index ++;
    }
    *(string + index) = 0;
    return string;
}


/**
 * @brief Parse numeric parameters 
 * 
 * @param string String parameter 
 * @return unsigned int Parsed number 
 */
static unsigned int shellExtParseNumber(char *string)
{
    NUM_Type type = NUM_TYPE_INT;
    char radix = 10;
    char *p = string;
    char offset = 0;
    signed char sign = 1;
    unsigned int valueInt = 0;
    float valueFloat = 0.0;
    unsigned int devide = 0;

    if (*string == '-')
    {
        sign = -1;
    }

    type = shellExtNumType(string + ((sign == -1) ? 1 : 0));

    switch ((char)type)
    {
    case NUM_TYPE_HEX:
        radix = 16;
        offset = 2;
        break;
    
    case NUM_TYPE_OCT:
        radix = 8;
        offset = 1;
        break;

    case NUM_TYPE_BIN:
        radix = 2;
        offset = 2;
        break;
    
    default:
        break;
    }

    p = string + offset + ((sign == -1) ? 1 : 0);

    while (*p)
    {
        if (*p == '.')
        {
            devide = 1;
            p++;
            continue;
        }
        valueInt = valueInt * radix + shellExtToNum(*p);
        devide *= 10;
        p++;
    }
    if (type == NUM_TYPE_FLOAT && devide != 0)
    {
        valueFloat = (float)valueInt / devide * sign;
        return *(unsigned int *)(&valueFloat);
    }
    else
    {
        return valueInt * sign;
    }
}


/**
 * @brief Analyze variable parameters 

 * 
 * @param shell shell object 
 * @param var variable 
 * @return unsigned int variable value
 */
static unsigned int shellExtParseVar(Shell *shell, char *var)
{
    ShellCommand *command = shellSeekCommand(shell,
                                             var + 1,
                                             shell->commandList.base,
                                             0);
    if (command)
    {
        return shellGetVarValue(shell, command);
    }
    else
    {
        return 0;
    }
}


/**
 * @brief Parsing parameters 
 * 
 * @param shell shell
 * @param string parameter 
 * @return unsigned int Analysis result 
 */
unsigned int shellExtParsePara(Shell *shell, char *string)
{
    if (*string == '\'' && *(string + 1))
    {
        return (unsigned int)shellExtParseChar(string);
    }
    else if (*string == '-' || (*string >= '0' && *string <= '9'))
    {
        return (unsigned int)shellExtParseNumber(string);
    }
    else if (*string == '$' && *(string + 1))
    {
        return shellExtParseVar(shell, string);
    }
    else if (*string)
    {
        return (unsigned long)shellExtParseString(string);
    }
    return 0;
}


/**
 * @brief Excuting function
 * 
 * @param shell shell
 * @param command  command
 * @param argc Number of parameters 
 * @param argv parameter 
 * @return int return value
 */
int shellExtRun(Shell *shell, ShellCommand *command, int argc, char *argv[])
{
    unsigned int params[8] = {0};
    int paramNum = command->attr.attrs.paramNum > (argc - 1) ? 
        command->attr.attrs.paramNum : (argc - 1);
    for (int i = 0; i < argc - 1; i++)
    {
        params[i] = shellExtParsePara(shell, argv[i + 1]);
    }
    switch (paramNum)
    {
    case 0:
        return command->data.cmd.function();
        // break;
    case 1:
        return command->data.cmd.function(params[0]);
        // break;
    case 2:
        return command->data.cmd.function(params[0], params[1]);
        // break;
    case 3:
        return command->data.cmd.function(params[0], params[1],
                                          params[2]);
        // break;
    case 4:
        return command->data.cmd.function(params[0], params[1],
                                          params[2], params[3]);
        // break;
    case 5:
        return command->data.cmd.function(params[0], params[1],
                                          params[2], params[3],
                                          params[4]);
        // break;
    case 6:
        return command->data.cmd.function(params[0], params[1],
                                          params[2], params[3],
                                          params[4], params[5]);
        // break;
    case 7:
        return command->data.cmd.function(params[0], params[1],
                                          params[2], params[3],
                                          params[4], params[5],
                                          params[6]);
        // break;
    default:
        return -1;
        // break;
    }
}

