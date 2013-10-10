/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_acc.h
--
--  Description :
--		acc module.
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

#ifndef _ISP_ACC_H_
#define	_ISP_ACC_H_

//acc LOOKUP TABLE offset
#define ISP_ACC_LUTB	0x6000

#define	ISP_ACC_LUTB_LENGTH	1792


typedef struct{
	IM_BOOL				enable;
	IM_BOOL				lutbModeNeedChange;
	IM_UINT32 			rdMode;	//rdMode = ISP_ROUND_XXX
	IM_INT32			coefe;
	isp_acc_lutb_type	lutbType;	
	isp_acc_hist_t		hist;
	isp_acc_co_matrix 	coMat;
	isp_acc_ro_matrix	roMat;
	
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_acc_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET acc_init(isp_acc_context_t *acc, isp_acc_config_t *cfg);

IM_RET acc_set_round_mode(isp_acc_context_t *acc, IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET acc_set_lutb(isp_acc_context_t *acc, isp_acc_lutb_type *lutbType);
IM_RET acc_set_hist(isp_acc_context_t *acc, isp_acc_hist_t *hist);
IM_RET acc_set_contrast_coef(isp_acc_context_t *acc, IM_INT32 coefe);
IM_RET acc_set_coef_matrix(isp_acc_context_t *acc, isp_acc_co_matrix *coMat);
IM_RET acc_set_ro_matrix(isp_acc_context_t *acc, isp_acc_ro_matrix *roMat);
IM_RET acc_set_enable(isp_acc_context_t *acc);
IM_RET acc_set_disable(isp_acc_context_t *acc);


#endif	//_ISP_ACC_H_
