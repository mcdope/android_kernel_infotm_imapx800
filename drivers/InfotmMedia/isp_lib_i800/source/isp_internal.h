/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_internal.h
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#ifndef _ISP_INTERNAL_H_
#define _ISP_INTERNAL_H_

#include "isp_common.h"
#include "isp_pwl.h"

#include "isp_bccb.h"
#include "isp_bdc.h"
#include "isp_lens.h"
#include "isp_fcc.h"
#include "isp_awb.h"
#include "isp_cmncsc.h"
#include "isp_ae.h"
#include "isp_hist.h"
#include "isp_ee.h"
#include "isp_af.h"
#include "isp_acc.h"
#include "isp_acm.h"
#include "isp_ief.h"
#include "isp_gma.h"
#include "isp_osd.h"
#include "isp_crop.h"
#include "isp_scl.h"                                                                 

typedef struct{
	IM_UINT32	ppMode;
	IM_Buffer	buffY[4];	// y, rgb, ycbycr
	IM_Buffer	buffCb[4];	// cb, cbcr
	IM_Buffer	buffCr[4];	// cr
	IM_UINT32	lengthY;	// 0 indicate not in use, user don't care.
	IM_UINT32	lengthCb;	// 0 indicate not in use, user don't care.
	IM_UINT32	lengthCr;	// 0 indicate not in use, user don't care.
}isp_dma_context_t;

typedef struct{
	IM_BOOL					enable;
	//input mode
	IM_UINT32				inWidth;
	IM_UINT32				inHeight;
	IM_UINT32				intfMode;		//ISP_INTFMODE_XXX
	IM_UINT32				inputBitsNum;
	IM_UINT32				rawMode;
	isp_itu_type_t			ituType;
	isp_signal_polarity_t	sigPol;
	//output mode
	IM_UINT32				outWidth;
	IM_UINT32				outHeight;
	IM_IMAGE_TYPE			outImgType;		//IM_IMAGE_XXX
	//sub module
	IM_BOOL					demEnable;		//demosiac enable
	isp_bccb_context_t 		*bccb;
	isp_bdc_context_t  		*bdc;
	isp_lens_context_t 		*lens;
	isp_gma_context_t  		*gma;
	isp_ee_context_t 		*ee;
	isp_fcc_context_t  		*fcc;
	isp_af_context_t  		*af;
	isp_awb_context_t 		*awb;
	isp_cmncsc_context_t  	*cmncsc;
	isp_ae_context_t 		*ae;
	isp_hist_context_t  	*hist;
	isp_acc_context_t 		*acc;
	isp_ief_context_t 		*ief;
	isp_acm_context_t  		*acm;
	isp_osd_context_t 		*osd;
	isp_crop_context_t 		*crop;
	isp_scl_context_t  		*scl;
	isp_dma_context_t 		dma;
	IM_UINT32 				regVal[ISP_REGISTERS];
	IM_UINT32 				regOfst[ISP_REGISTERS];
}isp_context_t; 

#endif /* #ifndef _ISP_INTERNAL_H_ */
