/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_gma.c
--
--  Description :
--		gma module.
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
#include "isp_gma.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"GMA_I:"
#define WARNHEAD	"GMA_W:"
#define ERRHEAD		"GMA_E:"
#define TIPHEAD		"GMA_T:"

/*gma=1.0,gma=1.8,gma=2.0,gma=2.2,gma=2.4,gma=2.6,gma=0.56,gma=0.5,gma=0.45,gma=0.42,gma=0.38*/
/*rcoef,
  gcoef,
  bcoef*/
//gma=1.0(default mode)
static const IM_UINT32 gmaCoef[96][11] = {
#include "isp_gma_table.h"
};

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: gma_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET gma_init(isp_gma_context_t *gma, isp_gma_config_t *cfg)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gma != IM_NULL);
	IM_ASSERT(gma->regVal != IM_NULL);
	IM_ASSERT(gma->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	/*check gma round mode*/
	if(cfg->rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("gma round mode error: rdMode = %d"), cfg->rdMode));
		return IM_RET_OK;
	}

	/*check gma coef mode*/
	if(cfg->mode > ISP_GMA_MODE_10)
	{
		IM_ERRMSG((IM_STR("gma mode error: mode = %d"), cfg->mode));
		return IM_RET_OK;
	}

	if(cfg->mode != ISP_GMA_MODE_0)//default mode
	{
		gma->modeNeedChange = IM_TRUE;
	}
	else
	{
		gma->modeNeedChange = IM_FALSE;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set gma round mode*/
		SetIspRegister(gma->regVal, ISP_GMA_ROUND, cfg->rdMode);

		/*set gma coef*/
		if(gma->modeNeedChange == IM_TRUE)
		{
			for(i=ISP_GMA_RCOEF00; i<ISP_GMA_BCOEF1F+1; i++)
			{
				SetIspRegister(gma->regVal, i, gmaCoef[i-ISP_GMA_RCOEF00][cfg->mode]);
				IM_JIF(isppwl_write_reg(gma->regOfst[i], gma->regVal[i]));
			}
			gma->modeNeedChange = IM_FALSE;
		}
		else
		{
			//round mode
			IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GMA_RCOEF00], gma->regVal[rISP_GMA_RCOEF00]));
		}

		/*set gma enable*/
		SetIspRegister(gma->regVal, ISP_GLB_CFG2_GABYPASS, 0);
	}
	else
	{
		/*set gma bypass*/
		SetIspRegister(gma->regVal, ISP_GLB_CFG2_GABYPASS, 1);
	}

	IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GLB_CFG2], gma->regVal[rISP_GLB_CFG2]));

	gma->rdMode = cfg->rdMode;
	gma->mode = cfg->mode;	
	gma->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: gma_set_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET gma_set_mode(isp_gma_context_t *gma, IM_UINT32 mode)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gma != IM_NULL);
	IM_ASSERT(gma->regVal != IM_NULL);
	IM_ASSERT(gma->regOfst != IM_NULL);

	/*check gma coef mode*/
	if(mode > ISP_GMA_MODE_10)
	{
		IM_ERRMSG((IM_STR("gma mode error: mode = %d"), mode));
		return IM_RET_OK;
	}

	if((gma->modeNeedChange == IM_TRUE) || (gma->mode != mode))
	{
		gma->modeNeedChange = IM_TRUE;
	}
	else
	{
		gma->modeNeedChange = IM_FALSE;
	}
#if 0
	if((gma->enable == IM_TRUE) && (gma->modeNeedChange == IM_TRUE))
	{
		for(i=ISP_GMA_RCOEF00; i<ISP_GMA_BCOEF1F+1; i++)
		{
			SetIspRegister(gma->regVal, i, gmaCoef[i-ISP_GMA_RCOEF00][mode]);
			IM_JIF(isppwl_write_reg(gma->regOfst[i], gma->regVal[i]));
		}
		gma->modeNeedChange = IM_FALSE;
	}
#else
	if(gma->enable == IM_TRUE)
	{
		if(gma->modeNeedChange == IM_TRUE)
		{
			for(i=ISP_GMA_RCOEF00; i<ISP_GMA_BCOEF1F+1; i++)
			{
				SetIspRegister(gma->regVal, i, gmaCoef[i-ISP_GMA_RCOEF00][mode]);
				IM_JIF(isppwl_write_reg(gma->regOfst[i], gma->regVal[i]));
			}
			gma->modeNeedChange = IM_FALSE;
		}
	}
#endif

	gma->mode = mode;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: gma_set_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET gma_set_round_mode(isp_gma_context_t *gma, IM_UINT32 rdMode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gma != IM_NULL);
	IM_ASSERT(gma->regVal != IM_NULL);
	IM_ASSERT(gma->regOfst != IM_NULL);

	/*check gma round mode*/
	if(rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("gma round mode error: rdMode = %d"), rdMode));
		return IM_RET_OK;
	}

	if(gma->rdMode == rdMode)
	{
		IM_WARNMSG((IM_STR("gma is already this rdMode")));
		return IM_RET_OK;
	}

	if(gma->enable == IM_TRUE)
	{
		SetIspRegister(gma->regVal, ISP_GMA_ROUND, rdMode);
		IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GMA_RCOEF00], gma->regVal[rISP_GMA_RCOEF00]));
	}

	gma->rdMode = rdMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
/*------------------------------------------------------------------------------

    Function name: gma_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET gma_set_enable(isp_gma_context_t *gma)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gma != IM_NULL);
	IM_ASSERT(gma->regVal != IM_NULL);
	IM_ASSERT(gma->regOfst != IM_NULL);

	if(gma->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("gma has been enabled")));
		return IM_RET_OK;	
	}
	
	//enable gma 
	/*set gma round mode*/
	SetIspRegister(gma->regVal, ISP_GMA_ROUND, gma->rdMode);

	/*set gma coef mode*/
	if(gma->modeNeedChange == IM_TRUE)
	{
		for(i=ISP_GMA_RCOEF00; i<ISP_GMA_BCOEF1F+1; i++)
		{
			SetIspRegister(gma->regVal, i, gmaCoef[i-ISP_GMA_RCOEF00][gma->mode]);
			IM_JIF(isppwl_write_reg(gma->regOfst[i], gma->regVal[i]));
		}
		gma->modeNeedChange = IM_FALSE;
	}
	else
	{
		//round mode
		IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GMA_RCOEF00], gma->regVal[rISP_GMA_RCOEF00]));
	}

	/*set gma enable*/
	SetIspRegister(gma->regVal, ISP_GLB_CFG2_GABYPASS, 0);
	IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GLB_CFG2], gma->regVal[rISP_GLB_CFG2]));

	gma->enable = IM_TRUE;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: gma_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET gma_set_disable(isp_gma_context_t *gma)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gma != IM_NULL);
	IM_ASSERT(gma->regVal != IM_NULL);
	IM_ASSERT(gma->regOfst != IM_NULL);

	if(gma->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("gma has been disabled")));
		return IM_RET_OK;	
	}
	
	//disable gma
	SetIspRegister(gma->regVal, ISP_GLB_CFG2_GABYPASS, 1);
	IM_JIF(isppwl_write_reg(gma->regOfst[rISP_GLB_CFG2], gma->regVal[rISP_GLB_CFG2]));

	gma->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
