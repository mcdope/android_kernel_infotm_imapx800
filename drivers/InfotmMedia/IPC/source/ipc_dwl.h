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
** v1.0.1	leo@2012/04/27: first commit.
**
*****************************************************************************/ 

#ifndef __IPC_DWL_H__
#define __IPC_DWL_H__

#if (TARGET_SYSTEM == FS_ANDROID)
typedef struct{
	// for all.
	IM_INT32	fd;

	// for shm.
	IM_INT32	shmSize;
	void *		shmVirAddr;
	IM_UINT32	shmPhyAddr;
}ipc_dwl_t;
#endif


ipc_dwl_t *ipcdwl_sync_init(IM_TCHAR *keyStr);
IM_RET ipcdwl_sync_deinit(ipc_dwl_t *dwl);
IM_RET ipcdwl_sync_set(ipc_dwl_t *dwl);
IM_RET ipcdwl_sync_reset(ipc_dwl_t *dwl);
IM_RET ipcdwl_sync_wait(ipc_dwl_t *dwl, IM_INT32 timeout);

ipc_dwl_t *ipcdwl_shm_init(IM_TCHAR *keyStr, IM_INT32 size);
IM_RET ipcdwl_shm_deinit(ipc_dwl_t *dwl);
IM_RET ipcdwl_shm_lock(ipc_dwl_t *dwl, void **buffer, IM_INT32 timeout);
IM_RET ipcdwl_shm_unlock(ipc_dwl_t *dwl);

ipc_dwl_t *ipcdwl_pipe_init(IM_TCHAR *keyStr, IM_INT32 usage, IM_INT32 size);
IM_RET ipcdwl_pipe_deinit(ipc_dwl_t *dwl);
IM_RET ipcdwl_pipe_write(ipc_dwl_t *dwl, void *data, IM_INT32 size);
IM_RET ipcdwl_pipe_read(ipc_dwl_t *dwl, void *data, IM_INT32 size, IM_INT32 timeout);


#endif	// __IPC_DWL_H__

