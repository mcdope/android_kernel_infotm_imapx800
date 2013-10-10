/*------------------------------------------------------------------------------
--      Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.                  --
--                                                                                                                                                        --
--      This program is free software; you can redistribute it and/or modify      --
--      it under the terms of the GNU General Public License as published by      --
--      the Free Software Foundation; either version 2 of the License, or                 --
--      (at your option) any later version.                                                                   --
--------------------------------------------------------------------------------
--      RCSfile: gc0307_aih.c
--
--  Description :
--
--      Author:
--     haishengchen   
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1       arsor@2012/09/028: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "gc0307_aih.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1


#define INFOHEAD	     "GC0307_XYC_I:"
#define WARNHEAD	"GC0307_XYC_W:"
#define ERRHEAD		"GC0307_XYC_E:"
#define TIPHEAD		"GC0307_XYC_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
}gc0307_aih_context_t;

static gc0307_aih_context_t gGc0307;
static pwl_handle_t gPwl = IM_NULL;
static IM_CHAR GC0307_ID = 0x00;

static camsenpwl_pmu_info_t gPmuInfo = {
	2, 	//useChs
	//channels
	{
		//channel0
		{
			"iovdd",	//pwName
			2800000,	//volt(mv)
		},
		//channel1
		{
			"dvdd",		//pwName
			1800000,	//volt(mv)
		},
	},
};

#define cam_gc0307_read(b,c,d) camsenpwl_i2c_read(gPwl,c,b,1,d)
#define cam_gc0307_write(b) camsenpwl_i2c_write(gPwl,b,2)

int gc0307_aih_switch_qvga(void)
{
	int i, ret;
	for(i = 0;i < (sizeof(gc0307_qvga_regs) / 2);i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_qvga_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
		msleep(10);
	}
	return 0;
}

//==================set effect====================================

IM_INT32 gc0307_aih_set_greenish(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_greenish_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_greenish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_blackboard(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_blackboard_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_blackboard_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_whiteboard(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_whiteboard_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_whiteboard_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}
IM_INT32 gc0307_aih_set_sepia(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_sepia_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_sepia_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}
IM_INT32 gc0307_aih_set_reddish(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_reddish_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_reddish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_yellowish(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_yellowish_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_yellowish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_negative(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_negative_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_negative_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_bandw(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_bandw_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_bandw_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_normal(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_normal_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_normal_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

//==============================================================

//==================set wb======================================
IM_INT32 gc0307_aih_set_auto(void)
{
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0307_auto_regs) / 2);i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_auto_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}


IM_INT32 gc0307_aih_set_home(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_home_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_home_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_office(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_office_regs) / 2);i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_office_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_sunny(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_sunny_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_sunny_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_cloudy(void)
{
	IM_INT32 i, ret;
	for(i = 0;i < (sizeof(gc0307_cloudy_regs) / 2); i++)
	{
		ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_cloudy_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0;
}

//====================================================================

//===========================set sceneMode=============================
IM_INT32 gc0307_aih_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"),IM_STR(_IM_FUNC_),enable));
	if(enable) /*night mode*/
	{
		if(gGc0307.bandMode == CAM_ANTIBANDING_MODE_60HZ)
		{
			for (i = 0;i < (sizeof(gc0307_default_regs) / 2); i++)
			{
				ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
		else
		{
			for(i = 0;i < (sizeof(gc0307_default_regs) / 2); i++)
			{
				ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
	}
	else
	{
		if(gGc0307.bandMode == CAM_ANTIBANDING_MODE_60HZ)
		{
			for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
			{
				ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
		else
		{
			for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
			{
				ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
	}
	return 0;
}

IM_INT32 gc0307_aih_set_scene_mode(IM_INT32 value)
{
	if(value ==gGc0307.sceneMode)
	{
		return 0;
	}
	
	IM_BOOL nightModeEn;
	IM_INFOMSG((IM_STR("%s(value = %d)"),IM_STR(_IM_FUNC_),value));
	if((value == CAM_SCENE_MODE_NIGHT) || (value == CAM_SCENE_MODE_NIGHT_PORTRAIT)) /*set night mode*/
	{
		nightModeEn = IM_TRUE;
	}else
	{
		nightModeEn = IM_FALSE;
	}
	
	gGc0307.sceneMode = value;
	
	return gc0307_aih_night_mode(nightModeEn);
}

IM_INT32 gc0307_aih_set_exposure(IM_INT32 value)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc0307.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -3:							/* EV -1.5 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -2:							/* EV -1 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -1:							/* EV -0.5 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 0:								/* EV 0 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 1:							/* EV +0.5 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 2:							/* EV +1 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 3:							/* EV +1.5 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 4:							/* EV +2 */
            for(i = 0; i < (sizeof(gc0307_default_regs) / 2); i++)
            {
                ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_default_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		default:
			return -1;
	}

	gGc0307.expVal = value;

	return 0;

}

IM_INT32 gc0307_aih_set_antibanding(IM_INT32 value)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc0307.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_50HZ:
			cam_gc0307_write((IM_UINT8 *)(&gc0307_night_50hz_regs));
            /*for(i = 0; i < (sizeof(gc0308_50hz_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_50hz_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }*/
			break;
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_60HZ:
			cam_gc0307_write((IM_UINT8 *)(&gc0307_night_60hz_regs));
            /*for(i = 0; i < (sizeof(gc0308_60hz_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_60hz_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }*/
			break;

		default:
			return -1;
	}

	gGc0307.bandMode = value;

	return 0;
}

//===================================================================
IM_INT32 gc0307_aih_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gGc0307.efType)
	{
		return 0;
	}
	
	switch(value)
	{
		case CAM_SPECIAL_EFFECT_NONE:
			gc0307_aih_set_normal();
		break;
		case CAM_SPECIAL_EFFECT_MONO:
			gc0307_aih_set_bandw();
		break;
		case CAM_SPECIAL_EFFECT_NEGATIVE:
			gc0307_aih_set_negative();
		break;
		case CAM_SPECIAL_EFFECT_SOLARIZE:
			gc0307_aih_set_yellowish();
		break;
		case CAM_SPECIAL_EFFECT_PASTEL:
			gc0307_aih_set_reddish();
		break;
		case CAM_SPECIAL_EFFECT_SEPIA:
			gc0307_aih_set_sepia();
		break;
		case CAM_SPECIAL_EFFECT_WHITEBOARD:
			gc0307_aih_set_whiteboard();
		break;
		case CAM_SPECIAL_EFFECT_BLACKBOARD:
			gc0307_aih_set_blackboard();
		break;
		case CAM_SPECIAL_EFFECT_AQUA:
			gc0307_aih_set_greenish();
		break;
		case CAM_SPECIAL_EFFECT_POSTERIZE:
		break;
		case CAM_SPECIAL_EFFECT_MOSAIC:
		break;
		case CAM_SPECIAL_EFFECT_RESIZE:
		break;
		default:
		break;
	}
	gGc0307.efType = value;
	
	return 0;
}

IM_INT32 gc0307_aih_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"),IM_STR(_IM_FUNC_),value));
	
	if(value == gGc0307.wbMode)
	{
		return 0;
	}
	
	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			gc0307_aih_set_auto();
		break;
		case CAM_WB_MODE_INCANDESCENT:
			gc0307_aih_set_home();
		break;
		case CAM_WB_MODE_FLUORESCENT:
			gc0307_aih_set_office();
		break;
		case CAM_WB_MODE_DAYLIGHT:
			gc0307_aih_set_sunny();
		break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			gc0307_aih_set_cloudy();
		break;
		case CAM_WB_MODE_WARM_FLUORECENT:
		break;
		case CAM_WB_MODE_TWILIGHT:
		break;
		case CAM_WB_MODE_SHADE:
		break;
		default:
		break;
	}
	gGc0307.wbMode = value;
	return 0;
}



IM_RET gc0307_aih_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(gc0307_aih_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

	//set PWDN  state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(padNum, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(padNum, 0);

	/******************************
	*power down enable
	******************************/
	//set io outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
	camsenpwl_io_set_outdat(padNum, 1);

	return IM_RET_OK;
}

IM_RET gc0307_aih_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"),IM_STR(_IM_FUNC_)));	
	memcpy(pmuInfo,(void*)&gPmuInfo,sizeof(camsenpwl_pmu_info_t));
	return IM_RET_OK;
}



IM_RET gc0307_aih_init(pwl_handle_t pwl,IM_BOOL checkOnly)
{
	IM_INT32 i,ret;
	IM_CHAR buf = 0;
	IM_INFOMSG((IM_STR("%s()++"),IM_STR(_IM_FUNC_)));
	gPwl = pwl;
	
	camsenpwl_memset((void*)&gGc0307,0x0,sizeof(gGc0307));
	//init value
  gGc0307.wbMode = CAM_WB_MODE_AUTO;
	gGc0307.efType = CAM_SPECIAL_EFFECT_NONE;
	gGc0307.expVal = 0;
	gGc0307.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gGc0307.sceneMode = CAM_SCENE_MODE_AUTO;
	
	gGc0307.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	gGc0307.reset = camsenpwl_get_reset_padnum(gPwl);
	
	//config io
	/*io_index138(XCAMD12)<-->gc0308.MCLK, io_indexgGc0307.reset(XCAMD16)<-->gc0308.RESET,
	 * io_index136(XCAMD10)<-->gc0308.PWDN, io_index_XXX<-->gc0308.POWER*/

	/******************************
	*set each io mode
	******************************/
	//set IO mode, default is case0
	//volatile unsigned IM_INT32 *)(0x21e09000) = 0x0;

	//set POWER(io index) state(mode and dir)
	
	//set RESET(io index=gGc0307.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	
	camsenpwl_io_set_mode(gGc0307.reset,1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc0307.reset,0);
	
	//set pwdn state(mode and dir)
	//set io mode = 1(use as gpio mode)
	camsenpwl_io_set_mode(gGc0307.pwdn,1);
	//set io dir = 0(output dir)
	camsenpwl_io_set_dir(gGc0307.pwdn,0);
	
	/**************************************************
	*POWER enable: supply power, has enable all the time
	******************************************************/
	//delay some time
	msleep(5);
	
	/******************************
	*provided mclk
	******************************/
	camsenpwl_clock_enable(gPwl,24000000);//irequest MCLK 24MHZ
	
	//check real clock freq if need
	//camsenpwl_clock_get_freq(gPwl);

	/******************************
	*reset sensor
	******************************/
	//set io(index=gGc0307.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gGc0307.reset, 1);
	
	camsenpwl_io_set_outdat(gGc0307.reset,0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gGc0307.reset,0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gGc0307.reset,1);
	
	msleep(5);
	
	/******************************
	*power down disable
	******************************/
	//set io(index=136) outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
	camsenpwl_io_set_outdat(gGc0307.pwdn,1);
	
	camsenpwl_io_set_outdat(gGc0307.pwdn,0);
	
	msleep(5);
	
	//read sensor id
	if(checkOnly == IM_TRUE)
		{
			cam_gc0307_read((IM_UINT8 *)(&GC0307_ID),&buf,1);	
			IM_TIPMSG((IM_STR("%s(GC0307_ID = 0x%x,real value is 0x99)"),IM_STR(_IM_FUNC_),buf));
			if(buf != 0x99)
				{
					IM_ERRMSG((IM_STR("gc0307 id error>>>>>>>>>")));
					goto Fail;
				}
				return IM_RET_OK;
		}
		buf = 0;
		
		for(i = 0;i < (sizeof(gc0307_init_regs) / 2);i++)
		{
			ret = cam_gc0307_write((IM_UINT8 *)(&gc0307_init_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Fail to transfer data to i2c!")));
				/*if(gc0307_aih_repeat_init() != 0)
				{
						goto Fail;
				}*/
			}
			else{
				}
		}
		
		IM_INFOMSG((IM_STR("%s()--"),IM_STR(_IM_FUNC_)));
		
		return IM_RET_OK;

Fail:
	//power down enable 
	camsenpwl_io_set_outdat(gGc0307.pwdn,1);
	//reset sensor
	camsenpwl_io_set_outdat(gGc0307.reset,0);
	//close mclk
	camsenpwl_clock_disable(gPwl);
	
	return IM_RET_FAILED;
		
}

IM_RET gc0307_aih_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"),IM_STR(_IM_FUNC_)));
	
	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gGc0307.pwdn,1);
	
	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gGc0307.reset,0);
	
	/******************************
	*close mclk
	******************************/
	//set io mode = 1(gpio mode)
	//camsenpwl_io_set_mode(138, 1);
	camsenpwl_clock_disable(gPwl);
	
	/******************************************************
	*power close
	******************************************************/
	//camsenpwl_io_set_outdat(, 0);
	gPwl = IM_NULL;
	return ret;
	
}

IM_RET gc0307_aih_start(void)
{
	IM_INT32 i;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
		
}

IM_RET gc0307_xyc_stop(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET gc0307_aih_stop(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET gc0307_aih_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"),IM_STR(_IM_FUNC_)));
	caps->supportRes = CAM_RES_QVGA | CAM_RES_VGA;
	caps->maxRes = CAM_RES_VGA;
	caps->initRes = CAM_RES_VGA;
	
	return ret;
}

IM_RET gc0307_aih_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"),IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET gc0307_aih_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res,fps;
	IM_INFOMSG((IM_STR("%s"),IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;
	
	if(res == CAM_RES_QVGA)  //320*240
	{
		ret = gc0307_aih_switch_qvga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc0307_aih_switch_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else
	{
		IM_ERRMSG((IM_STR("this resolution(0x%x)&fps(0x%x) is not supported!"),res,fps));
	}
}

IM_RET gc0307_aih_set_property(IM_UINT32 property,void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"),IM_STR(_IM_FUNC_)));
	
	memcpy((void*)&value,p,sizeof(value));
	
	if(property == CAM_KEY_RW_WB_MODE)
	{
		gc0307_aih_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		gc0307_aih_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		gc0307_aih_set_scene_mode(value);
	}
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		gc0307_aih_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		gc0307_aih_set_antibanding(value);
	}
	
	msleep(20);
	
	return ret;
}

IM_RET gc0307_aih_get_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	switch(property)
	{
		case CAM_KEY_R_CAPS:
			value = CAM_CAP_WB_MODE_SUPPORT |
				CAM_CAP_SPECIAL_EFFECT_SUPPORT |
				CAM_CAP_SCENE_MODE_SUPPORT |
				CAM_CAP_ANTIBANDING |
				CAM_CAP_EXPOSURE |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_SUPPORT_WB_MODE:
			value = CAM_WB_MODE_AUTO |
				CAM_WB_MODE_INCANDESCENT |
				CAM_WB_MODE_FLUORESCENT |
				CAM_WB_MODE_DAYLIGHT |
				CAM_WB_MODE_CLOUDY_DAYLIGHT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_WB_MODE:
			memcpy(p, (void*)&gGc0307.wbMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SPECIAL_EFFECT:
			value = CAM_SPECIAL_EFFECT_NONE |
				CAM_SPECIAL_EFFECT_MONO |
				CAM_SPECIAL_EFFECT_NEGATIVE |
				//CAM_SPECIAL_EFFECT_SOLARIZE |
				//CAM_SPECIAL_EFFECT_PASTEL |
				CAM_SPECIAL_EFFECT_SEPIA |
				//CAM_SPECIAL_EFFECT_POSTERIZE |
				//CAM_SPECIAL_EFFECT_WHITEBOARD |
				//CAM_SPECIAL_EFFECT_BLACKBOARD |
				CAM_SPECIAL_EFFECT_AQUA |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy(p, (void*)&gGc0307.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gGc0307.sceneMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_MAX_EXPOSURE_COMPENSATION:
			value = 4;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_MIN_EXPOSURE_COMPENSATION:
			value = -4;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_EXPOSURE_COMPENSATION_STEP:
			value = 1;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_EXPOSURE_COMPENSATION:
			memcpy(p, (void*)&gGc0307.expVal, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_ANTIBANDING_MODE:
			value = CAM_ANTIBANDING_MODE_OFF |
				CAM_ANTIBANDING_MODE_50HZ |
				CAM_ANTIBANDING_MODE_60HZ |
				CAM_ANTIBANDING_MODE_AUTO |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_ANTIBANDING_MODE:
			memcpy(p, (void*)&gGc0307.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}


camsen_ops gc0307_aih_ops = {
	.name                   = "gc0307_aih",
	.i2c_dev_addr           = GC0307_I2C_ADDR,

	.sen_pwdn				= gc0307_aih_pwdn,
	.sen_get_pmu_info		= gc0307_aih_get_pmu_info,
	.sen_init				= gc0307_aih_init,
	.sen_deinit				= gc0307_aih_deinit,
	.sen_start				= gc0307_aih_start,
	.sen_stop				= gc0307_aih_stop,
	.sen_get_caps			= gc0307_aih_get_caps,
	.sen_set_out_mode		= gc0307_aih_set_out_mode,
	.sen_get_out_mode		= gc0307_aih_get_out_mode,
	.sen_set_property		= gc0307_aih_set_property,
	.sen_get_property		= gc0307_aih_get_property,
};
