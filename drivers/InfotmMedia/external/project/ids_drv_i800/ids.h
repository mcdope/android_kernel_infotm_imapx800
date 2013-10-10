/*
 * ids.h
 *
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Author:
 *	Sam<weize_ye@infotm.com>.
 *
 * Revision History:
 *
 * v1.0.7	leo@2012/08/11: readjust.
 */


#ifndef __IDS_H__
#define __IDS_H__


typedef struct{
	IM_INT32		type;	// IDSLIB_INST_xxx.
	IM_INT32		idsx;
	IM_INT32		wx;
	IM_INT32 		pixfmt;
	void *			vmhandle;
    void *          wlhandle;
    void *          extlhandle;
    void *          iehandle;
	struct mutex		lock;
	wait_queue_head_t	waitQ;
	im_list_handle_t	evtlst; // list of idslib_vmode_event_t.
	
	IM_BOOL			gotAcslock; // recored if has got lock, if yes, the inst should do unlock when release().
}idsdrv_instance_t;

typedef struct{
	IM_INT32 			extSwpDone;
	struct semaphore	extSwpSem; // extend fblayer swap buffer finished signal.
}idsdrv_fblayer_t;

typedef struct{
	idslib_init_config_t	initcfg;
	struct mutex		acslock;	// serial access lock.
	im_mempool_handle_t	mpl;
	im_list_handle_t	instlst;	// list of idsdrv_instance_t.
	
	idsdrv_fblayer_t	fbl;
	struct early_suspend	elySuspnd;
    IM_BOOL             suspended;

	bool	disWaitPostVsync;
	bool	waitVsync;
	struct completion comple;
}idsdrv_global_t;

#define IDSDRV_IOCTL_ENABLE_WAIT_POST_VSYNC		0x90000
#define IDSDRV_IOCTL_DISABLE_WAIT_POST_VSYNC	0x90001


#endif  /* __IDS_H__ */

