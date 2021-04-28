# letter shell 3.0

![version](https://img.shields.io/badge/version-3.0.5-brightgreen.svg)
![standard](https://img.shields.io/badge/standard-c99-brightgreen.svg)
![build](https://img.shields.io/badge/build-2020.08.23-brightgreen.svg)
![license](https://img.shields.io/badge/license-MIT-brightgreen.svg)

一个功能强大的嵌入式shell

## 简介

[letter shell 3.0](https://github.com/NevermindZZT/letter-shell/tree/shell3.0)是一个C语言编写的，可以嵌入在程序中的嵌入式shell，主要面向嵌入式设备，以C语言函数为运行单位，可以通过命令行调用，运行程序中的函数

该移植适配了Xiuos操作系统，并进行了精简。

原始版本的使用说明可参考[Letter shell 3.0 全新出发](https://nevermindzzt.github.io/2020/01/19/Letter%20shell%203.0%E5%85%A8%E6%96%B0%E5%87%BA%E5%8F%91/)

## 功能

- 命令自动补全
- 快捷键功能定义
- 命令权限管理
- 用户管理
- 变量支持
- 文件系统的支持

## 配置选项()

1. 配置宏

   下面列出了用于配置shell的宏，在使用前，可用menuconfig进行配置


   | 宏 | 意义 |
   | - | - |
   | TOOL_SHELL | 是否开启shell(默认y) |
   |   |   |
   | SHELL_HELP_LIST_USER | 是否在输入命令列表中列出用户 |
   | SHELL_HELP_LIST_VAR | 是否在输入命令列表中列出变量 |
   | SHELL_HELP_LIST_KEY | 是否在输入命令列表中列出按键 |
   | SHELL_ENTER_LF | 使用LF作为命令行回车触发 |
   | SHELL_ENTER_CR | 使用CR作为命令行回车触发 |
   | SHELL_ENTER_CRLF | 使用CRLF作为命令行回车触发 |
   | SHELL_COMMAND_MAX_LENGTH | shell命令最大长度 |
   | SHELL_PARAMETER_MAX_NUMBER | shell命令参数最大数量 |
   | SHELL_HISTORY_MAX_NUMBER | 历史命令记录数量 |
   | SHELL_MAX_NUMBER | 管理的最大shell数量 |
   | SHELL_DEFAULT_USER | shell默认用户 |
   | SHELL_DEFAULT_USER_PASSWORD | 默认用户密码 |
   | SHELL_LOCK_TIMEOUT | shell自动锁定超时 |

## 使用方式

### 命令定义

letter shell 3.0将可执行的函数定义，变量定义统一归为命令定义，使用相同的结构储存，查找和执行

#### 定义方式

letter shell 的命令导出方式支持在函数体外部，采用定义常量的方式定义命令，例如`SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE (SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,help, shellHelp, show command info\r\nhelp [cmd]);`

letter shell 的命令表方式只支持在shell_cmd_list.c源文件的shellCommandList数据结构中添加命令，例如`SHELL_CMD_ITEM(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,help, shellHelp, show command info\r\nhelp [cmd]),`

#### 命令导出宏

letter shell 3.0对可执行命令提供了一个宏，用于进行命令导出。

对可执行命令定义使用宏`SHELL_EXPORT_CMD`定义可执行命令，宏定义如下

```C
/**
 * @brief shell 命令定义
 *
 * @param _attr 命令属性
 * @param _name 命令名
 * @param _func 命令函数
 * @param _desc 命令描述
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
```

#### 命令属性字段说明

在命令定义中，有一个`attr`字段，表示该命令的属性，具体定义为

```C
union
{
    struct
    {
        unsigned char permission : 8;                       /**< command权限 */
        ShellCommandType type : 4;                          /**< command类型 */
        unsigned char enableUnchecked : 1;                  /**< 在未校验密码的情况下可用 */
        unsigned char  readOnly : 1;                        /**< 只读 */
        unsigned char reserve : 1;                          /**< 保留 */
        unsigned char paramNum : 4;                         /**< 参数数量 */
    } attrs;
    int value;
} attr;
```

在定义命令时，需要给定这些值，可以通过宏快速声明:

```c
SHELL_CMD_PERMISSION(permission)

SHELL_CMD_TYPE(type)

SHELL_CMD_ENABLE_UNCHECKED

SHELL_CMD_DISABLE_RETURN

SHELL_CMD_READ_ONLY

SHELL_CMD_PARAM_NUM(num)
```

### 命令导出时，函数参数形式

letter shell 3.0同时支持两种形式的函数定义方式，形如main函数定义的`func(int argc, char *agrv[])`以及形如普通C函数的定义`func(int i, char *str, ...)`，两种函数定义方式适用于不同的场景

#### main函数形式

使用此方式，一个函数定义的例子如下：

```C
int func(int argc, char *agrv[])
{
    printf("%dparameter(s)\r\n", argc);
    for (char i = 1; i < argc; i++)
    {
        printf("%s\r\n", argv[i]);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), func, func, test);
```

终端调用

```sh
letter:/$ func "hello world"
2 parameter(s)
hello world
```

#### 普通C函数形式

使用此方式，shell会自动对参数进行转化处理，目前支持二进制，八进制，十进制，十六进制整形，字符，字符串的自动处理，例子如下：

```C
int func(int i, char ch, char *str)
{
    printf("input int: %d, char: %c, string: %s\r\n", i, ch, str);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), func, func, test);
```

终端调用

```sh
letter:/$ func 666 'A' "hello world"
input int: 666, char: A, string: hello world
```

### 变量定义

letter shell 3.0支持导出变量，通过命令行查看，设置以及使用变量的值

- 导出变量

  变量导出使用`SHELL_EXPORT_VAR`宏，支持整形(char, short, int)，字符串，指针以及节点变量，变量导出需要使用引用的方式，如果不允许对变量进行修改，在属性中添加`SHELL_CMD_READ_ONLY`

  ```C
  int varInt = 0;
  SHELL_EXPORT_VAR(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_VAR_INT), varInt, &varInt, test);

  char str[] = "test string";
  SHELL_EXPORT_VAR(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_VAR_STRING), varStr, str, test);

  Log log;
  SHELL_EXPORT_VAR(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_VAR_POINT), log, &log, test);
  ```
- 查看变量

  在命令行直接输入导出的变量名即可查看变量当前的值

  ```sh
  letter:/$ varInt
  varInt = 0, 0x00000000

  letter:/$ varStr
  varStr = "test string"
  ```
- 修改变量

  使用`setVar`命令修改变量的值，对于字符串型变量，请确认字符串有分配足够的空间，指针类型的变量不可修改

  ```sh
  letter:/$ setVar varInt 45678
  varInt = 45678, 0x0000b26e

  letter:/$ setVar varStr "hello"
  varStr = "hello"
  ```
