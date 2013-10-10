/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_af.c
--
--  Description :
--		af module.
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
#include "isp_af.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"AF_I:"
#define WARNHEAD	"AF_W:"
#define ERRHEAD		"AF_E:"
#define TIPHEAD		"AF_T:"


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: af_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_init(isp_af_context_t *af, isp_af_config_t *cfg)
{
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
		
	/*check af type*/
	if(cfg->type > ISP_AF_TYPE_NINE_WINS)
	{
		IM_ERRMSG((IM_STR("Invalid type: type=%d"), cfg->type));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//check af blokPosition
	for(i=0; i<9; i++)
	{
		if((cfg->coordinate.blokPos[i].x0 < 0) || (cfg->coordinate.blokPos[i].x0 > 4095)
			|| (cfg->coordinate.blokPos[i].y0 < 0) || (cfg->coordinate.blokPos[i].y0 > 4095)
			|| (cfg->coordinate.blokPos[i].x1 < 0) || (cfg->coordinate.blokPos[i].x1 > 4095)
			|| (cfg->coordinate.blokPos[i].y1 < 0) || (cfg->coordinate.blokPos[i].y1 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid blokPosition: x%d0=%d, y%d0=%d, x%d1=%d, y%d1=%d"),
				i, cfg->coordinate.blokPos[i].x0, i, cfg->coordinate.blokPos[i].y0, i, cfg->coordinate.blokPos[i].x1, i, cfg->coordinate.blokPos[i].y1));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	
	if(cfg->enable == IM_TRUE)
	{
		/*set af type*/
		SetIspRegister(af->regVal, ISP_AF_CNTL_TYPE, cfg->type);

		/*set af block coordinate*/
		for(i=0; i<9; i++)
		{
			SetIspRegister(af->regVal, ISP_AF_P0ST_X00+4*i, cfg->coordinate.blokPos[i].x0);
			SetIspRegister(af->regVal, ISP_AF_P0ST_Y00+4*i, cfg->coordinate.blokPos[i].y0);
			SetIspRegister(af->regVal, ISP_AF_P0ED_X01+4*i, cfg->coordinate.blokPos[i].x1);
			SetIspRegister(af->regVal, ISP_AF_P0ED_Y01+4*i, cfg->coordinate.blokPos[i].y1);
		}

		/*set af enable*/
		SetIspRegister(af->regVal, ISP_GLB_CFG2_AFBYPASS, 0);
		
		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_CNTL], af->regVal[rISP_AF_CNTL]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ST], af->regVal[rISP_AF_P0ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ED], af->regVal[rISP_AF_P0ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ST], af->regVal[rISP_AF_P1ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ED], af->regVal[rISP_AF_P1ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ST], af->regVal[rISP_AF_P2ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ED], af->regVal[rISP_AF_P2ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ST], af->regVal[rISP_AF_P3ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ED], af->regVal[rISP_AF_P3ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ST], af->regVal[rISP_AF_P4ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ED], af->regVal[rISP_AF_P4ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ST], af->regVal[rISP_AF_P5ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ED], af->regVal[rISP_AF_P5ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ST], af->regVal[rISP_AF_P6ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ED], af->regVal[rISP_AF_P6ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ST], af->regVal[rISP_AF_P7ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ED], af->regVal[rISP_AF_P7ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ST], af->regVal[rISP_AF_P8ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ED], af->regVal[rISP_AF_P8ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_GLB_CFG2], af->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set af bypass*/
		SetIspRegister(af->regVal, ISP_GLB_CFG2_AFBYPASS, 1);		
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_GLB_CFG2], af->regVal[rISP_GLB_CFG2]));
	}

	af->type = cfg->type;
	isppwl_memcpy((void*)(&af->coordinate), (void *)(&cfg->coordinate), sizeof(isp_af_coordinate_t));
	af->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: af_set_type

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_set_type(isp_af_context_t *af, IM_UINT32 type)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);

	/*check af type*/
	if(type > ISP_AF_TYPE_NINE_WINS)
	{
		IM_ERRMSG((IM_STR("Invalid type: type=%d"), type));
		return IM_RET_INVALID_PARAMETER;
	}

	if(af->type == type)
	{
		IM_INFOMSG((IM_STR("af has already been this type")));
		return IM_RET_OK;	
	}
		
	if(af->enable == IM_TRUE)
	{
		SetIspRegister(af->regVal, ISP_AF_CNTL_TYPE, type);
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_CNTL], af->regVal[rISP_AF_CNTL]));
	}

	af->type = type;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: af_set_block_coordinate

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_set_block_coordinate(isp_af_context_t *af, isp_af_coordinate_t *coordinate)
{
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);
	IM_ASSERT(coordinate != IM_NULL);

	//check af blokPosition
	for(i=0; i<9; i++)
	{
		if((coordinate->blokPos[i].x0 < 0) || (coordinate->blokPos[i].x0 >4095)
			|| (coordinate->blokPos[i].y0 < 0) || (coordinate->blokPos[i].y0 >4095)
			|| (coordinate->blokPos[i].x1 < 0) || (coordinate->blokPos[i].x1 >4095)
			|| (coordinate->blokPos[i].y1 < 0) || (coordinate->blokPos[i].y1 >4095))
		{
			IM_ERRMSG((IM_STR("Invalid blokPosition: x%d0=%d, y%d0=%d, x%d1=%d, y%d1=%d"),
				i, coordinate->blokPos[i].x0, i, coordinate->blokPos[i].y0, i, coordinate->blokPos[i].x1, i, coordinate->blokPos[i].y1));
			return IM_RET_INVALID_PARAMETER;
		}
	}

	
	if(af->enable == IM_TRUE)
	{
		for(i=0; i<9; i++)
		{
			SetIspRegister(af->regVal, ISP_AF_P0ST_X00+i*4, coordinate->blokPos[i].x0);
			SetIspRegister(af->regVal, ISP_AF_P0ST_Y00+i*4, coordinate->blokPos[i].y0);
			SetIspRegister(af->regVal, ISP_AF_P0ED_X01+i*4, coordinate->blokPos[i].x1);
			SetIspRegister(af->regVal, ISP_AF_P0ED_Y01+i*4, coordinate->blokPos[i].y1);
		}
		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ST], af->regVal[rISP_AF_P0ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ED], af->regVal[rISP_AF_P0ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ST], af->regVal[rISP_AF_P1ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ED], af->regVal[rISP_AF_P1ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ST], af->regVal[rISP_AF_P2ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ED], af->regVal[rISP_AF_P2ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ST], af->regVal[rISP_AF_P3ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ED], af->regVal[rISP_AF_P3ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ST], af->regVal[rISP_AF_P4ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ED], af->regVal[rISP_AF_P4ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ST], af->regVal[rISP_AF_P5ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ED], af->regVal[rISP_AF_P5ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ST], af->regVal[rISP_AF_P6ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ED], af->regVal[rISP_AF_P6ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ST], af->regVal[rISP_AF_P7ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ED], af->regVal[rISP_AF_P7ED]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ST], af->regVal[rISP_AF_P8ST]));
		IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ED], af->regVal[rISP_AF_P8ED]));
	}

	isppwl_memcpy((void*)(&af->coordinate), (void *)(coordinate), sizeof(isp_af_coordinate_t));	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: af_get_result

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_get_result(isp_af_context_t *af, isp_af_result_t *rsut)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);
	IM_ASSERT(rsut != IM_NULL);

	IM_JIF(isppwl_read_reg(af->regOfst[rISP_AF_RSUT], &af->regVal[rISP_AF_RSUT]));
	af->rsut.value = GetIspRegister(af->regVal, ISP_AF_RSUT);
	rsut->value = af->rsut.value;

	for(i=0; i<9; i++)
	{
		IM_JIF(isppwl_read_reg(af->regOfst[rISP_AF_CNT0 + i], &af->regVal[rISP_AF_CNT0 + i]));
		af->rsut.cnt[i] = GetIspRegister(af->regVal, ISP_AF_CNT0+i);
		rsut->cnt[i] = af->rsut.cnt[i];	
	}

	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: af_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_set_enable(isp_af_context_t *af)
{
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);

	if(af->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("af has been enabled")));
		return IM_RET_OK;	
	}
	
	/*set af type*/
	SetIspRegister(af->regVal, ISP_AF_CNTL_TYPE, af->type);

	/*set af block coordinate*/
	for(i=0; i<9; i++)
	{
		SetIspRegister(af->regVal, ISP_AF_P0ST_X00+4*i, af->coordinate.blokPos[i].x0);
		SetIspRegister(af->regVal, ISP_AF_P0ST_Y00+4*i, af->coordinate.blokPos[i].y0);
		SetIspRegister(af->regVal, ISP_AF_P0ED_X01+4*i, af->coordinate.blokPos[i].x1);
		SetIspRegister(af->regVal, ISP_AF_P0ED_Y01+4*i, af->coordinate.blokPos[i].y1);
	}

	/*set af enable*/
	SetIspRegister(af->regVal, ISP_GLB_CFG2_AFBYPASS, 0);

	//isppwl_write_regs();
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_CNTL], af->regVal[rISP_AF_CNTL]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ST], af->regVal[rISP_AF_P0ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P0ED], af->regVal[rISP_AF_P0ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ST], af->regVal[rISP_AF_P1ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P1ED], af->regVal[rISP_AF_P1ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ST], af->regVal[rISP_AF_P2ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P2ED], af->regVal[rISP_AF_P2ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ST], af->regVal[rISP_AF_P3ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P3ED], af->regVal[rISP_AF_P3ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ST], af->regVal[rISP_AF_P4ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P4ED], af->regVal[rISP_AF_P4ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ST], af->regVal[rISP_AF_P5ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P5ED], af->regVal[rISP_AF_P5ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ST], af->regVal[rISP_AF_P6ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P6ED], af->regVal[rISP_AF_P6ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ST], af->regVal[rISP_AF_P7ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P7ED], af->regVal[rISP_AF_P7ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ST], af->regVal[rISP_AF_P8ST]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_AF_P8ED], af->regVal[rISP_AF_P8ED]));
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_GLB_CFG2], af->regVal[rISP_GLB_CFG2]));

	af->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: af_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET af_set_disable(isp_af_context_t *af)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(af != IM_NULL);
	IM_ASSERT(af->regVal != IM_NULL);
	IM_ASSERT(af->regOfst != IM_NULL);

	if(af->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("af has been disabled")));
		return IM_RET_OK;	
	}
	//disable af
	
	SetIspRegister(af->regVal, ISP_GLB_CFG2_AFBYPASS, 1);
	IM_JIF(isppwl_write_reg(af->regOfst[rISP_GLB_CFG2], af->regVal[rISP_GLB_CFG2]));

	af->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

