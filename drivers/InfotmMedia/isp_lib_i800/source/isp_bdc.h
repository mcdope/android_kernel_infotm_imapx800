/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_bdc.h
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

#ifndef _ISP_BDC_H_
#define	_ISP_BDC_H_

//
// bdc dma direction.
//
#define ISP_BDC_DMA_READ		0
#define ISP_BDC_DMA_WRITE		1

typedef struct{
	IM_UINT32	direct;		//ISP_BDC_DMA_XXX
	IM_UINT32	ppMode;		//ISP_PPMODE_DISABLE(only one buffer) recommendation
	IM_Buffer	buff[4];
	IM_UINT32	length;
}bdc_dma_context_t;

typedef struct{
	IM_UINT32 			hiTh;
	IM_UINT32			loTh;
	IM_UINT32			detType;	//ISP_BDC_DETECT_TYPE_X
	bdc_dma_context_t 	dma;
}bdc_detect_context_t;

typedef struct{
	IM_BOOL				crtEn;
	IM_BOOL				denosEn;
	IM_UINT32			sltPepTh;
	IM_UINT32			nosLvl;			//ISP_BDC_CORRECT__NOISE_LEVELX
	bdc_dma_context_t 	dma;
}bdc_correct_context_t;

typedef struct{
	IM_BOOL				enable;
	IM_UINT32 			bdcMode;	//ISP_BDC_MODE_XXX
	IM_UINT32			detType;	//ISP_BDC_DETECT_TYPE_X
	IM_BOOL				crtEn;
	IM_BOOL				denosEn;
	IM_UINT32 			hiTh;
	IM_UINT32			loTh;
	IM_UINT32			sltPepTh;
	IM_UINT32			nosLvl;			//ISP_BDC_CORRECT__NOISE_LEVELX
	bdc_dma_context_t 	dma;
}bdc_config_t;

typedef struct{
	IM_BOOL				enable;
	IM_UINT32 			bdcMode;	//ISP_BDC_MODE_XXX
	IM_UINT32			detType;	//ISP_BDC_DETECT_TYPE_X
	IM_BOOL				crtEn;
	IM_BOOL				denosEn;
	IM_UINT32 			hiTh;
	IM_UINT32			loTh;
	IM_UINT32			sltPepTh;
	IM_UINT32			nosLvl;
	IM_UINT32			bpNum;
	IM_Buffer			dmaOutbuf;
	IM_Buffer			dmaInbuf;
	IM_UINT32			*regVal;
	IM_UINT32			*regOfst;
}isp_bdc_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET bdc_init(isp_bdc_context_t *bdc, bdc_config_t *cfg);

IM_RET bdc_set_detect_mode(isp_bdc_context_t *bdc, bdc_detect_context_t *det);
IM_RET bdc_set_correct_mode(isp_bdc_context_t *bdc, bdc_correct_context_t *crt);
IM_RET bdc_get_bp_number(isp_bdc_context_t *bdc, IM_UINT32 *bpNum);
IM_RET bdc_set_enable(isp_bdc_context_t *bdc);
IM_RET bdc_set_disable(isp_bdc_context_t *bdc);

#endif	//_ISP_BDC_H_
