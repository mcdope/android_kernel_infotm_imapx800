/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gc2035_demo.c
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
#include "gc2035_demo.h"

#define DBGINFO		0
#define DBGWARN	    	1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"GC2035_DEMO_I:"
#define WARNHEAD	"GC2035_DEMO_W:"
#define ERRHEAD		"GC2035_DEMO_E:"
#define TIPHEAD		"GC2035_DEMO_T:"

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
}gc2035_demo_context_t;

static gc2035_demo_context_t gGc2035;
static pwl_handle_t gPwl = IM_NULL;

static IM_UINT16 GC2035_VENID0 = 0xF0;
static IM_UINT16 GC2035_VENID1 = 0xF1;
//static IM_UINT16 GC2035_03 = 0x03;
//static IM_UINT16 GC2035_04 = 0x04;

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

#define cam_gc2035_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 1, d)
#define cam_gc2035_write(b) camsenpwl_i2c_write(gPwl, b, 2)
static IM_UINT8 shutterH = 0;
static IM_UINT8 shutterL = 0;
	
IM_INT32 gc2035_demo_svga_to_uxga(void){
	IM_UINT16 shutter = 0;		


	struct gc2035_regval_list gc2035_fe[] = {
		{0xfe, 0x00},
	};
	struct gc2035_regval_list gc2035_b6[] = {
		{0xb6, 0x00},
	};



	struct gc2035_regval_list gc2035_03[] = {
		{0x03, 0x0},
	};
	struct gc2035_regval_list gc2035_04[] = {
		{0x04, 0x0},
	};
	
	printk("%s:snake in.\n", __func__);
	

	/*****/
	gc2035_fe->value = 0x00;
	cam_gc2035_write((IM_UINT8 *)(gc2035_fe));  
	gc2035_b6->value = 0x00;
	cam_gc2035_write((IM_UINT8 *)(gc2035_b6));  


	cam_gc2035_read((IM_UINT8 *)(&gc2035_03), (IM_UINT8 *)&shutterH, 1);   	
	cam_gc2035_read((IM_UINT8 *)(&gc2035_04), (IM_UINT8 *)&shutterL, 1);
	printk("%s: snake gc2035_03 = 0x%x, gc2035_04 = 0x%x.\n", __func__, shutterH, shutterL);
	printk("%s: snake gc2035_03 = 0x%x, gc2035_04 = 0x%x.\n", __func__, gc2035_03[0].value, gc2035_04[0].value);
	shutter = ((shutterH << 8) |(shutterL & 0xff));
	shutter = shutter/2;
	if(shutter< 1) shutter= 1;


	gc2035_03->value = shutter >> 8;
	gc2035_04->value = shutter && 0xff;
	printk("%s:snake shutter = 0x%x, gc2035_03->value = 0x%x.\n", __func__, shutter, gc2035_03->value);
	cam_gc2035_write((IM_UINT8 *)(gc2035_03));  
	cam_gc2035_write((IM_UINT8 *)(gc2035_04)); 

    	msleep(200);

    	return 0;
}

IM_INT32 gc2035_demo_switch_uxga_before(void){
		struct gc2035_regval_list regval;
		IM_UINT8 f0;
		
		regval.value = 0x00;
		regval.reg = 0xfe;		
		cam_gc2035_write((IM_UINT8 *)(&regval));
		
		regval.value = 0x00;
		regval.reg = 0xb6;	
		cam_gc2035_write((IM_UINT8 *)(&regval));		
		
		regval.value = 0x00;
		regval.reg = 0x03;		
		cam_gc2035_read((IM_UINT8 *)(&regval), (IM_UINT8 *)&shutterH, 1);   

		regval.value = 0x00;
		regval.reg = 0x04;					
		cam_gc2035_read((IM_UINT8 *)(&regval), (IM_UINT8 *)&shutterL, 1);		

		regval.value = 0x00;
		regval.reg = 0xf0;					
		cam_gc2035_read((IM_UINT8 *)(&regval), (IM_UINT8 *)&f0, 1);				
		printk("%s: read 0x03 = 0x%x, 0x04 = 0x%x, f0 = 0x%x\n", __func__, shutterH, shutterL, f0);
	  return 0;				
}

IM_INT32 gc2035_demo_switch_uxga_after(void){
		IM_UINT16 shutter = 0;		
		
		struct gc2035_regval_list regval;

		regval.value = 0x00;
		regval.reg = 0xfe;		
		cam_gc2035_write((IM_UINT8 *)(&regval));

		regval.value = 0x00;
		regval.reg = 0xb6;	
		cam_gc2035_write((IM_UINT8 *)(&regval));		
								
		shutter = ((shutterH << 8) |(shutterL & 0xff));
		shutter = shutter/2;
		if(shutter< 1) shutter= 1;
	
		regval.value = shutter >> 8;
		regval.reg = 0x03;			
		cam_gc2035_write((IM_UINT8 *)(&regval)); 	
			
	//	printk("%s:snake shutter = 0x%x, 0x03 = 0x%x, 0x04 = 0x%x.\n", __func__, (shutter >> 8), (shutter && 0xff));
 		regval.value = shutter & 0xff;
		regval.reg = 0x04; 		
		cam_gc2035_write((IM_UINT8 *)(&regval)); 

              msleep(200);		
    return 0;
}


IM_INT32 gc2035_demo_switch_uxga(void){ 
    	IM_INT32 i, ret;

	printk("%s:snake in.\n", __func__);
	
	//gc2035_svga_to_uxga();	
	gc2035_demo_switch_uxga_before();	
	for(i = 0; i < (sizeof(gc2035_uxga_regs) / 2); i++)
	{
			ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_uxga_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
	}
	gc2035_demo_switch_uxga_after();
	return 0; 
}

IM_INT32 gc2035_demo_switch_svga(void){ 
	IM_INT32 i, ret;
	
		printk("%s:snake in.\n", __func__);
	for(i = 0; i < (sizeof(gc2035_svga_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_svga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_switch_vga(void){ 
	IM_INT32 i, ret;

	printk("%s:snake in.\n", __func__);
	for(i = 0; i < (sizeof(gc2035_vga_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_vga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}


IM_INT32 gc2035_demo_set_sepia(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_sepia_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_sepia_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_bluish(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_bluish_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_bluish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_greenish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_greenish_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_greenish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_reddish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_reddish_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_reddish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_yellowish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_yellowish_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_yellowish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_bandw(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_bandw_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_bandw_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_negative(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_negative_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_negative_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_normal(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_normal_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_normal_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_auto(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_auto_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_auto_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_sunny(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_sunny_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_sunny_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_cloudy(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_cloudy_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_cloudy_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_office(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_office_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_office_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_home(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc2035_home_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_home_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 gc2035_demo_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc2035.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			gc2035_demo_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			gc2035_demo_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			gc2035_demo_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			gc2035_demo_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			gc2035_demo_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			gc2035_demo_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			gc2035_demo_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			gc2035_demo_set_greenish(); 
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

	gGc2035.efType = value;

	return 0;
}

IM_INT32 gc2035_demo_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gGc2035.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			gc2035_demo_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			gc2035_demo_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			gc2035_demo_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			gc2035_demo_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			gc2035_demo_set_cloudy();
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

	gGc2035.wbMode = value;

	msleep(10);

	return 0;
}

IM_INT32 gc2035_demo_set_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		for(i = 0; i < (sizeof(gc2035_night_regs) / 2); i++)
		{
			ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_night_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}
	else  				/* Normal Mode */
	{
		for(i = 0; i < (sizeof(gc2035_sunset_regs) / 2); i++)
		{
			ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_sunset_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}

	return 0;
}

IM_INT32 gc2035_demo_set_scene_mode(IM_INT32 value)
{
	IM_BOOL nightModeEn;
	
	if(value == gGc2035.sceneMode)
	{
		return 0;
	}

	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if((value == CAM_SCENE_MODE_NIGHT) || (value == CAM_SCENE_MODE_NIGHT_PORTRAIT))	/*set night mode*/
	{
		nightModeEn = IM_TRUE;
	}
	else
	{
		nightModeEn = IM_FALSE;
	}

	gGc2035.sceneMode = value;

	return gc2035_demo_set_night_mode(nightModeEn);
}

IM_INT32 gc2035_demo_set_exposure(IM_INT32 value)
{
	IM_INT32 i, ret;

	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc2035.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
			for(i = 0; i < (sizeof(gc2035_ev_neg4_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_neg4_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -3:							/* EV -1.5 */
			for(i = 0; i < (sizeof(gc2035_ev_neg3_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_neg3_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -2:							/* EV -1 */
			for(i = 0; i < (sizeof(gc2035_ev_neg2_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_neg2_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case -1:							/* EV -0.5 */
			for(i = 0; i < (sizeof(gc2035_ev_neg1_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_neg1_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 0:								/* EV 0 */
			for(i = 0; i < (sizeof(gc2035_ev_zero_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_zero_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 1:							/* EV +0.5 */
			for(i = 0; i < (sizeof(gc2035_ev_pos1_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_pos1_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 2:							/* EV +1 */
			for(i = 0; i < (sizeof(gc2035_ev_pos2_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_pos2_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 3:							/* EV +1.5 */
			for(i = 0; i < (sizeof(gc2035_ev_pos3_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_pos3_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case 4:							/* EV +2 */
			for(i = 0; i < (sizeof(gc2035_ev_pos4_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_ev_pos4_regs[i]));
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

	gGc2035.expVal = value;

	msleep(20);

	return 0;
}

IM_INT32 gc2035_demo_set_antibanding(IM_INT32 value)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc2035.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_50HZ:
			for(i = 0; i < (sizeof(gc2035_night_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_50hz_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
			break;
		case CAM_ANTIBANDING_MODE_60HZ:
			for(i = 0; i < (sizeof(gc2035_night_regs) / 2); i++)
			{
				ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_60hz_regs[i]));
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

	gGc2035.bandMode = value;

	return 0;
}

//------- for flashlight
IM_INT32 gc2035_demo_flash_on(IM_INT32 on)
{
	IM_INFOMSG((IM_STR("%s(gc2035 FLASH ON= %d)"), IM_STR(_IM_FUNC_), on));
    if(0 == gGc2035.flashLight)
    {
        return 0;
    }
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGc2035.flashLight, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc2035.flashLight, 0);

	camsenpwl_io_set_outdat(gGc2035.flashLight, on);

	return 0;
}

IM_INT32 gc2035_demo_set_flash_mode(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gGc2035.flashMode)
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
	
	gGc2035.flashMode = value;
	return 0;
}

IM_INT32 gc2035_demo_set_light_on(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gGc2035.light)
	{
		return 0;
	}
	
	switch (value) {
	case 0:
        gc2035_demo_flash_on(0);
		break;
	case 1:
        gc2035_demo_flash_on(1);
		break;   
	default:
		return -1;
	}
	
	gGc2035.light = value;
	return 0;
}


//======================================================
IM_RET gc2035_demo_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(gc2035_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET gc2035_demo_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET gc2035_demo_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	IM_CHAR buf = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gGc2035, 0x0, sizeof(gGc2035));
	//init value
	gGc2035.wbMode = CAM_WB_MODE_AUTO;
	gGc2035.efType = CAM_SPECIAL_EFFECT_NONE;
	gGc2035.expVal = 0;
	gGc2035.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gGc2035.sceneMode = CAM_SCENE_MODE_AUTO;
	gGc2035.flashMode = CAM_FLASH_MODE_OFF;
	gGc2035.light = 0;

	gGc2035.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	gGc2035.reset = camsenpwl_get_reset_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(gc2035_PWDN= %d)"), IM_STR(_IM_FUNC_), gGc2035.pwdn));

	gGc2035.flashLight = camsenpwl_get_flash_light_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(gc2035_FLASHLIGHT= %d)"), IM_STR(_IM_FUNC_), gGc2035.flashLight));	

	if (gGc2035.flashLight != 0)
	{
		gc2035_demo_flash_on(0);
	}

	//config io

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
	
	//set RESET state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGc2035.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc2035.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGc2035.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc2035.pwdn, 0);

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
	//set io outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gGc2035.reset, 1);
	camsenpwl_io_set_outdat(gGc2035.reset, 0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gGc2035.reset, 1);
	
	msleep(5);

	/******************************
	*power down disable
	******************************/
	//set io outdata = 1(pwdn also  need rising edge, high active to disable pwdn, set 0x21e09110 bit4 to 1)
	camsenpwl_io_set_outdat(gGc2035.pwdn, 1);
	
	camsenpwl_io_set_outdat(gGc2035.pwdn, 0);

	msleep(5);

	//read sensor id
	if(checkOnly == IM_TRUE)
	{
		cam_gc2035_read((IM_UINT8 *)(&GC2035_VENID0), &buf, 1);
		IM_INFOMSG((IM_STR("%s(ID1 = 0x%x, real value is 0x20)"), IM_STR(_IM_FUNC_), buf));
        	if(buf != 0x20)
		{
			IM_ERRMSG((IM_STR("gc2035 id error!")));
			goto Fail;
		}
		cam_gc2035_read((IM_UINT8 *)(&GC2035_VENID1), &buf, 1);
		IM_INFOMSG((IM_STR("%s(ID2 = 0x%x, real value is 0x35)"), IM_STR(_IM_FUNC_), buf));
		if(buf != 0x35)
		{
			IM_ERRMSG((IM_STR("gc2035 id error!")));
			goto Fail;
		}
		return IM_RET_OK;
	}

	buf = 0;

	for(i = 0; i < (sizeof(gc2035_init_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_init_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			goto Fail;
		}
		else{
		}
	}

	//set vga preview , sensor use svga resolution
	for(i = 0; i < (sizeof(gc2035_svga_regs) / 2); i++)
	{
		ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_svga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			goto Fail;
		}
		else{
		}
	}



	msleep(100);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));


	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(gGc2035.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(gGc2035.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET gc2035_demo_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gGc2035.pwdn, 1);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gGc2035.reset, 0);

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

IM_RET gc2035_demo_start(void)
{
	//IM_INT32 ret;
//	struct gc2035_regval_list  gc2035_streaming = {0x01, 0x04, 0x03};
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
//	ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_streaming));
//	if(ret != 0)
//	{
//		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
//		return IM_RET_FAILED;
//	}
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET gc2035_demo_stop(void)
{
	//IM_INT32 ret;
//	struct gc2035_regval_list  gc2035_streaming = {0x01, 0x04, 0x00};
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
//	ret = cam_gc2035_write((IM_UINT8 *)(&gc2035_streaming));
//	if(ret != 0)
//	{
//		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
//		return IM_RET_FAILED;
//	}
	return IM_RET_OK;
}

IM_RET gc2035_demo_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	printk("%s:snake in.\n", __func__);
	caps->supportRes = CAM_RES_SVGA /*| CAM_RES_SVGA */|CAM_RES_UXGA; 
	caps->maxRes = CAM_RES_UXGA; 
	caps->initRes = CAM_RES_SVGA;

	return ret;
}

IM_RET gc2035_demo_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET gc2035_demo_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	printk("set mode , res is %x -----------------xxxx000000000000000000000000000\r\n", outMode->res);
	
	if(res == CAM_RES_UXGA)
	{
		ret = gc2035_demo_switch_uxga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_demo_switch_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = gc2035_demo_switch_svga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_demo_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_VGA)
	{
		ret = gc2035_demo_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_demo_switch_vga() failed!")));
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

IM_RET gc2035_demo_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		gc2035_demo_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		gc2035_demo_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		gc2035_demo_set_scene_mode(value);
	}
	//---- for flashlight
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		gc2035_demo_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		gc2035_demo_set_light_on(value);
	}
	//-------------
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		gc2035_demo_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		gc2035_demo_set_antibanding(value);
	}

	return ret;
}

IM_RET gc2035_demo_get_property(IM_UINT32 property, void *p)
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
			memcpy(p, (void*)&gGc2035.wbMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc2035.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gGc2035.sceneMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc2035.flashMode, sizeof(IM_INT32));
			break; 
		case CAM_KEY_RW_LIGHT_TURN_ON: 
			memcpy(p, (void*)&gGc2035.light, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc2035.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc2035.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops gc2035_demo_ops = {
	.name                   = "gc2035_demo",
	.i2c_dev_addr           = gc2035_I2C_ADDR,

	.sen_pwdn				= gc2035_demo_pwdn,
	.sen_get_pmu_info		= gc2035_demo_get_pmu_info,
	.sen_init				= gc2035_demo_init,
	.sen_deinit				= gc2035_demo_deinit,
	.sen_start				= gc2035_demo_start,
	.sen_stop				= gc2035_demo_stop,
	.sen_get_caps			= gc2035_demo_get_caps,
	.sen_set_out_mode		= gc2035_demo_set_out_mode,
	.sen_get_out_mode		= gc2035_demo_get_out_mode,
	.sen_set_property		= gc2035_demo_set_property,
	.sen_get_property		= gc2035_demo_get_property,
};
