/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.1.0	leo@2012/03/17: first commit.
** v1.2.1	leo@2012/03/30: Add kRemapVirAddr, but no use.
** v1.2.2	leo@2012/04/13: modify pmmpwl_pagemem_t, assume one-page in non-linear
**			mode, so the blockNum is totalPageNum.
** v1.2.3	leo@2012/04/21: add resvmem in pmmpwl_pagemem_t to support reserved memory
**			allocation.
** v1.2.4	leo@2012/05/02: add pmmpwl_flush_buffer().
**		    leo@2012/05/16: add isCache parameter to pmmpwl_alloc_page_memory().
** v1.2.8   leo@2012/08/22: add pgNum in pmmpwl_memblock_t to support more pages in one block.
** v1.2.9   leo@2012/09/10: add bigmem support.
** v1.2.12	leo@2012/11/30: rewrite bigmem.
*****************************************************************************/ 

#ifndef __PMM_PWL_H__
#define __PMM_PWL_H__


//
typedef struct{
    IM_BOOL     idle;
    IM_INT32    pgbase;
    IM_INT32    pgnum;
}bigmem_block_t;

// bigmem allocated from reserved memory.
typedef struct{
    IM_INT32            pageNum;	// page number in one block.
	IM_INT32			blockNum;
    IM_INT32            idleBlockNum;
    IM_BOOL             sysmem;
	IM_BOOL				isCache;
    im_mempool_handle_t mpl;
    im_list_handle_t	bmblst; // list of bigmem_block_t.
}bigmem_t;


#if (TARGET_SYSTEM == KERNEL_LINUX)
typedef struct{
	struct page	*pg;
	IM_INT32	pgNum;
	IM_UINT32	phyAddr;
	IM_BOOL		isCache;
}pmmpwl_memblock_t;

typedef struct{
	IM_BOOL			mapped;	// mapped form user virtual addr, else allocated by kernel.
	IM_BOOL			linear;	// if true, blockNum is 1, then blockNum > 1.
	IM_BOOL			isCache;
	IM_BOOL			resvmem; // if true, use reserved memory to allocated.
	IM_INT32		totalPageNum;

	// used in linear memory.
	void *			kVirAddr;
	IM_UINT32		phyAddr;

	// used in non-linear memory.
	IM_INT32		blockNum;
	pmmpwl_memblock_t	*blocks;
}pmmpwl_pagemem_t;
#endif

typedef void * bigmem_handle_t;


IM_RET pmmpwl_init(void);
IM_RET pmmpwl_deinit(void);

void *pmmpwl_malloc(IM_INT32 size);
void pmmpwl_free(void *p);
void *pmmpwl_memset(void *p, IM_CHAR c, IM_INT32 size);
void *pmmpwl_memcpy(void *dst, void *src, IM_INT32 size);
IM_TCHAR *pmmpwl_strcpy(IM_TCHAR *dst, IM_TCHAR *src);
IM_INT32 pmmpwl_strlen(IM_TCHAR *str);
IM_INT32 pmmpwl_strcmp(IM_TCHAR *dst, IM_TCHAR *src);

typedef void *	pmmpwl_lock_t;
IM_RET pmmpwl_lock_init(pmmpwl_lock_t *lck);
IM_RET pmmpwl_lock_deinit(pmmpwl_lock_t lck);
IM_RET pmmpwl_lock_lock(pmmpwl_lock_t lck);
IM_RET pmmpwl_lock_unlock(pmmpwl_lock_t lck);

/**
 * FUNC: This function allocate page-based memory.
 * PARAMS:
 * 	pageNum, requested page number.
 * 	linear, IM_TRUE--linear memory, IM_FALSE--non-linear memory.
 * 	isCache, cachable or noncachable.
 *	buffer
 *		vir_addr, user space virtual address.
 *		phy_addr, if linear is true, it's the physical address.
 *		size, the actual size allocated.
 *		flag, IM_BUFFER_FLAG_xxx.
 *	pglist, physical page array.
 * RETURN: privData, failed return IM_NULL.
 */
void *pmmpwl_alloc_page_memory(IN IM_INT32 pageNum, IN IM_BOOL linear, IN IM_BOOL isCache, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist);

/**
 * FUNC: This function map user-space virtual address to page-based memory.
 * PARAMS:
 * 	uVirAddr, user-space virtual address.
 * 	size, memory size.
 *	buffer
 *		vir_addr, user space virtual address.
 *		phy_addr, if linear is true, it's the physical address.
 *		size, the actual size allocated.
 *		flag, IM_BUFFER_FLAG_xxx.
 *	pglist, physical page array.
 * RETURN: privData, failed return IM_NULL.
 */
void *pmmpwl_map_useraddr_to_page_memory(IN void *uVirAddr, IN IM_INT32 size, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist, OUT IM_INT32 *actualPageNum);

/**
 * FUNC: This function free page memory which allocated previously.
 * PARAMS:
 * 	privData, kernal-space virtual address.
 * RETURN: no.
 */
void pmmpwl_free_page_memory(IN void *privData);

IM_RET pmmpwl_flush_buffer(IN void *privData, IN IM_INT32 flag);

bigmem_handle_t pmmpwl_init_bigmem(IN IM_INT32 pageNum, IN IM_INT32 blockNum, IN IM_BOOL isCache);
IM_RET pmmpwl_deinit_bigmem(IN bigmem_handle_t handle);
void *pmmpwl_alloc_bigmem_block(IN bigmem_handle_t handle, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist, IN IM_INT32 pglistLen);
IM_RET pmmpwl_free_bigmem_block(IN bigmem_handle_t handle, IN void *privData);

#endif	// __PMM_PWL_H__

