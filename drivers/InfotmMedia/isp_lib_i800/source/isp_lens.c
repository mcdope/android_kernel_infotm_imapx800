/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_lens.c
--
--  Description :
--		lens module.
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
#include "isp_lens.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"LENS_I:"
#define WARNHEAD	"LENS_W:"
#define ERRHEAD		"LENS_E:"
#define TIPHEAD		"LENS_T:"


/**************************************************
*             isp lens shading coef               *
*  {R_EVEN, R_ODD, G_EVEN, G_ODD, B_EVEN, B_ODD}  *
**************************************************/
//coef0:320*240, coef1:640*480, coef2:800*480, coef3:1920*1080, coef4:409683072
static const IM_UINT32 lensLutbCoef[][ISP_LENS_LUTB_LENGTH][6] = {
#include "isp_lens_table.h"
};

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: lens_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET lens_init(isp_lens_context_t *lens, isp_lens_config_t *cfg)
{ 
	IM_UINT32 i, j;
	IM_UINT32 lutb_offset;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(lens != IM_NULL);
	IM_ASSERT(lens->regVal != IM_NULL);
	IM_ASSERT(lens->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	/*check lens shading rh and rv*/
	if((lens->height <= 0) || (lens->width <= 0))
	{
		IM_ERRMSG((IM_STR("Invalid size: height=%d, width=%d"), lens->height, lens->width));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check lens shading rh and rv*/
	if((cfg->lutbMode != ISP_LENS_LUTB_MODE_0)
		&&(cfg->lutbMode != ISP_LENS_LUTB_MODE_1)
		&&(cfg->lutbMode != ISP_LENS_LUTB_MODE_2)
		&&(cfg->lutbMode != ISP_LENS_LUTB_MODE_3)
		&&(cfg->lutbMode != ISP_LENS_LUTB_MODE_4))
	{
		IM_ERRMSG((IM_STR("Invalid lens shading lutbMode: lutbMode%d"), cfg->lutbMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->enable == IM_TRUE)
	{
		SetIspRegister(lens->regVal, ISP_GLB_RH, (1024*1024)/lens->height);
		SetIspRegister(lens->regVal, ISP_GLB_RV, (1024*1024)/lens->width);
		
		/*set lens shading enable */
		SetIspRegister(lens->regVal, ISP_GLB_CFG2_LENSBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_RH], lens->regVal[rISP_GLB_RH]));
		IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_RV], lens->regVal[rISP_GLB_RV]));
		IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_CFG2], lens->regVal[rISP_GLB_CFG2]));

		//write lens shading lookup table
		for(i=0; i<6; i++)
		{
			for(j=0; j<ISP_LENS_LUTB_LENGTH; j++)
			{
				lutb_offset = ISP_LENS_LUTB0 + (i*0x400) + j*4;
				IM_JIF(isppwl_write_reg(lutb_offset, lensLutbCoef[cfg->lutbMode][j][i]));
			}
		}
		lens->lutbModeNeedChange = IM_FALSE;
	}
	else
	{
		/*set lens shading bypass */
		SetIspRegister(lens->regVal, ISP_GLB_CFG2_LENSBYPASS, 1);
		IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_CFG2], lens->regVal[rISP_GLB_CFG2]));
		lens->lutbModeNeedChange = IM_TRUE;
	}
	
	lens->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: lens_set_lutb

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET lens_set_lutb(isp_lens_context_t *lens, IM_UINT32 lutbMode)
{ 
	IM_UINT32 i, j;
	IM_UINT32 lutb_offset;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(lens != IM_NULL);
	IM_ASSERT(lens->regVal != IM_NULL);
	IM_ASSERT(lens->regOfst != IM_NULL);

	/*check lens shading rh and rv*/
	if(lutbMode > ISP_LENS_LUTB_MODE_4)
	{
		IM_ERRMSG((IM_STR("Invalid lens shading lutbMode: lutbMode%d"), lutbMode));
		return IM_RET_INVALID_PARAMETER;
	}
	if((lens->lutbModeNeedChange == IM_TRUE) || (lens->lutbMode != lutbMode))
	{
		lens->lutbModeNeedChange = IM_TRUE;
	}
	else
	{
		lens->lutbModeNeedChange = IM_FALSE;
	}
	
	if((lens->enable == IM_TRUE) && (lens->lutbModeNeedChange == IM_TRUE))
	{
		//write lens shading lookup table
		for(i=0; i<6; i++)
		{
			for(j=0; j<ISP_LENS_LUTB_LENGTH; j++)
			{
				lutb_offset = ISP_LENS_LUTB0 + (i*0x400) + j*4;
				IM_JIF(isppwl_write_reg(lutb_offset, lensLutbCoef[lutbMode][j][i]));
			}
		}
		lens->lutbModeNeedChange = IM_FALSE;
	}
	lens->lutbMode = lutbMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: lens_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET lens_set_enable(isp_lens_context_t *lens)
{
	IM_UINT32 i, j;
	IM_UINT32 lutb_offset;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(lens != IM_NULL);
	IM_ASSERT(lens->regVal != IM_NULL);
	IM_ASSERT(lens->regOfst != IM_NULL);

	if(lens->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("lens has been enabled")));
		return IM_RET_OK;	
	}
	//enable lens 
	/*set lens shading rh and rv*/
	if((lens->height <= 0) || (lens->width <= 0))
	{
		IM_ERRMSG((IM_STR("Invalid size: height=%d, width=%d"), lens->height, lens->width));
		return IM_RET_INVALID_PARAMETER;
	}

	SetIspRegister(lens->regVal, ISP_GLB_RH, (1024*1024)/lens->height);
	SetIspRegister(lens->regVal, ISP_GLB_RV, (1024*1024)/lens->width);
	SetIspRegister(lens->regVal, ISP_GLB_CFG2_LENSBYPASS, 0);

	IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_RH], lens->regVal[rISP_GLB_RH]));
	IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_RV], lens->regVal[rISP_GLB_RV]));
	IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_CFG2], lens->regVal[rISP_GLB_CFG2]));

	if(lens->lutbModeNeedChange == IM_TRUE)
	{
		//write lens shading lookup table
		for(i=0; i<6; i++)
		{
			for(j=0; j<ISP_LENS_LUTB_LENGTH; j++)
			{
				lutb_offset = ISP_LENS_LUTB0 + (i*0x400) + j*4;
				IM_JIF(isppwl_write_reg(lutb_offset, lensLutbCoef[lens->lutbMode][j][i]));
			}
		}
		lens->lutbModeNeedChange = IM_FALSE;
	}

	lens->enable = IM_TRUE;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: lens_set_disable

  Functional description:

Inputs:

Outputs:

Returns: 

------------------------------------------------------------------------------*/
IM_RET lens_set_disable(isp_lens_context_t *lens)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(lens != IM_NULL);
	IM_ASSERT(lens->regVal != IM_NULL);
	IM_ASSERT(lens->regOfst != IM_NULL);

	if(lens->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("lens has been disabled")));
		return IM_RET_OK;	
	}
	//disable lens
	SetIspRegister(lens->regVal, ISP_GLB_CFG2_LENSBYPASS, 1);
	IM_JIF(isppwl_write_reg(lens->regOfst[rISP_GLB_CFG2], lens->regVal[rISP_GLB_CFG2]));

	lens->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
