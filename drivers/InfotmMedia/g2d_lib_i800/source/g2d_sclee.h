/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_sclee.h
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __G2D_SCLEE_H_
#define	__G2D_SCLEE_H_


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET sclee_set_config(SCLEE *sclee, G2D_SCLEE *scleeCfg);
IM_RET sclee_set_scl_size(SCLEE *sclee, IM_UINT32 inWidth, 
								IM_UINT32 inHeight, IM_UINT32 outWidth, IM_UINT32 outHeight);
IM_RET sclee_set_scl_round_mode(SCLEE *sclee, IM_UINT32 vrdMode, IM_UINT32 hrdMode);
IM_RET sclee_set_scl_param_type(SCLEE *sclee, IM_UINT32 paramType);	//paramType = G2D_SCL_PARAM_TYPE_XXX
IM_RET sclee_set_scl_enable(SCLEE *sclee, IM_BOOL verEnable, IM_BOOL horEnable);

IM_RET sclee_set_order(SCLEE *sclee, IM_UINT32 order);	//order = G2D_ORDER_XXX
IM_RET sclee_set_ee_coefw(SCLEE *sclee, IM_UINT32 coefw);
IM_RET sclee_set_ee_coefa(SCLEE *sclee, IM_UINT32 coefa);
IM_RET sclee_set_ee_round_mode(SCLEE *sclee, IM_UINT32 rdMode);
IM_RET sclee_set_ee_gauss_mode(SCLEE *sclee, IM_UINT32 gasMode); 		//gasMode = G2D_EE_GAUSS_MODE_XXX
IM_RET sclee_set_ee_operator_type(SCLEE *sclee, IM_UINT32 opMatType);   //opMatType = G2D_EE_OPMAT_TYPE_XXX
IM_RET sclee_set_ee_error_threshold(SCLEE *sclee, IM_UINT32 errTh);
IM_RET sclee_set_ee_detect_threshold_matrix(SCLEE *sclee, G2D_EE_TH_MATRIX *matrix);
IM_RET sclee_set_ee_denoise_enable(SCLEE *sclee, IM_BOOL denosEnable);
IM_RET sclee_set_ee_enable(SCLEE *sclee, IM_BOOL eeEnable);

#endif	//__G2D_SCLEE_H_
