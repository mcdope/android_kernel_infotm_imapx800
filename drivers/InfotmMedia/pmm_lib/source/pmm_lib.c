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
** v1.2.1	leo@2012/04/05: 
**				1. when pmmlib_dmmu_init() failed, then user call pmmlib_dmmu_deinit(),
**					in this case, we only need return OK, but can not do IM_ASSERT(mm->dmmu != IM_NULL).
**					The same for pmmlib_dmmu_enable/disable().
**				2. fixed memory release bug in pmmlib_deinit().
**				3. when alloc/free buffer which needs update page table, in this case, don't
**					stall/unstall while updating PTE. Because one, IDS don't allow stall/unstall
**					in the duration of the window layer work, second, in our design, when alloc or free
**					a buffer, the buffer we can sure it is not in use, so can safely update the PTE.
** v1.2.2	leo@2012/04/20: 
**				1. fixed pmmlib_mm_del_useraddr_for_statc() dataabort bug.
**				2. in pmmlib_deinit(), when abf is only for statc, then don't call pmmpwl_free_page_memory().
** v1.2.3	leo@2012/04/21: add pmmpwl_init() and pmmpwl_deinit() in pmmlib_init() and pmmlib_deinit() seperately.
** v1.2.4	leo@2012/04/28: when mapped buffer release, must remove devAddr's offset first.
**		leo@2012/05/02: add pmmlib_mm_flush_buffer().
**		leo@2012/05/15: in pmmlib_map_useraddr, put the size to IM_Buffer.size to return to user.
**		leo@2012/05/16: add ALC_FLAG_CACHE support.
** v1.2.7	leo@2012/07/11: add pmmlib_dmmu_reset() function.
** v1.2.8	leo@2012/08/20: add dmmulib_mmu_raw_reset() in pmmlib_dmmu_disable() to ensure devmmu
**              is stop work, meanwhile, add dmmulib_mmu_set_dtable() in pmmlib_dmmu_enable().
** v1.2.9   leo@2012/09/10: add big-memory support.
** v1.2.12	leo@2012/11/30: rewrite bigmem.
**
*****************************************************************************/ 

#include <InfotmMedia.h>
#include <IM_buffallocapi.h>
#include <IM_devmmuapi.h>
#include <dmmu_lib.h>
#include <pmm_lib.h>
#include <pmm_pwl.h>


#define DBGINFO		0
#define DBGWARN		1 
#define DBGERR		1 
#define DBGTIP		1 

#define INFOHEAD	"PMMLIB_I:"
#define WARNHEAD	"PMMLIB_W:"
#define ERRHEAD		"PMMLIB_E:"
#define TIPHEAD		"PMMLIB_T:"

//
//
//
typedef struct{
	dmmu_pagetable_t	handle;
	IM_UINT32		dtablePhyBase;
}pmm_pagetable_t;

#define DMMU_STAT_UNKNOWN	(0)
#define DMMU_STAT_INITED	(1)
#define DMMU_STAT_ENABLED	(2)
typedef struct{
	IM_UINT32		devid;
	IM_INT32		stat;
}pmm_dmmu_t;

typedef struct{
	IM_TCHAR		owner[ALC_OWNER_LEN_MAX];
	im_list_handle_t	abfList;	// list of alc_buffer_t.
	pmm_dmmu_t		*dmmu;	// link to pmm_instance_t->dmmu[x].

    bigmem_handle_t bigmem;
	IM_INT32	bgmBlockSize;
	IM_INT32	bgmBlockNum;
	IM_BOOL		bgmIsCache;
}pmm_mm_t;

typedef struct{
	pmm_pagetable_t 	pageTable;
	pmm_dmmu_t		dmmu[DMMU_DEV_MAX];
	im_mempool_handle_t 	mpl;
	im_list_handle_t	mmlist;	// list of pmm_mm_t.

	IM_INT32		bufferId;	// range: 1--4095
}pmm_instance_t;

//
//
//
static pmm_instance_t *gPmm = IM_NULL;

static IM_INLINE IM_INT32 genBufferId(void)
{
	gPmm->bufferId++;
	if((gPmm->bufferId > 4095) || (gPmm->bufferId == 0)){
		gPmm->bufferId = 1;
	}
	return gPmm->bufferId;
}

//
//
//
#define UPALIGN_SIZE(size)	(((size) + (PMM_PAGE_SIZE - 1)) & ~(PMM_PAGE_SIZE -1))
#define SIZE_TO_PAGE(size)	((size) >> PMM_PAGE_SHIFT)

#define DMMU_ALL_ENABLE_STALL()		\
	for(i=0; i<DMMU_DEV_MAX; i++){	\
		if(gPmm->dmmu[i].stat == DMMU_STAT_ENABLED){	\
			IM_ASSERT(dmmulib_mmu_enable_stall(gPmm->dmmu[i].devid) == IM_RET_OK);	\
		}	\
	}

#define DMMU_ALL_DISABLE_STALL()	\
	for(i=0; i<DMMU_DEV_MAX; i++){	\
		if(gPmm->dmmu[i].stat == DMMU_STAT_ENABLED){	\
			IM_ASSERT(dmmulib_mmu_disable_stall(gPmm->dmmu[i].devid) == IM_RET_OK);	\
		}	\
	}

#define DMMU_ALL_UPDATE_PTABLE(alcBuffer)	\
	for(i=0; i<DMMU_DEV_MAX; i++){	\
		if(gPmm->dmmu[i].stat == DMMU_STAT_ENABLED){	\
			IM_ASSERT(dmmulib_mmu_update_ptable(gPmm->dmmu[i].devid, alcBuffer->devAddr, alcBuffer->pageNum) == IM_RET_OK);	\
			IM_ASSERT(dmmulib_mmu_disable_stall(gPmm->dmmu[i].devid) == IM_RET_OK);	\
		}	\
	}

#define DMMU_ALL_UPDATE_PTABLE_NOSTALL(alcBuffer)	\
	for(i=0; i<DMMU_DEV_MAX; i++){	\
		if(gPmm->dmmu[i].stat == DMMU_STAT_ENABLED){	\
			IM_ASSERT(dmmulib_mmu_update_ptable(gPmm->dmmu[i].devid, alcBuffer->devAddr, alcBuffer->pageNum) == IM_RET_OK);	\
		}	\
	}


IM_RET pmmlib_init(OUT pmm_handle_t *phandle, IN IM_TCHAR *owner)
{
	IM_BOOL first = IM_FALSE;
	pmm_mm_t *mm = IM_NULL;

	IM_ASSERT(PMM_PAGE_SHIFT == DMMU_PAGE_SHIFT);
	IM_ASSERT(PMM_PAGE_SIZE == DMMU_PAGE_SIZE);
	IM_ASSERT(PMM_PAGE_SHIFT == PAGE_SHIFT);
	IM_ASSERT(PMM_PAGE_SIZE == PAGE_SIZE);

	IM_INFOMSG((IM_STR("%s(owner=%s)"), IM_STR(_IM_FUNC_), (owner==IM_NULL)?IM_STR("unknown"):owner));
	IM_ASSERT(phandle != IM_NULL);

	// init pmm instance.
	if(gPmm == IM_NULL){
		IM_INFOMSG((IM_STR("create the global pmm instance")));
		first = IM_TRUE;

		if(pmmpwl_init() != IM_RET_OK){
			IM_ERRMSG((IM_STR("pmmpwl_init() failed")));
			return IM_RET_FAILED;
		}

		gPmm = (pmm_instance_t *)pmmpwl_malloc(sizeof(pmm_instance_t));
		if(gPmm == IM_NULL){
			IM_ERRMSG((IM_STR("malloc(gPmm) failed")));
			goto Fail;
		}
		pmmpwl_memset((void *)gPmm, 0, sizeof(pmm_instance_t));
		gPmm->bufferId = IM_BUFFER_INVALID_ID;

		gPmm->mpl = im_mpool_init((func_mempool_malloc_t)pmmpwl_malloc, (func_mempool_free_t)pmmpwl_free);
		if(gPmm->mpl == IM_NULL){
			IM_ERRMSG((IM_STR("im_mpool_init() failed")));
			goto Fail;
		}

		gPmm->mmlist = im_list_init(0, gPmm->mpl);
		if(gPmm->mmlist == IM_NULL){
			IM_ERRMSG((IM_STR("im_list_init(mmlist) failed")));
			goto Fail;
		}

		if(dmmulib_pagetable_init(&gPmm->pageTable.handle, &gPmm->pageTable.dtablePhyBase) != IM_RET_OK){
			IM_ERRMSG((IM_STR("dmmulib_pagetable_init() failed")));
			goto Fail;
		}
		IM_INFOMSG((IM_STR("dtablePhyBase=0x%x"), gPmm->pageTable.dtablePhyBase));
	}

	// init mm
	mm = (pmm_mm_t *)pmmpwl_malloc(sizeof(pmm_mm_t));
	if(mm == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(mm) failed")));
		goto Fail;
	}
	pmmpwl_memset((void *)mm, 0, sizeof(pmm_mm_t));

	if((owner == IM_NULL) || (pmmpwl_strlen(owner) >= ALC_OWNER_LEN_MAX)){
		pmmpwl_strcpy(mm->owner, IM_STR("unknown"));
	}else{
		pmmpwl_strcpy(mm->owner, owner);
	}
	mm->abfList = im_list_init(sizeof(alc_buffer_t), gPmm->mpl);
	if(mm->abfList == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init(abfList) failed")));
		goto Fail;
	}

	im_list_put_back(gPmm->mmlist, (void *)mm);
	*phandle = (pmm_handle_t)mm;
	return IM_RET_OK;
Fail:
	if(mm != IM_NULL){
		if(mm->abfList != IM_NULL){
			im_list_deinit(mm->abfList);
		}
		pmmpwl_free((void *)mm);
	}

	if(first == IM_TRUE){
		if(gPmm != IM_NULL){
			if(gPmm->pageTable.handle != IM_NULL){
				dmmulib_pagetable_deinit(gPmm->pageTable.handle);
			}
			if(gPmm->mmlist != IM_NULL){
				im_list_deinit(gPmm->mmlist);
			}
			if(gPmm->mpl != IM_NULL){
				im_mpool_deinit(gPmm->mpl);
			}
			pmmpwl_free((void *)gPmm);
			gPmm = IM_NULL;
		}
	}
	return IM_RET_FAILED;
}

IM_RET pmmlib_deinit(IN pmm_handle_t handle)
{
	IM_INT32 i;
	alc_buffer_t *abf;
	pmm_mm_t *mm = (pmm_mm_t *)handle;

	IM_INFOMSG((IM_STR("%s(%s)"), IM_STR(_IM_FUNC_), mm->owner));
	IM_ASSERT(handle != IM_NULL);

	//
	abf = (alc_buffer_t *)im_list_begin(mm->abfList);
	while(abf != IM_NULL){
		IM_INFOMSG((IM_STR("deinit free: owner=%s, vir_addr=0x%x, phy_addr=0x%x, devAddr=0x%x, privData=0x%x, attri=0x%x, bufferId=%d)"), 
			mm->owner, (IM_INT32)abf->buffer.vir_addr, abf->buffer.phy_addr, abf->devAddr, 
			(IM_INT32)abf->privData, abf->attri, IM_BUFFER_GET_ID(abf->buffer)));
		if(abf->attri & ALC_BUFFER_ATTRI_DEVADDR){
			//DMMU_ALL_ENABLE_STALL();
			dmmulib_pagetable_unmap(gPmm->pageTable.handle, abf->devAddr & ~(PMM_PAGE_SIZE - 1), abf->pageNum);
			//DMMU_ALL_UPDATE_PTABLE(abf);
			DMMU_ALL_UPDATE_PTABLE_NOSTALL(abf);
		}
		if((abf->attri & ALC_BUFFER_ATTRI_ALLOCATED) || (abf->attri & ALC_BUFFER_ATTRI_MAPPED)){
            if(abf->attri & ALC_BUFFER_ATTRI_BIGMEM){
                if(mm->bigmem != IM_NULL){
                    pmmpwl_free_bigmem_block(mm->bigmem, abf->privData);
                }
            }else{
			    pmmpwl_free_page_memory(abf->privData);
            }
		}

		abf = (alc_buffer_t *)im_list_erase(mm->abfList, abf);
	}

    if(mm->bigmem != IM_NULL){
        pmmlib_mm_deinit_bigmem(handle);
    }

	if(mm->dmmu != IM_NULL){
		pmmlib_dmmu_deinit(handle);
	}

	//
	im_list_deinit(mm->abfList);
	pmmpwl_free((void *)mm);

	im_list_erase(gPmm->mmlist, (void *)mm);
	if(im_list_empty(gPmm->mmlist)){
		dmmulib_pagetable_deinit(gPmm->pageTable.handle);
		im_list_deinit(gPmm->mmlist);
		im_mpool_deinit(gPmm->mpl);
		pmmpwl_free((void *)gPmm);
		gPmm = IM_NULL;
		pmmpwl_deinit();
		IM_INFOMSG((IM_STR("destroy the global pmm instance")));
	}

	return IM_RET_OK;
}

IM_RET pmmlib_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), devid));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(devid < DMMU_DEV_MAX);

	//
	IM_ASSERT(mm->dmmu == IM_NULL);

	if(gPmm->dmmu[devid].stat != DMMU_STAT_UNKNOWN){
		IM_ERRMSG((IM_STR("the mmu(devid=%d) no resource"), devid));
		return IM_RET_FAILED;
	}

	//
	if(dmmulib_mmu_init(devid) != IM_RET_OK){
		IM_ERRMSG((IM_STR("dmmulib_mmu_init(devid=%d) failed"), devid));
		return IM_RET_FAILED;
	}
	if(dmmulib_mmu_set_dtable(devid, gPmm->pageTable.dtablePhyBase) != IM_RET_OK){
		IM_ERRMSG((IM_STR("dmmulib_mmu_set_dtable(devid=%d) failed"), devid));
		dmmulib_mmu_deinit(devid);
		return IM_RET_FAILED;
	}
	gPmm->dmmu[devid].devid = devid;
	gPmm->dmmu[devid].stat = DMMU_STAT_INITED;

	//
	mm->dmmu = &gPmm->dmmu[devid];

	return IM_RET_OK;
}

IM_RET pmmlib_dmmu_deinit(IN pmm_handle_t handle)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), mm->dmmu->devid));
	IM_ASSERT(handle != IM_NULL);

	if(mm->dmmu == IM_NULL){
		IM_WARNMSG((IM_STR("the dmmu has not been inited")));
		return IM_RET_OK;
	}

	if(mm->dmmu->stat == DMMU_STAT_ENABLED){
		pmmlib_dmmu_disable(handle);
	}

	//
	if(mm->dmmu->stat != DMMU_STAT_UNKNOWN){
		dmmulib_mmu_deinit(mm->dmmu->devid);
		mm->dmmu->stat = DMMU_STAT_UNKNOWN;
	}
	mm->dmmu = IM_NULL;

	return IM_RET_OK;
}

IM_RET pmmlib_dmmu_reset(IN pmm_handle_t handle)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), mm->dmmu->devid));
	IM_ASSERT(handle != IM_NULL);

	if(mm->dmmu == IM_NULL){
		IM_ERRMSG((IM_STR("the dmmu has not been inited")));
		return IM_RET_FAILED;
	}
	IM_ASSERT(mm->dmmu->stat != DMMU_STAT_UNKNOWN);

	if(mm->dmmu->stat == DMMU_STAT_ENABLED){
		if(dmmulib_mmu_disable_paging(mm->dmmu->devid) != IM_RET_OK){
			IM_ERRMSG((IM_STR("dmmulib_mmu_disable_paging(devid=%d) failed"), mm->dmmu->devid));
		}
	}

	if(dmmulib_mmu_raw_reset(mm->dmmu->devid) != IM_RET_OK){
			IM_ERRMSG((IM_STR("dmmulib_mmu_raw_reset(devid=%d) failed"), mm->dmmu->devid));
            return IM_RET_FAILED;
    }

	if(dmmulib_mmu_set_dtable(mm->dmmu->devid, gPmm->pageTable.dtablePhyBase) != IM_RET_OK){
		IM_ERRMSG((IM_STR("dmmulib_mmu_set_dtable(devid=%d) failed"), mm->dmmu->devid));
	}

	mm->dmmu->stat = DMMU_STAT_INITED;

	return IM_RET_OK;
}

IM_RET pmmlib_dmmu_enable(IN pmm_handle_t handle)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), mm->dmmu->devid));
	IM_ASSERT(handle != IM_NULL);
	
	if(mm->dmmu == IM_NULL){
		IM_ERRMSG((IM_STR("the dmmu has not been inited")));
		return IM_RET_FAILED;
	}
	IM_ASSERT(mm->dmmu->stat != DMMU_STAT_UNKNOWN);

	if(mm->dmmu->stat != DMMU_STAT_ENABLED){
        if(dmmulib_mmu_enable_paging(mm->dmmu->devid) != IM_RET_OK){
            IM_ERRMSG((IM_STR("dmmulib_mmu_enable_paging(devid=%d) failed"), mm->dmmu->devid));
            return IM_RET_FAILED;
		}
		mm->dmmu->stat = DMMU_STAT_ENABLED;
	}

	return IM_RET_OK;
}

IM_RET pmmlib_dmmu_disable(IN pmm_handle_t handle)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), mm->dmmu->devid));
	IM_ASSERT(handle != IM_NULL);
	
	if(mm->dmmu == IM_NULL){
		IM_ERRMSG((IM_STR("the dmmu has not been inited")));
		return IM_RET_FAILED;
	}
	IM_ASSERT(mm->dmmu->stat != DMMU_STAT_UNKNOWN);

	if(mm->dmmu->stat == DMMU_STAT_ENABLED){
		if(dmmulib_mmu_disable_paging(mm->dmmu->devid) != IM_RET_OK){
			IM_ERRMSG((IM_STR("dmmulib_mmu_disable_paging(devid=%d) failed"), mm->dmmu->devid));
            if(dmmulib_mmu_raw_reset(mm->dmmu->devid) != IM_RET_OK){
                IM_ERRMSG((IM_STR("dmmulib_mmu_raw_reset(devid=%d) failed"), mm->dmmu->devid));
                return IM_RET_FAILED;
            }
            if(dmmulib_mmu_set_dtable(mm->dmmu->devid, gPmm->pageTable.dtablePhyBase) != IM_RET_OK){
                IM_ERRMSG((IM_STR("dmmulib_mmu_set_dtable(devid=%d) failed"), mm->dmmu->devid));
                return IM_RET_FAILED;
            }
		}
		mm->dmmu->stat = DMMU_STAT_INITED;
	}

	return IM_RET_OK;
}

IM_RET pmmlib_mm_init_bigmem(IN pmm_handle_t handle, IN IM_INT32 blockSize, IN IM_INT32 blockNum, IN IM_BOOL isCache)
{
	IM_INT32 pgnum = SIZE_TO_PAGE(UPALIGN_SIZE(blockSize));
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(%s, blockSize=%d(pgnum=%d), blockNum=%d, isCache=%d)"), IM_STR(_IM_FUNC_), mm->owner, blockSize, pgnum, blockNum, isCache));
	IM_ASSERT(handle != IM_NULL);
    
    if(mm->bigmem != IM_NULL){
        IM_WARNMSG((IM_STR("%s bigmem has been inited"), mm->owner));
        return IM_RET_OK;
    }

    mm->bigmem = pmmpwl_init_bigmem(pgnum, blockNum, isCache);
	if(mm->bigmem == IM_NULL){
        IM_ERRMSG((IM_STR("pmmpwl_init_bigmem(pgnum=%d, blockNum=%d, isCache=%d) failed"), pgnum, blockNum, isCache));
        return IM_RET_FAILED;
    }
	mm->bgmBlockSize = blockSize;
	mm->bgmBlockNum = blockNum;
	mm->bgmIsCache = isCache;
    return IM_RET_OK;
}

IM_RET pmmlib_mm_deinit_bigmem(IN pmm_handle_t handle)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(%s)"), IM_STR(_IM_FUNC_), mm->owner));
	IM_ASSERT(handle != IM_NULL);

    if(mm->bigmem == IM_NULL){
        IM_WARNMSG((IM_STR("%s bigmem is not inited"), mm->owner));
        return IM_RET_OK;
    } 

	if(pmmpwl_deinit_bigmem(mm->bigmem) != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmpwl_deinit_bigmem() failed")));
        return IM_RET_FAILED;
    }
    mm->bigmem = IM_NULL;
    return IM_RET_OK;
}

IM_RET pmmlib_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer)
{
	IM_INT32 i;
	IM_BOOL linear, isCache;
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_UINT32 pageNum, *pageList = IM_NULL;
	dmmu_pt_map_t ptmap;

	IM_INFOMSG((IM_STR("%s(%s, size=%d, flag=0x%x)"), IM_STR(_IM_FUNC_), mm->owner, size, flag));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(alcBuffer != IM_NULL);

	//
    if(flag & ALC_FLAG_BIGMEM){
        if(mm->bigmem == IM_NULL){
            IM_ERRMSG((IM_STR("%s no bigmem to allocate"), mm->owner));
			return IM_RET_FAILED;
        }
		size = mm->bgmBlockSize;
		if(mm->bgmIsCache == IM_TRUE){
			flag |= ALC_FLAG_CACHE;
		}else{
			flag &= ~ALC_FLAG_CACHE;
		}
	}

	//
	size = UPALIGN_SIZE(size);
	pmmpwl_memset((void *)alcBuffer, 0, sizeof(alc_buffer_t));

	// prepare pagelist.
	pageNum = SIZE_TO_PAGE(size);
	pageList = (IM_UINT32 *)im_mpool_alloc(gPmm->mpl, sizeof(IM_UINT32) * pageNum);
	if(pageList == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pageList) failed")));
		goto Fail;
	}

	// alloc page memory.
    if(flag & ALC_FLAG_BIGMEM){
        alcBuffer->privData = pmmpwl_alloc_bigmem_block(mm->bigmem, &alcBuffer->buffer, pageList, pageNum);
        if(alcBuffer->privData == IM_NULL){
            IM_ERRMSG((IM_STR("pmmpwl_alloc_bigmem_block() failed")));
            goto Fail;
        }
        alcBuffer->attri |= ALC_BUFFER_ATTRI_BIGMEM;
    }
    else{
        linear = (flag & ALC_FLAG_PHY_LINEAR_PREFER)?IM_FALSE:IM_TRUE;
        isCache = (flag & ALC_FLAG_CACHE) ? IM_TRUE : IM_FALSE;
        alcBuffer->privData = pmmpwl_alloc_page_memory(pageNum, linear, isCache, &alcBuffer->buffer, pageList);
        if(alcBuffer->privData == IM_NULL){
            IM_ERRMSG((IM_STR("pmmpwl_alloc_page_memory(pageNum=%d, linear=%d, isCache=%d) failed"), pageNum, linear, isCache));
            goto Fail;
        }
    }
    IM_ASSERT(IM_BUFFER_HAS_PHY_FLAG(alcBuffer->buffer));
    alcBuffer->attri |= ALC_BUFFER_ATTRI_ALLOCATED;
	IM_BUFFER_SET_ID(alcBuffer->buffer, genBufferId());
	
	// alloc devAddr and update ptable.
	if(flag & ALC_FLAG_DEVADDR){
		//DMMU_ALL_ENABLE_STALL();
		ptmap.pageNum = pageNum;
		ptmap.pageList = pageList;
		if(dmmulib_pagetable_map(gPmm->pageTable.handle, &ptmap, &alcBuffer->devAddr) != IM_RET_OK){
			IM_ERRMSG((IM_STR("dmmulib_pagetable_map() failed")));
			//DMMU_ALL_DISABLE_STALL();
			goto Fail;
		}
		alcBuffer->pageNum = pageNum;
		//DMMU_ALL_UPDATE_PTABLE(alcBuffer);
		DMMU_ALL_UPDATE_PTABLE_NOSTALL(alcBuffer);

		alcBuffer->attri |= ALC_BUFFER_ATTRI_DEVADDR;
		alcBuffer->buffer.flag |= IM_BUFFER_FLAG_DEVADDR;
	}
	
	IM_ASSERT(im_list_put_back(mm->abfList, (void *)alcBuffer) == IM_RET_OK);

	// free this temporary pagelist.
	im_mpool_free(gPmm->mpl, (void *)pageList);

	IM_INFOMSG((IM_STR("alcBuffer:vir_addr=0x%x, phy_addr=0x%x, size=%d, flag=0x%x, privData=0x%x, devAddr=0x%x, attri=0x%x"),
		(IM_INT32)alcBuffer->buffer.vir_addr, alcBuffer->buffer.phy_addr, alcBuffer->buffer.size, alcBuffer->buffer.flag, 
		(IM_INT32)alcBuffer->privData, alcBuffer->devAddr, alcBuffer->attri));

	return IM_RET_OK;
Fail:
	if(alcBuffer->devAddr != 0){
		dmmulib_pagetable_unmap(gPmm->pageTable.handle, alcBuffer->devAddr, alcBuffer->pageNum);
	}
	if(alcBuffer->privData != IM_NULL){
        if(flag & ALC_FLAG_BIGMEM){
            pmmpwl_free_bigmem_block(mm->bigmem, alcBuffer->privData);
        }else{
		    pmmpwl_free_page_memory(alcBuffer->privData);
        }
	}
	if(pageList != IM_NULL){
		im_mpool_free(gPmm->mpl, (void *)pageList);
	}
	return IM_RET_FAILED;
}

IM_RET pmmlib_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer)
{
	IM_INT32 i;
	alc_buffer_t *abf;
	pmm_mm_t *mm = (pmm_mm_t *)handle;

	IM_INFOMSG((IM_STR("%s(%s, vir_addr=0x%x, phy_addr=0x%x, devAddr=0x%x, privData=0x%x, attri=0x%x, bufferId=%d)"), 
		IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)alcBuffer->buffer.vir_addr, alcBuffer->buffer.phy_addr, alcBuffer->devAddr, 
		(IM_INT32)alcBuffer->privData, alcBuffer->attri, IM_BUFFER_GET_ID(alcBuffer->buffer)));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(alcBuffer != IM_NULL);
	IM_ASSERT(alcBuffer->attri & ALC_BUFFER_ATTRI_ALLOCATED);

	// here, cannot use im_list_erase(alcBuffer), cause this alcBuffer is a contents-copy of allocated previously.
	abf = (alc_buffer_t *)im_list_begin(mm->abfList);
	while(abf != IM_NULL){
		if(abf->privData == alcBuffer->privData){
			im_list_erase(mm->abfList, abf);
			break;
		}
		abf = (alc_buffer_t *)im_list_next(mm->abfList);
	}
	IM_ASSERT(abf != IM_NULL);

	// free the buffer and update ptable.
	if(alcBuffer->attri & ALC_BUFFER_ATTRI_DEVADDR){
		//DMMU_ALL_ENABLE_STALL();
		dmmulib_pagetable_unmap(gPmm->pageTable.handle, alcBuffer->devAddr, alcBuffer->pageNum);
		//DMMU_ALL_UPDATE_PTABLE(abf);
		DMMU_ALL_UPDATE_PTABLE_NOSTALL(abf);
	}

    if(alcBuffer->attri & ALC_BUFFER_ATTRI_BIGMEM){
        if(mm->bigmem != IM_NULL){
            pmmpwl_free_bigmem_block(mm->bigmem, alcBuffer->privData);
        }
    }else{
        pmmpwl_free_page_memory(alcBuffer->privData);
    }

	return IM_RET_OK;
}

IM_RET pmmlib_mm_map_useraddr(IN pmm_handle_t handle, IN void *uVirAddr, IN IM_INT32 size, OUT alc_buffer_t *alcBuffer)
{
	IM_INT32 i, usize = size;
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_UINT32 pageNum, *pageList = IM_NULL, offset;
	dmmu_pt_map_t ptmap;

	IM_INFOMSG((IM_STR("%s(%s, uVirAddr=0x%x, size=%d)"), IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)uVirAddr, size));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(uVirAddr != IM_NULL);
	IM_ASSERT(alcBuffer != IM_NULL);

	//
	offset = (IM_UINT32)uVirAddr & 0xfff;	// assume system page size is 4KB.
	size = UPALIGN_SIZE(size + offset);

	//
	pmmpwl_memset((void *)alcBuffer, 0, sizeof(alc_buffer_t));

	// prepare pagelist.
	pageNum = SIZE_TO_PAGE(size);
	pageList = (IM_UINT32 *)im_mpool_alloc(gPmm->mpl, sizeof(IM_UINT32) * pageNum);
	if(pageList == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pageList) failed")));
		goto Fail;
	}

	// map uVirAddr to page memory.
	alcBuffer->privData = pmmpwl_map_useraddr_to_page_memory((void *)((IM_INT32)uVirAddr & ~0xfff), size, &alcBuffer->buffer, pageList, &pageNum);
	if(alcBuffer->privData == IM_NULL){
		IM_ERRMSG((IM_STR("pmmpwl_map_useraddr_to_page_memory() failed")));
		goto Fail;
	}
	IM_ASSERT(IM_BUFFER_HAS_PHY_FLAG(alcBuffer->buffer));
	IM_ASSERT(alcBuffer->buffer.flag & IM_BUFFER_FLAG_MAPPED);
	alcBuffer->attri |= ALC_BUFFER_ATTRI_MAPPED;
	IM_BUFFER_SET_ID(alcBuffer->buffer, genBufferId());
	alcBuffer->buffer.vir_addr += offset;
	alcBuffer->buffer.phy_addr += offset;
	alcBuffer->buffer.size = usize;

	// alloc devAddr and update ptable.
	//DMMU_ALL_ENABLE_STALL();
	ptmap.pageNum = pageNum;
	ptmap.pageList = pageList;
	if(dmmulib_pagetable_map(gPmm->pageTable.handle, &ptmap, &alcBuffer->devAddr) != IM_RET_OK){
		IM_ERRMSG((IM_STR("dmmulib_pagetable_map() failed")));
		//DMMU_ALL_DISABLE_STALL();
		goto Fail;
	}
	//DMMU_ALL_UPDATE_PTABLE(alcBuffer);
	DMMU_ALL_UPDATE_PTABLE_NOSTALL(alcBuffer);
	alcBuffer->devAddr += offset;
	alcBuffer->pageNum = pageNum;
	alcBuffer->attri |= ALC_BUFFER_ATTRI_DEVADDR;
	alcBuffer->buffer.flag |= IM_BUFFER_FLAG_DEVADDR;
	
	IM_ASSERT(im_list_put_back(mm->abfList, (void *)alcBuffer) == IM_RET_OK);

	// free this temporary pagelist.
	im_mpool_free(gPmm->mpl, (void *)pageList);

	IM_INFOMSG((IM_STR("alcBuffer:vir_addr=0x%x, phy_addr=0x%x, size=%d, flag=0x%x, privData=0x%x, devAddr=0x%x, attri=0x%x"),
		(IM_INT32)alcBuffer->buffer.vir_addr, alcBuffer->buffer.phy_addr, alcBuffer->buffer.size, alcBuffer->buffer.flag, 
		(IM_INT32)alcBuffer->privData, alcBuffer->devAddr, alcBuffer->attri));

	return IM_RET_OK;
Fail:
	if(alcBuffer->devAddr != 0){
		dmmulib_pagetable_unmap(gPmm->pageTable.handle, alcBuffer->devAddr, alcBuffer->pageNum);
	}
	if(alcBuffer->privData != IM_NULL){
		pmmpwl_free_page_memory(alcBuffer->privData);
	}
	if(pageList != IM_NULL){
		im_mpool_free(gPmm->mpl, (void *)pageList);
	}
	return IM_RET_FAILED;
}

IM_RET pmmlib_mm_unmap_useraddr(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer)
{
	IM_INT32 i;
	alc_buffer_t *abf;
	pmm_mm_t *mm = (pmm_mm_t *)handle;

	IM_INFOMSG((IM_STR("%s(%s, uVirAddr=0x%x, phy_addr=0x%x, devAddr=0x%x, privData=0x%x, attri=0x%x, bufferId=%d)"), 
		IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)alcBuffer->buffer.vir_addr, alcBuffer->buffer.phy_addr, alcBuffer->devAddr, 
		(IM_INT32)alcBuffer->privData, alcBuffer->attri, IM_BUFFER_GET_ID(alcBuffer->buffer)));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(alcBuffer != IM_NULL);
	IM_ASSERT(alcBuffer->attri & ALC_BUFFER_ATTRI_MAPPED);
	IM_ASSERT(alcBuffer->buffer.flag & IM_BUFFER_FLAG_MAPPED);

	// here, cannot use im_list_erase(alcBuffer), cause this alcBuffer is a contents-copy of allocated previously.
	abf = (alc_buffer_t *)im_list_begin(mm->abfList);
	while(abf != IM_NULL){
		if(abf->privData == alcBuffer->privData){
			im_list_erase(mm->abfList, abf);
			break;
		}
		abf = (alc_buffer_t *)im_list_next(mm->abfList);
	}
	IM_ASSERT(abf != IM_NULL);

	// free the buffer and update ptable.
	IM_ASSERT(alcBuffer->attri & ALC_BUFFER_ATTRI_DEVADDR);
	//DMMU_ALL_ENABLE_STALL();
	dmmulib_pagetable_unmap(gPmm->pageTable.handle, alcBuffer->devAddr & ~(PMM_PAGE_SIZE - 1), alcBuffer->pageNum);
	//DMMU_ALL_UPDATE_PTABLE(alcBuffer);
	DMMU_ALL_UPDATE_PTABLE_NOSTALL(alcBuffer);

	pmmpwl_free_page_memory(alcBuffer->privData);
	
	return IM_RET_OK;
}

IM_RET pmmlib_mm_flush_buffer(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer, IN IM_INT32 flag)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	IM_INFOMSG((IM_STR("%s(%s, uVirAddr=0x%x, phy_addr=0x%x, devAddr=0x%x, privData=0x%x, attri=0x%x, bufferId=%d, flag=%d)"), 
		IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)alcBuffer->buffer.vir_addr, alcBuffer->buffer.phy_addr, alcBuffer->devAddr, 
		(IM_INT32)alcBuffer->privData, alcBuffer->attri, IM_BUFFER_GET_ID(alcBuffer->buffer), flag));
	return pmmpwl_flush_buffer(alcBuffer->privData, flag);
}

IM_RET pmmlib_mm_add_useraddr_for_statc(IN pmm_handle_t handle, IN void *uVirAddr, IM_INT32 size)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	alc_buffer_t alcBuffer;

	IM_INFOMSG((IM_STR("%s(%s, uVirAddr=0x%x, size=%d)"), IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)uVirAddr, size));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(uVirAddr != IM_NULL);
	
	pmmpwl_memset((void *)&alcBuffer, 0, sizeof(alc_buffer_t));
	alcBuffer.buffer.vir_addr = uVirAddr;
	alcBuffer.buffer.size = size;

	if(im_list_put_back(mm->abfList, (void *)&alcBuffer) != IM_RET_OK){
		IM_ERRMSG((IM_STR("im_list_put_back() failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET pmmlib_mm_del_useraddr_for_statc(IN pmm_handle_t handle, IN void *uVirAddr)
{
	pmm_mm_t *mm = (pmm_mm_t *)handle;
	alc_buffer_t *alcBuffer;

	IM_INFOMSG((IM_STR("%s(%s, uVirAddr=0x%x)"), IM_STR(_IM_FUNC_), mm->owner, (IM_INT32)uVirAddr));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(uVirAddr != IM_NULL);

	alcBuffer = (alc_buffer_t *)im_list_begin(mm->abfList);
	while(alcBuffer != IM_NULL){	// uVirAddr never same in the same process.
		if(alcBuffer->buffer.vir_addr == uVirAddr){
			im_list_erase(mm->abfList, alcBuffer);
			return IM_RET_OK;
		}
		alcBuffer = (alc_buffer_t *)im_list_next(mm->abfList);
	}

	return IM_RET_FAILED;	// not found this uVirAddr.
}

IM_RET pmmlib_statc_get_owner_num(IN pmm_handle_t handle, OUT IM_INT32 *num)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(num != IM_NULL);
	*num = im_list_size(gPmm->mmlist);
	return IM_RET_OK;
}

IM_RET pmmlib_statc_get_owner(IN pmm_handle_t handle, IN IM_INT32 index, OUT IM_TCHAR *owner)
{
	pmm_mm_t *mm;
	IM_INFOMSG((IM_STR("%s(index=%d)"), IM_STR(_IM_FUNC_), index));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(owner != IM_NULL);

	mm = (pmm_mm_t *)im_list_get_index(gPmm->mmlist, index);
	if(mm == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_get_index(index=%d) failed"), index));
		return IM_RET_FAILED;
	}

	pmmpwl_strcpy(owner, mm->owner);
	return IM_RET_OK;
}

IM_RET pmmlib_statc_get_owner_buffer_num(IN pmm_handle_t handle, IN IM_TCHAR *owner, OUT IM_INT32 *num)
{
	pmm_mm_t *mm;
	IM_INFOMSG((IM_STR("%s(owner=%s)"), IM_STR(_IM_FUNC_), owner));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(owner != IM_NULL);
	IM_ASSERT(num != IM_NULL);

	mm = (pmm_mm_t *)im_list_begin(gPmm->mmlist);
	while(mm != IM_NULL){
		if(pmmpwl_strcmp(owner, mm->owner) == 0){
			break;
		}
		mm = (pmm_mm_t *)im_list_next(gPmm->mmlist);
	}

	if(mm == IM_NULL){
		IM_ERRMSG((IM_STR("not found the owner %s"), owner));
		return IM_RET_FAILED;
	}

	//
	*num = im_list_size(mm->abfList);

	return IM_RET_OK;
}

IM_RET pmmlib_statc_get_owner_buffer(IN pmm_handle_t handle, IN IM_TCHAR *owner, IN IM_INT32 index, OUT alc_buffer_t *alcBuffer)
{
	pmm_mm_t *mm;
	alc_buffer_t *abf;
	IM_INFOMSG((IM_STR("%s(owner=%s, index=%d)"), IM_STR(_IM_FUNC_), owner, index));
	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(owner != IM_NULL);
	IM_ASSERT(alcBuffer != IM_NULL);

	mm = (pmm_mm_t *)im_list_begin(gPmm->mmlist);
	while(mm != IM_NULL){
		if(pmmpwl_strcmp(owner, mm->owner) == 0){
			break;
		}
		mm = (pmm_mm_t *)im_list_next(gPmm->mmlist);
	}

	if(mm == IM_NULL){
		IM_ERRMSG((IM_STR("not found the owner %s"), owner));
		return IM_RET_FAILED;
	}

	//
	abf = (alc_buffer_t *)im_list_get_index(mm->abfList, index);
	if(abf == IM_NULL){
		IM_ERRMSG((IM_STR("not found the owner %s index %d alcBuffer"), owner, index));
		return IM_RET_FAILED;
	}
	pmmpwl_memcpy((void *)alcBuffer, (void *)abf, sizeof(alc_buffer_t));

	return IM_RET_OK;
}

alc_buffer_t * pmmlib_getAlcBufferFromPhyAddr(IN pmm_handle_t handle, IN IM_UINT32 phyAddr)
{
	alc_buffer_t *abf;
	pmm_mm_t *mm = (pmm_mm_t *)handle;

	IM_INFOMSG((IM_STR("%s(%s, phyAddr=0x%x)"), IM_STR(_IM_FUNC_), mm->owner, phyAddr));
	IM_ASSERT(handle != IM_NULL);

	//
	abf = (alc_buffer_t *)im_list_begin(mm->abfList);
	while(abf != IM_NULL){
		if(abf->buffer.phy_addr == phyAddr){
			return abf;
		}
		abf = (alc_buffer_t *)im_list_next(mm->abfList);
	}

	return IM_NULL;
}

