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
* @brief:   function declaration and structure defintion of linklist
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/2
*
*/

#ifndef __XS_KLIST_H__
#define __XS_KLIST_H__

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINKLIST_FLAG_FIFO                0x00           
#define LINKLIST_FLAG_PRIO                0x01      

typedef struct SysDoubleLinklistNode
{
    struct SysDoubleLinklistNode *node_next;                         
    struct SysDoubleLinklistNode *node_prev;                         
} DoubleLinklistType;                

// Single List 
typedef struct SingleLinklistNode
{
    struct SingleLinklistNode *node_next;                       
} SysSingleLinklistType;              


struct CommonMember
{
    char name[32];
    uint8_t type;
    uint8_t flag;
    DoubleLinklistType list;
};

#define      CONTAINER_OF(item, type, member) \
    ((type *)((char *)(item) - (unsigned long)(&((type *)0)->member)))


#define DOUBLE_LINKLIST_OBJ_INIT(obj) { &(obj), &(obj) }

void InitDoubleLinkList(DoubleLinklistType *linklist_head);
void DoubleLinkListInsertNodeAfter(DoubleLinklistType *linklist, DoubleLinklistType *linklist_node);
void DoubleLinkListInsertNodeBefore(DoubleLinklistType *linklist, DoubleLinklistType *linklist_node);
void DoubleLinkListRmNode(DoubleLinklistType *linklist_node);
int IsDoubleLinkListEmpty(const DoubleLinklistType *linklist);
struct SysDoubleLinklistNode *DoubleLinkListGetHead(const DoubleLinklistType *linklist);
struct SysDoubleLinklistNode *DoubleLinkListGetNext(const DoubleLinklistType *linklist,
        const struct SysDoubleLinklistNode *linklist_node);
unsigned int DoubleLinkListLenGet(const DoubleLinklistType *linklist);

#define SYS_DOUBLE_LINKLIST_ENTRY(item, type, member) \
   CONTAINER_OF(item, type, member)

#define DOUBLE_LINKLIST_FOR_EACH(item, head) \
    for (item = (head)->node_next; item != (head); item = item->node_next)

#define DOUBLE_LINKLIST_FOR_EACH_SAFE(item, node_next, head) \
	for (item = (head)->node_next, node_next = item->node_next; item != (head); \
		item = node_next, node_next = item->node_next)

#define DOUBLE_LINKLIST_FOR_EACH_ENTRY(item, head, member) \
    for (item = SYS_DOUBLE_LINKLIST_ENTRY((head)->node_next, typeof(*item), member); \
         &item->member != (head); \
         item = SYS_DOUBLE_LINKLIST_ENTRY(item->member.node_next, typeof(*item), member))

#define DOUBLE_LINKLIST_FOR_EACH_ENTRY_SAFE(item, node_next, head, member) \
    for (item = SYS_DOUBLE_LINKLIST_ENTRY((head)->node_next, typeof(*item), member), \
         node_next = SYS_DOUBLE_LINKLIST_ENTRY(item->member.node_next, typeof(*item), member); \
         &item->member != (head); \
         item = node_next, node_next = SYS_DOUBLE_LINKLIST_ENTRY(node_next->member.node_next, typeof(*node_next), member))

#define DOUBLE_LINKLIST_FIRST_ENTRY(ptr, type, member) \
              SYS_DOUBLE_LINKLIST_ENTRY((ptr)->node_next, type, member)

#define SYS_SINGLE_LINKLIST_OBJ_INIT(obj) { NONE }

void InitSingleLinkList(SysSingleLinklistType *linklist);
void AppendSingleLinkList(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node);
void SingleLinkListNodeInsert(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node);
unsigned int SingleLinkListGetLen(const SysSingleLinklistType *linklist);
SysSingleLinklistType *SingleLinkListRmNode(SysSingleLinklistType *linklist, SysSingleLinklistType *linklist_node);
SysSingleLinklistType *SingleLinkListGetFirstNode(SysSingleLinklistType *linklist);
SysSingleLinklistType *SingleLinkListGetTailNode(SysSingleLinklistType *linklist);
SysSingleLinklistType *SingleLinkListGetNextNode(SysSingleLinklistType *linklist_node);
int IsSingleLinkListEmpty(SysSingleLinklistType *linklist);

#define SYS_SINGLE_LINKLIST_ENTRY(node, type, member) \
   CONTAINER_OF(node, type, member)

#define SINGLE_LINKLIST_FOR_EACH(item, head) \
    for (item = (head)->node_next; item != NONE; item = item->node_next)

#define SINGLE_LINKLIST_FOR_EACH_ENTRY(item, head, member) \
    for (item = SYS_SINGLE_LINKLIST_ENTRY((head)->node_next, typeof(*item), member); \
         &item->member != (NONE); \
         item = SYS_SINGLE_LINKLIST_ENTRY(item->member.node_next, typeof(*item), member))

#define SINGLE_LINKLIST_FIRST_ENTRY(ptr, type, member) \
    SYS_SINGLE_LINKLIST_ENTRY((ptr)->node_next, type, member)

#define SINGLE_LINKLIST_TAIL_ENTRY(ptr, type, member) \
    SYS_SINGLE_LINKLIST_ENTRY(SingleLinkListGetTailNode(ptr), type, member)

#ifdef __cplusplus
}
#endif

#endif
