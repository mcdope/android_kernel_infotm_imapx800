/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_scl.c
--
--  Description :
--		scl module.
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
#include "isp_scl.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"SCL_I:"
#define WARNHEAD	"SCL_W:"
#define ERRHEAD		"SCL_E:"
#define TIPHEAD		"SCL_T:"


static const IM_UINT32 scl_params[][6*64] = { 
#include "isp_scl_param_table.h"
};

/*{coef11,coef12,coef13,coef21,coef22,coef23,
coef31,coef32,coef33,ofta,oftb}*/
static const IM_INT32 csc[][11] = {
//ISP_SCL_CSC_MODE_0:rgb2yuv
{263, 516, 100, 450, -377, -73, -152, -298, 450, 16, 0},
//ISP_SCL_CSC_MODE_1(default cscMode): rgb2yuv
{263, 516, 100, -152, -298, 450, 450, -377, -73, 16, 0},	
};	



/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: scl_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_init(isp_scl_context_t *scl, scl_config_t *cfg)
{
	IM_UINT32 i;
	IM_UINT32 vscaling;
	IM_UINT32 hscaling;
	
	//default value 
	IM_UINT32 endianFormat = 0;	
	IM_UINT32 halfWordSwap = 0;
	IM_UINT32 byteSwap = 0;
	IM_UINT32 isRGB = 1;
	IM_UINT32 coFmt = 0;
	IM_UINT32 stFmt = 0;
	
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_))); 
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);

	/*check scl round mode*/
	if((cfg->vrdMode > ISP_ROUND_NEAREST) || (cfg->hrdMode > ISP_ROUND_NEAREST))
	{
		IM_ERRMSG((IM_STR("scl round mode error: vrdMode = %d, hrdMode = %d"), cfg->vrdMode, cfg->hrdMode));
		return IM_RET_OK;
	}

	/*check scl input size*/
	if((cfg->inPut.inHeight < 0) || (cfg->inPut.inHeight > 4096)
		|| (cfg->inPut.inWidth < 0) || (cfg->inPut.inWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid input size: height=%d, width=%d"),
			cfg->inPut.inHeight, cfg->inPut.inWidth));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check scl output type*/
	if((cfg->outPut.outHeight < 1) || (cfg->outPut.outHeight > 4096)
		|| (cfg->outPut.outWidth < 1) || (cfg->outPut.outWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid output size: height=%d, width=%d"),
			cfg->outPut.outHeight, cfg->outPut.outWidth));
		return IM_RET_INVALID_PARAMETER;
	}

	if((cfg->outPut.outFormat != IM_IMAGE_RGB8880) && (cfg->outPut.outFormat != IM_IMAGE_RGB0888)
		&& (cfg->outPut.outFormat != IM_IMAGE_BGR8880) && (cfg->outPut.outFormat != IM_IMAGE_BGR0888)
		&& (cfg->outPut.outFormat != IM_IMAGE_YUV420P) && (cfg->outPut.outFormat != IM_IMAGE_YUV420SP)
		&& (cfg->outPut.outFormat != IM_IMAGE_YUV422P) && (cfg->outPut.outFormat != IM_IMAGE_YUV422SP)
		&& (cfg->outPut.outFormat != IM_IMAGE_YUV444P))
	{
		IM_ERRMSG((IM_STR("Invalid(not support this)output format:outFormat=%d"),
			cfg->outPut.outFormat));
		return IM_RET_INVALID_PARAMETER;
	}

	switch(cfg->outPut.outFormat)//for YUV422/YUV420, SCL must enable
	{
		case IM_IMAGE_RGB0888:
			endianFormat = 0;	
			halfWordSwap = 0;
			byteSwap = 0;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_RGB8880:
			endianFormat = 1;	
			halfWordSwap = 0;
			byteSwap = 0;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_BGR0888:
			endianFormat = 1;	
			halfWordSwap = 1;
			byteSwap = 1;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_BGR8880:
			endianFormat = 0;	
			halfWordSwap = 1;
			byteSwap = 1;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_YUV444P:
			isRGB = 0;
			coFmt = 0;
			stFmt = 0;
			break;
		case IM_IMAGE_YUV420P:
			isRGB = 0;
			coFmt = 2;
			stFmt = 0;
			cfg->horEnable = IM_TRUE;
			cfg->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV420SP:
			isRGB = 0;
			coFmt = 2;
			stFmt = 1;
			cfg->horEnable = IM_TRUE;
			cfg->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV422P:
			isRGB = 0;
			coFmt = 1;
			stFmt = 0;
			cfg->horEnable = IM_TRUE;
			cfg->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV422SP:
			isRGB = 0;
			coFmt = 1;
			stFmt = 1;
			cfg->horEnable = IM_TRUE;
			cfg->verEnable = IM_TRUE;
			break;
		default:
			break;
	}

	//set scaleing
	vscaling = (cfg->inPut.inHeight*1024)/cfg->outPut.outHeight;
	hscaling = (cfg->inPut.inWidth*1024)/cfg->outPut.outWidth;
	IM_INFOMSG((IM_STR("scaling comput from input and output size:vscaling=%d,hscaling=%d"),
			vscaling, hscaling));
		
	if((cfg->horEnable == IM_TRUE) || (cfg->verEnable == IM_TRUE))
	{
		/*set scl round mode*/
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VROUND, cfg->vrdMode);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HROUND, cfg->hrdMode);


		/*set scl param type*/
		//use default value
		
		/*set scl input size*/
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IVRES, cfg->inPut.inHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IHRES, cfg->inPut.inWidth-1);


		/*set scl output type*/
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OVRES, cfg->outPut.outHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OHRES, cfg->outPut.outWidth-1);



		SetIspRegister(scl->regVal, ISP_SCL_SCR_VSCALING, vscaling);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HSCALING, hscaling);
	
		/*set scl enable or bypass state*/
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VPASSBY, (cfg->verEnable==IM_TRUE)?0:1);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HPASSBY, (cfg->horEnable==IM_TRUE)?0:1);

		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_IFSR], scl->regVal[rISP_SCL_IFSR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSR], scl->regVal[rISP_SCL_OFSR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}
	else
	{
		//disable scl csc function
		//SetIspRegister(scl->regVal, ISP_SCL_OFSTR_ISRGB, 1);
		
		/*set scl bypass*/
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VPASSBY, 1);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HPASSBY, 1);

		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}

	//===========================================================================
	//data format 
	//===========================================================================
	//set csc mode
	//not use default value, use mode0
	SetIspRegister(scl->regVal, ISP_SCL_COEF11, csc[0][0]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF12, csc[0][1]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF13, csc[0][2]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF21, csc[0][3]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF22, csc[0][4]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF23, csc[0][5]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF31, csc[0][6]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF32, csc[0][7]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF33, csc[0][8]);
	SetIspRegister(scl->regVal, ISP_SCL_OFFSET_OFFA, csc[0][9]);
	SetIspRegister(scl->regVal, ISP_SCL_OFFSET_OFFB, csc[0][10]);

	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF11], scl->regVal[rISP_SCL_COEF11]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF12], scl->regVal[rISP_SCL_COEF12]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF13], scl->regVal[rISP_SCL_COEF13]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF21], scl->regVal[rISP_SCL_COEF21]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF22], scl->regVal[rISP_SCL_COEF22]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF23], scl->regVal[rISP_SCL_COEF23]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF31], scl->regVal[rISP_SCL_COEF31]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF32], scl->regVal[rISP_SCL_COEF32]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF33], scl->regVal[rISP_SCL_COEF33]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFFSET], scl->regVal[rISP_SCL_OFFSET]));
	
	/*set inter val*/
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_BPP24BL, endianFormat);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_BTSWAP, byteSwap);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_HWSWAP, halfWordSwap);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_ISRGB, isRGB);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_COFMT, coFmt);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_STFMT, stFmt);
	
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSTR], scl->regVal[rISP_SCL_OFSTR]));

	scl->cscMode = ISP_SCL_CSC_MODE_0;
	scl->vrdMode = cfg->vrdMode;
	scl->hrdMode = cfg->hrdMode;
	scl->paramType = ISP_SCL_PARAM_TYPE_0;
	scl->inPut = cfg->inPut;
	scl->outPut = cfg->outPut;
	scl->vscaling = vscaling;
	scl->hscaling = hscaling;
	scl->verEnable = cfg->verEnable;
	scl->horEnable = cfg->horEnable;

	scl->endianFormat = endianFormat;	
	scl->halfWordSwap = halfWordSwap;
	scl->byteSwap = byteSwap;
	scl->isRGB = isRGB;
	scl->coFmt = coFmt;
	scl->stFmt = stFmt;
	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_input

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_input(isp_scl_context_t *scl, scl_input_t *inPut)
{
	IM_UINT32 vscaling;
	IM_UINT32 hscaling;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_))); 
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	IM_ASSERT(inPut != IM_NULL);

	/*check scl input size*/
	if((inPut->inHeight < 0) || (inPut->inHeight > 4096)
		|| (inPut->inWidth < 0) || (inPut->inWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid input size: height=%d, width=%d"),
			inPut->inHeight, inPut->inWidth));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//set scaleing
	if((scl->outPut.outHeight < 1) || (scl->outPut.outHeight > 4096)
		|| (scl->outPut.outWidth < 1) || (scl->outPut.outWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid output size: height=%d, width=%d"),
			scl->outPut.outHeight, scl->outPut.outWidth));
		return IM_RET_INVALID_PARAMETER;
	}
	vscaling = (inPut->inHeight*1024)/scl->outPut.outHeight;
	hscaling = (inPut->inWidth*1024)/scl->outPut.outWidth;
	IM_INFOMSG((IM_STR("scaling comput from input and output size:vscaling=%d,hscaling=%d"),
			vscaling, hscaling));

	if((scl->horEnable == IM_TRUE) || (scl->verEnable == IM_TRUE))
	{
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IVRES, inPut->inHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IHRES, inPut->inWidth-1);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VSCALING, vscaling);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HSCALING, hscaling);
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_IFSR], scl->regVal[rISP_SCL_IFSR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}

	scl->inPut.inWidth = inPut->inWidth;
	scl->inPut.inHeight = inPut->inHeight;
	scl->vscaling = vscaling;
	scl->hscaling = hscaling;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_output

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_output(isp_scl_context_t *scl, scl_output_t *outPut)
{
	IM_UINT32 vscaling;
	IM_UINT32 hscaling;

	IM_UINT32 endianFormat = 0;	
	IM_UINT32 halfWordSwap = 0;
	IM_UINT32 byteSwap = 0;
	IM_UINT32 isRGB = 1;
	IM_UINT32 coFmt = 0;
	IM_UINT32 stFmt = 0;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	IM_ASSERT(outPut != IM_NULL);

	/*check scl output type*/
	if((outPut->outHeight < 1) || (outPut->outHeight > 4096)
		|| (outPut->outWidth < 1) || (outPut->outWidth > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid output size: height=%d, width=%d"),
			outPut->outHeight, outPut->outWidth));
		return IM_RET_INVALID_PARAMETER;
	}
	if((outPut->outFormat != IM_IMAGE_RGB8880) && (outPut->outFormat != IM_IMAGE_RGB0888)
		&& (outPut->outFormat != IM_IMAGE_BGR8880) && (outPut->outFormat != IM_IMAGE_BGR0888)
		&& (outPut->outFormat != IM_IMAGE_YUV420P) && (outPut->outFormat != IM_IMAGE_YUV420SP)
		&& (outPut->outFormat != IM_IMAGE_YUV422P) && (outPut->outFormat != IM_IMAGE_YUV422SP)
		&& (outPut->outFormat != IM_IMAGE_YUV444P))
	{
		IM_ERRMSG((IM_STR("Invalid(not support this)output format:outFormat=%d"),
			outPut->outFormat));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//set scaleing
	vscaling = (scl->inPut.inHeight*1024)/outPut->outHeight;
	hscaling = (scl->inPut.inWidth*1024)/outPut->outWidth;
	IM_INFOMSG((IM_STR("scaling comput from input and output size:vscaling=%d,hscaling=%d"),
			vscaling, hscaling));

	switch(outPut->outFormat)
	{
		case IM_IMAGE_RGB0888:
			endianFormat = 0;	
			halfWordSwap = 0;
			byteSwap = 0;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_RGB8880:
			endianFormat = 1;	
			halfWordSwap = 0;
			byteSwap = 0;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_BGR0888:
			endianFormat = 1;	
			halfWordSwap = 1;
			byteSwap = 1;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_BGR8880:
			endianFormat = 0;	
			halfWordSwap = 1;
			byteSwap = 1;
			isRGB = 1;
			coFmt = 0;
			break;
		case IM_IMAGE_YUV444P:
			isRGB = 0;
			coFmt = 0;
			stFmt = 0;
			scl->horEnable = IM_TRUE;
			scl->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV420P:
			isRGB = 0;
			coFmt = 2;
			stFmt = 0;
			scl->horEnable = IM_TRUE;
			scl->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV420SP:
			isRGB = 0;
			coFmt = 2;
			stFmt = 1;
			scl->horEnable = IM_TRUE;
			scl->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV422P:
			isRGB = 0;
			coFmt = 1;
			stFmt = 0;
			scl->horEnable = IM_TRUE;
			scl->verEnable = IM_TRUE;
			break;
		case IM_IMAGE_YUV422SP:
			isRGB = 0;
			coFmt = 1;
			stFmt = 1;
			scl->horEnable = IM_TRUE;
			scl->verEnable = IM_TRUE;
			break;
		default:
			break;
	}

	if((scl->horEnable == IM_TRUE) || (scl->verEnable == IM_TRUE))
	{
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OVRES, outPut->outHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OHRES, outPut->outWidth-1);
		
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VSCALING, vscaling);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HSCALING, hscaling);

		/*set scl enable*/
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VPASSBY, 0);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HPASSBY, 0);
	
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSR], scl->regVal[rISP_SCL_OFSR]));
		//IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSTR], scl->regVal[rISP_SCL_OFSTR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}


	/*set inter val*/
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_BPP24BL, endianFormat);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_BTSWAP, byteSwap);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_HWSWAP, halfWordSwap);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_ISRGB, isRGB);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_COFMT, coFmt);
	SetIspRegister(scl->regVal, ISP_SCL_OFSTR_STFMT, stFmt);
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSTR], scl->regVal[rISP_SCL_OFSTR]));

	isppwl_memcpy((void *)(&(scl->outPut)), (void *)(outPut), sizeof(scl_output_t));

	scl->outPut.outWidth = outPut->outWidth;
	scl->outPut.outHeight = outPut->outHeight;
	scl->vscaling = vscaling;
	scl->hscaling = hscaling;
	scl->endianFormat = endianFormat;	
	scl->halfWordSwap = halfWordSwap;
	scl->byteSwap = byteSwap;
	scl->isRGB = isRGB;
	scl->coFmt = coFmt;
	scl->stFmt = stFmt;
	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_round_mode(isp_scl_context_t *scl, IM_UINT32 vrdMode, IM_UINT32 hrdMode)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	
	/*check scl round mode*/
	if((vrdMode > ISP_ROUND_NEAREST) || (hrdMode > ISP_ROUND_NEAREST))
	{
		IM_ERRMSG((IM_STR("scl round mode error: vrdMode = %d, hrdMode = %d"), vrdMode, hrdMode));
		return IM_RET_OK;
	}

	if((scl->horEnable == IM_TRUE) || (scl->verEnable == IM_TRUE))
	{
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VROUND, vrdMode);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HROUND, hrdMode);
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}

	scl->vrdMode = vrdMode;
	scl->hrdMode = hrdMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_param_type

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_param_type(isp_scl_context_t *scl, IM_UINT32 paramType)
{
	IM_UINT32 i;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);

	/*set scl param type*/
	if((paramType != ISP_SCL_PARAM_TYPE_0) && (paramType != ISP_SCL_PARAM_TYPE_1))
	{
		IM_ERRMSG((IM_STR("Invalid paramType=%d"), paramType));
		return IM_RET_INVALID_PARAMETER;
	}

	if(scl->paramType == paramType)
	{
		IM_INFOMSG((IM_STR("scl param type is already this type")));
		return IM_RET_OK;
	}
	
	/*set scl param type*/
	for(i=0; i<6*64; i++)
	{
		SetIspRegister(scl->regVal, i+ISP_SCL_HSCR0_0, scl_params[paramType][i]);
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_HSCR0_0+i], scl->regVal[rISP_SCL_HSCR0_0+i]));
	}

	scl->paramType = paramType;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_csc_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_csc_mode(isp_scl_context_t *scl, IM_UINT32 cscMode)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	
	/*check csc cscMode*/
	if((cscMode != ISP_SCL_CSC_MODE_0) && (cscMode != ISP_SCL_CSC_MODE_1))
	{
		IM_ERRMSG((IM_STR("Invalid cscMode=%d"), cscMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cscMode == scl->cscMode)
	{
		IM_INFOMSG((IM_STR("it has been this csc mode")));
		return IM_RET_OK;
	}

	SetIspRegister(scl->regVal, ISP_SCL_COEF11, csc[cscMode][0]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF12, csc[cscMode][1]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF13, csc[cscMode][2]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF21, csc[cscMode][3]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF22, csc[cscMode][4]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF23, csc[cscMode][5]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF31, csc[cscMode][6]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF32, csc[cscMode][7]);
	SetIspRegister(scl->regVal, ISP_SCL_COEF33, csc[cscMode][8]);
	SetIspRegister(scl->regVal, ISP_SCL_OFFSET_OFFA, csc[cscMode][9]);
	SetIspRegister(scl->regVal, ISP_SCL_OFFSET_OFFB, csc[cscMode][10]);

	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF11], scl->regVal[rISP_SCL_COEF11]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF12], scl->regVal[rISP_SCL_COEF12]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF13], scl->regVal[rISP_SCL_COEF13]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF21], scl->regVal[rISP_SCL_COEF21]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF22], scl->regVal[rISP_SCL_COEF22]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF23], scl->regVal[rISP_SCL_COEF23]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF31], scl->regVal[rISP_SCL_COEF31]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF32], scl->regVal[rISP_SCL_COEF32]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_COEF33], scl->regVal[rISP_SCL_COEF33]));
	IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFFSET], scl->regVal[rISP_SCL_OFFSET]));

	scl->cscMode = cscMode;
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: scl_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET scl_set_enable(isp_scl_context_t *scl, IM_BOOL verEnable, IM_BOOL horEnable)
{
	IM_UINT32 i;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(scl != IM_NULL);
	IM_ASSERT(scl->regVal != IM_NULL);
	IM_ASSERT(scl->regOfst != IM_NULL);
	
	/*set scl enable or bypass state*/
	SetIspRegister(scl->regVal, ISP_SCL_SCR_VPASSBY, (verEnable==IM_TRUE)?0:1);
	SetIspRegister(scl->regVal, ISP_SCL_SCR_HPASSBY, (horEnable==IM_TRUE)?0:1);

	if((horEnable == IM_TRUE) || (verEnable == IM_TRUE))
	{
		/*set scl round mode*/
		SetIspRegister(scl->regVal, ISP_SCL_SCR_VROUND, scl->vrdMode);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HROUND, scl->hrdMode);

		/*set scl input size*/
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IVRES, scl->inPut.inHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_IFSR_IHRES, scl->inPut.inWidth-1);


		/*set scl output type*/
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OVRES, scl->outPut.outHeight-1);
		SetIspRegister(scl->regVal, ISP_SCL_OFSR_OHRES, scl->outPut.outWidth-1);
		

		SetIspRegister(scl->regVal, ISP_SCL_SCR_VSCALING, scl->vscaling);
		SetIspRegister(scl->regVal, ISP_SCL_SCR_HSCALING, scl->hscaling);
	
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_IFSR], scl->regVal[rISP_SCL_IFSR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSR], scl->regVal[rISP_SCL_OFSR]));
		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}
	else
	{
		//disable scl csc function
		//SetIspRegister(scl->regVal, ISP_SCL_OFSTR_ISRGB, 1);
		//IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_OFSTR], scl->regVal[rISP_SCL_OFSTR]));

		IM_JIF(isppwl_write_reg(scl->regOfst[rISP_SCL_SCR], scl->regVal[rISP_SCL_SCR]));
	}

	scl->verEnable = verEnable;
	scl->horEnable = horEnable;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

