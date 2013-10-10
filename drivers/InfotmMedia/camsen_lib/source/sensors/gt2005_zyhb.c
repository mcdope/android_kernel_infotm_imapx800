/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gt2005_zyhb.c
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
#include "gt2005_zyhb.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"GT2005_ZYHB_I:"
#define WARNHEAD	"GT2005_ZYHB_W:"
#define ERRHEAD		"GT2005_ZYHB_E:"
#define TIPHEAD		"GT2005_ZYHB_T:"

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
}gt2005_zyhb_context_t;

static gt2005_zyhb_context_t gGt2005;
static pwl_handle_t gPwl = IM_NULL;

static IM_UINT16 GT2005_VENID0 = 0x0000;
static IM_UINT16 GT2005_VENID1 = 0x0100;
static IM_UINT16 GT2005_0012 = 0x1200;
static IM_UINT16 GT2005_0013 = 0x1300;
static IM_UINT16 GT2005_0014 = 0x1400;
static IM_UINT16 GT2005_0015 = 0x1500;
static IM_UINT16 GT2005_0016 = 0x1600;
static IM_UINT16 GT2005_0017 = 0x1700;
static IM_UINT16 GT2005_0314 = 0x1403;

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

#define cam_gt2005_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 2, d)
#define cam_gt2005_write(b) camsenpwl_i2c_write(gPwl, b, 3)

IM_INT32 gt2005_zyhb_svga_to_uxga(void){
	IM_UINT16 shutter = 0;		
	IM_UINT16 AGain_shutter = 0;
	IM_UINT16 DGain_shutter = 0;

	IM_UINT16 shutterH = 0;
	IM_UINT16 shutterL = 0;
	IM_UINT16 AGain_shutterH = 0;
	IM_UINT16 AGain_shutterL = 0;
	IM_UINT16 DGain_shutterH = 0;
	IM_UINT16 DGain_shutterL = 0;

	IM_CHAR reg0x0304 = 0;
	IM_CHAR reg0x0305 = 0;
	IM_CHAR reg0x0306 = 0;
	IM_CHAR reg0x0307 = 0;
	IM_CHAR reg0x0308 = 0;


	struct gt2005_regval_list gt2005_0300[] = {
		{0x03, 0x00, 0xc1},
	};

	struct gt2005_regval_list gt2005_0304[] = {
		{0x03, 0x04, 0x00},
	};

	struct gt2005_regval_list gt2005_0305[] = {
		{0x03, 0x05, 0x00},
	};

	struct gt2005_regval_list gt2005_0306[] = {
		{0x03, 0x06, 0x00},
	};

	struct gt2005_regval_list gt2005_0307[] = {
		{0x03, 0x07, 0x00},
	};

	struct gt2005_regval_list gt2005_0308[] = {
		{0x03, 0x08, 0x00},
	};

	/*****/
	gt2005_0300->value = 0xc1;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0300));  

	cam_gt2005_read((IM_UINT8 *)(&GT2005_0012), (IM_UINT8 *)&shutterH, 1);   	
	cam_gt2005_read((IM_UINT8 *)(&GT2005_0013), (IM_UINT8 *)&shutterL, 1);

	
	cam_gt2005_read((IM_UINT8 *)(&GT2005_0014), (IM_UINT8 *)&AGain_shutterH, 1);
	cam_gt2005_read((IM_UINT8 *)(&GT2005_0015), (IM_UINT8 *)&AGain_shutterL, 1);

//	AGain_shutter = ((AGain_shutterH << 8) | AGain_shutterL);
	
	cam_gt2005_read((IM_UINT8 *)(&GT2005_0016), (IM_UINT8 *)&DGain_shutterH, 1);
	cam_gt2005_read((IM_UINT8 *)(&GT2005_0017), (IM_UINT8 *)&DGain_shutterL, 1);

	DGain_shutter = ((DGain_shutterH << 8) | (DGain_shutterL&0xff));

	DGain_shutter = DGain_shutter>>2;	
	
	shutter = ((shutterH << 8) |(shutterL&0xff));

/*
 *	Close ALC
 */
	gt2005_0300->value = 0x41;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0300));  

	//shutter = shutter / 2;

	reg0x0304 = (shutter >> 8) ;
	gt2005_0304->value = reg0x0304;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0304));  

	
	reg0x0305 = shutter & 0xff ;
	gt2005_0305->value = reg0x0305;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0305));  
		
	
	reg0x0307 = AGain_shutterL ;
	gt2005_0307->value = reg0x0307;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0307));

	reg0x0306 = AGain_shutterH;
	gt2005_0306->value = reg0x0306;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0306));

	reg0x0308 = DGain_shutter;
	gt2005_0308->value = reg0x0308;
	cam_gt2005_write((IM_UINT8 *)(gt2005_0308));

	IM_CHAR buf = 0x69;
    cam_gt2005_read((IM_UINT8 *)(&GT2005_0314), &buf, 1);
    IM_INFOMSG((IM_STR("%s(reg0314 = 0x%x)"), IM_STR(_IM_FUNC_), buf));
    cam_gt2005_read((IM_UINT8 *)(&GT2005_VENID0), &buf, 1);
    IM_INFOMSG((IM_STR("%s(ID1 = 0x%x, real value is 0x51)"), IM_STR(_IM_FUNC_), buf));
    cam_gt2005_read((IM_UINT8 *)(&GT2005_VENID1), &buf, 1);
    IM_INFOMSG((IM_STR("%s(ID2 = 0x%x, real value is 0x38)"), IM_STR(_IM_FUNC_), buf));

    msleep(200);

    return 0;
}

IM_INT32 gt2005_zyhb_switch_uxga(void){ 
    IM_INT32 i, ret;

    for(i = 0; i < (sizeof(gt2005_uxga_regs) / 3); i++)
    {
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_uxga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}

    gt2005_zyhb_svga_to_uxga();

	return 0; 
}

IM_INT32 gt2005_zyhb_switch_svga(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_svga_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_svga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_switch_vga(void){ 
	IM_INT32 i, ret;

	for(i = 0; i < (sizeof(gt2005_vga_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_vga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}


IM_INT32 gt2005_zyhb_set_sepia(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_sepia_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_sepia_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_bluish(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_bluish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_bluish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_greenish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_greenish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_greenish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_reddish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_reddish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_reddish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_yellowish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_yellowish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_yellowish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_bandw(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_bandw_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_bandw_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_negative(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_negative_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_negative_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_normal(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_normal_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_normal_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_auto(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_auto_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_auto_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_sunny(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_sunny_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_sunny_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_cloudy(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_cloudy_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_cloudy_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_office(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_office_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_office_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_home(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gt2005_home_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_home_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gt2005_zyhb_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGt2005.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			gt2005_zyhb_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			gt2005_zyhb_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			gt2005_zyhb_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			gt2005_zyhb_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			gt2005_zyhb_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			gt2005_zyhb_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			gt2005_zyhb_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			gt2005_zyhb_set_greenish(); 
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

	gGt2005.efType = value;

	return 0;
}

IM_INT32 gt2005_zyhb_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gGt2005.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			gt2005_zyhb_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			gt2005_zyhb_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			gt2005_zyhb_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			gt2005_zyhb_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			gt2005_zyhb_set_cloudy();
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

	gGt2005.wbMode = value;

	msleep(10);

	return 0;
}

IM_INT32 gt2005_zyhb_set_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		for(i = 0; i < (sizeof(gt2005_night_regs) / 3); i++)
		{
			ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_night_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}
	else  				/* Normal Mode */
	{
		for(i = 0; i < (sizeof(gt2005_sunset_regs) / 3); i++)
		{
			ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_sunset_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}

	return 0;
}

IM_INT32 gt2005_zyhb_set_scene_mode(IM_INT32 value)
{
	if(value == gGt2005.sceneMode)
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

	gGt2005.sceneMode = value;

	return gt2005_zyhb_set_night_mode(nightModeEn);
}

IM_INT32 gt2005_zyhb_set_exposure(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGt2005.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_neg4_regs));
			break;
		case -3:							/* EV -1.5 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_neg3_regs));
			break;
		case -2:							/* EV -1 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_neg2_regs));
			break;
		case -1:							/* EV -0.5 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_neg1_regs));
			break;
		case 0:								/* EV 0 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_zero_regs));
			break;
		case 1:							/* EV +0.5 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_pos1_regs));
			break;
		case 2:							/* EV +1 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_pos2_regs));
			break;
		case 3:							/* EV +1.5 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_pos3_regs));
			break;
		case 4:							/* EV +2 */
			cam_gt2005_write((IM_UINT8 *)(&gt2005_ev_pos4_regs));
			break;
		default:
			return -1;
	}

	gGt2005.expVal = value;

	msleep(20);

	return 0;
}

IM_INT32 gt2005_zyhb_set_antibanding(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGt2005.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_50HZ:
			cam_gt2005_write((IM_UINT8 *)(&gt2005_50hz_regs));
			break;

		case CAM_ANTIBANDING_MODE_60HZ:
			cam_gt2005_write((IM_UINT8 *)(&gt2005_60hz_regs));
			break;

		default:
			return -1;
	}

	gGt2005.bandMode = value;

	return 0;
}

//------- for flashlight
IM_INT32 gt2005_zyhb_flash_on(IM_INT32 on)
{
	IM_INFOMSG((IM_STR("%s(GC2015 FLASH ON= %d)"), IM_STR(_IM_FUNC_), on));
    if(0 == gGt2005.flashLight)
    {
        return 0;
    }
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGt2005.flashLight, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGt2005.flashLight, 0);

	camsenpwl_io_set_outdat(gGt2005.flashLight, on);

	return 0;
}

IM_INT32 gt2005_zyhb_set_flash_mode(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gGt2005.flashMode)
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
	
	gGt2005.flashMode = value;
	return 0;
}

IM_INT32 gt2005_zyhb_set_light_on(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gGt2005.light)
	{
		return 0;
	}
	
	switch (value) {
	case 0:
        gt2005_zyhb_flash_on(0);
		break;
	case 1:
        gt2005_zyhb_flash_on(1);
		break;   
	default:
		return -1;
	}
	
	gGt2005.light = value;
	return 0;
}


//======================================================
IM_RET gt2005_zyhb_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(GT2005_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

	//set PWDN  state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(padNum, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(padNum, 0);

	/******************************
	*power down enable
	******************************/
	//set io outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
	camsenpwl_io_set_outdat(padNum, 0);

	return IM_RET_OK;
}

IM_RET gt2005_zyhb_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET gt2005_zyhb_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	IM_CHAR buf = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gGt2005, 0x0, sizeof(gGt2005));
	//init value
	gGt2005.wbMode = CAM_WB_MODE_AUTO;
	gGt2005.efType = CAM_SPECIAL_EFFECT_NONE;
	gGt2005.expVal = 0;
	gGt2005.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gGt2005.sceneMode = CAM_SCENE_MODE_AUTO;
   gGt2005.flashMode = CAM_FLASH_MODE_OFF;
	gGt2005.light = 0;

	gGt2005.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(GT2005_PWDN= %d)"), IM_STR(_IM_FUNC_), gGt2005.pwdn));
	gGt2005.reset = camsenpwl_get_reset_padnum(gPwl);

	gGt2005.flashLight = camsenpwl_get_flash_light_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(GC2015_FLASHLIGHT= %d)"), IM_STR(_IM_FUNC_), gGt2005.flashLight));	

	if (gGt2005.flashLight != 0)
	{
		gt2005_zyhb_flash_on(0);
	}

	//config io
	/*io_index138(XCAMD12)<-->gt2005.MCLK, io_indexgGt2005.reset(XCAMD16)<-->gt2005.RESET,
	 * io_index136(XCAMD10)<-->gt2005.PWDN, io_index<-->gt2005.POWER*/

	/******************************
	*set each io mode
	******************************/
	//set IO mode, default is case0
	//volatile unsigned IM_INT32 *)(0x21e09000) = 0x0;

	//set POWER(io index) state(mode and dir)
	//set io mode =1(use as gpio mode)
	//camsenpwl_io_set_mode(, 1);
	//set io dir =0(output dir)
	//camsenpwl_io_set_dir(, 0);
	
	//set RESET(io index=gGt2005.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGt2005.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGt2005.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGt2005.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGt2005.pwdn, 0);

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
	*reset sensor
	******************************/
	//set io(index=gGt2005.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gGt2005.reset, 1);
	camsenpwl_io_set_outdat(gGt2005.reset, 0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gGt2005.reset, 1);
	
	msleep(5);

	/******************************
	*power down disable
	******************************/
	//set io outdata = 1(pwdn also  need rising edge, high active to disable pwdn, set 0x21e09110 bit4 to 1)
	camsenpwl_io_set_outdat(gGt2005.pwdn, 0);
	
	camsenpwl_io_set_outdat(gGt2005.pwdn, 1);

	msleep(5);

	//read sensor id
	if(checkOnly == IM_TRUE)
	{
		cam_gt2005_read((IM_UINT8 *)(&GT2005_VENID0), &buf, 1);
		IM_INFOMSG((IM_STR("%s(ID1 = 0x%x, real value is 0x51)"), IM_STR(_IM_FUNC_), buf));
        if(buf != 0x51)
		{
			IM_ERRMSG((IM_STR("gt2005 id error!")));
            goto Fail;
		}
		cam_gt2005_read((IM_UINT8 *)(&GT2005_VENID1), &buf, 1);
		IM_INFOMSG((IM_STR("%s(ID2 = 0x%x, real value is 0x38)"), IM_STR(_IM_FUNC_), buf));
		if(buf != 0x38)
		{
			IM_ERRMSG((IM_STR("gt2005 id error!")));
			goto Fail;
		}
		return IM_RET_OK;
	}

	buf = 0;

	for(i = 0; i < (sizeof(gt2005_init_regs) / 3); i++)
	{
		ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_init_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			goto Fail;
		}
		else{
		}
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(gGt2005.pwdn, 0);
	//reset sensor
	camsenpwl_io_set_outdat(gGt2005.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET gt2005_zyhb_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gGt2005.pwdn, 0);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gGt2005.reset, 0);

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

IM_RET gt2005_zyhb_start(void)
{
	IM_INT32 ret;
	struct gt2005_regval_list  gt2005_streaming = {0x01, 0x04, 0x03};
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_streaming));
	if(ret != 0)
	{
		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
		return IM_RET_FAILED;
	}
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET gt2005_zyhb_stop(void)
{
	IM_INT32 ret;
	struct gt2005_regval_list  gt2005_streaming = {0x01, 0x04, 0x00};
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	ret = cam_gt2005_write((IM_UINT8 *)(&gt2005_streaming));
	if(ret != 0)
	{
		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
		return IM_RET_FAILED;
	}
	return ret;
}

IM_RET gt2005_zyhb_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA | CAM_RES_SVGA |CAM_RES_UXGA; 
	caps->maxRes = CAM_RES_UXGA; 
	caps->initRes = CAM_RES_SVGA;

	return ret;
}

IM_RET gt2005_zyhb_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET gt2005_zyhb_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_UXGA)
	{
		ret = gt2005_zyhb_switch_uxga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gt2005_zyhb_switch_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = gt2005_zyhb_switch_svga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gt2005_zyhb_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_VGA)
	{
		ret = gt2005_zyhb_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gt2005_zyhb_switch_vga() failed!")));
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

IM_RET gt2005_zyhb_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		gt2005_zyhb_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		gt2005_zyhb_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		gt2005_zyhb_set_scene_mode(value);
	}
	//---- for flashlight
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		gt2005_zyhb_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		gt2005_zyhb_set_light_on(value);
	}
	//-------------
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		gt2005_zyhb_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		gt2005_zyhb_set_antibanding(value);
	}

	return ret;
}

IM_RET gt2005_zyhb_get_property(IM_UINT32 property, void *p)
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
			memcpy(p, (void*)&gGt2005.wbMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGt2005.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gGt2005.sceneMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGt2005.flashMode, sizeof(IM_INT32));
			break; 
		case CAM_KEY_RW_LIGHT_TURN_ON: 
			memcpy(p, (void*)&gGt2005.light, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGt2005.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGt2005.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops gt2005_zyhb_ops = {
	.name                   = "gt2005_zyhb",
	.i2c_dev_addr           = GT2005_I2C_ADDR,

	.sen_pwdn				= gt2005_zyhb_pwdn,
	.sen_get_pmu_info		= gt2005_zyhb_get_pmu_info,
	.sen_init				= gt2005_zyhb_init,
	.sen_deinit				= gt2005_zyhb_deinit,
	.sen_start				= gt2005_zyhb_start,
	.sen_stop				= gt2005_zyhb_stop,
	.sen_get_caps			= gt2005_zyhb_get_caps,
	.sen_set_out_mode		= gt2005_zyhb_set_out_mode,
	.sen_get_out_mode		= gt2005_zyhb_get_out_mode,
	.sen_set_property		= gt2005_zyhb_set_property,
	.sen_get_property		= gt2005_zyhb_get_property,
};
