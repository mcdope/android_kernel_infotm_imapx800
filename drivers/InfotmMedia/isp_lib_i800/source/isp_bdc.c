/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_bdc.c
--
--  Description :
--		bdc module.
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
#include "isp_bdc.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"BDC_I:"
#define WARNHEAD	"BDC_W:"
#define ERRHEAD		"BDC_E:"
#define TIPHEAD		"BDC_T:"



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: bdc_set_dma

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_set_dma(isp_bdc_context_t *bdc, bdc_dma_context_t *dma)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 i;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);
	IM_ASSERT(dma != IM_NULL);

	if((dma->direct != ISP_BDC_DMA_WRITE) && (dma->direct != ISP_BDC_DMA_READ)){
		IM_ERRMSG((IM_STR("Invalid dma direct(%d)"), dma->direct));
		return IM_RET_INVALID_PARAMETER;
	}

	if(dma->ppMode > ISP_PPMODE_4_BUFFER){
		IM_ERRMSG((IM_STR("Invalid ppMode(%d)"), dma->ppMode));
		return IM_RET_INVALID_PARAMETER;
	}
	if(dma->length < 0){
		IM_ERRMSG((IM_STR("Invalid length(%d)"), dma->length));
		return IM_RET_INVALID_PARAMETER;
	}

	// Address must 64-bits(8bytes) aligment
	for(i=0; i<((dma->ppMode==ISP_PPMODE_DISABLE)?1:dma->ppMode); i++){
		if((dma->buff[i].phy_addr & 0x7) || 
			(dma->buff[i].size < dma->length)){
			IM_ERRMSG((IM_STR("Invalid buff, vir=0x%x, phy=0x%x, size=%d, length=%d"), 
				(IM_UINT32)dma->buff[i].vir_addr, dma->buff[i].phy_addr, dma->buff[i].size, 
				dma->length));
			return IM_RET_INVALID_PARAMETER;
		}
	}
		
	//
	/*buf0*/
	//does transfer next frame? 
	if(dma->ppMode != ISP_PPMODE_DISABLE)
	{
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_NTFRMEN1, 1);
	}else{
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_NTFRMEN1, 0);
	}
	//auto reload set
	SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_AUTORELOAD1, 1);
	//dma dir(w/r memory)
	SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_FRMREAD, dma->direct);
	//Double Word(64bits,8bytes) length of data
	SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_FRMLEN1, dma->length >> 3);
	//address of buf0
	SetIspRegister(bdc->regVal, ISP_BPDMA_FBA1, dma->buff[0].phy_addr);

	if(dma->ppMode != ISP_PPMODE_DISABLE)
	{
		/*buf1*/
		//does transfer next frame? 
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC2_NTFRMEN2, 1);

		//auto reload set
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC2_AUTORELOAD2, 1);
		//dma dir(write to memory)
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC2_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC2_FRMLEN2, dma->length >> 3);
		//address of buf1
		SetIspRegister(bdc->regVal, ISP_BPDMA_FBA2, dma->buff[1].phy_addr);

		/*buf2*/
		//does transfer next frame? 
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC3_NTFRMEN3, 1);

		//auto reload set
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC3_AUTORELOAD3, 1);
		//dma dir(write to memory)
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC3_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC3_FRMLEN3, dma->length >> 3);
		//address of buf2
		SetIspRegister(bdc->regVal, ISP_BPDMA_FBA2, dma->buff[2].phy_addr);

		/*buf3*/
		//does transfer next frame? 
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC4_NTFRMEN4, 1);

		//auto reload set
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC4_AUTORELOAD4, 1);
		//dma dir(write to memory)
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC4_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC4_FRMLEN4, dma->length >> 3);
		//address of buf3
		SetIspRegister(bdc->regVal, ISP_BPDMA_FBA4, dma->buff[3].phy_addr);
	}

	//isppwl_write_regs();
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_FBA1], bdc->regVal[rISP_BPDMA_FBA1]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC2], bdc->regVal[rISP_BPDMA_CC2]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_FBA2], bdc->regVal[rISP_BPDMA_FBA2]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC3], bdc->regVal[rISP_BPDMA_CC3]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_FBA3], bdc->regVal[rISP_BPDMA_FBA3]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC4], bdc->regVal[rISP_BPDMA_CC4]));
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_FBA4], bdc->regVal[rISP_BPDMA_FBA4]));

	//
	//ch3(bdc dma) enable 
	//
	if(bdc->enable == IM_TRUE)
	{
		/*enable bdc dma, first bpdma enable, than ch3dma enable*/
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));

		SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));

	}
	return ret;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: bdc_init

  Functional description:

Inputs:

Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_init(isp_bdc_context_t *bdc, bdc_config_t *cfg)
{ 
	IM_RET ret;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	/*check bdc work mode*/
	if((cfg->bdcMode != ISP_BDC_MODE_DETECT)&&(cfg->bdcMode != ISP_BDC_MODE_CORRECT)){
		IM_ERRMSG((IM_STR("Invalid dma bdcMode(%d)"), cfg->bdcMode));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check bdc detect type*/
	if((cfg->detType != ISP_BDC_DETECT_TYPE_0)&&(cfg->detType != ISP_BDC_DETECT_TYPE_1)){
		IM_ERRMSG((IM_STR("Invalid dma bdcMode(%d)"), cfg->detType));
		return IM_RET_INVALID_PARAMETER;
	}	

	//check detect threshold
	if((cfg->hiTh < 0) || (cfg->hiTh > 4095)
		|| (cfg->loTh < 0)|| (cfg->loTh > 255))
	{
		IM_ERRMSG((IM_STR("Invalid detect threshold: hiTh=%d, loTh=%d"), 
					cfg->hiTh, cfg->loTh));
		return IM_RET_INVALID_PARAMETER;
	}

	//check noise level and saltpepper threshold
	if(((cfg->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS) 
		&& (cfg->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_EDGE)
		/*&& (cfg->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_CONTENT)*/)
		|| (cfg->sltPepTh < 0) || (cfg->sltPepTh > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid correct paras: noiselevel=%d, sltPepTh=%d"), 
					cfg->nosLvl, cfg->sltPepTh));
		return IM_RET_INVALID_PARAMETER;
	}

	
	if(cfg->enable == IM_TRUE)
	{
		/*set bdc work mode*/
		if(cfg->bdcMode == ISP_BDC_MODE_CORRECT)
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x2);
		}
		else if(cfg->detType == ISP_BDC_DETECT_TYPE_0)
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x0);
		}
		else
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x1);
		}
		
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_H_DET, cfg->hiTh);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_L_DET, cfg->loTh);

		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_NOISELEVEL, cfg->nosLvl);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_THSALTPEPPER, cfg->sltPepTh);

		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG0], bdc->regVal[rISP_BDC_CFG0]));
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG2], bdc->regVal[rISP_BDC_CFG2]));

		if(cfg->bdcMode == ISP_BDC_MODE_DETECT)
		{
			/*set bdc dma*/
			ret = bdc_set_dma(bdc, &cfg->dma);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("Set bdc dma failed!!")));
				return IM_RET_FAILED;
			}
			
			/*enable bdc dma, first bpdma enable, than ch3dma enable*/
			SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
			SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
			
			/*set bdc enable*/
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 0);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
		}
		else
		{
			/*set denoise*/
			if(cfg->denosEn == IM_TRUE)
			{
				SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 0);
				IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
			}
			else
			{
				SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 1);
				IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
			}
			
			/*set bdc dma*/
			if(cfg->crtEn == IM_TRUE)
			{
				//set bdc dma read
				if(cfg->dma.direct != ISP_BDC_DMA_READ)
				{
					IM_WARNMSG((IM_STR("dma should be reading direction")));
					cfg->dma.direct = ISP_BDC_DMA_WRITE;
				}
				ret = bdc_set_dma(bdc, &cfg->dma);
				if(ret != IM_RET_OK)
				{
					IM_ERRMSG((IM_STR("Set bdc dma failed!!")));
					return IM_RET_FAILED;
				}
			}
			
			/*set bdc enable*/
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 0);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));

			/*enable bdc dma, first bpdma enable, than ch3dma enable*/
			if(cfg->crtEn == IM_TRUE)
			{
				SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 1);
				IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
				SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 1);
				IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
			}
		}
	}
	else
	{
		/*set bdc denoise bypass*/
		SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));

		/*disable bdc dma*/
		SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 0);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 0);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
	
		/*set bdc bypass*/
		SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
	}
	
	bdc->bdcMode = cfg->bdcMode;
	bdc->detType = cfg->detType;
	bdc->crtEn = cfg->crtEn;
	bdc->denosEn = cfg->denosEn;
	bdc->hiTh = cfg->hiTh;
	bdc->loTh = cfg->loTh;
	bdc->nosLvl = cfg->nosLvl;
	bdc->sltPepTh = cfg->sltPepTh;
	bdc->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bdc_set_detect_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_set_detect_mode(isp_bdc_context_t *bdc, bdc_detect_context_t *det)
{
	IM_RET	ret;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);
	IM_ASSERT(det != IM_NULL);
	
	//check detect threshold 
	if((det->hiTh < 0) || (det->hiTh > 4095)
		|| (det->loTh < 0)|| (det->loTh > 255))
	{
		IM_ERRMSG((IM_STR("Invalid detect threshold: hiTh=%d, loTh=%d"), 
					det->hiTh, det->loTh));
		return IM_RET_INVALID_PARAMETER;
	}

	if(bdc->enable == IM_TRUE)
	{
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_H_DET, det->hiTh);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_L_DET, det->loTh);
		/*set bdc work mode*/
		if(det->detType == ISP_BDC_DETECT_TYPE_0)
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x0);
		}
		else
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x1);
		}
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG0], bdc->regVal[rISP_BDC_CFG0]));
	}

	//set bdc dma write
	if(det->dma.direct != ISP_BDC_DMA_WRITE)
	{
		IM_WARNMSG((IM_STR("dma should be writing direction")));
		det->dma.direct = ISP_BDC_DMA_WRITE;
	}
	ret = bdc_set_dma(bdc, &det->dma);
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("Set bdc dma failed!!")));
		return IM_RET_FAILED;
	}

	bdc->hiTh = det->hiTh;
	bdc->loTh = det->loTh;
	bdc->bdcMode = ISP_BDC_MODE_DETECT;
	bdc->detType = det->detType;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bdc_set_correct_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_set_correct_mode(isp_bdc_context_t *bdc, bdc_correct_context_t *crt)
{
	IM_RET ret;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);
	IM_ASSERT(crt != IM_NULL);

	//check noise level and saltpepper threshold
	if(((crt->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS) 
		&& (crt->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_EDGE)
		/*&& (crt->nosLvl != ISP_BDC_CORRECT_NOISE_LEVEL_CONTENT)*/)
		|| (crt->sltPepTh < 0) || (crt->sltPepTh > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid correct paras: noiselevel=%d, sltPepTh=%d"), 
					crt->nosLvl, crt->sltPepTh));
		return IM_RET_INVALID_PARAMETER;
	}

	if(bdc->enable == IM_TRUE)
	{
		if(crt->denosEn == IM_TRUE)
		{
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 0);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
		}
		else
		{
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
		}
		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_NOISELEVEL, crt->nosLvl);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_THSALTPEPPER, crt->sltPepTh);
		
		//set bdc work mode
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x2);
		
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG0], bdc->regVal[rISP_BDC_CFG0]));
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG2], bdc->regVal[rISP_BDC_CFG2]));
	}
	
	if(crt->crtEn == IM_TRUE)
	{
		//set bdc dma read
		if(crt->dma.direct != ISP_BDC_DMA_READ)
		{
			IM_WARNMSG((IM_STR("dma should be reading direction")));
			crt->dma.direct = ISP_BDC_DMA_WRITE;
		}
		ret = bdc_set_dma(bdc, &crt->dma);
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("Set bdc dma failed!!")));
			return IM_RET_FAILED;
		}
	}

	bdc->nosLvl = crt->nosLvl;
	bdc->sltPepTh = crt->sltPepTh;
	bdc->bdcMode = ISP_BDC_MODE_CORRECT;
	bdc->crtEn = crt->crtEn;
	bdc->denosEn = crt->denosEn;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bdc_get_bp_number

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_get_bp_number(isp_bdc_context_t *bdc, IM_UINT32 *bpNum)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);

	IM_JIF(isppwl_read_reg(bdc->regOfst[rISP_BDC_BNUM], &bdc->regVal[rISP_BDC_BNUM]));
	bdc->bpNum = GetIspRegister(bdc->regVal, ISP_BDC_BNUM);
	*bpNum = bdc->bpNum;
	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bdc_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_set_enable(isp_bdc_context_t *bdc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);

	if(bdc->enable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("bdc has been enabled")));
		return IM_RET_OK;	
	}
	//enable bdc 
		
	if(bdc->bdcMode == ISP_BDC_MODE_DETECT)
	{
		/*set detect threshold*/
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_H_DET, bdc->hiTh);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_TH_L_DET, bdc->loTh);

		/*set bdc work mode*/
		if(bdc->detType == ISP_BDC_DETECT_TYPE_0)
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x0);
		}
		else
		{
			SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x1);
		}
		
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG0], bdc->regVal[rISP_BDC_CFG0]));
	
		/*enable bdc dma, first bpdma enable, than ch3dma enable*/
		SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
		SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 1);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
		
		/*set bdc enable*/
		SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 0);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set correct params*/
		if(bdc->denosEn == IM_TRUE)
		{
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 0);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
		}
		else
		{
			SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));
		}
		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_NOISELEVEL, bdc->nosLvl);
		SetIspRegister(bdc->regVal, ISP_BDC_CFG2_THSALTPEPPER, bdc->sltPepTh);
		/*set bdc work mode*/
		SetIspRegister(bdc->regVal, ISP_BDC_CFG0_BDWORKMD, 0x2);
		
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG0], bdc->regVal[rISP_BDC_CFG0]));
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BDC_CFG2], bdc->regVal[rISP_BDC_CFG2]));
	
		/*set bdc enable*/
		SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 0);
		IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));

		if(bdc->crtEn == IM_TRUE)
		{
			/*enable bdc dma, first bpdma enable, than ch3dma enable*/
			SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));
			SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 1);
			IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
		}
	}
	bdc->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bdc_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bdc_set_disable(isp_bdc_context_t *bdc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bdc != IM_NULL);
	IM_ASSERT(bdc->regVal != IM_NULL);
	IM_ASSERT(bdc->regOfst != IM_NULL);

	if(bdc->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("bdc has been disabled")));
		return IM_RET_OK;	
	}
	
	/*disable bdc denoise*/
	SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCDENOSBYPASS, 1);
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));

	/*disable bdc dma*/
	SetIspRegister(bdc->regVal, ISP_GLB_CFG1_CH3DMAEN, 0);
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG1], bdc->regVal[rISP_GLB_CFG1]));
	SetIspRegister(bdc->regVal, ISP_BPDMA_CC1_EN, 0);
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_BPDMA_CC1], bdc->regVal[rISP_BPDMA_CC1]));

	/*disable bdc*/
	SetIspRegister(bdc->regVal, ISP_GLB_CFG2_BDCBYPASS, 1);
	IM_JIF(isppwl_write_reg(bdc->regOfst[rISP_GLB_CFG2], bdc->regVal[rISP_GLB_CFG2]));

	bdc->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

