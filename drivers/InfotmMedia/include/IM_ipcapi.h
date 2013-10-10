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
** v1.0.1	leo@2012/04/27: first commit. features list:
**			1. support sync, pipe, shm;
**			2. only support one-process to one-process, cannot support one-to-more or more-to-one;
**
*****************************************************************************/ 

#ifndef __IM_IPCAPI_H__
#define __IM_IPCAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef IPCAPI_EXPORTS
	#define IPC_API		__declspec(dllexport)	/* For dll lib */
#else
	#define IPC_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define IPC_API
#endif	
/*############################################################################*/

//
//
//
typedef void *	IPCSYNC_HANDLE;
typedef void *	IPCSHM_HANDLE;
typedef void *	IPCPIPE_HANDLE;

//
//
//
#define IPC_TYPE_SYNC	(1)
#define IPC_TYPE_SHM	(2)
#define IPC_TYPE_PIPE	(3)

//
#define IPC_PIPE_USAGE_READ	(0x1)
#define IPC_PIPE_USAGE_WRITE	(0x2)
#define IPC_PIPE_USAGE_SINGLE	(0x10)

//
#define IPC_KEYSTR_MAX_LEN	(32)


IPC_API IM_UINT32 ipc_version(IM_TCHAR *ver_string);

IPC_API IM_RET ipcsync_init(OUT IPCSYNC_HANDLE *handle, IN IM_TCHAR *keyStr);
IPC_API IM_RET ipcsync_deinit(IN IPCSYNC_HANDLE handle);
IPC_API IM_RET ipcsync_set(IN IPCSYNC_HANDLE handle);
IPC_API IM_RET ipcsync_reset(IN IPCSYNC_HANDLE handle);
IPC_API IM_RET ipcsync_wait(IN IPCSYNC_HANDLE handle, IN IM_INT32 timeout);

IPC_API IM_RET ipcshm_init(OUT IPCSHM_HANDLE *handle, IN IM_TCHAR *keyStr, IN IM_INT32 size);
IPC_API IM_RET ipcshm_deinit(IN IPCSHM_HANDLE handle);
IPC_API IM_RET ipcshm_lock(IN IPCSHM_HANDLE handle, OUT void **buffer, IN IM_INT32 timeout);
IPC_API IM_RET ipcshm_unlock(IN IPCSHM_HANDLE handle);

IPC_API IM_RET ipcpipe_init(OUT IPCPIPE_HANDLE *handle, IN IM_TCHAR *keyStr, IN IM_INT32 usage, IN IM_INT32 size);
IPC_API IM_RET ipcpipe_deinit(IN IPCPIPE_HANDLE handle);
IPC_API IM_RET ipcpipe_read(IN IPCPIPE_HANDLE handle, OUT void *data, IN IM_INT32 size, IN IM_INT32 timeout);
IPC_API IM_RET ipcpipe_write(IN IPCPIPE_HANDLE handle, IN void *data, IN IM_INT32 size);


/*############################################################################*/
#ifdef __cplusplus
}
#endif

#endif	// __IM_IPCAPI_H__
