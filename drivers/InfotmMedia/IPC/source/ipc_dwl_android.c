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
** v1.0.2	leo@2012/04/28: fixed the timeout from 'ms' to 's' and 'us' error.
**		leo@2012/05/04: fixed select timeout return processing.
** v1.0.3	leo@2012/10/24: "ipc" driver node change to "ipcx".
**
*****************************************************************************/

#include <sys/mman.h>
#include <sys/time.h>
#include <sys/select.h>

#include <InfotmMedia.h>
#include <IM_ipcapi.h>
#include <ipc_uk.h>
#include <ipc_dwl.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IPCDWL_I:"
#define WARNHEAD	"IPCDWL_W:"
#define ERRHEAD		"IPCDWL_E:"
#define TIPHEAD		"IPCDWL_T:"


#define IPC_DEV_NODE	"/dev/ipcx"	


ipc_dwl_t *ipcdwl_sync_init(IM_TCHAR *keyStr)
{
	ipc_dwl_t *dwl = IM_NULL;
	IM_TCHAR ks[IPC_KEYSTR_MAX_LEN + 1];
	IM_INFOMSG((IM_STR("%s(keyStr=%s)"), IM_STR(_IM_FUNC_), keyStr));

	//
	dwl = (ipc_dwl_t *)malloc(sizeof(ipc_dwl_t));
	if(dwl == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_dwl_t) failed")));
		return IM_NULL;
	}
	memset((void *)dwl, 0, sizeof(ipc_dwl_t));

	//
	dwl->fd = open(IPC_DEV_NODE, O_RDWR);
	if(dwl->fd < 0){
		IM_ERRMSG((IM_STR("open(%s) failed"), IM_STR(IPC_DEV_NODE)));
		goto Fail;
	}

	//
	strcpy(ks, keyStr);
	if(ioctl(dwl->fd, IPC_IOCTL_SYNC_INIT, ks)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SYNC_INIT) failed")));
		goto Fail;
	}

	return dwl;
Fail:
	if(dwl->fd){
		close(dwl->fd);
	}
	free(dwl);
	return IM_NULL;
}

IM_RET ipcdwl_sync_deinit(ipc_dwl_t *dwl)
{
	IM_RET ret=IM_RET_OK; 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_SYNC_DEINIT, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SYNC_DEINIT) failed")));
		ret = IM_RET_FAILED;
	}
	close(dwl->fd);
	free(dwl);
	return ret;
}

IM_RET ipcdwl_sync_set(ipc_dwl_t *dwl)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_SYNC_SET, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SYNC_SET) failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET ipcdwl_sync_reset(ipc_dwl_t *dwl)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_SYNC_RESET, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SYNC_RESET) failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET ipcdwl_sync_wait(ipc_dwl_t *dwl, IM_INT32 timeout)
{
	IM_INT32 ret=0;
	fd_set rfds;
	struct timeval tv;
	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));
	
	FD_ZERO(&rfds);
	FD_SET(dwl->fd, &rfds);
	if(timeout == -1){
		do{
			tv.tv_sec = 10;
			tv.tv_usec = 0;	// 10s
			ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
			IM_INFOMSG((IM_STR("select() ret=%d"), ret));
		}while(ret == 0);
	}else{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
		IM_INFOMSG((IM_STR("select() ret=%d"), ret));
	}
	
	return (ret==0)?IM_RET_TIMEOUT:((ret>0)?IM_RET_OK:IM_RET_FAILED);
}

ipc_dwl_t *ipcdwl_shm_init(IM_TCHAR *keyStr, IM_INT32 size)
{
	ipc_dwl_t *dwl = IM_NULL;
	ipc_ioctl_shm_init_t ds_shm_init;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, size=%d)"), IM_STR(_IM_FUNC_), keyStr, size));

	//
	dwl = (ipc_dwl_t *)malloc(sizeof(ipc_dwl_t));
	if(dwl == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_dwl_t) failed")));
		return IM_NULL;
	}
	memset((void *)dwl, 0, sizeof(ipc_dwl_t));

	//
	dwl->fd = open(IPC_DEV_NODE, O_RDWR);
	if(dwl->fd < 0){
		IM_ERRMSG((IM_STR("open(%s) failed"), IM_STR(IPC_DEV_NODE)));
		goto Fail;
	}

	//
	strcpy(ds_shm_init.keyStr, keyStr);
	ds_shm_init.size = size;
	if(ioctl(dwl->fd, IPC_IOCTL_SHM_INIT, &ds_shm_init)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SHM_INIT) failed")));
		goto Fail;
	}
	dwl->shmPhyAddr = ds_shm_init.phyAddr;
	dwl->shmSize = ds_shm_init.size;
	IM_INFOMSG((IM_STR("shm.phyAddr=0x%x, shm.size=%d"), dwl->shmPhyAddr, dwl->shmSize));

	dwl->shmVirAddr = mmap(0, dwl->shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, dwl->fd, dwl->shmPhyAddr);
	if(dwl->shmVirAddr == MAP_FAILED){
		IM_ERRMSG((IM_STR("mmap(shm) failed")));
		goto Fail;
	}
	IM_INFOMSG((IM_STR("shm.virAddr=0x%x"), (IM_INT32)dwl->shmVirAddr));

	return dwl;
Fail:
	if(dwl->fd){
		close(dwl->fd);	// close would do shm deinit also.
	}
	free(dwl);
	return IM_NULL;
}

IM_RET ipcdwl_shm_deinit(ipc_dwl_t *dwl)
{
	IM_RET ret=IM_RET_OK; 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_SHM_DEINIT, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SHM_DEINIT) failed")));
		ret = IM_RET_FAILED;
	}
	munmap(dwl->shmVirAddr, dwl->shmSize);
	close(dwl->fd);
	free(dwl);
	return ret;
}

IM_RET ipcdwl_shm_lock(ipc_dwl_t *dwl, void **buffer, IM_INT32 timeout)
{
	IM_INT32 ret=0;
	fd_set rfds;
	struct timeval tv;
	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));
	
	FD_ZERO(&rfds);
	FD_SET(dwl->fd, &rfds);
	if(timeout == -1){
		do{
			tv.tv_sec = 10;
			tv.tv_usec = 0;	// 10s
			ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
			IM_INFOMSG((IM_STR("select() ret=%d"), ret));
		}while(ret == 0);
	}else{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
		IM_INFOMSG((IM_STR("select() ret=%d"), ret));
	}

	if(ret <= 0){
		IM_ERRMSG((IM_STR("ipcdwl_shm_lock's select() failed, ret=%d"), ret));
		return (ret==0) ? IM_RET_TIMEOUT : IM_RET_FAILED;
	}

	if(ioctl(dwl->fd, IPC_IOCTL_SHM_LOCK, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SHM_LOCK) failed")));
		return IM_RET_FAILED;
	}
	*buffer = dwl->shmVirAddr;
	return IM_RET_OK;
}

IM_RET ipcdwl_shm_unlock(ipc_dwl_t *dwl)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_SHM_UNLOCK, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_SHM_UNLOCK) failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

ipc_dwl_t *ipcdwl_pipe_init(IM_TCHAR *keyStr, IM_INT32 usage, IM_INT32 size)
{
	ipc_dwl_t *dwl = IM_NULL;
	ipc_ioctl_pipe_init_t ds_pipe_init;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, usage=0x%x, size=%d)"), IM_STR(_IM_FUNC_), keyStr, usage, size));

	//
	dwl = (ipc_dwl_t *)malloc(sizeof(ipc_dwl_t));
	if(dwl == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(ipc_dwl_t) failed")));
		return IM_NULL;
	}
	memset((void *)dwl, 0, sizeof(ipc_dwl_t));

	//
	dwl->fd = open(IPC_DEV_NODE, O_RDWR);
	if(dwl->fd < 0){
		IM_ERRMSG((IM_STR("open(%s) failed"), IM_STR(IPC_DEV_NODE)));
		goto Fail;
	}

	//
	strcpy(ds_pipe_init.keyStr, keyStr);
	ds_pipe_init.usage = usage;
	ds_pipe_init.size = size;
	if(ioctl(dwl->fd, IPC_IOCTL_PIPE_INIT, &ds_pipe_init)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_PIPE_INIT) failed")));
		goto Fail;
	}

	return dwl;
Fail:
	if(dwl->fd){
		close(dwl->fd);	// close would do shm deinit also.
	}
	free(dwl);
	return IM_NULL;
}

IM_RET ipcdwl_pipe_deinit(ipc_dwl_t *dwl)
{
	IM_RET ret=IM_RET_OK; 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(ioctl(dwl->fd, IPC_IOCTL_PIPE_DEINIT, IM_NULL)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_PIPE_DEINIT) failed")));
		ret = IM_RET_FAILED;
	}
	close(dwl->fd);
	free(dwl);
	return ret;
}

IM_RET ipcdwl_pipe_write(ipc_dwl_t *dwl, void *data, IM_INT32 size)
{
	ipc_ioctl_pipe_rw_t ds_pipe_rw;
	IM_INFOMSG((IM_STR("%s(size=%d)"), IM_STR(_IM_FUNC_), size));

	ds_pipe_rw.data = data;
	ds_pipe_rw.size = size;
	if(ioctl(dwl->fd, IPC_IOCTL_PIPE_WRITE, &ds_pipe_rw)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_PIPE_WRITE) failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET ipcdwl_pipe_read(ipc_dwl_t *dwl, void *data, IM_INT32 size, IM_INT32 timeout)
{
	IM_INT32 ret=0;
	fd_set rfds;
	struct timeval tv;
	ipc_ioctl_pipe_rw_t ds_pipe_rw;
	IM_INFOMSG((IM_STR("%s(size=%d, timeout=%d)"), IM_STR(_IM_FUNC_), size, timeout));
	
	FD_ZERO(&rfds);
	FD_SET(dwl->fd, &rfds);
	if(timeout == -1){
		do{
			tv.tv_sec = 10;
			tv.tv_usec = 0;	// 10s
			ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
			IM_INFOMSG((IM_STR("select() ret=%d"), ret));
		}while(ret == 0);
	}else{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ret = select(dwl->fd+1, &rfds, IM_NULL, IM_NULL, &tv);
		IM_INFOMSG((IM_STR("select() ret=%d"), ret));
	}

	if(ret <= 0){
		IM_ERRMSG((IM_STR("ipcdwl_pipe_read's select() failed, ret=%d"), ret));
		return (ret==0) ? IM_RET_TIMEOUT : IM_RET_FAILED;
	}

	ds_pipe_rw.data = data;
	ds_pipe_rw.size = size;
	if(ioctl(dwl->fd, IPC_IOCTL_PIPE_READ, &ds_pipe_rw)){
		IM_ERRMSG((IM_STR("ioctl(IPC_IOCTL_PIPE_READ) failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}


