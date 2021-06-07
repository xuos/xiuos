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

#include <xiuos.h>

#ifdef FS_VFS

#include <string.h>
#include <stdlib.h>
#include <iot-vfs_posix.h>
#include "utility.h"

int IsPrefixedWith(char *str, char *prefix)
{
    while (*prefix != '\0') {
        if (*str != *prefix)
            return 0;
        str++;
        prefix++;
    }
    return 1;
}

char *RemoveUnsafePrefix(char *file_name)
{
    char *ret = file_name;

    while (1) {
        if (*ret == '/') {
            ret++;
            continue;
        }
        if (IsPrefixedWith(ret, "/../" + 1)) {
            ret += 3;
            continue;
        }
        char *p = strstr(ret, "/../");
        if (!p)
            break;
        ret = p + 4;
    }

    return ret;
}

int IsSuffixedWith(char *file_name, char *suffix)
{
    int len = strlen(file_name);
    int suff_len = strlen(suffix);

    return len > suff_len &&
            strcmp(&file_name[len - suff_len], suffix) == 0;
}

int TruncateExtension(char *file_name, char *ext)
{
    int len = strlen(file_name);
    int ext_len = strlen(ext);

    if (!IsSuffixedWith(file_name, ext))
        return -1;

    file_name[len - ext_len] = '\0';
    return 0;
}

unsigned char read8(int fd)
{
    unsigned char ret;

    read(fd, &ret, 1);

    return ret;
}

uint16_t read16Le(int fd)
{
    uint16_t ret = 0;
    unsigned char buf[2];

    read(fd, buf, 2);
    for (int i = 0; i < 2; i++)
        ret |= (uint16_t)buf[i] << (i * 8);

    return ret;
}

uint32_t read32Le(int fd)
{
    uint32_t ret = 0;
    unsigned char buf[4];

    read(fd, buf, 4);
    for (int i = 0; i < 4; i++)
        ret |= (uint32_t)buf[i] << (i * 8);

    return ret;
}

uint64_t read64Le(int fd)
{
    uint64_t ret = 0;
    unsigned char buf[8];

    read(fd, buf, 8);
    for (int i = 0; i < 8; i++)
        ret |= (uint32_t)buf[i] << (i * 8);

    return ret;
}

#endif
