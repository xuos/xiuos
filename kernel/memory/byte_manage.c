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
* @file:    byte_manage.c
* @brief:   memory management file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/8
*
*/

#include <xiuos.h>
#include <string.h>

#define MEM_STATS

/* Covert pointer to other structure */
#define PTR2ALLOCNODE(pointer)	            (struct DynamicAllocNode *)(pointer)
#define PTR2FREENODE(pointer)               (struct DynamicFreeNode *)(pointer)

/* Calculate the size of AllocNode and FreeNode */
#define SIZEOF_DYNAMICALLOCNODE_MEM	            (sizeof(struct DynamicAllocNode))
#define SIZEOF_PTR_MEM	                		(sizeof(struct DynamicFreeNode *))
#define SIZEOF_XSFREENODE_MEM	            	(SIZEOF_DYNAMICALLOCNODE_MEM + 2* SIZEOF_PTR_MEM)

/* Set the limits of buddy memory */
#define MEM_LOW_SHIFT	    (6)
#define MEM_HIGH_SHIFT	    (20)
#define MEM_LOW_RANGE	    (1 << MEM_LOW_SHIFT)
#define MEM_HIGH_RANGE	    (1 << MEM_HIGH_SHIFT)
#define MEM_LINKNRS	        (MEM_HIGH_SHIFT-MEM_LOW_SHIFT +1)

/* These masks are used to get the flags and data field of memory blocks */
#define STATIC_BLOCK_MASK                    0x80000000
#define DYNAMIC_BLOCK_MASK                   0x40000000
#define DYNAMIC_BLOCK_NO_EXTMEM_MASK         DYNAMIC_BLOCK_MASK
// #define DYNAMIC_BLOCK_EXTMEM1_MASK           0x40010000 ///< dynamic memory block external SRAM 1
// #define DYNAMIC_BLOCK_EXTMEM2_MASK           0x40020000 ///< dynamic memory block external SRAM 2
// #define DYNAMIC_BLOCK_EXTMEM3_MASK           0x40030000 ///< dynamic memory block external SRAM 3
// #define DYNAMIC_BLOCK_EXTMEM4_MASK           0x40040000 ///< dynamic memory block external SRAM 4
#define DYNAMIC_BLOCK_EXTMEMn_MASK(n)           (DYNAMIC_BLOCK_MASK | (0xFF & n) << 16)

#define ALLOC_BLOCK_MASK                     0xc0000000
#define DYNAMIC_REMAINING_MASK               0x3fffffff

#define SIZEOF_32B	                (32)
#define SIZEOF_64B	                (64)

#define SMALL_SIZE_32B(ITEMSIZE)    ((ITEMSIZE + SIZEOF_DYNAMICALLOCNODE_MEM) * SMALL_NUMBER_32B)    /* Calculate the total size for SIZEOF_32B blocks*/
#define SMALL_SIZE_64B(ITEMSIZE)    ((ITEMSIZE + SIZEOF_DYNAMICALLOCNODE_MEM) * SMALL_NUMBER_64B)    /* Calculate the total size for SIZEOF_64B blocks*/

/**
 * The structure describes an allocated memory block from dynamic buddy memory.
 */
struct DynamicAllocNode
{
	x_size_t size;                /* the size of dynamicAllocNode */
	uint32 prev_adj_size;         /* the size of the previous adjacent node, (dynamic alloc node or dynamic free node */
	uint32 flag;                /* |static_dynamic[32-24]|ext_sram[23-16]|res[15-8]|res[7-0]| */
};

/**
 * The structure describes a released memory block in dynamic buddy memory.
 */
struct DynamicFreeNode
{
	x_size_t size;                /* the size of dynamicAllocNode */
	uint32 prev_adj_size;       /* the size of the previous adjacent node, (dynamic alloc node or dynamic free node */
	uint32 flag;                /* |static_dynamic_region_flag[32-24]|ext_sram_idx[23-16]|res[15-8]|res[7-0]| */

	struct DynamicFreeNode *next;
	struct DynamicFreeNode *prev;
};

/**
 * The structure is the heart of Dynamic memory.
 */
struct DynamicBuddyMemory
{

	x_ubase dynamic_buddy_start;
	x_ubase dynamic_buddy_end;
	x_ubase active_memory;
	x_ubase max_ever_usedmem;
	x_ubase static_memory;
	uint64 mm_total_size;                                 /* record the total size of dynamic buddy memory */

	struct DynamicAllocNode *mm_dynamic_start[1];         /* record the start boundary of dynamic buddy memory */
	struct DynamicAllocNode *mm_dynamic_end[1];           /* record the end boundary of dynamic buddy memory */
	struct DynamicFreeNode mm_freenode_list[MEM_LINKNRS]; /* multiple lists */
	struct DynamicBuddyMemoryDone *done;
};

/**
 * The structure is for static memory mangement, such as SIZEOF_32B and SIZEOF_64B
 */
struct segment
{
	x_size_t  block_size;   /* record the size of static memory block */
	uint8 *	  freelist;     /* list for all free static memory blocks */
	int block_total_count;  /* total static memory blocks */
	int block_free_count;   /* the remaining count of static memory blocks */
	struct StaticMemoryDone *done;
};
/**
 * The index of static memory blocks
 */
enum {
	MM_SEGMENT_32B=0,
	MM_SEGMENT_64B,
	MM_SMALL_SEGMENTS
};

/**
 * The structure is the operation of dynamic alloc node.
 */
struct DynamicAllocNodeDone
{
    int (*JudgeStaticOrDynamic)(struct DynamicAllocNode *address);
    int (*JudgeAllocated)(struct DynamicAllocNode *memory_ptr);
};

struct ByteMemory
{
	struct DynamicBuddyMemory dynamic_buddy_manager;	/* the manager of dynamic buddy memory */
	struct segment static_manager[MM_SMALL_SEGMENTS];	/* the manager of static memory */
	struct DynamicAllocNodeDone *done;
};

/**
 * The structure is the operation of static memory.
 */
struct StaticMemoryDone
{
	void (*init)(struct ByteMemory *byte_memory);
	void* (*malloc)(struct ByteMemory *byte_memory, x_size_t size);
	void (*release)(void *pointer);
};

/**
 * The structure is the operation of dynamic memory.
 */
struct DynamicBuddyMemoryDone
{
	void (*init)(struct DynamicBuddyMemory *dynamic_buddy, x_ubase dynamic_buddy_start,x_ubase dynamic_buddy_size);
	void* (*malloc)(struct DynamicBuddyMemory *dynamic_buddy, x_size_t size, uint32 extsram_mask);
	void (*release)(struct ByteMemory *byte_memory, void *pointer);
	int (*JudgeLegal)(struct DynamicBuddyMemory *dynamic_buddy, void *pointer);
};


static struct ByteMemory ByteManager;

#ifdef SEPARATE_COMPILE
static struct ByteMemory UserByteManager;
#endif

#ifdef MEM_EXTERN_SRAM
static struct ByteMemory ExtByteManager[EXTSRAM_MAX_NUM] = {0};
#endif
/**
 * This function determines whether the address is valid.
 *
 * @param dynamic_buddy
 * @param pointer the memory address
 *
 * @return valid return RET_TRUE; or invalid, return 0.
 */
static int JudgeValidAddressRange(struct DynamicBuddyMemory *dynamic_buddy, void *pointer)
{
	NULL_PARAM_CHECK(dynamic_buddy);
	NULL_PARAM_CHECK(pointer);

    /* the given address is between the physical start address and physical end address */
    if(((struct DynamicAllocNode *)pointer > dynamic_buddy->mm_dynamic_start[0]) && ((struct DynamicAllocNode *)pointer < dynamic_buddy->mm_dynamic_end[0])) {
        return RET_TRUE;
    }
    /* invalid address */
    return 0;
}

/**
 * This function judges whether the memory address is in static or dynamic memory.
 *
 * @param address the given memory address
 *
 * @return the memory type, 1 on static memory area; 0 on dynamic memory area
 */
static int SmallMemTypeAlloc(struct DynamicAllocNode *address)
{
	NULL_PARAM_CHECK(address);

	if(address->flag & STATIC_BLOCK_MASK) {
        return RET_TRUE;
    }

    return 0;
}

/**
 * This function judges whether the memory block is allocated.
 *
 * @param memory_ptr the memory block to be judged
 *
 * @return the result, 1 on allocated node; 0 on release node
 */
static int MmAllocNode(struct DynamicAllocNode *memory_ptr)
{
	NULL_PARAM_CHECK(memory_ptr);

	if(memory_ptr->flag & ALLOC_BLOCK_MASK) {
        return RET_TRUE;
    }
    return 0;
}

static struct DynamicAllocNodeDone NodeDone = {
	SmallMemTypeAlloc,
	MmAllocNode,
};

/**
 * This function calculates the dynamic buddy mm_freenode_list according the give memory size.
 *
 * @param size  the memory size
 *
 * @return the mm_freenode_list index
 */
static int CaculateBuddyIndex(x_size_t size)
{
	int ndx = 0;
    
	if (size < MEM_HIGH_RANGE) {
		size >>= MEM_LOW_SHIFT;
		for (; size > 1; ndx++, size /= 2);
	} else {
		ndx = MEM_LINKNRS - 1;
	}
	return ndx;
}

/**
 * This function inserts freenode into dynamic buddy memory.
 *
 * @param dynamic_buddy the heart dynamic memory structure
 * @param release_node  the node to be released to dynamic_buddy
 */
static void AddNewNodeIntoBuddy(struct DynamicBuddyMemory *dynamic_buddy, struct DynamicFreeNode *release_node)
{
	int ndx = 0;
	struct DynamicFreeNode *nextFreeNode = NONE;
	struct DynamicFreeNode *prevFreeNode = NONE;

	NULL_PARAM_CHECK(dynamic_buddy);
	NULL_PARAM_CHECK(release_node);

	/* calculate the index value */
	ndx = CaculateBuddyIndex(release_node->size);

	/* find the most suitable location, which is sorted by size */
	for (prevFreeNode = &dynamic_buddy->mm_freenode_list[ndx], nextFreeNode = dynamic_buddy->mm_freenode_list[ndx].next;
         nextFreeNode && nextFreeNode->size && nextFreeNode->size < release_node->size;
         prevFreeNode = nextFreeNode, nextFreeNode = nextFreeNode->next);

	/* insert the release_node into the linklist */
    prevFreeNode->next = release_node;
    release_node->prev = prevFreeNode;
    release_node->next = nextFreeNode;

	if (nextFreeNode) {
        nextFreeNode->prev = release_node;
	}
}

/**
 * This function initializes the structure of dynamic buddy memory.
 *
 * @param dynamic_buddy the heart buddy structure
 * @param dynamic_buddy_start the physical start address of dynamic memory
 * @param dynamic_buddy_size the size of dynamic memory
 */
static void InitBuddy(struct DynamicBuddyMemory *dynamic_buddy, x_ubase dynamic_buddy_start,x_ubase dynamic_buddy_size)
{
	struct DynamicFreeNode *node = NONE;

	NULL_PARAM_CHECK(dynamic_buddy);

    /* record the dynamic memory size */
    dynamic_buddy->mm_total_size += (uint64)dynamic_buddy_size;

    /* record the start boundary of dynamic buddy memory */
    dynamic_buddy->mm_dynamic_start[0] = PTR2ALLOCNODE(dynamic_buddy_start);
    dynamic_buddy->mm_dynamic_start[0]->size = SIZEOF_DYNAMICALLOCNODE_MEM;
    dynamic_buddy->mm_dynamic_start[0]->prev_adj_size =  0;
	dynamic_buddy->mm_dynamic_start[0]->flag =  DYNAMIC_BLOCK_MASK;

    /* the initialized free node */
	node =(struct DynamicFreeNode *) ((x_ubase)dynamic_buddy_start + SIZEOF_DYNAMICALLOCNODE_MEM);
	node->size=(dynamic_buddy_size - 2* SIZEOF_DYNAMICALLOCNODE_MEM);
	node->prev_adj_size= SIZEOF_DYNAMICALLOCNODE_MEM;
	node->flag= 0;

    /* record the end boundary of dynamic buddy memory */
    dynamic_buddy->mm_dynamic_end[0] = PTR2ALLOCNODE((x_ubase)dynamic_buddy_start + (x_ubase)dynamic_buddy_size - SIZEOF_DYNAMICALLOCNODE_MEM);
    dynamic_buddy->mm_dynamic_end[0]->size = SIZEOF_DYNAMICALLOCNODE_MEM;
    dynamic_buddy->mm_dynamic_end[0]->prev_adj_size = node->size;
	dynamic_buddy->mm_dynamic_end[0]->flag = DYNAMIC_BLOCK_MASK;

    /* insert node into dynamic buddy memory */
	AddNewNodeIntoBuddy(dynamic_buddy,node);
}

/**
 * This function allocates dynamic memory from dynamic buddy memory.
 *
 * @param dynamic_buddy the heart dynamic buddy structure
 * @param size the memory size to be allocated
 * @param extsram_mask mask the memory region comes from ext sram
 *
 * @return pointer address on success; NULL on failure
 */
static void* BigMemMalloc(struct DynamicBuddyMemory *dynamic_buddy, x_size_t size, uint32 extsram_mask)
{
	int ndx = 0;
	uint32 allocsize = 0;
  	void *result = NONE;
	struct DynamicFreeNode *node = NONE;

	NULL_PARAM_CHECK(dynamic_buddy);

  	/* calculate the real size */
	allocsize = size + SIZEOF_DYNAMICALLOCNODE_MEM;
    /* if the size exceeds the upper limit, return MEM_LINKNRS - 1 */
	if (allocsize >= MEM_HIGH_RANGE) {
		ndx = MEM_LINKNRS - 1; 
	} else {
		/* convert the request size into a linklist index */
		ndx = CaculateBuddyIndex(allocsize);
	}

	/* best-fit method */
	for (node = dynamic_buddy->mm_freenode_list[ndx].next;
			 node && (node->size < allocsize);
			 node = node->next) {
	};
	/* get the best-fit freeNode */
	if (node && (node->size > allocsize)) {
		struct DynamicFreeNode *remainder;
		struct DynamicFreeNode *next;
		uint32 remaining;

		node->prev->next = node->next;
		if (node->next) {
			node->next->prev = node->prev;
		}

		remaining = node->size - allocsize;
		if (remaining >= MEM_LOW_RANGE){
			next = PTR2FREENODE(((char *)node) + node->size);

			/* create the remainder node */
			remainder = PTR2FREENODE(((char *)node) + allocsize);
			remainder->size		= remaining;

			remainder->prev_adj_size = allocsize;
			remainder->flag = 0;

			/* adjust the size of the node */
			node->size = allocsize;
			next->prev_adj_size = remaining;

			/* insert the remainder freeNode back into the dynamic buddy memory */
			AddNewNodeIntoBuddy(dynamic_buddy, remainder);
		}

		/* handle the case of an exact size match */

		node->flag = extsram_mask;
        result = (void *)((char *)node + SIZEOF_DYNAMICALLOCNODE_MEM);
	}

	/* failure allocation */
	if(result == NONE) {
#ifndef MEM_EXTERN_SRAM
	    KPrintf("%s: allocation failed, size %d.\n", __func__,allocsize);
#endif
        return result;
	}

#ifdef MEM_STATS
	/* statistic memory usage */
    dynamic_buddy->active_memory += node->size;
	if(dynamic_buddy->active_memory > dynamic_buddy->max_ever_usedmem)
        dynamic_buddy->max_ever_usedmem = dynamic_buddy->active_memory;
#endif
	return result;
}

/**
 * This function will release dynamic memory. It is called by x_free function.
 *
 * @param pointer
 */
static void BigMemFree( struct ByteMemory *byte_memory, void *pointer)
{
	struct DynamicFreeNode *node = NONE;
	struct DynamicFreeNode *prev = NONE;
	struct DynamicFreeNode *next = NONE;

	NULL_PARAM_CHECK(byte_memory);
	NULL_PARAM_CHECK(pointer);

	/* get the freeNode according the pointer address */
	node = PTR2FREENODE((char*)pointer - SIZEOF_DYNAMICALLOCNODE_MEM);

#ifdef MEM_STATS
	/* statistic memory information */
    byte_memory->dynamic_buddy_manager.active_memory -= node->size;
#endif
	/* get the next sibling freeNode */
	next = PTR2FREENODE((char*)node+node->size);

	if(((next->flag & DYNAMIC_BLOCK_MASK) == 0)) {
		struct DynamicAllocNode *andbeyond;

		andbeyond = PTR2ALLOCNODE((char*)next + next->size);
		next->prev->next = next->next;
		if(next->next) {
			next->next->prev = next->prev;
		}

		node->size += next->size;
		andbeyond->prev_adj_size = node->size;
		next = (struct DynamicFreeNode*)andbeyond;
	}
    /* get the prev sibling freeNode */
	prev = (struct DynamicFreeNode*)((char*)node - node->prev_adj_size );
	if((prev->flag & DYNAMIC_BLOCK_MASK)==0) {

		prev->prev->next=prev->next;
		if(prev->next){
			prev->next->prev = prev->prev;
		}
		prev->size += node->size;
		next->prev_adj_size = prev->size;
		node = prev;
	}
	node->flag = 0;

	/* insert freeNode into dynamic buddy memory */
	AddNewNodeIntoBuddy(&byte_memory->dynamic_buddy_manager,node);
}

static struct DynamicBuddyMemoryDone DynamicDone = {
	InitBuddy,
	BigMemMalloc,
	BigMemFree,
	JudgeValidAddressRange,
};

/**
 * This function initializes the static segment struction.
 *
 * @param static_segment the static_segment to be initialized
 */
static void SmallMemInit(struct ByteMemory *byte_memory)
{
    register x_size_t offset = 0;
    struct segment *item = NONE;
    struct DynamicAllocNode *node = NONE;

	NULL_PARAM_CHECK(byte_memory);

    item = &byte_memory->static_manager[MM_SEGMENT_32B];

    /* allocate memory zone for [32b] */
    item->freelist = byte_memory->dynamic_buddy_manager.done->malloc(&byte_memory->dynamic_buddy_manager, SMALL_SIZE_32B(SIZEOF_32B), DYNAMIC_BLOCK_NO_EXTMEM_MASK);
    if(!item->freelist) {
        KPrintf("%s: no memory for small memory[32B].\n",__func__);
        item->block_free_count = 0;
        return;
    }

    /* initialize the attributes of static_segment_32B */
    item->block_size = SIZEOF_32B;
    item->block_total_count = SMALL_NUMBER_32B;
    item->block_free_count = SMALL_NUMBER_32B;

    for(offset = 0; offset < item->block_total_count; offset++) {
        node = PTR2ALLOCNODE((char*)item->freelist + offset * (SIZEOF_32B + SIZEOF_DYNAMICALLOCNODE_MEM));
        node->size =(x_size_t) ((char*)item->freelist + (offset + 1) * (SIZEOF_32B + SIZEOF_DYNAMICALLOCNODE_MEM));
        node->flag = STATIC_BLOCK_MASK;
    }
    node->size = NONE;

    item = &byte_memory->static_manager[MM_SEGMENT_64B];

    /* allocate memory zone for [64B] */
    item->freelist = byte_memory->dynamic_buddy_manager.done->malloc(&byte_memory->dynamic_buddy_manager, SMALL_SIZE_64B(SIZEOF_64B),DYNAMIC_BLOCK_NO_EXTMEM_MASK);
    if(!item->freelist) {
        KPrintf("%s: no memory for small memory[64B].\n",__func__);
        return;
    }

    /* initialize the attributes of static_segment_64B */
    item->block_size = SIZEOF_64B;
    item->block_total_count = SMALL_NUMBER_64B;
    item->block_free_count = SMALL_NUMBER_64B;

    for(offset = 0; offset < item->block_total_count; offset++) {
        node = PTR2ALLOCNODE((char*)item->freelist + offset * (SIZEOF_64B + SIZEOF_DYNAMICALLOCNODE_MEM));
        node->size =(x_size_t) ((char*)item->freelist + (offset + 1) * (SIZEOF_64B + SIZEOF_DYNAMICALLOCNODE_MEM));
        node->flag = STATIC_BLOCK_MASK;
    }
    node->size = NONE;

#ifdef MEM_STATS
    /* statistic static memory information */
    byte_memory->dynamic_buddy_manager.static_memory = SMALL_SIZE_64B(SIZEOF_64B) + SMALL_SIZE_32B(SIZEOF_32B);
#endif
}

/**
 *
 * This function will release the static memory block to static segment.
 *
 * @param pointer the memory to be released
 */
static void SmallMemFree(void *pointer)
{
	struct segment *static_segment = NONE;
	struct DynamicAllocNode *node = NONE;

	NULL_PARAM_CHECK(pointer);

	/* get the allocNode */
	node = PTR2ALLOCNODE((char*)pointer-SIZEOF_DYNAMICALLOCNODE_MEM);
    static_segment = (struct segment*)(x_size_t)node->size;

    /* update the statistic information of static_segment */
	node->size = (x_size_t)static_segment->freelist;
    static_segment->freelist = (uint8 *)node;
    static_segment->block_free_count++;

    /* parameter detection */
	CHECK(static_segment->block_free_count <= static_segment->block_total_count);
}

/**
 * This funcation allocates a static memory block from static segment.
 *
 * @param static_segment the heart static segment structure to allocate static memory
 * @param size the size to be allocated
 *
 * @return pointer address on success; NULL on failure
 */
static void *SmallMemMalloc(struct ByteMemory *byte_memory, x_size_t size)
{
	uint8 i = 0;
	void *result = NONE;
	struct DynamicAllocNode *node = NONE;
	struct segment *static_segment = NONE;

	NULL_PARAM_CHECK(byte_memory);

	if (size == SIZEOF_32B)
	   static_segment = &byte_memory->static_manager[0];
	else
	   static_segment = &byte_memory->static_manager[1];

	/* current static segment has free static memory block */
	if(static_segment->block_free_count>0) {
	    /* get the head static memory block */
        result = static_segment->freelist;
        node = PTR2ALLOCNODE(static_segment->freelist);
		node->flag = STATIC_BLOCK_MASK;

		/* update the statistic information of static segment */
        static_segment->freelist = (uint8 *)(long)(node->size);
        static_segment->block_free_count--;
		node->size = (long)static_segment;
	}

	if(result) {
	    /* return static memory block */
		return (char*)result + SIZEOF_DYNAMICALLOCNODE_MEM;
	}

	/* the static memory block is exhausted, now turn to dynamic buddy memory for allocation. */
    result = byte_memory->dynamic_buddy_manager.done->malloc(&byte_memory->dynamic_buddy_manager, size, DYNAMIC_BLOCK_NO_EXTMEM_MASK);
#ifdef MEM_EXTERN_SRAM
	if(NONE == result) {
		for(i = 0; i < EXTSRAM_MAX_NUM; i++) {
			if(NONE != ExtByteManager[i].done) {
				result = ExtByteManager[i].dynamic_buddy_manager.done->malloc(&ExtByteManager[i].dynamic_buddy_manager, size, DYNAMIC_BLOCK_EXTMEMn_MASK(i + 1));
				if (result){
					CHECK(ExtByteManager[i].dynamic_buddy_manager.done->JudgeLegal(&ExtByteManager[i].dynamic_buddy_manager, ret - SIZEOF_DYNAMICALLOCNODE_MEM));
					break;
				}	
			}
		}
	}
#endif
	return result;
}

static struct StaticMemoryDone StaticDone = {
	SmallMemInit,
	SmallMemMalloc,
	SmallMemFree,
};

/**
 * This function is provided to allocate memory block.
 *
 * @param size the memory size to be allocated
 *
 * @return pointer on success; NULL on failure
 */
void *x_malloc(x_size_t size)
{
	uint8 i = 0;
	void *ret = NONE;
	register x_base lock = 0;

	 /* parameter detection */

#ifdef MEM_EXTERN_SRAM
		/* parameter detection */
	if(size == 0 ){
		 return NONE;
	}
	if((size >  ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start - ByteManager.dynamic_buddy_manager.active_memory)){
		lock = CriticalAreaLock();
		/* alignment */
		size = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
		goto try_extmem;
	}
	   
#else
		/* parameter detection */
	if((size == 0) || (size >  ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start - ByteManager.dynamic_buddy_manager.active_memory))
	    return NONE;
#endif
	/* hold lock before allocation */
	lock = CriticalAreaLock();
    /* alignment */
	size = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
	/* determine allocation operation from static segments or dynamic buddy memory */
#ifdef KERNEL_SMALL_MEM_ALLOC
	if(size <= SIZEOF_32B) {
		ret = ByteManager.static_manager[0].done->malloc(&ByteManager,SIZEOF_32B);
	} else if(size <= SIZEOF_64B) {
		ret = ByteManager.static_manager[1].done->malloc(&ByteManager,SIZEOF_64B);
	} else
#endif
	{
        ret = ByteManager.dynamic_buddy_manager.done->malloc(&ByteManager.dynamic_buddy_manager, size, DYNAMIC_BLOCK_NO_EXTMEM_MASK);
		if(ret != NONE)
        	CHECK(ByteManager.dynamic_buddy_manager.done->JudgeLegal(&ByteManager.dynamic_buddy_manager, ret - SIZEOF_DYNAMICALLOCNODE_MEM));

#ifdef MEM_EXTERN_SRAM
try_extmem:
		if(NONE == ret) {
			for(i = 0; i < EXTSRAM_MAX_NUM; i++) {
				if(NONE != ExtByteManager[i].done) {
					ret = ExtByteManager[i].dynamic_buddy_manager.done->malloc(&ExtByteManager[i].dynamic_buddy_manager, size, DYNAMIC_BLOCK_EXTMEMn_MASK(i + 1));
					if (ret){
						CHECK(ExtByteManager[i].dynamic_buddy_manager.done->JudgeLegal(&ExtByteManager[i].dynamic_buddy_manager, ret - SIZEOF_DYNAMICALLOCNODE_MEM));
						break;
					}
					
				}
			}
		}
#endif
	}
	/* release lock */
	CriticalAreaUnLock(lock);
  	return ret;
}

/**
 * This function is provided to re-allocate memory block.
 *
 * @param pointer the old memory pointer
 * @param size the memory size to be re-allocated
 *
 * @return pointer on success; NULL on failure
 */
void *x_realloc(void *pointer, x_size_t size)
{
	x_size_t newsize = 0;
	x_size_t oldsize = 0;
	void *newmem = NONE;
	struct DynamicAllocNode *oldnode = NONE;

	/* the given pointer is NULL */
	if (pointer == NONE)
		return x_malloc(size);

    /* parameter detection */
	if (size == 0) {
		x_free(pointer);
		return NONE;
	}
    CHECK(ByteManager.dynamic_buddy_manager.done->JudgeLegal(&ByteManager.dynamic_buddy_manager,pointer));

	/* alignment and calculate the real size */
    newsize = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
	newsize += SIZEOF_DYNAMICALLOCNODE_MEM;

	oldnode= PTR2ALLOCNODE((char*)pointer - SIZEOF_DYNAMICALLOCNODE_MEM);
    CHECK(ByteManager.done->JudgeAllocated(oldnode));

    /* achieve the old memory size */
	if(ByteManager.done->JudgeStaticOrDynamic(oldnode)) {
        oldsize = ((struct segment*)(long)(oldnode->size))->block_size;
	} else {
        oldsize = oldnode->size - SIZEOF_DYNAMICALLOCNODE_MEM;
	}

	/* allocate new memory */
	newmem = x_malloc(size);
	if(newmem == NONE) {
        return NONE;
	}

	/* copy the old memory and then release old memory pointer */
	memcpy((char*)newmem, (char*) pointer,size > oldsize ? oldsize : size);
	x_free(pointer);

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
void *x_calloc(x_size_t count, x_size_t size)
{
	void *p = NONE;

    /* parameter detection */
    if(count * size >  ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start - ByteManager.dynamic_buddy_manager.active_memory)
        return NONE;

    /* calls x_malloc to allocate count * size memory */
	p = x_malloc(count * size);

	/* zero the memory */
	if (p)
		memset((char*)p, 0, count * size);

	return p;
}

/**
 * This function is provided to release memory block.
 *
 * @param pointer the memory to be released
 */
void x_free(void *pointer)
{
	x_base lock = 0;
	struct DynamicAllocNode *node = NONE;

    /* parameter detection */
	if (pointer == NONE)
		return ;

    CHECK(ByteManager.dynamic_buddy_manager.done->JudgeLegal(&ByteManager.dynamic_buddy_manager,pointer));

    /* hold lock before release */
	lock = CriticalAreaLock();
	node = PTR2ALLOCNODE((char*)pointer-SIZEOF_DYNAMICALLOCNODE_MEM);
    CHECK(ByteManager.done->JudgeAllocated(node));
	
    /* judge release the memory block ro static_segment or dynamic buddy memory */
#ifdef KERNEL_SMALL_MEM_ALLOC
	if(node->flag & STATIC_BLOCK_MASK) {
		ByteManager.static_manager->done->release(pointer);
	} else
#endif
	{
#ifdef MEM_EXTERN_SRAM
	/* judge the pointer is not malloced from extern memory*/
	if(0 == (node->flag & 0xFF0000)) {
		ByteManager.dynamic_buddy_manager.done->release(&ByteManager,pointer);
	}

	/* judge the pointer is malloced from extern memory*/
	if(0 != (node->flag & 0xFF0000)) {
		ExtByteManager[((node->flag & 0xFF0000) >> 16) - 1].dynamic_buddy_manager.done->release(&ExtByteManager[((node->flag & 0xFF0000) >> 16) - 1],pointer);
	}
#else
	ByteManager.dynamic_buddy_manager.done->release(&ByteManager,pointer);
#endif		
	}
	/* release the lock */
	CriticalAreaUnLock(lock);
}

#ifdef MEM_EXTERN_SRAM
/**
 * This function initializes the dynamic buddy memory of extern sram.
 *
 * @param start_phy_address the start physical address for static and dynamic memory
 * @param end_phy_address the end physical address for static and dynamic memory
 * @param extsram_idx the idx of extsram chip
 */
void ExtSramInitBoardMemory(void *start_phy_address, void *end_phy_address, uint8 extsram_idx)
{
	register x_size_t offset = 0;

	NULL_PARAM_CHECK(start_phy_address);
	NULL_PARAM_CHECK(end_phy_address);

	KDEBUG_NOT_IN_INTERRUPT;
	struct DynamicBuddyMemory *uheap = &ExtByteManager[extsram_idx].dynamic_buddy_manager;

	/* align begin and end addr to page */
	ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_start = ALIGN_MEN_UP((x_ubase)start_phy_address, MM_PAGE_SIZE);
	ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_end   = ALIGN_MEN_DOWN((x_ubase)end_phy_address, MM_PAGE_SIZE);
    KPrintf("%s: 0x%x-0x%x extsram_idx = %d\n",__func__,ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_start,ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_end, extsram_idx);

    /* parameter detection */
	if (ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_start >= ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_end) {
		KPrintf("ExtSramInitBoardMemory, wrong address[0x%x - 0x%x]\n",
				(x_ubase)start_phy_address, (x_ubase)end_phy_address);
		return;
	}

    uheap->mm_total_size = 0;
	memset(uheap->mm_freenode_list, 0, SIZEOF_XSFREENODE_MEM * MEM_LINKNRS);

	/* initialize the freeNodeList */
	for (offset = 1; offset < MEM_LINKNRS; offset++) {
		uheap->mm_freenode_list[offset - 1].next = &uheap->mm_freenode_list[offset];
		uheap->mm_freenode_list[offset].prev	 = &uheap->mm_freenode_list[offset - 1];
	}

	ExtByteManager[extsram_idx].dynamic_buddy_manager.done = &DynamicDone;
	ExtByteManager[extsram_idx].done = &NodeDone;


    /* dynamic buddy memory initialization */
	ExtByteManager[extsram_idx].dynamic_buddy_manager.done->init(&ExtByteManager[extsram_idx].dynamic_buddy_manager, ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_start, ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_end - ExtByteManager[extsram_idx].dynamic_buddy_manager.dynamic_buddy_start);
}
#endif

/**
 * This function initializes the static segments and dynamic buddy memory structures.
 *
 * @param start_phy_address the start physical address for static and dynamic memory
 * @param end_phy_address the end physical address for static and dynamic memory
 */
void InitBoardMemory(void *start_phy_address, void *end_phy_address)
{
	register x_size_t offset = 0;

	NULL_PARAM_CHECK(start_phy_address);
	NULL_PARAM_CHECK(end_phy_address);

	KDEBUG_NOT_IN_INTERRUPT;
	struct DynamicBuddyMemory *mheap = &ByteManager.dynamic_buddy_manager;

	/* align begin and end addr to page */
	ByteManager.dynamic_buddy_manager.dynamic_buddy_start = ALIGN_MEN_UP((x_ubase)start_phy_address, MM_PAGE_SIZE);
	ByteManager.dynamic_buddy_manager.dynamic_buddy_end   = ALIGN_MEN_DOWN((x_ubase)end_phy_address, MM_PAGE_SIZE);
    KPrintf("%s: 0x%x-0x%x \n",__func__,ByteManager.dynamic_buddy_manager.dynamic_buddy_start,ByteManager.dynamic_buddy_manager.dynamic_buddy_end);

    /* parameter detection */
	if (ByteManager.dynamic_buddy_manager.dynamic_buddy_start >= ByteManager.dynamic_buddy_manager.dynamic_buddy_end) {
		KPrintf("InitBoardMemory, wrong address[0x%x - 0x%x]\n",
				(x_ubase)start_phy_address, (x_ubase)end_phy_address);
		return;
	}

    mheap->mm_total_size = 0;
	memset(mheap->mm_freenode_list, 0, SIZEOF_XSFREENODE_MEM * MEM_LINKNRS);

	/* initialize the freeNodeList */
	for (offset = 1; offset < MEM_LINKNRS; offset++) {
		mheap->mm_freenode_list[offset - 1].next = &mheap->mm_freenode_list[offset];
		mheap->mm_freenode_list[offset].prev	 = &mheap->mm_freenode_list[offset - 1];
	}

	ByteManager.dynamic_buddy_manager.done = &DynamicDone;
	ByteManager.static_manager[MM_SEGMENT_32B].done = &StaticDone;
	ByteManager.static_manager[MM_SEGMENT_64B].done = &StaticDone;
	ByteManager.done = &NodeDone;


    /* dynamic buddy memory initialization */
	ByteManager.dynamic_buddy_manager.done->init(&ByteManager.dynamic_buddy_manager, ByteManager.dynamic_buddy_manager.dynamic_buddy_start, ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start);

    /* dynamic static segments initialization */
#ifdef KERNEL_SMALL_MEM_ALLOC
	ByteManager.static_manager->done->init(&ByteManager);
#endif
}

#ifdef SEPARATE_COMPILE

/**
 * This function is provided to allocate user memory block.
 *
 * @param size the memory size to be allocated
 *
 * @return pointer on success; NULL on failure
 */
void *x_umalloc(x_size_t size)
{
	uint8 i = 0;
	void *ret = NONE;
	register x_base lock = 0;


#ifdef MEM_EXTERN_SRAM
	/* parameter detection */
	if(size == 0 ){
		 return NONE;
	}
	if((size >  ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start - ByteManager.dynamic_buddy_manager.active_memory)){
		lock = CriticalAreaLock();
		/* alignment */
		size = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
		goto try_extmem;
	}
	   
#else
	/* parameter detection */
	if((size == 0) || (size >  UserByteManager.dynamic_buddy_manager.dynamic_buddy_end - UserByteManager.dynamic_buddy_manager.dynamic_buddy_start - UserByteManager.dynamic_buddy_manager.active_memory))
	    return NONE;
#endif

	/* hold lock before allocation */
	lock = CriticalAreaLock();
    /* alignment */
	size = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
    ret = UserByteManager.dynamic_buddy_manager.done->malloc(&UserByteManager.dynamic_buddy_manager,size);
    if(ret != NONE)
        CHECK(UserByteManager.dynamic_buddy_manager.done->JudgeLegal(&UserByteManager.dynamic_buddy_manager, ret - SIZEOF_DYNAMICALLOCNODE_MEM));


#ifdef MEM_EXTERN_SRAM
try_extmem:
	if(NONE == ret) {
		for(i = 0; i < EXTSRAM_MAX_NUM; i++) {
			if(NONE != ExtByteManager[i].done) {
				ret = ExtByteManager[i].dynamic_buddy_manager.done->malloc(&ExtByteManager[i].dynamic_buddy_manager, size, DYNAMIC_BLOCK_EXTMEMn_MASK(i + 1));
				if (ret) {
					CHECK(ExtByteManager[i].dynamic_buddy_manager.done->JudgeLegal(&ExtByteManager[i].dynamic_buddy_manager, ret - SIZEOF_DYNAMICALLOCNODE_MEM));
					break;
				}
			}
		}
	}
#endif
	/* release lock */
	CriticalAreaUnLock(lock);
  	return ret;
}

/**
 * This function is provided to re-allocate memory block.
 *
 * @param pointer the old memory pointer
 * @param size the memory size to be re-allocated
 *
 * @return pointer on success; NULL on failure
 */
void *x_urealloc(void *pointer, x_size_t size)
{
	x_size_t newsize = 0;
	x_size_t oldsize = 0;
	void *newmem = NONE;
	struct DynamicAllocNode *oldnode = NONE;

	/* the given pointer is NULL */
	if (pointer == NONE)
		return x_umalloc(size);

    /* parameter detection */
	if (size == 0) {
		x_ufree(pointer);
		return NONE;
	}
    CHECK(UserByteManager.dynamic_buddy_manager.done->JudgeLegal(&UserByteManager.dynamic_buddy_manager,pointer));

	/* alignment and calculate the real size */
    newsize = ALIGN_MEN_UP(size, MEM_ALIGN_SIZE);
	newsize += SIZEOF_DYNAMICALLOCNODE_MEM;

	oldnode= PTR2ALLOCNODE((char*)pointer - SIZEOF_DYNAMICALLOCNODE_MEM);
    CHECK(UserByteManager.done->JudgeAllocated(oldnode));

    /* achieve the old memory size */
	if(UserByteManager.done->JudgeStaticOrDynamic(oldnode)) {
        oldsize = ((struct segment*)(oldnode->size))->block_size;
	} else {
        oldsize = oldnode->size - SIZEOF_DYNAMICALLOCNODE_MEM;
	}

	/* allocate new memory */
	newmem = x_umalloc(size);
	if(newmem == NONE) {
        return NONE;
	}

	/* copy the old memory and then release old memory pointer */
	memcpy((char*)newmem, (char*) pointer,size > oldsize ? oldsize : size);
	x_ufree(pointer);

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
void *x_ucalloc(x_size_t count, x_size_t size)
{
	void *p = NONE;

    /* parameter detection */
    if(count * size >  UserByteManager.dynamic_buddy_manager.dynamic_buddy_end - UserByteManager.dynamic_buddy_manager.dynamic_buddy_start - UserByteManager.dynamic_buddy_manager.active_memory)
        return NONE;

    /* calls x_malloc to allocate count * size memory */
	p = x_umalloc(count * size);

	/* zero the memory */
	if (p)
		memset((char*)p, 0, count * size);

	return p;
}

/**
 * This function is provided to release memory block.
 *
 * @param pointer the memory to be released
 */
void x_ufree(void *pointer)
{
	x_base lock = 0;
	struct DynamicAllocNode *node = NONE;

    /* parameter detection */
	if (pointer == NONE)
		return ;
    CHECK(UserByteManager.dynamic_buddy_manager.done->JudgeLegal(&UserByteManager.dynamic_buddy_manager,pointer));

    /* hold lock before release */
	lock = CriticalAreaLock();
	node = PTR2ALLOCNODE((char*)pointer-SIZEOF_DYNAMICALLOCNODE_MEM);
    CHECK(UserByteManager.done->JudgeAllocated(node));

#ifdef MEM_EXTERN_SRAM
	/* judge the pointer is not malloced from extern memory*/
	if(0 == (node->flag & 0xFF0000)) {
		UserByteManager.dynamic_buddy_manager.done->release(&ByteManager,pointer);
	}

	/* judge the pointer is malloced from extern memory*/
	if(0 != (node->flag & 0xFF0000)) {
		ExtByteManager[((node->flag & 0xFF0000) >> 16) - 1].dynamic_buddy_manager.done->release(&ExtByteManager[((node->flag & 0xFF0000) >> 16) - 1],pointer);
	}

#else
	UserByteManager.dynamic_buddy_manager.done->release(&UserByteManager,pointer);
#endif
	/* release the lock */
	CriticalAreaUnLock(lock);
}

/**
 * This function initializes the static segments and dynamic buddy memory structures.
 *
 * @param start_phy_address the start physical address for static and dynamic memory
 * @param end_phy_address the end physical address for static and dynamic memory
 */
void UserInitBoardMemory(void *start_phy_address, void *end_phy_address)
{
	register x_size_t offset = 0;

	NULL_PARAM_CHECK(start_phy_address);
	NULL_PARAM_CHECK(end_phy_address);

	KDEBUG_NOT_IN_INTERRUPT;
	struct DynamicBuddyMemory *uheap = &UserByteManager.dynamic_buddy_manager;

	/* align begin and end addr to page */
	UserByteManager.dynamic_buddy_manager.dynamic_buddy_start = ALIGN_MEN_UP((x_ubase)start_phy_address, MM_PAGE_SIZE);
	UserByteManager.dynamic_buddy_manager.dynamic_buddy_end   = ALIGN_MEN_DOWN((x_ubase)end_phy_address, MM_PAGE_SIZE);
    KPrintf("%s: 0x%x-0x%x \n",__func__,UserByteManager.dynamic_buddy_manager.dynamic_buddy_start,UserByteManager.dynamic_buddy_manager.dynamic_buddy_end);

    /* parameter detection */
	if (UserByteManager.dynamic_buddy_manager.dynamic_buddy_start >= UserByteManager.dynamic_buddy_manager.dynamic_buddy_end) {
		KPrintf("InitBoardMemory, wrong address[0x%x - 0x%x]\n",
				(x_ubase)start_phy_address, (x_ubase)end_phy_address);
		return;
	}

    uheap->mm_total_size = 0;
	memset(uheap->mm_freenode_list, 0, SIZEOF_XSFREENODE_MEM * MEM_LINKNRS);

	/* initialize the freeNodeList */
	for (offset = 1; offset < MEM_LINKNRS; offset++) {
		uheap->mm_freenode_list[offset - 1].next = &uheap->mm_freenode_list[offset];
		uheap->mm_freenode_list[offset].prev	 = &uheap->mm_freenode_list[offset - 1];
	}

	UserByteManager.dynamic_buddy_manager.done = &DynamicDone;
	UserByteManager.done = &NodeDone;


    /* dynamic buddy memory initialization */
	UserByteManager.dynamic_buddy_manager.done->init(&UserByteManager.dynamic_buddy_manager, UserByteManager.dynamic_buddy_manager.dynamic_buddy_start, UserByteManager.dynamic_buddy_manager.dynamic_buddy_end - UserByteManager.dynamic_buddy_manager.dynamic_buddy_start);

}

#endif

#ifdef MEM_STATS

/**
 * This function obtains the statistic information about memory
 *
 * @param total_memory the total memory
 * @param used_memory  the meory being used
 * @param max_used_memory the max allocated memory
 */
void MemoryInfo(uint32 *total_memory, uint32 *used_memory, uint32 *max_used_memory)
{
	if (NONE != total_memory)
		*total_memory = ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start;

	if (NONE != used_memory)
		*used_memory = ByteManager.dynamic_buddy_manager.active_memory;

	if (NONE != max_used_memory)
		*max_used_memory = ByteManager.dynamic_buddy_manager.max_ever_usedmem;
}

#ifdef TOOL_SHELL
#include <shell.h>

void ShowBuddy(void);
void ShowMemory(void);
/**
 * This function will list the statistic information about memory.
 */
void ShowMemory(void)
{
	int i = 0;
	KPrintf("total memory: %d\n", ByteManager.dynamic_buddy_manager.dynamic_buddy_end - ByteManager.dynamic_buddy_manager.dynamic_buddy_start);
	KPrintf("used memory : %d\n", ByteManager.dynamic_buddy_manager.active_memory);
	KPrintf("maximum allocated memory: %d\n", ByteManager.dynamic_buddy_manager.max_ever_usedmem);
	KPrintf("total cache szie: %d, %d/%d[32B],%d/%d[64B]\n", ByteManager.dynamic_buddy_manager.static_memory,ByteManager.static_manager[0].block_free_count,SMALL_NUMBER_32B,ByteManager.static_manager[1].block_free_count,SMALL_NUMBER_64B);
#ifdef MEM_EXTERN_SRAM
	for(i = 0; i < EXTSRAM_MAX_NUM; i++) {
		if(NONE != ExtByteManager[i].done){
			KPrintf("\nlist extern sram[%d] memory information\n\n",i);
			KPrintf("extern sram total memory: %d\n", ExtByteManager[i].dynamic_buddy_manager.dynamic_buddy_end - ExtByteManager[i].dynamic_buddy_manager.dynamic_buddy_start);
			KPrintf("extern sram used memory : %d\n", ExtByteManager[i].dynamic_buddy_manager.active_memory);
			KPrintf("extern sram maximum allocated memory: %d\n", ExtByteManager[i].dynamic_buddy_manager.max_ever_usedmem);
		}
	}
#endif
	ShowBuddy();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
											ShowMemory,ShowMemory,list memory usage information);
/**
 * This function will list the freeNodeList information on dynamic buddy memory.
 */
void ShowBuddy(void)
{
	int i = 0;
	int lock = 0;
	struct DynamicFreeNode *debug = NONE;

	lock = CriticalAreaLock();
	KPrintf("\n\033[41;1mlist memory information\033[0m\n", __func__);
    for (debug = ByteManager.dynamic_buddy_manager.mm_freenode_list[0].next;
         debug;debug = debug->next){
        KPrintf("%s,current is %x,next is %x, size %u, flag %x\n",__func__, debug, debug->next,debug->size,debug->flag);
    };
    KPrintf("\nlist memory information\n\n");
#ifdef MEM_EXTERN_SRAM
	for(i = 0; i < EXTSRAM_MAX_NUM; i++) {
		if(NONE != ExtByteManager[i].done){
			KPrintf("\nlist extern sram[%d] memory information\n\n",i);
			for (debug = ExtByteManager[i].dynamic_buddy_manager.mm_freenode_list[0].next;
				debug;debug = debug->next){
				KPrintf("%s,current is %x,next is %x, size %u, flag %x\n",__func__, debug, debug->next,debug->size,debug->flag);
			};
		}
	}
	
#endif
	CriticalAreaUnLock(lock);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
ShowBuddy,ShowBuddy,list memory usage information);
#endif     
#endif     

