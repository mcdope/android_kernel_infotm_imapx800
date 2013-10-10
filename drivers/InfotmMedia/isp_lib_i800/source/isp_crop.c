/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_crop.c
--
--  Description :
--		crop module.
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
#include "isp_crop.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CROP_I:"
#define WARNHEAD	"CROP_W:"
#define ERRHEAD		"CROP_E:"
#define TIPHEAD		"CROP_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: crop_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET crop_init(isp_crop_context_t *crop, isp_crop_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(crop != IM_NULL);
	IM_ASSERT(crop->regVal != IM_NULL);
	IM_ASSERT(crop->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check crop coordinate*/
	if((cfg->coordinate.x0 > 4095) || (cfg->coordinate.y0 > 4095)
		|| (cfg->coordinate.x1 > 4095) || (cfg->coordinate.y1 > 4095)
		|| (cfg->coordinate.x1 < cfg->coordinate.x0) || (cfg->coordinate.y1 < cfg->coordinate.y0))
	{
		IM_ERRMSG((IM_STR("Invalid position: x0=%d, y0=%d, x1=%d, y1=%d"),
			cfg->coordinate.x0, cfg->coordinate.y0, cfg->coordinate.x1, cfg->coordinate.y1));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set crop coordinate and enable*/
		SetIspRegister(crop->regVal, ISP_CROP_START_X, cfg->coordinate.x0);
		SetIspRegister(crop->regVal, ISP_CROP_START_Y, cfg->coordinate.y0);
		SetIspRegister(crop->regVal, ISP_CROP_END_X, cfg->coordinate.x1);
		SetIspRegister(crop->regVal, ISP_CROP_END_Y, cfg->coordinate.y1);
		SetIspRegister(crop->regVal, ISP_GLB_CFG2_CROPBYPASS, 0);

		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_START], crop->regVal[rISP_CROP_START]));
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_END], crop->regVal[rISP_CROP_END]));
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_GLB_CFG2], crop->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		SetIspRegister(crop->regVal, ISP_GLB_CFG2_CROPBYPASS, 1);
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_GLB_CFG2], crop->regVal[rISP_GLB_CFG2]));
	}

	isppwl_memcpy((void*)(&crop->coordinate), (void *)(&cfg->coordinate), sizeof(isp_crop_coordinate_t));	
	crop->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: crop_set_coordinate

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET crop_set_coordinate(isp_crop_context_t *crop, isp_crop_coordinate_t *coordinate)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(crop != IM_NULL);
	IM_ASSERT(crop->regVal != IM_NULL);
	IM_ASSERT(crop->regOfst != IM_NULL);
	IM_ASSERT(coordinate != IM_NULL);

	//check crop coordinate
	if((coordinate->x0 > 4095) || (coordinate->y0 > 4095)
		|| (coordinate->x1 > 4095) || (coordinate->y1 > 4095)
		|| (coordinate->x1 < coordinate->x0) || (coordinate->y1 < coordinate->y0))
	{
		IM_ERRMSG((IM_STR("Invalid position: x0=%d, y0=%d, x1=%d, y1=%d"),
			coordinate->x0, coordinate->y0, coordinate->x1, coordinate->y1));
		return IM_RET_INVALID_PARAMETER;
	}

	
	if(crop->enable == IM_TRUE)
	{
		SetIspRegister(crop->regVal, ISP_CROP_START_X, coordinate->x0);
		SetIspRegister(crop->regVal, ISP_CROP_START_Y, coordinate->y0);
		SetIspRegister(crop->regVal, ISP_CROP_END_X, coordinate->x1);
		SetIspRegister(crop->regVal, ISP_CROP_END_Y, coordinate->y1);

		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_START], crop->regVal[rISP_CROP_START]));
		IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_END], crop->regVal[rISP_CROP_END]));
	}

	isppwl_memcpy((void*)(&crop->coordinate), (void *)(coordinate), sizeof(isp_crop_coordinate_t));	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: crop_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET crop_set_enable(isp_crop_context_t *crop)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(crop != IM_NULL);
	IM_ASSERT(crop->regVal != IM_NULL);
	IM_ASSERT(crop->regOfst != IM_NULL);

	if(crop->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("crop has been enabled")));
		return IM_RET_OK;	
	}

	SetIspRegister(crop->regVal, ISP_CROP_START_X, crop->coordinate.x0);
	SetIspRegister(crop->regVal, ISP_CROP_START_Y, crop->coordinate.y0);
	SetIspRegister(crop->regVal, ISP_CROP_END_X, crop->coordinate.x1);
	SetIspRegister(crop->regVal, ISP_CROP_END_Y, crop->coordinate.y1);
	SetIspRegister(crop->regVal, ISP_GLB_CFG2_CROPBYPASS, 0);

	IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_START], crop->regVal[rISP_CROP_START]));
	IM_JIF(isppwl_write_reg(crop->regOfst[rISP_CROP_END], crop->regVal[rISP_CROP_END]));
	IM_JIF(isppwl_write_reg(crop->regOfst[rISP_GLB_CFG2], crop->regVal[rISP_GLB_CFG2]));

	crop->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: crop_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET crop_set_disable(isp_crop_context_t *crop)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(crop != IM_NULL);
	IM_ASSERT(crop->regVal != IM_NULL);
	IM_ASSERT(crop->regOfst != IM_NULL);

	if(crop->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("crop has been disabled")));
		return IM_RET_OK;	
	}
	//disable crop
	SetIspRegister(crop->regVal, ISP_GLB_CFG2_CROPBYPASS, 1);
	IM_JIF(isppwl_write_reg(crop->regOfst[rISP_GLB_CFG2], crop->regVal[rISP_GLB_CFG2]));

	crop->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
