/**
 * @file shell.h
 * @author Letter (NevermindZZT@gmail.com)
 * @brief letter shell
 * @version 3.0.0
 * @date 2019-12-30
 * 
 * @copyright (c) 2020 Letter
 * 
 */

#ifndef     __SHELL_H__
#define     __SHELL_H__

#include "xsconfig.h"
#include "stdlib.h"

#define     SHELL_VERSION               "3.0.5"                 /**< version */


/**
 * @brief get system time (ms)
 *        
 * @note When this macro is not defined, 
 * you cannot use double-clicking the tab to complete the command help, 
 * and you cannot use the shell timeout lock
 */
#define     SHELL_GET_TICK()            CalculteTimeMsFromTick(CurrentTicksGain())


/**
 * @brief shell permitt
 * 
 * @param permission permission level
 */
#define     SHELL_CMD_PERMISSION(permission) \
            (permission & 0x000000FF)

/**
 * @brief shell COMMAND TYPE
 * 
 * @param type type of command
 */
#define     SHELL_CMD_TYPE(type) \
            ((type & 0x0000000F) << 8)

/**
 * @brief Enable command without verifying the password
 */
#define     SHELL_CMD_ENABLE_UNCHECKED \
            (1 << 12)

/**
 * @brief Disable print return value  
 */
#define     SHELL_CMD_DISABLE_RETURN \
            (1 << 13)

/**
 * @brief Read-only attributes (only valid for variables) 
 */
#define     SHELL_CMD_READ_ONLY \
            (1 << 14)

/**
 * @brief Number of command parameters 
 */
#define     SHELL_CMD_PARAM_NUM(num) \
            ((num & 0x0000000F)) << 16

    /**
     * @brief shell Command definition 
     * 
     * @param _attr Command attributes 
     * @param _name Command name 
     * @param _func Command function 
     * @param _desc Command description 
     */
    #define SHELL_EXPORT_CMD(_attr, _name, _func, _desc) \
            const char shellCmd##_name[] = #_name; \
            const char shellDesc##_name[] = #_desc; \
            const ShellCommand \
            shellCommand##_name SECTION("shellCommand") =  \
            { \
                .attr.value = _attr, \
                .data.cmd.name = shellCmd##_name, \
                .data.cmd.function = (int (*)())_func, \
                .data.cmd.desc = shellDesc##_name \
            }


    /**
     * @brief shell variable definition
     * 
     * @param _attr Command attributes
     * @param _name Command name
     * @param _value Command function
     * @param _desc Command description
     */
    #define SHELL_EXPORT_VAR(_attr, _name, _value, _desc) \
            const char shellCmd##_name[] = #_name; \
            const char shellDesc##_name[] = #_desc; \
            const ShellCommand \
            shellVar##_name SECTION("shellCommand") =  \
            { \
                .attr.value = _attr, \
                .data.var.name = shellCmd##_name, \
                .data.var.value = (void *)_value, \
                .data.var.desc = shellDesc##_name \
            }

 

    /**
     * @brief shell Key definition 
     * 
     * @param _attr Key attributes 
     * @param _value Key value 
     * @param _func Key function 
     * @param _desc Key description 
     */
    #define SHELL_EXPORT_KEY(_attr, _value, _func, _desc) \
            const char shellDesc##_value[] = #_desc; \
            const ShellCommand \
            shellKey##_value SECTION("shellCommand") =  \
            { \
                .attr.value = _attr|SHELL_CMD_TYPE(SHELL_TYPE_KEY), \
                .data.key.value = _value, \
                .data.key.function = (void (*)(Shell *))_func, \
                .data.key.desc = shellDesc##_value \
            }



/**
 * @brief shell command type
 */
typedef enum
{
    SHELL_TYPE_CMD_MAIN = 0,                                    /**< main function form command  */
    SHELL_TYPE_CMD_FUNC,                                        /**< C function form command  */
    SHELL_TYPE_VAR_INT,                                         /**< int variable  */
    SHELL_TYPE_VAR_SHORT,                                       /**< short variable */
    SHELL_TYPE_VAR_CHAR,                                        /**< char variable */
    SHELL_TYPE_VAR_STRING,                                      /**< string variable */
    SHELL_TYPE_VAR_POINT,                                       /**< point variable */
    SHELL_TYPE_VAR_NODE,                                        /**< node variable*/
    SHELL_TYPE_USER,                                            /**< user */
    SHELL_TYPE_KEY,                                             /**< Key */
} ShellCommandType;


/**
 * @brief Shell definition
 */
typedef struct shell_def
{
    struct
    {
        const struct shell_command *user;                       /**< Current user  */
        int activeTime;                                         /**< shell activation time  */
        char *path;                                             /**< Current shell path  */
    
    } info;
    struct
    {
        unsigned short length;                                  /**< Input data length  */
        unsigned short cursor;                                  /**< Current cursor position  */
        char *buffer;                                           /**< Input buffer  */
        char *param[SHELL_PARAMETER_MAX_NUMBER];                /**< parameter  */
        unsigned short bufferSize;                              /**< Input buffer size  */
        unsigned short paramCount;                              /**< Number of parameters  */
        int keyValue;                                           /**< Enter key value  */
    } parser;
    struct
    {
        char *item[SHELL_HISTORY_MAX_NUMBER];                   /**< history record  */
        unsigned short number;                                  /**< Number of history records  */
        unsigned short record;                                  /**< Current record location  */
        signed short offset;                                    /**< Current history offset  */
    } history;
    struct
    {
        void *base;                                             /**< Command table base address  */
        unsigned short count;                                   /**< Number of commands  */
    } commandList;
    struct
    {
        unsigned char isChecked : 1;                            /**< Password verification passed  */
        unsigned char isActive : 1;                             /**< Current active Shell  */
        unsigned char tabFlag : 1;                              /**< tab Flag */
    } status;
    signed char (*read)(char *);                                /**< shell read function */
    void (*write)(const char);                                  /**< shell write function */
} Shell;


/**
 * @brief shell command definition
 */
typedef struct shell_command
{
    union
    {
        struct
        {
            unsigned char permission : 8;                       /**< command permission */
            ShellCommandType type : 4;                          /**< command type*/
            unsigned char enableUnchecked : 1;                  /**< Enable when Unchecked */
            unsigned char disableReturn : 1;                    /**< Disable return value output  */
            unsigned char  readOnly : 1;                        /**< readOnly */
            unsigned char reserve : 1;                          /**< reserve*/
            unsigned char paramNum : 4;                         /**< Number of parameters  */
        } attrs;
        int value;
    } attr;                                                     /**< attribution */
    union
    {
        struct
        {
            const char *name;                                   /**< name */
            int (*function)();                                  /**< function  */
            const char *desc;                                   /**< Command description  */
        } cmd;                                                  /**< Command definition */
        struct
        {
            const char *name;                                   /**< varibale name */
            void *value;                                        /**< value of variable */
            const char *desc;                                   /**< Variable description  */
        } var;                                                  /**< variable definition */
        struct
        {
            const char *name;                                   /**< user name */
            const char *password;                               /**< user's password */
            const char *desc;                                   /**< user's description */
        } user;                                                 /**< user definition */
        struct
        {
            int value;                                          /**< key value */
            void (*function)(Shell *);                          /**< funciton for key */
            const char *desc;                                   /**< key description */
        } key;                                                  /**< key definition */
    } data; 
} ShellCommand;

/**
 * @brief Shell node variable attributes 
 */
typedef struct
{
    void *var;                                                  /**< Variable reference  */
    int (*get)();                                               /**< Variable get method  */
    int (*set)();                                               /**< Variable set method  */
} ShellNodeVarAttr;


#define shellSetPath(_shell, _path)     (_shell)->info.path = _path
#define shellGetPath(_shell)            ((_shell)->info.path)

void shellInit(Shell *shell, char *buffer, unsigned short size);
unsigned short shellWriteString(Shell *shell, const char *string);
Shell* shellGetCurrent(void);
void shellHandler(Shell *shell, char data);
void shellTask(void *param);




signed char shellCompanionAdd(Shell *shell, int id, void *object);
signed char shellCompanionDel(Shell *shell, int id);
void *shellCompanionGet(Shell *shell, int id);


#endif

