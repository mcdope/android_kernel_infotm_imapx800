/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of ids lib, in Kernel call
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/5/11 :  first commit :
** 1.0.5	 Sam@2012/6/05 :  add acc & acm support
**			 Sam@2012/6/27 :  LoadEnable off --> acc enable
** 						      LoadEnable On  --> register configuration enable
** 							  So ACC has bugs, would not use it any more.
** 2.0.1     Sam@2013/2/25 :  new version
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <mach/items.h>

#include <ids_pwl.h>
#include <ids_lib.h>
#include <ie_lib.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IELIB_I:"
#define WARNHEAD	"IELIB_W:"
#define ERRHEAD		"IELIB_E:"
#define TIPHEAD		"IELIB_T:"

typedef struct{
    IM_INT32  idsx;
    IM_INT32  width;
    IM_INT32  height;
    IM_UINT32 ACCLevel;
    IM_UINT32 ACMLevel;
    IM_UINT32 GammaLevel;
    IM_UINT32 DitherLevel;
    IM_UINT32 DitherPort;

	IM_UINT32 IeFlagSuspend;
} IEStruct;

static IEStruct gIEHandle[2] = {{0},{0}};


typedef struct{
	IM_UINT32 oftst;
	IM_UINT32 gaori;
	IM_UINT32 gaoes;
}gamma_struct_t;

static const IM_UINT32 accLutbCoef0[ENH_IE_ACC_LUTB_LENGTH]={
	#include "ie_acc_table.h"
};

static const gamma_struct_t gammaCoef[][ENH_IE_GAMMA_COEF_LENGTH]={
	#include "ie_gamma_table.h"
};

static IM_RET ie_basic_enable(IM_INT32 idsx, IM_INT32 width, IM_INT32 height)
{
	// set ie width & height 
	IM_INFOMSG((IM_STR("%s (idsx=%d, width =%d , height = %d - "),IM_STR(_IM_FUNC_),idsx,width,height));
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, ENH_FSR,ENH_FSR_VRES,12,height-1);
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, ENH_FSR,ENH_FSR_HRES,12,width-1);
		
	// y2r enable 	
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR,ENH_IECR_Y2RPASSBY,1,0);
	// r2y enable 
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR,ENH_IECR_R2YPASSBY,1,0);
	// hist enable 
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR,ENH_IECR_HISTPASSBY,1,0);

	return IM_RET_OK;
}

IM_RET ielib_init(ie_handle_t *handle, IM_INT32 idsx)
{
	IM_INFOMSG((IM_STR("%s, idsx=%d"), IM_STR(_IM_FUNC_),idsx));

    gIEHandle[idsx].idsx = idsx;
	*handle = (ie_handle_t)&gIEHandle[idsx];
	return IM_RET_OK;
}

IM_RET ielib_deinit(ie_handle_t handle)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ielib_set_width_height(ie_handle_t handle, IM_INT32 width, IM_INT32 height)
{
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;
    
    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d, width=%d, height=%d)"), IM_STR(_IM_FUNC_), idsx, width, height));

    ie_basic_enable(idsx, width, height);
    ie->width = width;
    ie->height = height;

    return IM_RET_OK;
}

IM_RET ielib_acc_set(ie_handle_t handle, IM_UINT32 acc)
{
	IM_UINT32 i, acccoe, value, level, idsx;
    IEStruct *ie = (IEStruct*)handle;
    
    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d, acc=%d)"), IM_STR(_IM_FUNC_), idsx, acc));

    if (acc > 100){                                                    
        IM_ERRMSG((IM_STR("acc level not supported(%d)-\n"),acc));
        return IM_RET_FAILED;
    }

	if(!acc){
        idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_ACCPASSBY, 1 , 1); 
        ie->ACCLevel = 0;
        return IM_RET_OK;
	}

	idspwl_read_reg((idsx==0)?MODULE_IDS0:MODULE_IDS1, ENH_IECR, &value);
	if (((value >> ENH_IECR_ACCREADY) & 0x01) == 0) {
		// acc input data set 
		IM_INFOMSG((IM_STR("to get acc ready ")));
		for(i = 0;i < ENH_IE_ACC_LUTB_LENGTH;i++)
		{
			value = 0;
			value = i << ENH_ACCLUTR_ACCLUTA | (accLutbCoef0[i] *4);
			idspwl_write_reg((idsx==0)?MODULE_IDS0:MODULE_IDS1, ENH_ACCLUTR, value);
		}

		// acc ready 
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, ENH_IECR, ENH_IECR_ACCREADY, 1 , 1);
	}

    level = acc/20;
	if (level)
	{
		switch(level) {
			case 1:
				acccoe = 2867;
				break;
			case 2:
				acccoe = 3277;
				break;
			case 3:
				acccoe = 4096;
				break;
			case 4:
				acccoe = 4915;
				break;
			case 5:
				acccoe = 5325;
				break;
			default :
				IM_ERRMSG((IM_STR(" error : acc level not support  ")));
				return IM_RET_FAILED;
		}
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACCCOER, ENH_ACCCOER_ACCCOE, 16, acccoe);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_ACCPASSBY, 1 , 0); 
    }

	ie->ACCLevel = acc ;
	return IM_RET_OK;
}

IM_RET ielib_acc_get_status(ie_handle_t handle, IM_UINT32 *acc)
{
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;

    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

	*acc = ie->ACCLevel;
	return IM_RET_OK;
}

IM_RET ielib_acm_set(ie_handle_t handle, IM_UINT32 acm)
{
	IM_UINT32 acmcoep, acmcoer, acmcoeg, acmcoeb, val;
	IM_UINT32 acmcoest, idsx;
    IEStruct *ie = (IEStruct*)handle;
	
    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d, acm=%d)"), IM_STR(_IM_FUNC_),idsx, acm));

	if (ie->IeFlagSuspend == 1){
		IM_INFOMSG((IM_STR(" ielib in suspend state , not to respond")));
		return IM_RET_OK;
	}

    if (acm > 100){                                                    
        IM_ERRMSG((IM_STR("acm level not supported(%d)-\n"),acm));
        return IM_RET_FAILED;
    }

    if(acm){
        acmcoest = acm * 50 + 1000;
        idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACMTHER, 0, 16, acmcoest);
        idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_ACMPASSBY, 1 , 0);

		/* The following is to set P/R/G/B channel respectively.
		 * 		P : R/G/B channels adjusted in the the proportion.
		 *		R : just R channel adjusted to a certain proportion.
		 *		G : just G channel adjusted to a certain proportion.
		 *		B : just B channel adjusted to a certain proportion.
		 *		P/R/G/B modify are based on that ACM is switched on .
		 *		Here, just for local IDS(ids0 usually) need.
		 */
		if (idsx == 0){
			acmcoep = 0x800;
			acmcoer = 0x800;
			acmcoeg = 0x800;
			acmcoeb = 0x800;
			if (item_exist("ids.loc.dev0.acm_level_p")){
				val = item_integer("ids.loc.dev0.acm_level_p", 0);
				if (val > 0)
					acmcoep = val * 50;
			}
			if (item_exist("ids.loc.dev0.acm_level_r")){
				val = item_integer("ids.loc.dev0.acm_level_r", 0);
				if (val > 0)
					acmcoer = val * 50;
			}
			if (item_exist("ids.loc.dev0.acm_level_g")){
				val = item_integer("ids.loc.dev0.acm_level_g", 0);
				if (val > 0)
					acmcoeg = val * 50;
			}
			if (item_exist("ids.loc.dev0.acm_level_b")){
				val = item_integer("ids.loc.dev0.acm_level_b", 0);
				if (val > 0)
					acmcoeb = val * 50;
			}
			idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACMCOEP, 0, 16, acmcoep);
			idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACMCOER, 0, 16, acmcoer);
			idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACMCOEG, 0, 16, acmcoeg);
			idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_ACMCOEB, 0, 16, acmcoeb);
		}
    }else{
        idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_ACMPASSBY, 1 , 1);
    }

    ie->ACMLevel = acm;
	return IM_RET_OK;
}

IM_RET ielib_acm_get_status(ie_handle_t handle, IM_UINT32 *acm)
{
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    idsx = ie->idsx;

	*acm = ie->ACMLevel;
	return IM_RET_OK;
}

IM_RET ielib_gamma_set(ie_handle_t handle, IM_UINT32 gamma)
{
	IM_UINT32 i, val, index, level, idsx;
    IEStruct *ie = (IEStruct*)handle;

    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d, gamma=%d)"), IM_STR(_IM_FUNC_), idsx, gamma));

	if (!gamma){
        idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_GAPASSBY, 1 , 1); 
        ie->GammaLevel = gamma;
        return IM_RET_OK;
	}

    level = (gamma + 24) / 25; 
	if (level){
        /*
		switch(level){
			case 1: // 0.45
				index = 0;
				break;
			case 2: // 1
				index = 1;
				break;
			case 3: //1.8
				index = 2;
				break;
			case 4: // 2.2
				index = 3;
				break;
			default:
				IM_ERRMSG((IM_STR(" error : gamma level not support  ")));
				return IM_RET_FAILED;
		}*/
		index = 0;
		for (i = 0; i < 32; i ++){
			val = ((gammaCoef[index][i].oftst & 7) << 18) | ((gammaCoef[index][i].gaori & 0xff) << 10)
				| ((gammaCoef[index][i].gaoes ) & 0x3FF);
	        IM_INFOMSG((IM_STR("sam : gamma i=%d , val=0x%x  \n"),i,val));
			idspwl_write_reg((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_RGAMMA + i * 4 , val); //R
			idspwl_write_reg((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_GGAMMA + i * 4 , val); //G
			idspwl_write_reg((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_BGAMMA + i * 4 , val); //B
		}
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,ENH_IECR, ENH_IECR_GAPASSBY, 1 , 0); 
	}

	ie->GammaLevel = gamma;
	return IM_RET_OK;
}

IM_RET ielib_gamma_get_status(ie_handle_t handle, IM_UINT32 *gamma)
{
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;
    
    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
	
	*gamma = ie->GammaLevel;
	return IM_RET_OK;
}

IM_RET ielib_dither_set(ie_handle_t handle, IM_INT32 portType, IM_BOOL en)
{
	IM_UINT32 width, height;
	IM_UINT32 rch=2, gch=2, bch=2;
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;

    idsx = ie->idsx;
	width = ie->width;
	height = ie->height;

	IM_INFOMSG((IM_STR("%s(idsx=%d, en=%d)"), IM_STR(_IM_FUNC_), idsx, en));

	if (!en)
    {
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,DIT_DCR,DIT_DCR_PASSBY,1,1);
		ie->DitherLevel = 0;
        return IM_RET_OK;
	}
	
	// later modify it , temporarily dev0 by default.
    switch (portType){
        case DISPDEV_DATAPORT_RGB565:
            rch = 5;
            gch = 4;
            bch = 5;
            break;
        case DISPDEV_DATAPORT_RGB666:
            rch = 4;
            gch = 4;
            bch = 4;
            break;
        case DISPDEV_DATAPORT_RGB888:
            IM_WARNMSG((IM_STR(" data port 888, no need to enable dither ")));
            return IM_RET_OK;
        default : 
            IM_ERRMSG((IM_STR(" dither set : port type not supported - ")));
            return IM_RET_FAILED;     
    }

    ie->DitherPort = portType;
	if(en){
		// set vres & hres
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,DIT_FSR,DIT_FSR_VRES,12,height - 1);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,DIT_FSR,DIT_FSR_HRES,12,width - 1);

		IM_INFOMSG((IM_STR(" dither : width = %d , height=%d  --"),width,height));
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, DIT_DCR,DIT_DCR_RNB,3,rch);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, DIT_DCR,DIT_DCR_GNB,3,gch);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, DIT_DCR,DIT_DCR_BNB,3,bch);

		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,DIT_DCR,DIT_DCR_PASSBY,1,0);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1,DIT_DCR,DIT_DCR_TEMPO,1,1);
		ie->DitherLevel = 1;
    }
	return IM_RET_OK;
}

IM_RET ielib_dither_get_status(ie_handle_t handle, IM_BOOL *en)
{
    IM_INT32 idsx;
    IEStruct *ie = (IEStruct*)handle;

    idsx = ie->idsx;
	IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

	*en = (IM_BOOL)ie->DitherLevel;
	return IM_RET_OK;
}

IM_RET ielib_suspend(ie_handle_t handle)
{
    IEStruct *ie = (IEStruct*)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ie->IeFlagSuspend = 1;
    return IM_RET_OK;
}

IM_RET ielib_resume(ie_handle_t handle)
{
    IEStruct *ie = (IEStruct*)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ie->IeFlagSuspend = 0;
    ielib_set_width_height(handle, ie->width, ie->height);

    ielib_acc_set(handle, ie->ACCLevel);
    ielib_acm_set(handle, ie->ACMLevel);
    ielib_gamma_set(handle, ie->GammaLevel);
    ielib_dither_set(handle, ie->DitherPort, ie->DitherLevel);

    return IM_RET_OK;
}


