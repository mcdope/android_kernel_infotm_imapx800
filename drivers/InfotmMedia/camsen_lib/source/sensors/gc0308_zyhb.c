/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gc0308_zyhb.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/05/07: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "gc0308_zyhb.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"GC0308_ZYHB_I:"
#define WARNHEAD	"GC0308_ZYHB_W:"
#define ERRHEAD		"GC0308_ZYHB_E:"
#define TIPHEAD		"GC0308_ZYHB_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
}gc0308_zyhb_context_t;

static gc0308_zyhb_context_t gGc0308;
static pwl_handle_t gPwl = IM_NULL;
static IM_CHAR GC0308_ID = 0x00;

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

#define cam_gc0308_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 1, d)
#define cam_gc0308_write(b) camsenpwl_i2c_write(gPwl, b, 2)


int gc0308_zyhb_switch_qvga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gc0308_qvga_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_qvga_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
		msleep(10);
	}
	return 0;
}
int gc0308_zyhb_switch_vga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gc0308_vga_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_vga_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
		msleep(10);
	}
	return 0;
}	

IM_INT32 gc0308_zyhb_set_sepia(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_sepia_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_sepia_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_bluish(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_bluish_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_bluish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_greenish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_greenish_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_greenish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_reddish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_reddish_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_reddish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_yellowish(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_yellowish_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_yellowish_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_bandw(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_bandw_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_bandw_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_negative(void){
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_negative_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_negative_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_normal(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_normal_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_normal_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_auto(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_auto_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_auto_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_sunny(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_sunny_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_sunny_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_cloudy(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_cloudy_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_cloudy_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_office(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_office_regs) / 2); i++)
	{
		ret = cam_gc0308_write( (IM_UINT8 *)(&gc0308_office_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}
IM_INT32 gc0308_zyhb_set_home(void){ 
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(gc0308_home_regs) / 2); i++)
	{
		ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_home_regs[i]));
		if(ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return -1;
		}
	}
	return 0; 
}


IM_INT32 gc0308_zyhb_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc0308.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			gc0308_zyhb_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			gc0308_zyhb_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			gc0308_zyhb_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			gc0308_zyhb_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			gc0308_zyhb_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			gc0308_zyhb_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			gc0308_zyhb_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			gc0308_zyhb_set_greenish(); 
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

	gGc0308.efType = value;

	return 0;
}

IM_INT32 gc0308_zyhb_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gGc0308.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			gc0308_zyhb_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			gc0308_zyhb_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			gc0308_zyhb_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			gc0308_zyhb_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			gc0308_zyhb_set_cloudy();
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

	gGc0308.wbMode = value;

	return 0;
}

IM_INT32 gc0308_zyhb_set_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		if(gGc0308.bandMode == CAM_ANTIBANDING_MODE_60HZ)
		{
			for(i = 0; i < (sizeof(gc0308_night_60hz_regs) / 2); i++)
			{
				ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_night_60hz_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
		else
		{
			for(i = 0; i < (sizeof(gc0308_night_50hz_regs) / 2); i++)
			{
				ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_night_50hz_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
	}
	else  				/* Normal Mode */
	{
		if(gGc0308.bandMode == CAM_ANTIBANDING_MODE_60HZ)
		{
			for(i = 0; i < (sizeof(gc0308_sunset_60hz_regs) / 2); i++)
			{
				ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_sunset_60hz_regs[i]));
				if(ret != 0)
				{
					IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
					return ret;
				}
			}
		}
		else
		{
			for(i = 0; i < (sizeof(gc0308_sunset_50hz_regs) / 2); i++)
			{
				ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_sunset_50hz_regs[i]));
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

IM_INT32 gc0308_zyhb_set_scene_mode(IM_INT32 value)
{
	if(value == gGc0308.sceneMode)
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

	gGc0308.sceneMode = value;

	return gc0308_zyhb_set_night_mode(nightModeEn);
}

IM_INT32 gc0308_zyhb_set_exposure(IM_INT32 value)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc0308.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
            for(i = 0; i < (sizeof(gc0308_ev_neg4_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_neg4_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -3:							/* EV -1.5 */
            for(i = 0; i < (sizeof(gc0308_ev_neg3_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_neg3_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -2:							/* EV -1 */
            for(i = 0; i < (sizeof(gc0308_ev_neg2_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_neg2_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case -1:							/* EV -0.5 */
            for(i = 0; i < (sizeof(gc0308_ev_neg1_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_neg1_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 0:								/* EV 0 */
            for(i = 0; i < (sizeof(gc0308_ev_zero_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_zero_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 1:							/* EV +0.5 */
            for(i = 0; i < (sizeof(gc0308_ev_pos1_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_pos1_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 2:							/* EV +1 */
            for(i = 0; i < (sizeof(gc0308_ev_pos2_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_pos2_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 3:							/* EV +1.5 */
            for(i = 0; i < (sizeof(gc0308_ev_pos3_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_pos3_regs[i]));
                if(ret != 0)
                {
                    IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                    return ret;
                }
            }
			break;
		case 4:							/* EV +2 */
            for(i = 0; i < (sizeof(gc0308_ev_pos4_regs) / 2); i++)
            {
                ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_ev_pos4_regs[i]));
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

	gGc0308.expVal = value;

	return 0;
}

IM_INT32 gc0308_zyhb_set_antibanding(IM_INT32 value)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc0308.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_50HZ:
			cam_gc0308_write((IM_UINT8 *)(&gc0308_50hz_regs));
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
			cam_gc0308_write((IM_UINT8 *)(&gc0308_60hz_regs));
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

	gGc0308.bandMode = value;

	return 0;
}

//======================================================
IM_RET gc0308_zyhb_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(GC0308_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET gc0308_zyhb_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

#define REPEAT_TIMES 5
IM_INT32 gc0308_zyhb_repeat_init()
{
    IM_INT32 i, num = 0;
    IM_INT32 ret;
	IM_TIPMSG((IM_STR("#################################%s()##################################"), IM_STR(_IM_FUNC_)));

    do{
        num++;
        IM_TIPMSG((IM_STR("%s()::repeat init num=%d"), IM_STR(_IM_FUNC_), num));
        //power down enable
        camsenpwl_io_set_outdat(gGc0308.pwdn, 1);
        //reset sensor
        camsenpwl_io_set_outdat(gGc0308.reset, 0);
        //close mclk
        camsenpwl_clock_disable(gPwl);

        //set RESET(io index=gGc0308.reset) state(mode and dir)
        //set io mode =1(use as gpio mode)
        camsenpwl_io_set_mode(gGc0308.reset, 1);
        //set io dir =0(output dir)
        camsenpwl_io_set_dir(gGc0308.reset, 0);

        //set pwdn state(mode and dir)
        //set io mode =1(use as gpio mode)
        camsenpwl_io_set_mode(gGc0308.pwdn, 1);
        //set io dir =0(output dir)
        camsenpwl_io_set_dir(gGc0308.pwdn, 0);

        /******************************************************
         *POWER enable: supply power, has enable all the time
         ******************************************************/
        //delay some time
        msleep(5);

        /******************************
         *provided mclk
         ******************************/
        camsenpwl_clock_enable(gPwl, 24000000);//irequest MCLK 24MHz

        //check real clock freq if need
        //camsenpwl_clock_get_freq(gPwl);

        /******************************
         *reset sensor
         ******************************/
        //set io(index=gGc0308.reset) outdata = 1->0->1(reset need rising edge)
        //camsenpwl_io_set_outdat(gGc0308.reset, 1);
        camsenpwl_io_set_outdat(gGc0308.reset, 0);

        msleep(5);

        camsenpwl_io_set_outdat(gGc0308.reset, 1);

        msleep(5);

        /******************************
         *power down disable
         ******************************/
        //set io(index=136) outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
        camsenpwl_io_set_outdat(gGc0308.pwdn, 1);

        camsenpwl_io_set_outdat(gGc0308.pwdn, 0);

        msleep(5);
        //reset sensor than write default value again
        camsenpwl_io_set_outdat(gGc0308.reset, 0);

        msleep(5);

        camsenpwl_io_set_outdat(gGc0308.reset, 1);

        msleep(5);

        for(i = 0; i < (sizeof(gc0308_init_regs) / 2); i++)
        {
            ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_init_regs[i]));
            if(ret != 0)
            {
                IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                break;
            }
        }
        
        if(i == sizeof(gc0308_init_regs)/2)
        {
            return 0;
        }

    }while(num<REPEAT_TIMES);

    return -1;
}

IM_RET gc0308_zyhb_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	IM_CHAR buf = 0;
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gGc0308, 0x0, sizeof(gGc0308));
	//init value
	gGc0308.wbMode = CAM_WB_MODE_AUTO;
	gGc0308.efType = CAM_SPECIAL_EFFECT_NONE;
	gGc0308.expVal = 0;
	gGc0308.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gGc0308.sceneMode = CAM_SCENE_MODE_AUTO;

	gGc0308.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	gGc0308.reset = camsenpwl_get_reset_padnum(gPwl);

	//config io
	/*io_index138(XCAMD12)<-->gc0308.MCLK, io_indexgGc0308.reset(XCAMD16)<-->gc0308.RESET,
	 * io_index136(XCAMD10)<-->gc0308.PWDN, io_index_XXX<-->gc0308.POWER*/

	/******************************
	*set each io mode
	******************************/
	//set IO mode, default is case0
	//volatile unsigned IM_INT32 *)(0x21e09000) = 0x0;

	//set POWER(io index) state(mode and dir)
	
	//set RESET(io index=gGc0308.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGc0308.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc0308.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gGc0308.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gGc0308.pwdn, 0);

	/******************************************************
	*POWER enable: supply power, has enable all the time
	******************************************************/
	//delay some time
	msleep(5);

	/******************************
	*provided mclk
	******************************/
	camsenpwl_clock_enable(gPwl, 24000000);//irequest MCLK 24MHz

	//check real clock freq if need
	//camsenpwl_clock_get_freq(gPwl);

	/******************************
	*reset sensor
	******************************/
	//set io(index=gGc0308.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gGc0308.reset, 1);
	camsenpwl_io_set_outdat(gGc0308.reset, 0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gGc0308.reset, 1);
	
	msleep(5);

	/******************************
	*power down disable
	******************************/
	//set io(index=136) outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
	camsenpwl_io_set_outdat(gGc0308.pwdn, 1);
	
	camsenpwl_io_set_outdat(gGc0308.pwdn, 0);
	
	msleep(5);

	//read sensor id
	if(checkOnly == IM_TRUE)
	{
		cam_gc0308_read((IM_UINT8 *)(&GC0308_ID), &buf, 1);
		IM_INFOMSG((IM_STR("%s(GC0308_ID = 0x%x, real value is 0x9b)"), IM_STR(_IM_FUNC_), buf));
	    if(buf != 0x9b)
		{
			IM_ERRMSG((IM_STR("gc0308 id error!")));
			goto Fail;
		}
		return IM_RET_OK;
	}
	buf = 0;

	for(i = 0; i < (sizeof(gc0308_init_regs) / 2); i++)
	{
		ret = cam_gc0308_write((IM_UINT8 *)(&gc0308_init_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			//IM_ERRMSG((IM_STR("reg=0x%x, value=0x%x!"), gc0308_init_regs[i].reg, gc0308_init_regs[i].value));
            if(gc0308_zyhb_repeat_init() != 0)
            {
                goto Fail;
            }
		}
		else{
		}
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(gGc0308.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(gGc0308.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET gc0308_zyhb_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gGc0308.pwdn, 1);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gGc0308.reset, 0);

	/******************************
	*close mclk
	******************************/
	camsenpwl_clock_disable(gPwl);

	/******************************************************
	*power close
	******************************************************/
	//camsenpwl_io_set_outdat(xx, 0);

	gPwl = IM_NULL;

	return ret;
}

IM_RET gc0308_zyhb_start(void)
{
	IM_INT32 i;
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET gc0308_zyhb_stop(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET gc0308_zyhb_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_QVGA | CAM_RES_VGA;
	caps->maxRes = CAM_RES_VGA; 
	caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET gc0308_zyhb_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET gc0308_zyhb_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_VGA)
	{
		ret = gc0308_zyhb_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc0308_zyhb_switch_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_QVGA)
	{
		ret = gc0308_zyhb_switch_qvga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc0308_zyhb_switch_qvga() failed!")));
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

IM_RET gc0308_zyhb_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		gc0308_zyhb_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		gc0308_zyhb_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		gc0308_zyhb_set_scene_mode(value);
	}
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		gc0308_zyhb_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		gc0308_zyhb_set_antibanding(value);
	}

	msleep(20);

	return ret;
}

IM_RET gc0308_zyhb_get_property(IM_UINT32 property, void *p)
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
			memcpy(p, (void*)&gGc0308.wbMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc0308.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gGc0308.sceneMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc0308.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gGc0308.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops gc0308_zyhb_ops = {
	.name                   = "gc0308_zyhb",
	.i2c_dev_addr           = GC0308_I2C_ADDR,

	.sen_pwdn				= gc0308_zyhb_pwdn,
	.sen_get_pmu_info		= gc0308_zyhb_get_pmu_info,
	.sen_init				= gc0308_zyhb_init,
	.sen_deinit				= gc0308_zyhb_deinit,
	.sen_start				= gc0308_zyhb_start,
	.sen_stop				= gc0308_zyhb_stop,
	.sen_get_caps			= gc0308_zyhb_get_caps,
	.sen_set_out_mode		= gc0308_zyhb_set_out_mode,
	.sen_get_out_mode		= gc0308_zyhb_get_out_mode,
	.sen_set_property		= gc0308_zyhb_set_property,
	.sen_get_property		= gc0308_zyhb_get_property,
};

