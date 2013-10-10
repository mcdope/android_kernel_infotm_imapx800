/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file wq.h
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

#ifndef __WQ_H__
#define __WQ_H__

/*
 * keep priority number macro defined from high to low(2 levels now).
 */
//#define WQ_PKG_PRIO_LEVELS	3
#define WQ_PKG_PRIO_LEVELS	2

#define WQ_PKG_PRIO_SYNC		0//the most high level, used for sync call
#define WQ_PKG_PRIO_CRITICAL	1
#if(WQ_PKG_PRIO_LEVELS == 3)
#define WQ_PKG_PRIO_NORMAL		2
#endif

#define WQ_WAIT_SYNC	0
#define WQ_WAIT_ASYNC	1

typedef struct{
	IM_INT32	pkgid;	// from 0x0 to 0x7fffffff.
	IM_INT32	prio;	// WQ_PKG_PRIO_xxx.
	void *		userData;	// will deliver to func_wq_exec().
	IM_RET		ret;	// returned by func_wq_exec().
}wq_package_t;

typedef void *	wq_handle_t;
typedef void *	wq_user_t;
typedef IM_RET (*func_wq_exec_t)(void *userData);

typedef void (*func_free_package)(wq_user_t usr, wq_package_t *pkg);

IM_RET wq_init(OUT wq_handle_t *handle, IN func_wq_exec_t func);
IM_RET wq_deinit(IN wq_handle_t handle);
IM_RET wq_init_user(IN wq_handle_t handle, OUT wq_user_t *usr , IN func_free_package freePackage);
IM_RET wq_deinit_user(IN wq_handle_t handle, IN wq_user_t usr);
IM_RET wq_put_package(IN wq_handle_t handle, IN wq_user_t usr, IN wq_package_t *pkg);
IM_RET wq_cancel_package(IN wq_handle_t handle, IN wq_user_t usr, IN IM_INT32 pkgid);	// if pkgid is -1, will cancel all pkgs belong to the usrid.
IM_RET wq_wait_complete(IN wq_handle_t handle, IN wq_user_t usr, IM_INT32 type, OUT wq_package_t **pkg);//type = WQ_WAIT_XXX


#endif	// __WQ_H__


