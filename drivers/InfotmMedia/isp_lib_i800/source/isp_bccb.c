/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: bccb_lib.c
--
--  Description :
--		bccb module.
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
#include "isp_bccb.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"BCCB_I:"
#define WARNHEAD	"BCCB_W:"
#define ERRHEAD		"BCCB_E:"
#define TIPHEAD		"BCCB_T:"


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: bccb_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_init(isp_bccb_context_t *bccb, bccb_config_t *cfg)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);
	IM_ASSERT(cfg != IM_NULL);
	
	/*check bccb mode*/
	if(cfg->bccbMode > ISP_BCCB_MODE_CBBC)
	{
		IM_ERRMSG((IM_STR("bccb mode error: mode = %d"), cfg->bccbMode));
		return IM_RET_OK;
	}
	
	/*check bc module*/
	if((cfg->bc.rBlkTh < 0) || (cfg->bc.rBlkTh > 2047)
		|| (cfg->bc.grBlkTh < 0) || (cfg->bc.grBlkTh > 2047)
		|| (cfg->bc.bBlkTh < 0) || (cfg->bc.bBlkTh > 2047)
		|| (cfg->bc.gbBlkTh < 0) || (cfg->bc.gbBlkTh > 2047))
	{
		IM_ERRMSG((IM_STR("Invalid blackThreshold: rblkth=%d, grblkth=%d, bblkth=%d, gbblkth=%d"),
									cfg->bc.rBlkTh, cfg->bc.grBlkTh, cfg->bc.bBlkTh, cfg->bc.gbBlkTh));
		return IM_RET_INVALID_PARAMETER;
	}

	/*check cb module*/
	if((cfg->cb.rGain < 0) || (cfg->cb.rGain > 1023)
		|| (cfg->cb.bGain < 0) || (cfg->cb.bGain > 1023)
		|| (cfg->cb.grGain < 0) || (cfg->cb.grGain > 1023)
		|| (cfg->cb.gbGain < 0)|| (cfg->cb.gbGain > 1023))
	{
		IM_ERRMSG((IM_STR("Invalid gain: rGain=%d, bGain=%d, grGain=%d, gbGain=%d"), 
					cfg->cb.rGain, cfg->cb.bGain, cfg->cb.grGain, cfg->cb.gbGain));
		return IM_RET_INVALID_PARAMETER;
	}


	if((cfg->bcEnable==IM_TRUE)&&(cfg->cbEnable==IM_TRUE))
	{
		/*set bccb mode*/
		SetIspRegister(bccb->regVal, ISP_GLB_CFG3_BCCBMODE, cfg->bccbMode);

		/*set bc module and enable*/
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_R_BLACKTH, cfg->bc.rBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GR_BLACKTH, cfg->bc.grBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_B_BLACKTH, cfg->bc.bBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GB_BLACKTH, cfg->bc.gbBlkTh);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 0);
		
		/*set cb module and enable*/	
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBRGAIN, cfg->cb.rGain);
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBBGAIN, cfg->cb.bGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINR, cfg->cb.grGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINB, cfg->cb.gbGain);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG3], bccb->regVal[rISP_GLB_CFG3]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_BLC_CFG], bccb->regVal[rISP_BLC_CFG]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_RBGAIN], bccb->regVal[rISP_CB_RBGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_GGGAIN], bccb->regVal[rISP_CB_GGGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	else if(cfg->bcEnable==IM_TRUE)
	{
		/*set bc module and enable*/
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_R_BLACKTH, cfg->bc.rBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GR_BLACKTH, cfg->bc.grBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_B_BLACKTH, cfg->bc.bBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GB_BLACKTH, cfg->bc.gbBlkTh);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 0);
		
		/*set cb bypass*/	
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 1);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_BLC_CFG], bccb->regVal[rISP_BLC_CFG]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	else if(cfg->cbEnable==IM_TRUE)
	{
		/*set bc bypass*/
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 1);
		
		/*set cb module and enable*/	
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBRGAIN, cfg->cb.rGain);
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBBGAIN, cfg->cb.bGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINR, cfg->cb.grGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINB, cfg->cb.gbGain);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_RBGAIN], bccb->regVal[rISP_CB_RBGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_GGGAIN], bccb->regVal[rISP_CB_GGGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set bc bypass*/
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 1);
		/*set cb bypass*/	
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 1);
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}

	bccb->bccbMode = cfg->bccbMode;
	bccb->bc.rBlkTh = cfg->bc.rBlkTh;
	bccb->bc.grBlkTh = cfg->bc.grBlkTh;
	bccb->bc.bBlkTh = cfg->bc.bBlkTh;
	bccb->bc.gbBlkTh = cfg->bc.gbBlkTh;
	bccb->cb.rGain = cfg->cb.rGain;
	bccb->cb.bGain = cfg->cb.bGain;
	bccb->cb.grGain = cfg->cb.grGain;
	bccb->cb.gbGain = cfg->cb.gbGain;
	bccb->bcEnable = cfg->bcEnable;
	bccb->cbEnable = cfg->cbEnable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_mode

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_mode(isp_bccb_context_t *bccb, IM_UINT32 mode)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);
	
	/*check bccb mode*/
	if(mode > ISP_BCCB_MODE_CBBC)
	{
		IM_ERRMSG((IM_STR("bccb mode error: mode = %d"), mode));
		return IM_RET_OK;
	}
	
	if(bccb->bccbMode == mode)
	{
		IM_INFOMSG((IM_STR("bccb is already this mode")));
		return IM_RET_OK;
	}


	if((bccb->bcEnable==IM_TRUE)&&(bccb->cbEnable==IM_TRUE))
	{
		SetIspRegister(bccb->regVal, ISP_GLB_CFG3_BCCBMODE, mode);
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG3], bccb->regVal[rISP_GLB_CFG3]));
	}
	
	bccb->bccbMode = mode;	
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_bc

        Functional description: set black compensate(bc) threshold

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_bc(isp_bccb_context_t *bccb, bc_context_t *bc)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);
	IM_ASSERT(bc != IM_NULL);

	/*check bc module*/	
	if((bc->rBlkTh < 0) || (bc->rBlkTh > 2047)
		|| (bc->grBlkTh < 0) || (bc->grBlkTh > 2047)
		|| (bc->bBlkTh < 0) || (bc->bBlkTh > 2047)
		|| (bc->gbBlkTh < 0) || (bc->gbBlkTh > 2047))

	{
		IM_ERRMSG((IM_STR("Invalid blackThreshold: rblkth=%d, grblkth=%d, bblkth=%d, gbblkth=%d"),
													bc->rBlkTh, bc->grBlkTh, bc->bBlkTh, bc->gbBlkTh));
		return IM_RET_INVALID_PARAMETER;
	}

	if(bccb->bcEnable==IM_TRUE)
	{
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_R_BLACKTH, bc->rBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GR_BLACKTH, bc->grBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_B_BLACKTH, bc->bBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GB_BLACKTH, bc->gbBlkTh);
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_BLC_CFG], bccb->regVal[rISP_BLC_CFG]));
	}

	bccb->bc.rBlkTh = bc->rBlkTh;
	bccb->bc.grBlkTh = bc->grBlkTh;
	bccb->bc.bBlkTh = bc->bBlkTh;
	bccb->bc.gbBlkTh = bc->gbBlkTh;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_cb

        Functional description: set cb gain

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_cb(isp_bccb_context_t *bccb, cb_context_t *cb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);
	IM_ASSERT(cb != IM_NULL);
	
	/*check cb module*/	
	if((cb->rGain < 0) || (cb->rGain > 1024)
		|| (cb->bGain < 0) || (cb->bGain > 1024)
		|| (cb->grGain < 0)|| (cb->grGain > 1024)
		|| (cb->gbGain < 0)|| (cb->gbGain > 1024))
	{
		IM_ERRMSG((IM_STR("Invalid gain: rGain=%d, bGain=%d, grGain=%d, gbGain=%d"), 
					cb->rGain, cb->bGain, cb->grGain, cb->gbGain));
		return IM_RET_INVALID_PARAMETER;
	}

	if(bccb->cbEnable==IM_TRUE)
	{
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBRGAIN, cb->rGain);
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBBGAIN, cb->bGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINR, cb->grGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINB, cb->gbGain);
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_RBGAIN], bccb->regVal[rISP_CB_RBGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_GGGAIN], bccb->regVal[rISP_CB_GGGAIN]));
	}

	bccb->cb.rGain = cb->rGain;
	bccb->cb.bGain = cb->bGain;
	bccb->cb.grGain = cb->grGain;
	bccb->cb.gbGain = cb->gbGain;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_bc_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_bc_enable(isp_bccb_context_t *bccb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);

	if(bccb->bcEnable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("bc has been enabled")));
		return IM_RET_OK;	
	}

	if(bccb->cbEnable == IM_TRUE)
	{
		/*set bccb mode*/
		SetIspRegister(bccb->regVal, ISP_GLB_CFG3_BCCBMODE, bccb->bccbMode);

		/*set bc module and enable*/
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_R_BLACKTH, bccb->bc.rBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GR_BLACKTH, bccb->bc.grBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_B_BLACKTH, bccb->bc.bBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GB_BLACKTH, bccb->bc.gbBlkTh);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG3], bccb->regVal[rISP_GLB_CFG3]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_BLC_CFG], bccb->regVal[rISP_BLC_CFG]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set bc module and enable*/
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_R_BLACKTH, bccb->bc.rBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GR_BLACKTH, bccb->bc.grBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_B_BLACKTH, bccb->bc.bBlkTh);
		SetIspRegister(bccb->regVal, ISP_BLC_CFG_GB_BLACKTH, bccb->bc.gbBlkTh);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_BLC_CFG], bccb->regVal[rISP_BLC_CFG]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	
	bccb->bcEnable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_bc_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_bc_disable(isp_bccb_context_t *bccb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);

	if(bccb->bcEnable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("bc has been disabled")));
		return IM_RET_OK;	
	}
	//disable bc
	
	SetIspRegister(bccb->regVal, ISP_GLB_CFG2_BCBYPASS, 1);
	IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));

	bccb->bcEnable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_cb_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_cb_enable(isp_bccb_context_t *bccb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);

	if(bccb->cbEnable == IM_TRUE)
	{
		IM_INFOMSG((IM_STR("cb has been enabled")));
		return IM_RET_OK;	
	}
	//enable cb
	if(bccb->bcEnable == IM_TRUE)
	{
		/*set bccb mode*/
		SetIspRegister(bccb->regVal, ISP_GLB_CFG3_BCCBMODE, bccb->bccbMode);
		
		/*set cb module and enable*/	
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBRGAIN, bccb->cb.rGain);
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBBGAIN, bccb->cb.bGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINR, bccb->cb.grGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINB, bccb->cb.gbGain);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG3], bccb->regVal[rISP_GLB_CFG3]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_RBGAIN], bccb->regVal[rISP_CB_RBGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_GGGAIN], bccb->regVal[rISP_CB_GGGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set cb module and enable*/	
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBRGAIN, bccb->cb.rGain);
		SetIspRegister(bccb->regVal, ISP_CB_RBGAIN_CBBGAIN, bccb->cb.bGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINR, bccb->cb.grGain);
		SetIspRegister(bccb->regVal, ISP_CB_GGGAIN_CBGGAINB, bccb->cb.gbGain);
		SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 0);
		
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_RBGAIN], bccb->regVal[rISP_CB_RBGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_CB_GGGAIN], bccb->regVal[rISP_CB_GGGAIN]));
		IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));
	}

	bccb->cbEnable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: bccb_set_cb_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET bccb_set_cb_disable(isp_bccb_context_t *bccb)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(bccb != IM_NULL);
	IM_ASSERT(bccb->regVal != IM_NULL);
	IM_ASSERT(bccb->regOfst != IM_NULL);

	if(bccb->cbEnable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("cb has been disabled")));
		return IM_RET_OK;	
	}
	//disable cb
	SetIspRegister(bccb->regVal, ISP_GLB_CFG2_CBBYPASS, 1);
	IM_JIF(isppwl_write_reg(bccb->regOfst[rISP_GLB_CFG2], bccb->regVal[rISP_GLB_CFG2]));

	bccb->cbEnable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}
