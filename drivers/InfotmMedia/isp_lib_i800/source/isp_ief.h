/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ief.h
--
--  Description :
--		ief module.
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

#ifndef _ISP_IEF_H_
#define	_ISP_IEF_H_


typedef struct{
	IM_BOOL					enable;
	IM_BOOL					cscEn;
	IM_BOOL					cscModeNeedChange;
	IM_UINT32				type;	//ISP_IEF_TYPE_XXX
	isp_ief_rgcf_matrix		rgcfMat;
	isp_ief_select_matrix	selMat;
	isp_ief_csc_matrix		cscMat;
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_ief_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET ief_init(isp_ief_context_t *ief, isp_ief_config_t *cfg);

IM_RET ief_set_type(isp_ief_context_t *ief, IM_UINT32 type);	//type = ISP_IEF_TYPE_XXX
IM_RET ief_set_rgcf_matrix(isp_ief_context_t *ief, isp_ief_rgcf_matrix *rgcfMat);
IM_RET ief_set_color_select_mode_matrix(isp_ief_context_t *ief, isp_ief_select_matrix *selMat);
IM_RET ief_set_csc_matrix(isp_ief_context_t *ief, isp_ief_csc_matrix *cscMat);
IM_RET ief_set_csc_enable(isp_ief_context_t *ief);
IM_RET ief_set_csc_disable(isp_ief_context_t *ief);
IM_RET ief_set_enable(isp_ief_context_t *ief);
IM_RET ief_set_disable(isp_ief_context_t *ief);

#endif	//_ISP_IEF_H_
