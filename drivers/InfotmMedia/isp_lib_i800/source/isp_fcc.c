/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_fcc.c
--
--  Description :
--		fcc module.
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
#include "isp_fcc.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"FCC_I:"
#define WARNHEAD	"FCC_W:"
#define ERRHEAD		"FCC_E:"
#define TIPHEAD		"FCC_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: fcc_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET fcc_init(isp_fcc_context_t *fcc, isp_fcc_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(fcc != IM_NULL);
	IM_ASSERT(fcc->regVal != IM_NULL);
	IM_ASSERT(fcc->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check fcc threshold*/
	if(cfg->threshold > 255)
	{
		IM_ERRMSG((IM_STR("Invalid threshold: threshold=%d"), cfg->threshold));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set fcc threshold and enable*/
		SetIspRegister(fcc->regVal, ISP_FCC_THD, cfg->threshold);
		SetIspRegister(fcc->regVal, ISP_GLB_CFG2_FCCBYPASS, 0);
		IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_FCC_THD], fcc->regVal[rISP_FCC_THD]));
		IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_GLB_CFG2], fcc->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		SetIspRegister(fcc->regVal, ISP_GLB_CFG2_FCCBYPASS, 1);
		IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_GLB_CFG2], fcc->regVal[rISP_GLB_CFG2]));
	}

	fcc->threshold = cfg->threshold;
	fcc->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: fcc_set_threshold

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET fcc_set_threshold(isp_fcc_context_t *fcc, IM_UINT32 threshold)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(fcc != IM_NULL);
	IM_ASSERT(fcc->regVal != IM_NULL);
	IM_ASSERT(fcc->regOfst != IM_NULL);

	/*check fcc threshold*/
	if(threshold > 255)
	{
		IM_ERRMSG((IM_STR("Invalid threshold: threshold=%d"), threshold));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(fcc->enable == IM_TRUE)
	{
		/*set fcc threshold*/
		SetIspRegister(fcc->regVal, ISP_FCC_THD, threshold);
		IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_FCC_THD], fcc->regVal[rISP_FCC_THD]));
	}
	
	fcc->threshold = threshold;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: fcc_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET fcc_set_enable(isp_fcc_context_t *fcc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(fcc != IM_NULL);
	IM_ASSERT(fcc->regVal != IM_NULL);
	IM_ASSERT(fcc->regOfst != IM_NULL);

	if(fcc->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("fcc has been enabled")));
		return IM_RET_OK;	
	}

	SetIspRegister(fcc->regVal, ISP_FCC_THD, fcc->threshold);
	SetIspRegister(fcc->regVal, ISP_GLB_CFG2_FCCBYPASS, 0);
	IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_FCC_THD], fcc->regVal[rISP_FCC_THD]));
	IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_GLB_CFG2], fcc->regVal[rISP_GLB_CFG2]));

	fcc->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: fcc_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET fcc_set_disable(isp_fcc_context_t *fcc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(fcc != IM_NULL);
	IM_ASSERT(fcc->regVal != IM_NULL);
	IM_ASSERT(fcc->regOfst != IM_NULL);

	if(fcc->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("fcc has been disabled")));
		return IM_RET_OK;	
	}
	//disable fcc
	SetIspRegister(fcc->regVal, ISP_GLB_CFG2_FCCBYPASS, 1);
	IM_JIF(isppwl_write_reg(fcc->regOfst[rISP_GLB_CFG2], fcc->regVal[rISP_GLB_CFG2]));

	fcc->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
