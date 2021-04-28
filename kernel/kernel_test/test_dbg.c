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
* @file TestDbg.c
* @brief support to test dbg function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>

#define KDBG_TEST1 1
#define KDBG_TEST2 0

static uint32 TestDbg(void)
{
    x_err_t flag;

    SYS_WARN("TestDbg function 1!\n");
    SYS_ERR("TestDbg function 2!\n");
    DBG("TestDbg function 3!\n");

    SYS_KDEBUG_LOG(KDBG_TEST1, ("TestDbg kernel function 1!\n"));
    SYS_KDEBUG_LOG(KDBG_TEST2, ("TestDbg kernel function 2!\n"));

    return EOK;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
                                                TestDbg, TestDbg, Test the log debug Function);
