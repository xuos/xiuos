/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS  is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file:    double_link.c
* @brief:   functions definition of single list for application
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include "list.h"

void AppInitSingleList(SinglelistType *list)
{
    list->node_next = NULL;
}

void AppAppendSingleList(SinglelistType *list, SinglelistType *list_node)
{
    struct SinglelistNode *node;

    node = list;
    while (node->node_next) node = node->node_next;

    node->node_next = list_node;
    list_node->node_next = NULL;
}

void AppSingleListNodeInsert(SinglelistType *list, SinglelistType *list_node)
{
    list_node->node_next = list->node_next;
    list->node_next = list_node;
}

unsigned int AppSingleListGetLen(const SinglelistType *list)
{
    unsigned int length = 0;
    const SinglelistType *tmp_list = list->node_next;
    while (tmp_list != NULL)
    {
        tmp_list = tmp_list->node_next;
        length ++;
    }

    return length;
}

SinglelistType *AppSingleListRmNode(SinglelistType *list, SinglelistType *list_node)
{
    struct SinglelistNode *node = list;
    while (node->node_next && node->node_next != list_node) node = node->node_next;

    if (node->node_next != (SinglelistType *)0){
        node->node_next = node->node_next->node_next;
    }

    return list;
}

SinglelistType *AppSingleListGetFirstNode(SinglelistType *list)
{
    return list->node_next;
}

SinglelistType *AppSingleListGetTailNode(SinglelistType *list)
{
    while (list->node_next) list = list->node_next;

    return list;
}

SinglelistType *AppSingleListGetNextNode(SinglelistType *list_node)
{
    return list_node->node_next;
}

int AppIsSingleListEmpty(SinglelistType *list)
{
    return list->node_next == NULL;
}