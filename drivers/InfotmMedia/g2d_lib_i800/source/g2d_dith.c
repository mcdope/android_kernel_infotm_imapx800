/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_dith.c
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <g2d_pwl.h>
#include <g2d_lib.h>
#include "g2d.h"
#include "g2d_reg_drv.h"
#include "g2d_dith.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"DITH_I:"
#define WARNHEAD	"DITH_W:"
#define ERRHEAD		"DITH_E:"
#define TIPHEAD		"DITH_T:"

/****************************************************************************
coef[0]-->coef[35]:
	coef00,coef01,...,coef36, coef37,decoef12,decoef20,decoef21,decoef22
*****************************************************************************/
//for G2D_DITH_COEF_TYPE_0(default coef)
static IM_UINT32 dithMat0[36] = {
0x00, 0x10, 0x08, 0x18,	//coef00, coef01,coef02, coef03
0x02, 0x12, 0x0a, 0x1a,	//coef04, coef05,coef06, coef07
0x0c, 0x1c, 0x04, 0x14,	//coef10, coef11,coef12, coef13
0x0e, 0x1e, 0x06, 0x16,	//coef14, coef15,coef16, coef17
0x03, 0x13, 0x0b, 0x1b,	//coef20, coef21,coef22, coef23
0x01, 0x11, 0x09, 0x19,	//coef24, coef25,coef26, coef27
0x0f, 0x1f, 0x07, 0x17,	//coef30, coef31,coef32, coef33
0x0d, 0x1d, 0x05, 0x15,	//coef34, coef35,coef36, coef37
0x1c, 0x0c, 0x14, 0x04	//decoef12,decoef20,decoef21,decoef22
};

//for G2D_DITH_COEF_TYPE_1
static IM_UINT32 dithMat1[36] = {
0x00, 0x10, 0x08, 0x18,	//coef00, coef01,coef02, coef03
0x02, 0x12, 0x0a, 0x1a,	//coef04, coef05,coef06, coef07
0x0c, 0x1c, 0x04, 0x14,	//coef10, coef11,coef12, coef13
0x0e, 0x1e, 0x06, 0x16,	//coef14, coef15,coef16, coef17
0x03, 0x13, 0x0b, 0x1b,	//coef20, coef21,coef22, coef23
0x01, 0x11, 0x09, 0x19,	//coef24, coef25,coef26, coef27
0x0f, 0x1f, 0x07, 0x17,	//coef30, coef31,coef32, coef33
0x0d, 0x1d, 0x05, 0x15,	//coef34, coef35,coef36, coef37
0x1c, 0x0c, 0x14, 0x04	//decoef12,decoef20,decoef21,decoef22
};

IM_RET dith_set_config(DITHER *dith, G2D_DITHER *dithCfg)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(dith != IM_NULL);
	
	//
	//set init value
	//
	
	/*set size*/
	dith_set_size(dith, dithCfg->width, dithCfg->height);

	/*set channel bits reduce*/
	dith_set_channel_bits_reduce(dith, dithCfg->rChannel, dithCfg->gChannel, dithCfg->bChannel);

	/*set coefType*/
	if(dithCfg->coefType != G2D_DITH_COEF_TYPE_DEFAULT)
		dith_set_coef_type(dith, dithCfg->coefType);
	else
		dith->coefType = G2D_DITH_COEF_TYPE_DEFAULT;

	/*set tempo enable state*/
	dith_set_tempo_enable(dith, dithCfg->tempoEn);

	/*set dither enable state*/
	dith_set_enable(dith, dithCfg->dithEn);
	
	return IM_RET_OK;
}


IM_RET dith_set_size(DITHER *dith, IM_UINT32 width, IM_UINT32 height)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(dith != IM_NULL);

	if((width > 4096) || (height > 4096)){
		IM_ERRMSG((IM_STR("dither size error: width=%d, height=%d"), width, height));
		return IM_RET_FAILED;	
	}
	
	IM_ASSERT(dith->regVal != IM_NULL);

	SetG2DRegister(dith->regVal, G2D_DIT_FRAME_SIZE_HRES, (width-1));
	SetG2DRegister(dith->regVal, G2D_DIT_FRAME_SIZE_VRES, (height-1));

	dith->width = width;
	dith->height = height;
	
	return IM_RET_OK;
}

IM_RET dith_set_channel_bits_reduce(DITHER *dith, IM_UINT32 rChannel, 
													IM_UINT32 gChannel, IM_UINT32 bChannel)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(dith != IM_NULL);

	if((rChannel > 5) || (gChannel > 5) || (bChannel > 5)){
		IM_ERRMSG((IM_STR("dither channel bits reduced error: rch=%d, gch=%d, bch=%d"), rChannel, gChannel, bChannel));
		return IM_RET_FAILED;	
	}
	
	IM_ASSERT(dith->regVal != IM_NULL);

	SetG2DRegister(dith->regVal, G2D_DIT_DI_CNTL_RNB, rChannel);
	SetG2DRegister(dith->regVal, G2D_DIT_DI_CNTL_GNB, gChannel);
	SetG2DRegister(dith->regVal, G2D_DIT_DI_CNTL_BNB, bChannel);

	dith->rChannel = rChannel;
	dith->gChannel = gChannel;
	dith->bChannel = bChannel;
	
	return IM_RET_OK;
}

IM_RET dith_set_coef_type(DITHER *dith, IM_UINT32 coefType)
{
	IM_UINT32 *dithMat;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(dith != IM_NULL);
	
	IM_ASSERT(dith->regVal != IM_NULL);

	if((coefType < G2D_DITH_COEF_TYPE_0) || (coefType > G2D_DITH_COEF_TYPE_LAST)){
		IM_ERRMSG((IM_STR("dither coefType error: coefType=%d"), coefType));
		return IM_RET_FAILED;	
	}

	if(dith->coefType != coefType){
		if(coefType == G2D_DITH_COEF_TYPE_DEFAULT)
			dithMat = dithMat0;
		else //G2D_DITH_COEF_TYPE_LAST
			dithMat = dithMat1;
		
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0003_COEF00, dithMat[0]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0003_COEF01, dithMat[1]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0003_COEF02, dithMat[2]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0003_COEF03, dithMat[3]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0407_COEF04, dithMat[4]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0407_COEF05, dithMat[5]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0407_COEF06, dithMat[6]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_0407_COEF07, dithMat[7]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1013_COEF10, dithMat[8]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1013_COEF11, dithMat[9]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1013_COEF12, dithMat[10]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1013_COEF13, dithMat[11]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1417_COEF14, dithMat[12]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1417_COEF15, dithMat[13]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1417_COEF16, dithMat[14]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_1417_COEF17, dithMat[15]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2023_COEF20, dithMat[16]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2023_COEF21, dithMat[17]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2023_COEF22, dithMat[18]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2023_COEF23, dithMat[19]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2427_COEF24, dithMat[20]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2427_COEF25, dithMat[21]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2427_COEF26, dithMat[22]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_2427_COEF27, dithMat[23]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3033_COEF30, dithMat[24]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3033_COEF31, dithMat[25]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3033_COEF32, dithMat[26]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3033_COEF33, dithMat[27]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3437_COEF34, dithMat[28]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3437_COEF35, dithMat[29]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3437_COEF36, dithMat[30]);
		SetG2DRegister(dith->regVal, G2D_DIT_DM_COEF_3437_COEF37, dithMat[31]);
		SetG2DRegister(dith->regVal, G2D_DIT_DE_COEF_COEF12, dithMat[32]);
		SetG2DRegister(dith->regVal, G2D_DIT_DE_COEF_COEF20, dithMat[33]);
		SetG2DRegister(dith->regVal, G2D_DIT_DE_COEF_COEF21, dithMat[34]);
		SetG2DRegister(dith->regVal, G2D_DIT_DE_COEF_COEF22, dithMat[35]);
	}

	dith->coefType = coefType;
	return IM_RET_OK;
}

IM_RET dith_set_tempo_enable(DITHER *dith, IM_BOOL tempoEn)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(dith != IM_NULL);

	/*set dith temporal enable*/
	IM_ASSERT(dith->regVal != IM_NULL);

	SetG2DRegister(dith->regVal, G2D_DIT_DI_CNTL_TEMPO, (tempoEn==IM_TRUE)?1:0);

	dith->tempoEn = tempoEn;
	
	return IM_RET_OK;

}

IM_RET dith_set_enable(DITHER *dith, IM_BOOL dithEn)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(dith != IM_NULL);
	IM_ASSERT(dith->regVal != IM_NULL);

	SetG2DRegister(dith->regVal, G2D_DIT_DI_CNTL_BYPASS, (dithEn==IM_TRUE)?0:1);

	dith->dithEn = dithEn;
	
	return IM_RET_OK;
}



  
