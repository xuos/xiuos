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
* @file TestAvltree.c
* @brief support to test avltree function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <device.h>
#include <string.h>
#include <stdlib.h>
#include <xs_avltree.h>

static char string[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9};
static AvlNodeType g_avlnode;

static AvlNodeType create_tree(AvlNodeType avl_node)
{
    uint32 i = 0;
    uint32 string_len = strlen(string);

    for(i = 0; i < string_len; i ++)
    {
        KPrintf("avl_node %8p data %d\n", avl_node, string[i]);
        avl_node = AvlTreeInsertNode(avl_node, string[i]);
    }

    return avl_node;
}

static AvlNodeType DeleteNode(AvlNodeType avl_node, int32 data)
{
    avl_node = AvlTreeDeleteNode(avl_node, data);

    return avl_node;
}

static AvlNodeType DeleteTree(AvlNodeType avl_node)
{
    uint32 i = 0;
    uint32 string_len = strlen(string);

    for(i = 0; i < string_len; i ++)
    {
        avl_node = AvlTreeDeleteNode(avl_node, string[i]);
    }

    return avl_node;
}

static AvlNodeType InsertNode(AvlNodeType avl_node, int32 data)
{
    avl_node = AvlTreeInsertNode(avl_node, data);

    return avl_node;
}

static AvlNodeType SearchNode(AvlNodeType avl_node, int32 data)
{
    AvlNodeType dst_node;
    dst_node = AvlNodeSearchNode(avl_node, data);

    return dst_node;
}

static AvlNodeType ModyfiNode(AvlNodeType avl_node, int32 src_data, int32 dst_data)
{
    AvlNodeType dst_node;
    dst_node = AvlNodeModifyNode(avl_node, src_data, dst_data);

    return dst_node;
}

static void PrintfRootnode(void)
{
    KPrintf("g_avlnode %8p data %d left %8p data %d right %8p data %d\n",
        g_avlnode, g_avlnode->data, g_avlnode->left, g_avlnode->left->data, g_avlnode->right, g_avlnode->right->data);

    KPrintf("g_avlnode left %8p data %d left %8p data %d right %8p data %d\n",
        g_avlnode->left, g_avlnode->left->data, g_avlnode->left->left, g_avlnode->left->left->data, g_avlnode->left->right, g_avlnode->left->right->data);

    KPrintf("g_avlnode right %8p data %d left %8p data %d right %8p data %d\n",
        g_avlnode->right, g_avlnode->right->data, g_avlnode->right->left, g_avlnode->right->left->data, g_avlnode->right->right, g_avlnode->right->right->data);
}

static void InorderPrintftree(AvlNodeType avl_node)
{
    if(avl_node)
    {
        if(avl_node->left)
        {
            InorderPrintftree(avl_node->left);
        }

        KPrintf("%d ", avl_node->data);

        if(avl_node->right)
        {
            InorderPrintftree(avl_node->right);
        }
    }
    else
    {
        KPrintf("nil \n");
    }
}

static uint32 TestAvltree(int argc, char *argv[])
{
    if(0 == argc)
    {
        KPrintf("TestAvltree needs parameters. Just return\n");
        return EOK;
    }

    if(0 == strcmp("-c", argv[1]))
    {
        g_avlnode = create_tree(g_avlnode);
        if(NONE == g_avlnode)
        {
            KPrintf("TestAvltree g_avlnode create failed!\n");
            return ERROR;
        }

        PrintfRootnode();
        InorderPrintftree(g_avlnode);
    }
    else if(0 == strcmp("-d", argv[1]))
    {
        if(3 == argc)
        {
            int32 data = atoi(argv[2]);

            g_avlnode = DeleteNode(g_avlnode, data);

            PrintfRootnode();
            InorderPrintftree(g_avlnode);
        }
        else if(2 == argc)
        {
            g_avlnode = DeleteTree(g_avlnode);

            PrintfRootnode();
            InorderPrintftree(g_avlnode);
        }
    }
    else if(0 == strcmp("-i", argv[1]))
    {
        if(3 == argc)
        {
            int32 data = atoi(argv[2]);

            g_avlnode = InsertNode(g_avlnode, data);

            PrintfRootnode();
            InorderPrintftree(g_avlnode);
        }
    }
    else if(0 == strcmp("-s", argv[1]))
    {
        if(3 == argc)
        {
            int32 data = atoi(argv[2]);

            AvlNodeType dst_node = SearchNode(g_avlnode, data);

            KPrintf("find data %dx%d node %8p\n", dst_node->data, data, dst_node);
            PrintfRootnode();
        }
    }
    else if(0 == strcmp("-m", argv[1]))
    {
        if(4 == argc)
        {
            int32 src_data = atoi(argv[2]);
            int32 dst_data = atoi(argv[3]);

            g_avlnode = ModyfiNode(g_avlnode, src_data, dst_data);

            PrintfRootnode();
            InorderPrintftree(g_avlnode);
        }
    }

    return EOK;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(4),
                                                TestAvltree, TestAvltree, Test the AVL tree Function);
