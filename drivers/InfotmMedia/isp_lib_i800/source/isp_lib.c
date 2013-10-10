/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_lib.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
-- v1.0.2	arsor@2012/07/20: add ee setting detect threshold martix api.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_internal.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"ISP_LIB_I:"
#define WARNHEAD	"ISP_LIB_W:"
#define ERRHEAD		"ISP_LIB_E:"
#define TIPHEAD		"ISP_LIB_T:"

#define ISP_CHECK(exp)	if(!(exp)){IM_ERRMSG((IM_STR("isp is NULL, maybe isp is not initial"))); return IM_RET_FAILED;}

#define Y	0
#define U	1
#define V	2


/*********************************
*    	isp wb mode gain		 *
*   { R_gain, G_gain, B_gain }   *
*********************************/
static const IM_FLOAT ispWbGain[ISP_LAST_WB_MODE + 1][3] = {
#include "isp_wb_gain_table.h"
};

IM_RET setDmaBuffer(IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer);
IM_RET getDmaBuffer(IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer);

static isp_context_t *isp = IM_NULL;


/*------------------------------------------------------------------------------

    Function name: isplib_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_init(isp_config_t *ispCfg)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 i;
	IM_UINT32 framePixNum = 0;
	bccb_config_t bccbCfg;
	bdc_config_t bdcCfg;
	hist_config_t histCfg;
	osd_config_t osdCfg;
	scl_config_t sclCfg;


	IM_UINT32 ispRegOfst[] = {
		#include "ispreg_offset_table.h"
	};

	
  	IM_INFOMSG((IM_STR("%s++"), IM_STR(_IM_FUNC_)));
	
	if(ispCfg == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_FAILED;
	}    
	ret = isppwl_init();    
	if(ret != IM_RET_OK)    
	{
		IM_ERRMSG((IM_STR("isppwl_init failed!")));
		return IM_RET_FAILED;
	}    

	isp = (isp_context_t *)isppwl_malloc(sizeof(isp_context_t));
	if(isp == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("isppwl_malloc failed!")));
		isppwl_deinit();
		return IM_RET_FAILED;    
	}
	/* set everything initially zero */
	isppwl_memset(isp, 0, sizeof(isp_context_t));
	
	/*initial registers offset*/
	for(i=0; i<ISP_REGISTERS; i++)
	{
		isp->regOfst[i] = ispRegOfst[i];
		IM_JIF(isppwl_read_reg(isp->regOfst[i], &(isp->regVal[i])));
	}

	/*ISP SW RESET*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_ISPSRST, 1);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));
	
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_ISPSRST, 0);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));

	/*ISP DMAVSYNC RESET ENABLE*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_DMAVSYNC, 1);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));

	//
	//set input mode and save it to isp context
	//
	
	//input size
	if((ispCfg->inHeight < 0) || (ispCfg->inHeight > 4096)
		||(ispCfg->inWidth < 0) || (ispCfg->inWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid input size: height=%d, width=%d"), ispCfg->inHeight, ispCfg->inWidth));
		goto Fail;
	}
	//interface mode
	if((ispCfg->intfMode != ISP_INTFMODE_IO_RAWRGB)
		&&(ispCfg->intfMode != ISP_INTFMODE_MIPI_RAWRGB)
		&&(ispCfg->intfMode != ISP_INTFMODE_MIPI_YUV)
		&&(ispCfg->intfMode != ISP_INTFMODE_ITU_RGB16BIT)
		&&(ispCfg->intfMode != ISP_INTFMODE_ITU_RGB888)
		&&(ispCfg->intfMode != ISP_INTFMODE_ITU_YUV422)
		&&(ispCfg->intfMode != ISP_INTFMODE_ITU_YUV444))
	{
		IM_ERRMSG((IM_STR("intfMode value is not invalide: intfMode=%d"), ispCfg->intfMode));
		goto Fail;
	}	
	//input data number of bits
	if((ispCfg->inputBitsNum != ISP_INPUT_LOW_BITS_8) 
		&& (ispCfg->inputBitsNum != ISP_INPUT_LOW_BITS_10) 
		&& (ispCfg->inputBitsNum != ISP_INPUT_MID_BITS_8) 
		&& (ispCfg->inputBitsNum != ISP_INPUT_BITS_12))
	{
		IM_ERRMSG((IM_STR("Invalid input raw data bits number: %d"), ispCfg->inputBitsNum));
		goto Fail;
	}
	//rawdata mode
	if((ispCfg->rawMode != ISP_RAWDATAMODE_RGRG) && (ispCfg->rawMode != ISP_RAWDATAMODE_BGBG) 
		&& (ispCfg->rawMode != ISP_RAWDATAMODE_GRGR) && (ispCfg->rawMode != ISP_RAWDATAMODE_GBGB))
	{
		IM_ERRMSG((IM_STR("Invalid input raw data mode: %d"), ispCfg->rawMode));
		goto Fail;
	}
	//itu mode
	if((ispCfg->ituType.scanMode != ISP_ITU_SCAN_INTERLACED) && (ispCfg->ituType.scanMode != ISP_ITU_SCAN_PROGRESSIVE))
	{
		IM_ERRMSG((IM_STR("Invalid input itu scan mode: %d"), ispCfg->ituType.scanMode));
		goto Fail;
	}
	if((ispCfg->ituType.format != ISP_ITU_FORMAT_ITU601) && (ispCfg->ituType.format != ISP_ITU_FORMAT_ITU656))
	{
		IM_ERRMSG((IM_STR("Invalid input itu format: %d"), ispCfg->ituType.format));
		goto Fail;
	}
	if(ispCfg->ituType.order > 5)
	{
		IM_ERRMSG((IM_STR("Invalid input itu order: %d"), ispCfg->ituType.order));
		goto Fail;
	}

	framePixNum = ispCfg->inWidth*ispCfg->inHeight;

	/*set resolution correlative registers*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_VPIXNUM, ispCfg->inWidth);
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_HPIXNUM, ispCfg->inHeight);
	SetIspRegister(isp->regVal, ISP_AWB_MXN, framePixNum);
	SetIspRegister(isp->regVal, ISP_GLB_RH, (1024*1024)/ispCfg->inHeight);
	SetIspRegister(isp->regVal, ISP_GLB_RV, (1024*1024)/ispCfg->inWidth);
	/*set input data type*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INTFMODE, ispCfg->intfMode);
	SetIspRegister(isp->regVal, ISP_REG_CONFIG_INPUTBITS, ispCfg->inputBitsNum);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_RAWMODE, ispCfg->rawMode);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_SCANMODE, ispCfg->ituType.scanMode);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_FORMAT, ispCfg->ituType.format);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_ORDER, ispCfg->ituType.order);

	//set signal polarity invert
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_HSYNC, ispCfg->sigPol.hsync);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_HREF, ispCfg->sigPol.href);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_VSYNC, ispCfg->sigPol.vsync);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_PCLK, ispCfg->sigPol.pclk);

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG0], isp->regVal[rISP_GLB_CFG0]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_AWB_MXN], isp->regVal[rISP_AWB_MXN]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RH], isp->regVal[rISP_GLB_RH]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RV], isp->regVal[rISP_GLB_RV]));

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG3], isp->regVal[rISP_GLB_CFG3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_REG_CONFIG], isp->regVal[rISP_REG_CONFIG]));

	isp->inWidth = ispCfg->inWidth;	
	isp->inHeight = ispCfg->inHeight;
	isp->intfMode = ispCfg->intfMode;
	isp->rawMode = ispCfg->rawMode;
	isp->inputBitsNum = ispCfg->inputBitsNum;
	isp->ituType.scanMode = ispCfg->ituType.scanMode;
	isp->ituType.format = ispCfg->ituType.format;
	isp->ituType.order = ispCfg->ituType.order;
	isp->sigPol.hsync = ispCfg->sigPol.hsync;
	isp->sigPol.href = ispCfg->sigPol.href;
	isp->sigPol.vsync = ispCfg->sigPol.vsync;
	isp->sigPol.pclk = ispCfg->sigPol.pclk;

	//
	//set output mode and save to isp context
	//
	if((ispCfg->outWidth < 0) || (ispCfg->outHeight > 4096)
		||(ispCfg->outHeight < 0) || (ispCfg->outHeight > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid output size: height=%d, width=%d"), ispCfg->outHeight, ispCfg->outHeight));
		goto Fail;
	}
	if((ispCfg->outImgType != IM_IMAGE_RGB8880) && (ispCfg->outImgType != IM_IMAGE_RGB0888)
		&& (ispCfg->outImgType != IM_IMAGE_BGR0888) && (ispCfg->outImgType != IM_IMAGE_BGR8880)
		&& (ispCfg->outImgType != IM_IMAGE_YUV420P) && (ispCfg->outImgType != IM_IMAGE_YUV420SP)
		&& (ispCfg->outImgType != IM_IMAGE_YUV422P) && (ispCfg->outImgType != IM_IMAGE_YUV422SP)
		&& (ispCfg->outImgType != IM_IMAGE_YUV444P))
	{
		IM_ERRMSG((IM_STR("Invalid(not support this)output format:outFormat=%d"),
			ispCfg->outImgType));
		goto Fail;
	}
	isp->outWidth = ispCfg->outWidth;
	isp->outHeight =  ispCfg->outHeight;
	isp->outImgType = ispCfg->outImgType;

	//
	//sub module
	//
	/*init demosaic module*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG2_DEMBYPASS, (ispCfg->demEnable==IM_TRUE)?0:1);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG2], isp->regVal[rISP_GLB_CFG2]));
	isp->demEnable = ispCfg->demEnable;

	
	/*init bccb module*/
	isp->bccb = (isp_bccb_context_t *)isppwl_malloc(sizeof(isp_bccb_context_t));
 
	if(isp->bccb == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));
		goto Fail;
	}

	/* set everything initially zero */
	isppwl_memset(isp->bccb, 0, sizeof(isp_bccb_context_t));
	isp->bccb->regVal = isp->regVal;
	isp->bccb->regOfst = isp->regOfst;

	bccbCfg.bccbMode = ispCfg->bccbCfg.bccbMode;
	bccbCfg.bc.rBlkTh = ispCfg->bccbCfg.blkTh;
	bccbCfg.bc.grBlkTh = ispCfg->bccbCfg.blkTh;
	bccbCfg.bc.bBlkTh = ispCfg->bccbCfg.blkTh;
	bccbCfg.bc.gbBlkTh = ispCfg->bccbCfg.blkTh;
#if 0//add white gain mode at initinal
	bccbCfg.cb.rGain = (IM_UINT32)(ispWbGain[ispCfg->bccbCfg.wbGainMode][0]*ispCfg->bccbCfg.rGain);
	bccbCfg.cb.grGain = (IM_UINT32)(ispWbGain[ispCfg->bccbCfg.wbGainMode][1]*ispCfg->bccbCfg.gGain);
	bccbCfg.cb.gbGain = bccbCfg.cb.grGain;
	bccbCfg.cb.bGain = (IM_UINT32)(ispWbGain[ispCfg->bccbCfg.wbGainMode][2]*ispCfg->bccbCfg.bGain);	
#else
	bccbCfg.cb.rGain = (IM_UINT32)(ispCfg->bccbCfg.rGain);
	bccbCfg.cb.grGain = (IM_UINT32)(ispCfg->bccbCfg.gGain);
	bccbCfg.cb.gbGain = bccbCfg.cb.grGain;
	bccbCfg.cb.bGain = (IM_UINT32)(ispCfg->bccbCfg.bGain);	
#endif
	bccbCfg.bcEnable = ispCfg->bccbCfg.bcEnable;
	bccbCfg.cbEnable = ispCfg->bccbCfg.cbEnable;

	ret = bccb_init(isp->bccb, &bccbCfg);
	if(ret != IM_RET_OK)    
	{
		IM_ERRMSG((IM_STR("bccb_init failed!")));
		goto Fail;
	} 

	/*init bdc module*/
	isp->bdc = (isp_bdc_context_t *)isppwl_malloc(sizeof(isp_bdc_context_t));
 
	if(isp->bdc == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));
		goto Fail;    
	}

	/* set everything initially zero */
	isppwl_memset(isp->bdc, 0, sizeof(isp_bdc_context_t));
	isp->bdc->regVal = isp->regVal;
	isp->bdc->regOfst = isp->regOfst;

	bdcCfg.bdcMode = ispCfg->bdcCfg.bdcMode;
	bdcCfg.detType = ispCfg->bdcCfg.detType;
	bdcCfg.hiTh = ispCfg->bdcCfg.hiTh;
	bdcCfg.loTh = ispCfg->bdcCfg.loTh;
	
	//set bp correct
	bdcCfg.crtEn = (ispCfg->bdcCfg.crtType & ISP_BDC_BPCRT_ENABLE)?(IM_TRUE):(IM_FALSE);

	//set denoise
	if((ispCfg->bdcCfg.crtType & ISP_BDC_GAUSS_ENABLE)
		&&(ispCfg->bdcCfg.crtType & ISP_BDC_SLTPEP_ENABLE))
	{
		bdcCfg.nosLvl = ispCfg->bdcCfg.nosLvl;
		bdcCfg.sltPepTh = ispCfg->bdcCfg.sltPepTh;
		bdcCfg.denosEn = IM_TRUE;
	}
	else if(ispCfg->bdcCfg.crtType & ISP_BDC_GAUSS_ENABLE)
	{
		bdcCfg.nosLvl = ispCfg->bdcCfg.nosLvl;
		bdcCfg.sltPepTh = 4095;
		bdcCfg.denosEn = IM_TRUE;
	}
	else if(ispCfg->bdcCfg.crtType & ISP_BDC_SLTPEP_ENABLE)
	{
		bdcCfg.nosLvl = ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS;
		bdcCfg.sltPepTh = ispCfg->bdcCfg.sltPepTh;
		bdcCfg.denosEn = IM_TRUE;
	}
	else
	{
		bdcCfg.nosLvl = ispCfg->bdcCfg.nosLvl;
		bdcCfg.sltPepTh = ispCfg->bdcCfg.sltPepTh;
		bdcCfg.denosEn = IM_FALSE;
	}
	
	if(ispCfg->bdcCfg.enable == IM_TRUE)
	{
		//set dma
		bdcCfg.dma.length = ispCfg->bdcCfg.dmaBuf.size;
		IM_BUFFER_COPYTO_BUFFER(bdcCfg.dma.buff[0], ispCfg->bdcCfg.dmaBuf);
		/*bdcCfg.dma.buff[0].vir_addr = ispCfg->bdcCfg.dmaBuf.vir_addr;
		bdcCfg.dma.buff[0].phy_addr = ispCfg->bdcCfg.dmaBuf.phy_addr;
		bdcCfg.dma.buff[0].size = ispCfg->bdcCfg.dmaBuf.size;
		bdcCfg.dma.buff[0].flag = ispCfg->bdcCfg.dmaBuf.flag;*/

		//set only one buffer to DMA out
		bdcCfg.dma.ppMode = ISP_PPMODE_DISABLE;

		if(ISP_BDC_MODE_DETECT == bdcCfg.bdcMode)//detect mode
		{
			bdcCfg.dma.direct = ISP_BDC_DMA_WRITE;
			IM_BUFFER_COPYTO_BUFFER(isp->bdc->dmaOutbuf, bdcCfg.dma.buff[0]);
		}
		else//correct mode
		{
			bdcCfg.dma.direct = ISP_BDC_DMA_READ;
			IM_BUFFER_COPYTO_BUFFER(isp->bdc->dmaInbuf, bdcCfg.dma.buff[0]);
		}
	}
	
	bdcCfg.enable = ispCfg->bdcCfg.enable;

	ret = bdc_init(isp->bdc, &bdcCfg);
	if(ret != IM_RET_OK)    
	{
		IM_ERRMSG((IM_STR("bdc_init failed!")));
		goto Fail;
	}

	/*init lens module*/                                         
	isp->lens = (isp_lens_context_t *)isppwl_malloc(sizeof(isp_lens_context_t));
	                                                            
	if(isp->lens == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->lens, 0, sizeof(isp_lens_context_t));         
	isp->lens->regVal = isp->regVal;                             
	isp->lens->regOfst = isp->regOfst;                           
	isp->lens->height = isp->inHeight;
	isp->lens->width = isp->inWidth;	
	
	ret = lens_init(isp->lens, &ispCfg->lensCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("lens_init failed!")));
		goto Fail;                                              
	}

	/*init gma module*/                                         
	isp->gma = (isp_gma_context_t *)isppwl_malloc(sizeof(isp_gma_context_t));
	                                                            
	if(isp->gma == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->gma, 0, sizeof(isp_gma_context_t));         
	isp->gma->regVal = isp->regVal;                             
	isp->gma->regOfst = isp->regOfst;
		
	ret = gma_init(isp->gma, &ispCfg->gmaCfg);
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("gma_init failed!")));
		goto Fail;
	}

	/*init ee module*/                                         
	isp->ee = (isp_ee_context_t *)isppwl_malloc(sizeof(isp_ee_context_t));
	                                                            
	if(isp->ee == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->ee, 0, sizeof(isp_ee_context_t));         
	isp->ee->regVal = isp->regVal;                             
	isp->ee->regOfst = isp->regOfst;
	
	ret = ee_init(isp->ee, &ispCfg->eeCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("ee_init failed!")));                  
		goto Fail;                                              
	}


	/*init fcc module*/                                         
	isp->fcc = (isp_fcc_context_t *)isppwl_malloc(sizeof(isp_fcc_context_t));
	                                                            
	if(isp->fcc == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->fcc, 0, sizeof(isp_fcc_context_t));
	isp->fcc->regVal = isp->regVal;                             
	isp->fcc->regOfst = isp->regOfst;

	ret = fcc_init(isp->fcc, &ispCfg->fccCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("fcc_init failed!")));                  
		goto Fail;                                              
	}

	/*init af module*/                                         
	isp->af = (isp_af_context_t *)isppwl_malloc(sizeof(isp_af_context_t));
	                                                            
	if(isp->af == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->af, 0, sizeof(isp_af_context_t));         
	isp->af->regVal = isp->regVal;                             
	isp->af->regOfst = isp->regOfst;                           
	
	ret = af_init(isp->af, &ispCfg->afCfg);
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("af_init failed!")));
		goto Fail;                                              
	}

	/*init awb module*/
	isp->awb = (isp_awb_context_t *)isppwl_malloc(sizeof(isp_awb_context_t));
	                                                            
	if(isp->awb == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->awb, 0, sizeof(isp_awb_context_t));         
	isp->awb->regVal = isp->regVal;                             
	isp->awb->regOfst = isp->regOfst;
	isp->awb->framePixNum = ispCfg->inWidth*ispCfg->inHeight;

	ret = awb_init(isp->awb, &ispCfg->awbCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("awb_init failed!")));                  
		goto Fail;                                              
	}

	/*init cmncsc module*/                                         
	isp->cmncsc = (isp_cmncsc_context_t *)isppwl_malloc(sizeof(isp_cmncsc_context_t));
	                                                            
	if(isp->cmncsc == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->cmncsc, 0, sizeof(isp_cmncsc_context_t));         
	isp->cmncsc->regVal = isp->regVal;                             
	isp->cmncsc->regOfst = isp->regOfst;
	
	ret = cmncsc_init(isp->cmncsc, &ispCfg->cmncscCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("cmncsc_init failed!")));
		goto Fail;                                              
	}
	
	/*init ae module*/                                         
	isp->ae = (isp_ae_context_t *)isppwl_malloc(sizeof(isp_ae_context_t));
	                                                            
	if(isp->ae == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}
		                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->ae, 0, sizeof(isp_ae_context_t));         
	isp->ae->regVal = isp->regVal;                             
	isp->ae->regOfst = isp->regOfst;
		
	ret = ae_init(isp->ae, &ispCfg->aeCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("ae_init failed!")));
		goto Fail;                                              
	}

	/*init hist module*/                                         
	isp->hist = (isp_hist_context_t *)isppwl_malloc(sizeof(isp_hist_context_t));
	                                                            
	if(isp->hist == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->hist, 0, sizeof(isp_hist_context_t));         
	isp->hist->regVal = isp->regVal;                             
	isp->hist->regOfst = isp->regOfst;

	histCfg.enable = ispCfg->histCfg.enable;
	//(width*height)*20/32
	histCfg.blitTh1 = (ispCfg->inHeight*ispCfg->inWidth*ispCfg->histCfg.blitTh1Num)/ispCfg->histCfg.blitTh1Den;
	
	histCfg.thrMat.th1 = ispCfg->histCfg.thrMat.th1;
	histCfg.thrMat.th2 = ispCfg->histCfg.thrMat.th2;
	ret = hist_init(isp->hist, &histCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("hist_init failed!")));
		goto Fail;                                              
	}

	/*init acc module*/                                         
	isp->acc = (isp_acc_context_t *)isppwl_malloc(sizeof(isp_acc_context_t));
	                                                            
	if(isp->acc == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->acc, 0, sizeof(isp_acc_context_t));         
	isp->acc->regVal = isp->regVal;                             
	isp->acc->regOfst = isp->regOfst;
		
	ret = acc_init(isp->acc, &ispCfg->accCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("acc_init failed!")));
		goto Fail;                                              
	}

	/*init ief module*/                                         
	isp->ief = (isp_ief_context_t *)isppwl_malloc(sizeof(isp_ief_context_t));
	                                                            
	if(isp->ief == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->ief, 0, sizeof(isp_ief_context_t));         
	isp->ief->regVal = isp->regVal;                             
	isp->ief->regOfst = isp->regOfst;

	ret = ief_init(isp->ief, &ispCfg->iefCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("ief_init failed!")));
		goto Fail;                                              
	}

	/*init acm module*/                                         
	isp->acm = (isp_acm_context_t *)isppwl_malloc(sizeof(isp_acm_context_t));
	                                                            
	if(isp->acm == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->acm, 0, sizeof(isp_acm_context_t));         
	isp->acm->regVal = isp->regVal;                             
	isp->acm->regOfst = isp->regOfst;

	ret = acm_init(isp->acm, &ispCfg->acmCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("acm_init failed!")));
		goto Fail;                                              
	}

	/*init osd module*/                                         
	isp->osd = (isp_osd_context_t *)isppwl_malloc(sizeof(isp_osd_context_t));
	                                                            
	if(isp->osd == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->osd, 0, sizeof(isp_osd_context_t));         
	isp->osd->regVal = isp->regVal;                             
	isp->osd->regOfst = isp->regOfst;   

	osdCfg.enable= ispCfg->osdCfg.enable;
	osdCfg.bgColor = ispCfg->osdCfg.bgColor;
	osdCfg.outWidth = ispCfg->osdCfg.outWidth;
	osdCfg.outHeight = ispCfg->osdCfg.outHeight;

	//wnd0
	osdCfg.wnd0.enable = ispCfg->osdCfg.wnd0.enable;
	isppwl_memcpy(&(osdCfg.wnd0.mapclr), &(ispCfg->osdCfg.wnd0.mapclr), sizeof(isp_osd_mapcolor_t));
	isppwl_memcpy(&(osdCfg.wnd0.coordinate), &(ispCfg->osdCfg.wnd0.coordinate), sizeof(isp_osd_coordinate_t));

	//wnd1
	osdCfg.wnd1.enable = ispCfg->osdCfg.wnd1.enable;
	osdCfg.wnd1.imgFormat = ispCfg->osdCfg.wnd1.imgFormat;
	isppwl_memcpy(&(osdCfg.wnd1.palette), &(ispCfg->osdCfg.wnd1.palette), sizeof(isp_osd_palette_t));
	isppwl_memcpy(&(osdCfg.wnd1.swap), &(ispCfg->osdCfg.wnd1.swap), sizeof(isp_osd_swap_t));
	isppwl_memcpy(&(osdCfg.wnd1.alpha), &(ispCfg->osdCfg.wnd1.alpha), sizeof(isp_osd_alpha_t));
	isppwl_memcpy(&(osdCfg.wnd1.mapclr), &(ispCfg->osdCfg.wnd1.mapclr), sizeof(isp_osd_mapcolor_t));
	isppwl_memcpy(&(osdCfg.wnd1.clrkey), &(ispCfg->osdCfg.wnd1.clrkey), sizeof(isp_osd_colorkey_t));
	isppwl_memcpy(&(osdCfg.wnd1.vm), &(ispCfg->osdCfg.wnd1.vm), sizeof(isp_osd_vm_t));
	isppwl_memcpy(&(osdCfg.wnd1.coordinate), &(ispCfg->osdCfg.wnd1.coordinate), sizeof(isp_osd_coordinate_t));

	osdCfg.wnd1.bm.mode = ISP_OSD_BUFFER_SEL_MANUAL;
	osdCfg.wnd1.bm.number = 0;
	osdCfg.wnd1.bm.selType = ISP_OSD_BUFSEL_BUF0;

	osdCfg.wnd1.buffer.mask[0] = IM_FALSE;
	IM_BUFFER_COPYTO_BUFFER(osdCfg.wnd1.buffer.buff[0], ispCfg->osdCfg.wnd1.buf);
	/*osdCfg.wnd1.buffer.buff[0].vir_addr = ispCfg->osdCfg.wnd1.buf.vir_addr;	
	osdCfg.wnd1.buffer.buff[0].phy_addr = ispCfg->osdCfg.wnd1.buf.phy_addr;
	osdCfg.wnd1.buffer.buff[0].size = ispCfg->osdCfg.wnd1.buf.size;
	osdCfg.wnd1.buffer.buff[0].flag = ispCfg->osdCfg.wnd1.buf.flag;*/
	osdCfg.wnd1.buffer.mask[1] = IM_TRUE;
	osdCfg.wnd1.buffer.mask[2] = IM_TRUE;
	osdCfg.wnd1.buffer.mask[3] = IM_TRUE;
	
	//osdCfg.wnd1.imageSize = ;
	//osdCfg.wnd1.buffOffset = ;
	//osdCfg.wnd1.bitBuffOffset = ;

	ret = osd_init(isp->osd, &osdCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("osd_init failed!")));
		goto Fail;                                              
	}

	/*init crop module*/                                         
	isp->crop = (isp_crop_context_t *)isppwl_malloc(sizeof(isp_crop_context_t));
	                                                            
	if(isp->crop == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->crop, 0, sizeof(isp_crop_context_t));         
	isp->crop->regVal = isp->regVal;                             
	isp->crop->regOfst = isp->regOfst;


	ret = crop_init(isp->crop, &ispCfg->cropCfg);                                   
	if(ret != IM_RET_OK)                                        
	{                                                           
		IM_ERRMSG((IM_STR("crop_init failed!")));                  
		goto Fail;                                              
	}


	/*init scl module*/                                         
	isp->scl = (isp_scl_context_t *)isppwl_malloc(sizeof(isp_scl_context_t));
	                                                            
	if(isp->scl == IM_NULL)                                     
	{                                                           
		IM_ERRMSG((IM_STR("isppwl_malloc failed")));                 
		goto Fail;                                              
	}                                                           
	                                                            
	/* set everything initially zero */                         
	isppwl_memset(isp->scl, 0, sizeof(isp_scl_context_t));         
	isp->scl->regVal = isp->regVal;                             
	isp->scl->regOfst = isp->regOfst;

	sclCfg.vrdMode = ispCfg->sclCfg.vrdMode;
	sclCfg.hrdMode = ispCfg->sclCfg.hrdMode;

#if 1
	sclCfg.inPut.inWidth = ispCfg->sclCfg.sclInWidth;
	sclCfg.inPut.inHeight = ispCfg->sclCfg.sclInHeight;

	sclCfg.outPut.outWidth = ispCfg->sclCfg.sclOutWidth;
	sclCfg.outPut.outHeight = ispCfg->sclCfg.sclOutHeight;
	sclCfg.outPut.outFormat = ispCfg->outImgType;
#else
	sclCfg.inPut.inWidth = ispCfg->inWidth;
	sclCfg.inPut.inHeight = ispCfg->inHeight;

	sclCfg.outPut.outWidth = ispCfg->outWidth;
	sclCfg.outPut.outHeight = ispCfg->outHeight;
	sclCfg.outPut.outFormat = ispCfg->outImgType;
#endif
	sclCfg.verEnable = ispCfg->sclCfg.verEnable;
	sclCfg.horEnable = ispCfg->sclCfg.horEnable;
	
	ret = scl_init(isp->scl, &sclCfg);                                   
	if(ret != IM_RET_OK)
	{                                                           
		IM_ERRMSG((IM_STR("scl_init failed!")));
		goto Fail;                                              
	}


  	IM_INFOMSG((IM_STR("%s--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;;
	
Fail:

	isplib_deinit();
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: isplib_deinit

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_deinit(void)
{
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ISP_CHECK(isp != IM_NULL);

	if(isp->bccb != IM_NULL)
	{
		isppwl_free(isp->bccb);
	}
	if(isp->bdc != IM_NULL)
	{
		isppwl_free(isp->bdc);
	}
	if(isp->lens != IM_NULL) 
	{                              
		isppwl_free(isp->lens);
	}
	if(isp->gma != IM_NULL)
	{                              
		isppwl_free(isp->gma);
	}
	if(isp->ee != IM_NULL)
	{                              
		isppwl_free(isp->ee);
	}  
	if(isp->fcc != IM_NULL)
	{                              
		isppwl_free(isp->fcc);
	}
	if(isp->af != IM_NULL)
	{                              
		isppwl_free(isp->af);
	}
	if(isp->awb != IM_NULL)
	{                              
		isppwl_free(isp->awb);
	}
	if(isp->cmncsc != IM_NULL)
	{                              
		isppwl_free(isp->cmncsc);
	} 
	if(isp->ae != IM_NULL)
	{                              
		isppwl_free(isp->ae);
	}
	if(isp->hist != IM_NULL)
	{                              
		isppwl_free(isp->hist);
	}
	if(isp->acc != IM_NULL)
	{                              
		isppwl_free(isp->acc);
	}
	if(isp->ief != IM_NULL)
	{                              
		isppwl_free(isp->ief);
	}
	if(isp->acm != IM_NULL)
	{                              
		isppwl_free(isp->acm);
	}
	if(isp->osd != IM_NULL)
	{
		osd_deinit(isp->osd);
		isppwl_free(isp->osd);
	}
	if(isp->crop != IM_NULL)
	{                              
		isppwl_free(isp->crop);
	}
	if(isp->scl != IM_NULL) 
	{                              
		isppwl_free(isp->scl);
	}
	isppwl_free(isp);
	isp = IM_NULL;
	isppwl_deinit();	

	return ret;	
}

/*------------------------------------------------------------------------------

    Function name: isplib_set_buffers

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_set_buffers(isp_dma_config_t *dmaCfg)
{
	IM_UINT32 i;
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));	
	ISP_CHECK(isp != IM_NULL);
	
	if(dmaCfg == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	IM_INFOMSG((IM_STR("dmaCfg->ppMode=%d"), dmaCfg->ppMode));
	if(dmaCfg->ppMode > ISP_PPMODE_4_BUFFER){
		IM_ERRMSG((IM_STR("Invalid ppMode(%d)"), dmaCfg->ppMode));
		return IM_RET_INVALID_PARAMETER;
	}
	isp->dma.ppMode = dmaCfg->ppMode;
	for(i=0; i<((dmaCfg->ppMode==ISP_PPMODE_DISABLE)?1:dmaCfg->ppMode); i++){
		IM_INFOMSG((IM_STR("dmaCfg->buffY[%d] vir=0x%x, phy=0x%x, size=%d"), 
					i, (IM_UINT32)dmaCfg->buffY[i].vir_addr, dmaCfg->buffY[i].phy_addr, dmaCfg->buffY[i].size));
		IM_INFOMSG((IM_STR("dmaCfg->buffCb[%d] vir=0x%x, phy=0x%x, size=%d"), 
					i, (IM_UINT32)dmaCfg->buffCb[i].vir_addr, dmaCfg->buffCb[i].phy_addr, dmaCfg->buffCb[i].size));
		IM_INFOMSG((IM_STR("dmaCfg->buffCr[%d] vir=0x%x, phy=0x%x, size=%d"), 
					i, (IM_UINT32)dmaCfg->buffCr[i].vir_addr, dmaCfg->buffCr[i].phy_addr, dmaCfg->buffCr[i].size));
		isp->dma.buffY[i] = dmaCfg->buffY[i];
		isp->dma.buffCb[i] = dmaCfg->buffCb[i];
		isp->dma.buffCr[i] = dmaCfg->buffCr[i];
	}
	if(isp->dma.ppMode == ISP_PPMODE_1_BUFFER){
		for(i=1; i<4; i++){
			isp->dma.buffY[i].vir_addr = isp->dma.buffY[0].vir_addr;
			isp->dma.buffY[i].phy_addr = isp->dma.buffY[0].phy_addr;
			isp->dma.buffY[i].size = isp->dma.buffY[0].size;
			isp->dma.buffY[i].flag = isp->dma.buffY[0].flag;

			isp->dma.buffCb[i].vir_addr = isp->dma.buffCb[0].vir_addr;
			isp->dma.buffCb[i].phy_addr = isp->dma.buffCb[0].phy_addr;
			isp->dma.buffCb[i].size = isp->dma.buffCb[0].size;
			isp->dma.buffCb[i].flag = isp->dma.buffCb[0].flag;

			isp->dma.buffCr[i].vir_addr = isp->dma.buffCr[0].vir_addr;
			isp->dma.buffCr[i].phy_addr = isp->dma.buffCr[0].phy_addr;
			isp->dma.buffCr[i].size = isp->dma.buffCr[0].size;
			isp->dma.buffCr[i].flag = isp->dma.buffCr[0].flag;
		}
	}else if(isp->dma.ppMode == ISP_PPMODE_2_BUFFER){
		isp->dma.buffY[2].vir_addr = isp->dma.buffY[0].vir_addr;
		isp->dma.buffY[2].phy_addr = isp->dma.buffY[0].phy_addr;
		isp->dma.buffY[2].size = isp->dma.buffY[0].size;
		isp->dma.buffY[2].flag = isp->dma.buffY[0].flag;
		isp->dma.buffCb[2].vir_addr = isp->dma.buffCb[0].vir_addr;
		isp->dma.buffCb[2].phy_addr = isp->dma.buffCb[0].phy_addr;
		isp->dma.buffCb[2].size = isp->dma.buffCb[0].size;
		isp->dma.buffCb[2].flag = isp->dma.buffCb[0].flag;
		isp->dma.buffCr[2].vir_addr = isp->dma.buffCr[0].vir_addr;
		isp->dma.buffCr[2].phy_addr = isp->dma.buffCr[0].phy_addr;
		isp->dma.buffCr[2].size = isp->dma.buffCr[0].size;
		isp->dma.buffCr[2].flag = isp->dma.buffCr[0].flag;

		isp->dma.buffY[3].vir_addr = isp->dma.buffY[1].vir_addr;
		isp->dma.buffY[3].phy_addr = isp->dma.buffY[1].phy_addr;
		isp->dma.buffY[3].size = isp->dma.buffY[1].size;
		isp->dma.buffY[3].flag = isp->dma.buffY[1].flag;
		isp->dma.buffCb[3].vir_addr = isp->dma.buffCb[1].vir_addr;
		isp->dma.buffCb[3].phy_addr = isp->dma.buffCb[1].phy_addr;
		isp->dma.buffCb[3].size = isp->dma.buffCb[1].size;
		isp->dma.buffCb[3].flag = isp->dma.buffCb[1].flag;
		isp->dma.buffCr[3].vir_addr = isp->dma.buffCr[1].vir_addr;
		isp->dma.buffCr[3].phy_addr = isp->dma.buffCr[1].phy_addr;
		isp->dma.buffCr[3].size = isp->dma.buffCr[1].size;
		isp->dma.buffCr[3].flag = isp->dma.buffCr[1].flag;
	}	

	// check.
	if((isp->outImgType	!= IM_IMAGE_RGB8880) 	&&
		(isp->outImgType != IM_IMAGE_RGB0888)  	&&
		(isp->outImgType != IM_IMAGE_BGR0888)  	&&
		(isp->outImgType != IM_IMAGE_BGR8880)	&&
		(isp->outImgType != IM_IMAGE_YUV420P)	&&
		(isp->outImgType != IM_IMAGE_YUV420SP)	&&
		(isp->outImgType != IM_IMAGE_YUV422P)	&&
		(isp->outImgType != IM_IMAGE_YUV422SP)	&&
		(isp->outImgType != IM_IMAGE_YUV444P))
	{
		IM_ERRMSG((IM_STR("Invalid output image type(%d)"), isp->outImgType));
		return IM_RET_INVALID_PARAMETER;
	}

	switch(isp->outImgType)
	{
		case IM_IMAGE_RGB8880:
		case IM_IMAGE_RGB0888:
		case IM_IMAGE_BGR0888:
		case IM_IMAGE_BGR8880:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight) << 2;
			isp->dma.lengthCb = 0;
			isp->dma.lengthCr = 0;
			break;
		case IM_IMAGE_YUV420P:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight);
			isp->dma.lengthCb = isp->dma.lengthY >> 2;
			isp->dma.lengthCr = isp->dma.lengthCb;
			break;
		case IM_IMAGE_YUV420SP:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight);
			isp->dma.lengthCb = isp->dma.lengthY >> 1;
			isp->dma.lengthCr = 0;
			break;
		case IM_IMAGE_YUV422P:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight);
			isp->dma.lengthCb = isp->dma.lengthY >> 1;
			isp->dma.lengthCr = isp->dma.lengthCb;
			break;
		case IM_IMAGE_YUV422SP:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight);
			isp->dma.lengthCb = isp->dma.lengthY;
			isp->dma.lengthCr = 0;
			break;
		case IM_IMAGE_YUV444P:
			isp->dma.lengthY = (isp->outWidth*isp->outHeight);
			isp->dma.lengthCb = isp->dma.lengthY;
			isp->dma.lengthCr = isp->dma.lengthCb;
			break;
		default:
			break;
	}

	// Address must 64-bits(8bytes) aligment
	for(i=0; i<((dmaCfg->ppMode==ISP_PPMODE_DISABLE)?1:dmaCfg->ppMode); i++){
		if(isp->dma.lengthY != 0){
			if((dmaCfg->buffY[i].phy_addr & 0x7) || 
				(dmaCfg->buffY[i].size < isp->dma.lengthY)){
				IM_ERRMSG((IM_STR("Invalid buffY, vir=0x%x, phy=0x%x, size=%d, lengthY=%d"), 
					(IM_UINT32)dmaCfg->buffY[i].vir_addr, dmaCfg->buffY[i].phy_addr, dmaCfg->buffY[i].size, 
					isp->dma.lengthY));
				return IM_RET_INVALID_PARAMETER;
			}
		}

		if(isp->dma.lengthCb != 0){
			if((dmaCfg->buffCb[i].phy_addr & 0x7) || 
				(dmaCfg->buffCb[i].size < isp->dma.lengthCb)){
				IM_ERRMSG((IM_STR("Invalid buffCb, vir=0x%x, phy=0x%x, size=%d, lengthCb=%d"), 
					(IM_UINT32)dmaCfg->buffCb[i].vir_addr, dmaCfg->buffCb[i].phy_addr, dmaCfg->buffCb[i].size, 
					isp->dma.lengthCb));
				return IM_RET_INVALID_PARAMETER;
			}
		}

		if(isp->dma.lengthCr != 0){
			if((dmaCfg->buffCr[i].phy_addr & 0x7) || 
				(dmaCfg->buffCr[i].size < isp->dma.lengthCr)){
				IM_ERRMSG((IM_STR("Invalid buffCr, vir=0x%x, phy=0x%x, size=%d, lengthCr=%d"), 
					(IM_UINT32)dmaCfg->buffCr[i].vir_addr, dmaCfg->buffCr[i].phy_addr, dmaCfg->buffCr[i].size, 
					isp->dma.lengthCr));
				return IM_RET_INVALID_PARAMETER;
			}
		}
	}

	if(isp->dma.lengthY != 0){
		//
		/*buf0*/
		//whether transfer next frame? 
		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_NTFRMEN1, 1);
		}else{
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_NTFRMEN1, 0);
		}
		//auto reload set
		SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_AUTORELOAD1, 1);
		//dmaCfg dir(write to memory)
		SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_FRMLEN1, isp->dma.lengthY >> 3);
		//address of buf0
		SetIspRegister(isp->regVal, ISP_CH0DMA_FBA1, isp->dma.buffY[0].phy_addr);

		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			/*buf1*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC2_NTFRMEN2, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC2_AUTORELOAD2, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC2_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC2_FRMLEN2, isp->dma.lengthY >> 3);
			//address of buf1
			SetIspRegister(isp->regVal, ISP_CH0DMA_FBA2, isp->dma.buffY[1].phy_addr);

			/*buf2*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC3_NTFRMEN3, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC3_AUTORELOAD3, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC3_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC3_FRMLEN3, isp->dma.lengthY >> 3);
			//address of buf2
			SetIspRegister(isp->regVal, ISP_CH0DMA_FBA3, isp->dma.buffY[2].phy_addr);

			/*buf3*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC4_NTFRMEN4, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC4_AUTORELOAD4, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC4_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH0DMA_CC4_FRMLEN4, isp->dma.lengthY >> 3);
			//address of buf3
			SetIspRegister(isp->regVal, ISP_CH0DMA_FBA4, isp->dma.buffY[3].phy_addr);
		}
	}	
	if(isp->dma.lengthCb != 0){
		//
		/*buf0*/
		//does transfer next frame? 
		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_NTFRMEN1, 1);
		}else{
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_NTFRMEN1, 0);
		}
		//auto reload set
		SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_AUTORELOAD1, 1);
		//dmaCfg dir(write to memory)
		SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_FRMLEN1, isp->dma.lengthCb>> 3);
		//address of buf0
		SetIspRegister(isp->regVal, ISP_CH1DMA_FBA1, isp->dma.buffCb[0].phy_addr);

		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			/*buf1*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC2_NTFRMEN2, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC2_AUTORELOAD2, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC2_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC2_FRMLEN2, isp->dma.lengthCb>> 3);
			//address of buf1
			SetIspRegister(isp->regVal, ISP_CH1DMA_FBA2, isp->dma.buffCb[1].phy_addr);

			/*buf2*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC3_NTFRMEN3, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC3_AUTORELOAD3, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC3_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC3_FRMLEN3, isp->dma.lengthCb>> 3);
			//address of buf2
			SetIspRegister(isp->regVal, ISP_CH1DMA_FBA2, isp->dma.buffCb[2].phy_addr);

			/*buf3*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC4_NTFRMEN4, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC4_AUTORELOAD4, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC4_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH1DMA_CC4_FRMLEN4, isp->dma.lengthCb>> 3);
			//address of buf3
			SetIspRegister(isp->regVal, ISP_CH1DMA_FBA4, isp->dma.buffCb[3].phy_addr);
		}
	}
	if(isp->dma.lengthCr != 0){
		//
		/*buf0*/
		//does transfer next frame? 
		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_NTFRMEN1, 1);
		}else{
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_NTFRMEN1, 0);
		}
		//auto reload set
		SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_AUTORELOAD1, 1);
		//dmaCfg dir(write to memory)
		SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_FRMREAD, 1);
		//Double Word(64bits,8bytes) length of data
		SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_FRMLEN1, isp->dma.lengthCr>> 3);
		//address of buf0
		SetIspRegister(isp->regVal, ISP_CH2DMA_FBA1, isp->dma.buffCr[0].phy_addr);

		if(isp->dma.ppMode != ISP_PPMODE_DISABLE)
		{
			/*buf1*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC2_NTFRMEN2, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC2_AUTORELOAD2, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC2_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC2_FRMLEN2, isp->dma.lengthCr>> 3);
			//address of buf1
			SetIspRegister(isp->regVal, ISP_CH2DMA_FBA2, isp->dma.buffCr[1].phy_addr);

			/*buf2*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC3_NTFRMEN3, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC3_AUTORELOAD3, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC3_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC3_FRMLEN3, isp->dma.lengthCr>> 3);
			//address of buf2
			SetIspRegister(isp->regVal, ISP_CH2DMA_FBA2, isp->dma.buffCr[2].phy_addr);

			/*buf3*/
			//does transfer next frame? 
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC4_NTFRMEN4, 1);

			//auto reload set
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC4_AUTORELOAD4, 1);
			//dmaCfg dir(write to memory)
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC4_FRMREAD, 1);
			//Double Word(64bits,8bytes) length of data
			SetIspRegister(isp->regVal, ISP_CH2DMA_CC4_FRMLEN4, isp->dma.lengthCr>> 3);
			//address of buf3
			SetIspRegister(isp->regVal, ISP_CH2DMA_FBA4, isp->dma.buffCr[3].phy_addr);
		}
	}

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC1], isp->regVal[rISP_CH0DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_FBA1], isp->regVal[rISP_CH0DMA_FBA1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC2], isp->regVal[rISP_CH0DMA_CC2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_FBA2], isp->regVal[rISP_CH0DMA_FBA2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC3], isp->regVal[rISP_CH0DMA_CC3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_FBA3], isp->regVal[rISP_CH0DMA_FBA3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC4], isp->regVal[rISP_CH0DMA_CC4]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_FBA4], isp->regVal[rISP_CH0DMA_FBA4]));

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC1], isp->regVal[rISP_CH1DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_FBA1], isp->regVal[rISP_CH1DMA_FBA1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC2], isp->regVal[rISP_CH1DMA_CC2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_FBA2], isp->regVal[rISP_CH1DMA_FBA2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC3], isp->regVal[rISP_CH1DMA_CC3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_FBA3], isp->regVal[rISP_CH1DMA_FBA3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC4], isp->regVal[rISP_CH1DMA_CC4]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_FBA4], isp->regVal[rISP_CH1DMA_FBA4]));

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC1], isp->regVal[rISP_CH2DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_FBA1], isp->regVal[rISP_CH2DMA_FBA1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC2], isp->regVal[rISP_CH2DMA_CC2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_FBA2], isp->regVal[rISP_CH2DMA_FBA2]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC3], isp->regVal[rISP_CH2DMA_CC3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_FBA3], isp->regVal[rISP_CH2DMA_FBA3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC4], isp->regVal[rISP_CH2DMA_CC4]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_FBA4], isp->regVal[rISP_CH2DMA_FBA4]));

	return ret;

Fail:
	return IM_RET_FAILED;
}

IM_RET isplib_set_input_mode(isp_input_mode_t *inputMode)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 framePixNum = 0;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	ISP_CHECK(isp != IM_NULL);

	if(inputMode == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("Invalid parameter, inputMode is NULL pointer")));
		return IM_RET_FAILED;
	}    

	//check input size
	if((inputMode->inHeight < 0) || (inputMode->inHeight > 4096)
		||(inputMode->inWidth < 0) || (inputMode->inWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid input size: height=%d, width=%d"), inputMode->inHeight, inputMode->inWidth));
		goto Fail;
	}
	//check interface mode
	if((inputMode->intfMode != ISP_INTFMODE_IO_RAWRGB)
		&&(inputMode->intfMode != ISP_INTFMODE_MIPI_RAWRGB)
		&&(inputMode->intfMode != ISP_INTFMODE_MIPI_YUV)
		&&(inputMode->intfMode != ISP_INTFMODE_ITU_RGB16BIT)
		&&(inputMode->intfMode != ISP_INTFMODE_ITU_RGB888)
		&&(inputMode->intfMode != ISP_INTFMODE_ITU_YUV422)
		&&(inputMode->intfMode != ISP_INTFMODE_ITU_YUV444))
	{
		IM_ERRMSG((IM_STR("intfMode value is not invalide: intfMode=%d"), inputMode->intfMode));
		goto Fail;
	}	
	//check input data number of bits
	if((inputMode->inputBitsNum != ISP_INPUT_LOW_BITS_8) 
		&& (inputMode->inputBitsNum != ISP_INPUT_LOW_BITS_10) 
		&& (inputMode->inputBitsNum != ISP_INPUT_MID_BITS_8) 
		&& (inputMode->inputBitsNum != ISP_INPUT_BITS_12))
	{
		IM_ERRMSG((IM_STR("Invalid input raw data bits number: %d"), inputMode->inputBitsNum));
		goto Fail;
	}
	//check rawdata mode
	if((inputMode->rawMode != ISP_RAWDATAMODE_RGRG) && (inputMode->rawMode != ISP_RAWDATAMODE_BGBG) 
		&& (inputMode->rawMode != ISP_RAWDATAMODE_GRGR) && (inputMode->rawMode != ISP_RAWDATAMODE_GBGB))
	{
		IM_ERRMSG((IM_STR("Invalid input raw data mode: %d"), inputMode->rawMode));
		goto Fail;
	}
	//check itu mode
	if((inputMode->ituType.scanMode != ISP_ITU_SCAN_INTERLACED) && (inputMode->ituType.scanMode != ISP_ITU_SCAN_PROGRESSIVE))
	{
		IM_ERRMSG((IM_STR("Invalid input itu scan mode: %d"), inputMode->ituType.scanMode));
		goto Fail;
	}
	if((inputMode->ituType.format != ISP_ITU_FORMAT_ITU601) && (inputMode->ituType.format != ISP_ITU_FORMAT_ITU656))
	{
		IM_ERRMSG((IM_STR("Invalid input itu format: %d"), inputMode->ituType.format));
		goto Fail;
	}
	if(inputMode->ituType.order > 5)
	{
		IM_ERRMSG((IM_STR("Invalid input itu order: %d"), inputMode->ituType.order));
		goto Fail;
	}

	framePixNum = inputMode->inWidth*inputMode->inHeight;

	/*set resolution correlative registers*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_VPIXNUM, inputMode->inWidth);
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_HPIXNUM, inputMode->inHeight);
	SetIspRegister(isp->regVal, ISP_AWB_MXN, framePixNum);
	SetIspRegister(isp->regVal, ISP_GLB_RH, (1024*1024)/inputMode->inHeight);
	SetIspRegister(isp->regVal, ISP_GLB_RV, (1024*1024)/inputMode->inWidth);
	/*set input data type*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INTFMODE, inputMode->intfMode);
	SetIspRegister(isp->regVal, ISP_REG_CONFIG_INPUTBITS, inputMode->inputBitsNum);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_RAWMODE, inputMode->rawMode);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_SCANMODE, inputMode->ituType.scanMode);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_FORMAT, inputMode->ituType.format);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_ITU_ORDER, inputMode->ituType.order);

	//set signal polarity invert
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_HSYNC, inputMode->sigPol.hsync);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_HREF, inputMode->sigPol.href);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_VSYNC, inputMode->sigPol.vsync);
	SetIspRegister(isp->regVal, ISP_GLB_CFG3_INVPOL_PCLK, inputMode->sigPol.pclk);

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG0], isp->regVal[rISP_GLB_CFG0]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_AWB_MXN], isp->regVal[rISP_AWB_MXN]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RH], isp->regVal[rISP_GLB_RH]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RV], isp->regVal[rISP_GLB_RV]));

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG3], isp->regVal[rISP_GLB_CFG3]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_REG_CONFIG], isp->regVal[rISP_REG_CONFIG]));

	isp->inWidth = inputMode->inWidth;	
	isp->inHeight = inputMode->inHeight;
	isp->intfMode = inputMode->intfMode;
	isp->rawMode = inputMode->rawMode;
	isp->inputBitsNum = inputMode->inputBitsNum;
	isp->ituType.scanMode = inputMode->ituType.scanMode;
	isp->ituType.format = inputMode->ituType.format;
	isp->ituType.order = inputMode->ituType.order;
	isp->sigPol.hsync = inputMode->sigPol.hsync;
	isp->sigPol.href = inputMode->sigPol.href;
	isp->sigPol.vsync = inputMode->sigPol.vsync;
	isp->sigPol.pclk = inputMode->sigPol.pclk;

	//set isp sub module that depends on inputMode changing (such as awb/osd and so on) 
	//TODO 
	return ret;
Fail:
	return IM_RET_FAILED;
}

IM_RET isplib_set_input_resolution(IM_UINT32 width, IM_UINT32 height)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 framePixNum = 0;
	scl_input_t inPut;
	isp_crop_coordinate_t coordinate;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	ISP_CHECK(isp != IM_NULL);

	//check input size
	if((height < 0) || (height > 4096)
		||(width < 0) || (width > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid input size: height=%d, width=%d"), height, width));
		goto Fail;
	}

	framePixNum = width*height;

	isplib_config_update(IM_FALSE);

	/*set resolution correlative registers*/
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_VPIXNUM, width);
	SetIspRegister(isp->regVal, ISP_GLB_CFG0_HPIXNUM, height);
	SetIspRegister(isp->regVal, ISP_AWB_MXN, framePixNum);
	SetIspRegister(isp->regVal, ISP_GLB_RH, (1024*1024)/height);
	SetIspRegister(isp->regVal, ISP_GLB_RV, (1024*1024)/width);

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG0], isp->regVal[rISP_GLB_CFG0]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_AWB_MXN], isp->regVal[rISP_AWB_MXN]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RH], isp->regVal[rISP_GLB_RH]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_RV], isp->regVal[rISP_GLB_RV]));

	isp->inWidth = width;	
	isp->inHeight = height;

	//set isp sub module that depends on input resolution changing (such as crop/scl and so on) 
	if((isp->scl->verEnable == IM_TRUE) || (isp->scl->horEnable == IM_TRUE))
	{
		inPut.inWidth = width;
		inPut.inHeight = height;	
		scl_set_input(isp->scl, &inPut);
	}
	//should set value again current zoom value???	
	if(isp->crop->enable == IM_TRUE)
	{
		coordinate.x0 = 0;
		coordinate.y0 = 0;
		coordinate.x1 = width;
		coordinate.y1 = height;
		crop_set_coordinate(isp->crop, &coordinate);
	}

	isplib_config_update(IM_TRUE);
	return ret;
Fail:
	return IM_RET_FAILED;
}

IM_RET isplib_set_output_mode(isp_output_mode_t *outputMode)
{
	IM_RET ret = IM_RET_OK;
	scl_output_t outPut;
	isp_crop_coordinate_t coordinate;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	ISP_CHECK(isp != IM_NULL);

	if(outputMode == IM_NULL)    
	{
		IM_ERRMSG((IM_STR("Invalid parameter, outputMode is NULL pointer")));
		return IM_RET_FAILED;
	}    
	//check out size
	if((outputMode->outWidth < 0) || (outputMode->outHeight > 4096)
		||(outputMode->outHeight < 0) || (outputMode->outHeight > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid output size: height=%d, width=%d"), outputMode->outHeight, outputMode->outHeight));
		goto Fail;
	}
	//check out format
	if((outputMode->outImgType != IM_IMAGE_RGB8880) && (outputMode->outImgType != IM_IMAGE_RGB0888)
		&& (outputMode->outImgType != IM_IMAGE_BGR0888) && (outputMode->outImgType != IM_IMAGE_BGR8880)
		&& (outputMode->outImgType != IM_IMAGE_YUV420P) && (outputMode->outImgType != IM_IMAGE_YUV420SP)
		&& (outputMode->outImgType != IM_IMAGE_YUV422P) && (outputMode->outImgType != IM_IMAGE_YUV422SP)
		&& (outputMode->outImgType != IM_IMAGE_YUV444P))
	{
		IM_ERRMSG((IM_STR("Invalid(not support this)output format:outFormat=%d"),
			outputMode->outImgType));
		goto Fail;
	}
		
	isplib_config_update(IM_FALSE);
	//set isp sub module that depends on outputMode changing (such as scl and so on) 
	if((isp->scl->verEnable == IM_TRUE) || (isp->scl->horEnable == IM_TRUE))
	{
		outPut.outWidth = outputMode->outWidth;
		outPut.outHeight = outputMode->outHeight;	
		outPut.outFormat = outputMode->outImgType;	
		scl_set_output(isp->scl, &outPut);
	}

	//should set value again current zoom value???	
	if(isp->crop->enable == IM_TRUE)
	{
		coordinate.x0 = 0;
		coordinate.y0 = 0;
		coordinate.x1 = outputMode->outWidth;
		coordinate.y1 = outputMode->outHeight;
		crop_set_coordinate(isp->crop, &coordinate);
	}

	isplib_config_update(IM_TRUE);
	isp->outWidth = outputMode->outWidth;
	isp->outHeight =  outputMode->outHeight;
	isp->outImgType = outputMode->outImgType;

	return ret;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: isplib_start

  Functional description:

Inputs:

Outputs:

Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_start(void)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 i = 0;

	IM_INFOMSG((IM_STR("%s++"), IM_STR(_IM_FUNC_)));
	ISP_CHECK(isp != IM_NULL);
	
	if(isp->enable == IM_TRUE)    
	{
		IM_ERRMSG((IM_STR("isp has been opened")));
		return IM_RET_OK;
	}  
	
	/*ISP REGISTER UPDATE ENABLE, only for test now*/
	SetIspRegister(isp->regVal, ISP_REG_CONFIG_UPDATE, 1);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_REG_CONFIG], isp->regVal[rISP_REG_CONFIG]));
	
	//enable dma
	//ch0(y)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH0DMAEN, (isp->dma.lengthY==0)?0:1);
	SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_EN, (isp->dma.lengthY==0)?0:1);
	//ch1(cb/u)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH1DMAEN, (isp->dma.lengthCb==0)?0:1);
	SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_EN, (isp->dma.lengthCb==0)?0:1);
	//ch2(cr/v)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH2DMAEN, (isp->dma.lengthCr==0)?0:1);
	SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_EN, (isp->dma.lengthCr==0)?0:1);

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC1], isp->regVal[rISP_CH0DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC1], isp->regVal[rISP_CH1DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC1], isp->regVal[rISP_CH2DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));

	
	//ISP enable
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_ISPEN, 1);	
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));

	isp->enable = IM_TRUE;
	IM_INFOMSG((IM_STR("%s--"), IM_STR(_IM_FUNC_)));

#if 0
	// acc/acm/awb done after second frame
	while(i<3)
	{
		i++;
		ret = isppwl_sync(-1);
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("isppwl_sync failed")));
			return IM_RET_FAILED;
		}
	}
#endif
	
	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}


/*------------------------------------------------------------------------------

    Function name: isplib_stop

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_stop(void)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));	
	ISP_CHECK(isp != IM_NULL);

	if(isp->enable == IM_FALSE)    
	{
		IM_ERRMSG((IM_STR("isp has been closed")));
		return IM_RET_OK;
	}  
	
	//ISP disable
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_ISPEN, 0);	
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));

	//disable dma
	//ch0(y)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH0DMAEN, 0);
	SetIspRegister(isp->regVal, ISP_CH0DMA_CC1_EN, 0);
	//ch1(cb/u)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH1DMAEN, 0);
	SetIspRegister(isp->regVal, ISP_CH1DMA_CC1_EN, 0);
	//ch2(cr/v)
	SetIspRegister(isp->regVal, ISP_GLB_CFG1_CH2DMAEN, 0);
	SetIspRegister(isp->regVal, ISP_CH2DMA_CC1_EN, 0);

	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG1], isp->regVal[rISP_GLB_CFG1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH0DMA_CC1], isp->regVal[rISP_CH0DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH1DMA_CC1], isp->regVal[rISP_CH1DMA_CC1]));
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_CH2DMA_CC1], isp->regVal[rISP_CH2DMA_CC1]));

	isp->enable = IM_FALSE;
	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: isplib_wait_hw_ready

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));
	ISP_CHECK(isp != IM_NULL);

	ret = isppwl_wait_hw_ready(intr, timeout);
	if((ret != IM_RET_OK) || ((*intr & ISPPWL_INTR_DMA_SYNC)==0))
	{
		IM_ERRMSG((IM_STR("intr failed:ret=%d, *intr=%d"), ret, *intr));
		return IM_RET_FAILED;
	}
	IM_INFOMSG((IM_STR("wait ok, *intr=0x%x"), *intr));

	return ret;
}


/*------------------------------------------------------------------------------

    Function name: isplib_get_ready_buffer

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_get_ready_buffer(IM_Buffer *readyBuffs, IM_Buffer *replaceBuffs)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 index, i;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ISP_CHECK(isp != IM_NULL);
	IM_ASSERT((readyBuffs != IM_NULL) && (replaceBuffs != IM_NULL));

	// get ready chx number.	
	IM_JIF(isppwl_read_reg(isp->regOfst[rISP_CH0DMA_CC1], &isp->regVal[rISP_CH0DMA_CC1]));
	index = GetIspRegister(isp->regVal, ISP_CH0DMA_CC1_FRMNUM);
	if(isp->dma.ppMode != ISP_PPMODE_DISABLE){
		index = (index != 0)?(index - 1):3;
	}
	IM_INFOMSG((IM_STR("ready buffer index=%d"), index));

	if(isp->dma.lengthY != 0){
		if((replaceBuffs[0].phy_addr & 0x07) || (replaceBuffs[0].size < isp->dma.lengthY)){
			IM_ERRMSG((IM_STR("Invalid replaceBuff[0], vir=0x%x, phy=0x%x, size=%d"),
						(IM_INT32)replaceBuffs[0].vir_addr, replaceBuffs[0].phy_addr, replaceBuffs[0].size));
			return IM_RET_FAILED;
		}
		IM_JIF(getDmaBuffer(Y, index, &readyBuffs[0]));
		IM_JIF(setDmaBuffer(Y, index, &replaceBuffs[0]));
		IM_INFOMSG((IM_STR("get ready buffer: vir=0x%x, phy=0x%x"), readyBuffs[0].vir_addr, readyBuffs[0].phy_addr));

		if(isp->dma.ppMode == ISP_PPMODE_1_BUFFER){
			for(i=0; i<4; i++){
				if(i != index){
					IM_JIF(setDmaBuffer(Y, i, &replaceBuffs[0]));
				}
			}
		}else if(isp->dma.ppMode == ISP_PPMODE_2_BUFFER){
			IM_JIF(setDmaBuffer(Y, (index+2)%4, &replaceBuffs[0]));
		}
	}

	if(isp->dma.lengthCb != 0){
		if((replaceBuffs[1].phy_addr & 0x07) || (replaceBuffs[1].size < isp->dma.lengthCb)){
			IM_ERRMSG((IM_STR("Invalid replaceBuff[0], vir=0x%x, phy=0x%x, size=%d"),
						(IM_INT32)replaceBuffs[1].vir_addr, replaceBuffs[1].phy_addr, replaceBuffs[1].size));
			return IM_RET_FAILED;
		}
		IM_JIF(getDmaBuffer(U, index, &readyBuffs[1]));
		IM_JIF(setDmaBuffer(U, index, &replaceBuffs[1]));

		if(isp->dma.ppMode == ISP_PPMODE_1_BUFFER){
			for(i=0; i<4; i++){
				if(i != index){
					IM_JIF(setDmaBuffer(U, i, &replaceBuffs[1]));
				}
			}
		}else if(isp->dma.ppMode == ISP_PPMODE_2_BUFFER){
			IM_JIF(setDmaBuffer(U, (index+2)%4, &replaceBuffs[1]));
		}
	}

	if(isp->dma.lengthCr != 0){
		if((replaceBuffs[2].phy_addr & 0x07) || (replaceBuffs[2].size < isp->dma.lengthCr)){
			IM_ERRMSG((IM_STR("Invalid replaceBuff[0], vir=0x%x, phy=0x%x, size=%d"),
						(IM_INT32)replaceBuffs[2].vir_addr, replaceBuffs[2].phy_addr, replaceBuffs[2].size));
			return IM_RET_FAILED;
		}
		IM_JIF(getDmaBuffer(V, index, &readyBuffs[2]));
		IM_JIF(setDmaBuffer(V, index, &replaceBuffs[2]));

		if(isp->dma.ppMode == ISP_PPMODE_1_BUFFER){
			for(i=0; i<4; i++){
				if(i != index){
					IM_JIF(setDmaBuffer(V, i, &replaceBuffs[2]));
				}
			}
		}else if(isp->dma.ppMode == ISP_PPMODE_2_BUFFER){
			IM_JIF(setDmaBuffer(V, (index+2)%4, &replaceBuffs[2]));
		}
	}

	return ret;
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: isplib_config_update

  Functional description:

Inputs:

Outputs:

Returns: 

------------------------------------------------------------------------------*/
IM_RET isplib_config_update(IM_BOOL  updateEn)
{
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));	
	ISP_CHECK(isp != IM_NULL);
	
	/*ISP REGISTER UPDATE*/
	SetIspRegister(isp->regVal, ISP_REG_CONFIG_UPDATE, (updateEn==IM_TRUE)?1:0);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_REG_CONFIG], isp->regVal[rISP_REG_CONFIG]));

	IM_INFOMSG((IM_STR("%s:####isplib update has success,upvalue=%d"), IM_STR(_IM_FUNC_), updateEn));	
	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;

}


IM_RET setDmaBuffer(IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer)
{
	IM_INT32 regNumIndex;
	//IM_INT32 regIdIndex;

	IM_INFOMSG((IM_STR("%s(ch=%d, alt=%d)"), IM_STR(_IM_FUNC_), ch, alt));

	if(ch == Y){
		isp->dma.buffY[alt].vir_addr = buffer->vir_addr;
		isp->dma.buffY[alt].phy_addr = buffer->phy_addr;
		isp->dma.buffY[alt].size = buffer->size;
		isp->dma.buffY[alt].flag = buffer->flag;
		regNumIndex = rISP_CH0DMA_FBA1 + (alt << 1);
	}else if(ch == U){
		isp->dma.buffCb[alt].vir_addr = buffer->vir_addr;
		isp->dma.buffCb[alt].phy_addr = buffer->phy_addr;
		isp->dma.buffCb[alt].size = buffer->size;
		isp->dma.buffCb[alt].flag = buffer->flag;
		regNumIndex = rISP_CH1DMA_FBA1 + (alt << 1);
	}else if(ch == V){
		isp->dma.buffCr[alt].vir_addr = buffer->vir_addr;
		isp->dma.buffCr[alt].phy_addr = buffer->phy_addr;
		isp->dma.buffCr[alt].size = buffer->size;
		isp->dma.buffCr[alt].flag = buffer->flag;
		regNumIndex = rISP_CH2DMA_FBA1 + (alt << 1);
	}

	//it can also use SetIspRegister, but it must get regIdIndex first
	//SetIspRegister(isp->regVal, regIdIndex, buffer->phy_addr);
	isp->regVal[regNumIndex] = buffer->phy_addr; 
	IM_INFOMSG((IM_STR("WRITE_REG(0x%x, 0x%x)"), isp->regOfst[regNumIndex], isp->regVal[regNumIndex]));
	IM_JIF(isppwl_write_reg(isp->regOfst[regNumIndex], isp->regVal[regNumIndex]));

	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

IM_RET getDmaBuffer(IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer)
{
	IM_INFOMSG((IM_STR("%s(ch=%d, alt=%d)"), IM_STR(_IM_FUNC_), ch, alt));
	if(ch == Y){
		buffer->vir_addr = isp->dma.buffY[alt].vir_addr;
		buffer->phy_addr = isp->dma.buffY[alt].phy_addr;
		buffer->size = isp->dma.buffY[alt].size;
		buffer->flag = isp->dma.buffY[alt].flag;
	}else if(ch == U){
		buffer->vir_addr = isp->dma.buffCb[alt].vir_addr;
		buffer->phy_addr = isp->dma.buffCb[alt].phy_addr;
		buffer->size = isp->dma.buffCb[alt].size;
		buffer->flag = isp->dma.buffCb[alt].flag;
	}else if(ch == V){
		buffer->vir_addr = isp->dma.buffCr[alt].vir_addr;
		buffer->phy_addr = isp->dma.buffCr[alt].phy_addr;
		buffer->size = isp->dma.buffCr[alt].size;
		buffer->flag = isp->dma.buffCr[alt].flag;
	}

	return IM_RET_OK;
}


//#####################################################################
// isp sub module
//#####################################################################
//bccb module
IM_RET isplib_bccb_set_mode(IM_UINT32 mode)
{
	ISP_CHECK(isp != IM_NULL);
	return bccb_set_mode(isp->bccb, mode);
}
IM_RET isplib_bccb_set_bc(IM_UINT32 blkTh)
{
	bc_context_t bc;
	ISP_CHECK(isp != IM_NULL);
	bc.rBlkTh = blkTh;
	bc.grBlkTh = blkTh;
	bc.gbBlkTh = blkTh;
	bc.bBlkTh = blkTh;
	return bccb_set_bc(isp->bccb, &bc);
}
IM_RET isplib_bccb_set_cb(IM_UINT32 rGain, IM_UINT32 gGain, IM_UINT32 bGain)
{
	cb_context_t cb;
	ISP_CHECK(isp != IM_NULL);
	cb.rGain = rGain; 
	cb.grGain = gGain; 
	cb.gbGain = cb.grGain; 
	cb.bGain = bGain; 
	return bccb_set_cb(isp->bccb, &cb);
}
IM_RET isplib_bccb_set_bc_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bccb_set_bc_enable(isp->bccb);
}
IM_RET isplib_bccb_set_bc_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bccb_set_bc_disable(isp->bccb);
}
IM_RET isplib_bccb_set_cb_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bccb_set_cb_enable(isp->bccb);
}
IM_RET isplib_bccb_set_cb_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bccb_set_cb_disable(isp->bccb);
}

//bdc module
IM_RET isplib_bdc_set_detect_mode(IM_Buffer dmaBuf, IM_UINT32 detType, IM_UINT32 hiTh, IM_UINT32 loTh)
{
	bdc_detect_context_t det;

	ISP_CHECK(isp != IM_NULL);

	det.detType = detType;
	det.hiTh = hiTh;
	det.loTh = loTh;
	
	//set dma
	det.dma.length = dmaBuf.size;
	IM_BUFFER_COPYTO_BUFFER(det.dma.buff[0], dmaBuf);
	/*det.dma.buff[0].vir_addr = dmaBuf.vir_addr;
	  det.dma.buff[0].phy_addr = dmaBuf.phy_addr;
	  det.dma.buff[0].size = dmaBuf.size;
	  det.dma.buff[0].flag = dmaBuf.flag;*/

	//set only one buffer to DMA out
	det.dma.ppMode = ISP_PPMODE_DISABLE;
	det.dma.direct = ISP_BDC_DMA_WRITE;
	IM_BUFFER_COPYTO_BUFFER(isp->bdc->dmaOutbuf, det.dma.buff[0]);

	return bdc_set_detect_mode(isp->bdc, &det);
}

IM_RET isplib_bdc_set_correct_mode(IM_Buffer dmaBuf, IM_UINT32 crtType, IM_UINT32 sltPepTh, IM_UINT32 nosLvl)
{
	bdc_correct_context_t crt;
	
	ISP_CHECK(isp != IM_NULL);

	//set bp correct
	crt.crtEn = (crtType & ISP_BDC_BPCRT_ENABLE)?(IM_TRUE):(IM_FALSE);
	
	//set denoise
	if((crtType & ISP_BDC_GAUSS_ENABLE) && (crtType & ISP_BDC_SLTPEP_ENABLE))
	{
		crt.nosLvl = nosLvl;
		crt.sltPepTh = sltPepTh;
		crt.denosEn = IM_TRUE;
	}
	else if(crtType & ISP_BDC_GAUSS_ENABLE)
	{
		crt.nosLvl = nosLvl;
		crt.sltPepTh = 4095;
		crt.denosEn = IM_TRUE;
	}
	else if(crtType & ISP_BDC_SLTPEP_ENABLE)
	{
		crt.nosLvl = ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS;
		crt.sltPepTh = sltPepTh;
		crt.denosEn = IM_TRUE;
	}
	else
	{
		crt.nosLvl = nosLvl;
		crt.sltPepTh = sltPepTh;
		crt.denosEn = IM_FALSE;
	}
	
	//set dma
	crt.dma.length = dmaBuf.size;
	IM_BUFFER_COPYTO_BUFFER(crt.dma.buff[0], dmaBuf);
	/*crt.dma.buff[0].vir_addr = dmaBuf.vir_addr;
	  crt.dma.buff[0].phy_addr = dmaBuf.phy_addr;
	  crt.dma.buff[0].size = dmaBuf.size;
	  crt.dma.buff[0].flag = dmaBuf.flag;*/

	//set only one buffer to DMA in
	crt.dma.ppMode = ISP_PPMODE_DISABLE;
	crt.dma.direct = ISP_BDC_DMA_READ;
	IM_BUFFER_COPYTO_BUFFER(isp->bdc->dmaInbuf, crt.dma.buff[0]);

	return bdc_set_correct_mode(isp->bdc, &crt);
}

#define BP_MAX_NUM (64*1024)	//64K

IM_RET bdc_bp_analyse(void* dmabuf, IM_UINT32 bpNum, IM_UINT32 width, IM_UINT32 height)
{
	IM_UINT32  i, j;
	IM_UINT32 *buf = (IM_UINT32 *)dmabuf;
	/*blokPos33 is orginal point with x33=blokPos[i][0], y33=blokPos[i][1]*/
	IM_UINT32  x11, y11;//blokPos11
	IM_UINT32  x13, y13;//blokPos13
	IM_UINT32  x15, y15;//blokPos15
	IM_UINT32  x22, y22;//blokPos22
	IM_UINT32  x24, y24;//blokPos24
	IM_UINT32  x31, y31;//blokPos31
	IM_UINT32  x35, y35;//blokPos35
	IM_UINT32  x42, y42;//blokPos42
	IM_UINT32  x44, y44;//blokPos44
	IM_UINT32  x51, y51;//blokPos51
	IM_UINT32  x53, y53;//blokPos53
	IM_UINT32  x55, y55;//blokPos55
	/*blokPos[][0]: width/x, blokPos[][1]: height/y*/
	IM_UINT32  blokPos[BP_MAX_NUM][2];
	/*out[]][0]:blokPos13/dout[32]
	out[]][1]:blokPos53/dout[33]
	out[]][2]:blokPos31/dout[34]
	out[]][3]:blokPos35/dout[35]
	out[]][4]:blokPos11/dout[36]
	out[]][5]:blokPos55/dout[37]
	out[]][6]:blokPos15/dout[38]
	out[]][7]:blokPos51/dout[39]
	out[]][8]:blokPos22/dout[40]
	out[]][9]:blokPos44/dout[41]
	out[]][10]:blokPos24/dout[42]
	out[]][11]:blokPos42/dout[43]*/
	IM_UINT32  out[BP_MAX_NUM][12];

	if(bpNum > BP_MAX_NUM)
	{
		IM_ERRMSG((IM_STR("bp_num(%d) is larger than max_bp_num(%d)!"), bpNum, BP_MAX_NUM));
		return IM_RET_FAILED;
	}
	//0: bad point, 1: not bad point
	for(i=0; i<bpNum; i++)
	{
		out[i][0] = 1;
		out[i][1] = 1;
		out[i][2] = 1;
		out[i][3] = 1;
		out[i][4] = 1;
		out[i][5] = 1;
		out[i][6] = 1;
		out[i][7] = 1;
		out[i][8] = 1;
		out[i][9] = 1;
		out[i][10] = 1;
		out[i][11] = 1;
		blokPos[i][0] = (buf[2*i]&0x00fff000)>>12;
		blokPos[i][1] = (buf[2*i]&0x00000fff);
		//IM_INFOMSG((IM_STR("badpoint%d: x=%d, y=%d"), i, blokPos[i][0], blokPos[i][1]));
	}

	for(i=0; i<bpNum; i++)
	{		
		//blokPos11
		x11 = (blokPos[i][0] > 2)?(blokPos[i][0] - 2):(2 - blokPos[i][0]);
		y11 = (blokPos[i][1] > 2)?(blokPos[i][1] - 2):(2 - blokPos[i][1]);
		//blokPos13
		x13 = blokPos[i][0];
		y13 = (blokPos[i][1] > 2)?(blokPos[i][1] - 2):(2 - blokPos[i][1]);
		//blokPos15
		x15 = (blokPos[i][0] + 2 < width)?(blokPos[i][0] + 2):((width-1)*2-(blokPos[i][0] + 2));
		y15 = (blokPos[i][1] > 2)?(blokPos[i][1] - 2):(2 - blokPos[i][1]);
		//blokPos22
		x22 = (blokPos[i][0] > 1)?(blokPos[i][0] - 1):(1 - blokPos[i][0]);
		y22 = (blokPos[i][1] > 1)?(blokPos[i][1] - 1):(1 - blokPos[i][1]);
		//blokPos24
		x24 = (blokPos[i][0] + 1 < width)?(blokPos[i][0] + 1):((width-1)*2-(blokPos[i][0] + 1));
		y24 = (blokPos[i][1] > 1)?(blokPos[i][1] - 1):(1 - blokPos[i][1]);
		//blokPos31
		x31 = (blokPos[i][0] > 2)?(blokPos[i][0] - 2):(2 - blokPos[i][0]);
		y31 = blokPos[i][1];
		//blokPos35
		x35 = (blokPos[i][0] + 2 < width)?(blokPos[i][0] + 2):((width-1)*2-(blokPos[i][0] + 2));
		y35 = blokPos[i][1];
		//blokPos42
		x42 = (blokPos[i][0] > 1)?(blokPos[i][0] - 1):(1 - blokPos[i][0]);
		y42 = (blokPos[i][1] + 1 < height)?(blokPos[i][1] + 1):((height-1)*2-(blokPos[i][1] + 1));
		//blokPos44
		x44 = (blokPos[i][0] + 1 < width)?(blokPos[i][0] + 1):((width-1)*2-(blokPos[i][0] + 1));
		y44 = (blokPos[i][1] + 1 < height)?(blokPos[i][1] + 1):((height-1)*2-(blokPos[i][1] + 1));
		//blokPos51
		x51 = (blokPos[i][0] > 2)?(blokPos[i][0] - 2):(2 - blokPos[i][0]);
		y51 = (blokPos[i][1] + 2 < height)?(blokPos[i][1] + 2):((height-1)*2-(blokPos[i][1] + 2));
		//blokPos53
		x53 = blokPos[i][0];
		y53 = (blokPos[i][1] + 2 < height)?(blokPos[i][1] + 2):((height-1)*2-(blokPos[i][1] + 2));
		//blokPos55
		x55 = (blokPos[i][0] + 2 < width)?(blokPos[i][0] + 2):((width-1)*2-(blokPos[i][0] + 2));
		y55 = (blokPos[i][1] + 2 < height)?(blokPos[i][1] + 2):((height-1)*2-(blokPos[i][1] + 2));

		for(j=0; j<bpNum; j++)
		{
			if((x11==blokPos[j][0])&&(y11==blokPos[j][1]))
				out[i][4] = 0;
			if((x13==blokPos[j][0])&&(y13==blokPos[j][1]))
				out[i][0] = 0;
			if((x15==blokPos[j][0])&&(y15==blokPos[j][1]))
				out[i][6] = 0;
			if((x22==blokPos[j][0])&&(y22==blokPos[j][1]))
				out[i][8] = 0;
			if((x24==blokPos[j][0])&&(y24==blokPos[j][1]))
				out[i][10] = 0;
			if((x31==blokPos[j][0])&&(y31==blokPos[j][1]))
				out[i][2] = 0;
			if((x35==blokPos[j][0])&&(y35==blokPos[j][1]))
				out[i][3] = 0;
			if((x42==blokPos[j][0])&&(y42==blokPos[j][1]))
				out[i][11] = 0;
			if((x44==blokPos[j][0])&&(y44==blokPos[j][1]))
				out[i][9] = 0;
			if((x51==blokPos[j][0])&&(y51==blokPos[j][1]))
				out[i][7] = 0;
			if((x53==blokPos[j][0])&&(y53==blokPos[j][1]))
				out[i][1] = 0;
			if((x55==blokPos[j][0])&&(y55==blokPos[j][1]))
				out[i][5] = 0;
		}
	}
	
	for(i=0; i<bpNum; i++)
	{
		buf[2*i+1] |= (out[i][0]<<0);
		buf[2*i+1] |= (out[i][1]<<1);
		buf[2*i+1] |= (out[i][2]<<2);
		buf[2*i+1] |= (out[i][3]<<3);
		buf[2*i+1] |= (out[i][4]<<4);
		buf[2*i+1] |= (out[i][5]<<5);
		buf[2*i+1] |= (out[i][6]<<6);
		buf[2*i+1] |= (out[i][7]<<7);
		buf[2*i+1] |= (out[i][8]<<8);
		buf[2*i+1] |= (out[i][9]<<9);
		buf[2*i+1] |= (out[i][10]<<10);
		buf[2*i+1] |= (out[i][11]<<11);
		//IM_INFOMSG((IM_STR("badpoint%d info: info=0x%x"), i, buf[2*i+1]));
	}
	return IM_RET_OK;

}

IM_RET isplib_bdc_get_bp_info(void)
{
	IM_RET ret = IM_RET_OK;

	IM_UINT32 bpNum;
	
	ISP_CHECK(isp != IM_NULL);

	//first disable bdc module before get data from bdc dma outbuf
	ret = bdc_set_disable(isp->bdc);
	if(ret != IM_RET_OK)    
	{
		IM_ERRMSG((IM_STR("bdc_set_disable failed!")));
		return IM_RET_FAILED;
	}

	ret = bdc_get_bp_number(isp->bdc, &bpNum);
	IM_INFOMSG((IM_STR("bdc bad point number is: %d"), bpNum));
	if(ret != IM_RET_OK)    
	{
		IM_ERRMSG((IM_STR("bdc_set_disable failed!")));
		return IM_RET_FAILED;
	}
	
	if(isp->bdc->dmaOutbuf.vir_addr == IM_NULL)
	{
		IM_ERRMSG((IM_STR("dmaOutbuf is NULL")));
		return IM_RET_FAILED;
	}

#if 0
	//only  for test because of bpnum bug
	if(bpNum == 0)
	{
		bpNum = 1500;
	}
#endif
	
	bdc_bp_analyse(isp->bdc->dmaOutbuf.vir_addr, bpNum, isp->inWidth, isp->inHeight);
	isp->bdc->bpNum = bpNum;
	return IM_RET_OK;
	
}

IM_RET isplib_bdc_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bdc_set_enable(isp->bdc);
}
IM_RET isplib_bdc_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return bdc_set_disable(isp->bdc);
}

//lens module
IM_RET isplib_lens_set_lutb(IM_UINT32 lutbMode)
{
	ISP_CHECK(isp != IM_NULL);
	return lens_set_lutb(isp->lens, lutbMode);
}
IM_RET isplib_lens_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return lens_set_enable(isp->lens);
}
IM_RET isplib_lens_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return lens_set_disable(isp->lens);
}

//demosaic module
IM_RET isplib_dem_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);

	SetIspRegister(isp->regVal, ISP_GLB_CFG2_DEMBYPASS, 0);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG2], isp->regVal[rISP_GLB_CFG2]));

	isp->demEnable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
IM_RET isplib_dem_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);

	SetIspRegister(isp->regVal, ISP_GLB_CFG2_DEMBYPASS, 1);
	IM_JIF(isppwl_write_reg(isp->regOfst[rISP_GLB_CFG2], isp->regVal[rISP_GLB_CFG2]));

	isp->demEnable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

//gma module
IM_RET isplib_gma_set_mode(IM_UINT32 mode)
{
	ISP_CHECK(isp != IM_NULL);
	return gma_set_mode(isp->gma, mode);
}
IM_RET isplib_gma_set_round_mode(IM_UINT32 rdMode)
{
	ISP_CHECK(isp != IM_NULL);
	return gma_set_round_mode(isp->gma, rdMode);
}
IM_RET isplib_gma_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return gma_set_enable(isp->gma);
}
IM_RET isplib_gma_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return gma_set_disable(isp->gma);
}

//ee module
IM_RET isplib_ee_set_coefw(IM_UINT32 coefw)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_coefw(isp->ee, coefw);
}
IM_RET isplib_ee_set_coefa(IM_UINT32 coefa)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_coefa(isp->ee, coefa);
}
IM_RET isplib_ee_set_round_mode(IM_UINT32 rdMode)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_round_mode(isp->ee, rdMode);
}
IM_RET isplib_ee_set_gauss_filter_enable(IM_BOOL gasEn)
{ 
	ISP_CHECK(isp != IM_NULL);
	return ee_set_gauss_filter_enable(isp->ee, gasEn);
}
IM_RET isplib_ee_set_gauss_mode(IM_UINT32 gasMode)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_gauss_mode(isp->ee, gasMode);
}
IM_RET isplib_ee_set_error_threshold(IM_UINT32 errTh)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_error_threshold(isp->ee, errTh);
}
IM_RET isplib_ee_set_detect_threshold(IM_UINT32 detTh)
{
	isp_ee_thr_matrix thrMat;
	ISP_CHECK(isp != IM_NULL);
	thrMat.hTh = detTh;
	thrMat.vTh = detTh;
	thrMat.d0Th = detTh;
	thrMat.d1Th = detTh;
	return ee_set_detect_threshold_matrix(isp->ee, &thrMat);
}
IM_RET isplib_ee_set_detect_threshold_matrix(isp_ee_thr_matrix *thrMat)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_detect_threshold_matrix(isp->ee, thrMat);
}
IM_RET isplib_ee_set_edge_operator_matrix(IM_UINT32 direction, isp_ee_op_matrix *opMat)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_edge_operator_matrix(isp->ee, direction, opMat);
}
IM_RET isplib_ee_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_enable(isp->ee);
}
IM_RET isplib_ee_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ee_set_disable(isp->ee);
}

//fcc module
IM_RET isplib_fcc_set_threshold(IM_UINT32 threshold)
{
	ISP_CHECK(isp != IM_NULL);
	return fcc_set_threshold(isp->fcc, threshold);
}
IM_RET isplib_fcc_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return fcc_set_enable(isp->fcc);
}
IM_RET isplib_fcc_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return fcc_set_disable(isp->fcc);
}

//af module
IM_RET isplib_af_set_type(IM_UINT32 type)
{
	ISP_CHECK(isp != IM_NULL);
	return af_set_type(isp->af, type);
}
IM_RET isplib_af_set_block_coordinate(isp_af_coordinate_t *coordinate)
{
	ISP_CHECK(isp != IM_NULL);
	return af_set_block_coordinate(isp->af, coordinate);
}
IM_RET isplib_af_get_result(isp_af_result_t *rsut)
{
	ISP_CHECK(isp != IM_NULL);
	return af_get_result(isp->af, rsut);
}
IM_RET isplib_af_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return af_set_enable(isp->af);
}
IM_RET isplib_af_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return af_set_disable(isp->af);
}

IM_RET isplib_awb_set_analyze_mode(IM_UINT32 anaMode)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_analyze_mode(isp->awb, anaMode);
}
IM_RET isplib_awb_set_roi_position(isp_awb_roi_position *roiPos)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_roi_position(isp->awb, roiPos);
}
IM_RET isplib_awb_set_threshold_matrix(isp_awb_thr_matrix *thrMat)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_threshold_matrix(isp->awb, thrMat);
}
IM_RET isplib_awb_set_par_matrix(isp_awb_par_matrix *parMat)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_par_matrix(isp->awb, parMat);
}
IM_RET isplib_awb_get_result(isp_awb_result_t *rsut)
{
	IM_RET ret;
	IM_UINT32 i;
	ISP_CHECK(isp != IM_NULL);
#if 0
	//read awb result after several frames
	while(i<3)
	{
		i++;
		ret = isppwl_sync(-1);
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("isppwl_sync failed")));
			return IM_RET_FAILED;
		}
	}

#endif
	return awb_get_result(isp->awb, rsut);
}
IM_RET isplib_awb_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_enable(isp->awb);
}
IM_RET isplib_awb_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return awb_set_disable(isp->awb);
}

//cmncsc module
IM_RET isplib_cmncsc_set_mode(IM_UINT32 mode)
{
	ISP_CHECK(isp != IM_NULL);
	return cmncsc_set_mode(isp->cmncsc, mode);
}
IM_RET isplib_cmncsc_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return cmncsc_set_enable(isp->cmncsc);
}
IM_RET isplib_cmncsc_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return cmncsc_set_disable(isp->cmncsc);
}

//ae module
IM_RET isplib_ae_set_block_select(isp_ae_block_select *blokSelect)
{
	ISP_CHECK(isp != IM_NULL);
	return ae_set_block_select(isp->ae, blokSelect);
}
IM_RET isplib_ae_set_block_coordinate(isp_ae_coordinate_t *coordinate)
{
	ISP_CHECK(isp != IM_NULL);
	return ae_set_block_coordinate(isp->ae, coordinate);
}
IM_RET isplib_ae_set_type(IM_UINT32 type)
{
	/*coordinateinat only for 640*480*/
	isp_ae_block_select blokSelect;
	isp_ae_coordinate_t coordinate;
	IM_UINT32 i;

	ISP_CHECK(isp != IM_NULL);

	isppwl_memcpy(&(coordinate), &(isp->ae->coordinate), sizeof(isp_ae_coordinate_t));

	if(type == ISP_AE_TYPE_CENTER)
	{
		blokSelect.blokEn = 0x1ffffff;
		blokSelect.blokNum = 25;
		ae_set_block_select(isp->ae, &blokSelect);

		coordinate.log2_blokPixNum = 13;
		for(i=0;i<25;i++)
		{		 
			coordinate.blokPos[i].x0 = 256;
			coordinate.blokPos[i].y0 = 208;
			coordinate.blokPos[i].x1 = 383;
			coordinate.blokPos[i].y1 = 271; 
		}
		
		ae_set_block_coordinate(isp->ae, &coordinate);
	}
	else if(type == ISP_AE_TYPE_CENTER_WEIGHT)
	{
		blokSelect.blokEn = 0x3f;
		blokSelect.blokNum = 6;
		ae_set_block_select(isp->ae, &blokSelect);

		coordinate.log2_blokPixNum = 13;
		//blokPos0
		coordinate.blokPos[0].x0 = 0;
		coordinate.blokPos[0].y0 = 16;
		coordinate.blokPos[0].x1 = 127;
		coordinate.blokPos[0].y1 = 79;
		//blokPos1
		coordinate.blokPos[1].x0 = 512;
		coordinate.blokPos[1].y0 = 16;
		coordinate.blokPos[1].x1 = 639;
		coordinate.blokPos[1].y1 = 79;
		//blokPos2
		coordinate.blokPos[2].x0 = 256;
		coordinate.blokPos[2].y0 = 208;
		coordinate.blokPos[2].x1 = 383;
		coordinate.blokPos[2].y1 = 271;
		//blokPos3
		coordinate.blokPos[3].x0 = 256;
		coordinate.blokPos[3].y0 = 208;
		coordinate.blokPos[3].x1 = 383;
		coordinate.blokPos[3].y1 = 271;
		//blokPos4
		coordinate.blokPos[4].x0 = 0;
		coordinate.blokPos[4].y0 = 400;
		coordinate.blokPos[4].x1 = 127;
		coordinate.blokPos[4].y1 = 463;
		//blokPos5
		coordinate.blokPos[5].x0 = 512;
		coordinate.blokPos[5].y0 = 400;
		coordinate.blokPos[5].x1 = 639;
		coordinate.blokPos[5].y1 = 463;
		
		ae_set_block_coordinate(isp->ae, &coordinate);
	}
	else if(type == ISP_AE_TYPE_AVERAGE)
	{
		blokSelect.blokEn = 0x1ffffff;
		blokSelect.blokNum = 25;
		ae_set_block_select(isp->ae, &blokSelect);
	}
	else if(type == ISP_AE_TYPE_ROI)
	{
		blokSelect.blokEn = 0x1;
		blokSelect.blokNum = 1;
		ae_set_block_select(isp->ae, &blokSelect);

		coordinate.log2_blokPixNum = 14;
		//blokPos0
		coordinate.blokPos[0].x0 = 265;
		coordinate.blokPos[0].y0 = 312;
		coordinate.blokPos[0].x1 = 392;
		coordinate.blokPos[0].y1 = 439;
		ae_set_block_coordinate(isp->ae, &coordinate);
	}
	else if(type == ISP_AE_TYPE_MAIN_REGION)
	{
		blokSelect.blokEn = 0x1f73800;
		blokSelect.blokNum = 11;
		ae_set_block_select(isp->ae, &blokSelect);
	}
	else
	{
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}
IM_RET isplib_ae_get_result(isp_ae_result_t *rsut)
{
	IM_RET ret;
	IM_UINT32 i;
	ISP_CHECK(isp != IM_NULL);
#if 0
	//read ae result after several frames
	while(i<3)
	{
		i++;
		ret = isppwl_sync(-1);
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("isppwl_sync failed")));
			return IM_RET_FAILED;
		}
	}

#endif
	return ae_get_result(isp->ae, rsut);
}
IM_RET isplib_ae_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ae_set_enable(isp->ae);
}
IM_RET isplib_ae_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ae_set_disable(isp->ae);
}

//hist module
IM_RET isplib_hist_set_backlit_threshold(IM_UINT32 blitTh1Num, IM_UINT32 blitTh1Den)
{
	IM_UINT32 blitTh;
	ISP_CHECK(isp != IM_NULL);
	blitTh = (isp->inHeight*isp->inWidth*blitTh1Num)/blitTh1Den;
	return hist_set_backlit_threshold(isp->hist, blitTh);
}
IM_RET isplib_hist_set_hist_threshold_matrix(isp_hist_thr_matrix *thrMat)
{
	ISP_CHECK(isp != IM_NULL);
	return hist_set_hist_threshold_matrix(isp->hist, thrMat);
}
IM_RET isplib_hist_get_result(isp_hist_result_t *rsut)
{
	ISP_CHECK(isp != IM_NULL);
	return hist_get_result(isp->hist, rsut);
}
IM_RET isplib_hist_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return hist_set_enable(isp->hist);
}
IM_RET isplib_hist_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return hist_set_disable(isp->hist);
}

//acc module
IM_RET isplib_acc_set_round_mode(IM_UINT32 rdMode)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_round_mode(isp->acc, rdMode);
}
IM_RET isplib_acc_set_lutb(isp_acc_lutb_type *lutbType)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_lutb(isp->acc, lutbType);
}
IM_RET isplib_acc_set_hist(isp_acc_hist_t *hist)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_hist(isp->acc, hist);
}
IM_RET isplib_acc_set_contrast_coef(IM_INT32 coefe)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_contrast_coef(isp->acc, coefe);
}
IM_RET isplib_acc_set_coef_matrix(isp_acc_co_matrix *coMat)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_coef_matrix(isp->acc, coMat);
}
IM_RET isplib_acc_set_ro_matrix(isp_acc_ro_matrix *roMat)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_ro_matrix(isp->acc, roMat);
}
IM_RET isplib_acc_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_enable(isp->acc);
}
IM_RET isplib_acc_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return acc_set_disable(isp->acc);
}

//ief module
IM_RET isplib_ief_set_type(IM_UINT32 type)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_type(isp->ief, type);
}
IM_RET isplib_ief_set_rgcf_matrix(isp_ief_rgcf_matrix *rgcfMat)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_rgcf_matrix(isp->ief, rgcfMat);
}
IM_RET isplib_ief_set_color_select_mode_matrix(isp_ief_select_matrix *selMat)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_color_select_mode_matrix(isp->ief, selMat);
}
IM_RET isplib_ief_set_csc_matrix(isp_ief_csc_matrix *cscMat)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_csc_matrix(isp->ief, cscMat);
}
IM_RET isplib_ief_set_csc_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_csc_enable(isp->ief);
}
IM_RET isplib_ief_set_csc_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_csc_disable(isp->ief);
}
IM_RET isplib_ief_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_enable(isp->ief);
}
IM_RET isplib_ief_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return ief_set_disable(isp->ief);
}

//acm module
IM_RET isplib_acm_set_round_mode(IM_UINT32 rdMode)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_round_mode(isp->acm, rdMode);
}
IM_RET isplib_acm_set_saturation_threshold(IM_INT32 ths)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_saturation_threshold(isp->acm, ths);
}
IM_RET isplib_acm_set_threshold_matrix(isp_acm_thr_matrix *thrMat)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_threshold_matrix(isp->acm, thrMat);
}
IM_RET isplib_acm_set_coef_matrix(isp_acm_coef_matrix *coMat)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_coef_matrix(isp->acm, coMat);
}
IM_RET isplib_acm_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_enable(isp->acm);
}
IM_RET isplib_acm_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return acm_set_disable(isp->acm);
}

//
//osd module
//
/*osd wndow0*/
IM_RET isplib_osd_wnd0_init(osd_wnd0_config_t *wnd0)
{
	osd_wnd0_context_t wnd0Ctx;
	ISP_CHECK(isp != IM_NULL);
	//wnd0
	wnd0Ctx.enable = wnd0->enable;
	isppwl_memcpy(&(wnd0Ctx.mapclr), &(wnd0->mapclr), sizeof(isp_osd_mapcolor_t));
	isppwl_memcpy(&(wnd0Ctx.coordinate), &(wnd0->coordinate), sizeof(isp_osd_coordinate_t));

	return osd_wnd0_init(isp->osd, &wnd0Ctx);
}
IM_RET isplib_osd_wnd0_deinit(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd0_deinit(isp->osd);
}
IM_RET isplib_osd_wnd0_set_mapcolor(isp_osd_mapcolor_t *map)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd0_set_mapcolor(isp->osd, map);
}
IM_RET isplib_osd_wnd0_set_coordinate(isp_osd_coordinate_t *coordinate)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd0_set_coordinate(isp->osd, coordinate);
}
IM_RET isplib_osd_wnd0_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd0_set_enable(isp->osd);
}
IM_RET isplib_osd_wnd0_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd0_set_disable(isp->osd);
} 

/*osd wndow1*/
IM_RET isplib_osd_wnd1_init(osd_wnd1_config_t *wnd1)
{
	osd_wnd1_context_t wnd1Ctx;
	ISP_CHECK(isp != IM_NULL);

	wnd1Ctx.enable = wnd1->enable;
	wnd1Ctx.imgFormat = wnd1->imgFormat;
	isppwl_memcpy(&(wnd1Ctx.palette), &(wnd1->palette), sizeof(isp_osd_palette_t));
	isppwl_memcpy(&(wnd1Ctx.swap), &(wnd1->swap), sizeof(isp_osd_swap_t));
	isppwl_memcpy(&(wnd1Ctx.alpha), &(wnd1->alpha), sizeof(isp_osd_alpha_t));
	isppwl_memcpy(&(wnd1Ctx.mapclr), &(wnd1->mapclr), sizeof(isp_osd_mapcolor_t));
	isppwl_memcpy(&(wnd1Ctx.clrkey), &(wnd1->clrkey), sizeof(isp_osd_colorkey_t));
	isppwl_memcpy(&(wnd1Ctx.vm), &(wnd1->vm), sizeof(isp_osd_vm_t));
	isppwl_memcpy(&(wnd1Ctx.coordinate), &(wnd1->coordinate), sizeof(isp_osd_coordinate_t));

	wnd1Ctx.bm.mode = ISP_OSD_BUFFER_SEL_MANUAL;
	wnd1Ctx.bm.number = 0;
	wnd1Ctx.bm.selType = ISP_OSD_BUFSEL_BUF0;

	wnd1Ctx.buffer.mask[0] = IM_FALSE;
	IM_BUFFER_COPYTO_BUFFER(wnd1Ctx.buffer.buff[0], wnd1->buf);
	/*wnd1Ctx.buffer.buff[0].vir_addr = wnd1->buf.vir_addr;	
	wnd1Ctx.buffer.buff[0].phy_addr = wnd1->buf.phy_addr;
	wnd1Ctx.buffer.buff[0].size = wnd1->buf.size;
	wnd1Ctx.buffer.buff[0].flag = wnd1->buf.flag;*/
	wnd1Ctx.buffer.mask[1] = IM_TRUE;
	wnd1Ctx.buffer.mask[2] = IM_TRUE;
	wnd1Ctx.buffer.mask[3] = IM_TRUE;
	
	//wnd1Ctx.imageSize = ;
	//wnd1Ctx.buffOffset = ;
	//wnd1Ctx.bitBuffOffset = ;

	return osd_wnd1_init(isp->osd, &wnd1Ctx);
}
IM_RET isplib_osd_wnd1_deinit(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_deinit(isp->osd);
}
IM_RET isplib_osd_wnd1_set_swap(isp_osd_swap_t *swap)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_swap(isp->osd, swap);
}
IM_RET isplib_osd_wnd1_set_alpha(isp_osd_alpha_t *alpha)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_alpha(isp->osd, alpha);
}
IM_RET isplib_osd_wnd1_set_format( IM_UINT32 imgFormat)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_format(isp->osd, imgFormat);
}
IM_RET isplib_osd_wnd1_set_palette_format(IM_UINT32 imgFormat, isp_osd_palette_t *palette)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_palette_format(isp->osd, imgFormat, palette);
}
IM_RET isplib_osd_wnd1_set_mapcolor(isp_osd_mapcolor_t *map)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_mapcolor(isp->osd, map);
}
IM_RET isplib_osd_wnd1_set_colorkey(isp_osd_colorkey_t *clrkey)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_colorkey(isp->osd, clrkey);
}
IM_RET isplib_osd_wnd1_set_buffer_mode( isp_osd_buffer_mode *bm)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_buffer_mode(isp->osd, bm);
}
IM_RET isplib_osd_wnd1_set_buffers(isp_osd_buffer_t *buffer)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_buffers(isp->osd, buffer);
}
IM_RET isplib_osd_wnd1_set_virtual_window(isp_osd_vm_t *vm)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_virtual_window(isp->osd, vm);
}
IM_RET isplib_osd_wnd1_set_coordinate(isp_osd_coordinate_t *coordinate)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_coordinate(isp->osd, coordinate);
}
IM_RET isplib_osd_wnd1_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_enable(isp->osd);
}
IM_RET isplib_osd_wnd1_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_wnd1_set_disable(isp->osd);
}

/*osd global*/
IM_RET isplib_osd_set_background_color(IM_UINT32 bgColor)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_set_background_color(isp->osd, bgColor);
}
IM_RET isplib_osd_set_out_size(IM_UINT32 width, IM_UINT32 height)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_set_out_size(isp->osd, width, height);
}
IM_RET isplib_osd_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_set_enable(isp->osd);
}
IM_RET isplib_osd_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return osd_set_disable(isp->osd);
}

//crop module
IM_RET isplib_crop_set_coordinate(isp_crop_coordinate_t *coordinate)
{
	ISP_CHECK(isp != IM_NULL);
	return crop_set_coordinate(isp->crop, coordinate);
}
IM_RET isplib_crop_set_enable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return crop_set_enable(isp->crop);
}
IM_RET isplib_crop_set_disable(void)
{
	ISP_CHECK(isp != IM_NULL);
	return crop_set_disable(isp->crop);
}

//scl module
IM_RET isplib_scl_set_input(IM_UINT32 inWidth, IM_UINT32 inHeight)
{
	scl_input_t inPut;
	inPut.inWidth = inWidth;
   	inPut.inHeight = inHeight;	
	ISP_CHECK(isp != IM_NULL);
	return scl_set_input(isp->scl, &inPut);
}
IM_RET isplib_scl_set_output(IM_UINT32 outWidth, IM_UINT32 outHeight)
{
	scl_output_t outPut;
	outPut.outWidth = outWidth;
   	outPut.outHeight = outHeight;	
   	outPut.outFormat = isp->outImgType;	
	ISP_CHECK(isp != IM_NULL);
	return scl_set_output(isp->scl, &outPut);
}
IM_RET isplib_scl_set_round_mode(IM_UINT32 vrdMode, IM_UINT32 hrdMode)
{
	ISP_CHECK(isp != IM_NULL);
	return scl_set_round_mode(isp->scl, vrdMode, hrdMode);
}
IM_RET isplib_scl_set_param_type(IM_UINT32 paramType)
{
	ISP_CHECK(isp != IM_NULL);
	return scl_set_param_type(isp->scl, paramType);
}
IM_RET isplib_scl_set_csc_mode(IM_UINT32 cscMode)
{
	ISP_CHECK(isp != IM_NULL);
	return scl_set_csc_mode(isp->scl, cscMode);
}
IM_RET isplib_scl_set_enable(IM_BOOL verEnable, IM_BOOL horEnable)
{
	ISP_CHECK(isp != IM_NULL);
	return scl_set_enable(isp->scl, verEnable, horEnable);
}

//digital zoom, zoom values multiply by 100, so real zoom = valuse/100
IM_RET isplib_set_digital_zoom(IM_UINT32 value)
{
	IM_UINT32 sclInWidth, sclInHeight;
	IM_UINT32 off_x, off_y;
	isp_crop_coordinate_t cropCoord;
	ISP_CHECK(isp != IM_NULL);

	if(value < 100)
	{
		IM_ERRMSG((IM_STR("digital value error(value=%d)!"), value));
		return IM_RET_FAILED;
	}

	off_x =((IM_UINT32) ((isp->inWidth*(value-100))/value))>>1;
	off_y = ((IM_UINT32)((isp->inHeight *(value-100))/value))>>1;
	cropCoord.x0 = off_x;
	cropCoord.y0 = off_y;
	cropCoord.x1 = isp->inWidth - off_x - 1; 
	cropCoord.y1 = isp->inHeight- off_y - 1; 
	sclInWidth = cropCoord.x1 - cropCoord.x0 + 1; 
	sclInHeight = cropCoord.y1 - cropCoord.y0 + 1; 
	isplib_config_update(IM_FALSE);
	isplib_crop_set_coordinate(&cropCoord);
	isplib_scl_set_input(sclInWidth, sclInHeight);
	isplib_crop_set_enable();
	isplib_scl_set_enable(IM_TRUE, IM_TRUE);
	isplib_config_update(IM_TRUE);

	return IM_RET_OK;
}

