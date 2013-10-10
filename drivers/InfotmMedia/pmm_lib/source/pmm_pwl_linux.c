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
** v1.2.1	leo@2012/04/05: non-linear memory allocate has bug, this version not fixed.
** v1.2.2	leo@2012/04/13: use alloc_page() & __free_page() to do the non-linear memory
**				allocate, then use dma_map_page() & dma_unmap_page() to mapping&unmapping,
**				fixed the cpu and devmmu access non-cohrent problem.
**				remove the implementation of pmmpwl_map_useraddr_to_page_memory().
** v1.2.3	leo@2012/04/23: add reserved memory to alloc linear memory policy.
** v1.2.4	leo@2012/05/02: 
**				1. test pmmpwl_map_useraddr_to_page_memory() ok.
**				2. add pmmpwl_flush_buffer().
**		    leo@2012/05/12:
**				1. when no enough gResvMem->idleNum to alloc, then don't try reserved memory but 
**				directly use OS memory to allocate.
**				2. add put_page() after get_user_pages(), to fix out-of-memory bug. 
**	        leo@2012/05/14: when alloc linear memory, put the kVirAddr to IM_Buffer.vir_addr.
**	        leo@2012/05/15: when map buffer, don't modify the buffer.size.
**	        leo@2012/05/16: add ALC_FLAG_CACHE support.
** v1.2.5	leo@2012/06/12: read reserved memory from system.
** v1.2.6	leo@2012/07/02: fixed memory overflow in front of reserved memory(the pgbase).
** v1.2.8	leo@2012/08/16: if system has no memory to allocate, the reserved memory can help for this. only
**			RESVMEM_ALLOC_NONLINEAR_ENABLE is defined. slotIdleMBlst[] designed for per-slot to advance alloc speed.
**			CAUTION: here we still has 2 method to enhance performance:
**			1. use slotBusyMBlst[] to advance free speed.
**			2. use system and reserved memory combined mode for non-linear memory allocate.
** v1.2.9   leo@2012/09/10: 
**          1. add bigmem support, which is a linear reserved memory blcok dedicated for some application.
**          2. adjust resvMem block max to 32MB.
** v1.2.10  leo@2012/10/22: support memblock max to 64MB, and adjust the method of modify the max pgsize.
** v1.2.11	leo@2012/11/16: change PMM_RESERVE_MEM_THRESHOLD from 512K to 128K, to decrease the memory press of system,
**				meanwhile, if system memroy is low, directly allocate from reserved memory can accelerate speed for the
**				system would do garbage-collection.
** v1.2.12	leo@2012/11/30: 
**				1. optimize the algorithm of resvmem alloc when the slot has remained mem-block.
**					it can turn on or off through "RESVMEM_ALGORITHM_BETTER_SPLIT".
**				2. optimize resvMemFreeBlock when the slot of the block is -1.
**				3. rewrite bigmem.
**
*****************************************************************************/ 

#include <asm/page.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <mach/mem-reserve.h>

#include <InfotmMedia.h>
#include <IM_buffallocapi.h>
#include <pmm_lib.h>
#include <pmm_pwl.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"PMMPWL_I:"
#define WARNHEAD	"PMMPWL_W:"
#define ERRHEAD		"PMMPWL_E:"
#define TIPHEAD		"PMMPWL_T:"


//#define TRACE_RESV_MEMBLK
//#define TRACE_BIGMEM


//
// here can define the max pgsize of memblock.
//
#define MEMBLK_PGSIZE_MAX	16384    // 4096--16MB, 8192--32MB, 16384--64MB.

#if (MEMBLK_PGSIZE_MAX == 4096)
    #define MEMBLK_SLOT_NUM		13
#elif (MEMBLK_PGSIZE_MAX == 8192)
    #define MEMBLK_SLOT_NUM		14
#elif (MEMBLK_PGSIZE_MAX == 16384)
    #define MEMBLK_SLOT_NUM		15
#else
    #define MEMBLK_SLOT_NUM		13  // default pgsize max is 16MB.
#endif


//
//
//
#define UPALIGN_SIZE(size)	(((size) + (PMM_PAGE_SIZE - 1)) & ~(PMM_PAGE_SIZE - 1))
#define SIZE_TO_PAGE(size)	((size) >> PMM_PAGE_SHIFT)
#define PAGE_TO_SIZE(page)	((page) << PMM_PAGE_SHIFT)


//
// reserved memblock management.
// 
// 1. resv_mem_t
// 			pgbase: reserved memory的基地址，以下都是page为单位；
//			totalNum: 总数量；
//			idleNum: 总空闲的数量；
//			unit: 最小内存块的维护单位；
//			sysmem: true--通过系统方式获得的内存，false--自己保留的内存;
//			mblst: 总的内存链表;
//			slotIdleMBlst[]: slot内存空闲块链表，始终维持地址空间的连续；
// resv_pgblock_t
//	  		base: 该内存块的基地址；
//	  		idle: 空闲标志；
//	  		slot:
//	  		idle:
//
typedef struct{
	IM_UINT32	base;	// base of page.
	IM_INT32	num;	// page number.
	IM_INT32	slot;
	IM_BOOL		idle;
}resv_pgblock_t;

#define SLOT_TO_PAGENUM(slot)	(1 << slot)
#define SLOT_TO_SIZE(slot)	PAGE_TO_SIZE(SLOT_TO_PAGENUM(slot))
typedef struct{
	IM_INT32		pgbase;
	IM_INT32		totalNum;	// total page number.
	IM_INT32		idleNum;	// idle page number.
	IM_INT32		unit;		// page number of the smallest mem-block.
	IM_BOOL			sysmem;		// reserved from mem-reserve.c.
	im_klist_handle_t	mblst;		// list of all node of resv_pgblock_t.
	IM_BOOL			recycled;	// if free memory(linear or nonlinear) to reserved, recycled is false.

	// byte: 4K, 8K, 16K, 32K, 64K, 128K, 256K, 512K, 1M,  2M,  4M,  8M,  16M, 32M, 64M
	// page: 1,  2,  4,   8,   16,  32,   64,   128,  256, 512, 1024,2048,4096,8192,16384
	im_mempool_handle_t	mpl;
	im_klist_handle_t	slotIdleMBlst[MEMBLK_SLOT_NUM];
}resv_mem_t;

static const IM_INT32 SLOT_PAGENUM_TABLE[MEMBLK_SLOT_NUM] =
{
#if (MEMBLK_SLOT_NUM == 13)
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
#elif (MEMBLK_SLOT_NUM == 14) 
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
#elif (MEMBLK_SLOT_NUM == 15)
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384
#endif
};

inline static IM_INT32 PAGENUM_TO_SLOT(IM_INT32 pn){
	IM_INT32 i;
	for(i=0; i<MEMBLK_SLOT_NUM; i++){
		if(pn == SLOT_PAGENUM_TABLE[i]){
			return i;
		}
	}
	return -1;
}

// user can define these 2 macro in compiler(i.e, Makefile).
#ifndef PMM_RESERVE_MEM_SIZE
#define PMM_RESERVE_MEM_SIZE	0x0
#endif

#ifndef PMM_RESERVE_MEM_BASE
#define PMM_RESERVE_MEM_BASE	0x0
#endif

#ifndef PMM_RESERVE_MEM_THRESHOLD
#define PMM_RESERVE_MEM_THRESHOLD	0x20000	// 128K
#endif

#define RESVMEM_ALLOC_NONLINEAR_ENABLE 

#define RESVMEM_ALGORITHM_BETTER_SPLIT

#define RESVMEM_PAGES_UNIT	(32)	// 128K

#ifdef RESVMEM_ALLOC_NONLINEAR_ENABLE 
#undef RESVMEM_PAGES_UNIT
#define RESVMEM_PAGES_UNIT	(8)	// 32K
#endif

#define RESVMEM_UPALIGN(pgnum)	((pgnum + RESVMEM_PAGES_UNIT - 1) & ~(RESVMEM_PAGES_UNIT - 1))
#define RESVMEM_ALIGNED(pgnum)	((pgnum & (RESVMEM_PAGES_UNIT - 1) == 0) ? IM_TRUE : IM_FALSE)

static resv_mem_t *gResvMem = IM_NULL;

//
//
//
static IM_RET resvMemInit(void);
static void resvMemDeinit(void);
static void resvMemSlotMerge(IM_INT32 slot, IM_INT32 pgbase);
static IM_RET resvMemFreeLinear(IN pmmpwl_pagemem_t *pagemem);
static IM_RET bigmemFreeBlock(bigmem_t *bigmem, pmmpwl_pagemem_t *pagemem);
static void resvMemFreeNonlinear(IN pmmpwl_pagemem_t *pagemem);
static void systemFreeLinear(IN pmmpwl_pagemem_t *pagemem);
static void systemFreeNonlinear(IN pmmpwl_pagemem_t *pagemem);


//
//
//
IM_RET pmmpwl_init(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gResvMem == IM_NULL){
		if(resvMemInit() != IM_RET_OK){
			IM_ERRMSG((IM_STR("resvMemInit() failed")));
			return IM_RET_FAILED;
		}
	}	
	return IM_RET_OK;
}

IM_RET pmmpwl_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gResvMem != IM_NULL){
		resvMemDeinit();
	}
	return IM_RET_OK;
}

void *pmmpwl_malloc(IM_INT32 size)
{
	return kmalloc(size, GFP_KERNEL);
}

void pmmpwl_free(void *p)
{
	return kfree(p);
}

void *pmmpwl_memset(void *p, IM_CHAR c, IM_INT32 size)
{
	return memset(p, c, size);
}

void *pmmpwl_memcpy(void *dst, void *src, IM_INT32 size)
{
	return memcpy(dst, src, size);
}

IM_TCHAR *pmmpwl_strcpy(IM_TCHAR *dst, IM_TCHAR *src)
{
	return strcpy(dst, src);
}

IM_INT32 pmmpwl_strlen(IM_TCHAR *str)
{
	return strlen(str);
}

IM_INT32 pmmpwl_strcmp(IM_TCHAR *dst, IM_TCHAR *src)
{
	return strcmp(dst, src);
}

inline static void RESVMEM_LIST_TRACE(IM_BOOL isMblst, IM_BOOL add, IM_INT32 slot, IM_INT32 pgbase)
{
#ifdef TRACE_RESV_MEMBLK
    if(isMblst){
        if(add){
            IM_TIPMSG((IM_STR("mblst+ %d"), pgbase));
        }else{
            IM_TIPMSG((IM_STR("mblst- %d"), pgbase));
        }
    }else{
      if(add){
	        IM_TIPMSG((IM_STR("slot[%d]+ %d"), slot, pgbase));
        }else{
	        IM_TIPMSG((IM_STR("slot[%d]- %d"), slot, pgbase));
        }
    }
#endif
}

static void resvMemTrace(IM_BOOL detail)
{
	IM_INT32 i, k;
	resv_pgblock_t *pgblk;

	IM_TIPMSG((IM_STR("=========================================================")));

	IM_TIPMSG((IM_STR("resvmem: totalNum=%d(size=%dKB), idleNum=%d(size=%dKB)"), 
		gResvMem->totalNum, PAGE_TO_SIZE(gResvMem->totalNum) >> 10, gResvMem->idleNum, PAGE_TO_SIZE(gResvMem->idleNum) >> 10));
	for(i=MEMBLK_SLOT_NUM-1; i>=0; i--){
		if(detail){
			IM_TIPMSG((IM_STR("--------------------slot %d (size=%dKB)------------------------------"), i, SLOT_TO_SIZE(i) >> 10));
			IM_TIPMSG((IM_STR("index\tbase(addr)")));
			k = 0;
			pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->slotIdleMBlst[i], IM_NULL);
			while(pgblk != IM_NULL){
				IM_TIPMSG((IM_STR("%d\t%d(0x%8x)"), k++, pgblk->base, PAGE_TO_SIZE(pgblk->base)));
				IM_ASSERT(pgblk->idle == IM_TRUE);
				pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->slotIdleMBlst[i], IM_NULL);
			}
			IM_TIPMSG((IM_STR("---------------------------------------------------------")));
		}
	}

	if(detail){
		IM_TIPMSG((IM_STR("---------------------------------------------------------")));
		IM_TIPMSG((IM_STR("index\tbase(addr)\t\tpgnum(size)\t\tslot\tidle")));
		k = 0;
		pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->mblst, IM_NULL);
		while(pgblk != IM_NULL){
			IM_TIPMSG((IM_STR("%d\t%d(0x%8x)\t%d(0x%x)\t\t%d\t%d"), k++, pgblk->base, PAGE_TO_SIZE(pgblk->base), 
				pgblk->num, PAGE_TO_SIZE(pgblk->num), pgblk->slot, pgblk->idle));
			pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
		}
		IM_TIPMSG((IM_STR("---------------------------------------------------------")));
	}
	IM_TIPMSG((IM_STR("=========================================================")));
}

static IM_RET resvMemFeedSlot(IM_INT32 pgbase, IM_INT32 pgnum)
{
	IM_INT32 i, j, k;
	resv_pgblock_t *pgblk;
	IM_INFOMSG((IM_STR("%s(pgbase=%d, pgnum=%d)"), IM_STR(_IM_FUNC_), pgbase, pgnum));

	for(i=MEMBLK_SLOT_NUM-1; pgnum > 0; i--){
		IM_ASSERT(i >= 0);
		k = pgnum >> i;	
		if(k == 0){
			continue;
		}
		// put in this slot, "k" is the number of pgblk in this slot. 
		for(j=0; j<k; j++){
			pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
			if(pgblk == IM_NULL){//in this case, the system is muddleheaded.
				IM_ERRMSG((IM_STR("kmalloc() failed")));
				return IM_RET_FAILED;
			}

			pgblk->base = pgbase;
			pgblk->num = SLOT_TO_PAGENUM(i);
			pgblk->slot = i;
			pgblk->idle = IM_TRUE;
            RESVMEM_LIST_TRACE(IM_FALSE, IM_TRUE, i, pgblk->base);
			im_klist_put(gResvMem->slotIdleMBlst[i], pgblk->base, (void *)pgblk);
            RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, pgblk->base);
			im_klist_put(gResvMem->mblst, pgblk->base, (void *)pgblk);
			
			if(im_klist_size(gResvMem->slotIdleMBlst[i]) > 1){
				resvMemSlotMerge(i, pgblk->base);
			}

			pgbase += SLOT_TO_PAGENUM(i);
			pgnum -= SLOT_TO_PAGENUM(i);
			IM_INFOMSG((IM_STR("pgbase=%d, pgnum=%d"), pgbase, pgnum));
		}
	}

	return IM_RET_OK;
}

static void resvMemSlotMerge(IM_INT32 slot, IM_INT32 pgbase)
{
	resv_pgblock_t *pgblk, *pre_pgblk=IM_NULL, *post_pgblk=IM_NULL;
	IM_INFOMSG((IM_STR("%s(slot=%d, pgbase=%d)"), IM_STR(_IM_FUNC_), slot, pgbase));

	if(slot >= MEMBLK_SLOT_NUM - 1){ // the largest blocks don't allow merge.
		IM_INFOMSG((IM_STR("the largest block slot(%d) don't allow merge"), slot));
		return;
	}

	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->slotIdleMBlst[slot], pgbase);
	IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
	IM_ASSERT((pgblk != IM_NULL) && (pgblk->idle == IM_TRUE));

	// first try to connect to previous, if cannot, then try to connect to post. CAUTION, prev and post don't 
	// all connect, casue it has no really effect, in resvMemFeedSlot, it would be split again.
	pre_pgblk = (resv_pgblock_t *)im_klist_prev(gResvMem->slotIdleMBlst[slot], IM_NULL);
	if((pre_pgblk != IM_NULL) && (pre_pgblk->base + pre_pgblk->num == pgblk->base)){
		IM_INFOMSG((IM_STR("pre_pgblk: base=%d, num=%d, slot=%d, idle=%d"), 
			pre_pgblk->base, pre_pgblk->num, pre_pgblk->slot, pre_pgblk->idle));
		IM_ASSERT(pre_pgblk->idle == IM_TRUE);
	}else{
		if(pre_pgblk != IM_NULL){
			im_klist_next(gResvMem->slotIdleMBlst[slot], IM_NULL);// goto pgblk and for then find the post.
			pre_pgblk = IM_NULL;// ignore preblk.
		}
		post_pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->slotIdleMBlst[slot], IM_NULL);
		if((post_pgblk != IM_NULL) && (pgblk->base + pgblk->num == post_pgblk->base)){
			IM_INFOMSG((IM_STR("post_pgblk: base=%d, num=%d, slot=%d, idle=%d"), 
				post_pgblk->base, post_pgblk->num, post_pgblk->slot, post_pgblk->idle));
			IM_ASSERT(post_pgblk->idle == IM_TRUE);
		}else{
			post_pgblk = IM_NULL;// ignore postblk.
		}
	}

	//
	if(pre_pgblk != IM_NULL){
		// 1. remove the 2 idleMB from the old slot.
		im_klist_erase(gResvMem->slotIdleMBlst[slot], pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, pgblk->base);
		pgblk->idle = IM_FALSE;
		im_klist_erase(gResvMem->slotIdleMBlst[slot], pre_pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, pre_pgblk->base);
		pre_pgblk->idle = IM_FALSE;

		// 2. the 2 idleMB will merge to 1 new MB, so remove 1 MB from mblst.
		im_klist_erase(gResvMem->mblst, pgblk->base);
        RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, pgblk->base);
        //IM_TIPMSG((IM_STR("%d:mblst- %d:%d"), _IM_LINE_, pgblk->base, pgblk->num));
		kfree((void *)pgblk);
	
		// 3. modify the new MB info in mblst.
		pre_pgblk->idle = IM_TRUE;
		pre_pgblk->num <<= 1;
		pre_pgblk->slot += 1; // merge to up-level slot.
        //IM_TIPMSG((IM_STR("%d:mblst* %d:%d"), _IM_LINE_, pre_pgblk->base, pre_pgblk->num));
		im_klist_put(gResvMem->slotIdleMBlst[pre_pgblk->slot], pre_pgblk->base, (void *)pre_pgblk);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_TRUE, pre_pgblk->slot, pre_pgblk->base);

		IM_INFOMSG((IM_STR("merge with pre_pgblk to a new MB for slot[%d]"), pre_pgblk->slot));

		// recursion slot merge.
		if(im_klist_size(gResvMem->slotIdleMBlst[pre_pgblk->slot]) > 1){
			resvMemSlotMerge(pre_pgblk->slot, pre_pgblk->base);
		}
	}
	else if(post_pgblk != IM_NULL){
		// 1. remove the 2 idleMB from the old slot.
		im_klist_erase(gResvMem->slotIdleMBlst[slot], pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, pgblk->base);
		pgblk->idle = IM_FALSE;
		im_klist_erase(gResvMem->slotIdleMBlst[slot], post_pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, post_pgblk->base);
		post_pgblk->idle = IM_FALSE;

		// 2. the 2 idleMB will merge to 1 new MB, so remove 1 MB form mblst.
		im_klist_erase(gResvMem->mblst, post_pgblk->base);
        RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, post_pgblk->base);
        //IM_TIPMSG((IM_STR("%d:mblst- %d:%d"), _IM_LINE_, post_pgblk->base, post_pgblk->num));
		kfree((void *)post_pgblk);

		// 3. modify the new MB info in mblst.
		pgblk->idle = IM_TRUE;
		pgblk->num <<= 1;
		pgblk->slot += 1; // merge to up-level slot.
        //IM_TIPMSG((IM_STR("%d:mblst* %d:%d"), _IM_LINE_, pgblk->base, pgblk->num));
		im_klist_put(gResvMem->slotIdleMBlst[pgblk->slot], pgblk->base, (void *)pgblk);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_TRUE, pgblk->slot, pgblk->base);
		
		IM_INFOMSG((IM_STR("merge with post_pgblk to a new MB for slot[%d]"), pgblk->slot));
		
		// recursion slot merge.
		if(im_klist_size(gResvMem->slotIdleMBlst[pgblk->slot]) > 1){
			resvMemSlotMerge(pgblk->slot, pgblk->base);
		}
	}
}

static IM_RET resvMemBlockMerge(IM_INT32 pgbase, IM_INT32 pgnum)
{
	IM_INT32 base, num, key;
	resv_pgblock_t *pgblk, *pre_pgblk, *post_pgblk;
	IM_INFOMSG((IM_STR("%s(pgbase=%d, pgnum=%d)"), IM_STR(_IM_FUNC_), pgbase, pgnum));

	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, pgbase);
	IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
	IM_ASSERT(pgblk != IM_NULL);
	IM_ASSERT(pgblk->num == pgnum);
	IM_ASSERT(pgblk->base == pgbase);
	IM_ASSERT(pgblk->idle == IM_TRUE);	// caller must put this block to idle in mblst first.

	// merge backward.
	base = pgblk->base;
	num = pgblk->num;
	pre_pgblk = im_klist_prev(gResvMem->mblst, IM_NULL);	// search backward to the first idle block.
	while((pre_pgblk != IM_NULL) && (pre_pgblk->idle == IM_TRUE)){
		IM_INFOMSG((IM_STR("pre_pgblk: base=%d, num=%d, slot=%d, idle=%d"), pre_pgblk->base, pre_pgblk->num, pre_pgblk->slot, pre_pgblk->idle));

		base = pre_pgblk->base;
		num += pre_pgblk->num;

		// 1. remove from slotIdleMBlst.
		IM_ASSERT(pre_pgblk->slot != -1);
		im_klist_erase(gResvMem->slotIdleMBlst[pre_pgblk->slot], pre_pgblk->base);
		pre_pgblk->idle = IM_FALSE;
		RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, pre_pgblk->slot, pre_pgblk->base);

		// 2. remove from mblst.
		im_klist_erase(gResvMem->mblst, pre_pgblk->base); // mblst pointer will auto goto pgblk.
        RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, pre_pgblk->base);

		// 3. free this pgblk.
		kfree((void *)pre_pgblk);

		pre_pgblk = im_klist_prev(gResvMem->mblst, IM_NULL);
	}

	// merge forward.
	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, pgbase); // go to the pgblk for followed merge post_gpblk.
	post_pgblk = im_klist_next(gResvMem->mblst, IM_NULL);
	while((post_pgblk != IM_NULL) && (post_pgblk->idle == IM_TRUE)){ // search forward until the firt non-idle block.
		IM_INFOMSG((IM_STR("post_pgblk: base=%d, num=%d, slot=%d, idle=%d"), post_pgblk->base, post_pgblk->num, post_pgblk->slot, post_pgblk->idle));

		num += post_pgblk->num;

		// 1. remove from slotIdleMBlst.
		IM_ASSERT(post_pgblk->slot != -1);
		im_klist_erase(gResvMem->slotIdleMBlst[post_pgblk->slot], post_pgblk->base);
		post_pgblk->idle = IM_FALSE;
		RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, post_pgblk->slot, post_pgblk->base);

		// 2. remove from mblst and free this pgblk.
		key = post_pgblk->base;
		kfree((void *)post_pgblk);
		post_pgblk = im_klist_erase(gResvMem->mblst, key); // mblst pointer will auto to next.
        RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, key);
	}

	// itself.
	im_klist_erase(gResvMem->mblst, pgblk->base);
	RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, pgblk->base);
	kfree((void *)pgblk);

	//
	IM_INFOMSG((IM_STR("base=%d, num=%d"), base, num));
	if(resvMemFeedSlot(base, num) != IM_RET_OK){ // if this happen, this library is confused.
		IM_ERRMSG((IM_STR("resvMemFeedSlot() failed")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

static IM_INT32 resvMemAllocBigMemBlock(IM_INT32 pageNum)
{
    IM_INT32 pgbase, pgnum, pgleft;
    IM_INT32 dock_pgbase = -1, dock_pgnum;
	resv_pgblock_t *pgblk, *post_pgblk, *new_pgblk;
	IM_INT32 pn = RESVMEM_UPALIGN(pageNum);
	IM_INFOMSG((IM_STR("%s(pageNum=%d, pn=%d)"), IM_STR(_IM_FUNC_), pageNum, pn));

    if(gResvMem->idleNum < pn){
        IM_ERRMSG((IM_STR("no reserved memory for bigmem block, gResvMem->idleNum=%d, pn=%d"), gResvMem->idleNum, pn));
        return -1;
    }

    //
 	pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->mblst, IM_NULL);
	while(pgblk != IM_NULL)
	{
		if(pgblk->idle != IM_TRUE){ // search the first idle pgblk.
			pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
			continue;
		}

		IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
		IM_ASSERT(pgblk->slot != -1);

		pgbase = pgblk->base;
		pgnum = pgblk->num;
		post_pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
		while((post_pgblk != IM_NULL) && (post_pgblk->idle == IM_TRUE)){
			IM_INFOMSG((IM_STR("post_pgblk: base=%d, num=%d, slot=%d, idle=%d"), 
				post_pgblk->base, post_pgblk->num, post_pgblk->slot, post_pgblk->idle));
			IM_ASSERT(pgbase + pgnum == post_pgblk->base);
			IM_ASSERT(post_pgblk->slot != -1);
			pgnum += post_pgblk->num;
			post_pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
		}

        // if no found a idle bigblock or found a best idle bigblock, dock to it.
        if(pgnum >= pn){
            if((dock_pgbase == -1) || (dock_pgnum > pgnum)){
                dock_pgbase = pgbase;
                dock_pgnum = pgnum;
                if(dock_pgnum == pn){
                    break; // cannot found a better pgnum.
                }
            }
        }

        pgblk = post_pgblk;
    }

    if(dock_pgbase == -1){
        IM_ERRMSG((IM_STR("not found any idle bigblock to alloc bigmem block(pn=%d)"), pn));
        return -1;
    }

    //
    IM_INFOMSG((IM_STR("found a bigblock(pgbase=%d, pgnum=%d) to alloc bigmem block"), dock_pgbase, dock_pgnum));
    dock_pgnum = 0;
  	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, dock_pgbase);
	while((pgblk != IM_NULL) && (dock_pgnum < pn))
	{
		IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
        IM_ASSERT(pgblk->idle == IM_TRUE);
		IM_ASSERT(pgblk->slot != -1);
        dock_pgnum += pgblk->num;
        im_klist_erase(gResvMem->slotIdleMBlst[pgblk->slot], pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, pgblk->slot, pgblk->base);
        pgbase = pgblk->base;
        kfree((void *)pgblk);
        pgblk = (resv_pgblock_t *)im_klist_erase(gResvMem->mblst, pgbase);
        RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, pgbase);
	}

	//
    pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
    IM_ASSERT(pgblk != IM_NULL); //in this case, the system is muddleheaded.
    pgblk->base = dock_pgbase;
    pgblk->num = pn;
    pgblk->slot = PAGENUM_TO_SLOT(pn);
    pgblk->idle = IM_FALSE;
    RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, pgblk->base);
    im_klist_put(gResvMem->mblst, pgblk->base, (void *)pgblk);

	gResvMem->idleNum -= pn;

    //
    pgleft = dock_pgnum - pn;
    if(pgleft != 0){
        pgbase = dock_pgbase + pn;
		new_pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
		IM_ASSERT(new_pgblk != IM_NULL); // in this case, the system is muddleheaded.
		new_pgblk->base = pgbase;
		new_pgblk->num = pgleft;
		new_pgblk->slot = PAGENUM_TO_SLOT(new_pgblk->num);
		new_pgblk->idle = IM_TRUE;
		RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, new_pgblk->base);
		im_klist_put(gResvMem->mblst, new_pgblk->base, (void *)new_pgblk);

		IM_ASSERT(resvMemBlockMerge(new_pgblk->base, new_pgblk->num) == IM_RET_OK);
    }

#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
    return dock_pgbase;
}

static IM_RET resvMemFreeBigMemBlock(IM_INT32 pgbase)
{
	IM_INT32 pn;
	resv_pgblock_t *pgblk;
	IM_INFOMSG((IM_STR("%s(pgbase=%d)"), IM_STR(_IM_FUNC_), pgbase));

  	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, pgbase);
    if(pgblk == IM_NULL){
        IM_ERRMSG((IM_STR("incorrect pgbase(%d), mblst not found"), pgbase));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->idle));
    IM_ASSERT(pgblk->idle == IM_FALSE);

	pn = pgblk->num;
	pgblk->idle = IM_TRUE;
	if(resvMemBlockMerge(pgblk->base, pgblk->num) != IM_RET_OK){
		IM_ERRMSG((IM_STR("resvMemBlockMerge(pgbase=%d, pgnum=%d) failed"), pgblk->base, pgblk->num));
		return IM_RET_FAILED;
	}

	//
	gResvMem->idleNum += pn; // pn must aligned to unit.

#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
    return IM_RET_OK;
}

static IM_RET resvMemInit(void)
{
	IM_INT32 i;
	dma_addr_t paddr;
	size_t size;
	IM_INFOMSG((IM_STR("%s(PMM_RESERVE_MEM_BASE=0x%x, PMM_RESERVE_MEM_SIZE=0x%x, PMM_RESERVE_MEM_THRESHOLD=0x%x)"), 
		IM_STR(_IM_FUNC_), PMM_RESERVE_MEM_BASE, PMM_RESERVE_MEM_SIZE, PMM_RESERVE_MEM_THRESHOLD));
	IM_ASSERT((PMM_RESERVE_MEM_BASE & (PMM_PAGE_SIZE - 1)) == 0);
	IM_ASSERT((PMM_RESERVE_MEM_SIZE & (PMM_PAGE_SIZE - 1)) == 0);
	IM_ASSERT((PMM_RESERVE_MEM_THRESHOLD & (PMM_PAGE_SIZE - 1)) == 0);

	gResvMem = (resv_mem_t *)kmalloc(sizeof(resv_mem_t), GFP_KERNEL);
	if(gResvMem == IM_NULL){
		IM_ERRMSG((IM_STR("kmalloc(gResvMem) failed")));
		return IM_RET_FAILED;
	}
	memset((void *)gResvMem, 0, sizeof(resv_mem_t));

	//
	if((PMM_RESERVE_MEM_BASE != 0) && (PMM_RESERVE_MEM_SIZE != 0)){
		gResvMem->pgbase = SIZE_TO_PAGE(PMM_RESERVE_MEM_BASE);	// down-align to pages.
		gResvMem->totalNum = SIZE_TO_PAGE(PMM_RESERVE_MEM_SIZE & ~(PMM_PAGE_SIZE - 1));
		gResvMem->sysmem = IM_FALSE;
	}else{
		paddr = imap_get_reservemem_paddr(RESERVEMEM_DEV_PMM);
		size = imap_get_reservemem_size(RESERVEMEM_DEV_PMM);
		gResvMem->pgbase = SIZE_TO_PAGE(paddr + 4095);	// align to next rounded page.
		gResvMem->totalNum = SIZE_TO_PAGE(size - (paddr & 4095));
		gResvMem->sysmem = IM_TRUE;
	}
	gResvMem->unit = RESVMEM_PAGES_UNIT;
    gResvMem->totalNum &= ~(RESVMEM_PAGES_UNIT - 1);
	IM_TIPMSG((IM_STR("RESVMEM: base=0x%x, size=0x%x, sysmem=%d, pgunit=%d"), 
		PAGE_TO_SIZE(gResvMem->pgbase), PAGE_TO_SIZE(gResvMem->totalNum), gResvMem->sysmem, gResvMem->unit));
#ifdef RESVMEM_ALLOC_NONLINEAR_ENABLE 
	IM_TIPMSG((IM_STR("RESVMEM_ALLOC_NONLINEAR_ENABLE = true")));
#else
	IM_TIPMSG((IM_STR("RESVMEM_ALLOC_NONLINEAR_ENABLE = false")));
#endif
	if((gResvMem->pgbase == 0) || (gResvMem->totalNum == 0)){
		IM_WARNMSG((IM_STR("no configure pmm reserve memory")));
		return IM_RET_OK;
	}

	//	
	gResvMem->idleNum = gResvMem->totalNum;	// the whole mem.
	gResvMem->recycled = IM_TRUE;

	//
	gResvMem->mpl = im_mpool_init((func_mempool_malloc_t)pmmpwl_malloc, (func_mempool_free_t)pmmpwl_free);
	if(gResvMem->mpl == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed")));
		goto Fail;
	}

	gResvMem->mblst = im_klist_init(0, gResvMem->mpl);
	if(gResvMem->mblst == IM_NULL){
		IM_ERRMSG((IM_STR("im_klist_init(mblst) failed")));
		goto Fail;
	}
	for(i=0; i<MEMBLK_SLOT_NUM; i++){
		gResvMem->slotIdleMBlst[i] = im_klist_init(0, gResvMem->mpl);
		if(gResvMem->slotIdleMBlst[i] == IM_NULL){
			IM_ERRMSG((IM_STR("im_klist_init(slotMBlst) failed")));
			goto Fail;
		}
	}

	// put the whole memory to per-slot from bigger to smaller.
	if(resvMemFeedSlot(gResvMem->pgbase, gResvMem->totalNum) != IM_RET_OK){
		IM_ERRMSG((IM_STR("resvMemFeedSlot() failed")));
		goto Fail;
	}

	//
#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
	return IM_RET_OK;
Fail:
	resvMemDeinit();
	return IM_RET_FAILED;
}

static void resvMemDeinit(void)
{
	IM_INT32 i;
	IM_UINT32 key;
	resv_pgblock_t *pgblk;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gResvMem->mblst != IM_NULL){
		pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->mblst, IM_NULL);
		while(pgblk != IM_NULL){
			key = pgblk->base;
			kfree((void *)pgblk);
			pgblk = (resv_pgblock_t *)im_klist_erase(gResvMem->mblst, key);
		}

		im_klist_deinit(gResvMem->mblst);
		gResvMem->mblst = IM_NULL;
	}
	for(i=0; i<MEMBLK_SLOT_NUM; i++){
		if(gResvMem->slotIdleMBlst[i] != IM_NULL){
			im_klist_deinit(gResvMem->slotIdleMBlst[i]);
			gResvMem->slotIdleMBlst[i] = IM_NULL;
		}
	}
	if(gResvMem->mpl != IM_NULL){
		im_mpool_deinit(gResvMem->mpl);
		gResvMem->mpl = IM_NULL;
	}
	kfree((void *)gResvMem);
	gResvMem = IM_NULL;
}

static IM_RET resvMemRecycle(void)
{
	IM_BOOL feed;
	IM_INT32 pgbase, pgnum;
	IM_UINT32 key;
	resv_pgblock_t *pgblk, *post_pgblk;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
    pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->mblst, IM_NULL);
	while(pgblk != IM_NULL)
	{
		if(pgblk->idle != IM_TRUE){ // search the first idle pgblk, then can do merge.
			pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
			continue;
		}

		IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
		IM_ASSERT(pgblk->slot != -1);

		feed = IM_FALSE;
		pgbase = pgblk->base;
		pgnum = pgblk->num;
		post_pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->mblst, IM_NULL);
		while((post_pgblk != IM_NULL) && (post_pgblk->idle == IM_TRUE)){
			IM_INFOMSG((IM_STR("post_pgblk: base=%d, num=%d, slot=%d, idle=%d"), 
				post_pgblk->base, post_pgblk->num, post_pgblk->slot, post_pgblk->idle));
			IM_ASSERT(pgbase + pgnum == post_pgblk->base);
			IM_ASSERT(post_pgblk->slot != -1);
			feed = IM_TRUE;
			pgnum += post_pgblk->num;
			im_klist_erase(gResvMem->slotIdleMBlst[post_pgblk->slot], post_pgblk->base);
			post_pgblk->idle = IM_FALSE;
            RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, post_pgblk->slot, post_pgblk->base);
			key = post_pgblk->base;
			kfree((void *)post_pgblk);
			post_pgblk = (resv_pgblock_t *)im_klist_erase(gResvMem->mblst, key);
            RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, key);
		}

		if(feed == IM_TRUE){
			IM_INFOMSG((IM_STR("feed: pgbase=%d, pgnum=%d"), pgbase, pgnum));
			im_klist_erase(gResvMem->slotIdleMBlst[pgblk->slot], pgblk->base);
			pgblk->idle = IM_FALSE;
            RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, pgblk->slot, pgblk->base);
			key = pgblk->base;
			kfree((void *)pgblk);
			im_klist_erase(gResvMem->mblst, key);
            RESVMEM_LIST_TRACE(IM_TRUE, IM_FALSE, -1, key);
			if(resvMemFeedSlot(pgbase, pgnum) != IM_RET_OK){
				IM_ERRMSG((IM_STR("resvMemFeedSlot() failed")));
				return IM_RET_FAILED;
			}
		}

		pgblk = post_pgblk;
	}

	//
#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
	return IM_RET_OK;
}

/*
 * the original block if could merge previous or latter to generate a "better idle block". 
 * "better idle block" means it generate a bigger slot block, see below sample:
 *
 * previous--2M	original--8M	latter--1M	user want allocate 6M
 * not better memory block figure: 2M--<6M>|2M--1M, after merge: 2M--<6M>--2M--1M
 * the better memory block figure: 2M--2M|<6M>--1M, after merge: 4M--<6M>--1M
 *
 * previous--2M	original--8M	latter--3M(1+2)	user want allocate 4M
 * not better memory block figure: 2M--<4M>|4M--3M, after merge: 2M--<4M>--4M--3M(1+2)
 * the better memory block figure: 2M--2M|<4M>|2M--3M, after merge: 4M--<4M>--4M--1M
 *
 * the resvMemGetBetterSplitOffset() function to split the pgleft and connect to pre & post, to fit
 * the "better split". if offset is 0, means pgleft all connect to post, else if offset is pgleft,
 * means pgleft all connect to previous.
 */
static inline IM_INT32 CALC_WEIGHT(IM_INT32 pre_pgnum, IM_INT32 post_pgnum)
{
#define W(x)		((1<<x) * x)
	IM_INT32 i, weight = 0;
	IM_INFOMSG((IM_STR("%s(pre_pgnum=%d, post_pgnum=%d)"), IM_STR(_IM_FUNC_), pre_pgnum, post_pgnum));

	for(i=MEMBLK_SLOT_NUM-1; i>=0; i--){
		if(pre_pgnum & (1<<i)){
			weight += W(i);
		}
		if(post_pgnum & (1<<i)){
			weight += W(i);
		}
		IM_INFOMSG((IM_STR("slot=%d, w=%d, weight=%d"), i, W(i), weight));
	}

	return weight;
}

static IM_INT32 resvMemGetBetterSplitOffset(IM_INT32 pre_pgnum, IM_INT32 post_pgnum, IM_INT32 pgleft)
{
	IM_INT32 i, tmp, weight=0, pgoffset = 0;
	IM_INFOMSG((IM_STR("%s(pre_pgnum=%d, post_pgnum=%d, pgleft=%d)"), IM_STR(_IM_FUNC_), pre_pgnum, post_pgnum, pgleft));

	if(pre_pgnum == 0){
		return 0;
	}else if(post_pgnum == 0){
		return pgleft;
	}

	// find the better weight, it's not the best(you think).
	for(i=MEMBLK_SLOT_NUM-1; i>=0; i--){
		if((pre_pgnum & (1<<i)) && (pgleft >= (1<<i))){
			tmp = CALC_WEIGHT(pre_pgnum + (1<<i), post_pgnum + (pgleft - (1<<i)));
			if(weight < tmp){
				weight = tmp;
				pgoffset = 1<<i;
				IM_INFOMSG((IM_STR("weight=%d, pgoffset=%d"), weight, pgoffset));
			}
		}
	}

	return pgoffset;	
}

static IM_INT32 resvMemBetterSplit(IM_INT32 base, IM_INT32 pn)
{
	IM_INT32 pgleft=0, pgoffset=0; 
	IM_INT32 pre_pgnum=0, post_pgnum=0;
	resv_pgblock_t *pgblk, *pre_pgblk, *post_pgblk, *new_pgblk, *new_pgblk2;
	IM_INFOMSG((IM_STR("%s(base=%d, pn=%d)"), IM_STR(_IM_FUNC_), base, pn));

	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, base);
	IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));
	IM_ASSERT((pgblk != IM_NULL) && (pgblk->idle == IM_TRUE));
	IM_ASSERT(pn < pgblk->num);

	pre_pgblk = im_klist_prev(gResvMem->mblst, IM_NULL);	// search backward.
	while((pre_pgblk != IM_NULL) && (pre_pgblk->idle == IM_TRUE)){
		pre_pgnum += pre_pgblk->num;
		pre_pgblk = im_klist_prev(gResvMem->mblst, IM_NULL);
	}

	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, base);	// go back to pgblk to serace forward.
	post_pgblk = im_klist_next(gResvMem->mblst, IM_NULL);	// search forward.
	while((post_pgblk != IM_NULL) && (post_pgblk->idle == IM_TRUE)){
		post_pgnum += post_pgblk->num;
		post_pgblk = im_klist_next(gResvMem->mblst, IM_NULL);
	}
	
	pgleft = pgblk->num - pn;
	IM_INFOMSG((IM_STR("pre_pgnum=%d, post_pgnum=%d, pgleft=%d"), pre_pgnum, post_pgnum, pgleft));
	pgoffset = resvMemGetBetterSplitOffset(pre_pgnum, post_pgnum, pgleft);
	IM_INFOMSG((IM_STR("the better split offset is: %d"), pgoffset));

	// remove pgblk from slotIdleMBlst.
	RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, pgblk->slot, pgblk->base);
	im_klist_erase(gResvMem->slotIdleMBlst[pgblk->slot], pgblk->base);
	
	//
	// pgoffset=0: [pre]--<used>|idle|--[post]
	// pgoffset=pgleft: [pre]--|idle|<used>--[post]
	// pgoffset=other: [pre]--|idle|<used>|idle|--[post]
	//
	if(pgoffset == 0){ // all connect to post.
		// modify the attributes in mblst.
		pgblk->idle = IM_FALSE;
		pgblk->num = pn;
		pgblk->slot = PAGENUM_TO_SLOT(pgblk->num);

		// generate a new_pgblk.
		new_pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
		IM_ASSERT(new_pgblk != IM_NULL); // in this case, the system is muddleheaded.
		new_pgblk->base = pgblk->base + pgblk->num;
		new_pgblk->num = pgleft;
		new_pgblk->slot = PAGENUM_TO_SLOT(new_pgblk->num);
		new_pgblk->idle = IM_TRUE;
		RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, new_pgblk->base);
		im_klist_put(gResvMem->mblst, new_pgblk->base, (void *)new_pgblk);

		IM_ASSERT(resvMemBlockMerge(new_pgblk->base, new_pgblk->num) == IM_RET_OK);
	}else if(pgoffset == pgleft){ // all connect to pre.
		// modify the attributes in mblst.
		pgblk->idle = IM_TRUE;
		pgblk->num = pgleft;
		pgblk->slot = PAGENUM_TO_SLOT(pgblk->num);

		// generate a new_pgblk.
		new_pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
		IM_ASSERT(new_pgblk != IM_NULL); // in this case, the system is muddleheaded.
		new_pgblk->base = pgblk->base + pgblk->num;
		new_pgblk->num = pn;
		new_pgblk->slot = PAGENUM_TO_SLOT(new_pgblk->num);
		new_pgblk->idle = IM_FALSE;
		RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, new_pgblk->base);
		im_klist_put(gResvMem->mblst, new_pgblk->base, (void *)new_pgblk);

		IM_ASSERT(resvMemBlockMerge(pgblk->base, pgblk->num) == IM_RET_OK);
	}else{
		// modify the attributes in mblst.
		pgblk->idle = IM_TRUE;
		pgblk->num = pgoffset;
		pgblk->slot = PAGENUM_TO_SLOT(pgblk->num);

		// generate a new_pgblk(pre).
		new_pgblk = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
		IM_ASSERT(new_pgblk != IM_NULL); // in this case, the system is muddleheaded.
		new_pgblk->base = pgblk->base + pgblk->num;
		new_pgblk->num = pn;
		new_pgblk->slot = PAGENUM_TO_SLOT(new_pgblk->num);
		new_pgblk->idle = IM_FALSE;
		RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, new_pgblk->base);
		im_klist_put(gResvMem->mblst, new_pgblk->base, (void *)new_pgblk);
		
		// generate a new_pgblk(post).
		new_pgblk2 = (resv_pgblock_t *)kmalloc(sizeof(resv_pgblock_t), GFP_KERNEL);
		IM_ASSERT(new_pgblk2 != IM_NULL); // in this case, the system is muddleheaded.
		new_pgblk2->base = new_pgblk->base + new_pgblk->num;
		new_pgblk2->num = pgleft - pgoffset;
		new_pgblk2->slot = PAGENUM_TO_SLOT(new_pgblk2->num);
		new_pgblk2->idle = IM_TRUE;
		RESVMEM_LIST_TRACE(IM_TRUE, IM_TRUE, -1, new_pgblk2->base);
		im_klist_put(gResvMem->mblst, new_pgblk2->base, (void *)new_pgblk2);

		IM_ASSERT(resvMemBlockMerge(pgblk->base, pgblk->num) == IM_RET_OK);
		IM_ASSERT(resvMemBlockMerge(new_pgblk2->base, new_pgblk2->num) == IM_RET_OK);
	}

	return base + pgoffset;
}

/**
 * Allocate linear page-memory from reserved. 
 * It maintains pagemem members include: kVirAddr, phyAddr.
 **/
static IM_RET resvMemAllocLinear(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_BOOL tryAgain=IM_TRUE;
	IM_INT32 slot, pn, pgleft=0, pgbase;
	resv_pgblock_t *pgblk;
	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));

	// pageNum must multiple of gResvMem->unit.
	pn = RESVMEM_UPALIGN(pageNum);
	IM_INFOMSG((IM_STR("pn=%d"), pn));
	if(pn > gResvMem->idleNum){
		IM_ERRMSG((IM_STR("no reserved memory enough to alloc, idleNum=%d, pn=%d"), gResvMem->idleNum, pn));
		return IM_RET_FAILED;
	}

	//
	do{
		for(slot=0; slot < MEMBLK_SLOT_NUM; slot++){
			if((SLOT_TO_PAGENUM(slot) >= pn) && (im_klist_size(gResvMem->slotIdleMBlst[slot]) > 0)){
				IM_INFOMSG((IM_STR("slot[%d](num=%d) can alloc pn=%d"), slot, SLOT_TO_PAGENUM(slot), pn));
				tryAgain = IM_FALSE;
				break;
			}
		}
		if(slot == MEMBLK_SLOT_NUM){ // not found slot can do alloc.
			if(gResvMem->recycled == IM_FALSE){
				IM_TIPMSG((IM_STR("recycle reserved memory and then try again.")));
				if(resvMemRecycle() != IM_RET_OK){
					IM_ERRMSG((IM_STR("resvMemRecycle() failed")));
					return IM_RET_FAILED;
				}
				gResvMem->recycled = IM_TRUE;
				tryAgain = IM_TRUE;
			}else{
				IM_ERRMSG((IM_STR("no reserved linear memory block to alloc, idleNum=%d, pn=%d"), gResvMem->idleNum, pn));
#ifdef TRACE_RESV_MEMBLK	
                resvMemTrace(IM_TRUE);
#endif
                return IM_RET_FAILED;
            }
        }
    }while(tryAgain);

    //
    pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->slotIdleMBlst[slot], IM_NULL);
    IM_ASSERT((pgblk != IM_NULL) && (pgblk->idle == IM_TRUE));

    pgleft = pgblk->num - pn;
    if(pgleft > 0){
#ifdef RESVMEM_ALGORITHM_BETTER_SPLIT
		pgbase = resvMemBetterSplit(pgblk->base, pn);
#else
		pgbase = pgblk->base;
        im_klist_erase(gResvMem->slotIdleMBlst[slot], pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, pgblk->base);
        pgblk->idle = IM_FALSE;
        pgblk->slot = PAGENUM_TO_SLOT(pn); // it may be -1.
		pgblk->num = pn;
		if(resvMemFeedSlot(pgblk->base + pn, pgleft) != IM_RET_OK){
			IM_ERRMSG((IM_STR("resvMemFeedSlot() failed")));
			return IM_RET_FAILED;
		}
#endif
	}else{
		pgbase = pgblk->base;
		IM_ASSERT(pgleft == 0);
		im_klist_erase(gResvMem->slotIdleMBlst[slot], pgblk->base);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, slot, pgblk->base);
		pgblk->idle = IM_FALSE; // modify the MB info in mblst.
	}

	gResvMem->idleNum -= pn;

	//
	pagemem->phyAddr = PAGE_TO_SIZE(pgbase);
	if(gResvMem->sysmem == IM_FALSE){
		if(isCache){
			pagemem->kVirAddr = ioremap(pagemem->phyAddr, PAGE_TO_SIZE(pn));
		}else{
			pagemem->kVirAddr = ioremap_nocache(pagemem->phyAddr, PAGE_TO_SIZE(pn));
		}
		if(pagemem->kVirAddr == IM_NULL){
			IM_ERRMSG((IM_STR("ioremap_nocache() failed")));
			goto Fail;
		}
	}else{
		pagemem->kVirAddr = phys_to_virt(pagemem->phyAddr);
		IM_ASSERT(pagemem->kVirAddr != 0);	
	}

	//
#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
	return IM_RET_OK;
Fail:
	resvMemFreeLinear(pagemem);
	return IM_RET_FAILED;
}

static IM_RET resvMemFreeBlock(IM_INT32 pgbase)
{
	resv_pgblock_t *pgblk;
	IM_INFOMSG((IM_STR("%s(pgbase=%d)"), IM_STR(_IM_FUNC_), pgbase));

	pgblk = (resv_pgblock_t *)im_klist_key(gResvMem->mblst, pgbase);
	IM_ASSERT((pgblk != IM_NULL) && (pgblk->idle == IM_FALSE));
	IM_INFOMSG((IM_STR("pgblk: base=%d, num=%d, slot=%d, idle=%d"), pgblk->base, pgblk->num, pgblk->slot, pgblk->idle));

	if(pgblk->slot != -1){
		pgblk->idle = IM_TRUE;
		im_klist_put(gResvMem->slotIdleMBlst[pgblk->slot], pgblk->base, (void *)pgblk);
        RESVMEM_LIST_TRACE(IM_FALSE, IM_TRUE, pgblk->slot, pgblk->base);
		if(im_klist_size(gResvMem->slotIdleMBlst[pgblk->slot]) > 1){
			resvMemSlotMerge(pgblk->slot, pgblk->base);
		}
		gResvMem->recycled = IM_FALSE; // it could do recycle.
	}else{
		pgblk->idle = IM_TRUE;
		if(resvMemBlockMerge(pgblk->base, pgblk->num) != IM_RET_OK){
			IM_ERRMSG((IM_STR("resvMemBlockMerge() failed")));
			return IM_RET_FAILED;
		}
	}

	return IM_RET_OK;
}

/**
 * Free linear page-memory allocated from reserved previously. 
 * It maintains pagemem members include: kVirAddr, phyAddr.
 **/
static IM_RET resvMemFreeLinear(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));

	if(resvMemFreeBlock(SIZE_TO_PAGE(pagemem->phyAddr)) != IM_RET_OK){
		IM_ERRMSG((IM_STR("resvMemFreeBlock() failed")));
		return IM_RET_FAILED;
	}
	gResvMem->idleNum += RESVMEM_UPALIGN(pagemem->totalPageNum);

	//
	if((pagemem->kVirAddr != IM_NULL) && (gResvMem->sysmem == IM_FALSE)){
		iounmap(pagemem->kVirAddr);
		pagemem->kVirAddr = IM_NULL;
	}
	pagemem->phyAddr = 0;

	//
#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
	return IM_RET_OK;
}

/**
 * Alloc nonlinear page-memory from reserved. 
 * It maintains pagemem members include: blockNum, blocks.
 **/
static IM_RET resvMemAllocNonlinear(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_INT32 i, k, pn, pgbase;
	resv_pgblock_t *pgblk;
	pmmpwl_memblock_t *mblk = IM_NULL;
	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));

	// pageNum must multiple of gResvMem->unit.
	pn = RESVMEM_UPALIGN(pageNum);
	IM_INFOMSG((IM_STR("pn=%d"), pn));
	if(pn > gResvMem->idleNum){
		IM_ERRMSG((IM_STR("no reserved memory enough to alloc, idleNum=%d, pn=%d"), gResvMem->idleNum, pn));
		return IM_RET_FAILED;
	}

	// max has "pn / gResvMem->unit" mblk.
	mblk = (pmmpwl_memblock_t *)pmmpwl_malloc(sizeof(pmmpwl_memblock_t) * (pn / gResvMem->unit));
	if(mblk == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(mblk) failed")));
		return IM_RET_FAILED;
	}
	pmmpwl_memset((void *)mblk, 0, sizeof(pmmpwl_memblock_t) * (pn / gResvMem->unit));
	pagemem->blocks = mblk;
	pagemem->blockNum = 0;

	//
	// find the idle memory blocks which size from smallest to biggest.
	//
	for(i=0, k=0; (i < MEMBLK_SLOT_NUM) && (pn > 0); i++)
	{
		pgblk = (resv_pgblock_t *)im_klist_begin(gResvMem->slotIdleMBlst[i], IM_NULL);
		while((pn > 0) && (pgblk != IM_NULL))
		{
			IM_ASSERT(pgblk->idle == IM_TRUE);
			
			if(pn <= SLOT_TO_PAGENUM(i)){
#ifdef RESVMEM_ALGORITHM_BETTER_SPLIT
				pgbase = resvMemBetterSplit(pgblk->base, pn);
#else
				// first remove from current idle-slot.
				pgbase = pgblk->base;
				im_klist_erase(gResvMem->slotIdleMBlst[i], pgblk->base);
				RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, i, pgblk->base);
				pgblk->idle = IM_FALSE;
				pgblk->num = pn;
				pgblk->slot = PAGENUM_TO_SLOT(pn); // it may be -1.
				if(resvMemFeedSlot(pgblk->base + pn, SLOT_TO_PAGENUM(i) - pn) != IM_RET_OK){
					IM_ERRMSG((IM_STR("resvMemFeedSlot() failed")));
					goto Fail;
				}
#endif
				// CAUTION: total pgNum must actually equal to the reqested pageNum(it bind to mmu pglist).
				mblk[k].pgNum = pn - ((pageNum % gResvMem->unit) ? (gResvMem->unit - (pageNum % gResvMem->unit)): 0);
				pn = 0;	
			}else{
				pgbase = pgblk->base;
				im_klist_erase(gResvMem->slotIdleMBlst[i], pgblk->base);
				RESVMEM_LIST_TRACE(IM_FALSE, IM_FALSE, i, pgblk->base);
				pgblk->idle = IM_FALSE;
				mblk[k].pgNum = SLOT_TO_PAGENUM(i);
				pn -= SLOT_TO_PAGENUM(i);
			}

			mblk[k].phyAddr = PAGE_TO_SIZE(pgbase);
			mblk[k].isCache = isCache;
			mblk[k].pg = IM_NULL;
			IM_INFOMSG((IM_STR("mblk[%d]:phyAddr=0x%x, pgNum=%d, from slot[%d]"), k, mblk[k].phyAddr, mblk[k].pgNum, i));
			pagemem->blockNum++;
			k++;
			
			if(pn > 0){
				pgblk = (resv_pgblock_t *)im_klist_next(gResvMem->slotIdleMBlst[i], IM_NULL);
			}
		}
	}

	gResvMem->idleNum -= RESVMEM_UPALIGN(pageNum);

	//
#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
	return IM_RET_OK;
Fail:
	resvMemFreeNonlinear(pagemem);
	return IM_RET_FAILED;
}

/**
 * Free nonlinear page-memory allocated from reserved previously.
 * It maintains pagemem members include: blockNum, blocks.
 **/
static void resvMemFreeNonlinear(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INT32 i;
	pmmpwl_memblock_t *mblk;
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));
	
	mblk = pagemem->blocks;
	IM_ASSERT(mblk != IM_NULL);

	for(i=0; i<pagemem->blockNum; i++){
		if(mblk->phyAddr != 0){
			IM_INFOMSG((IM_STR("mblk[%d]:phyAddr=0x%x, pgNum=%d"), i, mblk->phyAddr, mblk->pgNum));
			resvMemFreeBlock(SIZE_TO_PAGE(mblk->phyAddr));
			gResvMem->idleNum += RESVMEM_UPALIGN(mblk->pgNum);
		}
		mblk++;
	}

	pmmpwl_free((void *)pagemem->blocks);
	pagemem->blocks = IM_NULL;
	pagemem->blockNum = 0;

#ifdef TRACE_RESV_MEMBLK	
	resvMemTrace(IM_TRUE);
#endif
}

static void bigmemTrace(bigmem_t *bigmem)
{
    IM_INT32 i;
    bigmem_block_t *bmb;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_TIPMSG((IM_STR("=========================================================")));
    IM_TIPMSG((IM_STR("bigmem: pageNum=%d, blockNum=%d, idleBlockNum=%d, isCache=%d"), 
		bigmem->pageNum, bigmem->blockNum, bigmem->idleBlockNum, bigmem->isCache));
    i = 0;
    bmb = (bigmem_block_t *)im_list_begin(bigmem->bmblst);
    while(bmb != IM_NULL){
        IM_TIPMSG((IM_STR("bmblst[%d], idle=%d, pgbase=%d, pgnum=%d"), i, bmb->idle, bmb->pgbase, bmb->pgnum));
        bmb = (bigmem_block_t *)im_list_next(bigmem->bmblst);
        i++;
    }
	IM_TIPMSG((IM_STR("=========================================================")));
}

static IM_RET bigmemInit(bigmem_t *bigmem, IM_INT32 pageNum, IM_INT32 blockNum, IM_BOOL isCache)
{
    IM_INT32 pgbase, i, pn;
    bigmem_block_t *bmb = IM_NULL;

	IM_INFOMSG((IM_STR("%s(pageNum=%d, blockNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, blockNum, isCache));
    IM_ASSERT(bigmem != IM_NULL);

	// pageNum must multiple of gResvMem->unit.
	pn = RESVMEM_UPALIGN(pageNum);
	IM_INFOMSG((IM_STR("pn=%d"), pn));
	if(pn * blockNum > gResvMem->idleNum){
		IM_ERRMSG((IM_STR("no reserved memory enough to alloc, idleNum=%d, pn=%d, blockNum=%d"), gResvMem->idleNum, pn, blockNum));
		return IM_RET_FAILED;
	}

	//
    bigmem->mpl = im_mpool_init((func_mempool_malloc_t)pmmpwl_malloc, (func_mempool_free_t)pmmpwl_free);
	if(bigmem->mpl == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed")));
		goto Fail;
	}

    bigmem->bmblst = im_list_init(0, bigmem->mpl);
	if(bigmem->bmblst == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init() failed")));
		goto Fail;
	}

	for(i=0; i<blockNum; i++){
		bmb = (bigmem_block_t *)pmmpwl_malloc(sizeof(bigmem_block_t));
		if(bmb == IM_NULL){
			IM_ERRMSG((IM_STR("malloc(bmb) failed")));
			goto Fail;
		}
		memset((void *)bmb, 0, sizeof(bigmem_block_t));
		bmb->pgbase = -1;
		im_list_put_back(bigmem->bmblst, (void *)bmb);

		pgbase = resvMemAllocBigMemBlock(pn);
		if(pgbase == -1){
			IM_ERRMSG((IM_STR("resvMemAllocBigMemBlock(pn=%d) failed"), pn));
			goto Fail;
		}
		bmb->idle = IM_TRUE;
		bmb->pgbase = pgbase;	
		bmb->pgnum = pn;
		IM_INFOMSG((IM_STR("bmb%d: idle=%d, pgbase=%d, pgnum=%d"), i, bmb->idle, bmb->pgbase, bmb->pgnum));
	}
	
	bigmem->pageNum = pn;
	bigmem->blockNum = blockNum;
	bigmem->idleBlockNum = blockNum;
	bigmem->isCache = isCache;
	bigmem->sysmem = gResvMem->sysmem;

#ifdef TRACE_BIGMEM
	bigmemTrace(bigmem);
#endif
	return IM_RET_OK;
Fail:
	if(bigmem->bmblst != IM_NULL){
		bmb = (bigmem_block_t *)im_list_begin(bigmem->bmblst);
		while(bmb != IM_NULL){
			if(bmb->pgbase != -1){
				resvMemFreeBigMemBlock(bmb->pgbase);
			}
			pmmpwl_free((void *)bmb);
			bmb = (bigmem_block_t *)im_list_erase(bigmem->bmblst, bmb);
		}
		im_list_deinit(bigmem->bmblst);
		bigmem->bmblst = IM_NULL;
	}
	if(bigmem->mpl != IM_NULL){
		im_mpool_deinit(bigmem->mpl);
		bigmem->mpl = IM_NULL;
	}
    return IM_RET_FAILED;
}

static IM_RET bigmemDeinit(bigmem_t *bigmem)
{
    bigmem_block_t *bmb;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(bigmem != IM_NULL);
	IM_ASSERT(bigmem->bmblst != IM_NULL);
	IM_ASSERT(bigmem->mpl != IM_NULL);

	bmb = (bigmem_block_t *)im_list_begin(bigmem->bmblst);
	while(bmb != IM_NULL){
        IM_INFOMSG((IM_STR("bmb: base=%d, num=%d, idle=%d"), bmb->pgbase, bmb->pgnum, bmb->idle));
        if(bmb->idle != IM_TRUE){
            IM_WARNMSG((IM_STR("non-idle bmb: base=%d, num=%d"), bmb->pgbase, bmb->pgnum));
        }
		IM_ASSERT(bmb->pgbase != -1);
        resvMemFreeBigMemBlock(bmb->pgbase);
        pmmpwl_free((void *)bmb);

        bmb = (bigmem_block_t *)im_list_erase(bigmem->bmblst, bmb);
    }

	im_list_deinit(bigmem->bmblst);
	bigmem->bmblst = IM_NULL;

	im_mpool_deinit(bigmem->mpl);
	bigmem->mpl = IM_NULL;

	bigmem->pageNum = 0;
	bigmem->blockNum = 0;
	bigmem->idleBlockNum = 0;

	return IM_RET_OK;
}

static IM_RET bigmemAllocBlock(bigmem_t *bigmem, pmmpwl_pagemem_t *pagemem)
{
	bigmem_block_t *bmb;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(bigmem != IM_NULL);
    IM_ASSERT(pagemem != IM_NULL);

	if(bigmem->idleBlockNum == 0){
		IM_ERRMSG((IM_STR("bigmem idleBlockNum is 0, cannot alloc block")));
		return IM_RET_FAILED;
	}

	bmb = (bigmem_block_t *)im_list_begin(bigmem->bmblst);
	while((bmb != IM_NULL) && (bmb->idle != IM_TRUE)){
		bmb = (bigmem_block_t *)im_list_next(bigmem->bmblst);
	}
	IM_ASSERT(bmb != IM_NULL);

	//
	pagemem->phyAddr = PAGE_TO_SIZE(bmb->pgbase);
	if(bigmem->sysmem == IM_FALSE){
		if(bigmem->isCache){
			pagemem->kVirAddr = ioremap(pagemem->phyAddr, PAGE_TO_SIZE(bmb->pgnum));
		}else{
			pagemem->kVirAddr = ioremap_nocache(pagemem->phyAddr, PAGE_TO_SIZE(bmb->pgnum));
		}
		if(pagemem->kVirAddr == IM_NULL){
			IM_ERRMSG((IM_STR("ioremap_nocache() failed")));
			return IM_RET_FAILED;
		}
	}else{
		pagemem->kVirAddr = phys_to_virt(pagemem->phyAddr);
		IM_ASSERT(pagemem->kVirAddr != 0);	
	}

	//
	bmb->idle = IM_FALSE;
	bigmem->idleBlockNum--;
	IM_INFOMSG((IM_STR("bigmem alloc a block(pgbase=%d), idleBlockNum=%d"), bmb->pgbase, bigmem->idleBlockNum));

	//
#ifdef TRACE_BIGMEM	
	bigmemTrace(bigmem);
#endif
	return IM_RET_OK;
}

static IM_RET bigmemFreeBlock(bigmem_t *bigmem, pmmpwl_pagemem_t *pagemem)
{
	bigmem_block_t *bmb;
    IM_INT32 pgbase = SIZE_TO_PAGE(pagemem->phyAddr);
	IM_INFOMSG((IM_STR("%s(pgbase=%d)"), IM_STR(_IM_FUNC_), pgbase));
    IM_ASSERT(bigmem != IM_NULL);
    IM_ASSERT(pagemem != IM_NULL);

	bmb = (bigmem_block_t *)im_list_begin(bigmem->bmblst);
	while((bmb != IM_NULL) && (bmb->pgbase != pgbase)){
		bmb = (bigmem_block_t *)im_list_next(bigmem->bmblst);
    }
	if((bmb == IM_NULL) || (bmb->idle == IM_TRUE)){
        IM_WARNMSG((IM_STR("incorrect bmb, pgbase=%d"), pgbase));
		return IM_RET_FAILED;
	}

	bmb->idle = IM_TRUE;
	bigmem->idleBlockNum++;
	IM_INFOMSG((IM_STR("bigmem free a block(pgbase=%d), idleBlockNum=%d"), bmb->pgbase, bigmem->idleBlockNum));

	//
	if((pagemem->kVirAddr != IM_NULL) && (bigmem->sysmem == IM_FALSE)){
		iounmap(pagemem->kVirAddr);
		pagemem->kVirAddr = IM_NULL;
	}
	pagemem->phyAddr = 0;

	//
#ifdef TRACE_BIGMEM	
	bigmemTrace(bigmem);
#endif
	return IM_RET_OK;
}

/**
 * Allocate linear page-memory from system. 
 * It maintains pagemem members include: kVirAddr, phyAddr.
 **/
static IM_RET systemAllocLinear(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));

	if(isCache){
		pagemem->kVirAddr = kmalloc(PAGE_TO_SIZE(pageNum), GFP_KERNEL | /*GFP_HIGHUSER | __GFP_ZERO |*/ __GFP_NOWARN | __GFP_REPEAT);
		if(pagemem->kVirAddr == IM_NULL){
			//IM_ERRMSG((IM_STR("kmalloc(%d) failed"), PAGE_TO_SIZE(pageNum)));
			goto Fail;
		}
		pagemem->phyAddr = __pa(pagemem->kVirAddr);
	}else{
		pagemem->kVirAddr = kmalloc((pageNum << PAGE_SHIFT), GFP_KERNEL | /*GFP_HIGHUSER | __GFP_ZERO |*/ __GFP_NOWARN | __GFP_COLD | __GFP_REPEAT);
		if(pagemem->kVirAddr == IM_NULL){
			//IM_ERRMSG((IM_STR("kmalloc(%d) failed"), PAGE_TO_SIZE(pageNum)));
			goto Fail;
		}
		pagemem->phyAddr = dma_map_page(NULL, virt_to_page(pagemem->kVirAddr), 0, pageNum << PAGE_SHIFT, DMA_BIDIRECTIONAL);
	}

	return IM_RET_OK;
Fail:
	systemFreeLinear(pagemem);
	return IM_RET_FAILED;
}

/**
 * Free linear page-memory which allocated from system previously. 
 * It maintains pagemem members include: kVirAddr, phyAddr.
 **/
static void systemFreeLinear(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));
	if(pagemem->kVirAddr){
		if(pagemem->isCache){
			dma_unmap_page(NULL, pagemem->phyAddr, pagemem->totalPageNum << PAGE_SHIFT, DMA_BIDIRECTIONAL);
		}
		kfree(pagemem->kVirAddr);
		pagemem->kVirAddr = IM_NULL;
		pagemem->phyAddr = 0;
	}
}

/**
 * Alloc nonlinear page-memory from system. 
 * It maintains pagemem members include: blockNum, blocks.
 **/
static IM_RET systemAllocNonlinear(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_INT32 i;
	pmmpwl_memblock_t *mblk = IM_NULL;

	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));

	mblk = (pmmpwl_memblock_t *)pmmpwl_malloc(sizeof(pmmpwl_memblock_t) * pageNum);
	if(mblk == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(mblk) failed")));
		return IM_RET_FAILED;
	}
	pmmpwl_memset((void *)mblk, 0, sizeof(pmmpwl_memblock_t) * pageNum);
	pagemem->blocks = mblk;
	pagemem->blockNum = 0;

	for(i=0; i<pageNum; i++){
		if(isCache){
			mblk[i].pg = alloc_page(GFP_KERNEL | /*GFP_HIGHUSER | __GFP_ZERO |*/ __GFP_REPEAT | __GFP_NOWARN);
		}else{
			mblk[i].pg = alloc_page(GFP_KERNEL | /*GFP_HIGHUSER | __GFP_ZERO |*/ __GFP_REPEAT | __GFP_NOWARN | __GFP_COLD);
		}
		if(mblk[i].pg == IM_NULL){
			//IM_ERRMSG((IM_STR("alloc_page() failed")));
			goto Fail;
		}

		if(isCache){
			mblk[i].phyAddr = page_to_phys(mblk[i].pg);
		}else{
			mblk[i].phyAddr = dma_map_page(NULL, mblk[i].pg, 0, PAGE_SIZE, DMA_BIDIRECTIONAL);
		}
		mblk[i].isCache = isCache;
		mblk[i].pgNum = 1;
		pagemem->blockNum++;
		//IM_INFOMSG((IM_STR("mblk[%d].phyAddr=0x%x"), i, mblk[i].phyAddr));
	}
	
	return IM_RET_OK;
Fail:
	systemFreeNonlinear(pagemem);
	return IM_RET_FAILED;
}

/**
 * Free linear page-memory which allocated from system previously. 
 * It maintains pagemem members include: blockNum, blocks.
 **/
static void systemFreeNonlinear(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INT32 i;
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));

	for(i=0; i<pagemem->blockNum; i++){
		if(pagemem->blocks[i].pg != IM_NULL){
			if(! pagemem->blocks[i].isCache){
				dma_unmap_page(NULL, pagemem->blocks[i].phyAddr, PAGE_SIZE, DMA_BIDIRECTIONAL);
			}
			__free_page(pagemem->blocks[i].pg);
		}
	}
	pmmpwl_free((void *)pagemem->blocks);
	pagemem->blocks = IM_NULL;
	pagemem->blockNum = 0;
}

static IM_RET allocLinearPageMemory(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));

	if(pageNum >= SIZE_TO_PAGE(PMM_RESERVE_MEM_THRESHOLD)){
		if(gResvMem->idleNum >= pageNum){
			if(resvMemAllocLinear(pagemem, pageNum, isCache) == IM_RET_OK){
				pagemem->resvmem = IM_TRUE;
				return IM_RET_OK;
			}
		}
		
		IM_WARNMSG((IM_STR("resvMemAllocLinear(pageNum=%d) failed, try to use system memory."), pageNum));
		if(systemAllocLinear(pagemem, pageNum, isCache) != IM_RET_OK){
			IM_ERRMSG((IM_STR("resvMemAllocLinear + systemAllocLinear all failed, pageNum=%d"), pageNum));
			return IM_RET_FAILED;
		}
		pagemem->resvmem = IM_FALSE;
	}else{
		if(systemAllocLinear(pagemem, pageNum, isCache) == IM_RET_OK){
			pagemem->resvmem = IM_FALSE;
			return IM_RET_OK;
		}

		IM_WARNMSG((IM_STR("systemAllocLinear(pageNum=%d) failed, try to use reserved memory."), pageNum));
		if((gResvMem->idleNum < pageNum) || (resvMemAllocLinear(pagemem, pageNum, isCache) != IM_RET_OK)){
			IM_ERRMSG((IM_STR("systemAllocLinear + resvMemAllocLinear all failed, pageNum=%d"), pageNum));
			return IM_RET_FAILED;
		}
		pagemem->resvmem = IM_TRUE;
	}

	return IM_RET_OK;
}

static void freeLinearPageMemory(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));
	if(pagemem->resvmem == IM_TRUE){
		resvMemFreeLinear(pagemem);
	}else{
		systemFreeLinear(pagemem);
	}
}

static IM_RET allocNonlinearPageMemory(IN pmmpwl_pagemem_t *pagemem, IN IM_INT32 pageNum, IN IM_BOOL isCache)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, isCache));
	if(systemAllocNonlinear(pagemem, pageNum, isCache) == IM_RET_OK){
		pagemem->resvmem = IM_FALSE;
		return IM_RET_OK;
	}

#ifdef RESVMEM_ALLOC_NONLINEAR_ENABLE 
	IM_WARNMSG((IM_STR("systemAllocNonlinear(pageNum=%d) failed, try to use reserved memory."), pageNum));
	if((gResvMem->idleNum < pageNum) || (resvMemAllocNonlinear(pagemem, pageNum, isCache) != IM_RET_OK)){
		IM_ERRMSG((IM_STR("systemAllocNonlinear + resvMemAllocNonlinear all failed, pageNum=%d, resv->idleNum=%d"), pageNum, gResvMem->idleNum));
		return IM_RET_FAILED;
	}
	pagemem->resvmem = IM_TRUE;
	return IM_RET_OK;	
#else
	IM_ERRMSG((IM_STR("systemAllocNonlinear(pageNum=%d) failed"), pageNum));
	return IM_RET_FAILED;
#endif
}

static void freeNonlinearPageMemory(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INFOMSG((IM_STR("%s(pageNum=%d)"), IM_STR(_IM_FUNC_), pagemem->totalPageNum));
	if(pagemem->resvmem){
		resvMemFreeNonlinear(pagemem);
	}else{
		systemFreeNonlinear(pagemem);
	}
}

static void unmapUserPageMemory(IN pmmpwl_pagemem_t *pagemem)
{
	IM_INT32 i;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	for(i=0; i<pagemem->blockNum; i++){
		IM_INFOMSG((IM_STR("unmap%d phyAddr=0x%x"), i, pagemem->blocks[i].phyAddr));
		dma_unmap_page(NULL, pagemem->blocks[i].phyAddr, PAGE_SIZE, DMA_BIDIRECTIONAL);
	}
	pmmpwl_free((void *)pagemem->blocks);
}

bigmem_handle_t pmmpwl_init_bigmem(IN IM_INT32 pageNum, IN IM_INT32 blockNum, IN IM_BOOL isCache)
{
    bigmem_t *bigmem;
	IM_INFOMSG((IM_STR("%s(pageNum=%d, blockNum=%d)"), IM_STR(_IM_FUNC_), pageNum, blockNum));

    bigmem = (bigmem_t *)pmmpwl_malloc(sizeof(bigmem_t));
    if(bigmem == IM_NULL){
        IM_ERRMSG((IM_STR("malloc(bigmem_t) failed")));
        return IM_NULL;
    }
    pmmpwl_memset((void *)bigmem, 0, sizeof(bigmem_t));

    if(bigmemInit(bigmem, pageNum, blockNum, isCache) != IM_RET_OK){
        IM_ERRMSG((IM_STR("bigmemInit(pageNum=%d, blockNum=%d, isCache=%d) failed"), pageNum, blockNum, isCache));
        pmmpwl_free((void *)bigmem);
        return IM_NULL;
    }
    IM_TIPMSG((IM_STR("init bigmem: pageNum=%d, blockNum=%d, isCache=%d, size=0x%x"), 
		bigmem->pageNum, bigmem->blockNum, bigmem->isCache, PAGE_TO_SIZE(bigmem->pageNum * bigmem->blockNum)));

    return (bigmem_handle_t)bigmem; 
}

IM_RET pmmpwl_deinit_bigmem(IN bigmem_handle_t handle)
{
    bigmem_t *bigmem = (bigmem_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(bigmem != IM_NULL);

    if(bigmemDeinit(bigmem) != IM_RET_OK){
        IM_ERRMSG((IM_STR("bigmemDeinit() failed")));
        return IM_RET_FAILED;
    }

    pmmpwl_free((void *)bigmem);
    return IM_RET_OK;
}

void *pmmpwl_alloc_bigmem_block(IN bigmem_handle_t handle, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist, IN IM_INT32 pglistLen)
{
	IM_INT32 i, pageNum;
	pmmpwl_pagemem_t *pagemem = IM_NULL;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(buffer != IM_NULL);
	IM_ASSERT(pglist != IM_NULL);
	IM_ASSERT(PAGE_SIZE == 4096);	// if PAGE_SIZE is not 4096, the page_memory's alloc/free all need rewrite.

	// init pagemem, the alc_buffer_t->privData.
	pagemem = (pmmpwl_pagemem_t *)pmmpwl_malloc(sizeof(pmmpwl_pagemem_t));
	if(pagemem == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pagemem) failed")));
		return IM_NULL;
	}
	pmmpwl_memset((void *)pagemem, 0, sizeof(pmmpwl_pagemem_t));
	
	// allocate memory.
    if(bigmemAllocBlock((bigmem_t *)handle, pagemem) != IM_RET_OK){
        IM_ERRMSG((IM_STR("bigmemAllocBlock() failed")));
        goto Fail;
    }
    IM_INFOMSG((IM_STR("alloc bigmem block, kVirAddr=0x%x, phyAddr=0x%x"), (IM_INT32)pagemem->kVirAddr, pagemem->phyAddr));
	pageNum = ((bigmem_t *)handle)->pageNum;
    pagemem->linear = IM_TRUE;
    pagemem->totalPageNum = pageNum;
    pagemem->isCache = ((bigmem_t *)handle)->isCache;

    buffer->vir_addr = pagemem->kVirAddr;
    buffer->phy_addr = pagemem->phyAddr;
    buffer->size = pageNum << PAGE_SHIFT;
    buffer->flag = IM_BUFFER_FLAG_PHY;

	IM_ASSERT(pglistLen <= pageNum);
    for(i=0; i<pglistLen; i++){
        pglist[i] = SIZE_TO_PAGE(pagemem->phyAddr) + i;
    }

    return (void *)pagemem;
Fail:
    pmmpwl_free((void *)pagemem);
    return IM_NULL;
}

IM_RET pmmpwl_free_bigmem_block(IN bigmem_handle_t handle, IN void *privData)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(privData != IM_NULL);

    if(bigmemFreeBlock((bigmem_t *)handle, (pmmpwl_pagemem_t *)privData) != IM_RET_OK){
        IM_ERRMSG((IM_STR("bigmemFreeBlock() failed")));
        return IM_RET_FAILED;
    }

    pmmpwl_free(privData);
    return IM_RET_OK;
}

void *pmmpwl_alloc_page_memory(IN IM_INT32 pageNum, IN IM_BOOL linear, IN IM_BOOL isCache, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist)
{
	IM_INT32 i, j, k;
	pmmpwl_pagemem_t *pagemem = IM_NULL;

	IM_INFOMSG((IM_STR("%s(pageNum=%d, linear=%d, isCache=%d)"), IM_STR(_IM_FUNC_), pageNum, linear, isCache));
	IM_ASSERT(buffer != IM_NULL);
	IM_ASSERT(pglist != IM_NULL);
	IM_ASSERT(PAGE_SIZE == 4096);	// if PAGE_SIZE is not 4096, the page_memory's alloc/free all need rewrite.

	// init pagemem, the alc_buffer_t->privData.
	pagemem = (pmmpwl_pagemem_t *)pmmpwl_malloc(sizeof(pmmpwl_pagemem_t));
	if(pagemem == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pagemem) failed")));
		return IM_NULL;
	}
	pmmpwl_memset((void *)pagemem, 0, sizeof(pmmpwl_pagemem_t));
	
	// allocate memory.
	if(linear == IM_TRUE){
		if(allocLinearPageMemory(pagemem, pageNum, isCache) != IM_RET_OK){
			IM_ERRMSG((IM_STR("allocLinearPageMemory(pageNum=%d) failed"), pageNum));
			goto Fail;
		}
		IM_INFOMSG((IM_STR("alloc linear memory, kVirAddr=0x%x, phyAddr=0x%x, pageNum=%d, resvmem=%d"), 
			(IM_INT32)pagemem->kVirAddr, pagemem->phyAddr, pageNum, pagemem->resvmem));
		pagemem->linear = IM_TRUE;
		pagemem->totalPageNum = pageNum;
		pagemem->isCache = isCache;

		buffer->vir_addr = pagemem->kVirAddr;
		buffer->phy_addr = pagemem->phyAddr;
		buffer->size = pageNum << PAGE_SHIFT;
		buffer->flag = IM_BUFFER_FLAG_PHY;

		for(i=0; i<pageNum; i++){
			pglist[i] = SIZE_TO_PAGE(pagemem->phyAddr) + i;
		}
	}else{ 
		if(allocNonlinearPageMemory(pagemem, pageNum, isCache) != IM_RET_OK){
			IM_ERRMSG((IM_STR("allocNonlinearPageMemory(pageNum=%d) failed"), pageNum));
			goto Fail;
		}
		IM_INFOMSG((IM_STR("alloc nonlinear memory, blockNum=%d, pageNum=%d, resvmem=%d"), 
			(IM_INT32)pagemem->blockNum, pageNum, pagemem->resvmem));
		pagemem->linear = IM_FALSE;
		pagemem->totalPageNum = pageNum;
		pagemem->isCache = isCache;

		buffer->vir_addr = 0;
		buffer->phy_addr = pagemem->blocks[0].phyAddr;
		buffer->size = pageNum << PAGE_SHIFT;
		buffer->flag = IM_BUFFER_FLAG_PHY_NONLINEAR;

		for(i=0, k=0; i<pagemem->blockNum; i++){
			for(j=0; j<pagemem->blocks[i].pgNum; j++){
				pglist[k++] = SIZE_TO_PAGE(pagemem->blocks[i].phyAddr) + j;
			}
		}
	}

	return (void *)pagemem;
Fail:
	pmmpwl_free((void *)pagemem);
	return IM_NULL;
}

void *pmmpwl_map_useraddr_to_page_memory(IN void *uVirAddr, IN IM_INT32 size, OUT IM_Buffer *buffer, OUT IM_UINT32 *pglist, OUT IM_INT32 *actualPageNum)
{
	IM_INT32 i, pgNum, offset;
	pmmpwl_pagemem_t *pagemem = IM_NULL;
	pmmpwl_memblock_t *mblk = IM_NULL;
	struct page **pages = IM_NULL;
	//void *kvir;

	offset = (IM_UINT32)uVirAddr & 0xfff;	
	IM_INFOMSG((IM_STR("%s(uVirAddr=0x%x, size=%d, offset=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)uVirAddr, size, offset));
	IM_ASSERT(PAGE_SIZE == 4096);
	IM_ASSERT(((IM_UINT32)uVirAddr & (PAGE_SIZE - 1)) == 0);
	IM_ASSERT((size & (PMM_PAGE_SIZE - 1)) == 0);

	// init pagemem, the alc_buffer_t->privData.
	pagemem = (pmmpwl_pagemem_t *)pmmpwl_malloc(sizeof(pmmpwl_pagemem_t));
	if(pagemem == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pagemem) failed")));
		return IM_NULL;
	}
	pmmpwl_memset((void *)pagemem, 0, sizeof(pmmpwl_pagemem_t));

	//
	pages = (struct page **)pmmpwl_malloc(sizeof(struct page *) * SIZE_TO_PAGE(size));
	if(pages == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pages) failed")));
		goto Fail;
	}

	down_read(&current->mm->mmap_sem);
	pgNum = get_user_pages(current, current->mm, (unsigned int)uVirAddr, SIZE_TO_PAGE(size), 1, 0, pages, IM_NULL);
	up_read(&current->mm->mmap_sem);

	IM_INFOMSG((IM_STR("get_user_pages(), pgNum=%d"), pgNum));
	if((pgNum <= 0) || (pgNum != SIZE_TO_PAGE(size))){
		IM_ERRMSG((IM_STR("get_user_pages() failed, pgNum=%d, SIZE_TO_PAGE(%d)=%d"), pgNum, size, SIZE_TO_PAGE(size)));
		goto Fail;
	}

	//
	pagemem->linear = IM_FALSE;
	pagemem->mapped = IM_TRUE;
	pagemem->isCache = IM_FALSE;	// IM_TRUE;
	pagemem->totalPageNum = pgNum;
	pagemem->blockNum = pgNum;
	pagemem->blocks = (pmmpwl_memblock_t *)pmmpwl_malloc(sizeof(pmmpwl_memblock_t) * pagemem->blockNum);
	if(pagemem->blocks == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pagemem->blocks) failed")));
		goto Fail;
	}

	mblk = pagemem->blocks;
	for(i=0; i<pgNum; i++){
		mblk[i].pg = pages[i];
		//mblk[i].phyAddr = page_to_phys(pages[i]);
		mblk[i].phyAddr = dma_map_page(NULL, pages[i], 0, PAGE_SIZE, DMA_BIDIRECTIONAL);
		pglist[i] = SIZE_TO_PAGE(mblk[i].phyAddr);
		IM_INFOMSG((IM_STR("page%d, phyAddr=0x%x"), i, mblk[i].phyAddr));
		put_page(pages[i]);	// does it need mmap_sem lock ???
	}

	buffer->vir_addr = uVirAddr;
	buffer->phy_addr = pagemem->blocks->phyAddr;
	//buffer->size = size;	// don't modify buffer->size, because user-buffer size we don't change.
	buffer->flag = IM_BUFFER_FLAG_PHY_NONLINEAR | IM_BUFFER_FLAG_MAPPED;

	*actualPageNum = pgNum;

	pmmpwl_free((void *)pages);
	return (void *)pagemem;
Fail:
	if(pagemem->blocks != IM_NULL){
		for(i=0; i<pagemem->blockNum; i++){
			if(pagemem->blocks[i].pg != IM_NULL){
				dma_unmap_page(NULL, pagemem->blocks[i].phyAddr, PAGE_SIZE, DMA_BIDIRECTIONAL);
			}
		}
		pmmpwl_free((void *)pagemem->blocks);
	}
	if(pages != IM_NULL){
		pmmpwl_free((void *)pages);
	}
	pmmpwl_free((void *)pagemem);
	return IM_NULL;
}

IM_RET pmmpwl_flush_buffer(IN void *privData, IN IM_INT32 flag)
{
	IM_INT32 i;
	void *start_v, *end_v;
	IM_UINT32 start_p, end_p;
	pmmpwl_pagemem_t *pagemem = (pmmpwl_pagemem_t *)privData;

	IM_INFOMSG((IM_STR("%s(flag=%d, isCache=%d)"), IM_STR(_IM_FUNC_), flag, pagemem->isCache));
	IM_ASSERT(pagemem != IM_NULL);

	if(pagemem->isCache == IM_FALSE){
		IM_INFOMSG((IM_STR("isCache is false, so don't flush")));
		return IM_RET_OK;
	}

	if(pagemem->mapped == IM_FALSE)
	{
		if(pagemem->linear == IM_TRUE){
			IM_INFOMSG((IM_STR("flush buffer: phyAddr=0x%x, size=%d"), pagemem->phyAddr, PAGE_TO_SIZE(pagemem->totalPageNum)));
			// Flush L1 using virtual address, the entire range in one go.
			// Only flush if user space process has a valid write mapping on given address.
			start_v = pagemem->kVirAddr;
			end_v = (void *)((IM_UINT32)start_v + PAGE_TO_SIZE(pagemem->totalPageNum));
			dmac_flush_range(start_v, end_v);
			IM_INFOMSG((IM_STR("flush L1: start_v=0x%x, end_v=0x%x"), (IM_INT32)start_v, (IM_INT32)end_v));

			// Flush L2 using physical addresses.
			start_p = pagemem->phyAddr;
			end_p = start_p + PAGE_TO_SIZE(pagemem->totalPageNum);
			outer_flush_range(start_p, end_p);
			IM_INFOMSG((IM_STR("flush L2: start_p=0x%x, end_p=0x%x"), start_p, end_p));
		}else{
			for(i=0; i<pagemem->blockNum; i++){
				IM_INFOMSG((IM_STR("flush buffer: phyAddr=0x%x, size=%ld"), pagemem->blocks[i].phyAddr, PAGE_SIZE));
				// Flush L1 using virtual address, the entire range in one go.
				// Only flush if user space process has a valid write mapping on given address.
				start_v = page_address(pagemem->blocks[i].pg);
				end_v = (void *)((IM_INT32)start_v + PAGE_SIZE);
				dmac_flush_range(start_v, end_v);
				IM_INFOMSG((IM_STR("flush L1: start_v=0x%x, end_v=0x%x"), (IM_INT32)start_v, (IM_INT32)end_v));

				// Flush L2 using physical addresses.
				start_p = pagemem->blocks[i].phyAddr;
				end_p = start_p + PAGE_SIZE;
				outer_flush_range(start_p, end_p);
				IM_INFOMSG((IM_STR("flush L2: start_p=0x%x, end_p=0x%x"), start_p, end_p));
			}
		}
	}
	else{
		if(pagemem->linear == IM_TRUE){
			IM_INFOMSG((IM_STR("flush buffer: phyAddr=0x%x, size=%d"), pagemem->phyAddr, PAGE_TO_SIZE(pagemem->totalPageNum)));
			dma_sync_single_for_device(IM_NULL, pagemem->phyAddr, PAGE_TO_SIZE(pagemem->totalPageNum), (flag == ALC_FLUSH_CPU_TO_DEVICE) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		}else{
			for(i=0; i<pagemem->blockNum; i++){
				IM_INFOMSG((IM_STR("flush buffer: phyAddr=0x%x, size=%ld"), pagemem->blocks[i].phyAddr, PAGE_SIZE));
				dma_sync_single_for_device(IM_NULL, pagemem->blocks[i].phyAddr, PAGE_SIZE, (flag == ALC_FLUSH_CPU_TO_DEVICE) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
			}
		}
	}

	return IM_RET_OK;
}

void pmmpwl_free_page_memory(IN void *privData)
{
	pmmpwl_pagemem_t *pagemem = (pmmpwl_pagemem_t *)privData;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(pagemem != IM_NULL);

	if(pagemem->mapped == IM_FALSE){
		if(pagemem->linear == IM_TRUE){
            freeLinearPageMemory(pagemem);
		}else{
			freeNonlinearPageMemory(pagemem);
		}
	}else{
		unmapUserPageMemory(pagemem);
	}

	pmmpwl_free((void *)pagemem);
}


