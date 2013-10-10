/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: ov5640_zyhb.c
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
#include "ov5640_zyhb.h"

#define DBGINFO		1
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"OV5640_ZYHB_I:"
#define WARNHEAD	"OV5640_ZYHB_W:"
#define ERRHEAD		"OV5640_ZYHB_E:"
#define TIPHEAD		"OV5640_ZYHB_T:"

#define REG_ADDR_STEP 2
#define REG_DATA_STEP 1
#define REG_STEP 			(REG_ADDR_STEP+REG_DATA_STEP)

/*
 * The default register settings
 *
 */

struct regval_list {
	IM_UINT8 reg_num[REG_ADDR_STEP];
	IM_UINT8 value[REG_DATA_STEP];
};

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
}ov5640_zyhb_context_t;

static ov5640_zyhb_context_t gOv5640;
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

#define cam_ov5640_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 2, d)
#define cam_ov5640_write(b) camsenpwl_i2c_write(gPwl, b, 3)

/*
 * Write a list of register settings;
 */
static IM_INT32 cam_ov5640_write_array(struct ov5640_regval_list *vals , IM_UINT32 size)
{
	IM_INT32 i,ret;
//	unsigned char rd;
	if (size == 0)
		return -1;

	for(i = 0; i < size ; i++)
	{
		if(vals->reg_high == 0xff && vals->reg_low == 0xff) {
			mdelay(vals->value);
		}	
		else {	
            ret = cam_ov5640_write((IM_UINT8 *)(vals));
            if(ret)
            {
                IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                return -1;
            }
		}
		vals++;
	}
	
	return 0;
}

IM_INT32 ov5640_zyhb_to_qsxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    
    return cam_ov5640_write_array(ov5640_qsxga_regs, ARRAY_SIZE(ov5640_qsxga_regs));
}

IM_INT32 ov5640_zyhb_to_qxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_qxga_regs, ARRAY_SIZE(ov5640_qxga_regs));
}

IM_INT32 ov5640_zyhb_to_uxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_uxga_regs, ARRAY_SIZE(ov5640_uxga_regs));
}

IM_INT32 ov5640_zyhb_to_sxga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_sxga_regs, ARRAY_SIZE(ov5640_sxga_regs));
}

IM_INT32 ov5640_zyhb_to_xvga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_xvga_regs, ARRAY_SIZE(ov5640_xvga_regs));
}

IM_INT32 ov5640_zyhb_to_1080p(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_1080p_regs, ARRAY_SIZE(ov5640_1080p_regs));
}

IM_INT32 ov5640_zyhb_to_720p(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_720p_regs, ARRAY_SIZE(ov5640_720p_regs));
}

IM_INT32 ov5640_zyhb_to_svga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_svga_regs, ARRAY_SIZE(ov5640_svga_regs));
}

IM_INT32 ov5640_zyhb_to_vga(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_vga_regs, ARRAY_SIZE(ov5640_vga_regs));
}

//value, 1: enable auto white balance, 0: disable white balance
IM_INT32 ov5640_zyhb_set_auto(IM_INT32 value){ 
	IM_INT32 ret;
    struct regval_list rReg;
    struct ov5640_regval_list wReg;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ret = cam_ov5640_write_array(ov5640_auto_regs, ARRAY_SIZE(ov5640_auto_regs));
	if (ret < 0) {
        IM_ERRMSG((IM_STR("Write array err at ov5640_auto_regs!")));
		return ret;
	}
	
	rReg.reg_num[0] = 0x34;
	rReg.reg_num[1] = 0x06;
	ret = cam_ov5640_read(rReg.reg_num, rReg.value, 1);
	if (ret < 0) {
        IM_ERRMSG((IM_STR("Read err at ov5640_auto_regs!")));
		return ret;
	}

	switch(value) {
	case 0:
		rReg.value[0] |= 0x01;
		break;
	case 1:
		rReg.value[0] &= 0xfe;
		break;
	default:
		break;
	}

    wReg.reg_high = 0x34;
    wReg.reg_low = 0x06;
    wReg.value = rReg.value[0];    
    ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
	if (ret < 0) {
        IM_ERRMSG((IM_STR("Write err at ov5640_auto_regs!")));
		return ret;
	}
	mdelay(10);
	
	return 0; 
}

IM_INT32 ov5640_zyhb_set_sunny(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_sunny_regs, ARRAY_SIZE(ov5640_sunny_regs));
}

IM_INT32 ov5640_zyhb_set_cloudy(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_cloudy_regs, ARRAY_SIZE(ov5640_cloudy_regs));
}

IM_INT32 ov5640_zyhb_set_fluorescent(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_fluorescent_regs, ARRAY_SIZE(ov5640_fluorescent_regs));
}

IM_INT32 ov5640_zyhb_set_incandescence(void){ 
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    return cam_ov5640_write_array(ov5640_incandescence_regs, ARRAY_SIZE(ov5640_incandescence_regs));
}

IM_INT32 ov5640_zyhb_set_wb(IM_INT32 value)
{
    IM_INT32 ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gOv5640.wbMode)
	{
		return 0;
	}

    if(value == CAM_WB_MODE_AUTO)
    {
        return ov5640_zyhb_set_auto(1);
    }

    ret = ov5640_zyhb_set_auto(0);
    if(ret < 0) {
        IM_ERRMSG((IM_STR("set autowb disable failed!")));
        return ret;
    }

	switch(value)
	{
		case CAM_WB_MODE_INCANDESCENT:
			ret = ov5640_zyhb_set_incandescence();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			ret = ov5640_zyhb_set_fluorescent();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			ret = ov5640_zyhb_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			ret = ov5640_zyhb_set_cloudy();
			break;
		case CAM_WB_MODE_WARM_FLUORECENT:
		case CAM_WB_MODE_TWILIGHT:
		case CAM_WB_MODE_SHADE:
		case CAM_WB_MODE_AUTO:
		default:
            return -1;
	}

    if(ret < 0) {
        IM_ERRMSG((IM_STR("set wb mode failed!")));
        return ret;
    }

	mdelay(10);
	gOv5640.wbMode = value;
	return 0;
}

IM_INT32 ov5640_zyhb_set_exposure(IM_INT32 value)
{
    IM_INT32 ret;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gOv5640.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -1.7 */
            ret = cam_ov5640_write_array(ov5640_ev_neg4_regs, ARRAY_SIZE(ov5640_ev_neg4_regs));
			break;
		case -3:							/* EV -1.3 */
            ret = cam_ov5640_write_array(ov5640_ev_neg3_regs, ARRAY_SIZE(ov5640_ev_neg3_regs));
			break;
		case -2:							/* EV -1.0 */
            ret = cam_ov5640_write_array(ov5640_ev_neg2_regs, ARRAY_SIZE(ov5640_ev_neg2_regs));
			break;
		case -1:							/* EV -0.7 */
            ret = cam_ov5640_write_array(ov5640_ev_neg1_regs, ARRAY_SIZE(ov5640_ev_neg1_regs));
			break;
		case 0:								/* EV 0 */
            ret = cam_ov5640_write_array(ov5640_ev_zero_regs, ARRAY_SIZE(ov5640_ev_zero_regs));
			break;
		case 1:							/* EV +0.7 */
            ret = cam_ov5640_write_array(ov5640_ev_pos1_regs, ARRAY_SIZE(ov5640_ev_pos1_regs));
			break;
		case 2:							/* EV +1.0 */
            ret = cam_ov5640_write_array(ov5640_ev_pos2_regs, ARRAY_SIZE(ov5640_ev_pos2_regs));
			break;
		case 3:							/* EV +1.3 */
            ret = cam_ov5640_write_array(ov5640_ev_pos3_regs, ARRAY_SIZE(ov5640_ev_pos3_regs));
			break;
		case 4:							/* EV +1.7 */
            ret = cam_ov5640_write_array(ov5640_ev_pos4_regs, ARRAY_SIZE(ov5640_ev_pos4_regs));
			break;
		default:
			return -1;
	}


	mdelay(10);
	gOv5640.expVal = value;
	return 0;
}

IM_INT32 ov5640_zyhb_set_band_filter(IM_INT32 value)
{
	struct regval_list rReg;
	struct ov5640_regval_list wReg;
	IM_INT32 ret = 0;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gOv5640.bandMode)
	{
		return 0;
	}
	
	switch(value) {
		case CAM_ANTIBANDING_MODE_OFF:
			rReg.reg_num[0] = 0x3a;
			rReg.reg_num[1] = 0x00;
            ret = cam_ov5640_read(rReg.reg_num, rReg.value, 1);
			if (ret < 0)
                IM_ERRMSG((IM_STR("Read err at ov5640_set_band_filter!")));
				
			rReg.value[0] &= 0xdf;  //turn off band filter

            wReg.reg_high = 0x3a;
            wReg.reg_low = 0x00;
            wReg.value = rReg.value[0];    
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
			break;
		case CAM_ANTIBANDING_MODE_50HZ:
            wReg.reg_high = 0x3c;
            wReg.reg_low = 0x00;
            wReg.value = 0x04;    //50hz
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
            wReg.reg_high = 0x3c;
            wReg.reg_low = 0x01;
            wReg.value = 0x80;  //manual band filter    
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
			rReg.reg_num[0] = 0x3a;
			rReg.reg_num[1] = 0x00;
            ret = cam_ov5640_read(rReg.reg_num, rReg.value, 1);
			if (ret < 0)
                IM_ERRMSG((IM_STR("Read err at ov5640_set_band_filter!")));
				
			rReg.value[0] |= 0x20;  //turn on band filter

            wReg.reg_high = 0x3a;
            wReg.reg_low = 0x00;
            wReg.value = rReg.value[0];    
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
			break;
		case CAM_ANTIBANDING_MODE_60HZ:
            wReg.reg_high = 0x3c;
            wReg.reg_low = 0x00;
            wReg.value = 0x00;    //60hz
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
            wReg.reg_high = 0x3c;
            wReg.reg_low = 0x01;
            wReg.value = 0x80;  //manual band filter    
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
			rReg.reg_num[0] = 0x3a;
			rReg.reg_num[1] = 0x00;
            ret = cam_ov5640_read(rReg.reg_num, rReg.value, 1);
			if (ret < 0)
                IM_ERRMSG((IM_STR("Read err at ov5640_set_band_filter!")));
				
			rReg.value[0] |= 0x20;  //turn on band filter

            wReg.reg_high = 0x3a;
            wReg.reg_low = 0x00;
            wReg.value = rReg.value[0];    
            ret = cam_ov5640_write((IM_UINT8 *)(&wReg));
			if (ret < 0)
                IM_ERRMSG((IM_STR("Write err at ov5640_set_band_filter!")));
				
			break;
        case CAM_ANTIBANDING_MODE_AUTO:
        default:
            break;
	}

	mdelay(10);
	gOv5640.bandMode = value;
	return ret;
}

IM_INT32 ov5640_zyhb_flash_on(IM_INT32 on)
{
	IM_INFOMSG((IM_STR("%s(OV5640 FLASH ON= %d)"), IM_STR(_IM_FUNC_), on));
    if(0 == gOv5640.flashLight)
    {
        return 0;
    }
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5640.flashLight, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5640.flashLight, 0);

	camsenpwl_io_set_outdat(gOv5640.flashLight, on);

	return 0;
}

IM_INT32 ov5640_zyhb_set_flash_mode(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gOv5640.flashMode)
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
	
	gOv5640.flashMode = value;
	return 0;
}

IM_INT32 ov5640_zyhb_set_light_on(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gOv5640.light)
	{
		return 0;
	}
	
	switch (value) {
	case 0:
        ov5640_zyhb_flash_on(0);
		break;
	case 1:
        ov5640_zyhb_flash_on(1);
		break;   
	default:
		return -1;
	}
	
	gOv5640.light = value;
	return 0;
}

//======================================================
IM_RET ov5640_zyhb_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(OV5640_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

	//set PWDN  state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(padNum, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(padNum, 0);

	/******************************
	*power down enable
	******************************/
	//set io outdata = 1(pwdn high active to disable pwdn)
	camsenpwl_io_set_outdat(padNum, 1);

	return IM_RET_OK;
}

IM_RET ov5640_zyhb_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET ov5640_zyhb_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 ret;
    struct regval_list regs;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gOv5640, 0x0, sizeof(gOv5640));
	//init value
	gOv5640.wbMode = CAM_WB_MODE_AUTO;
	gOv5640.efType = CAM_SPECIAL_EFFECT_NONE;
	gOv5640.expVal = 0;
	gOv5640.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gOv5640.sceneMode = CAM_SCENE_MODE_AUTO;
	gOv5640.flashMode = CAM_FLASH_MODE_OFF;
	gOv5640.light = 0;

	gOv5640.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(OV5640_PWDN= %d)"), IM_STR(_IM_FUNC_), gOv5640.pwdn));
	gOv5640.reset = camsenpwl_get_reset_padnum(gPwl);

	gOv5640.flashLight = camsenpwl_get_flash_light_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(OV5640_PWDN= %d)"), IM_STR(_IM_FUNC_), gOv5640.flashLight));

    //close flash at init if it is exit
    if(gOv5640.flashLight != 0)
    {
        ov5640_zyhb_flash_on(0);
    }

	//config io
	/*io_index138(XCAMD12)<-->ov5640.MCLK, io_indexgOv5640.reset(XCAMD16)<-->ov5640.RESET,
	 * io_index136(XCAMD10)<-->ov5640.PWDN, io_index<-->ov5640.POWER*/

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
	
	//set RESET(io index=gOv5640.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5640.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5640.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gOv5640.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gOv5640.pwdn, 0);

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

	//check real clock freq if need
	//camsenpwl_clock_get_freq(gPwl);

	/******************************
	*power down disable
	******************************/
	//set io outdata = 0(pwdn high active to disable pwdn)
	camsenpwl_io_set_outdat(gOv5640.pwdn, 1);
	
	camsenpwl_io_set_outdat(gOv5640.pwdn, 0);

	mdelay(5);

	/******************************
	*reset sensor
	******************************/
	//set io(index=gOv5640.reset) outdata = 0->1(reset active low)
	camsenpwl_io_set_outdat(gOv5640.reset, 0);
	
	mdelay(5);
	
	camsenpwl_io_set_outdat(gOv5640.reset, 1);
	
	mdelay(5);

	//read sensor id
	if(checkOnly == IM_TRUE)
	{
        regs.reg_num[0] = 0x30;
        regs.reg_num[1] = 0x0a;
		ret = cam_ov5640_read(regs.reg_num, regs.value, 1);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            goto Fail;
        }
		IM_INFOMSG((IM_STR("%s(ID_HIGH = 0x%x, real value is 0x56)"), IM_STR(_IM_FUNC_), regs.value[0]));
		if(regs.value[0] != 0x56)
		{
			IM_ERRMSG((IM_STR("ov5640 id error!")));
			//goto Fail;
		}

        regs.reg_num[0] = 0x30;
        regs.reg_num[1] = 0x0b;
		ret = cam_ov5640_read(regs.reg_num, regs.value, 1);
        if(ret != 0)
        {
            IM_ERRMSG((IM_STR("Failed to read id value!")));
            goto Fail;
        }
		IM_INFOMSG((IM_STR("%s(ID_LOW = 0x%x, real value is 0x40)"), IM_STR(_IM_FUNC_), regs.value[0]));
		if(regs.value[0] != 0x40)
		{
			IM_ERRMSG((IM_STR("ov5640 id error!")));
			//goto Fail;
		}
		return IM_RET_OK;
	}

    ret = cam_ov5640_write_array(ov5640_init_regs, ARRAY_SIZE(ov5640_init_regs));
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        goto Fail;
    }

    ret = ov5640_zyhb_set_band_filter(CAM_ANTIBANDING_MODE_50HZ);
    if(ret != 0)
    {
        IM_ERRMSG((IM_STR("Failed to set default band filter!")));
        goto Fail;
    }


	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(gOv5640.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(gOv5640.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET ov5640_zyhb_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gOv5640.pwdn, 1);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gOv5640.reset, 0);

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

IM_RET ov5640_zyhb_start(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ov5640_zyhb_stop(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ov5640_zyhb_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_XVGA | CAM_RES_720P | CAM_RES_UXGA | CAM_RES_1080P | CAM_RES_320W | CAM_RES_500W; 
	caps->maxRes = CAM_RES_500W; 
    caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET ov5640_zyhb_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET ov5640_zyhb_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret = 0;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_VGA)
	{
		ret = ov5640_zyhb_to_vga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_vga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = ov5640_zyhb_to_svga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_XVGA)
	{
		ret = ov5640_zyhb_to_xvga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_xvga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_720P)
	{
		ret = ov5640_zyhb_to_720p(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_720p() failed!")));
			return IM_RET_FAILED;
		}
	}
	/*else if(res == CAM_RES_SXGA)
	{
		ret = ov5640_zyhb_to_sxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_sxga() failed!")));
			return IM_RET_FAILED;
		}
	}*/
	else if(res == CAM_RES_UXGA)
	{
		ret = ov5640_zyhb_to_uxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_1080P)
	{
		ret = ov5640_zyhb_to_1080p(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_1080p() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_320W)
	{
		ret = ov5640_zyhb_to_qxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_qxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_500W)
	{
		ret = ov5640_zyhb_to_qsxga(); 
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("ov5640_zyhb_to_qsxga() failed!")));
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

IM_RET ov5640_zyhb_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		ov5640_zyhb_set_wb(value);
	}
	/*else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		ov5640_zyhb_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		ov5640_zyhb_set_scene_mode(value);
	}*/
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		ov5640_zyhb_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		ov5640_zyhb_set_light_on(value);
	}
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		ov5640_zyhb_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		ov5640_zyhb_set_band_filter(value);
	}

	return ret;
}

IM_RET ov5640_zyhb_get_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	switch(property)
	{
		case CAM_KEY_R_CAPS:
			value = CAM_CAP_WB_MODE_SUPPORT |
				//CAM_CAP_SPECIAL_EFFECT_SUPPORT |
				//CAM_CAP_SCENE_MODE_SUPPORT |
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
			memcpy(p, (void*)&gOv5640.wbMode, sizeof(IM_INT32));
			break;
		/*case CAM_KEYdd_R_SUPPORT_SPECIAL_EFFECT:
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
			memcpy(p, (void*)&gOv5640.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gOv5640.sceneMode, sizeof(IM_INT32));
			break;*/
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
			memcpy(p, (void*)&gOv5640.flashMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_RW_LIGHT_TURN_ON:
			memcpy(p, (void*)&gOv5640.light, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gOv5640.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gOv5640.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops ov5640_zyhb_ops = {
	.name                   = "ov5640_zyhb",
	.i2c_dev_addr           = OV5640_I2C_ADDR,

	.sen_pwdn				= ov5640_zyhb_pwdn,
	.sen_get_pmu_info		= ov5640_zyhb_get_pmu_info,
	.sen_init				= ov5640_zyhb_init,
	.sen_deinit				= ov5640_zyhb_deinit,
	.sen_start				= ov5640_zyhb_start,
	.sen_stop				= ov5640_zyhb_stop,
	.sen_get_caps			= ov5640_zyhb_get_caps,
	.sen_set_out_mode		= ov5640_zyhb_set_out_mode,
	.sen_get_out_mode		= ov5640_zyhb_get_out_mode,
	.sen_set_property		= ov5640_zyhb_set_property,
	.sen_get_property		= ov5640_zyhb_get_property,
};
