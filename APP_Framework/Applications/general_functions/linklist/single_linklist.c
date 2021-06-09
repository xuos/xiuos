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
* @brief:   functions definition of single linklist for application
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include "list.h"

void InitSingleLinkList(SysSingleLinklistType *linklist)
{
    linklist->node_next = NULL;
}

void AppendSingleLinkList(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    struct SingleLinklistNode *node;

    node = linklist;
    while (node->node_next) node = node->node_next;

    node->node_next = linklist_node;
    linklist_node->node_next = NULL;
}

void SingleLinkListNodeInsert(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    linklist_node->node_next = linklist->node_next;
    linklist->node_next = linklist_node;
}

unsigned int SingleLinkListGetLen(const SysSingleLinklistType *linklist)
{
    unsigned int length = 0;
    const SysSingleLinklistType *tmp_list = linklist->node_next;
    while (tmp_list != NULL)
    {
        tmp_list = tmp_list->node_next;
        length ++;
    }

    return length;
}

SysSingleLinklistType *SingleLinkListRmNode(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    struct SingleLinklistNode *node = linklist;
    while (node->node_next && node->node_next != linklist_node) node = node->node_next;

    if (node->node_next != (SysSingleLinklistType *)0){
        node->node_next = node->node_next->node_next;
    }

    return linklist;
}

SysSingleLinklistType *SingleLinkListGetFirstNode(SysSingleLinklistType *linklist)
{
    return linklist->node_next;
}

SysSingleLinklistType *SingleLinkListGetTailNode(SysSingleLinklistType *linklist)
{
    while (linklist->node_next) linklist = linklist->node_next;

    return linklist;
}

SysSingleLinklistType *SingleLinkListGetNextNode(SysSingleLinklistType *linklist_node)
{
    return linklist_node->node_next;
}

int IsSingleLinkListEmpty(SysSingleLinklistType *linklist)
{
    return linklist->node_next == NULL;
}