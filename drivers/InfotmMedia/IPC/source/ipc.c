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
** v1.0.1	leo@2012/04/28: first commit.
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_ipcapi.h>
#include <ipc_dwl.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IPC_I:"
#define WARNHEAD	"IPC_W:"
#define ERRHEAD		"IPC_E:"
#define TIPHEAD		"IPC_T:"

//
//
//
#define VER_MAJOR	1
#define VER_MINOR	0
#define VER_PATCH	1
#define VER_STRING	"1.0.1"

typedef struct{
	// for all.
	IM_INT32        type;   // IPC_TYPE_xxx.
	ipc_dwl_t	*dwl;

	// for pipe.
	IM_INT32	pipeSize;
	IM_INT32	pipeUsage;
}ipc_instance_t;


IPC_API IM_UINT32 ipc_version(IM_TCHAR *ver_string)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ver_string){
		im_oswl_strcpy(ver_string, (IM_TCHAR *)IM_STR(VER_STRING));
	}
	return IM_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_PATCH);
}


IPC_API IM_RET ipcsync_init(OUT IPCSYNC_HANDLE *handle, IN IM_TCHAR *keyStr)
{
	ipc_instance_t *inst;
	IM_INFOMSG((IM_STR("%s(keyStr=%s)"), IM_STR(_IM_FUNC_), (keyStr==IM_NULL)?IM_NULL:keyStr));

	//
	if((handle == IM_NULL) || (keyStr == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	if(im_oswl_strlen(keyStr) > IPC_KEYSTR_MAX_LEN){
		IM_ERRMSG((IM_STR("keyStr too long!")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	inst = (ipc_instance_t *)malloc(sizeof(ipc_instance_t));
	if(inst == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_instance_t) failed")));
		return IM_RET_FAILED;
	}
	memset((void *)inst, 0, sizeof(ipc_instance_t));

	//
	inst->dwl = ipcdwl_sync_init(keyStr);
	if(inst->dwl == IM_NULL){
		IM_ERRMSG((IM_STR("ipcdwl_sync_init() failed")));
		free(inst);
		return IM_RET_FAILED;
	}

	//
	inst->type = IPC_TYPE_SYNC;
	*handle = (IPCSYNC_HANDLE)inst;

	return IM_RET_OK;
}

IPC_API IM_RET ipcsync_deinit(IN IPCSYNC_HANDLE handle)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SYNC)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	ret = ipcdwl_sync_deinit(inst->dwl);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_sync_deinit() failed, ret=%d"), ret));
		// only print error message, still to do release resource.
	}
	free(inst);

	return ret;
}

IPC_API IM_RET ipcsync_set(IN IPCSYNC_HANDLE handle)
{
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SYNC)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//
	if(ipcdwl_sync_set(inst->dwl) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_sync_set() failed")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

IPC_API IM_RET ipcsync_reset(IN IPCSYNC_HANDLE handle)
{
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SYNC)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//
	if(ipcdwl_sync_reset(inst->dwl) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_sync_reset() failed")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

IPC_API IM_RET ipcsync_wait(IN IPCSYNC_HANDLE handle, IN IM_INT32 timeout)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SYNC)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//
	ret = ipcdwl_sync_wait(inst->dwl, timeout);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_sync_wait() failed, ret=%d"), ret));
	}

	return ret;
}

IPC_API IM_RET ipcshm_init(OUT IPCSHM_HANDLE *handle, IN IM_TCHAR *keyStr, IN IM_INT32 size)
{
	ipc_instance_t *inst;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, size=%d)"), IM_STR(_IM_FUNC_), (keyStr==IM_NULL)?IM_NULL:keyStr, size));

	//
	if((handle == IM_NULL) || (keyStr == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	if(size <= 0){
		IM_ERRMSG((IM_STR("invalid parameter, size(%d) error"), size));
		return IM_RET_INVALID_PARAMETER;
	}
	if(im_oswl_strlen(keyStr) > IPC_KEYSTR_MAX_LEN){
		IM_ERRMSG((IM_STR("keyStr too long!")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	inst = (ipc_instance_t *)malloc(sizeof(ipc_instance_t));
	if(inst == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_instance_t) failed")));
		return IM_RET_FAILED;
	}
	memset((void *)inst, 0, sizeof(ipc_instance_t));

	//
	inst->dwl = ipcdwl_shm_init(keyStr, size);
	if(inst->dwl == IM_NULL){
		IM_ERRMSG((IM_STR("ipcdwl_shm_init() failed")));
		free(inst);
		return IM_RET_FAILED;
	}

	//
	inst->type = IPC_TYPE_SHM;
	*handle = (IPCSYNC_HANDLE)inst;

	return IM_RET_OK;
}

IPC_API IM_RET ipcshm_deinit(IN IPCSHM_HANDLE handle)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SHM)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	ret = ipcdwl_shm_deinit(inst->dwl);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_shm_deinit() failed, ret=%d"), ret));
		// only print error message, still to do release resource.
	}
	free(inst);

	return ret;
}

IPC_API IM_RET ipcshm_lock(IN IPCSHM_HANDLE handle, OUT void **buffer, IN IM_INT32 timeout)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SHM) || (buffer == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	ret = ipcdwl_shm_lock(inst->dwl, buffer, timeout);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_shm_lock() failed, ret=%d"), ret));
		return ret;
	}

	return IM_RET_OK;
}

IPC_API IM_RET ipcshm_unlock(IN IPCSHM_HANDLE handle)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_SHM)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	ret = ipcdwl_shm_unlock(inst->dwl);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_shm_unlock() failed, ret=%d"), ret));
		return ret;
	}

	return IM_RET_OK;
}

IPC_API IM_RET ipcpipe_init(OUT IPCPIPE_HANDLE *handle, IN IM_TCHAR *keyStr, IN IM_INT32 usage, IN IM_INT32 size)
{
	ipc_instance_t *inst;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, usage=0x%x, size=%d)"), IM_STR(_IM_FUNC_), (keyStr==IM_NULL)?IM_NULL:keyStr, usage, size));

	//
	if((handle == IM_NULL) || (keyStr == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	if(size <= 0){
		IM_ERRMSG((IM_STR("invalid parameter, size(%d) error"), size));
		return IM_RET_INVALID_PARAMETER;
	}
	if((!(usage & IPC_PIPE_USAGE_READ) && !(usage & IPC_PIPE_USAGE_WRITE)) || ((usage & IPC_PIPE_USAGE_READ) && (usage & IPC_PIPE_USAGE_WRITE))){
		IM_ERRMSG((IM_STR("invalid parameter, usage(0x%x) error"), usage));
		return IM_RET_INVALID_PARAMETER;
	}
	if(im_oswl_strlen(keyStr) > IPC_KEYSTR_MAX_LEN){
		IM_ERRMSG((IM_STR("keyStr too long!")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	inst = (ipc_instance_t *)malloc(sizeof(ipc_instance_t));
	if(inst == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_instance_t) failed")));
		return IM_RET_FAILED;
	}
	memset((void *)inst, 0, sizeof(ipc_instance_t));

	//
	inst->dwl = ipcdwl_pipe_init(keyStr, usage, size);
	if(inst->dwl == IM_NULL){
		IM_ERRMSG((IM_STR("ipcdwl_pipe_init() failed")));
		free(inst);
		return IM_RET_FAILED;
	}

	//
	inst->type = IPC_TYPE_PIPE;
	inst->pipeSize = size;
	inst->pipeUsage = usage;
	*handle = (IPCSYNC_HANDLE)inst;

	return IM_RET_OK;
}

IPC_API IM_RET ipcpipe_deinit(IN IPCPIPE_HANDLE handle)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_PIPE)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}

	//
	ret = ipcdwl_pipe_deinit(inst->dwl);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_pipe_deinit() failed, ret=%d"), ret));
		// only print error message, still to do release resource.
	}
	free(inst);

	return ret;
}

IPC_API IM_RET ipcpipe_read(IN IPCPIPE_HANDLE handle, OUT void *data, IN IM_INT32 size, IN IM_INT32 timeout)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s(data=0x%x, size=%d, timeout=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)data, size, timeout));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_PIPE) || 
		(data == IM_NULL) || (size <= 0) || (size > inst->pipeSize)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}
	if(!(inst->pipeUsage & IPC_PIPE_USAGE_READ)){
		IM_ERRMSG((IM_STR("this pipe cannot read from, pipeUsage=0x%x"), inst->pipeUsage));
		return IM_RET_FAILED;
	}

	//
	ret = ipcdwl_pipe_read(inst->dwl, data, size, timeout);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_pipe_read() failed, ret=%d"), ret));
		return ret;
	}

	return IM_RET_OK;
}

IPC_API IM_RET ipcpipe_write(IN IPCPIPE_HANDLE handle, IN void *data, IN IM_INT32 size)
{
	IM_RET ret;
	ipc_instance_t *inst = (ipc_instance_t *)handle;
	IM_INFOMSG((IM_STR("%s(data=0x%x, size=%d)"), IM_STR(_IM_FUNC_), (IM_INT32)data, size));

	//
	if((inst == IM_NULL) || (inst->type != IPC_TYPE_PIPE) || 
		(data == IM_NULL) || (size <= 0) || (size > inst->pipeSize)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_INVALID_PARAMETER;
	}
	if(!(inst->pipeUsage & IPC_PIPE_USAGE_WRITE)){
		IM_ERRMSG((IM_STR("this pipe cannot write to, pipeUsage=0x%x"), inst->pipeUsage));
		return IM_RET_FAILED;
	}

	//
	ret = ipcdwl_pipe_write(inst->dwl, data, size);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcdwl_pipe_write() failed, ret=%d"), ret));
		return ret;
	}

	return IM_RET_OK;
}


