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

#if defined(TOOL_SHELL) && defined(FS_VFS)
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <iot-vfs_posix.h>

#include "utility.h"

extern char working_dir[];

void ls(const char *path)
{
    DIR *dir;
    struct dirent *dirent;
    struct stat statbuf;

    if (path == NULL) {
        path = strdup(working_dir);
        if (path == NULL)
            return;
    } else {
        path = (char *)path;
    }

    if ((dir = opendir(path)) == NULL) {
        KPrintf("Failed to open directory %s\n", path);
        return;
    }

    KPrintf("Directory: %s\n", path);
    while ((dirent = readdir(dir)) != NULL) {
        stat(dirent->d_name, &statbuf);
        KPrintf("%-20s", dirent->d_name);
        if (S_ISREG(statbuf.st_mode))
            KPrintf("%-25lu\n", (unsigned long)statbuf.st_size);
        else
            KPrintf("%-25s\n", "<DIR>");
    }

    closedir(dir);
}


int cmd_ls(int argc, char **argv)
{
    if (argc == 1)
        ls(working_dir);
    else
        ls(argv[1]);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
ls,cmd_ls, List information about the FILES .);

static int CopyRecursive(const char *from, const char *to, char *buf,
        size_t buf_size)
{
    struct stat statbuf;
    int ret;

    if (strcmp(from, to) == 0)
        return 0;

    ret = stat(from, &statbuf);
    if (ret < 0) {
        KPrintf("Failed to read %s\n", from);
        return -1;
    }

    if (S_ISREG(statbuf.st_mode)) {
        int fd_from, fd_to;
        int size_read;
        int total_size_written = 0;

        fd_from = open(from, O_RDONLY);
        if (fd_from < 0) {
            KPrintf("Failed to open %s\n", from);
            return -1;
        }
        fd_to = open(to, O_WRONLY);
        if (fd_to < 0) {
            KPrintf("Failed to open %s\n", to);
            close(fd_from);
            return -1;
        }

        while ((size_read = read(fd_from, buf, buf_size)) > 0)
            total_size_written += write(fd_to, buf, size_read);

        close(fd_from);
        close(fd_to);

        if (total_size_written != statbuf.st_size) {
            KPrintf("Error copying %s to %s\n", from, to);
            return -1;
        }

        return 0;
    }

    DIR *dirp;
    struct dirent *dirent;
    char *sub_from, *sub_to;

    ret = mkdir(to, 0777);
    if (ret < 0) {
        KPrintf("Failed to create directory %s\n", to);
        return -1;
    }

    dirp = opendir(from);
    if (dirp == NULL) {
        KPrintf("Failed to open directory %s\n", from);
        return -1;
    }

    ret = 0;
    while ((dirent = readdir(dirp)) != NULL) {
        sub_from = malloc(strlen(from) + strlen(dirent->d_name) + 2);
        sub_to = malloc(strlen(to) + strlen(dirent->d_name) + 2);
        if (sub_from == NULL || sub_to == NULL) {
            KPrintf("Out of memory\n");
            ret = -1;
            goto err;
        }

        sprintf(sub_from, "%s/%s", from, dirent->d_name);
        sprintf(sub_to, "%s/%s", to, dirent->d_name);

        if (CopyRecursive(sub_from, sub_to, buf, buf_size) < 0) {
            ret = -1;
            goto err;
        }

        free(sub_from);
        free(sub_to);
    }

err:
    closedir(dirp);
    free(sub_from);
    free(sub_to);

    return ret;
}

static int copy(const char *from, const char *to)
{
    struct stat statbuf;
    char buf[128];
    char *abs_from, *abs_to;
    int abs_from_len, abs_to_len;
    char *last_name;
    int last_name_len;
    char *GetAbsolutePath(const char *parent, const char *filename);

    abs_from = GetAbsolutePath(NULL, from);
    abs_to = GetAbsolutePath(NULL, to);
    if (abs_from == NULL || abs_to == NULL)
        goto err;

    abs_from_len = strlen(abs_from);
    abs_to_len = strlen(abs_to);

    last_name = abs_from + abs_from_len - 1;
    while (*last_name != '/')
        last_name--;
    last_name_len = abs_from + abs_from_len - last_name;

    if (stat(abs_to, &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(abs_to, "/") == 0) {
                last_name++;
                last_name_len--;
            }
            abs_to = realloc(abs_to, abs_to_len + last_name_len + 1);
            if (abs_to == NULL)
                goto err;
            strcat(abs_to, last_name);
        } else {
            unlink(abs_to);
        }
    }

    return CopyRecursive(abs_from, abs_to, buf, sizeof(buf)) < 0;

err:
    free(abs_from);
    free(abs_to);

    return -1;
}

int cmd_cp(int argc, char *argv[])
{
    if (argc != 3) {
        KPrintf("Usage: cp SOURCE DEST\n");
        KPrintf("Copy SOURCE to DEST\n");

        return 0;
    }

    copy(argv[1], argv[2]);

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
cp,cmd_cp, Copy source to dest.);


static void RmRecursive(char *file_name, int recursive)
{
    struct stat statbuf;

    if (stat(file_name, &statbuf) < 0)
        return;

    if (recursive && S_ISDIR(statbuf.st_mode)) {
        DIR *dir;
        struct dirent *dirent;

        dir = opendir(file_name);
        while ((dirent = readdir(dir)) != NULL) {
            char *sub_file_name = malloc(strlen(file_name) +
                    strlen(dirent->d_name) + 2);
            if (sub_file_name == NULL) {
                KPrintf("Memory not enough\n");
                return;
            }
            sprintf(sub_file_name, "%s/%s", file_name, dirent->d_name);
            RmRecursive(sub_file_name, recursive);
            free(sub_file_name);
        }
        closedir(dir);
    }

    unlink(file_name);
}

int cmd_mv(int argc, char *argv[])
{
    if (argc != 3) {
        KPrintf("Usage: mv SOURCE DEST\n");
        KPrintf("Move SOURCE to DESn");

        return 0;
    }

    if (rename(argv[1], argv[2]) != 0)
        if (copy(argv[1], argv[2]) == 0)
            RmRecursive(argv[1], 1);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
mv,cmd_mv, Move frome source to dest.);


void cat(const char *filename)
{
    int size;
    char buf[128];
    int fd;

    if ((fd = open(filename, O_RDONLY)) < 0) {
        KPrintf("Failed to open file %s\n", filename);
        return;
    }

    while ((size = read(fd, buf, 128)) > 0)
        for (int i = 0; i < size; i++)
            putchar(buf[i]);

    close(fd);
}

int cmd_cat(int argc, char **argv)
{
    int index;
    extern void cat(const char *filename);

    if (argc == 1)
    {
        KPrintf("Usage: cat [FILE]...\n");
        KPrintf("Concatenate FILE(s)\n");
        return 0;
    }

    for (index = 1; index < argc; index ++)
    {
        cat(argv[index]);
    }

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
cat,cmd_cat,Concatenate FILE(s).);

int cmd_rm(int argc, char **argv)
{
    int recursive = 0;

    if (argc == 1) {
        KPrintf("Usage: rm FILE...\n");
        KPrintf("Remove (unlink) the FILE(s).\n");
        KPrintf("Suppported option flags:\n");
        KPrintf("    r    recursively remove folders\n");
        return 0;
    }

    if (argv[1][0] == '-') {
        if (strcmp(argv[1], "-r") != 0)
            KPrintf("Unknown options\n");
        recursive = 1;
        argc--;
        argv++;
    }

    for (int i = 1; i < argc; i++)
        RmRecursive(argv[i], recursive);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
rm,cmd_rm,Remove (unlink) the File(s).);

#ifdef VFS_USING_WORKDIR
int cmd_cd(int argc, char **argv)
{
    if (argc == 1)
    {
        KPrintf("%s\n", working_dir);
    }
    else if (argc == 2)
    {
        if (chdir(argv[1]) != 0)
        {
            KPrintf("No such directory: %s\n", argv[1]);
        }
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
cd,cmd_cd, Chage the shell wroking directory.);

int cmd_pwd(int argc, char **argv)
{
    KPrintf("%s\n", working_dir);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
pwd,cmd_pwd, print the name of the current working directory.);

#endif

int cmd_mkdir(int argc, char **argv)
{
    if (argc == 1)
    {
        KPrintf("Usage: mkdir [OPTION] DIRECTORY\n");
        KPrintf("Create the DIRECTORY, if they do not already exist.\n");
    }
    else
    {
        mkdir(argv[1], 0);
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
mkdir,cmd_mkdir, Create a directory.);

int cmd_df(int argc, char **argv)
{
    struct statfs buf;
    char *path = argc < 2 ? "/" : argv[1];
    static char *unit[] = {"B", "KB", "MB", "GB"};
    uint64_t integer, decimal = 0;
    int i;

    if (statfs(path, &buf) < 0) {
        KPrintf("statfs failed: %s\n", path);
        return -1;
    }

    integer = (uint64_t)buf.f_bsize * buf.f_bfree;
    for (i = 0; i < 4; i++) {
        if (integer < 1024)
            break;
        decimal = (integer % 1024) * 10 / 1024;
        integer /= 1024;
    }
    if (i >= 4)
        i = 3;

    KPrintf("Free disk space: %d.%d %s "
            "[ %d blocks, %d free blocks, %d bytes per block ]\n",
            (int)integer, (int)decimal, unit[i], buf.f_blocks, buf.f_bfree,
            buf.f_bsize);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
df,cmd_df,disk free);


int cmd_echo(int argc, char** argv)
{
    if (argc == 2)
    {
        KPrintf("%s\n", argv[1]);
    }
    else if (argc == 3)
    {
        int fd;

        fd = open(argv[2], O_RDWR | O_APPEND | O_CREAT, 0);
        if (fd >= 0)
        {
            write (fd, argv[1], strlen(argv[1]));
            close(fd);
        }
        else
        {
            KPrintf("open file:%s failed!\n", argv[2]);
        }
    }
    else
    {
        KPrintf("Usage: echo \"string\" [filename]\n");
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
echo,cmd_echo,echo string to file.);

static void FindRecursive(char *name, char *path)
{
    struct stat statbuf;
    DIR *dir = opendir(path);
    struct dirent *dirent;

    while ((dirent = readdir(dir)) != NULL) {
        char *sub_path = malloc(strlen(path) +
                strlen(dirent->d_name) + 2);
        if (sub_path == NULL) {
            KPrintf("Memory not enough\n");
            return;
        }
        sprintf(sub_path, "%s/%s", path, dirent->d_name);
        if (stat(sub_path, &statbuf) < 0) {
            KPrintf("Failed to access %s\n", sub_path);
            free(sub_path);
            continue;
        }
        if (strstr(dirent->d_name, name) != NULL)
            KPrintf("%s\n", sub_path);
        if (S_ISDIR(statbuf.st_mode))
            FindRecursive(name, sub_path);
        free(sub_path);
    }
}

static void FindPrintUsage()
{
    KPrintf("Usage: find NAME PATH\n");
}

int cmd_find(int argc, char **argv)
{
    struct stat statbuf;

    if (argc != 3) {
        FindPrintUsage();
        return 0;
    }

    TruncateExtension(argv[2], "/");
    if (strcmp(argv[2], "/") == 0)
        argv[2][0] = '\0';
    if (stat(argv[2], &statbuf) < 0 || !S_ISDIR(statbuf.st_mode)) {
        KPrintf("No such directory: %s\n", argv[2]);
        return 0;
    }

    if (strstr(argv[1], "/") != NULL) {
        KPrintf("Invalid file name: %s\n", argv[1]);
        return 0;
    }

    FindRecursive(argv[1], argv[2]);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
find,cmd_find,search file with given name in specified path.);

int cmd_tar(int argc, char **argv)
{
    extern int tar(int argc, char **argv);
    tar(argc, argv);

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
tar,cmd_tar,create or extarct tar archive.);


int cmd_gzip(int argc, char **argv)
{
    extern int gzip(int argc, char **argv);
    gzip(argc, argv);

    return 0;
}

SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
gzip,cmd_gzip, creat or extart gzip compressed files);


int cmd_gunzip(int argc, char **argv)
{
    extern int gunzip(int argc, char **argv);
    gunzip(argc, argv);

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
gunzip,cmd_gunzip,decompress gzip files.);

int cmd_unzip(int argc, char **argv)
{
    extern int unzip(int argc, char **argv);
    unzip(argc, argv);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
unzip,cmd_unzip, decompress zip files.);


int cmd_bzip2(int argc, char **argv)
{
    extern int bzip2(int argc, char **argv);
    bzip2(argc, argv);

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
bzip2,cmd_bzip2,create or extract bzip2 compressed files.);

int cmd_bunzip2(int argc, char **argv)
{
    extern int bunzip2(int argc, char **argv);
    bunzip2(argc, argv);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
bunzip2,cmd_bunzip2,decompress bzip2 files.);


#endif

