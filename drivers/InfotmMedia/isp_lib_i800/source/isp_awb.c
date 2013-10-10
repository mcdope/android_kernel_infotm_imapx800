/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_awb.c
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

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_awb.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"AWB_I:"
#define WARNHEAD	"AWB_W:"
#define ERRHEAD		"AWB_E:"
#define TIPHEAD		"AWB_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: awb_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_init(isp_awb_context_t *awb, isp_awb_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check awb anaMode*/
	if(cfg->anaMode > ISP_AWB_ANALYZE_MODE_IMP)
	{
		IM_ERRMSG((IM_STR("Invalid awb analyze mode: anaMode = %d"), cfg->anaMode));
		return IM_RET_INVALID_PARAMETER;
	}
		
	/*check awb ROI position*/
	if((cfg->roiPos.x00 < 0) || (cfg->roiPos.x00 > 4095)
		|| (cfg->roiPos.x01 < 0) || (cfg->roiPos.x01 > 4095)
		|| (cfg->roiPos.y00 < 0) || (cfg->roiPos.y00 > 4095)
		|| (cfg->roiPos.y01 < 0) || (cfg->roiPos.y01 > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: x00=%d, x01=%d, y00=%d, y01=%d"), 
			cfg->roiPos.x00, cfg->roiPos.x01, cfg->roiPos.y00, cfg->roiPos.y01));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check awb threshold*/
	if((cfg->thrMat.thy1 < 0) || (cfg->thrMat.thy1 > 4095)
		|| (cfg->thrMat.thy2 < 0) || (cfg->thrMat.thy2 > 4095)
		|| (cfg->thrMat.costh < 0) || (cfg->thrMat.costh > 4095)
		|| (cfg->thrMat.errth < 0) || (cfg->thrMat.errth > 4095)
		|| (cfg->thrMat.coef1 < 0) || (cfg->thrMat.coef1 > 4095)
		|| (cfg->thrMat.cosbth < 0) || (cfg->thrMat.cosbth > 4095)
		|| (cfg->thrMat.cbAmp < 0) || (cfg->thrMat.cbAmp > 31)
		|| (cfg->thrMat.crAmp < 0) || (cfg->thrMat.crAmp > 31)
		|| (cfg->thrMat.meanth < 0) || (cfg->thrMat.meanth > 31))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: thy1=%d, thy2=%d, costh=%d, errth=%d, coef1=%d, cosbth=%d, cbAmp=%d, crAmp=%d, meanth=%d"), 
			cfg->thrMat.thy1, cfg->thrMat.thy2, cfg->thrMat.costh, cfg->thrMat.errth, cfg->thrMat.coef1, cfg->thrMat.cosbth, cfg->thrMat.cbAmp, cfg->thrMat.crAmp, cfg->thrMat.meanth));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check awb PAR thrMat*/
	if((cfg->parMat.bpDown < 0) || (cfg->parMat.bpDown > 511)
		|| (cfg->parMat.bpUp < 0) || (cfg->parMat.bpUp > 511)
		|| (cfg->parMat.rpDown < 0) || (cfg->parMat.rpDown > 511)
		|| (cfg->parMat.rpUp < 0) || (cfg->parMat.rpUp > 511))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: bpDown=%d, bpUp=%d, rpDown=%d, rpUp=%d"), 
			cfg->parMat.bpDown, cfg->parMat.bpUp, cfg->parMat.rpDown, cfg->parMat.rpUp));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check awb frame pixnum*/
	if((awb->framePixNum < 0) || (awb->framePixNum > 4096*4096))
	{
		IM_ERRMSG((IM_STR("Invalid frame pixnum: awb->framePixNum=%d"), awb->framePixNum));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((cfg->enable == IM_TRUE) && (cfg->anaEnable == IM_TRUE))
	{	
		/*set awb anaMode*/
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_MODE, cfg->anaMode);
		
		/*set awb ROI position*/
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X00, cfg->roiPos.x00);
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X01, cfg->roiPos.x01);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y00, cfg->roiPos.y00);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y01, cfg->roiPos.y01);
		
		/*set awb threshold*/
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y1, cfg->thrMat.thy1);
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y2, cfg->thrMat.thy2);
		SetIspRegister(awb->regVal, ISP_AWB_THE_COSTH, cfg->thrMat.costh);
		SetIspRegister(awb->regVal, ISP_AWB_THE_ERR, cfg->thrMat.errth);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COEF1, cfg->thrMat.coef1);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COSBTH, cfg->thrMat.cosbth);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CBAMP, cfg->thrMat.cbAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CRAMP, cfg->thrMat.crAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_MEANTH, cfg->thrMat.meanth);

		/*set awb PAR thrMat*/
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_DOWN, cfg->parMat.bpDown);
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_UP, cfg->parMat.bpUp);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_DOWN, cfg->parMat.rpDown);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_UP, cfg->parMat.rpUp);

		/*set awb frame pixnum*/
		SetIspRegister(awb->regVal, ISP_AWB_MXN, awb->framePixNum);

		/*set awb and awbana enable*/
		SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBBYPASS, 0);
		SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBABYPASS, 0);
	
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MXN], awb->regVal[rISP_AWB_MXN]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_TH1], awb->regVal[rISP_AWB_TH1]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_THE], awb->regVal[rISP_AWB_THE]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_CONB], awb->regVal[rISP_AWB_CONB]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_RPAR], awb->regVal[rISP_AWB_RPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_BPAR], awb->regVal[rISP_AWB_BPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MODETH], awb->regVal[rISP_AWB_MODETH]));
	
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_XPAR], awb->regVal[rISP_AWB_XPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_YPAR], awb->regVal[rISP_AWB_YPAR]));

		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_GLB_CFG2], awb->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set awb and awbana enable or bypass state*/
		SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBBYPASS, (cfg->enable==IM_TRUE)?0:1);
		SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBABYPASS, (cfg->anaEnable==IM_TRUE)?0:1);
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_GLB_CFG2], awb->regVal[rISP_GLB_CFG2]));
	}

	awb->anaMode = cfg->anaMode;
	isppwl_memcpy((void*)(&awb->roiPos), (void *)(&cfg->roiPos), sizeof(isp_awb_roi_position));
	isppwl_memcpy((void*)(&awb->thrMat), (void *)(&cfg->thrMat), sizeof(isp_awb_thr_matrix));
	isppwl_memcpy((void*)(&awb->parMat), (void *)(&cfg->parMat), sizeof(isp_awb_par_matrix));
	awb->enable = cfg->enable;
	awb->anaEnable = cfg->anaEnable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_analyze_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_analyze_mode(isp_awb_context_t *awb, IM_UINT32 anaMode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);	
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);
	/*check awb anaMode*/
	if(anaMode > ISP_AWB_ANALYZE_MODE_IMP)
	{
		IM_ERRMSG((IM_STR("Invalid awb analyze mode: anaMode = %d"), anaMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((awb->anaMode == anaMode) /*&& (anaMode =! ISP_AWB_ANALYZE_MODE_ROI)*/)
	{
		IM_INFOMSG((IM_STR("awb anaMode has already been this anaMode")));
		return IM_RET_OK;	
	}		
	

	if((awb->enable == IM_TRUE)&&(awb->anaEnable == IM_TRUE))
	{		
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_MODE, anaMode);
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MODETH], awb->regVal[rISP_AWB_MODETH]));
	}
	awb->anaMode = anaMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_roi_position

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_roi_position(isp_awb_context_t *awb, isp_awb_roi_position *roiPos)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);

	if(roiPos == IM_NULL)
	{
		IM_ERRMSG((IM_STR("roiPos is NULL while anaMode is ROI")));
		return IM_RET_INVALID_PARAMETER;
	}
	/*check awb ROI position*/
	if((roiPos->x00 < 0) || (roiPos->x00 > 4095)
		|| (roiPos->x01 < 0) || (roiPos->x01 > 4095)
		|| (roiPos->y00 < 0) || (roiPos->x01 > 4095)
		|| (roiPos->y01 < 0) || (roiPos->x01 > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: x00=%d, x01=%d, y00=%d, y01=%d"), 
			roiPos->x00, roiPos->x01, roiPos->y00, roiPos->y01));
		return IM_RET_INVALID_PARAMETER;
	}

	if((awb->enable == IM_TRUE)&&(awb->anaEnable == IM_TRUE))
	{		
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X00, roiPos->x00);
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X01, roiPos->x01);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y00, roiPos->y00);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y01, roiPos->y01);
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_XPAR], awb->regVal[rISP_AWB_XPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_YPAR], awb->regVal[rISP_AWB_YPAR]));
	}
	
	isppwl_memcpy((void*)(&awb->roiPos), (void *)(roiPos), sizeof(isp_awb_roi_position));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_threshold_matrix(isp_awb_context_t *awb, isp_awb_thr_matrix *thrMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);
	IM_ASSERT(thrMat != IM_NULL);

	/*check awb threshold*/
	if((thrMat->thy1 < 0) || (thrMat->thy1 > 4095)
		|| (thrMat->thy2 < 0) || (thrMat->thy2 > 4095)
		|| (thrMat->costh < 0) || (thrMat->costh > 4095)
		|| (thrMat->errth < 0) || (thrMat->errth > 4095)
		|| (thrMat->coef1 < 0) || (thrMat->coef1 > 4095)
		|| (thrMat->cosbth < 0) || (thrMat->cosbth > 4095)
		|| (thrMat->cbAmp < 0) || (thrMat->cbAmp > 31)
		|| (thrMat->crAmp < 0) || (thrMat->crAmp > 31)
		|| (thrMat->meanth < 0) || (thrMat->meanth > 31))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: thy1=%d, thy2=%d, costh=%d, errth=%d, coef1=%d, cosbth=%d, cbAmp=%d, crAmp=%d, meanth=%d"), 
			thrMat->thy1, thrMat->thy2, thrMat->costh, thrMat->errth, thrMat->coef1, thrMat->cosbth, thrMat->cbAmp, thrMat->crAmp, thrMat->meanth));
		return IM_RET_INVALID_PARAMETER;
	}

	if((awb->enable == IM_TRUE)&&(awb->anaEnable == IM_TRUE))
	{
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y1, thrMat->thy1);
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y2, thrMat->thy2);
		SetIspRegister(awb->regVal, ISP_AWB_THE_COSTH, thrMat->costh);
		SetIspRegister(awb->regVal, ISP_AWB_THE_ERR, thrMat->errth);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COEF1, thrMat->coef1);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COSBTH, thrMat->cosbth);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CBAMP, thrMat->cbAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CRAMP, thrMat->crAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_MEANTH, thrMat->meanth);
		
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_TH1], awb->regVal[rISP_AWB_TH1]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_THE], awb->regVal[rISP_AWB_THE]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_CONB], awb->regVal[rISP_AWB_CONB]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MODETH], awb->regVal[rISP_AWB_MODETH]));
	}

	isppwl_memcpy((void*)(&awb->thrMat), (void *)(thrMat), sizeof(isp_awb_thr_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_par_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_par_matrix(isp_awb_context_t *awb, isp_awb_par_matrix *parMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);
	IM_ASSERT(parMat != IM_NULL);

	/*check awb PAR matrix*/
	if((parMat->bpDown < 0) || (parMat->bpDown > 511)
		|| (parMat->bpUp < 0) || (parMat->bpUp > 511)
		|| (parMat->rpDown < 0) || (parMat->rpDown > 511)
		|| (parMat->rpUp < 0) || (parMat->rpUp > 511))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: bpDown=%d, bpUp=%d, rpDown=%d, rpUp=%d"), 
			parMat->bpDown, parMat->bpUp, parMat->rpDown, parMat->rpUp));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((awb->enable == IM_TRUE)&&(awb->anaEnable == IM_TRUE))
	{
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_DOWN, parMat->bpDown);
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_UP, parMat->bpUp);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_DOWN, parMat->rpDown);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_UP, parMat->rpUp);
		
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_RPAR], awb->regVal[rISP_AWB_RPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_BPAR], awb->regVal[rISP_AWB_BPAR]));
	}

	isppwl_memcpy((void*)(&awb->parMat), (void *)(parMat), sizeof(isp_awb_par_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
/*------------------------------------------------------------------------------

    Function name: awb_get_result

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_get_result(isp_awb_context_t *awb, isp_awb_result_t *rsut)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);
	IM_ASSERT(rsut != IM_NULL);

	IM_JIF(isppwl_read_reg(awb->regOfst[rISP_AWB_RGAIN], &awb->regVal[rISP_AWB_RGAIN]));
	IM_JIF(isppwl_read_reg(awb->regOfst[rISP_AWB_GGAIN], &awb->regVal[rISP_AWB_GGAIN]));
	IM_JIF(isppwl_read_reg(awb->regOfst[rISP_AWB_BGAIN], &awb->regVal[rISP_AWB_BGAIN]));
	IM_JIF(isppwl_read_reg(awb->regOfst[rISP_AWB_CBSUM], &awb->regVal[rISP_AWB_CBSUM]));
	IM_JIF(isppwl_read_reg(awb->regOfst[rISP_AWB_CRSUM], &awb->regVal[rISP_AWB_CRSUM]));
	
	awb->rsut.rGain = GetIspRegister(awb->regVal, ISP_AWB_RGAIN);
	awb->rsut.gGain = GetIspRegister(awb->regVal, ISP_AWB_GGAIN);
	awb->rsut.bGain = GetIspRegister(awb->regVal, ISP_AWB_BGAIN);
	awb->rsut.cbSum = GetIspRegister(awb->regVal, ISP_AWB_CBSUM);
	awb->rsut.crSum = GetIspRegister(awb->regVal, ISP_AWB_CRSUM);
	rsut->rGain = awb->rsut.rGain;
	rsut->gGain = awb->rsut.gGain;
	rsut->bGain = awb->rsut.bGain;
	rsut->cbSum = awb->rsut.cbSum;
	rsut->crSum = awb->rsut.crSum;
		
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_enable(isp_awb_context_t *awb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);

	if((awb->enable == IM_TRUE)&&(awb->anaEnable == IM_TRUE))
	{
		IM_INFOMSG((IM_STR("awb has been enabled")));
		return IM_RET_OK;	
	}

	/*set awb anaMode*/
	SetIspRegister(awb->regVal, ISP_AWB_MODETH_MODE, awb->anaMode);
	if(awb->anaMode == ISP_AWB_ANALYZE_MODE_ROI)
	{
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X00, awb->roiPos.x00);
		SetIspRegister(awb->regVal, ISP_AWB_XPAR_X01, awb->roiPos.x01);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y00, awb->roiPos.y00);
		SetIspRegister(awb->regVal, ISP_AWB_YPAR_Y01, awb->roiPos.y01);
	}
	/*if(awb->anaMode == ISP_AWB_ANALYZE_MODE_IMP)
	{*/
		/*set awb threshold*/
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y1, awb->thrMat.thy1);
		SetIspRegister(awb->regVal, ISP_AWB_TH1_Y2, awb->thrMat.thy2);
		SetIspRegister(awb->regVal, ISP_AWB_THE_COSTH, awb->thrMat.costh);
		SetIspRegister(awb->regVal, ISP_AWB_THE_ERR, awb->thrMat.errth);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COEF1, awb->thrMat.coef1);
		SetIspRegister(awb->regVal, ISP_AWB_CONB_COSBTH, awb->thrMat.cosbth);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CBAMP, awb->thrMat.cbAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_CRAMP, awb->thrMat.crAmp);
		SetIspRegister(awb->regVal, ISP_AWB_MODETH_MEANTH, awb->thrMat.meanth);

		/*set awb PAR matrix*/
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_DOWN, awb->parMat.bpDown);
		SetIspRegister(awb->regVal, ISP_AWB_BPAR_UP, awb->parMat.bpUp);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_DOWN, awb->parMat.rpDown);
		SetIspRegister(awb->regVal, ISP_AWB_RPAR_UP, awb->parMat.rpUp);
	/*}*/

	/*set awb frame pixnum*/
	SetIspRegister(awb->regVal, ISP_AWB_MXN, awb->framePixNum);

	/*set awb and awbana enable*/
	SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBBYPASS, 0);
	SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBABYPASS, 0);
	
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MXN], awb->regVal[rISP_AWB_MXN]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_TH1], awb->regVal[rISP_AWB_TH1]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_THE], awb->regVal[rISP_AWB_THE]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_CONB], awb->regVal[rISP_AWB_CONB]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_RPAR], awb->regVal[rISP_AWB_RPAR]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_BPAR], awb->regVal[rISP_AWB_BPAR]));
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_MODETH], awb->regVal[rISP_AWB_MODETH]));
	if(awb->anaMode == ISP_AWB_ANALYZE_MODE_ROI)
	{
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_XPAR], awb->regVal[rISP_AWB_XPAR]));
		IM_JIF(isppwl_write_reg(awb->regOfst[rISP_AWB_YPAR], awb->regVal[rISP_AWB_YPAR]));
	}
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_GLB_CFG2], awb->regVal[rISP_GLB_CFG2]));

	awb->enable = IM_TRUE;
	awb->anaEnable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: awb_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET awb_set_disable(isp_awb_context_t *awb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(awb != IM_NULL);
	IM_ASSERT(awb->regVal != IM_NULL);
	IM_ASSERT(awb->regOfst != IM_NULL);

	if((awb->enable != IM_TRUE) && (awb->anaEnable != IM_TRUE))
	{
		IM_INFOMSG((IM_STR("awb has been disabled")));
		return IM_RET_OK;	
	}
	//disable awb
	SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBBYPASS, 1);
	SetIspRegister(awb->regVal, ISP_GLB_CFG2_AWBABYPASS, 1);
	IM_JIF(isppwl_write_reg(awb->regOfst[rISP_GLB_CFG2], awb->regVal[rISP_GLB_CFG2]));

	awb->enable = IM_FALSE;
	awb->anaEnable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

