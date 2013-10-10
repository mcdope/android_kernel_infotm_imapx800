/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_cmncsc.c
--
--  Description :
--		common colorspace conversion module.
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
#include "isp_cmncsc.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"BDC_I:"
#define WARNHEAD	"BDC_W:"
#define ERRHEAD		"BDC_E:"
#define TIPHEAD		"BDC_T:"

/*{coef11,coef12,coef13,coef21,coef22,coef23,
coef31,coef32,coef33,ofta,oftb}*/
static const IM_INT32 csc[][11] = {
//rgb2yuv, default mode
{263, 516, 100, -152, -298, 450, 450, -377, -73, 64, 0},
//yuv2rgb
{1192, 0, 1634, 1192, -402, -833, 1192, 2065, 0, 0, 64},
};


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: cmncsc_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET cmncsc_init(isp_cmncsc_context_t *cmncsc, isp_cmncsc_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(cmncsc != IM_NULL);
	IM_ASSERT(cmncsc->regVal != IM_NULL);
	IM_ASSERT(cmncsc->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	/*check csc mode*/
	if(cfg->mode > ISP_CMNCSC_YUV2RGB)
	{
		IM_ERRMSG((IM_STR("Invalid cfg->mode=%d"), cfg->mode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->mode != ISP_CMNCSC_RGB2YUV)//default mode
	{
		cmncsc->modeNeedChange = IM_TRUE;
	}
	else
	{
		cmncsc->modeNeedChange = IM_FALSE;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set cmncsc matrix*/
		if(cmncsc->modeNeedChange == IM_TRUE)
		{
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF11, csc[cfg->mode][0]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF12, csc[cfg->mode][1]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF13, csc[cfg->mode][2]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF21, csc[cfg->mode][3]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF22, csc[cfg->mode][4]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF23, csc[cfg->mode][5]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF31, csc[cfg->mode][6]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF32, csc[cfg->mode][7]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_COEF33, csc[cfg->mode][8]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFA, csc[cfg->mode][9]);
			SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFB, csc[cfg->mode][10]);
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF11], cmncsc->regVal[rISP_CMN_COEF11]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF12], cmncsc->regVal[rISP_CMN_COEF12]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF13], cmncsc->regVal[rISP_CMN_COEF13]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF21], cmncsc->regVal[rISP_CMN_COEF21]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF22], cmncsc->regVal[rISP_CMN_COEF22]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF23], cmncsc->regVal[rISP_CMN_COEF23]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF31], cmncsc->regVal[rISP_CMN_COEF31]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF32], cmncsc->regVal[rISP_CMN_COEF32]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF33], cmncsc->regVal[rISP_CMN_COEF33]));
			IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_OFFSET], cmncsc->regVal[rISP_CMN_OFFSET]));
			cmncsc->modeNeedChange = IM_FALSE;
		}
		/*set cmncsc enable*/
		SetIspRegister(cmncsc->regVal, ISP_GLB_CFG2_CMNCSCBYPASS, 0);
	}
	else
	{
		/*set cmncsc  bypass*/
		SetIspRegister(cmncsc->regVal, ISP_GLB_CFG2_CMNCSCBYPASS, 1);
	}
	IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_GLB_CFG2], cmncsc->regVal[rISP_GLB_CFG2]));

	cmncsc->mode = cfg->mode;
	cmncsc->enable = cfg->enable;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: cmncsc_set_mode

  Functional description:

Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET cmncsc_set_mode(isp_cmncsc_context_t *cmncsc, IM_UINT32 mode)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(cmncsc != IM_NULL);
	IM_ASSERT(cmncsc->regVal != IM_NULL);
	IM_ASSERT(cmncsc->regOfst != IM_NULL);

	/*check csc mode*/
	if(mode > ISP_CMNCSC_YUV2RGB)
	{
		IM_ERRMSG((IM_STR("Invalid mode=%d"), mode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((cmncsc->modeNeedChange == IM_TRUE) || (cmncsc->mode != mode))
	{
		cmncsc->modeNeedChange = IM_FALSE;
	}
	else
	{
		cmncsc->modeNeedChange = IM_FALSE;
	}

	if((cmncsc->enable == IM_TRUE) && (cmncsc->modeNeedChange == IM_TRUE))
	{
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF11, csc[mode][0]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF12, csc[mode][1]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF13, csc[mode][2]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF21, csc[mode][3]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF22, csc[mode][4]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF23, csc[mode][5]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF31, csc[mode][6]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF32, csc[mode][7]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF33, csc[mode][8]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFA, csc[mode][9]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFB, csc[mode][10]);
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF11], cmncsc->regVal[rISP_CMN_COEF11]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF12], cmncsc->regVal[rISP_CMN_COEF12]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF13], cmncsc->regVal[rISP_CMN_COEF13]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF21], cmncsc->regVal[rISP_CMN_COEF21]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF22], cmncsc->regVal[rISP_CMN_COEF22]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF23], cmncsc->regVal[rISP_CMN_COEF23]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF31], cmncsc->regVal[rISP_CMN_COEF31]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF32], cmncsc->regVal[rISP_CMN_COEF32]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF33], cmncsc->regVal[rISP_CMN_COEF33]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_OFFSET], cmncsc->regVal[rISP_CMN_OFFSET]));
		cmncsc->modeNeedChange = IM_FALSE;
	}
	cmncsc->mode = mode;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}
/*------------------------------------------------------------------------------

    Function name: cmncsc_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET cmncsc_set_enable(isp_cmncsc_context_t *cmncsc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(cmncsc != IM_NULL);
	IM_ASSERT(cmncsc->regVal != IM_NULL);
	IM_ASSERT(cmncsc->regOfst != IM_NULL);

	if(cmncsc->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("cmncsc has been enabled")));
		return IM_RET_OK;	
	}

	if(cmncsc->modeNeedChange == IM_TRUE)
	{
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF11, csc[cmncsc->mode][0]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF12, csc[cmncsc->mode][1]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF13, csc[cmncsc->mode][2]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF21, csc[cmncsc->mode][3]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF22, csc[cmncsc->mode][4]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF23, csc[cmncsc->mode][5]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF31, csc[cmncsc->mode][6]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF32, csc[cmncsc->mode][7]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_COEF33, csc[cmncsc->mode][8]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFA, csc[cmncsc->mode][9]);
		SetIspRegister(cmncsc->regVal, ISP_CMN_OFFSET_OFFB, csc[cmncsc->mode][10]);
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF11], cmncsc->regVal[rISP_CMN_COEF11]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF12], cmncsc->regVal[rISP_CMN_COEF12]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF13], cmncsc->regVal[rISP_CMN_COEF13]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF21], cmncsc->regVal[rISP_CMN_COEF21]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF22], cmncsc->regVal[rISP_CMN_COEF22]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF23], cmncsc->regVal[rISP_CMN_COEF23]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF31], cmncsc->regVal[rISP_CMN_COEF31]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF32], cmncsc->regVal[rISP_CMN_COEF32]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_COEF33], cmncsc->regVal[rISP_CMN_COEF33]));
		IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_CMN_OFFSET], cmncsc->regVal[rISP_CMN_OFFSET]));
		cmncsc->modeNeedChange = IM_FALSE;
	}

	/*set cmncsc enable*/
	SetIspRegister(cmncsc->regVal, ISP_GLB_CFG2_CMNCSCBYPASS, 0);
	IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_GLB_CFG2], cmncsc->regVal[rISP_GLB_CFG2]));

	cmncsc->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: cmncsc_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET cmncsc_set_disable(isp_cmncsc_context_t *cmncsc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(cmncsc != IM_NULL);
	IM_ASSERT(cmncsc->regVal != IM_NULL);
	IM_ASSERT(cmncsc->regOfst != IM_NULL);

	if(cmncsc->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("cmncsc has been disabled")));
		return IM_RET_OK;	
	}
	//disable cmncsc
	SetIspRegister(cmncsc->regVal, ISP_GLB_CFG2_CMNCSCBYPASS, 1);
	IM_JIF(isppwl_write_reg(cmncsc->regOfst[rISP_GLB_CFG2], cmncsc->regVal[rISP_GLB_CFG2]));

	cmncsc->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
