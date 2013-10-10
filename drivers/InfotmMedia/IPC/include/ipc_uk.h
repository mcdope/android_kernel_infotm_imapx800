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

#ifndef __IPC_UK_H__
#define __IPC_UK_H__


//
// ioctl code.
//
#define IPC_IOCTL_SYNC_INIT	0x1000	// param: keyStr--IM_TCHAR[IPC_KEYSTR_MAX_LEN + 1].
#define IPC_IOCTL_SYNC_DEINIT	0x1001	// param: none.
#define IPC_IOCTL_SYNC_SET	0x1002	// param: none.
#define IPC_IOCTL_SYNC_RESET	0x1003	// param: none.

#define IPC_IOCTL_SHM_INIT	0x2000	// param: ipc_ioctl_shm_init_t.
#define IPC_IOCTL_SHM_DEINIT	0x2001	// param: none.
#define IPC_IOCTL_SHM_LOCK	0x2002	// param: none. 
#define IPC_IOCTL_SHM_UNLOCK	0x2003	// param: none.

#define IPC_IOCTL_PIPE_INIT	0x3000	// param: ipc_ioctl_pipe_init_t. 
#define IPC_IOCTL_PIPE_DEINIT	0x3001	// param: none.
#define IPC_IOCTL_PIPE_WRITE	0x3002	// param: ipc_ioctl_pipe_rw_t.
#define IPC_IOCTL_PIPE_READ	0x3003	// param: ipc_ioctl_pipe_rw_t.

typedef struct{
	IM_TCHAR	keyStr[IPC_KEYSTR_MAX_LEN + 1];
	IM_INT32	size;	// [INOUT]
	IM_UINT32	phyAddr;	// [INOUT]
}ipc_ioctl_shm_init_t;

typedef struct{
	IM_TCHAR	keyStr[IPC_KEYSTR_MAX_LEN + 1];
	IM_INT32	size;
	IM_INT32	usage;
}ipc_ioctl_pipe_init_t;

typedef struct{
	void *		data;	// [INOUT]
	IM_INT32	size;
}ipc_ioctl_pipe_rw_t;


#endif	// __IPC_UK_H__

