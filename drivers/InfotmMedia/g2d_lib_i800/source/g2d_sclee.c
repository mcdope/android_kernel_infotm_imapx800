/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_sclee.c
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
#include "g2d_sclee.h"

#ifdef _ANDROID_
#define LOG_TAG "SCL"
#endif

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"SCL_I:"
#define WARNHEAD	"SCL_W:"
#define ERRHEAD		"SCL_E:"
#define TIPHEAD		"SCL_T:"


static const IM_UINT32 scl_params[6*64] = { 
#include "g2d_scl_params_table.h"
};


/*{coef00,coef01,coef02,coef10,coef11,coef12,coef20,coef21,coef22}*/
//for G2D_EE_OPMAT_TYPE_0(use default coef)
static IM_INT32 OpMat0[4*9] = {
-1, -2, -1,  0,  0,  0,  1,  2,  1, //HMat
-1,  0,  1, -2,  0,  2, -1,  0,  1, //VMat
-2, -1,  0, -1,  0,  1,  0,  1,  2,	//D0Mat
 0, -1, -2,  1,  0, -1,  2,  1,  0	//D1Mat
};

//for G2D_EE_OPMAT_TYPE_1
static IM_INT32 OpMat1[4*9] = {
-1, -2, -1,  0,  0,  0,  1,  2,  1, //HMat
-1,  0,  1, -2,  0,  2, -1,  0,  1, //VMat
-2, -1,  0, -1,  0,  1,  0,  1,  2,	//D0Mat
 0, -1, -2,  1,  0, -1,  2,  1,  0	//D1Mat
};


/*maybe IM_UINT32*/
//for G2D_EE_GAUSS_MODE_0(use default mode)
static const IM_INT32 GASMat0[9] = {
 2,  4,  2,
 4,  8,  4,
 2,  4,  2
};

//for G2D_EE_GAUSS_MODE_1
static const IM_INT32 GASMat1[9] = {
 4,  4,  4,
 4,  0,  4,
 4,  4,  4
};

//for G2D_EE_GAUSS_MODE_2
static const IM_INT32 GASMat2[9] = {
 1,  4,  1,
 4, 12,  4,
 1,  4,  1
};


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: sclee_set_config

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_config(SCLEE *sclee, G2D_SCLEE *scleeCfg)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	//
	//set init value
	//
	
	/*set scl size*/
	sclee_set_scl_size(sclee, scleeCfg->inWidth, scleeCfg->inHeight, scleeCfg->outWidth, scleeCfg->outHeight);

	/*set scl round mode*/
	sclee_set_scl_round_mode(sclee, scleeCfg->vrdMode, scleeCfg->hrdMode);

	/*set scl paramType*/
	if(scleeCfg->paramType != G2D_SCL_PARAM_TYPE_DEFAULT)
		sclee_set_scl_param_type(sclee, scleeCfg->paramType);
	else
		sclee->paramType = G2D_SCL_PARAM_TYPE_DEFAULT;

	/*set scl enable state*/
	sclee_set_scl_enable(sclee, scleeCfg->verEnable, scleeCfg->horEnable);

	/*set scl_ee order*/
	sclee_set_order(sclee, scleeCfg->order);

	/*set ee coefw*/
	sclee_set_ee_coefw(sclee, scleeCfg->coefw);

	/*set ee coefa*/
	sclee_set_ee_coefa(sclee, scleeCfg->coefa);

	/*set ee round mode*/
	sclee_set_ee_round_mode(sclee, scleeCfg->rdMode);

	/*set ee gauss mode*/
	if(scleeCfg->gasMode != G2D_EE_GAUSS_MODE_DEFAULT)
		sclee_set_ee_gauss_mode(sclee, scleeCfg->gasMode);
	else
		sclee->gasMode = G2D_EE_GAUSS_MODE_DEFAULT;

	/*set ee operator mat*/
	if(scleeCfg->opMatType != G2D_EE_OPMAT_TYPE_DEFAULT)
		sclee_set_ee_operator_type(sclee, scleeCfg->opMatType);
	else
		sclee->opMatType = G2D_EE_OPMAT_TYPE_DEFAULT;

	/*set ee error threshold*/
	sclee_set_ee_error_threshold(sclee, scleeCfg->errTh);

	/*set ee detect threshold*/
	sclee_set_ee_detect_threshold_matrix(sclee, &(scleeCfg->thrMat));

	/*set ee denoise enable state*/
	sclee_set_ee_denoise_enable(sclee, scleeCfg->denosEnable);

	/*set ee enable state*/
	sclee_set_ee_enable(sclee, scleeCfg->eeEnable);
		
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_scl_size

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_scl_size(SCLEE *sclee, IM_UINT32 inWidth, IM_UINT32 inHeight,
											IM_UINT32 outWidth, IM_UINT32 outHeight)
{
	IM_UINT32 vscaling;
	IM_UINT32 hscaling;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_))); 
  
	IM_ASSERT(sclee != IM_NULL);
	
	IM_ASSERT(sclee->regVal != IM_NULL);

	/*check scl size*/
	if((inHeight <= 0) || (inHeight > 4096)
		|| (inWidth <= 0) || (inWidth > 4096)
		|| (outHeight <= 0) || (outHeight > 4096)
		|| (outWidth <= 0) || (outWidth > 4096)){
		IM_ERRMSG((IM_STR("Invalid size: inHeight=%d, inWidth=%d, outHeight=%d, outWidth=%d"),
			inHeight, inWidth, outHeight, outWidth));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//set scaleing
	vscaling = (inHeight*1024)/outHeight;
	hscaling = (inWidth*1024)/outWidth;
	IM_INFOMSG((IM_STR("scaling comput from input and output size:vscaling=%d,hscaling=%d"),
				vscaling, hscaling));

	SetG2DRegister(sclee->regVal, G2D_SCL_IFSR_IVRES, inHeight-1);
	SetG2DRegister(sclee->regVal, G2D_SCL_IFSR_IHRES, inWidth-1);
	SetG2DRegister(sclee->regVal, G2D_SCL_OFSR_OVRES, outHeight-1);
	SetG2DRegister(sclee->regVal, G2D_SCL_OFSR_OHRES, outWidth-1);
	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_VSCALING, vscaling);
	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_HSCALING, hscaling);

	sclee->inHeight = inHeight;
	sclee->inWidth = inWidth;
	sclee->outHeight = outHeight;
	sclee->outWidth = outWidth;
	sclee->vscaling = vscaling;
	sclee->hscaling = hscaling;
	
	return IM_RET_OK;
}


/*------------------------------------------------------------------------------

    Function name: sclee_set_scl_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_scl_round_mode(SCLEE *sclee, IM_UINT32 vrdMode, IM_UINT32 hrdMode)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(sclee != IM_NULL);
	IM_ASSERT(sclee->regVal != IM_NULL);
	
	/*check scl round mode*/
	IM_ASSERT((vrdMode == G2D_ROUND_MINUS) || (vrdMode == G2D_ROUND_NEAREST));

	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_VROUND, vrdMode);
	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_HROUND, hrdMode);

	sclee->vrdMode = vrdMode;
	sclee->hrdMode = hrdMode;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_scl_param_type

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_scl_param_type(SCLEE *sclee, IM_UINT32 paramType)
{
	IM_UINT32 i;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(sclee != IM_NULL);
	
	IM_ASSERT(sclee->regVal != IM_NULL);

	/*set scl param type*/
	if((paramType < G2D_SCL_PARAM_TYPE_0) || (paramType > G2D_SCL_PARAM_TYPE_LAST)){
		IM_ERRMSG((IM_STR("scl paramType error: paramType=%d"), paramType));
		return IM_RET_FAILED;	
	}
	
	if(sclee->paramType != paramType){
		/*set scl param type*/
		if(paramType == G2D_SCL_PARAM_TYPE_0){ 	//default type
			for(i=0; i<6*64; i++){
				SetG2DRegister(sclee->regVal, i+G2D_SCL_HSCR0_0, scl_params[i]);
			}
		}else{ /*if(paramType == G2D_SCL_PARAM_TYPE_1)*/
			/*h0` = 0; h1` = v0; h2` = v1; h3` = 0; v0` = v0; v1` = v1; */
			for(i=0; i<64; i++){
				SetG2DRegister(sclee->regVal, i+G2D_SCL_HSCR0_0, 0);
				SetG2DRegister(sclee->regVal, i+G2D_SCL_HSCR3_0, 0);
				SetG2DRegister(sclee->regVal, i+G2D_SCL_HSCR1_0, scl_params[i+256]);
				SetG2DRegister(sclee->regVal, i+G2D_SCL_HSCR2_0, scl_params[i+320]);
				SetG2DRegister(sclee->regVal, i+G2D_SCL_VSCR0_0, scl_params[i+256]);
				SetG2DRegister(sclee->regVal, i+G2D_SCL_VSCR1_0, scl_params[i+320]);
			}
		}
	}

	sclee->paramType = paramType;
	return IM_RET_OK;
}


/*------------------------------------------------------------------------------

    Function name: sclee_set_scl_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_scl_enable(SCLEE *sclee, IM_BOOL verEnable, IM_BOOL horEnable)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
  
	IM_ASSERT(sclee != IM_NULL);
	
	IM_ASSERT(sclee->regVal != IM_NULL);
	
	/*set scl enable or bypass state*/
	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_VPASSBY, (verEnable==IM_TRUE)?0:1);
	SetG2DRegister(sclee->regVal, G2D_SCL_SCR_HPASSBY, (horEnable==IM_TRUE)?0:1);

	sclee->verEnable = verEnable;
	sclee->horEnable = horEnable;
	
	return IM_RET_OK;
}



/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_coefw

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_order(SCLEE *sclee, IM_UINT32 order)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	IM_ASSERT((order == G2D_ORDER_SCLEE) || (order == G2D_ORDER_EESCL));

	if(sclee->order == order){
		IM_INFOMSG((IM_STR("ee round mode has already been this mode")));
		return IM_RET_OK;	
	}
	
	/*set scl ee order */
	IM_ASSERT(sclee->regVal != IM_NULL);
	
	SetG2DRegister(sclee->regVal, G2D_EE_CNTL_FIRST, order);
	
	sclee->order = order;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_coefw

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_coefw(SCLEE *sclee, IM_UINT32 coefw)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	
	IM_ASSERT(sclee->regVal != IM_NULL);

	/*check ee coefw*/
	if((coefw < 0) || (coefw > 255)){
		IM_ERRMSG((IM_STR("Invalid coef: coefw=%d"), coefw));
		return IM_RET_INVALID_PARAMETER;
	}

	/*set ee coefw*/
	SetG2DRegister(sclee->regVal, G2D_EE_COEF_COEFW, coefw);

	sclee->coefw = coefw;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_coefa

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_coefa(SCLEE *sclee, IM_UINT32 coefa)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	
	IM_ASSERT(sclee->regVal != IM_NULL);

	/*check ee coefw and coefa */
	if((coefa < 0) || (coefa > 255)){
		IM_ERRMSG((IM_STR("Invalid coef: coefa=%d"), coefa));
		return IM_RET_INVALID_PARAMETER;
	}

	/*set ee coefa*/
	SetG2DRegister(sclee->regVal, G2D_EE_COEF_COEFA, coefa);

	sclee->coefa = coefa;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_round_mode(SCLEE *sclee, IM_UINT32 rdMode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	IM_ASSERT((rdMode == G2D_ROUND_MINUS) || (rdMode == G2D_ROUND_NEAREST));

	if(sclee->rdMode == rdMode){
		IM_INFOMSG((IM_STR("ee round mode has already been this mode")));
		return IM_RET_OK;	
	}
	
	/*set ee round mode */
	IM_ASSERT(sclee->regVal != IM_NULL);

	SetG2DRegister(sclee->regVal, G2D_EE_CNTL_ROUND, rdMode);

	sclee->rdMode = rdMode;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_gauss_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_gauss_mode(SCLEE *sclee, IM_UINT32 gasMode)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	
	/*set ee gauss operator matrix mode*/
	IM_ASSERT(sclee->regVal != IM_NULL);

	if((gasMode < G2D_EE_GAUSS_MODE_0) || (gasMode > G2D_EE_GAUSS_MODE_LAST)){
		IM_ERRMSG((IM_STR("ee gaussMode error: gasMode=%d"), gasMode));
		return IM_RET_FAILED;	
	}

	if(sclee->gasMode != gasMode){
		if(gasMode == G2D_EE_GAUSS_MODE_0){	//default gauss matrix coef
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM00, GASMat0[0]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM01, GASMat0[1]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM02, GASMat0[2]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM10, GASMat0[3]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_HIGH, ((GASMat0[4]&0x8)>>3));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_LOW, (GASMat0[4]&0x7));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM12, GASMat0[5]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM20, GASMat0[6]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM21, GASMat0[7]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM22, GASMat0[8]);
		}else if(gasMode == G2D_EE_GAUSS_MODE_1){
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM00, GASMat1[0]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM01, GASMat1[1]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM02, GASMat1[2]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM10, GASMat1[3]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_HIGH, ((GASMat1[4]&0x8)>>3));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_LOW, (GASMat1[4]&0x7));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM12, GASMat1[5]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM20, GASMat1[6]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM21, GASMat1[7]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM22, GASMat1[8]);
		}else if(gasMode == G2D_EE_GAUSS_MODE_2){
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM00, GASMat2[0]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM01, GASMat2[1]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM02, GASMat2[2]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM10, GASMat2[3]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_HIGH, ((GASMat2[4]&0x8)>>3));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM11_LOW, (GASMat2[4]&0x7));
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM12, GASMat2[5]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM20, GASMat2[6]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM21, GASMat2[7]);
			SetG2DRegister(sclee->regVal, G2D_EE_MAT4_GAM22, GASMat2[8]);
		}
	}

	sclee->gasMode = gasMode;	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_detect_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_operator_type(SCLEE *sclee, IM_UINT32 opMatType)
{
	IM_UINT32 *opMat;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	
	/*set ee gauss operator matrix mode*/
	IM_ASSERT(sclee->regVal != IM_NULL);

	if((opMatType < G2D_EE_OPMAT_TYPE_0) || (opMatType > G2D_EE_OPMAT_TYPE_LAST)){
		IM_ERRMSG((IM_STR("ee opMatType error: opMatType=%d"), opMatType));
		return IM_RET_FAILED;	
	}

	if(sclee->opMatType != opMatType){
		if(opMatType == G2D_EE_OPMAT_TYPE_0)	//default gauss matrix coef
			opMat = OpMat0;
		else/*if(opMatType == G2D_EE_OPMAT_TYPE_1)*/
			opMat = OpMat1;

		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM00, opMat[0]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM01, opMat[1]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM02, opMat[2]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM10, opMat[3]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM11, opMat[4]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM12, opMat[5]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM20, opMat[6]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM21, opMat[7]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT0_HM22, opMat[8]);
		
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM00, opMat[9]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM01, opMat[10]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM02, opMat[11]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM10, opMat[12]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM11, opMat[13]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM12, opMat[14]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM20, opMat[15]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM21, opMat[16]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT1_VM22, opMat[17]);
		
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M00, opMat[18]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M01, opMat[19]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M02, opMat[20]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M10, opMat[21]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M11, opMat[22]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M12, opMat[23]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M20, opMat[24]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M21, opMat[25]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT2_D0M22, opMat[26]);

		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M00, opMat[27]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M01, opMat[28]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M02, opMat[29]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M10, opMat[30]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M11, opMat[31]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M12, opMat[32]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M20, opMat[33]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M21, opMat[34]);
		SetG2DRegister(sclee->regVal, G2D_EE_MAT3_D1M22, opMat[35]);
	}

	sclee->opMatType = opMatType;	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_error_threshold

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_error_threshold(SCLEE *sclee, IM_UINT32 errTh)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);

	/* check ee error threshold*/
	if((errTh< 0) || (errTh > 4095)){
		IM_ERRMSG((IM_STR("Invalid error threshold: errTh=%d"), errTh));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*set ee error threshold*/
	IM_ASSERT(sclee->regVal != IM_NULL);

	SetG2DRegister(sclee->regVal, G2D_EE_COEF_ERR, errTh);

	sclee->errTh = errTh;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_detect_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_detect_threshold_matrix(SCLEE *sclee, G2D_EE_TH_MATRIX *matrix)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);
	IM_ASSERT(matrix != IM_NULL);

	/*set ee detect threshold matrix */
	IM_ASSERT(sclee->regVal != IM_NULL);
	
	if((matrix->hTh < 0) || (matrix->hTh > 255)
		|| (matrix->vTh < 0) || (matrix->vTh > 255)
		|| (matrix->d0Th < 0) || (matrix->d0Th > 255)
		|| (matrix->d1Th < 0) || (matrix->d1Th > 255)){
		IM_ERRMSG((IM_STR("Invalid matrix value: hTh=%d, vTh=%d, d0Th=%d, d1Th=%d"), 
			matrix->hTh, matrix->vTh, matrix->d0Th, matrix->d1Th));
		return IM_RET_INVALID_PARAMETER;
	}

	SetG2DRegister(sclee->regVal, G2D_EE_THRE_H, matrix->hTh);
	SetG2DRegister(sclee->regVal, G2D_EE_THRE_V, matrix->vTh);
	SetG2DRegister(sclee->regVal, G2D_EE_THRE_D0, matrix->d0Th);
	SetG2DRegister(sclee->regVal, G2D_EE_THRE_D1, matrix->d1Th);

	g2dpwl_memcpy((void*)(&sclee->thrMat), (void *)(matrix), sizeof(G2D_EE_TH_MATRIX));

	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_denoise_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_denoise_enable(SCLEE *sclee, IM_BOOL denosEnable)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(sclee != IM_NULL);

	if(sclee->denosEnable == denosEnable){
		IM_INFOMSG((IM_STR("ee gauss filter for flat region has already been this state")));
		return IM_RET_OK;	
	}
	
	/*set ee gauss filter for flat region enable*/
	IM_ASSERT(sclee->regVal != IM_NULL);

	SetG2DRegister(sclee->regVal, G2D_EE_CNTL_DENOISE_EN, (denosEnable==IM_TRUE)?1:0);

	sclee->denosEnable= denosEnable;
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: sclee_set_ee_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET sclee_set_ee_enable(SCLEE *sclee, IM_BOOL eeEnable)
{ 
	IM_ASSERT(sclee != IM_NULL);

	IM_ASSERT(sclee->regVal != IM_NULL);

 	IM_INFOMSG((IM_STR("eeBypass=%d"), (eeEnable==IM_TRUE)?0:1));
	
	SetG2DRegister(sclee->regVal, G2D_EE_CNTL_BYPASS, (eeEnable==IM_TRUE)?0:1);

	sclee->eeEnable = eeEnable;
	
	return IM_RET_OK;
}


