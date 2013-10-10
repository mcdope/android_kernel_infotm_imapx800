/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_bccb.h
--
--  Description :
--		bccb module.
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

#ifndef _ISP_BCCB_H_
#define	_ISP_BCCB_H_


typedef struct{
	IM_UINT32	rBlkTh;
	IM_UINT32	grBlkTh;
	IM_UINT32	bBlkTh;
	IM_UINT32	gbBlkTh;
}bc_context_t;

typedef struct{
	IM_UINT32	rGain;
	IM_UINT32	bGain;
	IM_UINT32	grGain;
	IM_UINT32	gbGain;
}cb_context_t;

typedef struct{
	IM_BOOL			bcEnable;
	IM_BOOL			cbEnable;
	IM_UINT32		bccbMode;	//ISP_BCCB_MODE_XXX
	bc_context_t	bc;
	cb_context_t	cb;
}bccb_config_t;

typedef struct{
	IM_BOOL			bcEnable;
	IM_BOOL			cbEnable;
	IM_UINT32 		bccbMode;	//ISP_BCCB_MODE_XXX
	bc_context_t	bc;
	cb_context_t	cb;	
	IM_UINT32		*regVal;
	IM_UINT32		*regOfst;
}isp_bccb_context_t;

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET bccb_init(isp_bccb_context_t *bccb, bccb_config_t *cfg);

IM_RET bccb_set_mode(isp_bccb_context_t *bccb, IM_UINT32 mode);
IM_RET bccb_set_bc(isp_bccb_context_t *bccb, bc_context_t *bc);
IM_RET bccb_set_cb(isp_bccb_context_t *bccb, cb_context_t *cb);
IM_RET bccb_set_bc_enable(isp_bccb_context_t *bccb);
IM_RET bccb_set_bc_disable(isp_bccb_context_t *bccb);
IM_RET bccb_set_cb_enable(isp_bccb_context_t *bccb);
IM_RET bccb_set_cb_disable(isp_bccb_context_t *bccb);

#endif	//_ISP_BCCB_H_
