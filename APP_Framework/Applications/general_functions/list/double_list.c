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
* @brief:   functions definition of double list for application
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include "list.h"

void AppInitDoubleList(DoublelistType *list_head)
{
    list_head->node_next = list_head;
    list_head->node_prev = list_head;
}

void AppDoubleListInsertNodeAfter(DoublelistType *list, DoublelistType *list_node)
{
    list->node_next->node_prev = list_node;
    list_node->node_next = list->node_next;

    list->node_next = list_node;
    list_node->node_prev = list;
}

void AppDoubleListInsertNodeBefore(DoublelistType *list, DoublelistType *list_node)
{
    list->node_prev->node_next = list_node;
    list_node->node_prev = list->node_prev;

    list->node_prev = list_node;
    list_node->node_next = list;
}

void AppDoubleListRmNode(DoublelistType *list_node)
{
    list_node->node_next->node_prev = list_node->node_prev;
    list_node->node_prev->node_next = list_node->node_next;

    list_node->node_next = list_node;
    list_node->node_prev = list_node;
}

int AppIsDoubleListEmpty(const DoublelistType *list)
{
    return list->node_next == list;
}

struct DoublelistNode *AppDoubleListGetHead(const DoublelistType *list)
{
    return AppIsDoubleListEmpty(list) ? NULL : list->node_next;
}

struct DoublelistNode *AppDoubleListGetNext(const DoublelistType *list,
        const struct DoublelistNode *list_node)
{
    return list_node->node_next == list ? NULL : list_node->node_next;
}

unsigned int AppDoubleListLenGet(const DoublelistType *list)
{
    unsigned int linklist_length = 0;
    const DoublelistType *tmp_node = list;
    while (tmp_node->node_next != list)
    {
        tmp_node = tmp_node->node_next;
        linklist_length ++;
    }

    return linklist_length;
}