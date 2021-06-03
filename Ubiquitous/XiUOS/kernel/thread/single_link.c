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
* @file:    single_link.c
* @brief:   functions definition of single linklist
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/

#include <xs_klist.h>

/**
 * This function will init a linklist.
 *
 * @param linklist_head linklist node
 */
void InitSingleLinkList(SysSingleLinklistType *linklist)
{
    linklist->node_next = NONE;
}

void AppendSingleLinkList(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    struct SingleLinklistNode *node;

    node = linklist;
    while (node->node_next) node = node->node_next;

    node->node_next = linklist_node;
    linklist_node->node_next = NONE;
}
/**
 * This function will insert a node into list after the node.
 *
 * @param linklist linklist node
 * @param linklist_node the node needed to inserted
 */

void SingleLinkListNodeInsert(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    linklist_node->node_next = linklist->node_next;
    linklist->node_next = linklist_node;
}

/**
 * This function will get length of the list.
 *
 * @param linklist list head
 * @return length
 */

unsigned int SingleLinkListGetLen(const SysSingleLinklistType *linklist)
{
    unsigned int length = 0;
    const SysSingleLinklistType *tmp_list = linklist->node_next;
    while (tmp_list != NONE)
    {
        tmp_list = tmp_list->node_next;
        length ++;
    }

    return length;
}

/**
 * This function will remove a node from the list.
 *
 * @param linklist_node the node needed to removed
 */

SysSingleLinklistType *SingleLinkListRmNode(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node)
{
    struct SingleLinklistNode *node = linklist;
    while (node->node_next && node->node_next != linklist_node) node = node->node_next;

    if (node->node_next != (SysSingleLinklistType *)0){
        node->node_next = node->node_next->node_next;
    }

    return linklist;
}

/**
 * This function will get the head of the list.
 *
 * @param linklist list head
 * @return list head
 */
SysSingleLinklistType *SingleLinkListGetFirstNode(SysSingleLinklistType *linklist)
{
    return linklist->node_next;
}

/**
 * This function will get the tail node of the list.
 *
 * @param linklist list head
 * @return list taile node
 */
SysSingleLinklistType *SingleLinkListGetTailNode(SysSingleLinklistType *linklist)
{
    while (linklist->node_next) linklist = linklist->node_next;

    return linklist;
}

/**
 * This function will get the next node of the list head.
 *
 * @param linklist list head
 * @param linklist_node list head
 * @return next node of the list head
 */

SysSingleLinklistType *SingleLinkListGetNextNode(SysSingleLinklistType *linklist_node)
{
    return linklist_node->node_next;
}

/**
 * This function will judge the list is empty.
 *
 * @param linklist the list head
 * @return true
 */

int IsSingleLinkListEmpty(SysSingleLinklistType *linklist)
{
    return linklist->node_next == NONE;
}