/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_acm.c
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

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_acm.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"ACM_I:"
#define WARNHEAD	"ACM_W:"
#define ERRHEAD		"ACM_E:"
#define TIPHEAD		"ACM_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: acm_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_init(isp_acm_context_t *acm, isp_acm_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check acm round mode*/
	if(cfg->rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acm round mode error: rdMode = %d"), cfg->rdMode));
		return IM_RET_OK;
	}

	/*check acm saturation threshold*/ 
	if((cfg->ths < -32768) || (cfg->ths > 32767))
	{
		IM_ERRMSG((IM_STR("Invalid saturation threshold :ths=%d"), cfg->ths));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check acm threshold coMat*/
	if((cfg->thrMat.tha < 0) || (cfg->thrMat.tha > 255)
		|| (cfg->thrMat.thb < 0) || (cfg->thrMat.thb > 255)
		|| (cfg->thrMat.thc < 0) || (cfg->thrMat.thc > 255))
	{
		IM_ERRMSG((IM_STR("Invalid coMat value: tha=%d, thb=%d, thc=%d"), 
			cfg->thrMat.tha, cfg->thrMat.thb, cfg->thrMat.thc));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check acm coef coMat*/
	if((cfg->coMat.coefr < 0) || (cfg->coMat.coefr > 65535)
		|| (cfg->coMat.coefg < 0) || (cfg->coMat.coefg > 65535)
		|| (cfg->coMat.coefb < 0) || (cfg->coMat.coefb > 65535)
		|| (cfg->coMat.coefp < 0) || (cfg->coMat.coefp > 65535)
		|| (cfg->coMat.m0r < 0) || (cfg->coMat.m0r > 4095)
		|| (cfg->coMat.m1r < 0) || (cfg->coMat.m1r > 4095)
		|| (cfg->coMat.m2r < 0) || (cfg->coMat.m2r > 4095)
		|| (cfg->coMat.m3r < 0) || (cfg->coMat.m3r > 4095)
		|| (cfg->coMat.m4r < 0) || (cfg->coMat.m4r > 4095)
		|| (cfg->coMat.m0g < 0) || (cfg->coMat.m0g > 4095)
		|| (cfg->coMat.m1g < 0) || (cfg->coMat.m1g > 4095)
		|| (cfg->coMat.m2g < 0) || (cfg->coMat.m2g > 4095)
		|| (cfg->coMat.m3g < 0) || (cfg->coMat.m3g > 4095)
		|| (cfg->coMat.m4g < 0) || (cfg->coMat.m4g > 4095)
		|| (cfg->coMat.m0b < 0) || (cfg->coMat.m0b > 4095)
		|| (cfg->coMat.m1b < 0) || (cfg->coMat.m1b > 4095)
		|| (cfg->coMat.m2b < 0) || (cfg->coMat.m2b > 4095)
		|| (cfg->coMat.m3b < 0) || (cfg->coMat.m3b > 4095)
		|| (cfg->coMat.m4b < 0) || (cfg->coMat.m4b > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid coMat value: cor=%d, cog=%d, cob=%d, cop=%d"), 
			cfg->coMat.coefr, cfg->coMat.coefg, cfg->coMat.coefb, cfg->coMat.coefp));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0r=%d, m1r=%d, m2r=%d, m3r=%d, m4r=%d"), 
			cfg->coMat.m0r, cfg->coMat.m1r, cfg->coMat.m2r, cfg->coMat.m3r, cfg->coMat.m4r));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0g=%d, m1g=%d, m2g=%d, m3g=%d, m4g=%d"), 
			cfg->coMat.m0g, cfg->coMat.m1g, cfg->coMat.m2g, cfg->coMat.m3g, cfg->coMat.m4g));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0b=%d, m1b=%d, m2b=%d, m3b=%d, m4b=%d"), 
			cfg->coMat.m0b, cfg->coMat.m1b, cfg->coMat.m2b, cfg->coMat.m3b, cfg->coMat.m4b));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set acm round mode*/
		SetIspRegister(acm->regVal, ISP_ACM_CNTLA_ROUND, cfg->rdMode);

		//set acm saturation threshold
		SetIspRegister(acm->regVal, ISP_ACM_CNTLA_THS, cfg->ths);
		
		//set acm threshold matrix
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THA, cfg->thrMat.tha);
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THB, cfg->thrMat.thb);
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THC, cfg->thrMat.thc);

		//set acm coef matrix
		SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COP, cfg->coMat.coefp);
		SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COR, cfg->coMat.coefr);
		SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COG, cfg->coMat.coefg);
		SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COB, cfg->coMat.coefb);
		SetIspRegister(acm->regVal, ISP_ACM_COEF0_M0R, cfg->coMat.m0r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF1_M1R, cfg->coMat.m1r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF1_M2R, cfg->coMat.m2r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF2_M3R, cfg->coMat.m3r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF2_M4R, cfg->coMat.m4r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF3_M0G, cfg->coMat.m0g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF3_M1G, cfg->coMat.m1g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF4_M2G, cfg->coMat.m2g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF4_M3G, cfg->coMat.m3g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF5_M4G, cfg->coMat.m4g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF5_M0B, cfg->coMat.m0b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF6_M1B, cfg->coMat.m1b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF6_M2B, cfg->coMat.m2b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF7_M3B, cfg->coMat.m3b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF7_M4B, cfg->coMat.m4b);

		/*set acm enable*/
		SetIspRegister(acm->regVal, ISP_GLB_CFG2_ACMBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLA], acm->regVal[rISP_ACM_CNTLA]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLB], acm->regVal[rISP_ACM_CNTLB]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFPR], acm->regVal[rISP_ACM_COEFPR]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFGB], acm->regVal[rISP_ACM_COEFGB]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF0], acm->regVal[rISP_ACM_COEF0]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF1], acm->regVal[rISP_ACM_COEF1]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF2], acm->regVal[rISP_ACM_COEF2]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF3], acm->regVal[rISP_ACM_COEF3]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF4], acm->regVal[rISP_ACM_COEF4]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF5], acm->regVal[rISP_ACM_COEF5]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF6], acm->regVal[rISP_ACM_COEF6]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF7], acm->regVal[rISP_ACM_COEF7]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_GLB_CFG2], acm->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set acm bypass*/
		SetIspRegister(acm->regVal, ISP_GLB_CFG2_ACMBYPASS, 1);
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_GLB_CFG2], acm->regVal[rISP_GLB_CFG2]));
	}

	acm->rdMode = cfg->rdMode;
	acm->ths = cfg->ths;
	isppwl_memcpy((void*)(&acm->thrMat), (void *)(&cfg->thrMat), sizeof(isp_acm_thr_matrix));
	isppwl_memcpy((void*)(&acm->coMat), (void *)(&cfg->coMat), sizeof(isp_acm_coef_matrix));
	acm->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_round_mode(isp_acm_context_t *acm, IM_UINT32 rdMode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);
	
	/*check acm round mode*/
	if(rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("acm round mode error: rdMode = %d"), rdMode));
		return IM_RET_OK;
	}
	if(acm->rdMode == rdMode)
	{
		IM_WARNMSG((IM_STR("acm round mode has already been this mode")));
		return IM_RET_OK;	
	}	

	if(acm->enable == IM_TRUE)
	{
		SetIspRegister(acm->regVal, ISP_ACM_CNTLA_ROUND, rdMode);
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLA], acm->regVal[rISP_ACM_CNTLA]));
	}

	acm->rdMode = rdMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_saturation_threshold

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_saturation_threshold(isp_acm_context_t *acm, IM_INT32 ths)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);

	//check acm saturation threshold 
	if((ths < -32768) || (ths > 32767))
	{
		IM_ERRMSG((IM_STR("Invalid saturation threshold :ths=%d"), ths));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acm->enable == IM_TRUE)
	{
		SetIspRegister(acm->regVal, ISP_ACM_CNTLA_THS, ths);
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLA], acm->regVal[rISP_ACM_CNTLA]));
	}

	acm->ths = ths;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_threshold_matrix(isp_acm_context_t *acm, isp_acm_thr_matrix *thrMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);
	IM_ASSERT(thrMat != IM_NULL);

	//check acm threshold matrix
	if((thrMat->tha < 0) || (thrMat->tha > 255)
		|| (thrMat->thb < 0) || (thrMat->thb > 255)
		|| (thrMat->thc < 0) || (thrMat->thc > 255))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: tha=%d, thb=%d, thc=%d"), 
			thrMat->tha, thrMat->thb, thrMat->thc));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acm->enable == IM_TRUE)
	{
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THA, thrMat->tha);
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THB, thrMat->thb);
		SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THC, thrMat->thc);
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLB], acm->regVal[rISP_ACM_CNTLB]));
	}

	isppwl_memcpy((void*)(&acm->thrMat), (void *)(thrMat), sizeof(isp_acm_thr_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_coef_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_coef_matrix(isp_acm_context_t *acm, isp_acm_coef_matrix *coMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);
	IM_ASSERT(coMat != IM_NULL);

	/*check acm coef matrix*/
	if((coMat->coefr < 0) || (coMat->coefr > 65535)
		|| (coMat->coefg < 0) || (coMat->coefg > 65535)
		|| (coMat->coefb < 0) || (coMat->coefb > 65535)
		|| (coMat->coefp < 0) || (coMat->coefp > 65535)
		|| (coMat->m0r < 0) || (coMat->m0r > 4095)
		|| (coMat->m1r < 0) || (coMat->m1r > 4095)
		|| (coMat->m2r < 0) || (coMat->m2r > 4095)
		|| (coMat->m3r < 0) || (coMat->m3r > 4095)
		|| (coMat->m4r < 0) || (coMat->m4r > 4095)
		|| (coMat->m0g < 0) || (coMat->m0g > 4095)
		|| (coMat->m1g < 0) || (coMat->m1g > 4095)
		|| (coMat->m2g < 0) || (coMat->m2g > 4095)
		|| (coMat->m3g < 0) || (coMat->m3g > 4095)
		|| (coMat->m4g < 0) || (coMat->m4g > 4095)
		|| (coMat->m0b < 0) || (coMat->m0b > 4095)
		|| (coMat->m1b < 0) || (coMat->m1b > 4095)
		|| (coMat->m2b < 0) || (coMat->m2b > 4095)
		|| (coMat->m3b < 0) || (coMat->m3b > 4095)
		|| (coMat->m4b < 0) || (coMat->m4b > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid coMat value: cor=%d, cog=%d, cob=%d, cop=%d"), 
			coMat->coefr, coMat->coefg, coMat->coefb, coMat->coefp));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0r=%d, m1r=%d, m2r=%d, m3r=%d, m4r=%d"), 
			coMat->m0r, coMat->m1r, coMat->m2r, coMat->m3r, coMat->m4r));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0g=%d, m1g=%d, m2g=%d, m3g=%d, m4g=%d"), 
			coMat->m0g, coMat->m1g, coMat->m2g, coMat->m3g, coMat->m4g));
		IM_ERRMSG((IM_STR("Invalid coMat value: m0b=%d, m1b=%d, m2b=%d, m3b=%d, m4b=%d"), 
			coMat->m0b, coMat->m1b, coMat->m2b, coMat->m3b, coMat->m4b));
		return IM_RET_INVALID_PARAMETER;
	}

	if(acm->enable == IM_TRUE)
	{
		SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COP, coMat->coefp);
		SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COR, coMat->coefr);
		SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COG, coMat->coefg);
		SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COB, coMat->coefb);
		SetIspRegister(acm->regVal, ISP_ACM_COEF0_M0R, coMat->m0r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF1_M1R, coMat->m1r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF1_M2R, coMat->m2r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF2_M3R, coMat->m3r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF2_M4R, coMat->m4r);
		SetIspRegister(acm->regVal, ISP_ACM_COEF3_M0G, coMat->m0g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF3_M1G, coMat->m1g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF4_M2G, coMat->m2g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF4_M3G, coMat->m3g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF5_M4G, coMat->m4g);
		SetIspRegister(acm->regVal, ISP_ACM_COEF5_M0B, coMat->m0b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF6_M1B, coMat->m1b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF6_M2B, coMat->m2b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF7_M3B, coMat->m3b);
		SetIspRegister(acm->regVal, ISP_ACM_COEF7_M4B, coMat->m4b);
		
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFPR], acm->regVal[rISP_ACM_COEFPR]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFGB], acm->regVal[rISP_ACM_COEFGB]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF0], acm->regVal[rISP_ACM_COEF0]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF1], acm->regVal[rISP_ACM_COEF1]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF2], acm->regVal[rISP_ACM_COEF2]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF3], acm->regVal[rISP_ACM_COEF3]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF4], acm->regVal[rISP_ACM_COEF4]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF5], acm->regVal[rISP_ACM_COEF5]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF6], acm->regVal[rISP_ACM_COEF6]));
		IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF7], acm->regVal[rISP_ACM_COEF7]));
	}

	isppwl_memcpy((void*)(&acm->coMat), (void *)(coMat), sizeof(isp_acm_coef_matrix));	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_enable(isp_acm_context_t *acm)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);

	if(acm->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("acm has been enabled")));
		return IM_RET_OK;	
	}

	//enable acm 
	
	/*set acm round mode*/
	SetIspRegister(acm->regVal, ISP_ACM_CNTLA_ROUND, acm->rdMode);

	//set acm saturation threshold
	SetIspRegister(acm->regVal, ISP_ACM_CNTLA_THS, acm->ths);
	
	//set acm threshold matrix
	SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THA, acm->thrMat.tha);
	SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THB, acm->thrMat.thb);
	SetIspRegister(acm->regVal, ISP_ACM_CNTLB_THC, acm->thrMat.thc);


	//set acm coef matrix
	SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COP, acm->coMat.coefp);
	SetIspRegister(acm->regVal, ISP_ACM_COEFPR_COR, acm->coMat.coefr);
	SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COG, acm->coMat.coefg);
	SetIspRegister(acm->regVal, ISP_ACM_COEFGB_COB, acm->coMat.coefb);
	SetIspRegister(acm->regVal, ISP_ACM_COEF0_M0R, acm->coMat.m0r);
	SetIspRegister(acm->regVal, ISP_ACM_COEF1_M1R, acm->coMat.m1r);
	SetIspRegister(acm->regVal, ISP_ACM_COEF1_M2R, acm->coMat.m2r);
	SetIspRegister(acm->regVal, ISP_ACM_COEF2_M3R, acm->coMat.m3r);
	SetIspRegister(acm->regVal, ISP_ACM_COEF2_M4R, acm->coMat.m4r);
	SetIspRegister(acm->regVal, ISP_ACM_COEF3_M0G, acm->coMat.m0g);
	SetIspRegister(acm->regVal, ISP_ACM_COEF3_M1G, acm->coMat.m1g);
	SetIspRegister(acm->regVal, ISP_ACM_COEF4_M2G, acm->coMat.m2g);
	SetIspRegister(acm->regVal, ISP_ACM_COEF4_M3G, acm->coMat.m3g);
	SetIspRegister(acm->regVal, ISP_ACM_COEF5_M4G, acm->coMat.m4g);
	SetIspRegister(acm->regVal, ISP_ACM_COEF5_M0B, acm->coMat.m0b);
	SetIspRegister(acm->regVal, ISP_ACM_COEF6_M1B, acm->coMat.m1b);
	SetIspRegister(acm->regVal, ISP_ACM_COEF6_M2B, acm->coMat.m2b);
	SetIspRegister(acm->regVal, ISP_ACM_COEF7_M3B, acm->coMat.m3b);
	SetIspRegister(acm->regVal, ISP_ACM_COEF7_M4B, acm->coMat.m4b);

	/*set acm enable*/
	SetIspRegister(acm->regVal, ISP_GLB_CFG2_ACMBYPASS, 0);


	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLA], acm->regVal[rISP_ACM_CNTLA]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_CNTLB], acm->regVal[rISP_ACM_CNTLB]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFPR], acm->regVal[rISP_ACM_COEFPR]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEFGB], acm->regVal[rISP_ACM_COEFGB]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF0], acm->regVal[rISP_ACM_COEF0]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF1], acm->regVal[rISP_ACM_COEF1]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF2], acm->regVal[rISP_ACM_COEF2]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF3], acm->regVal[rISP_ACM_COEF3]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF4], acm->regVal[rISP_ACM_COEF4]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF5], acm->regVal[rISP_ACM_COEF5]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF6], acm->regVal[rISP_ACM_COEF6]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_ACM_COEF7], acm->regVal[rISP_ACM_COEF7]));
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_GLB_CFG2], acm->regVal[rISP_GLB_CFG2]));

	acm->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: acm_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET acm_set_disable(isp_acm_context_t *acm)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(acm != IM_NULL);
	IM_ASSERT(acm->regVal != IM_NULL);
	IM_ASSERT(acm->regOfst != IM_NULL);

	if(acm->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("acm has been disabled")));
		return IM_RET_OK;	
	}
	//disable acm
	
	SetIspRegister(acm->regVal, ISP_GLB_CFG2_ACMBYPASS, 1);
	IM_JIF(isppwl_write_reg(acm->regOfst[rISP_GLB_CFG2], acm->regVal[rISP_GLB_CFG2]));

	acm->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
