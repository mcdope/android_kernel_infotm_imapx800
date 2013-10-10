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
** v1.2.2	leo@2012/04/13: define PMM_PAGE_SIZE and PMM_PAGE_SHIFT.
** v1.2.4	leo@2012/05/02: add PMMLIB_IOCTL_MM_FLUSH_BUFFER and pmmlib_ioctl_ds_mm_flush_buffer_t.
** v1.2.7	leo@2012/07/11: add pmmlib_dmmu_reset() function.
** v1.2.9	leo@2012/09/10: add bigmem support.
** v1.2.12	leo@2012/11/30: rewrite bigmem.
**
*****************************************************************************/ 

#ifndef __PMM_LIB_H__
#define __PMM_LIB_H__

//
//
//
typedef void *	pmm_handle_t;


//
//
//
#define PMM_PAGE_SIZE	(4096)
#define PMM_PAGE_SHIFT	(12)

//#############################################################################
IM_RET pmmlib_init(OUT pmm_handle_t *phandle, IN IM_TCHAR *owner);
IM_RET pmmlib_deinit(IN pmm_handle_t handle);

// dmmu
IM_RET pmmlib_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid);
IM_RET pmmlib_dmmu_deinit(IN pmm_handle_t handle);
IM_RET pmmlib_dmmu_reset(IN pmm_handle_t handle);
IM_RET pmmlib_dmmu_enable(IN pmm_handle_t handle);
IM_RET pmmlib_dmmu_disable(IN pmm_handle_t handle);

// mm
IM_RET pmmlib_mm_init_bigmem(IN pmm_handle_t handle, IN IM_INT32 blockSize, IN IM_INT32 blockNum, IN IM_BOOL isCache);
IM_RET pmmlib_mm_deinit_bigmem(IN pmm_handle_t handle);
IM_RET pmmlib_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer);
IM_RET pmmlib_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer);
IM_RET pmmlib_mm_map_useraddr(IN pmm_handle_t handle, IN void *uVirAddr, IN IM_INT32 size, OUT alc_buffer_t *alcBuffer);
IM_RET pmmlib_mm_unmap_useraddr(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer);
IM_RET pmmlib_mm_flush_buffer(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer, IN IM_INT32 flag);
IM_RET pmmlib_mm_add_useraddr_for_statc(IN pmm_handle_t handle, IN void *uVirAddr, IN IM_INT32 size);
IM_RET pmmlib_mm_del_useraddr_for_statc(IN pmm_handle_t handle, IN void *uVirAddr);

// statc
IM_RET pmmlib_statc_get_owner_num(IN pmm_handle_t handle, OUT IM_INT32 *num);
IM_RET pmmlib_statc_get_owner(IN pmm_handle_t handle, IN IM_INT32 index, OUT IM_TCHAR *owner);
IM_RET pmmlib_statc_get_owner_buffer_num(IN pmm_handle_t handle, IN IM_TCHAR *owner, OUT IM_INT32 *num);
IM_RET pmmlib_statc_get_owner_buffer(IN pmm_handle_t handle, IN IM_TCHAR *owner, IN IM_INT32 index, OUT alc_buffer_t *alcBuffer);

// simple tools
alc_buffer_t * pmmlib_getAlcBufferFromPhyAddr(IN pmm_handle_t handle, IN IM_UINT32 phyAddr);

//
// ioctl, ds=data struct.
//
#define PMMLIB_IOCTL_INIT			0x1000		// pmmlib_init(), ds=IM_TCHAR *, max length is ALC_OWNER_LEN_MAX.
#define PMMLIB_IOCTL_DEINIT			0x1001		// pmmlib_deinit(), ds=.
#define PMMLIB_IOCTL_DMMU_INIT			0x2000		// pmmlib_dmuu_init(), ds=IM_UINT32 *.
#define PMMLIB_IOCTL_DMMU_DEINIT		0x2001		// pmmlib_dmuu_deinit(), ds=.
#define PMMLIB_IOCTL_DMMU_ENABLE		0x2002		// pmmlib_dmuu_enable(), ds=.
#define PMMLIB_IOCTL_DMMU_DISABLE		0x2003		// pmmlib_dmuu_disable(), ds=.
#define PMMLIB_IOCTL_MM_ALLOC			0x3000		// pmmlib_mm_alloc(), ds=pmmlib_ioctl_ds_mm_alloc_t *.
#define PMMLIB_IOCTL_MM_FREE			0x3001		// pmmlib_mm_free(), ds=alc_buffer_t *.
#define PMMLIB_IOCTL_MM_MAP_USERADDR		0x3002		// pmmlib_mm_map_useraddr(), ds=pmmlib_ioctl_ds_mm_map_useraddr_t *.
#define PMMLIB_IOCTL_MM_UNMAP_USERADDR		0x3003		// pmmlib_mm_unmap_useraddr(), ds=alc_buffer_t *.
#define PMMLIB_IOCTL_MM_ADD_USERADDR_FOR_STATC	0x3004		// pmmlib_mm_add_useraddr_for_statc(), ds=pmmlib_ioctl_ds_mm_add_useraddr_for_statc_t *.
#define PMMLIB_IOCTL_MM_DEL_USERADDR_FOR_STATC	0x3005		// pmmlib_mm_del_useraddr_for_statc(), ds=void *.
#define PMMLIB_IOCTL_MM_FLUSH_BUFFER		0x3006		// pmmlib_mm_flush_buffer(), ds=pmmlib_ioctl_ds_mm_flush_buffer_t *.
#define PMMLIB_IOCTL_MM_INIT_BIGMEM			0x3007		// pmmlib_mm_init_bigmem, ds=pmmlib_ioctl_ds_mm_init_bigmem_t *.
#define PMMLIB_IOCTL_MM_DEINIT_BIGMEM		0x3008		// pmmlib_mm_deinit_bigmem(), ds=.
#define PMMLIB_IOCTL_STATC_LOCK_ACCESS		0x4000		// ds=.
#define PMMLIB_IOCTL_STATC_UNLOCK_ACCESS	0x4001		// ds=.
#define PMMLIB_IOCTL_STATC_GET_OWNER_NUM	0x4002		// pmmlib_statc_get_owner_num(), ds=IM_INT32 *.
#define PMMLIB_IOCTL_STATC_GET_OWNER		0x4003		// pmmlib_statc_get_owner(), ds=pmmlib_ioctl_ds_statc_get_owner_t *.
#define PMMLIB_IOCTL_STATC_GET_OWNER_BUFFER_NUM	0x4004		// pmmlib_statc_get_owner_buffer_num(), ds=pmmlib_ioctl_ds_statc_get_owner_buffer_num_t *.
#define PMMLIB_IOCTL_STATC_GET_OWNER_BUFFER	0x4005		// pmmlib_statc_get_owner_buffer(), ds=pmmlib_ioctl_ds_statc_get_owner_buffer_t *.

typedef struct{
	IM_INT32	size;
	IM_INT32	flag;
	alc_buffer_t	alcBuffer;
}pmmlib_ioctl_ds_mm_alloc_t;

typedef struct{
	void		*uVirAddr;
	IM_INT32	size;
	alc_buffer_t	alcBuffer;
}pmmlib_ioctl_ds_mm_map_useraddr_t;

typedef struct{
	IM_INT32	flag;
	alc_buffer_t	alcBuffer;
}pmmlib_ioctl_ds_mm_flush_buffer_t;

typedef struct{
	void		*uVirAddr;
	IM_INT32	size;
}pmmlib_ioctl_ds_mm_add_useraddr_for_statc_t;

typedef struct{
	IM_INT32	blockSize;
	IM_INT32	blockNum;
	IM_BOOL		isCache;
}pmmlib_ioctl_ds_mm_init_bigmem_t;

typedef struct{
	IM_INT32	index;
	IM_TCHAR	owner[ALC_OWNER_LEN_MAX];
}pmmlib_ioctl_ds_statc_get_owner_t;

typedef struct{
	IM_TCHAR	owner[ALC_OWNER_LEN_MAX];
	IM_INT32	num;
}pmmlib_ioctl_ds_statc_get_owner_buffer_num_t;

typedef struct{
	IM_TCHAR	owner[ALC_OWNER_LEN_MAX];
	IM_INT32	index;
	alc_buffer_t	alcBuffer;
}pmmlib_ioctl_ds_statc_get_owner_buffer_t;


#endif	// __PMM_LIB_H__

