/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_hist.c
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

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_hist.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"HIST_I:"
#define WARNHEAD	"HIST_W:"
#define ERRHEAD		"HIST_E:"
#define TIPHEAD		"HIST_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: hist_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_init(isp_hist_context_t *hist, hist_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check hist and backlit threshold*/
	if((cfg->blitTh1 < 0) || (cfg->blitTh1 > 16777215)
		|| (cfg->thrMat.th1 < 0) || (cfg->thrMat.th1 > 127)
		|| (cfg->thrMat.th2 < 0) || (cfg->thrMat.th2 > 255))
	{
		IM_ERRMSG((IM_STR("Invalid threshold: blitTh1=%d, th1=%d, th2=%d"), 
			cfg->blitTh1, cfg->thrMat.th1, cfg->thrMat.th2));
		return IM_RET_INVALID_PARAMETER;
	}


	if(cfg->enable == IM_TRUE)
	{
		/*set hist and backlit threshold*/
		SetIspRegister(hist->regVal, ISP_BLIT_TH1, cfg->blitTh1);
		SetIspRegister(hist->regVal, ISP_HIST_TH_1, cfg->thrMat.th1);
		SetIspRegister(hist->regVal, ISP_HIST_TH_2, cfg->thrMat.th2);
		/*set hist enable*/
		SetIspRegister(hist->regVal, ISP_GLB_CFG2_HISTBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_BLIT_TH], hist->regVal[rISP_BLIT_TH]));
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_HIST_TH], hist->regVal[rISP_HIST_TH]));
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_GLB_CFG2], hist->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set hist bypass*/
		SetIspRegister(hist->regVal, ISP_GLB_CFG2_HISTBYPASS, 1);
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_GLB_CFG2], hist->regVal[rISP_GLB_CFG2]));
	}
	hist->blitTh1 = cfg->blitTh1;
	isppwl_memcpy((void*)(&hist->thrMat), (void *)(&cfg->thrMat), sizeof(isp_hist_thr_matrix));
	hist->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: hist_set_hist_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_set_backlit_threshold(isp_hist_context_t *hist, IM_UINT32 blitTh1)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);

	/*check hist threshold*/
	if((blitTh1 < 0) || (blitTh1 > 16777215))
	{
		IM_ERRMSG((IM_STR("Invalid backlit threshold: blitTh1=%d"), blitTh1));
		return IM_RET_INVALID_PARAMETER;
	}

	if(hist->enable == IM_TRUE)
	{
		/*set backlit threshold*/
		SetIspRegister(hist->regVal, ISP_BLIT_TH1, blitTh1);
		
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_BLIT_TH], hist->regVal[rISP_BLIT_TH]));
	}

	hist->blitTh1 = blitTh1;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: hist_set_hist_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_set_hist_threshold_matrix(isp_hist_context_t *hist, isp_hist_thr_matrix *thrMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);
	IM_ASSERT(thrMat != IM_NULL);

	/*check hist threshold*/
	if((thrMat->th1 < 0) || (thrMat->th1 > 127)
		|| (thrMat->th2 < 0) || (thrMat->th2 > 255))
	{
		IM_ERRMSG((IM_STR("Invalid threshold:  th1=%d, th2=%d"), 
			 thrMat->th1, thrMat->th2));
		return IM_RET_INVALID_PARAMETER;
	}

	if(hist->enable == IM_TRUE)
	{
		/*set hist threshold*/ 
		SetIspRegister(hist->regVal, ISP_HIST_TH_1, thrMat->th1);
		SetIspRegister(hist->regVal, ISP_HIST_TH_2, thrMat->th2);
		 
		IM_JIF(isppwl_write_reg(hist->regOfst[rISP_HIST_TH], hist->regVal[rISP_HIST_TH]));
	}

	isppwl_memcpy((void*)(&hist->thrMat), (void *)(thrMat), sizeof(isp_hist_thr_matrix));	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: hist_get_result

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_get_result(isp_hist_context_t *hist, isp_hist_result_t *rsut)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);
	IM_ASSERT(rsut != IM_NULL);

	IM_JIF(isppwl_read_reg(hist->regOfst[rISP_BACK_LIT], &hist->regVal[rISP_BACK_LIT]));
	hist->rsut.bkLit = GetIspRegister(hist->regVal, ISP_BACK_LIT);
	rsut->bkLit = hist->rsut.bkLit;
	
	for(i=0; i<16; i++)
	{
		IM_JIF(isppwl_read_reg(hist->regOfst[rISP_HIST_COUNTER1 + i], &hist->regVal[rISP_HIST_COUNTER1 + i]));
		hist->rsut.hist[i] = GetIspRegister(hist->regVal, ISP_HIST_COUNTER1+i);
		rsut->hist[i] = hist->rsut.hist[i];
	}
		
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: hist_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_set_enable(isp_hist_context_t *hist)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);

	if(hist->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("hist has been enabled")));
		return IM_RET_OK;	
	}
	//enable hist 
	/*set hist and backlit threshold*/
	SetIspRegister(hist->regVal, ISP_BLIT_TH1, hist->blitTh1);
	SetIspRegister(hist->regVal, ISP_HIST_TH_1, hist->thrMat.th1);
	SetIspRegister(hist->regVal, ISP_HIST_TH_2, hist->thrMat.th2);
	/*set hist enable*/
	SetIspRegister(hist->regVal, ISP_GLB_CFG2_HISTBYPASS, 0);

	IM_JIF(isppwl_write_reg(hist->regOfst[rISP_BLIT_TH], hist->regVal[rISP_BLIT_TH]));
	IM_JIF(isppwl_write_reg(hist->regOfst[rISP_HIST_TH], hist->regVal[rISP_HIST_TH]));
	IM_JIF(isppwl_write_reg(hist->regOfst[rISP_GLB_CFG2], hist->regVal[rISP_GLB_CFG2]));

	hist->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: hist_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET hist_set_disable(isp_hist_context_t *hist)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(hist != IM_NULL);
	IM_ASSERT(hist->regVal != IM_NULL);
	IM_ASSERT(hist->regOfst != IM_NULL);

	if(hist->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("hist has been disabled")));
		return IM_RET_OK;	
	}
	//disable hist
	SetIspRegister(hist->regVal, ISP_GLB_CFG2_HISTBYPASS, 1);
	IM_JIF(isppwl_write_reg(hist->regOfst[rISP_GLB_CFG2], hist->regVal[rISP_GLB_CFG2]));

	hist->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
