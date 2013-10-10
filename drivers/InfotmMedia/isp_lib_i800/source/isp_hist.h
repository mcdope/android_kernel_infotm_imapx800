/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_hist.h
--
--  Description :
--		hist and backlit module.
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

#ifndef _ISP_HIST_H_
#define	_ISP_HIST_H_

typedef struct{
	IM_BOOL						enable;
	IM_UINT32					blitTh1;
	isp_hist_thr_matrix			thrMat;
}hist_config_t;

typedef struct{
	IM_BOOL					enable;
	IM_UINT32				blitTh1;
	isp_hist_thr_matrix		thrMat;
	isp_hist_result_t		rsut;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_hist_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET hist_init(isp_hist_context_t *hist, hist_config_t *cfg);

IM_RET hist_set_backlit_threshold(isp_hist_context_t *hist, IM_UINT32 blitTh1);
IM_RET hist_set_hist_threshold_matrix(isp_hist_context_t *hist, isp_hist_thr_matrix *thrMat);
IM_RET hist_get_result(isp_hist_context_t *hist, isp_hist_result_t *rsut);
IM_RET hist_set_enable(isp_hist_context_t *hist);
IM_RET hist_set_disable(isp_hist_context_t *hist);


#endif	//_ISP_HIST_H_
