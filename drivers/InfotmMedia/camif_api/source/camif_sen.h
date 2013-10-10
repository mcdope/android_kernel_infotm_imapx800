/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camif_sen.h
--
--  Description :
--		Head file for camif_sen.c.	
--
--	Author:
--  	Jimmy Shu   <jimmy.shu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	Jimmy@2012/10/16: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __CAMIF_SEN_H__
#define __CAMIF_SEN_H__

#define BUFFER_NUM	(2+1)	// work_buffer + idle_buffer.

typedef struct{
	IM_BOOL					enabled;
	IM_INT32				sensorId;	//sensor module id
	IM_UINT32 				zmVal;		//zoom value
	IM_UINT32 				wbMode;		//white balance mode
	IM_UINT32 				spEffect;	//special effect
	IM_UINT32 				scMode;		//scene mode
	IM_INT32				bufferNum;
	IM_Buffer				buffers[BUFFER_NUM];	// [0--BUFFER_NUM-2] work buffers, [BUFFER_NUM-1] idle buffer.
	IM_INT32				imageSize;	// include y, cb and cr.
	IM_INT32				ySize;	// y, ycbycr
	IM_INT32				uSize;	// cb, cbcr
	IM_INT32				vSize;	// cr
	IM_Buffer				rdyBuffer;
	IM_BOOL					hasRdyFrame;
	cam_preview_config_t	preCfg;
	camsen_handle_t 		camsenHandle;
	camifpwl_spinlock_t		spinLock;
}camif_sen_context_t;


#endif	// __CAMIF_SEN_H__


