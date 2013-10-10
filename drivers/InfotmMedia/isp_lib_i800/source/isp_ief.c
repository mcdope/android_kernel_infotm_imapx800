/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ief.c
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

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_ief.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IEF_I:"
#define WARNHEAD	"IEF_W:"
#define ERRHEAD		"IEF_E:"
#define TIPHEAD		"IEF_T:"

/*{coef11,coef12,coef13,coef21,coef22,coef23,
coef31,coef32,coef33,ofta,oftb}*/

static const IM_INT32 csc_yuv2rgb[11] = {
1192, 0, 1634, 1192, -402, -833, 1192, 2065, 0,
0, 32};	//default mode

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: ief_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_init(isp_ief_context_t *ief, isp_ief_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check ief type*/
	if((cfg->type < ISP_IEF_TYPE_NORMAL) || (cfg->type > ISP_IEF_TYPE_COLOR_SELECT))
	{
		IM_ERRMSG((IM_STR("Invalid type: type=%d"), cfg->type));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check ief select mode matrix*/
	if((cfg->selMat.tha < 0) || (cfg->selMat.tha > 255)
		|| (cfg->selMat.thb < 0)|| (cfg->selMat.thb > 255)
		|| ((cfg->selMat.mode != ISP_IEF_COLOR_SELECT_MODE_R)
		&& (cfg->selMat.mode != ISP_IEF_COLOR_SELECT_MODE_G)
		&& (cfg->selMat.mode != ISP_IEF_COLOR_SELECT_MODE_B)))
	{
		IM_ERRMSG((IM_STR("Invalid para: tha=%d, thb=%d, mode=%d"), 
					cfg->selMat.tha, cfg->selMat.thb, cfg->selMat.mode));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check ief rgcf matrix*/
	if((cfg->rgcfMat.coefCb < 0) || (cfg->rgcfMat.coefCb > 511)
		|| (cfg->rgcfMat.coefCr < 0)|| (cfg->rgcfMat.coefCr > 511))
	{
		IM_ERRMSG((IM_STR("Invalid coef: coefCb=%d, coefCr=%d"), 
					cfg->rgcfMat.coefCb, cfg->rgcfMat.coefCr));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->cscMat.cscMode != ISP_IEF_CSC_YUV2RGB)//default mode
	{
		ief->cscModeNeedChange = IM_TRUE;
		/*check ief csc cscMat*/
		if((cfg->cscMat.oft_a < -512) || (cfg->cscMat.oft_a > 511)
				|| (cfg->cscMat.oft_b < -512)|| (cfg->cscMat.oft_b > 511))
		{
			IM_ERRMSG((IM_STR("Invalid offset: offset_a=%d, offset_b=%d"), 
						cfg->cscMat.oft_a, cfg->cscMat.oft_a));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef11 < -4096) || (cfg->cscMat.coef11 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef11=%d"), cfg->cscMat.coef11));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef12 < -4096) || (cfg->cscMat.coef12 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef12=%d"), cfg->cscMat.coef12));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef13 < -4096) || (cfg->cscMat.coef13 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef13=%d"), cfg->cscMat.coef13));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef21 < -4096) || (cfg->cscMat.coef21 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef21=%d"), cfg->cscMat.coef21));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef22 < -4096) || (cfg->cscMat.coef22 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef22=%d"), cfg->cscMat.coef22));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef23 < -4096) || (cfg->cscMat.coef23 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef23=%d"), cfg->cscMat.coef23));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef31 < -4096) || (cfg->cscMat.coef31 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef31=%d"), cfg->cscMat.coef31));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef32 < -4096) || (cfg->cscMat.coef32 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef32=%d"), cfg->cscMat.coef32));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cfg->cscMat.coef33 < -4096) || (cfg->cscMat.coef33 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef33=%d"), cfg->cscMat.coef33));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	else
	{
		ief->cscModeNeedChange = IM_FALSE;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set ief type*/
		SetIspRegister(ief->regVal, ISP_IEF_CNTL_TYPE, cfg->type);

		/*set ief rgcf cscMat*/
		SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCB, cfg->rgcfMat.coefCb);
		SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCR, cfg->rgcfMat.coefCr);
		
		/*set ief select mode cscMat*/
		SetIspRegister(ief->regVal, ISP_IEF_SMD_THASEL, cfg->selMat.tha);
		SetIspRegister(ief->regVal, ISP_IEF_SMD_THBSEL, cfg->selMat.thb);
		SetIspRegister(ief->regVal, ISP_IEF_SMD_SELECTMODE, cfg->selMat.mode);
		
		/*set ief enable*/
		SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFBYPASS, 0);
		
		//isppwl_write_regs()
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_CNTL], ief->regVal[rISP_IEF_CNTL]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_RGCF1], ief->regVal[rISP_IEF_RGCF1]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_SMD], ief->regVal[rISP_IEF_SMD]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set ief bypass*/
		SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFBYPASS, 1);
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));
	}

	if(cfg->cscEn == IM_TRUE)
	{
		/*set ief csc matrix*/
		if(ief->cscModeNeedChange == IM_TRUE)
		{
			SetIspRegister(ief->regVal, ISP_IEF_COEF11, cfg->cscMat.coef11);
			SetIspRegister(ief->regVal, ISP_IEF_COEF12, cfg->cscMat.coef12);
			SetIspRegister(ief->regVal, ISP_IEF_COEF13, cfg->cscMat.coef13);
			SetIspRegister(ief->regVal, ISP_IEF_COEF21, cfg->cscMat.coef21);
			SetIspRegister(ief->regVal, ISP_IEF_COEF22, cfg->cscMat.coef22);
			SetIspRegister(ief->regVal, ISP_IEF_COEF23, cfg->cscMat.coef23);
			SetIspRegister(ief->regVal, ISP_IEF_COEF31, cfg->cscMat.coef31);
			SetIspRegister(ief->regVal, ISP_IEF_COEF32, cfg->cscMat.coef32);
			SetIspRegister(ief->regVal, ISP_IEF_COEF33, cfg->cscMat.coef33);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTA, cfg->cscMat.oft_a);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTB, cfg->cscMat.oft_b);
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF11], ief->regVal[rISP_IEF_COEF11]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF12], ief->regVal[rISP_IEF_COEF12]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF13], ief->regVal[rISP_IEF_COEF13]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF21], ief->regVal[rISP_IEF_COEF21]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF22], ief->regVal[rISP_IEF_COEF22]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF23], ief->regVal[rISP_IEF_COEF23]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF31], ief->regVal[rISP_IEF_COEF31]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF32], ief->regVal[rISP_IEF_COEF32]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF33], ief->regVal[rISP_IEF_COEF33]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_OFFSET], ief->regVal[rISP_IEF_OFFSET]));
			ief->cscModeNeedChange = IM_FALSE;
		}

		/*set ief csc enable*/
		SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFCSCBYPASS, 0);
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set ief csc bypass*/
		SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFCSCBYPASS, 1);
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));
	}

	ief->type = cfg->type;
	ief->rgcfMat.coefCb = cfg->rgcfMat.coefCb;
	ief->rgcfMat.coefCr = cfg->rgcfMat.coefCr;
	isppwl_memcpy((void *)(&(ief->cscMat)), (void *)(&(cfg->cscMat)), sizeof(isp_ief_csc_matrix));
	isppwl_memcpy((void *)(&(ief->selMat)), (void *)(&(cfg->selMat)), sizeof(isp_ief_select_matrix));
	ief->enable = cfg->enable;
	ief->cscEn = cfg->cscEn;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_type

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_type(isp_ief_context_t *ief, IM_UINT32 type)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);

	if((type < ISP_IEF_TYPE_NORMAL) || (type > ISP_IEF_TYPE_COLOR_SELECT))
	{
		IM_ERRMSG((IM_STR("Invalid threshold: threshold=%d"), type));
		return IM_RET_INVALID_PARAMETER;
	}

	if(ief->type == type)
	{
		IM_WARNMSG((IM_STR("ief has already been this type")));
		return IM_RET_OK;	
	}
	
	if(ief->enable == IM_TRUE)
	{
		SetIspRegister(ief->regVal, ISP_IEF_CNTL_TYPE, type);
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_CNTL], ief->regVal[rISP_IEF_CNTL]));
	}

	ief->type = type;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_rgcf_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_rgcf_matrix(isp_ief_context_t *ief, isp_ief_rgcf_matrix *rgcfMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);
	IM_ASSERT(rgcfMat != IM_NULL);
	
	/*check ief rgcf matrix*/
	if((rgcfMat->coefCb < 0) || (rgcfMat->coefCb > 511)
		|| (rgcfMat->coefCr < 0)|| (rgcfMat->coefCr > 511))
	{
		IM_ERRMSG((IM_STR("Invalid coef: coefCb=%d, coefCr=%d"), 
					rgcfMat->coefCb, rgcfMat->coefCr));
		return IM_RET_INVALID_PARAMETER;
	}
#if 0
	if(ief->type != ISP_IEF_TYPE_SEPIA)
	{
		IM_ERRMSG((IM_STR("ief type is not SEPIA, should set ief type to SEPIA before")));
		return IM_RET_FAILED;
	}
#endif
	if(ief->enable == IM_TRUE)
	{
		SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCB, rgcfMat->coefCb);
		SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCR, rgcfMat->coefCr);

		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_RGCF1], ief->regVal[rISP_IEF_RGCF1]));
	}

	ief->rgcfMat.coefCb = rgcfMat->coefCb;
	ief->rgcfMat.coefCr = rgcfMat->coefCr;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_color_select_mode_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_color_select_mode_matrix(isp_ief_context_t *ief, isp_ief_select_matrix *selMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);
	IM_ASSERT(selMat != IM_NULL);
	
	/*check ief select mode matrix*/
	if((selMat->tha < 0) || (selMat->tha > 255)
		|| (selMat->thb < 0)|| (selMat->thb > 255)
		|| ((selMat->mode != ISP_IEF_COLOR_SELECT_MODE_R)
		&& (selMat->mode != ISP_IEF_COLOR_SELECT_MODE_G)
		&& (selMat->mode != ISP_IEF_COLOR_SELECT_MODE_B)))
	{
		IM_ERRMSG((IM_STR("Invalid para: tha=%d, thb=%d, mode=%d"), 
					selMat->tha, selMat->thb, selMat->mode));
		return IM_RET_INVALID_PARAMETER;
	}
#if 0
	if(ief->type != ISP_IEF_TYPE_COLOR_SELECT)
	{
		IM_ERRMSG((IM_STR("ief type is not color select, should set ief type to color select before")));
		return IM_RET_FAILED;
	}
#endif

	if(ief->enable == IM_TRUE)
	{
		SetIspRegister(ief->regVal, ISP_IEF_SMD_THASEL, selMat->tha);
		SetIspRegister(ief->regVal, ISP_IEF_SMD_THBSEL, selMat->thb);
		SetIspRegister(ief->regVal, ISP_IEF_SMD_SELECTMODE, selMat->mode);
		
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_SMD], ief->regVal[rISP_IEF_SMD]));
	}

	isppwl_memcpy((void *)(&(ief->selMat)), (void *)(selMat), sizeof(isp_ief_select_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_csc_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_csc_matrix(isp_ief_context_t *ief, isp_ief_csc_matrix *cscMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);
	IM_ASSERT(cscMat != IM_NULL);

	if((ief->cscModeNeedChange == IM_TRUE) || (ief->cscMat.cscMode != cscMat->cscMode) || (cscMat->cscMode == ISP_IEF_CSC_CUSTOM))
	{
		ief->cscModeNeedChange = IM_TRUE;
	}
	else
	{
		ief->cscModeNeedChange = IM_FALSE;
	}

	/*check ief csc cscMat*/
	if(cscMat->cscMode == ISP_IEF_CSC_CUSTOM)
	{
		if((cscMat->oft_a < -512) || (cscMat->oft_a > 511)
				|| (cscMat->oft_b < -512)|| (cscMat->oft_b > 511))
		{
			IM_ERRMSG((IM_STR("Invalid offset: offset_a=%d, offset_b=%d"), 
						cscMat->oft_a, cscMat->oft_a));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef11 < -4096) || (cscMat->coef11 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef11=%d"), cscMat->coef11));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef12 < -4096) || (cscMat->coef12 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef12=%d"), cscMat->coef12));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef13 < -4096) || (cscMat->coef13 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef13=%d"), cscMat->coef13));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef21 < -4096) || (cscMat->coef21 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef21=%d"), cscMat->coef21));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef22 < -4096) || (cscMat->coef22 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef22=%d"), cscMat->coef22));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef23 < -4096) || (cscMat->coef23 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef23=%d"), cscMat->coef23));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef31 < -4096) || (cscMat->coef31 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef31=%d"), cscMat->coef31));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef32 < -4096) || (cscMat->coef32 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef32=%d"), cscMat->coef32));
			return IM_RET_INVALID_PARAMETER;
		}
		if((cscMat->coef33 < -4096) || (cscMat->coef33 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid coef: coef33=%d"), cscMat->coef33));
			return IM_RET_INVALID_PARAMETER;
		}
	}

	if(ief->cscEn == IM_TRUE)
	{
		if(ief->cscModeNeedChange == IM_TRUE)
		{
			if(cscMat->cscMode == ISP_IEF_CSC_CUSTOM)
			{
				SetIspRegister(ief->regVal, ISP_IEF_COEF11, cscMat->coef11);
				SetIspRegister(ief->regVal, ISP_IEF_COEF12, cscMat->coef12);
				SetIspRegister(ief->regVal, ISP_IEF_COEF13, cscMat->coef13);
				SetIspRegister(ief->regVal, ISP_IEF_COEF21, cscMat->coef21);
				SetIspRegister(ief->regVal, ISP_IEF_COEF22, cscMat->coef22);
				SetIspRegister(ief->regVal, ISP_IEF_COEF23, cscMat->coef23);
				SetIspRegister(ief->regVal, ISP_IEF_COEF31, cscMat->coef31);
				SetIspRegister(ief->regVal, ISP_IEF_COEF32, cscMat->coef32);
				SetIspRegister(ief->regVal, ISP_IEF_COEF33, cscMat->coef33);
				SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTA, cscMat->oft_a);
				SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTB, cscMat->oft_b);
			}
			else
			{
				SetIspRegister(ief->regVal, ISP_IEF_COEF11, csc_yuv2rgb[0]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF12, csc_yuv2rgb[1]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF13, csc_yuv2rgb[2]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF21, csc_yuv2rgb[3]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF22, csc_yuv2rgb[4]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF23, csc_yuv2rgb[5]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF31, csc_yuv2rgb[6]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF32, csc_yuv2rgb[7]);
				SetIspRegister(ief->regVal, ISP_IEF_COEF33, csc_yuv2rgb[8]);
				SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTA, csc_yuv2rgb[9]);
				SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTB, csc_yuv2rgb[10]);
			}
			//isppwl_write_regs()
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF11], ief->regVal[rISP_IEF_COEF11]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF12], ief->regVal[rISP_IEF_COEF12]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF13], ief->regVal[rISP_IEF_COEF13]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF21], ief->regVal[rISP_IEF_COEF21]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF22], ief->regVal[rISP_IEF_COEF22]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF23], ief->regVal[rISP_IEF_COEF23]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF31], ief->regVal[rISP_IEF_COEF31]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF32], ief->regVal[rISP_IEF_COEF32]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF33], ief->regVal[rISP_IEF_COEF33]));
			IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_OFFSET], ief->regVal[rISP_IEF_OFFSET]));
			ief->cscModeNeedChange = IM_FALSE;
		}
	}

	isppwl_memcpy((void *)(&(ief->cscMat)), (void *)(cscMat), sizeof(isp_ief_csc_matrix));
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: ief_set_csc_enable

  Functional description:

Inputs:

Outputs:

Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_csc_enable(isp_ief_context_t *ief)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);

	if(ief->cscEn == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ief csc has been enabled")));
		return IM_RET_OK;	
	}
	if(ief->cscModeNeedChange == IM_TRUE)
	{
		if(ief->cscMat.cscMode == ISP_IEF_CSC_CUSTOM)
		{
			SetIspRegister(ief->regVal, ISP_IEF_COEF11, ief->cscMat.coef11);
			SetIspRegister(ief->regVal, ISP_IEF_COEF12, ief->cscMat.coef12);
			SetIspRegister(ief->regVal, ISP_IEF_COEF13, ief->cscMat.coef13);
			SetIspRegister(ief->regVal, ISP_IEF_COEF21, ief->cscMat.coef21);
			SetIspRegister(ief->regVal, ISP_IEF_COEF22, ief->cscMat.coef22);
			SetIspRegister(ief->regVal, ISP_IEF_COEF23, ief->cscMat.coef23);
			SetIspRegister(ief->regVal, ISP_IEF_COEF31, ief->cscMat.coef31);
			SetIspRegister(ief->regVal, ISP_IEF_COEF32, ief->cscMat.coef32);
			SetIspRegister(ief->regVal, ISP_IEF_COEF33, ief->cscMat.coef33);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTA, ief->cscMat.oft_a);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTB, ief->cscMat.oft_b);
		}
		else
		{
			SetIspRegister(ief->regVal, ISP_IEF_COEF11, csc_yuv2rgb[0]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF12, csc_yuv2rgb[1]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF13, csc_yuv2rgb[2]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF21, csc_yuv2rgb[3]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF22, csc_yuv2rgb[4]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF23, csc_yuv2rgb[5]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF31, csc_yuv2rgb[6]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF32, csc_yuv2rgb[7]);
			SetIspRegister(ief->regVal, ISP_IEF_COEF33, csc_yuv2rgb[8]);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTA, csc_yuv2rgb[9]);
			SetIspRegister(ief->regVal, ISP_IEF_OFFSET_OFTB, csc_yuv2rgb[10]);
		}
		//isppwl_write_regs()
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF11], ief->regVal[rISP_IEF_COEF11]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF12], ief->regVal[rISP_IEF_COEF12]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF13], ief->regVal[rISP_IEF_COEF13]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF21], ief->regVal[rISP_IEF_COEF21]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF22], ief->regVal[rISP_IEF_COEF22]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF23], ief->regVal[rISP_IEF_COEF23]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF31], ief->regVal[rISP_IEF_COEF31]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF32], ief->regVal[rISP_IEF_COEF32]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_COEF33], ief->regVal[rISP_IEF_COEF33]));
		IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_OFFSET], ief->regVal[rISP_IEF_OFFSET]));
		ief->cscModeNeedChange = IM_FALSE;
	}
	//enable ief csc
	/*set ief csc enable*/
	SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFCSCBYPASS, 0);
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));

	ief->cscEn = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_csc_disable(isp_ief_context_t *ief)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);

	if(ief->cscEn != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ief csc has been disabled")));
		return IM_RET_OK;	
	}
	//disable ief csc
	SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFCSCBYPASS, 1);
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));

	ief->cscEn = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}



/*------------------------------------------------------------------------------

    Function name: ief_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_enable(isp_ief_context_t *ief)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);

	if(ief->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ief has been enabled")));
		return IM_RET_OK;	
	}
	//enable ief 
	
	/*set ief type*/
	SetIspRegister(ief->regVal, ISP_IEF_CNTL_TYPE, ief->type);

	/*set ief rgcf cscMat*/
	SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCB, ief->rgcfMat.coefCb);
	SetIspRegister(ief->regVal, ISP_IEF_RGCF1_COCR, ief->rgcfMat.coefCr);
	
	/*set ief select mode cscMat*/
	SetIspRegister(ief->regVal, ISP_IEF_SMD_THASEL, ief->selMat.tha);
	SetIspRegister(ief->regVal, ISP_IEF_SMD_THBSEL, ief->selMat.thb);
	SetIspRegister(ief->regVal, ISP_IEF_SMD_SELECTMODE, ief->selMat.mode);
	
	/*set ief enable*/
	SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFBYPASS, 0);

	//isppwl_write_regs()
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_CNTL], ief->regVal[rISP_IEF_CNTL]));
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_RGCF1], ief->regVal[rISP_IEF_RGCF1]));
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_IEF_SMD], ief->regVal[rISP_IEF_SMD]));
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));

	ief->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ief_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ief_set_disable(isp_ief_context_t *ief)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ief != IM_NULL);
	IM_ASSERT(ief->regVal != IM_NULL);
	IM_ASSERT(ief->regOfst != IM_NULL);

	if(ief->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ief has been disabled")));
		return IM_RET_OK;	
	}
	//disable ief
	SetIspRegister(ief->regVal, ISP_GLB_CFG2_IEFBYPASS, 1);
	IM_JIF(isppwl_write_reg(ief->regOfst[rISP_GLB_CFG2], ief->regVal[rISP_GLB_CFG2]));

	ief->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}


