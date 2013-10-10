/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gc2035_gsg.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/10/19: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <csi_lib.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "gc2035_gsg.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1


#define INFOHEAD	"GC2035_GSG_I:"
#define WARNHEAD	"GC2035_GSG_W:"
#define ERRHEAD		"GC2035_GSG_E:"
#define TIPHEAD		"GC2035_GSG_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
    IM_BOOL csiEn;
}gc2035_gsg_context_t;

static gc2035_gsg_context_t gGc2035;
static pwl_handle_t gPwl = IM_NULL;

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

static IM_INT32 cam_gc2035_read(IM_UINT8 *reg, IM_UINT8 *value)
{
	return camsenpwl_i2c_read(gPwl, value, reg, REG_ADDR_STEP, REG_DATA_STEP);
}


static IM_INT32 cam_gc2035_write(IM_UINT8 *reg, IM_UINT8 *value)
{
	IM_UINT8 data[REG_STEP];
	IM_INT32 ret,i;
	
	for(i = 0; i < REG_ADDR_STEP; i++)
			data[i] = reg[i];
	for(i = REG_ADDR_STEP; i < REG_STEP; i++)
			data[i] = value[i-REG_ADDR_STEP];
	
	return camsenpwl_i2c_write(gPwl, data, REG_STEP);
}

/*
 * Write a list of register settings;
 */
static IM_INT32 cam_gc2035_write_array(struct gc2035_regval_list *vals , IM_UINT32 size)
{
	IM_INT32 i,ret;
	if (size == 0)
		return -1;

	for(i = 0; i < size ; i++)
	{
		if(vals->reg_num[0] == 0xff) {
			mdelay(vals->value[0]);
		}	
		else {	
            ret = cam_gc2035_write(vals->reg_num, vals->value);
            if(ret != 0)
            {
                IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                return -1;
            }
		}
		vals++;
	}
	
	return 0;
}

static IM_UINT8 shutterH = 0;
static IM_UINT8 shutterL = 0;
	
IM_INT32 gc2035_gsg_vga_to_uxga(void){
	IM_UINT16 shutter = 0;		

	struct gc2035_regval_list gc2035_fe[] = {
		{{0xfe},{0x00}},
	};
	struct gc2035_regval_list gc2035_b6[] = {
		{{0xb6},{0x00}},
	};

	struct gc2035_regval_list gc2035_03[] = {
		{{0x03},{0x00}},
	};
	struct gc2035_regval_list gc2035_04[] = {
		{{0x04},{0x00}},
	};
	
	/*****/
	gc2035_fe[0].value[0] = 0x00;
    cam_gc2035_write_array(gc2035_fe, ARRAY_SIZE(gc2035_fe));
	gc2035_b6[0].value[0] = 0x00;
    cam_gc2035_write_array(gc2035_b6, ARRAY_SIZE(gc2035_b6));


	cam_gc2035_read(gc2035_03[0].reg_num, (IM_UINT8 *)&shutterH);   	
	cam_gc2035_read(gc2035_04[0].reg_num, (IM_UINT8 *)&shutterL);
	shutter = ((shutterH << 8) |(shutterL & 0xff));
	shutter = shutter/2;
	if(shutter< 1) shutter= 1;

	gc2035_03[0].value[0] = shutter >> 8;
	gc2035_04[0].value[0] = shutter && 0xff;
    cam_gc2035_write_array(gc2035_03, ARRAY_SIZE(gc2035_03));
    cam_gc2035_write_array(gc2035_04, ARRAY_SIZE(gc2035_04));

	msleep(200);

	return 0;
}

IM_INT32 gc2035_gsg_switch_uxga_before(void){
	struct gc2035_regval_list regval;
	IM_UINT8 f0;

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0xfe;		
	cam_gc2035_write(regval.reg_num, regval.value);

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0xb6;	
	cam_gc2035_write(regval.reg_num, regval.value);

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0x03;		
	cam_gc2035_read(regval.reg_num, (IM_UINT8 *)&shutterH);   

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0x04;					
	cam_gc2035_read(regval.reg_num, (IM_UINT8 *)&shutterL);   

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0xf0;					
	cam_gc2035_read(regval.reg_num, &f0);   
	return 0;				
}

IM_INT32 gc2035_gsg_switch_uxga_after(void){
	IM_UINT16 shutter = 0;		

	struct gc2035_regval_list regval;

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0xfe;		
	cam_gc2035_write(regval.reg_num, regval.value);

	regval.value[0] = 0x00;
	regval.reg_num[0] = 0xb6;	
	cam_gc2035_write(regval.reg_num, regval.value);

	shutter = ((shutterH << 8) |(shutterL & 0xff));
	shutter = shutter/2;
	if(shutter< 1) shutter= 1;

	regval.value[0] = shutter >> 8;
	regval.reg_num[0] = 0x03;			
	cam_gc2035_write(regval.reg_num, regval.value);

	regval.value[0] = shutter & 0xff;
	regval.reg_num[0] = 0x04; 		
	cam_gc2035_write(regval.reg_num, regval.value);

	msleep(200);		
	return 0;
}

IM_INT32 gc2035_gsg_switch_uxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//gc2035_gsg_vga_to_uxga();	
	gc2035_gsg_switch_uxga_before();	

	cam_gc2035_write_array(gc2035_uxga_regs, ARRAY_SIZE(gc2035_uxga_regs));

	gc2035_gsg_switch_uxga_after();
	return 0; 
}

IM_INT32 gc2035_gsg_switch_svga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return cam_gc2035_write_array(gc2035_svga_regs, ARRAY_SIZE(gc2035_svga_regs));
}

IM_INT32 gc2035_gsg_switch_vga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return cam_gc2035_write_array(gc2035_vga_regs, ARRAY_SIZE(gc2035_vga_regs));
}

IM_INT32 gc2035_gsg_set_sepia(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return cam_gc2035_write_array(gc2035_sepia_regs, ARRAY_SIZE(gc2035_sepia_regs));
}

IM_INT32 gc2035_gsg_set_bluish(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return cam_gc2035_write_array(gc2035_bluish_regs, ARRAY_SIZE(gc2035_bluish_regs));
}

IM_INT32 gc2035_gsg_set_greenish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return cam_gc2035_write_array(gc2035_greenish_regs, ARRAY_SIZE(gc2035_greenish_regs));
}

IM_INT32 gc2035_gsg_set_reddish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_reddish_regs, ARRAY_SIZE(gc2035_reddish_regs));
}

IM_INT32 gc2035_gsg_set_yellowish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_yellowish_regs, ARRAY_SIZE(gc2035_yellowish_regs));
}

IM_INT32 gc2035_gsg_set_bandw(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_bandw_regs, ARRAY_SIZE(gc2035_bandw_regs));
}

IM_INT32 gc2035_gsg_set_negative(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_negative_regs, ARRAY_SIZE(gc2035_negative_regs));
}

IM_INT32 gc2035_gsg_set_normal(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_normal_regs, ARRAY_SIZE(gc2035_normal_regs));
}

IM_INT32 gc2035_gsg_set_auto(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_auto_regs, ARRAY_SIZE(gc2035_auto_regs));
}

IM_INT32 gc2035_gsg_set_sunny(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_sunny_regs, ARRAY_SIZE(gc2035_sunny_regs));
}

IM_INT32 gc2035_gsg_set_cloudy(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_cloudy_regs, ARRAY_SIZE(gc2035_cloudy_regs));
}

IM_INT32 gc2035_gsg_set_office(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_office_regs, ARRAY_SIZE(gc2035_office_regs));
}

IM_INT32 gc2035_gsg_set_home(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_gc2035_write_array(gc2035_home_regs, ARRAY_SIZE(gc2035_home_regs));
}

IM_INT32 gc2035_gsg_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gGc2035.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			gc2035_gsg_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			gc2035_gsg_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			gc2035_gsg_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			gc2035_gsg_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			gc2035_gsg_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			gc2035_gsg_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			gc2035_gsg_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			gc2035_gsg_set_greenish(); 
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

IM_INT32 gc2035_gsg_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gGc2035.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			gc2035_gsg_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			gc2035_gsg_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			gc2035_gsg_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			gc2035_gsg_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			gc2035_gsg_set_cloudy();
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

IM_INT32 gc2035_gsg_set_night_mode(IM_BOOL enable)
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		cam_gc2035_write_array(gc2035_night_regs, ARRAY_SIZE(gc2035_night_regs));
	}
	else  				/* Normal Mode */
	{
		cam_gc2035_write_array(gc2035_sunset_regs, ARRAY_SIZE(gc2035_sunset_regs));
	}
	return 0;
}

IM_INT32 gc2035_gsg_set_scene_mode(IM_INT32 value)
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

	return gc2035_gsg_set_night_mode(nightModeEn);
}

IM_INT32 gc2035_gsg_set_exposure(IM_INT32 value)
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
			cam_gc2035_write_array(gc2035_ev_neg4_regs, ARRAY_SIZE(gc2035_ev_neg4_regs));
			break;
		case -3:							/* EV -1.5 */
			cam_gc2035_write_array(gc2035_ev_neg3_regs, ARRAY_SIZE(gc2035_ev_neg3_regs));
			break;
		case -2:							/* EV -1 */
			cam_gc2035_write_array(gc2035_ev_neg2_regs, ARRAY_SIZE(gc2035_ev_neg2_regs));
			break;
		case -1:							/* EV -0.5 */
			cam_gc2035_write_array(gc2035_ev_neg1_regs, ARRAY_SIZE(gc2035_ev_neg1_regs));
			break;
		case 0:								/* EV 0 */
			cam_gc2035_write_array(gc2035_ev_zero_regs, ARRAY_SIZE(gc2035_ev_zero_regs));
			break;
		case 1:								/* EV +0.5 */
			cam_gc2035_write_array(gc2035_ev_pos1_regs, ARRAY_SIZE(gc2035_ev_pos1_regs));
			break;
		case 2:								/* EV +1 */
			cam_gc2035_write_array(gc2035_ev_pos2_regs, ARRAY_SIZE(gc2035_ev_pos2_regs));
			break;
		case 3:								/* EV +1.5 */
			cam_gc2035_write_array(gc2035_ev_pos3_regs, ARRAY_SIZE(gc2035_ev_pos3_regs));
			break;
		case 4:								/* EV +2 */
			cam_gc2035_write_array(gc2035_ev_pos4_regs, ARRAY_SIZE(gc2035_ev_pos4_regs));
			break;
		default:
			return -1;
	}

	gGc2035.expVal = value;

	msleep(20);

	return 0;
}

IM_INT32 gc2035_gsg_set_antibanding(IM_INT32 value)
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
			cam_gc2035_write_array(gc2035_50hz_regs, ARRAY_SIZE(gc2035_50hz_regs));
			break;
		case CAM_ANTIBANDING_MODE_60HZ:
			cam_gc2035_write_array(gc2035_60hz_regs, ARRAY_SIZE(gc2035_60hz_regs));
			break;
		default:
			return -1;
	}

	gGc2035.bandMode = value;

	return 0;
}

//======================================================
IM_RET gc2035_gsg_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(GC2035_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET gc2035_gsg_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET gc2035_gsg_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 ret;
    struct  gc2035_regval_list regs;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

    gPwl = pwl;

    camsenpwl_memset((void*)&gGc2035, 0x0, sizeof(gGc2035));
    //init value
    gGc2035.wbMode = CAM_WB_MODE_AUTO;
    gGc2035.efType = CAM_SPECIAL_EFFECT_NONE;
    gGc2035.expVal = 0;
    gGc2035.bandMode = CAM_ANTIBANDING_MODE_OFF;
    gGc2035.sceneMode = CAM_SCENE_MODE_AUTO;

    gGc2035.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
    IM_INFOMSG((IM_STR("%s(GC2035_PWDN= %d)"), IM_STR(_IM_FUNC_), gGc2035.pwdn));
    gGc2035.reset = camsenpwl_get_reset_padnum(gPwl);

    //enable mipi csi
	if(checkOnly != IM_TRUE)
    {
        if(csilib_init(1, 240) != IM_RET_OK) //the freq unit is Mbps(not MHz), so it must calculate from number of lanes.
        {
            IM_ERRMSG((IM_STR("Failed to init csi lib!")));
            return IM_RET_FAILED;
        }

        if(csilib_open() != IM_RET_OK)
        {
            IM_ERRMSG((IM_STR("Failed to open csi lib!")));
            csilib_deinit();
            return IM_RET_FAILED;
        }
        gGc2035.csiEn = IM_TRUE;

        mdelay(100);
    }


    //config io
    /*io_index138(XCAMD12)<-->gc2035.MCLK, io_indexgGc2035.reset(XCAMD16)<-->gc2035.RESET,
	 * io_index136(XCAMD10)<-->gc2035.PWDN*/

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
	
	//set RESET(io index=gGc2035.reset) state(mode and dir)
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
	mdelay(5);

	/******************************
	*provided mclk
	******************************/
	//set MCLK(io index=138) state(mode)
	//set io mode = 0(use as function mode)
	//camsenpwl_io_set_mode(138, 0);
	camsenpwl_clock_enable(gPwl, 24000000);//request MCLK 24MHz

	mdelay(5);

    //disable power down
	camsenpwl_io_set_outdat(gGc2035.pwdn, 0);

	mdelay(5);

    //hardware reset sensor
	camsenpwl_io_set_outdat(gGc2035.reset, 0);

	mdelay(5);

	camsenpwl_io_set_outdat(gGc2035.reset, 1);

	mdelay(10);

    //software reset sensor and wait
    /*regs.reg_num[0] = 0xfe;
    regs.value[0] = 0x80;
    ret = cam_gc2035_write(regs.reg_num, regs.value);
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("GC2035 Failed to write value!")));
    }*/

	mdelay(4);
	
	//read sensor id
	if(checkOnly == IM_TRUE)
	{
        //0xf0
        regs.reg_num[0] = 0xf0;
        regs.value[0] = 0xFF;
        ret = cam_gc2035_read(regs.reg_num, regs.value);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            goto Fail;
        }
        IM_INFOMSG((IM_STR("%s(chipId.value0 = 0x%x, real value is 0x20)"), IM_STR(_IM_FUNC_), regs.value[0]));
        if(regs.value[0] != 0x20)
        {
            IM_ERRMSG((IM_STR("gc2035 id error!")));
            goto Fail;
		}

        //0xf1
        regs.reg_num[0] = 0xf1;
        regs.value[0] = 0xFF;
        ret = cam_gc2035_read(regs.reg_num, regs.value);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            goto Fail;
        }
        IM_INFOMSG((IM_STR("%s(chipId.value1 = 0x%x, real value is 0x35)"), IM_STR(_IM_FUNC_), regs.value[0]));
        if(regs.value[0] != 0x35)
        {
            IM_ERRMSG((IM_STR("gc2035 id error!")));
            //goto Fail;
		}
		return IM_RET_OK;
	}

    ret = cam_gc2035_write_array(gc2035_init_regs, ARRAY_SIZE(gc2035_init_regs));
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        goto Fail;
    }

    ret = cam_gc2035_write_array(gc2035_vga_regs, ARRAY_SIZE(gc2035_vga_regs));
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        goto Fail;
    }

	return IM_RET_OK;

Fail:
    if(gGc2035.csiEn == IM_TRUE)
    {
        //disable mipi csi
        csilib_close();
        csilib_deinit();
    }

	//power down enable
	camsenpwl_io_set_outdat(gGc2035.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(gGc2035.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET gc2035_gsg_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(gGc2035.csiEn == IM_TRUE)
    {
        //disable mipi csi
        csilib_close();
        csilib_deinit();
    }

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

IM_RET gc2035_gsg_start(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET gc2035_gsg_stop(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET gc2035_gsg_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA; 
	caps->maxRes = CAM_RES_UXGA; 
	caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET gc2035_gsg_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}


IM_RET gc2035_gsg_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret = 0;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_VGA)
	{
		ret = gc2035_gsg_switch_vga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_gsg_switch_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = gc2035_gsg_switch_svga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_gsg_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_UXGA)
	{
		ret = gc2035_gsg_switch_uxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("gc2035_gsg_switch_uxga() failed!")));
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

IM_RET gc2035_gsg_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		gc2035_gsg_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		gc2035_gsg_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		gc2035_gsg_set_scene_mode(value);
	}
	/*else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		gc2035_gsg_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		gc2035_gsg_set_light_on(value);
	}*/
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		gc2035_gsg_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		gc2035_gsg_set_antibanding(value);
	}

	return ret;
}

IM_RET gc2035_gsg_get_property(IM_UINT32 property, void *p)
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
				//CAM_CAP_FLASH_MODE_SUPPORT |
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
	    //flash mode
		/*
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
		*/
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

camsen_ops gc2035_gsg_ops = {
	.name                   = "gc2035_gsg",
	.i2c_dev_addr           = GC2035_I2C_ADDR,

	.sen_pwdn				= gc2035_gsg_pwdn,
	.sen_get_pmu_info		= gc2035_gsg_get_pmu_info,
	.sen_init				= gc2035_gsg_init,
	.sen_deinit				= gc2035_gsg_deinit,
	.sen_start				= gc2035_gsg_start,
	.sen_stop				= gc2035_gsg_stop,
	.sen_get_caps			= gc2035_gsg_get_caps,
	.sen_set_out_mode		= gc2035_gsg_set_out_mode,
	.sen_get_out_mode		= gc2035_gsg_get_out_mode,
	.sen_set_property		= gc2035_gsg_set_property,
	.sen_get_property		= gc2035_gsg_get_property,
};
