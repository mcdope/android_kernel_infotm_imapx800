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
** v1.0.1	leo@2012/03/15: first commit.
**
*****************************************************************************/ 

#ifndef __IM_COMMON_H__
#define __IM_COMMON_H__

//
//
//
#define IM_SAFE_DELETE(ptr)     if(ptr != IM_NULL){delete(ptr); ptr=IM_NULL;}
#define IM_SAFE_RELEASE(ptr)    if(ptr != IM_NULL){ptr->release(); ptr=IM_NULL;}

//
//
//
#define IM_VERSION_STRING_LEN_MAX       16
#define IM_MAKE_VERSION(major, minor, patch)	(((major & 0xff) << 16) | ((minor & 0xff)<<8) | (patch & 0xff))
#define IM_VER_MAJOR(ver)	((ver >> 16) & 0xff)
#define IM_VER_MINOR(ver)	((ver >> 8) & 0xff)
#define IM_VER_PATCH(ver)	(ver & 0xff)

//
//
//
#define IM_JIF(exp)     \
	if(IM_FAILED(exp)){     \
		IM_ERRMSG((IM_STR("IM_JIF failed")));	\
		goto Fail;	\
	}

//
// IM_Buffer, Note, flag [15:0] used by buffalloc only, user can read but not write, [31:16] reserved for user.
// flag [15:4] indicate this buffer ID which is allocated by alc_alloc() or alc_map_useraddr(), 0 is invalid ID.
// flag [3:0] used to indicate some mark, i.e, IM_BUFFER_FLAG_xxx
//
#define IM_BUFFER_INVALID_ID			(0)
#define IM_BUFFER_SET_ID(buffer, id)		do{ buffer.flag &= ~0xfff0; buffer.flag |= ((id) << 4); }while(0)
#define IM_pBUFFER_SET_ID(pBuffer, id)		do{ pBuffer->flag &= ~0xfff0; pBuffer->flag |= ((id) << 4); }while(0)
#define IM_BUFFER_GET_ID(buffer)		((buffer.flag & 0xfff0) >> 4)
#define IM_pBUFFER_GET_ID(pBuffer)		((pBuffer->flag & 0xfff0) >> 4)

#define IM_BUFFER_FLAG_PHY			(1<<0)	// linear physical memory.
#define IM_BUFFER_FLAG_PHY_NONLINEAR		(1<<1)	// non-linear physical memory, it used for mmu, the details please see IM_devmmuapi.h
#define IM_BUFFER_FLAG_DEVADDR			(1<<2)	// this buffer has dev-mmu address, it can get by alc_get_devaddr().
#define IM_BUFFER_FLAG_MAPPED			(1<<3)

#define IM_BUFFER_HAS_PHY_FLAG(buffer)		(((buffer.flag & IM_BUFFER_FLAG_PHY) || (buffer.flag & IM_BUFFER_FLAG_PHY_NONLINEAR))?IM_TRUE:IM_FALSE)
#define IM_pBUFFER_HAS_PHY_FLAG(pBuffer)	(((pBuffer->flag & IM_BUFFER_FLAG_PHY) || (pBuffer->flag & IM_BUFFER_FLAG_PHY_NONLINEAR))?IM_TRUE:IM_FALSE)

#define IM_BUFFER_CLEAR(buffer)			do{buffer.vir_addr = IM_NULL; buffer.phy_addr = 0; buffer.size = 0; buffer.flag = 0; }while(0)
#define IM_pBUFFER_CLEAR(pBuffer)		do{pBuffer->vir_addr = IM_NULL; pBuffer->phy_addr = 0; pBuffer->size = 0; pBuffer->flag = 0; }while(0)
#define IM_BUFFER_COPYTO_BUFFER(dst, src)	do{ dst.vir_addr = src.vir_addr; dst.phy_addr = src.phy_addr;	\
						dst.size = src.size; dst.flag = src.flag; }while(0)
#define IM_BUFFER_COPYTO_pBUFFER(pDst, src)	do{ pDst->vir_addr = src.vir_addr; pDst->phy_addr = src.phy_addr;	\
						pDst->size = src.size; pDst->flag = src.flag; }while(0)
#define IM_pBUFFER_COPYTO_BUFFER(dst, pSrc)	do{ dst.vir_addr = pSrc->vir_addr; dst.phy_addr = pSrc->phy_addr;	\
						dst.size = pSrc->size; dst.flag = pSrc->flag; }while(0)
#define IM_pBUFFER_COPYTO_pBUFFER(pDst, pSrc)	do{ pDst->vir_addr = pSrc->vir_addr; pDst->phy_addr = pSrc->phy_addr;	\
						pDst->size = pSrc->size; pDst->flag = pSrc->flag; }while(0)

typedef struct{
	void * vir_addr;
	IM_UINT32 phy_addr;	// if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
	IM_UINT32 size;
	IM_UINT32 flag;	// [15:0] reserved, user cannot write to.
}IM_Buffer;

//
// deprecate, please don't use.
//
#define IM_BUFFER_ALIGN_NOLIMIT	(0)	// 1<<0
#define IM_BUFFER_ALIGN_4BYTES	(2)	// 1<<2
#define IM_BUFFER_ALIGN_8BYTES	(3)	// 1<<3
#define IM_BUFFER_ALIGN_16BYTES	(4)	// 1<<4
#define IM_BUFFER_ALIGN_32BYTES	(5)	// 1<<5
#define IM_BUFFER_ALIGN_64BYTES	(6)	// 1<<6

//
//
//
#define IM_SIZEALIGNED16(a) 	(((a)+15)&(~15))
#define IM_SIZEALIGNED8(b)	(((b)+7)&(~7))


#endif	// __IM_COMMON_H__

