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
* @file:    user_mem.c
* @brief:   the priviate user api of mem for application 
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/4/20
*
*/

#include "user_api.h"
#include <string.h>

/**
 * This function is provided to allocate memory block.
 *
 * @param size the memory size to be allocated
 *
 * @return pointer on success; NULL on failure
 */
void *UserMalloc(size_t size){
  return  (void *)(KSwitch1(KS_USER_MALLOC,(uintptr_t)size));
}

/**
 * This function is provided to release memory block.
 *
 * @param pointer the memory to be released
 */
void UserFree(void *pointer){
    KSwitch1(KS_USER_FREE,(uintptr_t)pointer);
    return;
}

/**
 * This function is provided to re-allocate memory block.
 *
 * @param pointer the old memory pointer
 * @param size the memory size to be re-allocated
 *
 * @return pointer on success; NULL on failure
 */
void *UserRealloc(void *pointer, size_t size){
    x_size_t newsize = 0;
	  x_size_t oldsize = 0;
	  void *newmem = NONE;

	  /* the given pointer is NULL */
	  if (pointer == NONE)
		  return UserMalloc(size);

    /* parameter detection */
	  if (size == 0) {
		  UserFree(pointer);
		  return NONE;
	  }

	  /* allocate new memory */
	  newmem = UserMalloc(size);
	  if(newmem == NONE) {
        return NONE;
	  }

	  /* copy the old memory and then release old memory pointer */
	  memcpy((char*)newmem, (char*) pointer,size > oldsize ? oldsize : size);
	  UserFree(pointer);

	  return newmem;
}

/**
 * This function will allocate memory blocks and then clear the memory.
 *
 * @param count the number of memory blocks
 * @param size the size of a memory block
 *
 * @return pointer on success; NULL on failure
 */
void *UserCalloc(size_t count, size_t size){
    void *p = NONE;

    /* calls x_malloc to allocate count * size memory */
	  p = UserMalloc(count * size);

	  /* zero the memory */
	  if (p)
		  memset((char*)p, 0, count * size);

	  return p;
}


