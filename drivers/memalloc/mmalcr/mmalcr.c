/*******************************************************************************
 * mmalcr.c
 *
 * Copyright (C) 2011 Sololz<sololz.luo@gmail.com>.
 * 
 * Main file of a allocator with out external fragments. This allocation 
 * based on allocator instance, and the allocate method is greatly improved
 * comparing to the simple single memory block list. 
 * This allocator is designed with three parts to store memory nodes in-
 * formation, one is the memory node Binary-Tree stores using memory nodes, 
 * one is idle memory nodes hash table, the last is a list contains all 
 * memory nods. 
 * 1. RB-Tree will improve the searching time cost from O(n) to O(log(n)). 
 * Hash table can't make it to O(log(n)), but much better than O(n). BT is 
 * indexed by memory node address, storing in using nodes because free API 
 * is designed just passing address parameter.
 * 2. Hash table indexed by size of memory nodes storing not used memory
 * nods because alloc API is designed only required by size. And not like
 * address, sizes might be the same, hash table can't make the worst time 
 * cost level to O(log(n)), but much better than O(n).
 * 3. All-Node-List stores all memory node, and the head of list stores a
 * memory region of the managed memory start, address of next node must
 * be larger than pre one.
 *
 * You might have notice that, there isn't any lock to protect allocator
 * data structure. Yes, I remove all of them, because I think caller can
 * do it as they wish. The main reason is that, in Linux kernel, at some
 * places, atomic code is required, tasklet etc. Also I refused to use
 * spin_lock because of the busy lock of CPU and the support of spinlock
 * will greatly increase the code amount.
 * I never suppose this allocator can be used any where, in user space, 
 * there are many function libraries provides similar allocators and use
 * much better allocate methods, STL templates in C++ etc.
 * The design method of a object is that, the create and release function
 * interface will do as much check and calculations as possible. Create
 * and release is designed to cost more time to gain more information.
 * Alloc and free API should be as faster as possible, so if any ops can
 * be done at create/release step, they should be there.
 *
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.0  2011/01/17 Sololz
 * 	Create this file.
 ******************************************************************************/

#include "mmalcr_private.h"

/* ########################################################################## */
/* Export function API. */

/*
 * FUNCTION
 * mmalcr_create()
 *
 * Create a allocator.
 */
mmalcr_t mmalcr_create(uint32_t size, uint32_t mm_start, uint32_t align_order)
{
	mmalcr_priv_t *alcr = NULL;
	uint32_t left_size = 0;

	if (ma_unlikely((size > MMALCR_MAX_PROCESS_SIZE) ||
			(size < MMALCR_MIN_PROCESS_SIZE))) {
		ma_error("*size* invalid.\n");
		return NULL;
	} else if (ma_unlikely((align_order > MMALCR_MAX_ALIGN_ORDER) ||
			(align_order < MMALCR_MIN_ALIGN_ORDER))) {
		ma_error("*align_order* invalid.\n");
		return NULL;
	} else if (ma_unlikely(mm_start % (1 << align_order))) {
		ma_error("*mm_start* not aligned.\n");
		return NULL;
	} else if (ma_unlikely(size > (MMALCR_MAX_ADDRESS - mm_start))) {
		ma_error("Memory region right bound overflow.\n");
		return NULL;
	}

	/* Allocate allocator data structure. */
	alcr = (mmalcr_priv_t *)ma_malloc(sizeof(mmalcr_priv_t));
	if (ma_unlikely(alcr == NULL)) {
		ma_error("Allocate allocator data structure error.\n");
		return NULL;
	}
	memset(alcr, 0x00, sizeof(mmalcr_priv_t));
	alcr->size = size;
	alcr->mm_start = mm_start;
	alcr->align_order = align_order;

	/* Create the first/original memory block node. */
	alcr->list = (mmalcr_node_t *)ma_malloc(sizeof(mmalcr_node_t));
	if (ma_unlikely(alcr->list == NULL)) {
		ma_error("Allocate first memory block node error.\n");
		goto mmalcr_create_error_return;
	}
	memset(alcr->list, 0x00, sizeof(mmalcr_node_t));
	alcr->list->size = size;
	alcr->list->addr = mm_start;
	alcr->list->align_order = align_order;

	/* Create and initialize hash table, first calculate the upper limit of 
	 * size order. */
	{
		uint32_t oi = 0;

		alcr->upper_order = 0;
		while (size > 0) {
			size >>= 1;
			alcr->upper_order++;
		}
		if (alcr->size == (1 << (alcr->upper_order - 1))) {
			alcr->upper_order--;
		}
		alcr->upper_order = (alcr->upper_order > MMALCR_MAX_PROCESS_SIZE_ORDER) ? \
			      MMALCR_MAX_PROCESS_SIZE_ORDER : alcr->upper_order;

		for (oi = alcr->align_order; oi <= alcr->upper_order; oi++) {
			uint32_t avg_cnt = 0;
			uint32_t avg_interval = 0;
			uint32_t tmp_cnt = 0;
			uint32_t tmp_index = 0;

			/* The divide times must not surpass the max divider. */
			avg_cnt = 1 << (oi - alcr->align_order);
			avg_cnt = (avg_cnt > MMALCR_MAX_DIVIDE) ? MMALCR_MAX_DIVIDE : avg_cnt;
			/* Adjust big size correspond divider, cuz big blocks will have a large *avg_cnt*
			 * value, but actually a memory block to managed can only contains few big blocks. */
			tmp_cnt = (alcr->upper_order - oi + 1) << 1;
			avg_cnt = (avg_cnt > tmp_cnt) ? tmp_cnt : avg_cnt;

			/* Calculate average interval for second level hash table. */
			avg_interval = ((1 << (oi - 1)) / avg_cnt);
			tmp_index = oi - MMALCR_MIN_ALIGN_ORDER;

			alcr->hash[tmp_index].entry = (mmalcr_hash_entry_t *)ma_malloc(avg_cnt * \
					sizeof(mmalcr_hash_entry_t));
			if (alcr->hash[tmp_index].entry == NULL) {
				ma_error("Allocate idle hash table error.\n");
				goto mmalcr_create_error_return;
			}
			memset(alcr->hash[tmp_index].entry, 0x00, sizeof(mmalcr_hash_entry_t) * avg_cnt);
			alcr->hash[tmp_index].avg_cnt = avg_cnt;
			alcr->hash[tmp_index].avg_interval = avg_interval;
		}
	}

	/* TODO: Create and initialize tree structure. */
	{
		alcr->nil_node.color = RBT_COLOR_BLACK;
	}

	/* Set idle hash table the first block. */
	insert_node_into_hash(alcr, alcr->list);

	alcr->initialized = MMALCR_INITIALIZED;
	return (mmalcr_t)alcr;

mmalcr_create_error_return:
	mmalcr_release((mmalcr_t)alcr);
	return NULL;
}

/*
 * FUNCTION
 * mmalcr_release()
 *
 * Destroy/Relase a allocator.
 */
mmalcr_ret_t mmalcr_release(mmalcr_t mmalcr)
{
	mmalcr_priv_t *alcr = (mmalcr_priv_t *)mmalcr;
	mmalcr_node_t *tmp = NULL;
	mmalcr_node_t *cur = NULL;
	int32_t i = 0;

	if (ma_unlikely(alcr == NULL)) {
		ma_error("Argument error.\n");
		return MMACLR_RET_EINVAL;
	}

	/* Release main memory block list. */
	cur = alcr->list;
	while (cur != NULL) {
		tmp = cur;
		cur = cur->list_next;

		ma_free(tmp);
	}

	/* TODO: Release tree. */

	/* Release hash tables, free all hash table entries. */
	for (i = 0; i < sizeof(alcr->hash) / sizeof(alcr->hash[0]); i++) {
		if (alcr->hash[i].entry != NULL) {
			ma_free(alcr->hash[i].entry);
		}
	}

	alcr->initialized = 0;
	ma_free(alcr);
	return MMALCR_RET_OK;
}

/*
 * FUNCTION
 * mmalcr_get_config()
 *
 * Get all mmaclr configurations.
 */
mmalcr_ret_t mmalcr_get_config(mmalcr_t mmalcr, mmalcr_config_t *cfg)
{
	mmalcr_priv_t *alcr = (mmalcr_priv_t *)mmalcr;

	if (ma_unlikely(alcr == NULL)) {
		ma_error("*mmalcr* is NULL.\n");
		return MMACLR_RET_EINVAL;
	} else if (ma_unlikely(cfg == NULL)) {
		ma_error("*cfg* is NULL.\n");
		return MMACLR_RET_EINVAL;
	}

	/* TODO */

	return MMALCR_RET_OK;
}

/*
 * FUNCTION
 * mmalcr_set_config()
 *
 * Set mmalcr configuration by asigned config flags.
 */
mmalcr_ret_t mmalcr_set_config(mmalcr_t mmalcr, mmalcr_config_t *cfg,
		mmalcr_config_flag_t flags)
{
	mmalcr_priv_t *alcr = (mmalcr_priv_t *)mmalcr;

	if (ma_unlikely(alcr == NULL)) {
		ma_error("*mmalcr* is NULL.\n");
		return MMACLR_RET_EINVAL;
	} else if (ma_unlikely(cfg == NULL)) {
		ma_error("*cfg* is NULL.\n");
		return MMACLR_RET_EINVAL;
	}

	/* TODO */

	return MMALCR_RET_OK;
}

/*
 * FUNCTION
 * mmalcr_alloc()
 *
 * Allocate function API of mmalcr.
 */
uint32_t mmalcr_alloc(mmalcr_t mmalcr, uint32_t size,
		uint32_t align_order)
{
	mmalcr_priv_t *alcr = (mmalcr_priv_t *)mmalcr;
	uint32_t addr = MMALCR_ERROR_ADDRESS;

	if (ma_unlikely(alcr == NULL)) {
		ma_error("Allocator instance pointer is NULL.\n");
		return addr;
	} else if (ma_unlikely(alcr->initialized != MMALCR_INITIALIZED)) {
		ma_error("Allocator instance not initialized.\n");
		return addr;
	} else if (ma_unlikely(alcr->size < size)) {
		ma_error("Required size invalid.\n");
		return addr;
	} else if (ma_unlikely((align_order < alcr->align_order) ||
			(align_order > MMALCR_MAX_ALIGN_ORDER))) {
		ma_error("Alignment invalid.\n");
		return addr;
	}

	/* TODO */

	return addr;
}

/*
 * FUNCTION
 * mmalcr_free()
 *
 * Free function API of mmalcr.
 */
mmalcr_ret_t mmalcr_free(mmalcr_t mmalcr, uint32_t addr)
{
	mmalcr_priv_t *alcr = (mmalcr_priv_t *)mmalcr;
	mmalcr_ret_t ret = 0;

	if (ma_unlikely(alcr == NULL)) {
		ma_error("Instance to be released is NULL.\n");
		return MMACLR_RET_EINVAL;
	} else if (ma_unlikely(alcr->initialized != MMALCR_INITIALIZED)) {
		ma_error("Allocator instance not initialized.\n");
		return MMACLR_RET_EINVAL;
	}

	/* TODO */

	return ret;
}

/* ########################################################################## */
/* 
 * HASH TABLE OPERATIONS. 
 * I design the hash table to be 2 level confirmed, first level indexed by the 
 * 2E[order] *order*, while the second level is indexed by size average interval.
 * This method can't accelarate the alloc/free time cost from O(n) to O(log(n)).
 * But when huge amount of small memory slashes allocated, the new alloc/free
 * will save about 10 to 100 times time cost at worst cases.
 */

/*
 * FUNCTION
 * get_node_from_hash()
 *
 * Get a node from hash table by asigned size.
 */
static mmalcr_node_t *get_node_from_hash(mmalcr_priv_t *alcr, \
		uint32_t size, uint32_t delete)
{
	mmalcr_node_t *node = NULL;
	uint32_t upper_order = 0;
	uint32_t order_index = 0;
	uint32_t avg_index = 0;

	/* FIXME: size align step has been done at start of allocation function.
	 * size = (size + (alcr->align_order << 1) - 1) & ~((alcr->align_order << 1) - 1);
	 */
	if (ma_unlikely(size > alcr->size)) {
		ma_error("*size* is too large.\n");
		return NULL;
	}

	/* Calculate the location of inserted node should be. */
	{
		uint32_t tmp_size = size;
		uint32_t tmp_left = 0;

		/* Get upper order. */
		upper_order = 0;
		while (tmp_size > 0) {
			tmp_size >>= 1;
			upper_order++;
		}
		if (size == (1 << (upper_order - 1))) {
			upper_order--;
		}
		tmp_left = size - (1 << (upper_order - 1));
		order_index = upper_order - MMALCR_MIN_ALIGN_ORDER;
		avg_index = (tmp_left - 1) / alcr->hash[order_index].avg_interval;
		node = alcr->hash[order_index].entry[avg_index].list;
		ma_debug("%d, %d, %d, %d.\n", tmp_left, node->upper_order, node->order_index, node->avg_index);
	}

	/* Find a memory block and divided it into fit size. */
	while (node != NULL) {
		/* TODO */
	}


	if (ma_unlikely(node == NULL)) {
		ma_error("No more memory.\n");
		return NULL;
	}

	/* TODO: Divide memory node into fit size one and left to be another node. */

	return node;
}

/*
 * FUNCTION
 * insert_node_into_hash()
 *
 * Insert a memory block node into hash table.
 */
static int32_t insert_node_into_hash(mmalcr_priv_t *alcr, mmalcr_node_t *node)
{
	if (ma_unlikely(alcr->size < node->size)) {
		ma_error("Size of memory node invalid.\n");
		return -EINVAL;
	}

	/* Calculate the location of inserted node should be. */
	{
		uint32_t tmp_size = node->size;
		uint32_t tmp_left = 0;

		/* Get upper order. */
		node->upper_order = 0;
		while (tmp_size > 0) {
			tmp_size >>= 1;
			node->upper_order++;
		}
		if (node->size == (1 << (node->upper_order - 1))) {
			node->upper_order--;
		}
		tmp_left = node->size - (1 << (node->upper_order - 1));
		node->order_index = node->upper_order - MMALCR_MIN_ALIGN_ORDER;
		node->avg_index = (tmp_left - 1) / alcr->hash[node->order_index].avg_interval;
		ma_debug("%d, %d, %d, %d.\n", tmp_left, node->upper_order, node->order_index, node->avg_index);
	}
	/* Insert memory block node into hash table. */
	{
		mmalcr_node_t *tmp_list = alcr->hash[node->order_index].entry[node->avg_index].list;

		if (tmp_list != NULL) {
			ma_debug("%d, %d.\n", alcr->hash[node->order_index].avg_cnt, \
					alcr->hash[node->order_index].avg_interval);
			tmp_list->hash_pre = node;
			node->hash_next = tmp_list;
			node->hash_pre = NULL;
			alcr->hash[node->order_index].entry[node->avg_index].list = node;
		} else {
			node->hash_pre = NULL;
			node->hash_next = NULL;
			alcr->hash[node->order_index].entry[node->avg_index].list = node;
		}
	}

	return 0;
}

/*
 * FUNCTION
 * delete_node_from_hash()
 *
 * Delete a node from hash table, the node to be deleted from a hash table is 
 * supposed to be in allocator hash table and the location information has 
 * already been calculated and stored in *node*. So I don't calculate again
 * to get the location of current node in hash table. Check of node informatoin
 * is necessary.
 */
static int32_t delete_node_from_hash(mmalcr_priv_t *alcr, mmalcr_node_t *node)
{
	/* Check inserted node elements validation. */
	if (ma_unlikely(node == NULL)) {
		ma_error("*node* is NULL.\n");
		return -EINVAL;
	} else if (ma_unlikely((alcr->size < node->size) || (alcr->upper_order < node->upper_order))) {
		ma_error("*node* invalid.\n");
		return -EINVAL;
	}

	if (ma_likely(alcr->hash[node->order_index].entry[node->avg_index].list != NULL)) {
		mmalcr_node_t *tmp_node = alcr->hash[node->order_index].entry[node->avg_index].list;
		while ((tmp_node != NULL) && (tmp_node != node)) {
			tmp_node = tmp_node->hash_next;
		}
		if (ma_unlikely(tmp_node == NULL)) {
			ma_error("No such node in hash table.\n");
			return -EINVAL;
		}

		if (node->hash_pre != NULL) {
			node->hash_pre->hash_next = node->hash_next;
		}
		if (node->hash_next != NULL) {
			node->hash_next->hash_pre = node->hash_pre;
		}
		if ((node->hash_pre == NULL) && (node->hash_next == NULL)) {
			alcr->hash[node->order_index].entry[node->avg_index].list = NULL;
		}
	} else {
		ma_error("Hash node to be deleted does not exist.\n");
		return -EINVAL;
	}

	return 0;
}
