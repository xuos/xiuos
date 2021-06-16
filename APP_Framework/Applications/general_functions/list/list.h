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
* @file:    xs_klist.h
* @brief:   function declaration and structure defintion of list
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/2
*
*/

#ifndef __LIST_H__
#define __LIST_H__

#include "libc.h"

#ifdef __cplusplus
extern "C" {
#endif
    

typedef struct DoublelistNode
{
    struct DoublelistNode *node_next;                         
    struct DoublelistNode *node_prev;                         
} DoublelistType;                

// Single List 
typedef struct SinglelistNode
{
    struct SinglelistNode *node_next;                       
} SinglelistType;              


#define      CONTAINER_OF(item, type, member) \
    ((type *)((char *)(item) - (unsigned long)(&((type *)0)->member)))


#define DOUBLE_LIST_OBJ_INIT(obj) { &(obj), &(obj) }

void AppInitDoubleList(DoublelistType *linklist_head);
void AppDoubleListInsertNodeAfter(DoublelistType *list, DoublelistType *list_node);
void AppDoubleListInsertNodeBefore(DoublelistType *list, DoublelistType *list_node);
void AppDoubleListRmNode(DoublelistType *list_node);
int AppIsDoubleListEmpty(const DoublelistType *list);
struct DoublelistNode *AppDoubleLinkListGetHead(const DoublelistType *list);
struct DoublelistNode *AppDoubleLinkListGetNext(const DoublelistType *list,
        const struct DoublelistNode *list_node);
unsigned int AppDoubleListLenGet(const DoublelistType *list);

#define DOUBLE_LIST_ENTRY(item, type, member) \
   CONTAINER_OF(item, type, member)

#define DOUBLE_LIST_FOR_EACH(item, head) \
    for (item = (head)->node_next; item != (head); item = item->node_next)

#define DOUBLE_LIST_FOR_EACH_SAFE(item, node_next, head) \
	for (item = (head)->node_next, node_next = item->node_next; item != (head); \
		item = node_next, node_next = item->node_next)

#define DOUBLE_LIST_FOR_EACH_ENTRY(item, head, member) \
    for (item = DOUBLE_LIST_ENTRY((head)->node_next, typeof(*item), member); \
         &item->member != (head); \
         item = DOUBLE_LIST_ENTRY(item->member.node_next, typeof(*item), member))

#define DOUBLE_LIST_FOR_EACH_ENTRY_SAFE(item, node_next, head, member) \
    for (item = DOUBLE_LIST_ENTRY((head)->node_next, typeof(*item), member), \
         node_next = DOUBLE_LIST_ENTRY(item->member.node_next, typeof(*item), member); \
         &item->member != (head); \
         item = node_next, node_next = DOUBLE_LIST_ENTRY(node_next->member.node_next, typeof(*node_next), member))

#define DOUBLE_LIST_FIRST_ENTRY(ptr, type, member) \
              DOUBLE_LIST_ENTRY((ptr)->node_next, type, member)

#define SINGLE_LIST_OBJ_INIT(obj) { NONE }

void AppInitSingleList(SinglelistType *list);
void AppAppendSingleList(SinglelistType *list, SinglelistType *list_node);
void AppSingleListNodeInsert(SinglelistType *list, SinglelistType *list_node);
unsigned int AppSingleListGetLen(const SinglelistType *list);
SinglelistType *AppSingleListRmNode(SinglelistType *list, SinglelistType *list_node);
SinglelistType *AppSingleListGetFirstNode(SinglelistType *list);
SinglelistType *AppSingleListGetTailNode(SinglelistType *list);
SinglelistType *AppSingleListGetNextNode(SinglelistType *list_node);
int AppIsSingleListEmpty(SinglelistType *list);

#define SINGLE_LIST_ENTRY(node, type, member) \
   CONTAINER_OF(node, type, member)

#define SINGLE_LIST_FOR_EACH(item, head) \
    for (item = (head)->node_next; item != NONE; item = item->node_next)

#define SINGLE_LIST_FOR_EACH_ENTRY(item, head, member) \
    for (item = SINGLE_LIST_ENTRY((head)->node_next, typeof(*item), member); \
         &item->member != (NONE); \
         item = SINGLE_LIST_ENTRY(item->member.node_next, typeof(*item), member))

#define SINGLE_LIST_FIRST_ENTRY(ptr, type, member) \
    SINGLE_LIST_ENTRY((ptr)->node_next, type, member)

#define SINGLE_LIST_TAIL_ENTRY(ptr, type, member) \
    SINGLE_LIST_ENTRY(AppSingleListGetTailNode(ptr), type, member)

#ifdef __cplusplus
}
#endif

#endif
