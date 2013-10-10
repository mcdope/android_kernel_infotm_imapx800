/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: sp2518_demo.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/08/23: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "sp2518_demo.h"

#define DBGINFO		0	
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"SP2518_DEMO_I:"
#define WARNHEAD	"SP2518_DEMO_W:"
#define ERRHEAD		"SP2518_DEMO_E:"
#define TIPHEAD		"SP2518_DEMO_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_UINT32 flashLight;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
	IM_INT32 flashMode;
	IM_INT32 light;
}sp2518_context_t;

static sp2518_context_t sp2518;
static pwl_handle_t gPwl = IM_NULL;


static IM_UINT16 SP2518_VENID = 0x02;

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

#define cam_sp2518_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 1, d)
#define cam_sp2518_write(b) camsenpwl_i2c_write(gPwl, b, 2)

IM_INT32 sp2518_demo_switch_uxga(void)
{ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_uxga_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_uxga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_switch_svga(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_svga_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_svga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_switch_vga(void){ 
	IM_INT32 i, ret;

	for(i = 0; i < (sizeof(sp2518_vga_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_vga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}


IM_INT32 sp2518_demo_set_sepia(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_sepia_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_sepia_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_bluish(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_bluish_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_bluish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_greenish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_greenish_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_greenish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_reddish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_reddish_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_reddish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_yellowish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_yellowish_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_yellowish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_bandw(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_bandw_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_bandw_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_negative(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_negative_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_negative_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_normal(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_normal_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_normal_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_auto(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_auto_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_auto_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_sunny(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_sunny_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_sunny_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_cloudy(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_cloudy_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_cloudy_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_office(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_office_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_office_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_home(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(sp2518_home_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_home_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 sp2518_demo_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == sp2518.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			sp2518_demo_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			sp2518_demo_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			sp2518_demo_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			sp2518_demo_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			sp2518_demo_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			sp2518_demo_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			sp2518_demo_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			sp2518_demo_set_greenish(); 
			break;                   
		case CAM_SPECIAL_EFFECT_MOSAIC:   
			break;                   
		case CAM_SPECIAL_EFFECT_RESIZE: 
			break;                   
		case CAM_SPECIAL_EFFECT_WHITEBOARD:  
			break;                   
		case CAM_SPECIAL_EFFECT_BLACKBOARD:   
			break;                   
		default:                         
			break;                   
	}                                        

	sp2518.efType = value;

	return 0;
}

IM_INT32 sp2518_demo_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == sp2518.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			sp2518_demo_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			sp2518_demo_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			sp2518_demo_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			sp2518_demo_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			sp2518_demo_set_cloudy();
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

	sp2518.wbMode = value;

	msleep(10);

	return 0;
}

IM_INT32 sp2518_demo_set_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		for(i = 0; i < (sizeof(sp2518_night_regs) / 2); i++)
		{
			ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_night_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}
	else  				/* Normal Mode */
	{
		for(i = 0; i < (sizeof(sp2518_sunset_regs) / 2); i++)
		{
			ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_sunset_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}

	return 0;
}

IM_INT32 sp2518_demo_set_scene_mode(IM_INT32 value)
{
	if(value == sp2518.sceneMode)
	{
		return 0;
	}

	IM_BOOL nightModeEn;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if((value == CAM_SCENE_MODE_NIGHT) || (value == CAM_SCENE_MODE_NIGHT_PORTRAIT))	/*set night mode*/
	{
		nightModeEn = IM_TRUE;
	}
	else
	{
		nightModeEn = IM_FALSE;
	}

	sp2518.sceneMode = value;

	return sp2518_demo_set_night_mode(nightModeEn);
}

IM_INT32 sp2518_demo_set_exposure(IM_INT32 value)
{
	IM_INT32 i,ret;
	
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == sp2518.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
			for(i = 0; i < (sizeof(sp2518_ev_neg4_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_neg4_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -3:							/* EV -1.5 */
			for(i = 0; i < (sizeof(sp2518_ev_neg3_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_neg3_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -2:							/* EV -1 */
			for(i = 0; i < (sizeof(sp2518_ev_neg2_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_neg2_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -1:							/* EV -0.5 */
			for(i = 0; i < (sizeof(sp2518_ev_neg1_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_neg1_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 0:								/* EV 0 */
			for(i = 0; i < (sizeof(sp2518_ev_zero_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_zero_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 1:							/* EV +0.5 */
			for(i = 0; i < (sizeof(sp2518_ev_pos1_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_pos1_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 2:							/* EV +1 */
			for(i = 0; i < (sizeof(sp2518_ev_pos2_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_pos2_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 3:							/* EV +1.5 */
			for(i = 0; i < (sizeof(sp2518_ev_pos3_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_pos3_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 4:							/* EV +2 */
			for(i = 0; i < (sizeof(sp2518_ev_pos4_regs) / 2); i++)
			{
				ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_ev_pos4_regs[i]));
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

	sp2518.expVal = value;

	msleep(20);

	return 0;
}

IM_INT32 sp2518_demo_set_antibanding(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == sp2518.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_50HZ:
			cam_sp2518_write((IM_UINT8 *)(&sp2518_50hz_regs));
			break;

		case CAM_ANTIBANDING_MODE_60HZ:
			cam_sp2518_write((IM_UINT8 *)(&sp2518_60hz_regs));
			break;

		default:
			return -1;
	}

	sp2518.bandMode = value;

	return 0;
}

//------- for flashlight
IM_INT32 sp2518_demo_flash_on(IM_INT32 on)
{
	IM_INFOMSG((IM_STR("%s(GC2015 FLASH ON= %d)"), IM_STR(_IM_FUNC_), on));
    if(0 == sp2518.flashLight)
    {
        return 0;
    }
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(sp2518.flashLight, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(sp2518.flashLight, 0);

	camsenpwl_io_set_outdat(sp2518.flashLight, on);

	return 0;
}

IM_INT32 sp2518_demo_set_flash_mode(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == sp2518.flashMode)
	{
		return 0;
	}
	
	switch (value) {
	case CAM_FLASH_MODE_OFF:
		break;
	case CAM_FLASH_MODE_ON:
		break;   
	case CAM_FLASH_MODE_AUTO:
		break;   
	case CAM_FLASH_MODE_TORCH:
		break;   
	case CAM_FLASH_MODE_RED_EYE:   
		break;   
	default:
		return -1;
	}
	
	sp2518.flashMode = value;
	return 0;
}

IM_INT32 sp2518_demo_set_light_on(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == sp2518.light)
	{
		return 0;
	}
	
	switch (value) {
	case 0:
        sp2518_demo_flash_on(0);
		break;
	case 1:
        sp2518_demo_flash_on(1);
		break;   
	default:
		return -1;
	}
	
	sp2518.light = value;
	return 0;
}


//======================================================
IM_RET sp2518_demo_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(SP2518_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET sp2518_demo_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET sp2518_demo_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	IM_CHAR buf = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&sp2518, 0x0, sizeof(sp2518));
	//init value
	sp2518.wbMode = CAM_WB_MODE_AUTO;
	sp2518.efType = CAM_SPECIAL_EFFECT_NONE;
	sp2518.expVal = 0;
	sp2518.bandMode = CAM_ANTIBANDING_MODE_OFF;
	sp2518.sceneMode = CAM_SCENE_MODE_AUTO;
   	sp2518.flashMode = CAM_FLASH_MODE_OFF;
	sp2518.light = 0;

	sp2518.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(SP2518_PWDN= %d)"), IM_STR(_IM_FUNC_), sp2518.pwdn));
	sp2518.reset = camsenpwl_get_reset_padnum(gPwl);

	sp2518.flashLight = camsenpwl_get_flash_light_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(GC2015_FLASHLIGHT= %d)"), IM_STR(_IM_FUNC_), sp2518.flashLight));	

	if (sp2518.flashLight != 0)
	{
		sp2518_demo_flash_on(0);
	}
	
	//set RESET(io index=sp2518.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(sp2518.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(sp2518.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(sp2518.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(sp2518.pwdn, 0);

	/******************************************************
	*POWER enable: supply power, has enable all the time
	******************************************************/
	//set io(index) outdata =1(power enable)
	//camsenpwl_io_set_outdat(, 1);
	//delay some time
	msleep(5);

	/******************************
	*provided mclk
	******************************/
	//set MCLK(io index=138) state(mode)
	//set io mode = 0(use as function mode)
	//camsenpwl_io_set_mode(138, 0);
	camsenpwl_clock_enable(gPwl, 24000000);//irequest MCLK 24MHz

	//check real clock freq if need
	//camsenpwl_clock_get_freq(gPwl);
	
	
	
	/******************************
	*power down disable
	******************************/
	//set io outdata = 1(pwdn also  need rising edge, high active to disable pwdn, set 0x21e09110 bit4 to 1)
	camsenpwl_io_set_outdat(sp2518.pwdn, 0);
	msleep(20);
	camsenpwl_io_set_outdat(sp2518.pwdn, 1);
	msleep(20);
	camsenpwl_io_set_outdat(sp2518.pwdn, 0);
	msleep(20);

	/******************************
	*reset sensor
	******************************/
	//set io(index=sp2518.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(sp2518.reset, 1);
	camsenpwl_io_set_outdat(sp2518.reset, 0);
	msleep(5);
	camsenpwl_io_set_outdat(sp2518.reset, 1);
	msleep(10);



	//read sensor id
	if(checkOnly == IM_TRUE)
	{
		cam_sp2518_read((IM_UINT8 *)(&SP2518_VENID), &buf, 1);
		IM_INFOMSG((IM_STR("%s(ID1 = 0x%x, real value is 0x53)"), IM_STR(_IM_FUNC_), buf));
        	if(buf != 0x53)
		{
			IM_ERRMSG((IM_STR("sp2518 id error!")));
			goto Fail;
		}
		return IM_RET_OK;
	}

	buf = 0;

	for(i = 0; i < (sizeof(sp2518_init_regs) / 2); i++)
	{
		ret = cam_sp2518_write((IM_UINT8 *)(&sp2518_init_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			goto Fail;
		}
	}

	ret = sp2518_demo_switch_vga();
	if(ret != 0)
	{
		IM_ERRMSG((IM_STR("sp2518_switch_vga() failed!")));
		goto Fail;
	}

	msleep(100);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(sp2518.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(sp2518.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET sp2518_demo_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	 *power down enable
	 ******************************/
	camsenpwl_io_set_outdat(sp2518.pwdn, 1);

	/******************************
	 *reset sensor
	 ******************************/
	camsenpwl_io_set_outdat(sp2518.reset, 0);

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

IM_RET sp2518_demo_start(void)
{
	IM_INT32 ret;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET sp2518_demo_stop(void)
{
	IM_INT32 ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET sp2518_demo_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA /*| CAM_RES_SVGA */| CAM_RES_UXGA; 
	caps->maxRes = CAM_RES_UXGA; 
	//caps->initRes = CAM_RES_SVGA; //image is err when set this resolution
	caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET sp2518_demo_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET sp2518_demo_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_UXGA)
	{
		ret = sp2518_demo_switch_uxga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("sp2518_switch_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = sp2518_demo_switch_svga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("sp2518_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_VGA)
	{
		ret = sp2518_demo_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("sp2518_switch_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else
	{
		IM_ERRMSG((IM_STR("this resolution(0x%x)&fps(0x%x) is not supported!"), res, fps));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

IM_RET sp2518_demo_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		sp2518_demo_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		sp2518_demo_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		sp2518_demo_set_scene_mode(value);
	}
	//---- for flashlight
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		sp2518_demo_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		sp2518_demo_set_light_on(value);
	}
	//-------------
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		sp2518_demo_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		sp2518_demo_set_antibanding(value);
	}

	return ret;
}

IM_RET sp2518_demo_get_property(IM_UINT32 property, void *p)
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
				CAM_CAP_FLASH_MODE_SUPPORT |
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
			memcpy(p, (void*)&sp2518.wbMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SPECIAL_EFFECT:
			value = CAM_SPECIAL_EFFECT_NONE |
				CAM_SPECIAL_EFFECT_MONO |
				CAM_SPECIAL_EFFECT_NEGATIVE |
				//CAM_SPECIAL_EFFECT_SOLARIZE |
				//CAM_SPECIAL_EFFECT_PASTEL |
				CAM_SPECIAL_EFFECT_SEPIA |
				//CAM_SPECIAL_EFFECT_POSTERIZE |
				CAM_SPECIAL_EFFECT_AQUA |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy(p, (void*)&sp2518.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&sp2518.sceneMode, sizeof(IM_INT32));
			break;
		//----- for flashlight
		case CAM_KEY_R_SUPPORT_FLASH_MODE:
			value = CAM_FLASH_MODE_OFF |
				//CAM_FLASH_MODE_AUTO |
				CAM_FLASH_MODE_ON |
				//CAM_FLASH_MODE_RED_EYE |
				//CAM_FLASH_MODE_TORCH |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_FLASH_MODE:
			memcpy(p, (void*)&sp2518.flashMode, sizeof(IM_INT32));
			break; 
		case CAM_KEY_RW_LIGHT_TURN_ON: 
			memcpy(p, (void*)&sp2518.light, sizeof(IM_INT32));
			break;
		//---------------

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
			memcpy(p, (void*)&sp2518.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&sp2518.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops sp2518_demo_ops = {
	.name                   = "sp2518_demo",
	.i2c_dev_addr           = SP2518_I2C_ADDR,

	.sen_pwdn				= sp2518_demo_pwdn,
	.sen_get_pmu_info		= sp2518_demo_get_pmu_info,
	.sen_init				= sp2518_demo_init,
	.sen_deinit				= sp2518_demo_deinit,
	.sen_start				= sp2518_demo_start,
	.sen_stop				= sp2518_demo_stop,
	.sen_get_caps			= sp2518_demo_get_caps,
	.sen_set_out_mode		= sp2518_demo_set_out_mode,
	.sen_get_out_mode		= sp2518_demo_get_out_mode,
	.sen_set_property		= sp2518_demo_set_property,
	.sen_get_property		= sp2518_demo_get_property,
};
