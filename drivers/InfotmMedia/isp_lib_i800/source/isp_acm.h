/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_acm.h
--
--  Description :
--		acm module.
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

#ifndef _ISP_ACM_H_
#define	_ISP_ACM_H_


typedef struct{
	IM_BOOL					enable;
	IM_UINT32				rdMode;	//ISP_ROUND_XXX
	IM_INT32				ths;
	isp_acm_thr_matrix		thrMat;
	isp_acm_coef_matrix		coMat;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_acm_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET acm_init(isp_acm_context_t *acm, isp_acm_config_t *cfg);

IM_RET acm_set_round_mode(isp_acm_context_t *acm, IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET acm_set_saturation_threshold(isp_acm_context_t *acm, IM_INT32 ths);
IM_RET acm_set_threshold_matrix(isp_acm_context_t *acm, isp_acm_thr_matrix *thrMat);
IM_RET acm_set_coef_matrix(isp_acm_context_t *acm, isp_acm_coef_matrix *coMat);
IM_RET acm_set_enable(isp_acm_context_t *acm);
IM_RET acm_set_disable(isp_acm_context_t *acm);


#endif	//_ISP_ACM_H_
