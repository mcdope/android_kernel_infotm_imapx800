/*******************************************************************************
 * mmalcr_private.h
 * 
 * Copyright (C) 2011 Sololz<sololz.luo@gmail.com>.
 *
 * Head file of allocator.
 *
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.0  2011/01/23 Sololz
 * 	Create this file.
 ******************************************************************************/

#if !defined(__MMALCR_PRIVATE_H__)
#define __MMALCR_PRIVATE_H__

#include "mmalcr.h"
#if defined(__KERNEL__)
#include "../memalloc.h"
#define ma_error	memalloc_error
#define ma_info		memalloc_info
#define ma_debug	memalloc_debug
#define ma_malloc(size) kmalloc(size, GFP_ATOMIC)
#define ma_free		kfree
#define ma_likely	likely
#define ma_unlikely	unlikely
#else	/* __KERNEL__ */
#define ma_error(error, ...)	do { printf("[ERROR] \"%s\" %s() %d : " error, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)
#define ma_info(info, ...)	do { printf("[INFO] : " info, ##__VA_ARGS__); } while (0)
#define ma_debug(debug, ...)	do { printf("[DEBUG] %d : " debug, __LINE__, ##__VA_ARGS__); } while (0)
#define ma_malloc	malloc
#define ma_free		free
/*
#define ma_likely	__builtin_expect(!!(x), 1)
#define ma_unlikely	__builtin_expect(!!(x), 0)
*/
#define ma_likely
#define ma_unlikely
#endif	/* __KERNEL__ */
#define ma_null		do {} while (0)

/* ############################################################################## */

#define MMALCR_INITIALIZED	(0x2b2b2b2b)

#define MMALCR_MAX_DIVIDE_ORDER	(6)
#define MMALCR_MAX_DIVIDE	(1 << MMALCR_MAX_DIVIDE_ORDER)

/* ############################################################################## */

/* Actual memory block node structure. */
typedef struct mmalcr_node_t {
	/* List control. */
	struct mmalcr_node_t *list_pre;
	struct mmalcr_node_t *list_next;
#define MEM_NODE_USED	(1)
#define MEM_NODE_IDLE	(0)
	uint32_t used;

	/* Hash control. */
	struct mmalcr_node_t *hash_pre;
	struct mmalcr_node_t *hash_next;
	uint32_t upper_order;
	uint32_t order_index;
	uint32_t avg_index;

	/* TODO: Tree control. */
	struct mmalcr_node_t *tree_pre;
	struct mmalcr_node_t *tree_next;
#define RBT_COLOR_BLACK	(0)
#define RBT_COLOR_RED	(1)
	uint32_t color;

	uint32_t addr;
	uint32_t size;
	uint32_t align_order;

	/* ATTENTION: Don't add any members before this line in current structure. */
} mmalcr_node_t;

/* Hash table entry structure. */
typedef struct mmalcr_hash_entry_t {
	struct mmalcr_hash_entry_t *entry;
	mmalcr_node_t *list;

	/* *avg_cnt* is only used in first level hash table as a element of 
	 * calculating memory block node location. */
	uint32_t avg_cnt;
	uint32_t avg_interval;
} mmalcr_hash_entry_t;

/* An allocator instance/object. */
typedef struct {
	mmalcr_node_t *list;

	mmalcr_node_t *tree;
	mmalcr_node_t nil_node;

	mmalcr_hash_entry_t hash[MMALCR_MAX_PROCESS_SIZE_ORDER - \
		MMALCR_MIN_ALIGN_ORDER + 1];

	uint32_t mm_start;
	void *virt_start;
	uint32_t size;
	uint32_t align_order;
	uint32_t upper_order;

	mmalcr_config_t cfg;

	int32_t initialized;
} mmalcr_priv_t;

/* ############################################################################## */
/* INTERNAL STATIC FUNCTION DECLARATIONS. */

static int32_t insert_node_into_hash(mmalcr_priv_t *alcr, mmalcr_node_t *node);
static int32_t delete_node_from_hash(mmalcr_priv_t *alcr, mmalcr_node_t *node);
static mmalcr_node_t *get_node_from_hash(mmalcr_priv_t *alcr, \
				uint32_t size, uint32_t delete);

#endif	/* __MMALCR_PRIVATE_H__ */
