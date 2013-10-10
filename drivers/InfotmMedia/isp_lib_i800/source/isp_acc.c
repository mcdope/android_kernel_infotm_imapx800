/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_acc.c
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

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_acc.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"ACC_I:"
#define WARNHEAD	"ACC_W:"
#define ERRHEAD		"ACC_E:"
#define TIPHEAD		"ACC_T:"



/**************************************************
*          isp acc look-up table coef             *
**************************************************/
static const IM_UINT32 accLutbCoef[][ISP_ACC_LUTB_LENGTH] = {
#include "isp_acc_table.h"
};

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: acc_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_init(isp_acc_context_t *acc, isp_acc_config_t *cfg)
{ 
	IM_UINT32 lutb_offset;
	IM_UINT32 i;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
		
	/*check acc round mode*/
	if(cfg->rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acc round mode error: rdMode = %d"), cfg->rdMode));
		return IM_RET_INVALID_PARAMETER;
	}

	//check look-up table control mode
	if(cfg->lutbType.lutbCtrMode > ISP_ACC_LUTB_CONTROL_MODE_CPU)
	{
		IM_ERRMSG((IM_STR("acc look-up table control mode error: mode = %d"), cfg->lutbType.lutbCtrMode));
		return IM_RET_INVALID_PARAMETER;
	}

	//check look-up table coef mode
	if(cfg->lutbType.lutbMode > ISP_ACC_LUTB_MODE_0)
	{
		IM_ERRMSG((IM_STR("acc look-up table coef mode error: mode = %d"), cfg->lutbType.lutbMode));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check acc hist*/
	if(cfg->hist.histRdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acc hist round mode error: rdMode = %d"), cfg->hist.histRdMode));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((cfg->hist.histFrames < 0) || (cfg->hist.histFrames > 31))
	{
		IM_ERRMSG((IM_STR("Invalid hist frames: nframes=%d"), cfg->hist.histFrames));
		return IM_RET_INVALID_PARAMETER;
	}

	//check acc contrast coefe
	if((cfg->coefe < -32768) || (cfg->coefe > 32767))
	{
		IM_ERRMSG((IM_STR("Invalid contrast coef :coefe=%d"), cfg->coefe));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check acc coef matrix*/
	if((cfg->coMat.coefa < 0) || (cfg->coMat.coefa > 15)
		|| (cfg->coMat.coefb < 0) || (cfg->coMat.coefb > 15)
		|| (cfg->coMat.coefc < 0) || (cfg->coMat.coefc > 15)
		|| (cfg->coMat.coefd < 0) || (cfg->coMat.coefd > 15))
	{
		IM_ERRMSG((IM_STR("Invalid roMat value: coa=%d, cob=%d, coc=%d, cod=%d"), 
			cfg->coMat.coefa, cfg->coMat.coefb, cfg->coMat.coefc, cfg->coMat.coefd));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check acc ro matrix*/
	if((cfg->roMat.roaHi < 0) || (cfg->roMat.roaHi > 255)
		|| (cfg->roMat.roaLo < 0) || (cfg->roMat.roaLo > 255)
		|| (cfg->roMat.robHi < 0) || (cfg->roMat.robHi > 255)
		|| (cfg->roMat.robLo < 0) || (cfg->roMat.robLo > 255)
		|| (cfg->roMat.rocHi < 0) || (cfg->roMat.rocHi > 255)
		|| (cfg->roMat.rocLo < 0) || (cfg->roMat.rocLo > 255))
	{
		IM_ERRMSG((IM_STR("Invalid roMat value: roaHi=%d, roaLo=%d, robHi=%d, robLo=%d, rocHi=%d, rocLo=%d"), 
			cfg->roMat.roaHi, cfg->roMat.roaLo, cfg->roMat.robHi, cfg->roMat.robLo, cfg->roMat.rocHi, cfg->roMat.rocLo));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set look-up table control mode*/
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_APBS, cfg->lutbType.lutbCtrMode);
		//look-up table not ready
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 0);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));

		for(i=0; i<ISP_ACC_LUTB_LENGTH; i++)
		{
			lutb_offset = ISP_ACC_LUTB + i*4;
			IM_JIF(isppwl_write_reg(lutb_offset, accLutbCoef[cfg->lutbType.lutbMode][i]));
		}
		//look-up table ready
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 1);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
		acc->lutbModeNeedChange = IM_FALSE;

		/*set acc round mode*/
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_ROUND, cfg->rdMode);

		/*set acc hist*/
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_HISTROUND, cfg->hist.histRdMode);
		SetIspRegister(acc->regVal, ISP_ACC_HISA_HISTFRAMES, cfg->hist.histFrames);

		//set acc contrast coefe
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_COE, cfg->coefe);
		
		/*set acc coef matrix*/
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COA, cfg->coMat.coefa);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COB, cfg->coMat.coefb);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COC, cfg->coMat.coefc);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COD, cfg->coMat.coefd);

		/*set acc ro matrix*/
		SetIspRegister(acc->regVal, ISP_ACC_HISA_ROAHI, cfg->roMat.roaHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISA_ROALO, cfg->roMat.roaLo);
		SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBHI, cfg->roMat.robHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBLO, cfg->roMat.robLo);
		SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCHI, cfg->roMat.rocHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCLO, cfg->roMat.rocLo);
		
		/*set acc enable*/
		SetIspRegister(acc->regVal, ISP_GLB_CFG2_ACCBYPASS, 0);

		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_ABCD], acc->regVal[rISP_ACC_ABCD]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISA], acc->regVal[rISP_ACC_HISA]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISB], acc->regVal[rISP_ACC_HISB]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISC], acc->regVal[rISP_ACC_HISC]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_GLB_CFG2], acc->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set acc bypass*/
		SetIspRegister(acc->regVal, ISP_GLB_CFG2_ACCBYPASS, 1);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_GLB_CFG2], acc->regVal[rISP_GLB_CFG2]));
		acc->lutbModeNeedChange = IM_TRUE;
	}

	acc->rdMode = cfg->rdMode;
	acc->lutbType.lutbCtrMode = cfg->lutbType.lutbCtrMode;
	acc->lutbType.lutbMode = cfg->lutbType.lutbMode;
	acc->hist.histRdMode = cfg->hist.histRdMode;
	acc->hist.histFrames = cfg->hist.histFrames;
	acc->coefe = cfg->coefe;
	isppwl_memcpy((void*)(&acc->coMat), (void *)(&cfg->coMat), sizeof(isp_acc_co_matrix));
	isppwl_memcpy((void*)(&acc->roMat), (void *)(&cfg->roMat), sizeof(isp_acc_ro_matrix));
	acc->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_round_mode(isp_acc_context_t *acc, IM_UINT32 rdMode)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);

	/*check acc round mode*/
	if(rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acc round mode error: rdMode = %d"), rdMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acc->rdMode == rdMode)
	{
		IM_INFOMSG((IM_STR("acc round mode has already been this mode")));
		return IM_RET_OK;	
	}

	if(acc->enable == IM_TRUE)
	{
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_ROUND, rdMode);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
	}

	acc->rdMode = rdMode;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_lutb

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_lutb(isp_acc_context_t *acc, isp_acc_lutb_type *lutbType)
{ 
	IM_UINT32 lutb_offset;
	IM_UINT32 i;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);
	IM_ASSERT(lutbType != IM_NULL);

	//check look-up table control mode
	if(lutbType->lutbCtrMode > ISP_ACC_LUTB_CONTROL_MODE_CPU)
	{
		IM_ERRMSG((IM_STR("acc look-up table control mode error: mode = %d"), lutbType->lutbCtrMode));
		return IM_RET_INVALID_PARAMETER;
	}

	//check look-up table coef mode
	if(lutbType->lutbMode > ISP_ACC_LUTB_MODE_0)
	{
		IM_ERRMSG((IM_STR("acc look-up table coef mode error: mode = %d"), lutbType->lutbMode));
		return IM_RET_INVALID_PARAMETER;
	}

	
	if((acc->lutbModeNeedChange == IM_TRUE) || (acc->lutbType.lutbMode != lutbType->lutbMode))
	{
		acc->lutbModeNeedChange = IM_TRUE;
	}
	else
	{
		acc->lutbModeNeedChange = IM_FALSE;
	}

	/*set acc look-up table*/
	if(acc->enable == IM_TRUE)
	{
		//set look-up table control mode
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_APBS, lutbType->lutbCtrMode);
		if(acc->lutbModeNeedChange == IM_TRUE)
		{
			//look-up table not ready
			SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 0);
			IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));

			for(i=0; i<ISP_ACC_LUTB_LENGTH; i++)
			{
				lutb_offset = ISP_ACC_LUTB + i*4;
				IM_JIF(isppwl_write_reg(lutb_offset, accLutbCoef[lutbType->lutbMode][i]));
			}
			//look-up table ready
			SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 1);
			IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
			acc->lutbModeNeedChange = IM_FALSE;
		}
	}

	acc->lutbType.lutbCtrMode = lutbType->lutbCtrMode;
	acc->lutbType.lutbMode = lutbType->lutbMode;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_hist

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_hist(isp_acc_context_t *acc, isp_acc_hist_t *hist)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);
	IM_ASSERT(hist != IM_NULL);
	
	/*check acc hist*/
	if(hist->histRdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acc hist round mode error: rdMode = %d"), hist->histRdMode));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((hist->histFrames < 0) || (hist->histFrames > 31))
	{
		IM_ERRMSG((IM_STR("Invalid hist frames: nframes=%d"), hist->histFrames));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acc->enable == IM_TRUE)
	{
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_HISTROUND, hist->histRdMode);
		SetIspRegister(acc->regVal, ISP_ACC_HISA_HISTFRAMES, hist->histRdMode);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISA], acc->regVal[rISP_ACC_HISA]));
	}

	acc->hist.histRdMode = hist->histRdMode;
	acc->hist.histFrames = hist->histFrames;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_contrast_coef

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_contrast_coef(isp_acc_context_t *acc, IM_INT32 coefe)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);

	//check acc contrast coefe
	if((coefe < -32768) || (coefe > 32767))
	{
		IM_ERRMSG((IM_STR("Invalid contrast coef :coefe=%d"), coefe));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acc->enable == IM_TRUE)
	{
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_COE, coefe);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
	}

	acc->coefe = coefe;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_coef_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_coef_matrix(isp_acc_context_t *acc, isp_acc_co_matrix *coMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);
	IM_ASSERT(coMat != IM_NULL);
	
	/*check acc coef matrix*/
	if((coMat->coefa < 0) || (coMat->coefa > 15)
		|| (coMat->coefb < 0) || (coMat->coefb > 15)
		|| (coMat->coefc < 0) || (coMat->coefc > 15)
		|| (coMat->coefd < 0) || (coMat->coefd > 15))
	{
		IM_ERRMSG((IM_STR("Invalid coMat value: coa=%d, cob=%d, coc=%d, cod=%d"), 
			coMat->coefa, coMat->coefb, coMat->coefc, coMat->coefd));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(acc->enable == IM_TRUE)
	{
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COA, coMat->coefa);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COB, coMat->coefb);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COC, coMat->coefc);
		SetIspRegister(acc->regVal, ISP_ACC_ABCD_COD, coMat->coefd);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_ABCD], acc->regVal[rISP_ACC_ABCD]));
	}

	isppwl_memcpy((void*)(&acc->coMat), (void *)(coMat), sizeof(isp_acc_co_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_ro_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_ro_matrix(isp_acc_context_t *acc, isp_acc_ro_matrix *roMat)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
 
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);
	IM_ASSERT(roMat != IM_NULL);

	/*check acc ro roMat*/
	if((roMat->roaHi < 0) || (roMat->roaHi > 255)
		|| (roMat->roaLo < 0) || (roMat->roaLo > 255)
		|| (roMat->robHi < 0) || (roMat->robHi > 255)
		|| (roMat->robLo < 0) || (roMat->robLo > 255)
		|| (roMat->rocHi < 0) || (roMat->rocHi > 255)
		|| (roMat->rocLo < 0) || (roMat->rocLo > 255))
	{
		IM_ERRMSG((IM_STR("Invalid roMat value: roaHi=%d, roaLo=%d, robHi=%d, robLo=%d, rocHi=%d, rocLo=%d"), 
			roMat->roaHi, roMat->roaLo, roMat->robHi, roMat->robLo, roMat->rocHi, roMat->rocLo));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(acc->enable == IM_TRUE)
	{
		SetIspRegister(acc->regVal, ISP_ACC_HISA_ROAHI, roMat->roaHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISA_ROALO, roMat->roaLo);
		SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBHI, roMat->robHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBLO, roMat->robLo);
		SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCHI, roMat->rocHi);
		SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCLO, roMat->rocLo);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISA], acc->regVal[rISP_ACC_HISA]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISB], acc->regVal[rISP_ACC_HISB]));
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISC], acc->regVal[rISP_ACC_HISC]));
	}

	isppwl_memcpy((void*)(&acc->roMat), (void *)(roMat), sizeof(isp_acc_ro_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acc_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_enable(isp_acc_context_t *acc)
{
	IM_UINT32 lutb_offset;
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);

	if(acc->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("acc has been enabled")));
		return IM_RET_OK;	
	}
	//enable acc 
	
	/*set acc round mode*/
	SetIspRegister(acc->regVal, ISP_ACC_CNTL_ROUND, acc->rdMode);

	/*set acc lutb*/
	//set look-up table control mode
	SetIspRegister(acc->regVal, ISP_ACC_CNTL_APBS, acc->lutbType.lutbCtrMode);
	if(acc->lutbModeNeedChange == IM_TRUE)
	{
		//look-up table not ready
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 0);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));

		for(i=0; i<ISP_ACC_LUTB_LENGTH; i++)
		{
			lutb_offset = ISP_ACC_LUTB + i*4;
			IM_JIF(isppwl_write_reg(lutb_offset, accLutbCoef[acc->lutbType.lutbMode][i]));
		}
		//look-up table ready
		SetIspRegister(acc->regVal, ISP_ACC_CNTL_READY, 1);
		IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
		acc->lutbModeNeedChange = IM_FALSE;
	}

	/*set acc hist*/
	SetIspRegister(acc->regVal, ISP_ACC_CNTL_HISTROUND, acc->hist.histRdMode);
	SetIspRegister(acc->regVal, ISP_ACC_HISA_HISTFRAMES, acc->hist.histFrames);

	//set acc contrast coefe
	SetIspRegister(acc->regVal, ISP_ACC_CNTL_COE, acc->coefe);

	/*set acc coef roMat*/
	SetIspRegister(acc->regVal, ISP_ACC_ABCD_COA, acc->coMat.coefa);
	SetIspRegister(acc->regVal, ISP_ACC_ABCD_COB, acc->coMat.coefb);
	SetIspRegister(acc->regVal, ISP_ACC_ABCD_COC, acc->coMat.coefc);
	SetIspRegister(acc->regVal, ISP_ACC_ABCD_COD, acc->coMat.coefd);


	/*set acc ro roMat*/
	SetIspRegister(acc->regVal, ISP_ACC_HISA_ROAHI, acc->roMat.roaHi);
	SetIspRegister(acc->regVal, ISP_ACC_HISA_ROALO, acc->roMat.roaLo);
	SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBHI, acc->roMat.robHi);
	SetIspRegister(acc->regVal, ISP_ACC_HISB_ROBLO, acc->roMat.robLo);
	SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCHI, acc->roMat.rocHi);
	SetIspRegister(acc->regVal, ISP_ACC_HISC_ROCLO, acc->roMat.rocLo);

	/*set acc enable*/
	SetIspRegister(acc->regVal, ISP_GLB_CFG2_ACCBYPASS, 0);

	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_CNTL], acc->regVal[rISP_ACC_CNTL]));
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_ABCD], acc->regVal[rISP_ACC_ABCD]));
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISA], acc->regVal[rISP_ACC_HISA]));
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISB], acc->regVal[rISP_ACC_HISB]));
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_ACC_HISC], acc->regVal[rISP_ACC_HISC]));
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_GLB_CFG2], acc->regVal[rISP_GLB_CFG2]));

	acc->enable = IM_TRUE;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: acc_set_disable

  Functional description:

Inputs:

Outputs:

Returns: 

------------------------------------------------------------------------------*/
IM_RET acc_set_disable(isp_acc_context_t *acc)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(acc != IM_NULL);
	IM_ASSERT(acc->regVal != IM_NULL);
	IM_ASSERT(acc->regOfst != IM_NULL);

	if(acc->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("acc has been disabled")));
		return IM_RET_OK;	
	}
	//disable acc

	SetIspRegister(acc->regVal, ISP_GLB_CFG2_ACCBYPASS, 1);
	IM_JIF(isppwl_write_reg(acc->regOfst[rISP_GLB_CFG2], acc->regVal[rISP_GLB_CFG2]));

	acc->enable = IM_FALSE;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}
