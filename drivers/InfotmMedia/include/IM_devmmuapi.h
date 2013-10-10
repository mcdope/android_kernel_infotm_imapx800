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
** v1.1.0	leo@2012/03/16: first commit.
**
*****************************************************************************/ 

#ifndef __IM_DEVMMUAPI_H__
#define __IM_DEVMMUAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef DEVMMU_EXPORTS
	#define DEVMMU_API		__declspec(dllexport)	/* For dll lib */
#else
	#define DEVMMU_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define DEVMMU_API
#endif	
//#############################################################################
//
//
//
typedef void * DMMUINST;

//
// keep the order from 0 to (max-1)
//
#define DMMU_DEV_G2D		(0)
#define DMMU_DEV_IDS0_W0	(1)
#define DMMU_DEV_IDS0_W1	(2)
#define DMMU_DEV_IDS0_W2	(3)
#define DMMU_DEV_IDS0_CBCR	(4)
#define DMMU_DEV_IDS1_W0	(5)
#define DMMU_DEV_IDS1_W1	(6)
#define DMMU_DEV_IDS1_W2	(7)
#define DMMU_DEV_IDS1_CBCR	(8)
#define DMMU_DEV_VDEC		(9)

#define DMMU_DEV_MAX		(10)


DEVMMU_API IM_UINT32 dmmu_version(OUT IM_TCHAR *ver_string);
DEVMMU_API IM_RET dmmu_init(OUT DMMUINST *pinst, IN IM_UINT32 devid);
DEVMMU_API IM_RET dmmu_deinit(IN DMMUINST inst);
DEVMMU_API IM_RET dmmu_enable(IN DMMUINST inst);
DEVMMU_API IM_RET dmmu_disable(IN DMMUINST inst);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_DEVMMUAPI_H__

