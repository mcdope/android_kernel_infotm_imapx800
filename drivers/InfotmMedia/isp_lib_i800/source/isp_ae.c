/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ae.c
--
--  Description :
--		ae module.
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
#include "isp_ae.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"AE_I:"
#define WARNHEAD	"AE_W:"
#define ERRHEAD		"AE_E:"
#define TIPHEAD		"AE_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: ae_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_init(isp_ae_context_t *ae, isp_ae_config_t *cfg)
{ 
	IM_UINT32 	i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	//check block select params
	if((cfg->blokSelect.blokNum > 25) || (cfg->blokSelect.blokEn > 0x1ffffff))
	{
		IM_ERRMSG((IM_STR("Invalid blokSelect: blokNum=%d, blokEn=0x%x"),
			cfg->blokSelect.blokNum, cfg->blokSelect.blokEn));
		return IM_RET_INVALID_PARAMETER;
	}
		
	//check log2(block pixnum)
	if(cfg->coordinate.log2_blokPixNum > 31)
	{
		IM_ERRMSG((IM_STR("Invalid log2_blokPixNum=%d"), cfg->coordinate.log2_blokPixNum));
		return IM_RET_INVALID_PARAMETER;
	}

	//check block blokPosition
	for(i=0; i<25; i++)
	{
		if((cfg->coordinate.blokPos[i].x0 < 0) || (cfg->coordinate.blokPos[i].x0 > 4095)
			|| (cfg->coordinate.blokPos[i].y0 < 0) || (cfg->coordinate.blokPos[i].y0 > 4095)
			|| (cfg->coordinate.blokPos[i].x1 < 0) || (cfg->coordinate.blokPos[i].x1 > 4095)
			|| (cfg->coordinate.blokPos[i].y1 < 0) || (cfg->coordinate.blokPos[i].y1 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid blokPosition[%d]: x0=%d, y0=%d, x1=%d, y1=%d"),
				i, cfg->coordinate.blokPos[i].x0, cfg->coordinate.blokPos[i].y0, cfg->coordinate.blokPos[i].x1, cfg->coordinate.blokPos[i].y1));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	
	if(cfg->enable == IM_TRUE)
	{
		/*set ae block num*/
		if(cfg->blokSelect.blokNum == 0)
		{
			SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/25);
		}
		else
		{
			SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/(cfg->blokSelect.blokNum));
		}
		/*set ae block enable*/
		SetIspRegister(ae->regVal, ISP_AE_BEN, cfg->blokSelect.blokEn);

		//set log2(block pixnum)
		SetIspRegister(ae->regVal, ISP_AE_CNTL_PNUM_N, cfg->coordinate.log2_blokPixNum);
		
		//set block blokPosition
		for(i=0; i<25; i++)
		{
			SetIspRegister(ae->regVal, (ISP_AE_X00_1+i*4), cfg->coordinate.blokPos[i].x1);
			SetIspRegister(ae->regVal, (ISP_AE_X00_0+i*4), cfg->coordinate.blokPos[i].x0);
			SetIspRegister(ae->regVal, (ISP_AE_Y00_1+i*4), cfg->coordinate.blokPos[i].y1);
			SetIspRegister(ae->regVal, (ISP_AE_Y00_0+i*4), cfg->coordinate.blokPos[i].y0);

			IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_X00+i*2], ae->regVal[rISP_AE_X00+i*2]));
			IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_Y00+i*2], ae->regVal[rISP_AE_Y00+i*2]));
		}
		
		/*set ae bypass*/
		SetIspRegister(ae->regVal, ISP_GLB_CFG2_AEBYPASS, 0);

		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_CNTL], ae->regVal[rISP_AE_CNTL]));
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_BEN], ae->regVal[rISP_AE_BEN]));		
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_GLB_CFG2], ae->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set ae bypass*/
		SetIspRegister(ae->regVal, ISP_GLB_CFG2_AEBYPASS, 1);
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_GLB_CFG2], ae->regVal[rISP_GLB_CFG2]));
	}
	
	ae->blokSelect.blokNum = cfg->blokSelect.blokNum;
	ae->blokSelect.blokEn = cfg->blokSelect.blokEn;
	isppwl_memcpy((void*)(&ae->coordinate), (void *)(&cfg->coordinate), sizeof(isp_ae_coordinate_t));
	ae->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ae_set_block_select

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_set_block_select(isp_ae_context_t *ae, isp_ae_block_select *blokSelect)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);
	IM_ASSERT(blokSelect != IM_NULL);

	//check block select params
	if((blokSelect->blokNum > 25) || (blokSelect->blokEn > 0x1ffffff))
	{
		IM_ERRMSG((IM_STR("Invalid blokSelect: blokNum=%d, blokEn=0x%x"),
			blokSelect->blokNum, blokSelect->blokEn));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(ae->enable == IM_TRUE)
	{
		/*set ae block num*/
		if(blokSelect->blokNum == 0)
		{
			SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/25);
		}
		else
		{
			SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/blokSelect->blokNum);
		}
		/*set ae block enable*/
		SetIspRegister(ae->regVal, ISP_AE_BEN, blokSelect->blokEn);
		
		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_CNTL], ae->regVal[rISP_AE_CNTL]));
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_BEN], ae->regVal[rISP_AE_BEN]));
	}

	ae->blokSelect.blokNum = blokSelect->blokNum;
	ae->blokSelect.blokEn = blokSelect->blokEn;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ae_set_block_coordinate

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_set_block_coordinate(isp_ae_context_t *ae, isp_ae_coordinate_t *coordinate)
{
	IM_UINT32	i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);
	IM_ASSERT(coordinate != IM_NULL);

	//check log2(block pixnum)
	if(coordinate->log2_blokPixNum > 31)
	{
		IM_ERRMSG((IM_STR("Invalid log2_blokPixNum=%d"), coordinate->log2_blokPixNum));
		return IM_RET_INVALID_PARAMETER;
	}

	//check block blokPosition
	for(i=0; i<25; i++)
	{
		if((coordinate->blokPos[i].x0 < 0) || (coordinate->blokPos[i].x0 > 4095)
			|| (coordinate->blokPos[i].y0 < 0) || (coordinate->blokPos[i].y0 > 4095)
			|| (coordinate->blokPos[i].x1 < 0) || (coordinate->blokPos[i].x1 > 4095)
			|| (coordinate->blokPos[i].y1 < 0) || (coordinate->blokPos[i].y1 > 4095))
		{
			IM_ERRMSG((IM_STR("Invalid blokPosition[%d]: x0=%d, y0=%d, x1=%d, y1=%d"),
				i, coordinate->blokPos[i].x0, coordinate->blokPos[i].y0, coordinate->blokPos[i].x1, coordinate->blokPos[i].y1));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	
	if(ae->enable == IM_TRUE)
	{	
		//set log2(block pixnum)
		SetIspRegister(ae->regVal, ISP_AE_CNTL_PNUM_N, coordinate->log2_blokPixNum);
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_CNTL], ae->regVal[rISP_AE_CNTL]));
		
		//set block blokPosition
		for(i=0; i<25; i++)
		{
			SetIspRegister(ae->regVal, (ISP_AE_X00_1+i*4), coordinate->blokPos[i].x1);
			SetIspRegister(ae->regVal, (ISP_AE_X00_0+i*4), coordinate->blokPos[i].x0);
			SetIspRegister(ae->regVal, (ISP_AE_Y00_1+i*4), coordinate->blokPos[i].y1);
			SetIspRegister(ae->regVal, (ISP_AE_Y00_0+i*4), coordinate->blokPos[i].y0);

			IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_X00+i*2], ae->regVal[rISP_AE_X00+i*2]));
			IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_Y00+i*2], ae->regVal[rISP_AE_Y00+i*2]));
		}
	}
	
	isppwl_memcpy((void*)(&ae->coordinate), (void *)(coordinate), sizeof(isp_ae_coordinate_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ae_get_result

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_get_result(isp_ae_context_t *ae, isp_ae_result_t *rsut)
{ 
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);
	IM_ASSERT(rsut != IM_NULL);

	IM_JIF(isppwl_read_reg(ae->regOfst[rISP_AE_YAVG], &ae->regVal[rISP_AE_YAVG]));
	rsut->avg = GetIspRegister(ae->regVal, ISP_AE_YAVG);
	
	//isppwl_read_regs();
	for(i=0; i<25; i++)
	{
		IM_JIF(isppwl_read_reg(ae->regOfst[rISP_COUNTER00 + i], &ae->regVal[rISP_COUNTER00 + i]));
		rsut->cnt[i] = GetIspRegister(ae->regVal, ISP_COUNTER00+i);
	}
		
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ae_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_set_enable(isp_ae_context_t *ae)
{
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);

	if(ae->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ae has been enabled")));
		return IM_RET_OK;	
	}

	/*set ae block num*/
	if(ae->blokSelect.blokNum == 0)
	{
		SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/25);
	}
	else
	{
		SetIspRegister(ae->regVal, ISP_AE_CNTL_NUM_1, (1<<15)/(ae->blokSelect.blokNum));
	}
	
	/*set ae block enable*/
	SetIspRegister(ae->regVal, ISP_AE_BEN, ae->blokSelect.blokEn);

	//set log2(block pixnum)
	SetIspRegister(ae->regVal, ISP_AE_CNTL_PNUM_N, ae->coordinate.log2_blokPixNum);
	
	//set block blokPosition
	for(i=0; i<25; i++)
	{
		SetIspRegister(ae->regVal, (ISP_AE_X00_1+i*4), ae->coordinate.blokPos[i].x1);
		SetIspRegister(ae->regVal, (ISP_AE_X00_0+i*4), ae->coordinate.blokPos[i].x0);
		SetIspRegister(ae->regVal, (ISP_AE_Y00_1+i*4), ae->coordinate.blokPos[i].y1);
		SetIspRegister(ae->regVal, (ISP_AE_Y00_0+i*4), ae->coordinate.blokPos[i].y0);

		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_X00+i*2], ae->regVal[rISP_AE_X00+i*2]));
		IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_Y00+i*2], ae->regVal[rISP_AE_Y00+i*2]));
	}
	
	/*set ae bypass*/
	SetIspRegister(ae->regVal, ISP_GLB_CFG2_AEBYPASS, 0);

	//isppwl_write_regs();
	IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_CNTL], ae->regVal[rISP_AE_CNTL]));
	IM_JIF(isppwl_write_reg(ae->regOfst[rISP_AE_BEN], ae->regVal[rISP_AE_BEN]));		
	IM_JIF(isppwl_write_reg(ae->regOfst[rISP_GLB_CFG2], ae->regVal[rISP_GLB_CFG2]));

	ae->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ae_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ae_set_disable(isp_ae_context_t *ae)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ae != IM_NULL);
	IM_ASSERT(ae->regVal != IM_NULL);
	IM_ASSERT(ae->regOfst != IM_NULL);

	if(ae->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("ae has been disabled")));
		return IM_RET_OK;	
	}

	/*set ae bypass*/
	SetIspRegister(ae->regVal, ISP_GLB_CFG2_AEBYPASS, 1);
	IM_JIF(isppwl_write_reg(ae->regOfst[rISP_GLB_CFG2], ae->regVal[rISP_GLB_CFG2]));

	ae->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
