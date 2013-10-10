/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_scl.h
--
--  Description :
--		scaler module.
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

#ifndef _ISP_SCL_H_
#define	_ISP_SCL_H_


typedef struct{
	IM_UINT32		inWidth;
	IM_UINT32		inHeight;
}scl_input_t;

typedef struct{
	IM_UINT32		outWidth;
	IM_UINT32		outHeight;
	IM_IMAGE_TYPE	outFormat;	//IM_IMAGE_XXX
}scl_output_t;

typedef struct{
	IM_BOOL			verEnable;
	IM_BOOL			horEnable;
	IM_UINT32		vrdMode;	//ISP_ROUND_XXX
	IM_UINT32		hrdMode;	//ISP_ROUND_XXX
	scl_input_t		inPut;
	scl_output_t	outPut;
	//IM_UINT32		paramType;	//ISP_SCL_PARAM_TYPE_XXX
	//IM_UINT32		cscMode;	//ISP_SCL_CSC_MODE_X
}scl_config_t;

typedef struct{
	IM_UINT32		vscaling;
	IM_UINT32		hscaling;
	IM_BOOL			verEnable;
	IM_BOOL			horEnable;
	IM_UINT32		vrdMode;	//ISP_ROUND_XXX
	IM_UINT32		hrdMode;	//ISP_ROUND_XXX
	scl_input_t		inPut;
	scl_output_t	outPut;
	IM_UINT32		paramType;	//ISP_SCL_PARAM_TYPE_XXX
	IM_UINT32		cscMode;	//ISP_SCL_CSC_MODE_X

	//inter val
	IM_UINT32		endianFormat;	
	IM_UINT32		halfWordSwap;
	IM_UINT32		byteSwap;
	IM_UINT32		isRGB;
	IM_UINT32		coFmt;
	IM_UINT32		stFmt;
	IM_UINT32		*regVal;
	IM_UINT32		*regOfst;
}isp_scl_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET scl_init(isp_scl_context_t *scl, scl_config_t *cfg);

IM_RET scl_set_input(isp_scl_context_t *scl, scl_input_t *inPut);
IM_RET scl_set_output(isp_scl_context_t *scl, scl_output_t *outPut);
IM_RET scl_set_round_mode(isp_scl_context_t *scl, IM_UINT32 vrdMode, IM_UINT32 hrdMode);
IM_RET scl_set_param_type(isp_scl_context_t *scl, IM_UINT32 paramType);
IM_RET scl_set_csc_mode(isp_scl_context_t *scl, IM_UINT32 cscMode);
IM_RET scl_set_enable(isp_scl_context_t *scl, IM_BOOL verEnable, IM_BOOL horEnable);

#endif	//_ISP_SCL_H_
