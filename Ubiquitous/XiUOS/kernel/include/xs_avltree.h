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
* @file:    xs_avltree.h
* @brief:   function declaration and structure defintion of avl tree
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <xs_base.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef KERNEL_AVL_TREE
struct AvlNode
{
    int32 data;
    uint32 height;

    struct AvlNode *left;
    struct AvlNode *right;
};

typedef struct AvlNode *AvlNodeType;

#ifndef AVL_MAX
#define AVL_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef AVL_ABS
#define AVL_ABS(a) ((a > 0) ? (a) : (-a))
#endif

/*This function will insert data to the avl tree*/
AvlNodeType AvlTreeInsertNode(AvlNodeType avl_node, int32 data);

/*This function will delete data from the avl tree*/
AvlNodeType AvlTreeDeleteNode(AvlNodeType avl_node, int32 data);

/*This function will modify certain data of the avl tree*/
AvlNodeType AvlNodeModifyNode(AvlNodeType avl_node, int32 src_data, int32 dst_data);

/*This function will return the avl node which has the data*/
AvlNodeType AvlNodeSearchNode(AvlNodeType avl_node, int32 data);
#endif
#ifdef __cplusplus
}
#endif

#endif
