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
* @brief:   functions definition of double linklist
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
void InitDoubleLinkList(DoubleLinklistType *linklist_head)
{
    linklist_head->node_next = linklist_head;
    linklist_head->node_prev = linklist_head;
}

/**
 * This function will insert a node into list after the node.
 *
 * @param linklist linklist node
 * @param linklist_node the node needed to inserted
 */
void DoubleLinkListInsertNodeAfter(DoubleLinklistType *linklist, DoubleLinklistType *linklist_node)
{
    linklist->node_next->node_prev = linklist_node;
    linklist_node->node_next = linklist->node_next;

    linklist->node_next = linklist_node;
    linklist_node->node_prev = linklist;
}

/**
 * This function will insert a node into list after the node.
 *
 * @param linklist linklist node
 * @param linklist_node the node needed to inserted
 */
void DoubleLinkListInsertNodeBefore(DoubleLinklistType *linklist, DoubleLinklistType *linklist_node)
{
    linklist->node_prev->node_next = linklist_node;
    linklist_node->node_prev = linklist->node_prev;

    linklist->node_prev = linklist_node;
    linklist_node->node_next = linklist;
}

/**
 * This function will remove a node from the list.
 *
 * @param linklist_node the node needed to removed
 */
void DoubleLinkListRmNode(DoubleLinklistType *linklist_node)
{
    linklist_node->node_next->node_prev = linklist_node->node_prev;
    linklist_node->node_prev->node_next = linklist_node->node_next;

    linklist_node->node_next = linklist_node;
    linklist_node->node_prev = linklist_node;
}

/**
 * This function will judge the list is empty.
 *
 * @param linklist the list head
 * @return true
 */
int IsDoubleLinkListEmpty(const DoubleLinklistType *linklist)
{
    return linklist->node_next == linklist;
}

/**
 * This function will get the head of the list.
 *
 * @param linklist list head
 * @return list head
 */
struct SysDoubleLinklistNode *DoubleLinkListGetHead(const DoubleLinklistType *linklist)
{
    return IsDoubleLinkListEmpty(linklist) ? NONE : linklist->node_next;
}

/**
 * This function will get the next node of the list head.
 *
 * @param linklist list head
 * @param linklist_node list head
 * @return next node of the list head
 */
struct SysDoubleLinklistNode *DoubleLinkListGetNext(const DoubleLinklistType *linklist,
        const struct SysDoubleLinklistNode *linklist_node)
{
    return linklist_node->node_next == linklist ? NONE : linklist_node->node_next;
}

/**
 * This function will get length of the list.
 *
 * @param linklist list head
 * @return length
 */
unsigned int DoubleLinkListLenGet(const DoubleLinklistType *linklist)
{
    unsigned int linklist_length = 0;
    const DoubleLinklistType *tmp_node = linklist;
    while (tmp_node->node_next != linklist)
    {
        tmp_node = tmp_node->node_next;
        linklist_length ++;
    }

    return linklist_length;
}