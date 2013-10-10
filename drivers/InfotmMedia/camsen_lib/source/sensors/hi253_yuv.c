/***************************************************************************** 
 ** hi253_yuv.c 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 **     
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **             
 ** Description: sensor config for sensor of ov2655 production 
 **             
 ** Author:     
 **     Jimmy<jimmy.shu@infotmic.com.cn>
 **      
 ** Revision History: 
 ** ­­­­­­­­­­­­­­­­­ 
 ** 1.0  20/07/2012    Jimmy  
 *******************************************************************************/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "hi253_yuv.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"hi253_YUV_I:"
#define WARNHEAD	"hi253_YUV_W:"
#define ERRHEAD		"hi253_YUV_E:"
#define TIPHEAD		"hi253_YUV_T:"

typedef struct {
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_INT32 wbMode;
	IM_INT32 efType;
	IM_INT32 expVal;
	IM_INT32 bandMode;
	IM_INT32 sceneMode;
}hi253_yuv_context_t;

static hi253_yuv_context_t gHi253;
static pwl_handle_t gPwl = IM_NULL;
static IM_CHAR  hi253_VENID0 = 0X04;
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
#define cam_hi253_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 1, d)
#define cam_hi253_write(b) camsenpwl_i2c_write(gPwl, b, 2)


static IM_CHAR hi253_read_cmos(IM_UCHAR addr)
{
	IM_CHAR buf = 0;
	cam_hi253_read((IM_UCHAR *)(&addr), &buf, 1);
	return buf;
}


static IM_INT32 hi253_write_cmos(IM_UCHAR addr, IM_UCHAR val)
{
	struct	hi253_regval_list  reg;
	IM_INT32 ret;

	 reg.reg = addr;
	 reg.value = val;
	 ret = cam_hi253_write( (IM_UCHAR *)(&reg));
	 if(ret)
	 {
		 IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
		 return -1;
	 }
	 else{
		 return 0;
	 }

}


IM_RET hi253_yuv_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(hi253_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET hi253_yuv_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}



static void hi253_set_mirror_flip(u8 image_mirror)
{
  /********************************************************
    * Page Mode 0: Reg 0x0011 bit[1:0] = [Y Flip : X Flip]
    * 0: Off; 1: On.
    *********************************************************/   
    u8 mirror;
  
    IM_ERRMSG((IM_STR("[Enter]:hi704 set Mirror_flip func:image_mirror=%d\n"),image_mirror));
	
    hi253_write_cmos(0x03,0x00);     //Page 0	
   mirror = (hi253_read_cmos(0x11) & 0xfc); 

    switch (image_mirror) 
    {
         case IMAGE_NORMAL:
    	mirror  |= 0x03; 
        break;
    case IMAGE_H_MIRROR:
        mirror  |= 0x02;
        break;
    case IMAGE_V_MIRROR:
        mirror  |= 0x01; 
        break;
    case IMAGE_HV_MIRROR:
        mirror  |= 0x00;
        break;
    	default:
	        mirror |= 0x00;
	break;
    }

    hi253_write_cmos(0x11, mirror);

}

IM_INT32 hi253_yuv_switch_uxga(void){ 
	IM_INT32 i, ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	
	hi253_write_cmos(0x10, 0x00);
	hi253_write_cmos(0x12, 0x05);

	hi253_write_cmos(0x03, 0x18);
	hi253_write_cmos(0x10, 0x00);
	hi253_write_cmos(0x11, 0x00);

	msleep(100);

	return 0; 
}

IM_INT32 hi253_yuv_switch_svga(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	for(i = 0; i < ((sizeof(hi253_svga_regs)) >>1 ); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_svga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
	
	return 0; 
}

IM_INT32 hi253_yuv_switch_vga(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	hi253_write_cmos(0x03, 0x00);
	//sleep on	
	hi253_write_cmos(0x03, hi253_read_cmos(0x01)|0x01);
	
	for(i = 0; i < ((sizeof(hi253_vga_regs)) >>1 ); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_vga_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}	
	hi253_write_cmos(0x03, 0x00);
	//sleep on	
	hi253_write_cmos(0x03, hi253_read_cmos(0x01)|0xfe);

	return 0; 
}


IM_INT32 hi253_yuv_set_sepia(void){ 
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_sepia_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_sepia_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
#endif

	return 0; 
}
IM_INT32 hi253_yuv_set_bluish(void){ 
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	for(i = 0; i < (sizeof(hi253_bluish_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_bluish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0;
#else
	return -1;
#endif
}
IM_INT32 hi253_yuv_set_greenish(void){
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	for(i = 0; i < (sizeof(hi253_greenish_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_greenish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0;
#else
	return -1;
#endif
}
IM_INT32 hi253_yuv_set_reddish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
#if 0
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(hi253_reddish_regs) / 2); i++)
	{
		ret = cam_hi253_write((unsigned char *)(&hi253_reddish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
IM_INT32 hi253_yuv_set_yellowish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}
IM_INT32 hi253_yuv_set_grayish(void){
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
#if 0
	IM_INT32 i, ret;
	for(i = 0; i < (sizeof(hi253_yellowish_regs) / 2); i++)
	{
		ret = cam_hi253_write((unsigned char *)(&hi253_yellowish_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else

	return -1;
#endif
}
IM_INT32 hi253_yuv_set_bandw(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_mono_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_mono_regs[i]));
		if(!ret)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0; 
}
IM_INT32 hi253_yuv_set_negative(void){
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_negative_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_negative_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0;
#else
	return -1;
#endif
}


IM_INT32 hi253_yuv_set_normal(void){ 
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_normal_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_normal_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0;
#else
	return -1;
#endif
}



IM_INT32 hi253_yuv_set_auto(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_auto_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_auto_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c:the %d th register\n"),i));
			return ret;
		}
	}
	return 0;
}
IM_INT32 hi253_yuv_set_sunny(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_sunny_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_sunny_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return ret;
		}
	}
	return 0; 
}

IM_INT32 hi253_yuv_set_cloudy(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_cloudy_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_cloudy_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return ret;
		}
	}
	return 0;
}

IM_INT32 hi253_yuv_set_office(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_office_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_office_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return ret;
		}
	}
	return 0;
}

IM_INT32 hi253_yuv_set_home(void){ 
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_home_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_home_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return ret;
		}
	}
	return 0;
}

IM_INT32 hi253_yuv_incande(void){ 
#if 1
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_office_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_office_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	return 0;
#else
	return -1;
#endif
}

IM_INT32 hi253_yuv_fluor(void){ 
	#if 1

	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(i = 0; i < (sizeof(hi253_fluor_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_fluor_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
			return -1;
		}
		else
		{
		}
	}
	
	return 0; 
#else
	return -1;
#endif
}

IM_RET hi253_yuv_sw_reset(void)
{
	IM_INT32 i, ret;
    //Software Reset
	for(i = 0; i < (sizeof(hi253_sw_reset_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_sw_reset_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return IM_RET_FAILED;
		}
		else{
		}
	}
	return IM_RET_OK;
}

IM_RET hi253_yuv_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	IM_CHAR buf = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gHi253, 0x0, sizeof(gHi253));
	//init value
	gHi253.wbMode = CAM_WB_MODE_AUTO;
	gHi253.efType = CAM_SPECIAL_EFFECT_NONE;
	gHi253.expVal = 0;
	gHi253.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gHi253.sceneMode = CAM_SCENE_MODE_AUTO;

	gHi253.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	IM_INFOMSG((IM_STR("(gHi253.pwdn = %d)"), gHi253.pwdn));
	gHi253.reset = camsenpwl_get_reset_padnum(gPwl);

	//config io
	/*io_index138(XCAMD12)<-->gc0308.MCLK, io_indexgHi253.reset(XCAMD16)<-->gc0308.RESET,
	 * io_index136(XCAMD10)<-->gc0308.PWDN, io_index_XXX<-->gc0308.POWER*/

	/******************************
	*set each io mode
	******************************/
	//set IO mode, default is case0
	//volatile unsigned IM_INT32 *)(0x21e09000) = 0x0;

	//set POWER(io index) state(mode and dir)
	
	//set RESET(io index=gHi253.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gHi253.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gHi253.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gHi253.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gHi253.pwdn, 0);

	/******************************************************
	*POWER enable: supply power, has enable all the time
	******************************************************/
	//delay some time
	msleep(5);

	/******************************
	*provided mclk
	******************************/
	camsenpwl_clock_enable(gPwl,24000000);
	/******************************
	*power down disable
	******************************/
	//set io(index=136) outdata = 1(pwdn also  need falsing edge, low active to disable pwdn)
	camsenpwl_io_set_outdat(gHi253.pwdn, 1);
	
	camsenpwl_io_set_outdat(gHi253.pwdn, 0);
	
	msleep(10);

	/******************************
	*reset sensor
	******************************/
	//set io(index=gHi253.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gHi253.reset, 1);
	camsenpwl_io_set_outdat(gHi253.reset, 0);
	
	msleep(5);
	
	camsenpwl_io_set_outdat(gHi253.reset, 1);
	
	msleep(5);
    //Software Reset
	if(IM_RET_OK != hi253_yuv_sw_reset()){
		IM_ERRMSG((IM_STR("hi253_yuv software reset fail!")));
		return IM_RET_FAILED;
	}
	//read sensor id
	if(checkOnly == IM_TRUE)
	{
		cam_hi253_read((IM_UINT8 *)(&hi253_VENID0), &buf, 1);
		IM_INFOMSG((IM_STR("%s(hi253_VENID0 = 0x%x, real value is 0x92)"), IM_STR(_IM_FUNC_), buf));
		if(buf != 0x92)
		{
			IM_ERRMSG((IM_STR("hi253_yuv id error!")));
			return IM_RET_FAILED;
		}
		return IM_RET_OK;
	}
	buf = 0;

	for(i = 0; i < (sizeof(hi253_init_regs) / 2); i++)
	{
		ret = cam_hi253_write((IM_UINT8 *)(&hi253_init_regs[i]));
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return IM_RET_FAILED;
		}
		else{
		}
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;
}
IM_RET hi253_yuv_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));


	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gHi253.reset, 0);

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gHi253.pwdn, 1);

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
IM_RET hi253_yuv_start(void)
{
	//IM_INT32 i;
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
/*	
	ret = hi253_yuv_switch_vga();
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
		return ret;
	}
*/	
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET hi253_yuv_stop(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return ret;
}
IM_RET hi253_yuv_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	caps->supportRes = CAM_RES_SVGA |CAM_RES_UXGA; 
	caps->maxRes = CAM_RES_UXGA; 
	caps->initRes = CAM_RES_SVGA;

	return ret;
}
IM_RET hi253_yuv_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET hi253_yuv_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_UXGA)
	{
		ret = hi253_yuv_switch_uxga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("hi253_yuv_switch_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret =hi253_yuv_switch_svga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("hi253_yuv_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_VGA)
	{
		ret = hi253_yuv_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("hi253_yuv_switch_vga() failed!")));
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

IM_INT32 hi253_yuv_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			hi253_yuv_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			hi253_yuv_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			hi253_yuv_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			hi253_yuv_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			hi253_yuv_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			hi253_yuv_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			hi253_yuv_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			hi253_yuv_set_greenish(); 
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

	gHi253.efType = value;

	return 0;
}




IM_INT32 hi253_yuv_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			hi253_yuv_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			hi253_yuv_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			hi253_yuv_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			hi253_yuv_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			hi253_yuv_set_cloudy();
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

	gHi253.wbMode = value;

	return 0;
}

IM_RET hi253_yuv_set_exposure(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	if(value == gHi253.expVal){
		return IM_RET_OK;
	}
	IM_INFOMSG((IM_STR("%s(value=%d)--"), IM_STR(_IM_FUNC_),value));
    
	hi253_write_cmos(0x03,0x10);
	hi253_write_cmos(0x12,(hi253_read_cmos(0x12)|0x10));//make sure the Yoffset control is opened.

    switch (value)
    {
    case -4:
        hi253_write_cmos(0x40,0xc0);
		
        break;
    case -3:
        hi253_write_cmos(0x40,0xb0);
        break;
    case -2:
        hi253_write_cmos(0x40,0xa0);
        break;
    case -1:
        hi253_write_cmos(0x40,0x90);
        break;
    case 0:
        hi253_write_cmos(0x40,0x00);
        break;
    case 1:
        hi253_write_cmos(0x40,0x10);
        break;
    case 2:
        hi253_write_cmos(0x40,0x20);
        break;
    case 3:
        hi253_write_cmos(0x40,0x30);
        break;
    case 4:
        hi253_write_cmos(0x40,0x40);
        break;
    default:
        return IM_RET_FAILED;
    }
	gHi253.expVal = value;

    return IM_RET_OK;
	
}
IM_INT32 hi253_yuv_set_night_mode(IM_BOOL enable)	
{
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	if (enable) 
	{
		for(i = 0; i < (sizeof(hi253_night_regs) / 2); i++)
		{
			ret = cam_hi253_write((IM_UINT8 *)(&hi253_night_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
				return ret;
			}
		}
	}
	else
	{
		for(i = 0; i < (sizeof(hi253_sunset_regs) / 2); i++)
		{
			ret = cam_hi253_write((IM_UINT8 *)(&hi253_sunset_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c\n")));
				return ret;
			}
		}	
	}
	msleep(10);
	
	return 0;
}

IM_INT32 hi253_yuv_set_scene_mode(IM_INT32 value)
{
	IM_BOOL nightModeEn;
	if(value == gHi253.sceneMode)
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

	gHi253.sceneMode = value;

	return hi253_yuv_set_night_mode(nightModeEn);
}

IM_INT32 hi253_yuv_set_antibanding(IM_INT32 value)
{
	IM_INT32 i = 0;
	IM_INT32 ret = 0;
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gHi253.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_50HZ:
			
		for(i = 0;i < (sizeof(hi253_50hz_regs)/2);i++){		
			ret = cam_hi253_write((IM_UINT8 *)(&hi253_50hz_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return -1;
			}
		}
		break;

		case CAM_ANTIBANDING_MODE_60HZ:
		for(i = 0;i < (sizeof(hi253_50hz_regs)/2);i++){		
			ret = cam_hi253_write((IM_UINT8 *)(&hi253_60hz_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return IM_RET_FAILED;
			}
		}
		break;

		default:
			return -1;
	}

	gHi253.bandMode = value;

	return 0;
}

IM_RET hi253_yuv_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));
	
	if(property == CAM_KEY_RW_WB_MODE)
	{
		hi253_yuv_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		hi253_yuv_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		hi253_yuv_set_scene_mode(value);
	}
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		hi253_yuv_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		hi253_yuv_set_antibanding(value);
	}

	return ret;
}

IM_RET hi253_yuv_get_property(IM_UINT32 property, void *p)
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
			memcpy(p, (void*)&gHi253.wbMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gHi253.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gHi253.sceneMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gHi253.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gHi253.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}


camsen_ops hi253_yuv_ops = {
	.name                   = "hi253_yuv",
	.i2c_dev_addr           = HI253_I2C_ADDR,

	.sen_pwdn				= hi253_yuv_pwdn,
	.sen_get_pmu_info		= hi253_yuv_get_pmu_info,
	.sen_init				= hi253_yuv_init,
	.sen_deinit				= hi253_yuv_deinit,
	.sen_start				= hi253_yuv_start,
	.sen_stop				= hi253_yuv_stop,
	.sen_get_caps			= hi253_yuv_get_caps,
	.sen_set_out_mode		= hi253_yuv_set_out_mode,
	.sen_get_out_mode		= hi253_yuv_get_out_mode,
	.sen_set_property		= hi253_yuv_set_property,
	.sen_get_property		= hi253_yuv_get_property,
};

