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
* @file:    id.c
* @brief:   the management with id for all kernel object
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/15
*
*/

#include <xiuos.h>

static int AllocId(struct IdManager *manager)
{
    int index = 0;
    int end = 0;
    int id = 0;
    uint8 entry = 0;

    NULL_PARAM_CHECK(manager);

    end = (manager->id_max + 7) / 8;

    for (index = 0; index < end; index++)
        if (manager->id_map[index] != 0xff)
            break;
    if (index == end)
        return -1;

    id = index * 8;
    entry = manager->id_map[index];
    while (entry & 0x1) {
        id++;
        entry >>= 1;
    }
    if (id >= manager->id_max)
        return -1;

    manager->id_map[index] |= (0x1 << (id % 8));

    return id;
}

static void FreeId(struct IdManager *manager, uint16 id)
{
    NULL_PARAM_CHECK(manager);

    manager->id_map[id / 8] &= ~(0x1 << (id % 8));
}

static void InsertObj(struct IdManager *manager, struct IdNode *idnode)
{
    NULL_PARAM_CHECK(manager);
    NULL_PARAM_CHECK(idnode);

    DoubleLinklistType *head = &manager->htable[idnode->id % manager->hoffset];

    if (head->node_prev == NONE)
        InitDoubleLinkList(head);

    DoubleLinkListInsertNodeAfter(head, &idnode->link);
}

static struct IdNode *GetObj(struct IdManager *manager, uint16 id)
{
    NULL_PARAM_CHECK(manager);

    DoubleLinklistType *head = &manager->htable[id % manager->hoffset];
    DoubleLinklistType *node = NONE;
    struct IdNode *idnode = NONE;

    if (head->node_prev == NONE) {
        InitDoubleLinkList(head);
        return NONE;
    }

    DOUBLE_LINKLIST_FOR_EACH(node, head) {
        idnode =CONTAINER_OF(node, struct IdNode, link);
        if (idnode->id == id)
            break;
        idnode = NONE;
    }

    return idnode;
}

static struct IdNode *RemoveObj(struct IdManager *manager, struct IdNode *idnode)
{
    NULL_PARAM_CHECK(manager);
    NULL_PARAM_CHECK(idnode);

    DoubleLinkListRmNode(&idnode->link);
}

int IdInsertObj(struct IdManager *manager, struct IdNode *idnode)
{
    int id = 0;

    NULL_PARAM_CHECK(manager);
    NULL_PARAM_CHECK(idnode);

    if ((id = AllocId(manager)) < 0)
        return -1;

    idnode->id = id;
    InsertObj(manager, idnode);

    return id;
}

struct IdNode *IdGetObj(struct IdManager *manager, uint16 id)
{
    if (manager == NONE || id >= manager->id_max)
        return NONE;

    return GetObj(manager, id);
}

void IdRemoveObj(struct IdManager *manager, uint16 id)
{
    NULL_PARAM_CHECK(manager);
    
    struct IdNode *idnode = IdGetObj(manager, id);

    if (idnode == NONE)
        return;

    FreeId(manager, idnode->id);
    RemoveObj(manager, idnode);
    idnode->id = -1;
}
