/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ee.h
--
--  Description :
--		ee module.
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

#ifndef _ISP_EE_H_
#define	_ISP_EE_H_


typedef struct{
	IM_BOOL				enable;
	IM_UINT32			coefw;
	IM_UINT32			coefa;
	IM_UINT32			rdMode;	//ISP_ROUND_XXX
	IM_BOOL				gasEn;
	IM_UINT32			gasMode;
	IM_UINT32			errTh;
	isp_ee_thr_matrix	thrMat;
	//isp_ee_op_matrix	hm;
	//isp_ee_op_matrix	vm;
	//isp_ee_op_matrix	d0m;
	//isp_ee_op_matrix	d1m;
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_ee_context_t;



/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET ee_init(isp_ee_context_t *ee, isp_ee_config_t *cfg);

IM_RET ee_set_coefw(isp_ee_context_t *ee, IM_UINT32 coefw);
IM_RET ee_set_coefa(isp_ee_context_t *ee, IM_UINT32 coefa);
IM_RET ee_set_round_mode(isp_ee_context_t *ee, IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET ee_set_gauss_filter_enable(isp_ee_context_t *ee, IM_BOOL gasEn);
IM_RET ee_set_gauss_mode(isp_ee_context_t *ee, IM_UINT32 gasMode);
IM_RET ee_set_error_threshold(isp_ee_context_t *ee, IM_UINT32 errTh);
IM_RET ee_set_detect_threshold_matrix(isp_ee_context_t *ee, isp_ee_thr_matrix *thrMat);
IM_RET ee_set_edge_operator_matrix(isp_ee_context_t *ee, IM_UINT32 direction, isp_ee_op_matrix *opMat);//direction = ISP_EE_DIRECTION_XXX
IM_RET ee_set_enable(isp_ee_context_t *ee);
IM_RET ee_set_disable(isp_ee_context_t *ee);


#endif	//_ISP_EE_H_
