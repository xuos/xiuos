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
* @file:    xs_id.h
* @brief:   function declaration and structure defintion of id manager
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#ifndef XS_ID_H
#define XS_ID_H

#include <xs_klist.h>

#ifdef __cplusplus
extern "C" {
#endif

struct IdNode 
{
    uint16 id;
    DoubleLinklistType link;
};

struct IdManager
{
    uint16 id_max;
    uint16 hoffset;

    uint8 *id_map;
    DoubleLinklistType *htable;
};



#define DECLARE_ID_MANAGER(_name, _id_max) \
uint8 __id_map_##_name[((_id_max) + 7) / 8]; \
DoubleLinklistType __htable_##_name[(_id_max) < ID_HTABLE_SIZE ? (_id_max) : ID_HTABLE_SIZE]; \
struct IdManager _name = { \
    .id_max = _id_max, \
    .hoffset = (_id_max) < ID_HTABLE_SIZE ? (_id_max) : ID_HTABLE_SIZE, \
    .id_map = __id_map_##_name, \
    .htable = __htable_##_name \
};

int IdInsertObj(struct IdManager *manager, struct IdNode *idnode);
struct IdNode *IdGetObj(struct IdManager *manager, uint16 id);
void IdRemoveObj(struct IdManager *manager, uint16 id);

#ifdef __cplusplus
}
#endif

#endif
