/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: ov5650_p48a.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/09/22: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "ov5650_p48a.h"

#define DBGINFO		1
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"OV5650_P48A_I:"
#define WARNHEAD	"OV5650_P48A_W:"
#define ERRHEAD		"OV5650_P48A_E:"
#define TIPHEAD		"OV5650_P48A_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
    IM_BOOL csiEn;
}ov5650_p48a_context_t;

static ov5650_p48a_context_t gOv5650;
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


static IM_INT32 cam_ov5650_read(IM_UINT8 *reg, IM_UINT8 *value)
{
	return camsenpwl_i2c_read(gPwl, value, reg, REG_ADDR_STEP, REG_DATA_STEP);
}


static IM_INT32 cam_ov5650_write(IM_UINT8 *reg, IM_UINT8 *value)
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
static IM_INT32 cam_ov5650_write_array(struct ov5650_regval_list *vals , IM_UINT32 size)
{
	IM_INT32 i,ret;
	if (size == 0)
		return -1;

	for(i = 0; i < size ; i++)
	{
		if(vals->reg_num[0] == 0xff && vals->reg_num[1] == 0xff) {
			mdelay(vals->value[0]);
		}	
		else {	
            ret = cam_ov5650_write(vals->reg_num, vals->value);
            if(ret != 0)
            {
                IM_ERRMSG((IM_STR("Failed to transfer data to i2c, reg_h=0x%x, reg_l=0x%x!"), vals->reg_num[0], vals->reg_num[1]));
                return -1;
            }
		}
		vals++;
	}
	
	return 0;
}

IM_INT32 ov5650_p48a_to_qsxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    
    return cam_ov5650_write_array(ov5650_ca_qsxga_regs, ARRAY_SIZE(ov5650_ca_qsxga_regs));
}

IM_INT32 ov5650_p48a_to_1080p(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5650_write_array(ov5650_pv_1080p_regs, ARRAY_SIZE(ov5650_pv_1080p_regs));
}

IM_INT32 ov5650_p48a_to_sxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5650_write_array(ov5650_ca_sxga_regs, ARRAY_SIZE(ov5650_ca_sxga_regs));
}

IM_INT32 ov5650_p48a_to_720p(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5650_write_array(ov5650_pv_720p_regs, ARRAY_SIZE(ov5650_pv_720p_regs));
}

IM_INT32 ov5650_p48a_to_vga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5650_write_array(ov5650_pv_vga_regs, ARRAY_SIZE(ov5650_pv_vga_regs));
}



//======================================================
IM_RET ov5650_p48a_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(OV5650_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET ov5650_p48a_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

#define REPEAT_TIMES 100
IM_INT32 ov5650_p48a_repeat_init()
{
    struct  ov5650_regval_list regs;
    IM_INT32 i, num = 0;
    IM_INT32 ret;
	IM_TIPMSG((IM_STR("#################################%s()##################################"), IM_STR(_IM_FUNC_)));

    do{
        num++;
        IM_TIPMSG((IM_STR("%s()::repeat init num=%d"), IM_STR(_IM_FUNC_), num));

	//set RESET(io index=gOv5650.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5650.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5650.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5650.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5650.pwdn, 0);

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

	camsenpwl_io_set_outdat(gOv5650.pwdn, 1);

	camsenpwl_io_set_outdat(gOv5650.reset, 0);

	mdelay(5);

    //
	camsenpwl_io_set_outdat(gOv5650.pwdn, 0);
	
    mdelay(5);

	camsenpwl_io_set_outdat(gOv5650.reset, 1);

	mdelay(20);

    //software reset sensor and wait
    regs.reg_num[0] = 0x30;
    regs.reg_num[1] = 0x08;
    regs.value[0] = 0x80;
    ret = cam_ov5650_write(regs.reg_num, regs.value);
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("OV5650 Failed to write value!")));
    }

	mdelay(4);

    ret = cam_ov5650_write_array(ov5650_init_regs, ARRAY_SIZE(ov5650_init_regs));
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to write default registers value!")));
    }
    else
    {
        IM_TIPMSG((IM_STR("Succesed to write default registers value!")));
        return 0;
    }

    }while(num<REPEAT_TIMES);

    return -1;
}

IM_RET ov5650_p48a_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 ret;
    struct  ov5650_regval_list regs;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

    gPwl = pwl;

    camsenpwl_memset((void*)&gOv5650, 0x0, sizeof(gOv5650));
    //init value
    gOv5650.wbMode = CAM_WB_MODE_AUTO;
    gOv5650.efType = CAM_SPECIAL_EFFECT_NONE;
    gOv5650.expVal = 0;
    gOv5650.bandMode = CAM_ANTIBANDING_MODE_OFF;
    gOv5650.sceneMode = CAM_SCENE_MODE_AUTO;

    gOv5650.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
    IM_INFOMSG((IM_STR("%s(OV5650_PWDN= %d)"), IM_STR(_IM_FUNC_), gOv5650.pwdn));
    gOv5650.reset = camsenpwl_get_reset_padnum(gPwl);


    //config io
    /*io_index138(XCAMD12)<-->ov5650.MCLK, io_indexgOv5650.reset(XCAMD16)<-->ov5650.RESET,
	 * io_index136(XCAMD10)<-->ov5650.PWDN, io_index<-->ov5650.POWER*/

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
	
	//set RESET(io index=gOv5650.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5650.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5650.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5650.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5650.pwdn, 0);

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

	camsenpwl_io_set_outdat(gOv5650.pwdn, 1);

	camsenpwl_io_set_outdat(gOv5650.reset, 0);

	mdelay(5);

    //
	camsenpwl_io_set_outdat(gOv5650.pwdn, 0);
	
    mdelay(5);

	camsenpwl_io_set_outdat(gOv5650.reset, 1);

	mdelay(20);

    //software reset sensor and wait
    regs.reg_num[0] = 0x30;
    regs.reg_num[1] = 0x08;
    regs.value[0] = 0x80;
    ret = cam_ov5650_write(regs.reg_num, regs.value);
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("OV5650 Failed to write value!")));
    }

	mdelay(4);
	
	//read sensor id
	if(checkOnly == IM_TRUE)
	{
        //0x300a
        regs.reg_num[0] = 0x30;
        regs.reg_num[1] = 0x0a;
        regs.value[0] = 0xFF;
        ret = cam_ov5650_read(regs.reg_num, regs.value);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            //goto Fail;
        }

        IM_INFOMSG((IM_STR("%s(chipId.value0 = 0x%x, real value is 0x56)"), IM_STR(_IM_FUNC_), regs.value[0]));
        
        if(regs.value[0] != 0x56)
        {
            IM_ERRMSG((IM_STR("ov5650 id error!")));
            //goto Fail;
		}

        //0x300b
        regs.reg_num[0] = 0x30;
        regs.reg_num[1] = 0x0b;
        regs.value[0] = 0xFF;
        ret = cam_ov5650_read(regs.reg_num, regs.value);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            //goto Fail;
        }
        IM_INFOMSG((IM_STR("%s(chipId.value1 = 0x%x, real value is 0x50/0x51)"), IM_STR(_IM_FUNC_), regs.value[0]));
        if(regs.value[0] != 0x51)
        {
            IM_ERRMSG((IM_STR("ov5650 id error!")));
            //goto Fail;
		}
        //only for test
        //0x4708
        regs.reg_num[0] = 0x47;
        regs.reg_num[1] = 0x08;
        regs.value[0] = 0xFF;
        ret = cam_ov5650_read(regs.reg_num, regs.value);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            //goto Fail;
        }
        IM_INFOMSG((IM_STR("%s(0x4708 = 0x%x, real value is 0x01)"), IM_STR(_IM_FUNC_), regs.value[0]));
        /*do{
            regs.reg_num[0] = 0x35;
            regs.reg_num[1] = 0x0b;
            regs.value[0] = 0x04;
            ret = cam_ov5650_write(regs.reg_num, regs.value);
            if(ret != 0)
            {
                IM_ERRMSG((IM_STR("Failed###########!")));
            }
            else
            {
                IM_TIPMSG((IM_STR("Succes...........!")));
            }
        }while(1);*/

        return IM_RET_OK;
    }

    ret = cam_ov5650_write_array(ov5650_init_regs, ARRAY_SIZE(ov5650_init_regs));
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        if(ov5650_p48a_repeat_init() != 0)
        {
            goto Fail;
        }
    }

    IM_TIPMSG((IM_STR("OV5650 success to write default registers!")));

    return IM_RET_OK;

Fail:

    //power down enable
    camsenpwl_io_set_outdat(gOv5650.pwdn, 1);
    //reset sensor
    camsenpwl_io_set_outdat(gOv5650.reset, 0);
    //close mclk
    camsenpwl_clock_disable(gPwl);

    return IM_RET_FAILED;
}

IM_RET ov5650_p48a_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gOv5650.pwdn, 1);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gOv5650.reset, 0);

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

IM_RET ov5650_p48a_start(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ov5650_p48a_stop(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ov5650_p48a_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA | CAM_RES_720P | CAM_RES_SXGA | CAM_RES_1080P | CAM_RES_500W; 
	caps->maxRes = CAM_RES_500W; 
    caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET ov5650_p48a_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}


IM_RET ov5650_p48a_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret = 0;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_VGA)
	{
		ret = ov5650_p48a_to_vga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5650_p48a_to_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_720P)
	{
		ret = ov5650_p48a_to_720p(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5650_p48a_to_720p() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SXGA)
	{
		ret = ov5650_p48a_to_sxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5650_p48a_to_sxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_1080P)
	{
		ret = ov5650_p48a_to_1080p(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5650_p48a_to_1080p() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_500W)
	{
		ret = ov5650_p48a_to_qsxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5650_p48a_to_qsxga() failed!")));
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

IM_RET ov5650_p48a_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	/*if(property == CAM_KEY_RW_WB_MODE)
	{
		ov5650_p48a_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		ov5650_p48a_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		ov5650_p48a_set_scene_mode(value);
	}
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		ov5650_p48a_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		ov5650_p48a_set_light_on(value);
	}
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		ov5650_p48a_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		ov5650_p48a_set_band_filter(value);
	}*/

	return ret;
}

IM_RET ov5650_p48a_get_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	switch(property)
	{
		case CAM_KEY_R_CAPS:
			value = //CAM_CAP_WB_MODE_SUPPORT |
				//CAM_CAP_SPECIAL_EFFECT_SUPPORT |
				//CAM_CAP_SCENE_MODE_SUPPORT |
				//CAM_CAP_FLASH_MODE_SUPPORT |
				//CAM_CAP_ANTIBANDING |
				//CAM_CAP_EXPOSURE |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		/*case CAM_KEY_R_SUPPORT_WB_MODE:
			value = CAM_WB_MODE_AUTO |
				CAM_WB_MODE_INCANDESCENT |
				CAM_WB_MODE_FLUORESCENT |
				CAM_WB_MODE_DAYLIGHT |
				CAM_WB_MODE_CLOUDY_DAYLIGHT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_WB_MODE:
			memcpy(p, (void*)&gOv5650.wbMode, sizeof(IM_INT32));
			break;
		case CAM_KEYdd_R_SUPPORT_SPECIAL_EFFECT:
			value = 
                CAM_SPECIAL_EFFECT_NONE |
				CAM_SPECIAL_EFFECT_MONO |
				CAM_SPECIAL_EFFECT_NEGATIVE |
				CAM_SPECIAL_EFFECT_SOLARIZE |
				CAM_SPECIAL_EFFECT_PASTEL |
				CAM_SPECIAL_EFFECT_SEPIA |
				CAM_SPECIAL_EFFECT_POSTERIZE |
				CAM_SPECIAL_EFFECT_AQUA |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy(p, (void*)&gOv5650.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gOv5650.sceneMode, sizeof(IM_INT32));
			break;
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
			memcpy(p, (void*)&gOv5650.flashMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_RW_LIGHT_TURN_ON:
			memcpy(p, (void*)&gOv5650.light, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gOv5650.expVal, sizeof(IM_INT32));
			break;
	    case CAM_KEY_R_SUPPORT_ANTIBANDING_MODE:
			value = CAM_ANTIBANDING_MODE_OFF |
				CAM_ANTIBANDING_MODE_50HZ |
				CAM_ANTIBANDING_MODE_60HZ |
				//CAM_ANTIBANDING_MODE_AUTO |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_ANTIBANDING_MODE:
			memcpy(p, (void*)&gOv5650.bandMode, sizeof(IM_INT32));
			break;*/
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops ov5650_p48a_ops = {
	.name                   = "ov5650_p48a",
	.i2c_dev_addr           = OV5650_I2C_ADDR,

	.sen_pwdn				= ov5650_p48a_pwdn,
	.sen_get_pmu_info		= ov5650_p48a_get_pmu_info,
	.sen_init				= ov5650_p48a_init,
	.sen_deinit				= ov5650_p48a_deinit,
	.sen_start				= ov5650_p48a_start,
	.sen_stop				= ov5650_p48a_stop,
	.sen_get_caps			= ov5650_p48a_get_caps,
	.sen_set_out_mode		= ov5650_p48a_set_out_mode,
	.sen_get_out_mode		= ov5650_p48a_get_out_mode,
	.sen_set_property		= ov5650_p48a_set_property,
	.sen_get_property		= ov5650_p48a_get_property,
};
