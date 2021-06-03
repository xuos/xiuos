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
* @file:    avl_tree.c
* @brief:   avl_tree file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include "xs_avltree.h"
#include <string.h>
#include <xs_kdbg.h>
#include <xs_klist.h>
#include <xs_memory.h>

/**
 * This function will return the node height of the avl tree
 *
 * @param avl_node avl tree node descriptor
 * @return 0
 */
static uint32 AvlTreeGetNodeHeight(AvlNodeType avl_node)
{
    if(avl_node) {
        return AVL_MAX(AvlTreeGetNodeHeight(avl_node->left), AvlTreeGetNodeHeight(avl_node->right)) + 1;
    } else {
        return 0;
    }
}

/**
 * This function will return the node balance factor of the avl tree
 *
 * @param avl_node avl tree node descriptor
 */
static int32 AvlTreeGetNodeBalanceFactor(AvlNodeType avl_node)
{
    if(avl_node) {
        return AvlTreeGetNodeHeight(avl_node->left) - AvlTreeGetNodeHeight(avl_node->right);
    } else {
        return 0;
    }
}

/**
 * This function will support right rotate to balance the avl tree(eg. LL case)
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeSetRightRotate(AvlNodeType avl_node)
{
    NULL_PARAM_CHECK(avl_node);

    AvlNodeType new_node = avl_node->left;

    avl_node->left = new_node->right;
    new_node->right = avl_node;

    new_node->height = AVL_MAX(AvlTreeGetNodeHeight(new_node->left), AvlTreeGetNodeHeight(new_node->right)) + 1;
    avl_node->height = AVL_MAX(AvlTreeGetNodeHeight(avl_node->left), AvlTreeGetNodeHeight(avl_node->right)) + 1;

    return new_node;
}

/**
 * This function will support left rotate to balance the avl tree(eg. RR case)
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeSetLeftRotate(AvlNodeType avl_node)
{
    NULL_PARAM_CHECK(avl_node);

    AvlNodeType new_node = avl_node->right;

    avl_node->right = new_node->left;
    new_node->left = avl_node;

    new_node->height = AVL_MAX(AvlTreeGetNodeHeight(new_node->left), AvlTreeGetNodeHeight(new_node->right)) + 1;
    avl_node->height = AVL_MAX(AvlTreeGetNodeHeight(avl_node->left), AvlTreeGetNodeHeight(avl_node->right)) + 1;

    return new_node;
}

/**
 * This function will support left and right rotate to balance the avl tree(eg. LR case)
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeSetLRRotate(AvlNodeType avl_node)
{
    NULL_PARAM_CHECK(avl_node);

    AvlNodeType new_node = NONE;
    AvlNodeType left_node = avl_node->left;

    /*step1 : Left rotate*/
    avl_node->left = AvlTreeSetLeftRotate(left_node);

    /*step2 : Right rotate*/
    new_node = AvlTreeSetRightRotate(avl_node);

    return new_node;
}

/**
 * This function will support right and left rotate to balance the avl tree(eg. RL case)
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeSetRLRotate(AvlNodeType avl_node)
{
    NULL_PARAM_CHECK(avl_node);

    AvlNodeType new_node = NONE;
    AvlNodeType right_node = avl_node->right;

    /*step1 : Right rotate*/
    avl_node->right = AvlTreeSetRightRotate(right_node);

    /*step2 : Left rotate*/
    new_node = AvlTreeSetLeftRotate(avl_node);

    return new_node;
}

/**
 * This function will balance the avl tree when inserting or deleting node
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeBalance(AvlNodeType avl_node)
{
    if(avl_node)
    {
        AvlNodeType new_node = NONE;
        uint32 avlnode_BF = AVL_ABS(AvlTreeGetNodeBalanceFactor(avl_node));

        if(avlnode_BF > 1) {
            if(AvlTreeGetNodeBalanceFactor(avl_node->left) > 0) {
                /*LL case*/
                new_node = AvlTreeSetRightRotate(avl_node);
            } else if(AvlTreeGetNodeBalanceFactor(avl_node->left) < 0) {
                /*LR case*/
                new_node = AvlTreeSetLRRotate(avl_node);
            } else if(AvlTreeGetNodeBalanceFactor(avl_node->right) < 0) {
                /*RR case*/
                new_node = AvlTreeSetLeftRotate(avl_node);
            } else if(AvlTreeGetNodeBalanceFactor(avl_node->right) > 0) {
                /*RL case*/
                new_node = AvlTreeSetRLRotate(avl_node);
            }
        } else {
            /*the avl tree is balanced, no need to rebalance*/
            new_node = avl_node;
        }

        return new_node;

    } else {
        return NONE;
    }
}

/**
 * This function will insert data to the avl tree
 *
 * @param avl_node avl tree node descriptor
 * @param data input data
 */
AvlNodeType AvlTreeInsertNode(AvlNodeType avl_node, int32 data)
{
    AvlNodeType new_node = NONE;

    if(NONE == avl_node) {
        new_node = x_malloc(sizeof(struct AvlNode));
        if(NONE == new_node) {
            KPrintf("AvlTreeInsertNode malloc AvlNode failed\n");
            x_free(new_node);
            return NONE;
        }
        new_node->data = data;
        new_node->left = NONE;
        new_node->right = NONE;
        new_node->height = 1;
        avl_node = new_node;
    } else if(data == avl_node->data) {
        /*input data is already existed*/
        KPrintf("the input data is already existed. just return\n");
        return avl_node;
    } else {
        if(avl_node->data < data) {
            avl_node->right = AvlTreeInsertNode(avl_node->right, data);
            if(NONE == avl_node->right) {
                KPrintf("AvlTreeInsertNode find right avl_node data failed\n");
                return NONE;
            }
        } else {
            avl_node->left = AvlTreeInsertNode(avl_node->left, data);
            if(NONE == avl_node->left) {
                KPrintf("AvlTreeInsertNode find left avl_node data failed\n");
                return NONE;
            }
        }
    }

    return AvlTreeBalance(avl_node);
}

/**
 * This function will find the pre node of the avl_node
 *
 * @param avl_node avl tree node descriptor
 * @return avl tree node
 */
static AvlNodeType AvlTreeFindPreNode(AvlNodeType avl_node)
{
    NULL_PARAM_CHECK(avl_node);

    AvlNodeType pre_node = NONE;

    if(avl_node->left) {
        if(avl_node->left->right) {
            pre_node = avl_node->left->right;
            while(pre_node->right) {
                pre_node = pre_node->right;
            }
        } else {
            pre_node = avl_node->left;
        }
    } else {
        pre_node = avl_node;
    }

    return pre_node;
}

/**
 * This function will delete a certain node, ultimate target is to find the leaf node
 *
 * @param avl_node avl tree node descriptor
 * @param data delete data
 * @return avl tree node
 */
static AvlNodeType AvlTreeDeleteLeafNode(AvlNodeType avl_node)
{
    AvlNodeType pre_node = NONE;

    if((NONE == avl_node->left) && (NONE == avl_node->right)) {
        /*Leaf Node*/
        avl_node = NONE;
        x_free(avl_node);
    } else if(NONE == avl_node->left) {
        /*Right child is Leaf Node*/
        avl_node->data = avl_node->right->data;
        avl_node->right = NONE;
        x_free(avl_node->right);
    } else if(NONE == avl_node->right) {
        /*Left child is Leaf Node*/
        avl_node->data = avl_node->left->data;
        avl_node->left = NONE;
        x_free(avl_node->left);
    } else {
        /*Find the pre node to replace the avl node, then delete the pre node*/
        pre_node = AvlTreeFindPreNode(avl_node);
        if(pre_node) {
            avl_node->data = pre_node->data;
            avl_node->left = AvlTreeDeleteNode(avl_node->left, pre_node->data);
        }
    }

    return avl_node;
}

/**
 * This function will delete data from the avl tree
 *
 * @param avl_node avl tree node descriptor
 * @param data delete data
 */
AvlNodeType AvlTreeDeleteNode(AvlNodeType avl_node, int32 data)
{
    if(avl_node) {
        if(data == avl_node->data) {
            avl_node = AvlTreeDeleteLeafNode(avl_node);
        } else {
            if(avl_node->data < data) {
                avl_node->right = AvlTreeDeleteNode(avl_node->right, data);
            } else {
                avl_node->left = AvlTreeDeleteNode(avl_node->left, data);
            }
        }

        return AvlTreeBalance(avl_node);
    } else {
        KPrintf("AvlTreeDeleteNode cannot find the delete data\n");
        return NONE;
    }
}

/**
 * This function will modify certain data of the avl tree
 *
 * @param avl_node avl tree node descriptor
 * @param src_data src data
 * @param dst_data dst data
 */
AvlNodeType AvlNodeModifyNode(AvlNodeType avl_node, int32 src_data, int32 dst_data)
{
    AvlNodeType dst_node = NONE;

    if(avl_node) {
        /*Step1 : delete the src data node*/
        avl_node = AvlTreeDeleteNode(avl_node, src_data);

        /*Step2 : insert the dst data node*/
        dst_node = AvlTreeInsertNode(avl_node, dst_data);

    } else {
        KPrintf("AvlNodeModifyNode cannot find the src data node\n");
        return NONE;
    }

    return dst_node;
}

/**
 * This function will return the avl node which has the data
 *
 * @param avl_node avl tree node descriptor
 * @param data data
 */
AvlNodeType AvlNodeSearchNode(AvlNodeType avl_node, int32 data)
{
    AvlNodeType dst_node = NONE;

    if(avl_node) {
        if(data == avl_node->data) {
            dst_node = avl_node;
            KPrintf("AvlNodeSearchNode %d is existed return the node\n", avl_node->data);
        } else if(avl_node->data < data) {
            dst_node = AvlNodeSearchNode(avl_node->right, data);
        } else {
            dst_node = AvlNodeSearchNode(avl_node->left, data);
        }
    } else {
        KPrintf("AvlNodeSearchNode avl_node is NONE.\n");
        return NONE;
    }

    return dst_node;
}
