/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file wq.c
--
--  Description :
--		
--
--	Author:
--  	Leo Zhang   <leo.zhang@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	leo@2012/03/16: first commit.
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8.
-- v1.0.3	arsor@2012/04/18: mmu test ok and fixed poll timeout bug.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include "g2d_pwl.h"
#include "wq.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"WQ_I:"
#define WARNHEAD	"WQ_W:"
#define ERRHEAD		"WQ_E:"
#define TIPHEAD		"WQ_T:"


/**
 * wq_context_t
 */
#define WQ_FLAG_WORKTHREAD_RUN	0x1

typedef struct{
	IM_INT32 		usrid;
	wq_package_t 	*pkg;
}wq_inpkg_t;

typedef struct _wq_context_t{
	g2dpwl_thread_t		thread;
	func_wq_exec_t		execRoutine;
	g2dpwl_lock_t		threadLock;
	IM_UINT32			flag;	// WQ_FLAG_xxx.

	g2dpwl_lock_t		inqLock;
	g2dpwl_signal_t		inqSig;
	IM_BOOL				inqSigHasSet;
	im_list_handle_t	inpkgQ[WQ_PKG_PRIO_LEVELS];
	IM_INT32			inpkgNum;	// include packages in all levels pkgQ.
	im_mempool_handle_t	mpool;
}wq_context_t;

/**
 * user_instance_t
 */
#define UINST_FLAG_PUTPKG		(1<<0)	// work thread put package which is done in user_inst_put_package.
#define UINST_FLAG_WAITING		(1<<1)	// external wait for out package in user_inst_wait_complete().
#define UINST_FLAG_DESTROYED	(1<<2)	// user_inst_destroy() has been called.


typedef struct{
	IM_UINT32			inpkgNum;		//number of package has been put to input package queue 
	//used for sync call
	wq_package_t 		*syncPkg;
	g2dpwl_signal_t		syncSig;

	im_list_handle_t	outq;
	g2dpwl_signal_t		outqSig;

	g2dpwl_lock_t		outLock;
	IM_UINT32			flag;	// UINST_FLAG_xxx.

	func_free_package	freePackage;
}user_inst_t;

static user_inst_t* user_inst_create(wq_context_t *wq, func_free_package freePackage)
{
	user_inst_t *uinst = IM_NULL;
	
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	uinst = (user_inst_t *)g2dpwl_malloc(sizeof(user_inst_t));
	if(uinst == IM_NULL){
		IM_ERRMSG((IM_STR("g2dpwl_malloc(uinst) failed")));
		return IM_NULL;
	}
	g2dpwl_memset((void *)uinst, 0, sizeof(user_inst_t));

	uinst->outq = im_list_init(0, wq->mpool);//not need to copy data in list
	if(uinst->outq == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init() failed")));
		g2dpwl_free((void *)uinst);
		return IM_NULL;
	}

	//sync call singal
	g2dpwl_sig_init(&(uinst->syncSig), IM_FALSE);
	IM_INFOMSG((IM_STR("%s(inst create:uinst=0x%x, uinst->syncSig=0x%x)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, (IM_INT32)uinst->syncSig));

	g2dpwl_sig_init(&(uinst->outqSig), IM_FALSE);

	g2dpwl_lock_init(&(uinst->outLock));

	uinst->inpkgNum = 0;
	uinst->syncPkg = IM_NULL;
	uinst->flag = 0;
	uinst->freePackage = freePackage;

	IM_INFOMSG((IM_STR("%s(uinst=0x%x)--"), IM_STR(_IM_FUNC_), (int)uinst));
	return uinst;
}

static IM_RET user_inst_destroy(user_inst_t *uinst, wq_context_t *wq)
{
	wq_package_t *pkg;
	wq_inpkg_t *inpkg;
	IM_INT32 i;
	IM_ASSERT(uinst != IM_NULL);
	IM_INFOMSG((IM_STR("%s(!!!!!!!!!!!!!!!!!!!!!uinst=0x%x, uinst->inpkgNum=%d)++"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, uinst->inpkgNum));

	//free inqueue package of this uinst
	g2dpwl_lock(wq->inqLock);
	if(uinst->inpkgNum > 0){
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----1"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
		for(i=0; i<WQ_PKG_PRIO_LEVELS; i++){
			inpkg = (wq_inpkg_t *)im_list_begin(wq->inpkgQ[i]);
			while((inpkg != IM_NULL) && (uinst->inpkgNum > 0)){
				IM_ASSERT(inpkg->pkg != IM_NULL);
				if(inpkg->usrid == (IM_INT32)uinst){
					IM_INFOMSG((IM_STR("%s(uinst=0x%x)----2"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
					uinst->freePackage((wq_user_t)uinst, inpkg->pkg);
					inpkg = (wq_inpkg_t *)im_list_erase(wq->inpkgQ[i], inpkg);
					uinst->inpkgNum--;
					wq->inpkgNum--; 
				}else{
					inpkg = (wq_inpkg_t *)im_list_next(wq->inpkgQ[i]);
				}
			}
			if(uinst->inpkgNum == 0){
				break;
			}
		}
	}
	g2dpwl_unlock(wq->inqLock);

	//free outqueue package
	g2dpwl_lock(uinst->outLock);
	if(uinst->freePackage != IM_NULL){
		i = im_list_begin(uinst->outq);
		IM_INFOMSG((IM_STR("%s(!!!!!!!!!!!!!!!uinst=0x%x, outpkgNum=%d)----3"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, i));
		pkg = (wq_package_t *)im_list_begin(uinst->outq);
		while(pkg != IM_NULL){
			//IM_INFOMSG((IM_STR("freeOutqPackage(uinst=0x%x, pkgid=%d)"), (IM_INT32)uinst, pkg->pkgid));
			uinst->freePackage((wq_user_t)uinst, pkg);
			pkg = (wq_package_t *)im_list_erase(uinst->outq, (void *)pkg);
		}
		//sync pkg free
		if(uinst->syncPkg != IM_NULL){
			pkg = uinst->syncPkg; 
			uinst->freePackage((wq_user_t)uinst, pkg);
			uinst->syncPkg = IM_NULL;
		}
	}
	im_list_deinit(uinst->outq);
	uinst->flag |= UINST_FLAG_DESTROYED;
	IM_INFOMSG((IM_STR("%s(inst destroy: uinst=0x%x, uinst->syncSig=0x%x)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, (IM_INT32)uinst->syncSig));
	g2dpwl_sig_set(uinst->syncSig);
	g2dpwl_sig_set(uinst->outqSig);
	g2dpwl_unlock(uinst->outLock);

	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----4"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
	//free uinst resource until uinst not used in other thread(that means uinst put and wait outPkg over)
	while((uinst->flag&UINST_FLAG_PUTPKG) || (uinst->flag&UINST_FLAG_WAITING)){
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----5"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
		msleep(1);
	}
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----6"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
	g2dpwl_sig_deinit(uinst->syncSig);
	g2dpwl_sig_deinit(uinst->outqSig);
	g2dpwl_lock_deinit(uinst->outLock);
	g2dpwl_free(uinst);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

static IM_RET user_inst_put_package(user_inst_t *uinst, wq_package_t *pkg)
{
	IM_ASSERT(uinst != IM_NULL);
	
	IM_INFOMSG((IM_STR("%s(uinst=0x%x, pkgid=%d)++"), IM_STR(_IM_FUNC_),(IM_INT32)uinst, pkg->pkgid));
	
	g2dpwl_lock(uinst->outLock);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----1"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	if(uinst->flag & UINST_FLAG_DESTROYED){
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----2"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
		uinst->freePackage((wq_user_t)uinst, pkg);
		g2dpwl_unlock(uinst->outLock);
		return IM_RET_FALSE;
	}
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----3"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
		//used for sync call
	if(pkg->prio == WQ_PKG_PRIO_SYNC){
		uinst->syncPkg = pkg;
		g2dpwl_sig_set(uinst->syncSig);
	}else{
		IM_INFOMSG((IM_STR("****************put to outq_lst=0x%x"), (int)&(uinst->outq)));
		im_list_put_back(uinst->outq, (void *)pkg);	
		IM_INFOMSG((IM_STR("****************pkgid=%d"), pkg->pkgid));
		if(im_list_size(uinst->outq) == 1){
			g2dpwl_sig_set(uinst->outqSig);
		}
	}
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----4"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	g2dpwl_unlock(uinst->outLock);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----5"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	uinst->flag &= ~UINST_FLAG_PUTPKG;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

static IM_RET user_inst_cancel_package(user_inst_t *uinst, wq_package_t *pkg)
{
	IM_ASSERT(uinst != IM_NULL);
	
	IM_INFOMSG((IM_STR("%s(uinst=0x%x, pkgid=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, pkg->pkgid));
	
	if(uinst->freePackage != IM_NULL){
		uinst->freePackage((wq_user_t)uinst, pkg);
	}

	return IM_RET_OK;
}

static IM_RET user_inst_wait_complete(user_inst_t *uinst, IM_INT32 type, wq_package_t **pkg)
{
	wq_package_t *lpkg = IM_NULL;
	IM_RET ret;
	IM_ASSERT(uinst != IM_NULL);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)++"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));

	if(uinst == IM_NULL){
		IM_WARNMSG((IM_STR("%s(), uinst for this usrid has been destroyed!"), IM_STR(_IM_FUNC_)));
		return IM_RET_FALSE;
	}
		
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----1"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	g2dpwl_lock(uinst->outLock);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----2"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	uinst->flag |= UINST_FLAG_WAITING;
	if(type == WQ_WAIT_SYNC){//SYNC WAIT
		IM_INFOMSG((IM_STR("%s(inst wait complete: uinst=0x%x, uinst->syncSig=0x%x)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, (IM_INT32)uinst->syncSig));
		ret = g2dpwl_sig_wait(uinst->syncSig, &uinst->outLock, -1/*600000*//*us*/);
		IM_INFOMSG((IM_STR("(@@@@wait signal success!!!)")));
		if(ret != IM_RET_OK){
			uinst->flag &= ~UINST_FLAG_WAITING;
			g2dpwl_unlock(uinst->outLock);
			return ret;
		}
		IM_INFOMSG((IM_STR("%s()------1"), IM_STR(_IM_FUNC_)));
		if(uinst->flag & UINST_FLAG_DESTROYED){
			uinst->flag &= ~UINST_FLAG_WAITING;
			g2dpwl_unlock(uinst->outLock);
			return IM_RET_FAILED;
		}
		IM_INFOMSG((IM_STR("%s()------2"), IM_STR(_IM_FUNC_)));
		*pkg = uinst->syncPkg;
		uinst->syncPkg = IM_NULL;
	}else{
		/***************************************************************************
		* list_size = 0:								  						   *	
		*  out package queue is empty, we must wait new output package signal.	   *
		*																		   *
		* list_size = 1:														   *
		*  It means output package signal has been up, we must down this signal    *
		*  using g2dpwl_sig_wait in order to use this signal again. 			   *
		***************************************************************************/
		if(im_list_size(uinst->outq) <= 1){
			IM_INFOMSG((IM_STR("%s(inst wait complete: uinst=0x%x, uinst->outqSig=0x%x)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, (IM_INT32)uinst->outqSig));
			if(g2dpwl_sig_wait(uinst->outqSig, &uinst->outLock, -1) != IM_RET_OK){
				uinst->flag &= ~UINST_FLAG_WAITING;
				g2dpwl_unlock(uinst->outLock);
				return IM_RET_FAILED;
			}
		}
		if(uinst->flag & UINST_FLAG_DESTROYED){
			uinst->flag &= ~UINST_FLAG_WAITING;
			g2dpwl_unlock(uinst->outLock);
			return IM_RET_FALSE;
		}else{
			IM_INFOMSG((IM_STR("################get from outq_lst=0x%x"), (int)&uinst->outq));
			lpkg = (wq_package_t *)im_list_begin(uinst->outq);
			*pkg = lpkg;
			im_list_erase(uinst->outq, (void *)lpkg);
			IM_INFOMSG((IM_STR("################id=%d"), lpkg->pkgid));
			IM_INFOMSG((IM_STR("complete, uinst=0x%x, pkgid=%d"), (IM_INT32)uinst, lpkg->pkgid));
		}
	}
	uinst->flag &= ~UINST_FLAG_WAITING;
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)----3"), IM_STR(_IM_FUNC_),(IM_INT32)uinst));
	g2dpwl_unlock(uinst->outLock);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}


//==============================================================================================

static void work_thread_entry(void *data)
{
	IM_RET ret;
	IM_INT32 i;
	wq_inpkg_t *inpkg;
	wq_package_t *pkg = IM_NULL;
	wq_context_t *wq = (wq_context_t *)data;
	user_inst_t *uinst = IM_NULL ;

	IM_INFOMSG((IM_STR("++++++%s()++++++"), IM_STR(_IM_FUNC_)));

	g2dpwl_lock(wq->threadLock);
	wq->flag |= WQ_FLAG_WORKTHREAD_RUN;
	g2dpwl_unlock(wq->threadLock);

	do{
		//
		g2dpwl_lock(wq->inqLock);
		/***************************************************************************
		* inpkgNum = 0(two conditions):											   *	
		*  1. input package queue is empty, we must wait new input package signal, *
		*  2. task has been completed, we must wait ending signal.				   *
		*																		   *
		* inqSigHasSet = IM_TRUE:															   *
		*  It means input package signal has been up, we must down this signal     *
		*  using g2dpwl_sig_wait in order to use this signal again. 			   *
		***************************************************************************/
		if((wq->inpkgNum == 0) || (wq->inqSigHasSet == IM_TRUE)){
			IM_INFOMSG((IM_STR("%s(wait inqueue signal:inqSig=0x%x, inNum=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)wq->inqSig, wq->inpkgNum));
			ret = g2dpwl_sig_wait(wq->inqSig, &wq->inqLock, -1);
			if((ret != IM_RET_OK) || (wq->inpkgNum == 0)){	// indicate quit this loop.
				if(ret != IM_RET_OK){
					IM_ERRMSG((IM_STR("g2dpwl_sig_wait(wq->inqSig) failed, ret=%d"), ret));
				}
				g2dpwl_unlock(wq->inqLock);
				break;
			}
			wq->inqSigHasSet = IM_FALSE;
		}
		for(i=0; i<WQ_PKG_PRIO_LEVELS; i++){	// get the highest priority pkg to execute first.
			IM_INFOMSG((IM_STR("@@@@@@@@@@@@@@@@get from inpkgQ_lst=0x%x"), (int)&wq->inpkgQ[i]));
			inpkg = (wq_inpkg_t *)im_list_begin(wq->inpkgQ[i]);
			if(inpkg != IM_NULL){
				IM_ASSERT(inpkg->pkg != IM_NULL);
				uinst = (user_inst_t *)inpkg->usrid;
				IM_INFOMSG((IM_STR("%s(uinst=0x%x)----1"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
				uinst->flag |= UINST_FLAG_PUTPKG;
				pkg = inpkg->pkg;
				break;
			}
		}
		im_list_erase(wq->inpkgQ[i], (void *)inpkg);
		uinst->inpkgNum--;
		wq->inpkgNum--;
		g2dpwl_unlock(wq->inqLock);
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----2"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
		IM_INFOMSG((IM_STR("######dequeue package: num=%d"), wq->inpkgNum));

		//
		IM_ASSERT(pkg != IM_NULL);
		IM_INFOMSG((IM_STR("exec(uinst=0x%x, pkgid=%d)"), (IM_INT32)uinst, pkg->pkgid));
		pkg->ret = wq->execRoutine(pkg->userData);
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----3"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
		//
		user_inst_put_package(uinst, pkg);
		IM_INFOMSG((IM_STR("%s(uinst=0x%x)----4"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
		uinst->flag &= ~UINST_FLAG_PUTPKG;
		g2dpwl_msleep(1);//add this for cpu can change to other thread 
	}while(1);

	g2dpwl_lock(wq->threadLock);
	wq->flag &= ~WQ_FLAG_WORKTHREAD_RUN;
	g2dpwl_unlock(wq->threadLock);

	IM_INFOMSG((IM_STR("------%s()------"), IM_STR(_IM_FUNC_)));
	return;	
}

IM_RET wq_init(OUT wq_handle_t *handle, IN func_wq_exec_t func)
{	
	IM_INT32 i;
	wq_context_t *wq;

	IM_ASSERT(handle != IM_NULL);
	IM_ASSERT(func != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	// allocate wq context.
	wq = (wq_context_t *)g2dpwl_malloc(sizeof(wq_context_t));
	if(wq == IM_NULL){
		IM_ERRMSG((IM_STR("g2dpwl_malloc(wq_context_t) failed!")));
		return IM_RET_NOMEMORY;
	}
	g2dpwl_memset((void *)wq, 0, sizeof(wq_context_t));

	wq->mpool = im_mpool_init((func_mempool_malloc_t)g2dpwl_malloc, (func_mempool_free_t)g2dpwl_free);
	if(wq->mpool == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed!")));
		goto Fail;
	}

	// initialize all input package queue.
	for(i=0; i<WQ_PKG_PRIO_LEVELS; i++){
		IM_JIF((wq->inpkgQ[i] = im_list_init(sizeof(wq_inpkg_t), wq->mpool)) != IM_NULL);
	}
	wq->inpkgNum = 0;

	IM_JIF(g2dpwl_lock_init(&wq->inqLock));
	//IM_JIF(g2dpwl_sig_init(&wq->inqSig, IM_TRUE));
	IM_JIF(g2dpwl_sig_init(&wq->inqSig, IM_FALSE));
	wq->inqSigHasSet = IM_FALSE;

	IM_JIF(g2dpwl_lock_init(&wq->threadLock));
	wq->execRoutine = func;
	wq->flag = 0;
	IM_JIF(g2dpwl_thread_init(&wq->thread, (g2dpwl_func_thread_entry_t)work_thread_entry, (void *)wq));
	do{ // wait the work thread start run.
		g2dpwl_lock(wq->threadLock);
		if(wq->flag & WQ_FLAG_WORKTHREAD_RUN){
			g2dpwl_unlock(wq->threadLock);
			break;
		}else{
			g2dpwl_unlock(wq->threadLock);
			g2dpwl_msleep(10);
		}
	}while(1);
	*handle = (wq_handle_t)wq;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
Fail:
	wq_deinit((wq_handle_t)wq);
	return IM_RET_FAILED;
}

IM_RET wq_deinit(IN wq_handle_t handle)
{
	IM_INT32 i;
	wq_context_t *wq = (wq_context_t *)handle;
	
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	if(wq->thread != IM_NULL){// to indicate the work thread quit.
		g2dpwl_lock(wq->inqLock);
		wq->inpkgNum = 0;
		g2dpwl_sig_set(wq->inqSig/*wq->inpkgQ[0]*/);
		g2dpwl_unlock(wq->inqLock);

		while(1){	// wait the work thread quit first.
			g2dpwl_lock(wq->threadLock);
			if(!(wq->flag & WQ_FLAG_WORKTHREAD_RUN)){
				g2dpwl_unlock(wq->threadLock);
				break;
			}else{
				g2dpwl_unlock(wq->threadLock);
				g2dpwl_msleep(10);
			}
		}
		g2dpwl_thread_deinit(wq->thread);
	}
	if(wq->threadLock != IM_NULL){
		g2dpwl_lock_deinit(wq->threadLock);
	}
	if(wq->inqLock != IM_NULL){
		g2dpwl_lock_deinit(wq->inqLock);
	}
	if(wq->inqSig != IM_NULL){
		g2dpwl_sig_deinit(wq->inqSig);
	}
	for(i=0; i<WQ_PKG_PRIO_LEVELS; i++){
		im_list_deinit(wq->inpkgQ[i]);
	}

	im_mpool_deinit(wq->mpool);

	g2dpwl_free((void *)wq);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET wq_init_user(IN wq_handle_t handle, OUT wq_user_t *usr, IN func_free_package freePackage)
{
	user_inst_t *uinst = IM_NULL;
	wq_context_t *wq = (wq_context_t *)handle;
	IM_ASSERT(wq != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	uinst = user_inst_create(wq, freePackage);
	*usr = (wq_user_t)uinst;
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)--"), IM_STR(_IM_FUNC_), (IM_INT32)uinst));
	return IM_RET_OK;
}

IM_RET wq_deinit_user(IN wq_handle_t handle, IN wq_user_t usr)
{
	wq_context_t *wq = (wq_context_t *)handle;
	user_inst_t *uinst = (user_inst_t *)usr;
	IM_ASSERT(wq != IM_NULL);
	IM_ASSERT(uinst != IM_NULL);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x)"), IM_STR(_IM_FUNC_), (int)uinst));

	return user_inst_destroy(uinst, wq);
}

IM_RET wq_put_package(IN wq_handle_t handle, IN wq_user_t usr, IN wq_package_t *pkg)
{
	wq_inpkg_t inpkg;
	wq_context_t *wq = (wq_context_t *)handle;
	user_inst_t *uinst = (user_inst_t *)usr;
	
	IM_ASSERT(wq != IM_NULL);
	IM_ASSERT(uinst != IM_NULL);
	IM_ASSERT(pkg != IM_NULL);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x, pkgid=%d)++"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, pkg->pkgid));

	g2dpwl_lock(wq->inqLock);
	IM_INFOMSG((IM_STR("================put to inpkgQ_lst=0x%x"), (int)&wq->inpkgQ[pkg->prio]));
	inpkg.usrid = (IM_INT32)uinst;
	inpkg.pkg = pkg;
	im_list_put_back(wq->inpkgQ[pkg->prio], (void *)&inpkg);
	IM_INFOMSG((IM_STR("================usrid=0x%x"), inpkg.usrid));
	uinst->inpkgNum++;

	if(wq->inpkgNum++ == 0){
		IM_INFOMSG((IM_STR("(set inqueue signal:inqSig=0x%x, inNum=%d)"), (IM_INT32)wq->inqSig, wq->inpkgNum));
		g2dpwl_sig_set(wq->inqSig);
		wq->inqSigHasSet = IM_TRUE;
	}
	g2dpwl_unlock(wq->inqLock);

	IM_INFOMSG((IM_STR("######put package to inqueue: num=%d"), wq->inpkgNum));

	IM_INFOMSG((IM_STR("%s(uinst=0x%x, pkgid=%d)--"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, pkg->pkgid));
	return IM_RET_OK;
}

IM_RET wq_cancel_package(IN wq_handle_t handle, IN wq_user_t usr, IN IM_INT32 pkgid)
{
	wq_inpkg_t *inpkg;
	wq_package_t *pkg;
	IM_INT32 i;
	wq_context_t *wq = (wq_context_t *)handle;
	user_inst_t *uinst = (user_inst_t *)usr;

	IM_ASSERT(wq != IM_NULL);
	IM_ASSERT(uinst != IM_NULL);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x, pkgid=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, pkgid));

	g2dpwl_lock(wq->inqLock);
	for(i=0; i<WQ_PKG_PRIO_LEVELS; i++){
		inpkg = (wq_inpkg_t *)im_list_begin(wq->inpkgQ[i]);
		
		while(inpkg != IM_NULL){
			IM_ASSERT(inpkg->pkg != IM_NULL);
			pkg = inpkg->pkg;
			if((inpkg->usrid == (IM_INT32)uinst) && ((pkgid == -1) || (pkg->pkgid == pkgid))){
				user_inst_cancel_package(uinst, pkg);
				inpkg = (wq_inpkg_t *)im_list_erase(wq->inpkgQ[i], inpkg);
				uinst->inpkgNum--;
				if(wq->inpkgNum-- == 1){
					g2dpwl_sig_reset(wq->inqSig);
				}
				if(pkgid != -1){	// only this pkgid will be cancel.
					break;
				}
			}else{
				inpkg = (wq_inpkg_t *)im_list_next(wq->inpkgQ[i]);
			}
		}
	}
	g2dpwl_unlock(wq->inqLock);

	return IM_RET_OK;
}

IM_RET wq_wait_complete(IN wq_handle_t handle, IN wq_user_t usr, IM_INT32 type, OUT wq_package_t **pkg)
{
	wq_context_t *wq = (wq_context_t *)handle;
	user_inst_t *uinst = (user_inst_t *)usr;
	IM_ASSERT(wq != IM_NULL);
	IM_ASSERT(uinst != IM_NULL);
	IM_INFOMSG((IM_STR("%s(uinst=0x%x, type=%d(0:sync, 1:async))"), IM_STR(_IM_FUNC_), (IM_INT32)uinst, type));
	return user_inst_wait_complete(uinst, type, pkg);
}


