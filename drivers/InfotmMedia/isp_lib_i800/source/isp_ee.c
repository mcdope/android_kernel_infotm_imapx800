/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ee.c
--
--  Description :
--		ee module.
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
#include "isp_ee.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"EE_I:"
#define WARNHEAD	"EE_W:"
#define ERRHEAD		"EE_E:"
#define TIPHEAD		"EE_T:"

/*{coef00,coef01,coef02,
coef10,coef11,coef12,
coef20,coef21,coef22}*/

#if 1	//default edge operator value

static const IM_INT32 HMat[9] = {
-1, -2, -1,
 0,  0,  0,
 1,  2,  1
};

static const IM_INT32 VMat[9] = {
-1, 0, 1,
-2, 0, 2,
-1, 0, 1
};

static const IM_INT32 D0Mat[9] = {
-2, -1, 0,
-1,  0, 1,
 0,  1, 2
};

static const IM_INT32 D1Mat[9] = {
 0, -1, -2,
 1,  0, -1,
 2,  1,  0
};
#endif

/*maybe IM_UINT32*/
static const IM_INT32 GASMat[][9] = {
{2,  4,  2,  4,  8,  4,  2,  4,  2},
{0,  4,  0,  4, 16,  4,  0,  4,  0},
};


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Function name: ee_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_init(isp_ee_context_t *ee, isp_ee_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check ee coefw and coefa */
	if((cfg->coefw < 0) || (cfg->coefw > 255)
		|| (cfg->coefa < 0) || (cfg->coefa > 255))
	{
		IM_ERRMSG((IM_STR("Invalid coef: coefw=%d, coefa=%d"), cfg->coefw, cfg->coefa));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check ee round mode */
	if(cfg->rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("ee round mode error: rdMode = %d"), cfg->rdMode));
		return IM_RET_OK;
	}

	/* check gasMode*/
	if(cfg->gasMode > ISP_EE_GAUSS_MODE_1)
	{
		IM_ERRMSG((IM_STR("ee gauss mode error: gasMode = %d"), cfg->gasMode));
		return IM_RET_OK;
	}

	/* check ee error threshold*/
	if((cfg->errTh< 0) || (cfg->errTh > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid error threshold: errTh=%d"), cfg->errTh));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*check ee detect threshold thrMat */
	if((cfg->thrMat.hTh < 0) || (cfg->thrMat.hTh > 255)
		|| (cfg->thrMat.vTh < 0) || (cfg->thrMat.vTh > 255)
		|| (cfg->thrMat.d0Th < 0) || (cfg->thrMat.d0Th > 255)
		|| (cfg->thrMat.d1Th < 0) || (cfg->thrMat.d1Th > 255))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: hTh=%d, vTh=%d, d0Th=%d, d1Th=%d"), 
			cfg->thrMat.hTh, cfg->thrMat.vTh, cfg->thrMat.d0Th, cfg->thrMat.d1Th));
		return IM_RET_INVALID_PARAMETER;
	}

	/*set ee gauss operator mode*/
	if(cfg->gasMode != ISP_EE_GAUSS_MODE_0)//default mode
	{
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM00, GASMat[cfg->gasMode][0]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM01, GASMat[cfg->gasMode][1]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM02, GASMat[cfg->gasMode][2]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM10, GASMat[cfg->gasMode][3]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM11, GASMat[cfg->gasMode][4]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM12, GASMat[cfg->gasMode][5]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM20, GASMat[cfg->gasMode][6]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM21, GASMat[cfg->gasMode][7]);
		SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM22, GASMat[cfg->gasMode][8]);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT4], ee->regVal[rISP_EE_MAT4]));
	}

	if(cfg->enable == IM_TRUE)
	{
		/*set ee coefw and coefa */
		SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFW, cfg->coefw);
		SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFA, cfg->coefa);
		
		/*set ee round mode */
		SetIspRegister(ee->regVal, ISP_EE_CNTL_EEROUND, cfg->rdMode);
		
		/*set ee gauss filter for flat region enable*/
		SetIspRegister(ee->regVal, ISP_EE_CNTL_EEDNOEN, (cfg->gasEn==IM_TRUE)?1:0);

		/*set ee error threshold*/
		SetIspRegister(ee->regVal, ISP_EE_THE, cfg->errTh);

		/*set ee detect threshold thrMat */
		SetIspRegister(ee->regVal, ISP_EE_TH_EH, cfg->thrMat.hTh);
		SetIspRegister(ee->regVal, ISP_EE_TH_EV, cfg->thrMat.vTh);
		SetIspRegister(ee->regVal, ISP_EE_TH_ED0, cfg->thrMat.d0Th);
		SetIspRegister(ee->regVal, ISP_EE_TH_ED1, cfg->thrMat.d1Th);

		/*set ee operator thrMat */ 
		//use default value

		/*set ee enable*/
		SetIspRegister(ee->regVal, ISP_GLB_CFG2_EEBYPASS, 0);

		//isppwl_write_regs();
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
		//IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT0], ee->regVal[rISP_EE_MAT0]));
		//IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT1], ee->regVal[rISP_EE_MAT1]));
		//IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT2], ee->regVal[rISP_EE_MAT2]));
		//IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT3], ee->regVal[rISP_EE_MAT3]));
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_THE], ee->regVal[rISP_EE_THE]));
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_TH], ee->regVal[rISP_EE_TH]));
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_GLB_CFG2], ee->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set ee enable or bypass state*/
		SetIspRegister(ee->regVal, ISP_GLB_CFG2_EEBYPASS, 1);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_GLB_CFG2], ee->regVal[rISP_GLB_CFG2]));
	}

	ee->coefw = cfg->coefw;
	ee->coefa = cfg->coefa;
	ee->rdMode = cfg->rdMode;
	ee->gasEn= cfg->gasEn;
	ee->gasMode = cfg->gasMode;
	ee->errTh = cfg->errTh;
	ee->thrMat.hTh = cfg->thrMat.hTh;
	ee->thrMat.vTh = cfg->thrMat.vTh;
	ee->thrMat.d0Th = cfg->thrMat.d0Th;
	ee->thrMat.d1Th = cfg->thrMat.d1Th;
	ee->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_coefw

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_coefw(isp_ee_context_t *ee, IM_UINT32 coefw)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	/*check ee coefw*/
	if((coefw < 0) || (coefw > 255))
	{
		IM_ERRMSG((IM_STR("Invalid coef: coefw=%d"), coefw));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(ee->enable == IM_TRUE)
	{
		/*set ee coefw*/
		SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFW, coefw);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
	}

	ee->coefw = coefw;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_coefa

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_coefa(isp_ee_context_t *ee, IM_UINT32 coefa)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	/*check ee coefw and coefa */
	if((coefa < 0) || (coefa > 255))
	{
		IM_ERRMSG((IM_STR("Invalid coef: coefa=%d"), coefa));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(ee->enable == IM_TRUE)
	{
		/*set ee coefa*/
		SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFA, coefa);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
	}

	ee->coefa = coefa;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_round_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_round_mode(isp_ee_context_t *ee, IM_UINT32 rdMode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	/*check ee round mode */
	if(rdMode > ISP_ROUND_NEAREST)
	{
		IM_ERRMSG((IM_STR("ee round mode error: rdMode = %d"), rdMode));
		return IM_RET_OK;
	}

	if(ee->rdMode == rdMode)
	{
		IM_INFOMSG((IM_STR("ee round mode has already been this mode")));
		return IM_RET_OK;	
	}
	
	/*set ee round mode */
	if(ee->enable == IM_TRUE)
	{
		SetIspRegister(ee->regVal, ISP_EE_CNTL_EEROUND, rdMode);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
	}

	ee->rdMode = rdMode;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_gauss_filter_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_gauss_filter_enable(isp_ee_context_t *ee, IM_BOOL gasEn)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	if(ee->gasEn == gasEn)
	{
		IM_INFOMSG((IM_STR("ee gauss filter for flat region has already been this state")));
		return IM_RET_OK;	
	}
	
	/*set ee gauss filter for flat region enable*/
	if(ee->enable == IM_TRUE)
	{
		SetIspRegister(ee->regVal, ISP_EE_CNTL_EEDNOEN, (gasEn==IM_TRUE)?1:0);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
	}

	ee->gasEn= gasEn;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_gauss_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_gauss_mode(isp_ee_context_t *ee, IM_UINT32 gasMode)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	/* check gasMode*/
	if(gasMode > ISP_EE_GAUSS_MODE_1)
	{
		IM_ERRMSG((IM_STR("ee gauss mode error: gasMode = %d"), gasMode));
		return IM_RET_OK;
	}

	if(ee->gasMode == gasMode)
	{
		IM_INFOMSG((IM_STR("ee gauss operator thrMat has already been this mode")));
		return IM_RET_OK;	
	}

	/*set ee gauss operator mode*/
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM00, GASMat[gasMode][0]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM01, GASMat[gasMode][1]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM02, GASMat[gasMode][2]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM10, GASMat[gasMode][3]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM11, GASMat[gasMode][4]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM12, GASMat[gasMode][5]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM20, GASMat[gasMode][6]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM21, GASMat[gasMode][7]);
	SetIspRegister(ee->regVal, ISP_EE_MAT4_GAM22, GASMat[gasMode][8]);
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT4], ee->regVal[rISP_EE_MAT4]));

	ee->gasMode = gasMode;	
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_error_threshold

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_error_threshold(isp_ee_context_t *ee, IM_UINT32 errTh)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	/* check ee error threshold*/
	if((errTh< 0) || (errTh > 4095))
	{
		IM_ERRMSG((IM_STR("Invalid error threshold: errTh=%d"), errTh));
		return IM_RET_INVALID_PARAMETER;
	}
	
	/*set ee error threshold*/
	if(ee->enable == IM_TRUE)
	{
		SetIspRegister(ee->regVal, ISP_EE_THE, errTh);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_THE], ee->regVal[rISP_EE_THE]));
	}

	ee->errTh = errTh;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_detect_threshold_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_detect_threshold_matrix(isp_ee_context_t *ee, isp_ee_thr_matrix *thrMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);
	IM_ASSERT(thrMat != IM_NULL);

	/*set ee detect threshold thrMat */
	if((thrMat->hTh < 0) || (thrMat->hTh > 255)
		|| (thrMat->vTh < 0) || (thrMat->vTh > 255)
		|| (thrMat->d0Th < 0) || (thrMat->d0Th > 255)
		|| (thrMat->d1Th < 0) || (thrMat->d1Th > 255))
	{
		IM_ERRMSG((IM_STR("Invalid thrMat value: hTh=%d, vTh=%d, d0Th=%d, d1Th=%d"), 
			thrMat->hTh, thrMat->vTh, thrMat->d0Th, thrMat->d1Th));
		return IM_RET_INVALID_PARAMETER;
	}

	if(ee->enable == IM_TRUE)
	{
		SetIspRegister(ee->regVal, ISP_EE_TH_EH, thrMat->hTh);
		SetIspRegister(ee->regVal, ISP_EE_TH_EV, thrMat->vTh);
		SetIspRegister(ee->regVal, ISP_EE_TH_ED0, thrMat->d0Th);
		SetIspRegister(ee->regVal, ISP_EE_TH_ED1, thrMat->d1Th);
		IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_TH], ee->regVal[rISP_EE_TH]));
	}

	isppwl_memcpy((void*)(&ee->thrMat), (void *)(thrMat), sizeof(isp_ee_thr_matrix));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_edge_operator_matrix

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_edge_operator_matrix(isp_ee_context_t *ee, IM_UINT32 direction, isp_ee_op_matrix *opMat)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);
	IM_ASSERT(opMat != IM_NULL);
	
	/*set ee operator matrix */
	if(direction > ISP_EE_DIRECTION_D1M)
	{
		IM_ERRMSG((IM_STR("Invalid direction: direction=%d"), direction));
		return IM_RET_INVALID_PARAMETER;
	}
		
	switch(direction)
	{
		case ISP_EE_DIRECTION_HM:
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM00, opMat->coef00);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM01, opMat->coef01);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM02, opMat->coef02);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM10, opMat->coef10);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM11, opMat->coef11);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM12, opMat->coef12);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM20, opMat->coef20);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM21, opMat->coef21);
			SetIspRegister(ee->regVal, ISP_EE_MAT0_HM22, opMat->coef22);
			IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT0], ee->regVal[rISP_EE_MAT0]));
			//isppwl_memcpy((void*)(&ee->hm), (void *)(opMat), sizeof(isp_ee_op_matrix));
			break;
		case ISP_EE_DIRECTION_VM:
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM00, opMat->coef00);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM01, opMat->coef01);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM02, opMat->coef02);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM10, opMat->coef10);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM11, opMat->coef11);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM12, opMat->coef12);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM20, opMat->coef20);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM21, opMat->coef21);
			SetIspRegister(ee->regVal, ISP_EE_MAT1_VM22, opMat->coef22);
			IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT1], ee->regVal[rISP_EE_MAT1]));
			//isppwl_memcpy((void*)(&ee->vm), (void *)(opMat), sizeof(isp_ee_op_matrix));
			break;
		case ISP_EE_DIRECTION_D0M:
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M00, opMat->coef00);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M01, opMat->coef01);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M02, opMat->coef02);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M10, opMat->coef10);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M11, opMat->coef11);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M12, opMat->coef12);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M20, opMat->coef20);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M21, opMat->coef21);
			SetIspRegister(ee->regVal, ISP_EE_MAT2_D0M22, opMat->coef22);
			IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT2], ee->regVal[rISP_EE_MAT2]));
			//isppwl_memcpy((void*)(&ee->d0m), (void *)(opMat), sizeof(isp_ee_op_matrix));
			break;
		default: //ISP_EE_DIRECTION_D1M
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M00, opMat->coef00);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M01, opMat->coef01);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M02, opMat->coef02);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M10, opMat->coef10);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M11, opMat->coef11);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M12, opMat->coef12);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M20, opMat->coef20);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M21, opMat->coef21);
			SetIspRegister(ee->regVal, ISP_EE_MAT3_D1M22, opMat->coef22);
			IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_MAT3], ee->regVal[rISP_EE_MAT3]));
			//isppwl_memcpy((void*)(&ee->d1m), (void *)(opMat), sizeof(isp_ee_op_matrix));
			break;	
	}

	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

  Function name: ee_set_enable

  Functional description:

  Inputs:

  Outputs:

  Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_enable(isp_ee_context_t *ee)
{ 
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	if(ee->enable == IM_TRUE)
	{
		IM_WARNMSG((IM_STR("ee has been enabled")));
		return IM_RET_OK;	
	}
	//enable ee 
	/*set ee coefw and coefa */
	SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFW, ee->coefw);
	SetIspRegister(ee->regVal, ISP_EE_CNTL_COEFA, ee->coefa);
	
	/*set ee round mode */
	SetIspRegister(ee->regVal, ISP_EE_CNTL_EEROUND, ee->rdMode);
	
	/*set ee gauss filter for flat region enable*/
	SetIspRegister(ee->regVal, ISP_EE_CNTL_EEDNOEN, (ee->gasEn==IM_TRUE)?1:0);

	/*set ee error threshold*/
	SetIspRegister(ee->regVal, ISP_EE_THE, ee->errTh);

	/*set ee detect threshold matrix */
	SetIspRegister(ee->regVal, ISP_EE_TH_EH, ee->thrMat.hTh);
	SetIspRegister(ee->regVal, ISP_EE_TH_EV, ee->thrMat.vTh);
	SetIspRegister(ee->regVal, ISP_EE_TH_ED0, ee->thrMat.d0Th);
	SetIspRegister(ee->regVal, ISP_EE_TH_ED1, ee->thrMat.d1Th);

	/*set ee enable*/
	SetIspRegister(ee->regVal, ISP_GLB_CFG2_EEBYPASS, 0);

	//isppwl_write_regs();
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_CNTL], ee->regVal[rISP_EE_CNTL]));
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_THE], ee->regVal[rISP_EE_THE]));
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_EE_TH], ee->regVal[rISP_EE_TH]));
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_GLB_CFG2], ee->regVal[rISP_GLB_CFG2]));

	ee->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: ee_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET ee_set_disable(isp_ee_context_t *ee)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(ee != IM_NULL);
	IM_ASSERT(ee->regVal != IM_NULL);
	IM_ASSERT(ee->regOfst != IM_NULL);

	if(ee->enable != IM_TRUE)
	{
		IM_WARNMSG((IM_STR("ee has been disabled")));
		return IM_RET_OK;	
	}
	//disable ee
	SetIspRegister(ee->regVal, ISP_GLB_CFG2_EEBYPASS, 1);
	IM_JIF(isppwl_write_reg(ee->regOfst[rISP_GLB_CFG2], ee->regVal[rISP_GLB_CFG2]));

	ee->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
