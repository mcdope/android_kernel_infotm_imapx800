/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_awb.h
--
--  Description :
--		awb module.
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

#ifndef _ISP_AWB_H_
#define	_ISP_AWB_H_

typedef struct{
	IM_BOOL					enable;
	IM_BOOL					anaEnable;
	IM_UINT32				anaMode;			//ISP_AWB_ANALYZE_MODE_XXX
	IM_UINT32				framePixNum;
	isp_awb_roi_position	roiPos;
	isp_awb_thr_matrix		thrMat;
	isp_awb_par_matrix		parMat;
	isp_awb_result_t		rsut;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_awb_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET awb_init(isp_awb_context_t *awb, isp_awb_config_t *cfg);

/*if anaMode = ISP_AWB_ANALYZE_MODE_GWD, roiPos, thrMat and parMat all shonot set value,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos need to set value, but thrMat and parMat should not ,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos should not set value, but thrMat and parMat need set value.*/
IM_RET awb_set_analyze_mode(isp_awb_context_t *awb, IM_UINT32 anaMode);
IM_RET awb_set_roi_position(isp_awb_context_t *awb, isp_awb_roi_position *roiPos);
IM_RET awb_set_threshold_matrix(isp_awb_context_t *awb, isp_awb_thr_matrix *thrMat);
IM_RET awb_set_par_matrix(isp_awb_context_t *awb, isp_awb_par_matrix *parMat);
IM_RET awb_get_result(isp_awb_context_t *awb, isp_awb_result_t *rsut);
IM_RET awb_set_enable(isp_awb_context_t *awb);
IM_RET awb_set_disable(isp_awb_context_t *awb);


#endif	//_ISP_AWB_H_
