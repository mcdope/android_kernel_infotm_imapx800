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
** v1.2.2	leo@2012/04/13: remove alc_map_useraddr() and alc_unmap_useraddr(), because these don't tested.
** v1.2.2	leo@2012/05/02: 
** 				1. support and tested alc_map_useraddr() and alc_unmap_useraddr() ok. 
**				2. add alc_flush_buffer() function.
** v1.2.4	leo@2012/05/16: add ALC_FLAG_CACHE to support alloc physical(linear/nonliear) buffer with cache.
** v1.2.9   leo@2012/09/10: add big-memory support, ALC_FLAG_BIGMEM.
** v1.2.12  leo@2012/11/30: modify alc_init_bigmem() interface.
**
*****************************************************************************/ 

#ifndef __IM_BUFFALLOCAPI_H__
#define __IM_BUFFALLOCAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef BUFFALLOC_EXPORTS
	#define BUFFALLOC_API		__declspec(dllexport)	/* For dll lib */
#else
	#define BUFFALLOC_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define BUFFALLOC_API
#endif	
//#############################################################################
// buffalloc context type.
typedef void * ALCCTX;

//
// allocate properties.
// flag, bit0--bit4 is only 1 can set at one time.
#define ALC_FLAG_PHY_PREFER		(1<<0)	// prefer linear physical memory, else can virtual memory.
#define ALC_FLAG_PHY_LINEAR_PREFER	(1<<1)	// prefer physical and linear, else can non-linear memory but must physical.
#define ALC_FLAG_PHY_MUST		(1<<2)	// physical and linear requirement.

#define ALC_FLAG_ALIGN_4BYTES		(1<<4)
#define ALC_FLAG_ALIGN_8BYTES		(1<<5)
#define ALC_FLAG_ALIGN_32BYTES		(1<<6)
#define ALC_FLAG_ALIGN_64BYTES		(1<<7)
#define ALC_FLAG_DEVADDR		(1<<12)	// request the devaddr, used for dev-mmu.
#define ALC_FLAG_CACHE			(1<<13)
#define ALC_FLAG_BIGMEM         (1<<14) // only work with ALC_FLAG_PHY_MUST, if not, it's be ignored also.


// flush flag.
#define ALC_FLUSH_DEVICE_TO_CPU		(1)
#define ALC_FLUSH_CPU_TO_DEVICE		(2)


/*============================Interface API==================================*/
/* 
 * FUNC: get conert version.
 * PARAMS: ver_string, save this version string.
 * RETURN: [31:16] major version number, [15:0] minor version number.
 */
BUFFALLOC_API IM_UINT32 alc_version(OUT IM_TCHAR *ver_string);

/* 
 * FUNC: open buffallocer.
 * PARAMS:
 * 	inst, save buffalloc context.
 * 	owner, owner of this buffalloc, max ALC_OWNER_LEN_MAX char.
 * RETURN: IM_RET_OK is successful, else failed.
 */
#define ALC_OWNER_LEN_MAX	16
BUFFALLOC_API IM_RET alc_open(OUT ALCCTX *inst, IN IM_TCHAR *owner);

/* 
 * FUNC: close buffalloc.
 * PARAMS: inst, buffalloc context created by alc_open().
 * RETURN: IM_RET_OK is successful, else failed.
 */
BUFFALLOC_API IM_RET alc_close(IN ALCCTX inst);

/* 
 * FUNC: set bigmem support, bigmem has a fixed attributes of ALC_FLAG_PHY_MUST.
 * PARAMS:
 *  inst, instance created by alc_open().
 *  blockSize, bigmem initial size of pre block, in bytes unit.
 *  blockNum, block number.
 *  isCache, if cache or not.
 * RETURN: IM_RET_OK is successful, else failed.
 */
BUFFALLOC_API IM_RET alc_init_bigmem(IN ALCCTX inst, IN IM_INT32 blockSize, IN IM_INT32 blockNum, IN IM_BOOL isCache);
BUFFALLOC_API IM_RET alc_deinit_bigmem(IN ALCCTX inst);

/*
 * FUNC: allocate buffer.
 * PARAMS: inst, buffalloc context created by alc_open();
 * 	size, requested size.
 *	buffer, allocated buffer infomation.
 *	flag, allocate properties.
 * RETURN: IM_RET_OK is successful
 * 	IM_RET_FALSE, when flag has ALC_FLAG_PHY_PREFER, but can not allocate PHY.
 * 	else failed.
 * NOTE: if flag has ALC_FLAG_BIGMEM, the size and ALC_FLAG_CACHE in flag will be ignored, the size
 *	default as a block. 
 */
BUFFALLOC_API IM_RET alc_alloc(IN ALCCTX inst, IN IM_INT32 size, OUT IM_Buffer *buffer, IN IM_INT32 flag);

/*
 * FUNC: free buffer.
 * PARAMS: inst, buffalloc context created by alc_open();
 *	buffer, previously allocated buffer.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
BUFFALLOC_API IM_RET alc_free(IN ALCCTX inst, IN IM_Buffer *buffer);

/*
 * FUNC: map user virtual address to IM_Buffer.
 * PARAMS: inst, buffalloc context created by alc_open();
 * 	usrVirAddr, user virtual address.
 * 	size, size of the usrVirAddr.
 *	buffer, allocated buffer infomation.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
BUFFALLOC_API IM_RET alc_map_useraddr(IN ALCCTX inst, IN void *usrVirAddr, IN IM_INT32 size, OUT IM_Buffer *buffer, OUT IM_UINT32 *devAddr);
BUFFALLOC_API IM_RET alc_unmap_useraddr(IN ALCCTX inst, IN IM_Buffer *buffer);

/*
 * FUNC: flush buffer cache, keep the coherence between cpu and device.
 * PARAMS: inst, buffalloc instance.
 * 	buffer, 
 * 	flag, see ALC_FLUSH_xxx.
 * RETURN: IM_RET_OK is successful, else failed.
 * NOTE: only mapped useraddr needs to do flush, the other has the feature inside.
 */
BUFFALLOC_API IM_RET alc_flush_buffer(IN ALCCTX inst, IN IM_Buffer *buffer, IN IM_INT32 flag);

/*
 * FUNC:
 * PARAMS:
 * RETURN:
 */
BUFFALLOC_API IM_RET alc_get_devaddr(IN ALCCTX inst, IN IM_Buffer *buffer, OUT IM_UINT32 *devAddr);

/*
 * alc buffer statistic tools interface.
 */
#define ALC_BUFFER_ATTRI_ALLOCATED	(1<<0)	// bit[0--4] has only 1 bit set simultineously.
#define ALC_BUFFER_ATTRI_MAPPED		(1<<1)

#define ALC_BUFFER_ATTRI_DEVADDR	(1<<4)
#define ALC_BUFFER_ATTRI_BIGMEM	    (1<<5)

typedef struct{
	IM_Buffer	buffer;
	IM_UINT32	devAddr;	// devmmu viewside address.
	IM_INT32	pageNum;
	IM_UINT32	attri;	// attributes.
	void *		privData;
}alc_buffer_t;

BUFFALLOC_API IM_RET alc_statc_list_owner(IN ALCCTX inst, INOUT im_list_handle_t list);	// list of IM_TCHAR[ALC_OWNER_LEN_MAX].
BUFFALLOC_API IM_RET alc_statc_list_owner_buffer(IN ALCCTX inst, IN IM_TCHAR *owenr, INOUT im_list_handle_t list);// list of alc_buffer_t.

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_BUFFALLOCAPI_H__

