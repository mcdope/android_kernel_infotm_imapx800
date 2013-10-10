/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: mt9d115_demo.c
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
#include "mt9d115_demo.h"

#define DBGINFO		0	
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"MT9D115_DEMO_I:"
#define WARNHEAD	"MT9D115_DEMO_W:"
#define ERRHEAD		"MT9D115_DEMO_E:"
#define TIPHEAD		"MT9D115_DEMO_T:"

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
}mt9d115_demo_context_t;

static mt9d115_demo_context_t gMt9d115;
static pwl_handle_t gPwl = IM_NULL;

static IM_BOOL In720pMode = IM_FALSE;
static IM_BOOL SetInitParameter = IM_FALSE;

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

//#define cam_mt9d115_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 2, d)
//#define cam_mt9d115_write(b) camsenpwl_i2c_write(gPwl, b, 4)

static IM_INT32 cam_mt9d115_read(IM_UINT8 *reg, IM_UINT8 *value)
{
	return camsenpwl_i2c_read(gPwl, value, reg, REG_ADDR_STEP, REG_DATA_STEP);
}

static IM_INT32 cam_mt9d115_write(IM_UINT8 *reg, IM_UINT8 *value)
{
	IM_UINT8 data[REG_STEP];
	IM_INT32 ret,i;
	
	for(i = 0; i < REG_ADDR_STEP; i++)
			data[i] = reg[i];
	for(i = REG_ADDR_STEP; i < REG_STEP; i++)
			data[i] = value[i-REG_ADDR_STEP];
	
	return camsenpwl_i2c_write(gPwl, data, REG_STEP);
}

//--------------------------------------

static IM_INT32 mt_i2c_write(IM_UINT16 reg, IM_UINT16 value, IM_UINT32 len)
{
	IM_RET ret;
	struct sbuf3 {
		IM_UINT8  reg_high;
		IM_UINT8  reg_low;
		IM_UINT8  value;
	};
	struct sbuf4 {
		IM_UINT8  reg_high;
		IM_UINT8  reg_low;
		IM_UINT8  value_high;
		IM_UINT8  value_low;
	};	 
	
	if (len == 3) {
        struct sbuf3 buf;
		buf.reg_high = reg >> 8;
		buf.reg_low = reg;
		buf.value = value;	
		//IM_ERRMSG((IM_STR("+++++GET---3---Reg&&value : 0x%x, 0x%x, 0x%x++++++"), buf.reg_high, buf.reg_low, buf.value));
    	        //ret = cam_ov5642_write((IM_UINT8 *)(&buf));
		ret = camsenpwl_i2c_write(gPwl, (IM_UINT8 *)(&buf), 3);			
		return ret;
	} else if (len == 4) { 
		struct sbuf4 buf;
		buf.reg_high = reg >> 8;
		buf.reg_low = reg;
		buf.value_high = value >> 8;
		buf.value_low = value;	
		//IM_ERRMSG((IM_STR("+++++GET---4---Reg&&value : 0x%x, 0x%x, 0x%x, 0x%x++++++"), buf.reg_high, buf.reg_low, buf.value_high, buf.value_low));
		ret = camsenpwl_i2c_write(gPwl, (IM_UINT8 *)(&buf), 4);
		return ret;
	} else return -1;
}



static IM_INT32 cam_mt9d115_change_config_command()
{
	IM_INT32 ret; 
	return ret;
}


IM_INT32 mt9d115_demo_switch_uxga(void){ 
	mt_i2c_write(0x098C, 0xA115, 4); 
	mt_i2c_write(0x0990, 0x0002, 4); 
	mt_i2c_write(0x098C, 0xA103, 4); 
	mt_i2c_write(0x0990, 0x0002, 4); 
	msleep(100);
	return 0; 
}

IM_INT32 mt9d115_demo_switch_svga(void){ 
	if ((In720pMode == IM_TRUE) || (SetInitParameter == IM_TRUE))
	{
		In720pMode = IM_FALSE;
	
	#if 1 //FPS慢，效果好；if 0 FPS快，噪点大 20130201
	//Reset 
  	mt_i2c_write(0x001A, 0x0051, 4);
  	msleep(50);  //10
  	mt_i2c_write(0x001A, 0x0050, 4);
  	msleep(50);  //10
	
	//[PLL]
	mt_i2c_write(0x0014, 0x2545 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0010, 0x0010 , 4);		// PLL_DIVIDERS
	mt_i2c_write(0x0012, 0x1FF7 , 4);		// PLL_P_DIVIDERS
	mt_i2c_write(0x0014, 0x2547 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0014, 0x2447 , 4);		// PLL_CONTROL
	msleep(10);
	mt_i2c_write(0x0014, 0x2047 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0014, 0x2046 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0018, 0x4028 , 4);		// STANDBY_CONTROL
  	msleep(30);
  
  	//[Timing]
	mt_i2c_write(0x321C, 0x0003 , 4);		// OFIFO_CONTROL_STATUS
	mt_i2c_write(0x098C, 0x2703 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0320 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2705 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0258 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2707 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0640 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2709 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04B0 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x270D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x270F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2711 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2713 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x064D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2715 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0111 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2717 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x046C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2719 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x005A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x01BE , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0131 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02B3 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2721 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0896 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2723 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2725 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2727 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04BB , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2729 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x064B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0111 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0024 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x003A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2731 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x00F6 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2733 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x008B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2735 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x050D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2737 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x080E , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2739 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x031F , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0257 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2747 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2749 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x063F , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x274B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x274D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04AF , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x222D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x005B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA408 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0015 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA409 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0017 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2411 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x005B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2413 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x006D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2415 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0061 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2417 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0074 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA404 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0010 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0003 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA410 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000A , 4);		// MCU_DATA_0
	//[AE]
		
	mt_i2c_write(0x098C, 0xA117 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA11D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA129 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA24F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0014 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA216 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0091 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0091 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2212 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x007F , 4);		// MCU_DATA_0

	
#if 1
	mt_i2c_write(0x3210, 0x01B0 , 4);		// COLOR_PIPELINE_CONTROL
	mt_i2c_write(0x364E, 0x0410 , 4);		// P_GR_P0Q0
	mt_i2c_write(0x3650, 0x65EB , 4);		// P_GR_P0Q1
	mt_i2c_write(0x3652, 0x01F2 , 4);		// P_GR_P0Q2
	mt_i2c_write(0x3654, 0x982F , 4);		// P_GR_P0Q3
	mt_i2c_write(0x3656, 0xB773 , 4);		// P_GR_P0Q4
	mt_i2c_write(0x3658, 0x7F4F , 4);		// P_RD_P0Q0
	mt_i2c_write(0x365A, 0x994B , 4);		// P_RD_P0Q1
	mt_i2c_write(0x365C, 0x0D12 , 4);		// P_RD_P0Q2
	mt_i2c_write(0x365E, 0x7BEA , 4);		// P_RD_P0Q3
	mt_i2c_write(0x3660, 0x9193 , 4);		// P_RD_P0Q4
	mt_i2c_write(0x3662, 0x0030 , 4);		// P_BL_P0Q0
	mt_i2c_write(0x3664, 0xB62B , 4);		// P_BL_P0Q1
	mt_i2c_write(0x3666, 0x4A31 , 4);		// P_BL_P0Q2
	mt_i2c_write(0x3668, 0x6AAE , 4);		// P_BL_P0Q3
	mt_i2c_write(0x366A, 0x83D3 , 4);		// P_BL_P0Q4
	mt_i2c_write(0x366C, 0x7F6F , 4);		// P_GB_P0Q0
	mt_i2c_write(0x366E, 0x282B , 4);		// P_GB_P0Q1
	mt_i2c_write(0x3670, 0x0332 , 4);		// P_GB_P0Q2
	mt_i2c_write(0x3672, 0x998F , 4);		// P_GB_P0Q3
	mt_i2c_write(0x3674, 0xBB93 , 4);		// P_GB_P0Q4
	mt_i2c_write(0x3676, 0x738C , 4);		// P_GR_P1Q0
	mt_i2c_write(0x3678, 0xF96F , 4);		// P_GR_P1Q1
	mt_i2c_write(0x367A, 0x84B0 , 4);		// P_GR_P1Q2
	mt_i2c_write(0x367C, 0x0952 , 4);		// P_GR_P1Q3
	mt_i2c_write(0x367E, 0x39B1 , 4);		// P_GR_P1Q4
	mt_i2c_write(0x3680, 0x094D , 4);		// P_RD_P1Q0
	mt_i2c_write(0x3682, 0x218F , 4);		// P_RD_P1Q1
	mt_i2c_write(0x3684, 0x6A8F , 4);		// P_RD_P1Q2
	mt_i2c_write(0x3686, 0xBAB1 , 4);		// P_RD_P1Q3
	mt_i2c_write(0x3688, 0xE951 , 4);		// P_RD_P1Q4
	mt_i2c_write(0x368A, 0x8DCC , 4);		// P_BL_P1Q0
	mt_i2c_write(0x368C, 0x862F , 4);		// P_BL_P1Q1
	mt_i2c_write(0x368E, 0x4E0E , 4);		// P_BL_P1Q2
	mt_i2c_write(0x3690, 0x3D51 , 4);		// P_BL_P1Q3
	mt_i2c_write(0x3692, 0x8F4F , 4);		// P_BL_P1Q4
	mt_i2c_write(0x3694, 0x038B , 4);		// P_GB_P1Q0
	mt_i2c_write(0x3696, 0x442F , 4);		// P_GB_P1Q1
	mt_i2c_write(0x3698, 0x8270 , 4);		// P_GB_P1Q2
	mt_i2c_write(0x369A, 0x9911 , 4);		// P_GB_P1Q3
	mt_i2c_write(0x369C, 0x21AF , 4);		// P_GB_P1Q4
	mt_i2c_write(0x369E, 0x5BD2 , 4);		// P_GR_P2Q0
	mt_i2c_write(0x36A0, 0x9C70 , 4);		// P_GR_P2Q1
	mt_i2c_write(0x36A2, 0x9D34 , 4);		// P_GR_P2Q2
	mt_i2c_write(0x36A4, 0xDB8A , 4);		// P_GR_P2Q3
	mt_i2c_write(0x36A6, 0xC075 , 4);		// P_GR_P2Q4
	mt_i2c_write(0x36A8, 0x5652 , 4);		// P_RD_P2Q0
	mt_i2c_write(0x36AA, 0x44B0 , 4);		// P_RD_P2Q1
	mt_i2c_write(0x36AC, 0xF272 , 4);		// P_RD_P2Q2
	mt_i2c_write(0x36AE, 0xC373 , 4);		// P_RD_P2Q3
	mt_i2c_write(0x36B0, 0xD276 , 4);		// P_RD_P2Q4
	mt_i2c_write(0x36B2, 0x2692 , 4);		// P_BL_P2Q0
	mt_i2c_write(0x36B4, 0x25D0 , 4);		// P_BL_P2Q1
	mt_i2c_write(0x36B6, 0xA533 , 4);		// P_BL_P2Q2
	mt_i2c_write(0x36B8, 0x8793 , 4);		// P_BL_P2Q3
	mt_i2c_write(0x36BA, 0x8676 , 4);		// P_BL_P2Q4
	mt_i2c_write(0x36BC, 0x5F12 , 4);		// P_GB_P2Q0
	mt_i2c_write(0x36BE, 0x9310 , 4);		// P_GB_P2Q1
	mt_i2c_write(0x36C0, 0xF253 , 4);		// P_GB_P2Q2
	mt_i2c_write(0x36C2, 0x2910 , 4);		// P_GB_P2Q3
	mt_i2c_write(0x36C4, 0xA316 , 4);		// P_GB_P2Q4
	mt_i2c_write(0x36C6, 0xFDAE , 4);		// P_GR_P3Q0
	mt_i2c_write(0x36C8, 0x4F51 , 4);		// P_GR_P3Q1
	mt_i2c_write(0x36CA, 0x6FD3 , 4);		// P_GR_P3Q2
	mt_i2c_write(0x36CC, 0x6910 , 4);		// P_GR_P3Q3
	mt_i2c_write(0x36CE, 0xA076 , 4);		// P_GR_P3Q4
	mt_i2c_write(0x36D0, 0x90ED , 4);		// P_RD_P3Q0
	mt_i2c_write(0x36D2, 0xB9EF , 4);		// P_RD_P3Q1
	mt_i2c_write(0x36D4, 0xC573 , 4);		// P_RD_P3Q2
	mt_i2c_write(0x36D6, 0x9933 , 4);		// P_RD_P3Q3
	mt_i2c_write(0x36D8, 0xE912 , 4);		// P_RD_P3Q4
	mt_i2c_write(0x36DA, 0x9470 , 4);		// P_BL_P3Q0
	mt_i2c_write(0x36DC, 0x4CAF , 4);		// P_BL_P3Q1
	mt_i2c_write(0x36DE, 0x31F0 , 4);		// P_BL_P3Q2
	mt_i2c_write(0x36E0, 0xB951 , 4);		// P_BL_P3Q3
	mt_i2c_write(0x36E2, 0x8E96 , 4);		// P_BL_P3Q4
	mt_i2c_write(0x36E4, 0x140D , 4);		// P_GB_P3Q0
	mt_i2c_write(0x36E6, 0x9ACF , 4);		// P_GB_P3Q1
	mt_i2c_write(0x36E8, 0x0DB2 , 4);		// P_GB_P3Q2
	mt_i2c_write(0x36EA, 0x9B33 , 4);		// P_GB_P3Q3
	mt_i2c_write(0x36EC, 0xD7F3 , 4);		// P_GB_P3Q4
	mt_i2c_write(0x36EE, 0xC034 , 4);		// P_GR_P4Q0
	mt_i2c_write(0x36F0, 0xB710 , 4);		// P_GR_P4Q1
	mt_i2c_write(0x36F2, 0xE0F7 , 4);		// P_GR_P4Q2
	mt_i2c_write(0x36F4, 0x64B4 , 4);		// P_GR_P4Q3
	mt_i2c_write(0x36F6, 0x143B , 4);		// P_GR_P4Q4
	mt_i2c_write(0x36F8, 0xDE93 , 4);		// P_RD_P4Q0
	mt_i2c_write(0x36FA, 0xA9B2 , 4);		// P_RD_P4Q1
	mt_i2c_write(0x36FC, 0xAC58 , 4);		// P_RD_P4Q2
	mt_i2c_write(0x36FE, 0x7A75 , 4);		// P_RD_P4Q3
	mt_i2c_write(0x3700, 0x19FB , 4);		// P_RD_P4Q4
	mt_i2c_write(0x3702, 0xB513 , 4);		// P_BL_P4Q0
	mt_i2c_write(0x3704, 0x5E51 , 4);		// P_BL_P4Q1
	mt_i2c_write(0x3706, 0x9738 , 4);		// P_BL_P4Q2
	mt_i2c_write(0x3708, 0xBA14 , 4);		// P_BL_P4Q3
	mt_i2c_write(0x370A, 0x1DBB , 4);		// P_BL_P4Q4
	mt_i2c_write(0x370C, 0xBF74 , 4);		// P_GB_P4Q0
	mt_i2c_write(0x370E, 0x31B1 , 4);		// P_GB_P4Q1
	mt_i2c_write(0x3710, 0x95B8 , 4);		// P_GB_P4Q2
	mt_i2c_write(0x3712, 0xA9D4 , 4);		// P_GB_P4Q3
	mt_i2c_write(0x3714, 0x31DB , 4);		// P_GB_P4Q4
	mt_i2c_write(0x3644, 0x0320 , 4);		// POLY_ORIGIN_C
	mt_i2c_write(0x3642, 0x0258 , 4);		// POLY_ORIGIN_R
	mt_i2c_write(0x3210, 0x01B8 , 4);		// COLOR_PIPELINE_CONTROL
	mt_i2c_write(0x098C, 0x2306 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x01D6 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2308 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF89 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFA1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF73 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x019C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2310 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2312 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFB0 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2314 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF2D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2316 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0223 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001E , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0022 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x002C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0024 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFCD , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0023 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2320 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0010 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2322 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0026 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2324 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFE9 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2326 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2328 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x003A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x005D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF69 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFE4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0006 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFFC , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA36D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA36E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0020 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA206 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x0415 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xF601 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x42C1 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x0326 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x11F6 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0143 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xC104 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x260A , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC04 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0425 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x33BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA362 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xBD04 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x3339 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xC6FF , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xF701 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x6439 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xFE01 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0435 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x6918 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xCE03 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x25CC , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0013 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xBDC2 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xB8CC , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0489 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xFD03 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0445 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x27CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x0325 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xFD01 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x69FE , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x02BD , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x18CE , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0339 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0455 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x11BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xC2B8 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xCC04 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xC8FD , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0347 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xCC03 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x39FD , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x02BD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0465 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xDE00 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x18CE , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x00C2 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xCC00 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x37BD , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xC2B8 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xCC04 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xEFDD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0475 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xE6CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x00C2 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xDD00 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xC601 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF701 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x64C6 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x03F7 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x0165 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0485 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x7F01 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x6639 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x3C3C , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x3C34 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xCC32 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x3EBD , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xA558 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x30ED , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0495 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xB2D7 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x30E7 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x06CC , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x323E , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xED00 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xEC04 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xBDA5 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04A5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x44CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x3244 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xBDA5 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x585F , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x30ED , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x02CC , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x3244 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xED00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04B5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xF601 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xD54F , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xEA03 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xAA02 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xBDA5 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x4430 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xE606 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x3838 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04C5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x3831 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x39BD , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xD661 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xF602 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF4C1 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x0126 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0BFE , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x02BD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04D5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xEE10 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xFC02 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xF5AD , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0039 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF602 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xF4C1 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0226 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x0AFE , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04E5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xEE10 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xFC02 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xF7AD , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0039 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x3CBD , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xB059 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04F5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x28BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA558 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x8300 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0027 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0BCC , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x0026 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x30ED , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x00C6 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0505 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x03BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA544 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x3839 , 4);		// MCU_DATA_2
	mt_i2c_write(0x098C, 0x2006 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0415 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA005 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xAB20 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0060 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0020 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA206 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA11E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA108 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA10A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA10C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
#endif
	//-----------------------------------------------------
	mt_i2c_write(0x001E, 0x0477, 4);		// slew
	
	//for mode a/b output format
	mt_i2c_write(0x098C, 0x2755, 4);		// mode_output_format_a
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence		
	mt_i2c_write(0x098C, 0x2757, 4);		// mode_output_format_b 
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence
	//----------------------------------------------------

	mt_i2c_write(0x0018, 0x0028 , 4);		// STANDBY_CONTROL
  	msleep(200);	
	mt_i2c_write(0x098C, 0xA103 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0006 , 4);		// MCU_DATA_0
  	msleep(100);	
	mt_i2c_write(0x098C, 0xA103 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0005 , 4);		// MCU_DATA_0
  	msleep(100);	
	
	#endif 

	#if 0
	// 20120130
	//Reset 
  	mt_i2c_write(0x001A, 0x0051, 4);
  	msleep(50);  //10
  	mt_i2c_write(0x001A, 0x0050, 4);
  	msleep(50);  //10
  	
	//[PLL]
	mt_i2c_write(0x0014, 0x2545, 4);	//PLL Control: BYPASS PLL = 9541
	mt_i2c_write(0x0010, 0x0010, 4);	//PLL Dividers = 16
	mt_i2c_write(0x0012, 0x1FF7, 4);	//PLL P Dividers = 8183
	mt_i2c_write(0x0014, 0x2547, 4);	//PLL Control: PLL_ENABLE on = 9543
	mt_i2c_write(0x0014, 0x2447, 4);	//PLL Control: SEL_LOCK_DET on = 9287
	msleep(10);               // Allow PLL to lock
	mt_i2c_write(0x0014, 0x2047, 4);	//PLL Control: PLL_BYPASS off = 8263
	mt_i2c_write(0x0014, 0x2046, 4);	//PLL Control: = 8262
	mt_i2c_write(0x0018, 0x4028, 4); 	// STANDBY_CONTROL
	msleep(30);
	
	//[Timing]
	mt_i2c_write(0x321C,0x0003, 4);	//By Pass TxFIFO = 3
	mt_i2c_write(0x98C, 0x2703, 4);	//Output Width (A)
	mt_i2c_write(0x990, 0x0320, 4);	//      = 800
	mt_i2c_write(0x98C, 0x2705, 4);	//Output Height (A)
	mt_i2c_write(0x990, 0x0258, 4);	//      = 600
	mt_i2c_write(0x98C, 0x2707, 4);	//Output Width (B)
	mt_i2c_write(0x990, 0x0640, 4);	//      = 1600
	mt_i2c_write(0x98C, 0x2709, 4);	//Output Height (B)
	mt_i2c_write(0x990, 0x04B0, 4);	//      = 1200
	mt_i2c_write(0x98C, 0x270D, 4);	//Row Start (A)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x270F, 4);	//Column Start (A)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x2711, 4);	//Row End (A)
	mt_i2c_write(0x990, 0x4BD,  4);	//      = 1213
	mt_i2c_write(0x98C, 0x2713, 4);	//Column End (A)
	mt_i2c_write(0x990, 0x064D, 4);	//      = 1613
	mt_i2c_write(0x98C, 0x2715, 4);	//Row Speed (A)
	mt_i2c_write(0x990, 0x0111, 4);	//      = 273
	mt_i2c_write(0x98C, 0x2717, 4);	//Read Mode (A)
	mt_i2c_write(0x990, 0x006C, 4);	//      = 108
	mt_i2c_write(0x98C, 0x2719, 4);	//sensor_fine_correction (A)
	mt_i2c_write(0x990, 0x003A, 4);	//      = 58
	mt_i2c_write(0x98C, 0x271B, 4);	//sensor_fine_IT_min (A)
	mt_i2c_write(0x990, 0x00F6, 4);	//      = 246
	mt_i2c_write(0x98C, 0x271D, 4);	//sensor_fine_IT_max_margin (A)
	mt_i2c_write(0x990, 0x008B, 4);	//      = 139
	mt_i2c_write(0x98C, 0x271F, 4);	//Frame Lines (A)
	mt_i2c_write(0x990, 0x02B5, 4);	//      = 693
	mt_i2c_write(0x98C, 0x2721, 4);	//Line Length (A)
	mt_i2c_write(0x990, 0x088F, 4);	//      = 2191
	mt_i2c_write(0x98C, 0x2723, 4);	//Row Start (B)
	mt_i2c_write(0x990, 0x0004, 4);	//      = 4
	mt_i2c_write(0x98C, 0x2725, 4);	//Column Start (B)
	mt_i2c_write(0x990, 0x0004, 4);	//      = 4
	mt_i2c_write(0x98C, 0x2727, 4);	//Row End (B)
	mt_i2c_write(0x990, 0x4BB, 4);  //      = 1211
	mt_i2c_write(0x98C, 0x2729, 4);	//Column End (B)
	mt_i2c_write(0x990, 0x64B, 4);  //      = 1611
	mt_i2c_write(0x98C, 0x272B, 4);	//Row Speed (B)
	mt_i2c_write(0x990, 0x0111, 4);	//      = 273
	mt_i2c_write(0x98C, 0x272D, 4);	//Read Mode (B)
	mt_i2c_write(0x990, 0x0024, 4);	//      = 36
	mt_i2c_write(0x98C, 0x272F, 4);	//sensor_fine_correction (B)
	mt_i2c_write(0x990, 0x003A, 4);	//      = 58
	mt_i2c_write(0x98C, 0x2731, 4);	//sensor_fine_IT_min (B)
	mt_i2c_write(0x990, 0x00F6, 4);	//      = 246
	mt_i2c_write(0x98C, 0x2733, 4);	//sensor_fine_IT_max_margin (B)
	mt_i2c_write(0x990, 0x008B, 4);	//      = 139
	mt_i2c_write(0x98C, 0x2735, 4);	//Frame Lines (B)
	mt_i2c_write(0x990, 0x050D, 4);	//      = 1293
	mt_i2c_write(0x98C, 0x2737, 4);	//Line Length (B)
	mt_i2c_write(0x990, 0x080E, 4);	//      = 2062
	mt_i2c_write(0x98C, 0x2739, 4);	//Crop_X0 (A)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x273B, 4);	//Crop_X1 (A)
	mt_i2c_write(0x990, 0x031F, 4);	//      = 799
	mt_i2c_write(0x98C, 0x273D, 4);	//Crop_Y0 (A)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x273F, 4);	//Crop_Y1 (A)
	mt_i2c_write(0x990, 0x0257, 4);	//      = 599
	mt_i2c_write(0x98C, 0x2747, 4);	//Crop_X0 (B)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x2749, 4);	//Crop_X1 (B)
	mt_i2c_write(0x990, 0x063F, 4);	//      = 1599
	mt_i2c_write(0x98C, 0x274B, 4);	//Crop_Y0 (B)
	mt_i2c_write(0x990, 0x0000, 4);	//      = 0
	mt_i2c_write(0x98C, 0x274D, 4);	//Crop_Y1 (B)
	mt_i2c_write(0x990, 0x04AF, 4);	//      = 1199
	mt_i2c_write(0x98C, 0x222D, 4);	//R9 Step
	mt_i2c_write(0x990, 0x005B, 4);	//      = 91
	mt_i2c_write(0x98C, 0xA408, 4);	//search_f1_50
	mt_i2c_write(0x990, 0x0015, 4);	//      = 21
	mt_i2c_write(0x98C, 0xA409, 4);	//search_f2_50
	mt_i2c_write(0x990, 0x0017, 4);	//      = 23
	mt_i2c_write(0x98C, 0xA40A, 4);	//search_f1_60
	mt_i2c_write(0x990, 0x001A, 4);	//      = 26
	mt_i2c_write(0x98C, 0xA40B, 4);	//search_f2_60
	mt_i2c_write(0x990, 0x001C, 4);	//      = 28
	mt_i2c_write(0x98C, 0x2411, 4);	//R9_Step_60 (A)
	mt_i2c_write(0x990, 0x005B, 4);	//      = 91
	mt_i2c_write(0x98C, 0x2413, 4);	//R9_Step_50 (A)
	mt_i2c_write(0x990, 0x006E, 4);	//      = 110
	mt_i2c_write(0x98C, 0x2415, 4);	//R9_Step_60 (B)
	mt_i2c_write(0x990, 0x0061, 4);	//      = 97
	mt_i2c_write(0x98C, 0x2417, 4);	//R9_Step_50 (B)
	mt_i2c_write(0x990, 0x0074, 4);	//      = 116
	mt_i2c_write(0x98C, 0xA404, 4);	//FD Mode
	mt_i2c_write(0x990, 0x0010, 4);	//      = 16
	mt_i2c_write(0x98C, 0xA40D, 4);	//Stat_min
	mt_i2c_write(0x990, 0x0002, 4);	//      = 2
	mt_i2c_write(0x98C, 0xA40E, 4);	//Stat_max
	mt_i2c_write(0x990, 0x0003, 4);	//      = 3
	mt_i2c_write(0x98C, 0xA410, 4);	//Min_amplitude
	mt_i2c_write(0x990, 0x000A, 4);	//      = 10
	
	
	mt_i2c_write(0x098C, 0xA20E, 4); 	// MCU_ADDRESS [AE_MAX_VIRTGAIN]
	mt_i2c_write(0x0990, 0x0080, 4); 	// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2212, 4); 	// MCU_ADDRESS [AE_MAX_DGAIN_AE1]
	mt_i2c_write(0x0990, 0x0100, 4); 	// MCU_DATA_0
	                                      
	mt_i2c_write(0x3084, 0x240C, 4); 	// RESERVED_CORE_3084                                     
	mt_i2c_write(0x3092, 0x0A4C, 4); 	// RESERVED_CORE_3092                                     
	mt_i2c_write(0x3094, 0x4C4C, 4); 	// RESERVED_CORE_3094                                     
	mt_i2c_write(0x3096, 0x4C54, 4); 	// RESERVED_CORE_3096                                     
	mt_i2c_write(0x3210, 0x01B8, 4); 	// COLOR_PIPELINE_CONTROL                                 
	mt_i2c_write(0x098C, 0x2B28, 4); 	// MCU_ADDRESS [HG_LL_BRIGHTNESSSTART]                    
	mt_i2c_write(0x0990, 0x35E8, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2B2A, 4); 	// MCU_ADDRESS [HG_LL_BRIGHTNESSSTOP]                     
	mt_i2c_write(0x0990, 0xB3B0, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB20, 4); 	// MCU_ADDRESS [HG_LL_SAT1]                               
	mt_i2c_write(0x0990, 0x004B, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB24, 4); 	// MCU_ADDRESS [HG_LL_SAT2]                               
	mt_i2c_write(0x0990, 0x0000, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB25, 4); 	// MCU_ADDRESS [HG_LL_INTERPTHRESH2]                      
	mt_i2c_write(0x0990, 0x00FF, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB30, 4); 	// MCU_ADDRESS [HG_NR_STOP_R]                             
	mt_i2c_write(0x0990, 0x00FF, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB31, 4); 	// MCU_ADDRESS [HG_NR_STOP_G]                             
	mt_i2c_write(0x0990, 0x00FF, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB32, 4); 	// MCU_ADDRESS [HG_NR_STOP_B]                             
	mt_i2c_write(0x0990, 0x00FF, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB33, 4); 	// MCU_ADDRESS [HG_NR_STOP_OL]                            
	mt_i2c_write(0x0990, 0x0057, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB34, 4); 	// MCU_ADDRESS [HG_NR_GAINSTART]                          
	mt_i2c_write(0x0990, 0x0080, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB35, 4); 	// MCU_ADDRESS [HG_NR_GAINSTOP]                           
	mt_i2c_write(0x0990, 0x00FF, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB36, 4); 	// MCU_ADDRESS [HG_CLUSTERDC_TH]                          
	mt_i2c_write(0x0990, 0x0014, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xAB37, 4); 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]                      
	mt_i2c_write(0x0990, 0x0003, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2B38, 4); 	// MCU_ADDRESS [HG_GAMMASTARTMORPH]                       
	mt_i2c_write(0x0990, 0x32C8, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2B3A, 4); 	// MCU_ADDRESS [HG_GAMMASTOPMORPH]                        
	mt_i2c_write(0x0990, 0x7918, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2B62, 4); 	// MCU_ADDRESS [HG_FTB_START_BM]                          
	mt_i2c_write(0x0990, 0xFFFE, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2B64, 4); 	// MCU_ADDRESS [HG_FTB_STOP_BM]                           
	mt_i2c_write(0x0990, 0xFFFF, 4); 	// MCU_DATA_0  
	
	//[AWB AND CCM]                                           
	mt_i2c_write(0x098C, 0x2306, 4); 	// MCU_ADDRESS [AWB_CCM_L_0]                              
	mt_i2c_write(0x0990, 0x0180, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2308, 4); 	// MCU_ADDRESS [AWB_CCM_L_1]                              
	mt_i2c_write(0x0990, 0xFF00, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x230A, 4); 	// MCU_ADDRESS [AWB_CCM_L_2]                              
	mt_i2c_write(0x0990, 0x0080, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x230C, 4); 	// MCU_ADDRESS [AWB_CCM_L_3]                              
	mt_i2c_write(0x0990, 0xFF66, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x230E, 4); 	// MCU_ADDRESS [AWB_CCM_L_4]                              
	mt_i2c_write(0x0990, 0x0180, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2310, 4); 	// MCU_ADDRESS [AWB_CCM_L_5]                              
	mt_i2c_write(0x0990, 0xFFEE, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2312, 4); 	// MCU_ADDRESS [AWB_CCM_L_6]                              
	mt_i2c_write(0x0990, 0xFFCD, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2314, 4); 	// MCU_ADDRESS [AWB_CCM_L_7]                              
	mt_i2c_write(0x0990, 0xFECD, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2316, 4); 	// MCU_ADDRESS [AWB_CCM_L_8]                              
	mt_i2c_write(0x0990, 0x019A, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2318, 4); 	// MCU_ADDRESS [AWB_CCM_L_9]                              
	mt_i2c_write(0x0990, 0x0020, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x231A, 4); 	// MCU_ADDRESS [AWB_CCM_L_10]                             
	mt_i2c_write(0x0990, 0x0033, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x231C, 4); 	// MCU_ADDRESS [AWB_CCM_RL_0]                             
	mt_i2c_write(0x0990, 0x0100, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x231E, 4); 	// MCU_ADDRESS [AWB_CCM_RL_1]                             
	mt_i2c_write(0x0990, 0xFF9A, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2320, 4); 	// MCU_ADDRESS [AWB_CCM_RL_2]                             
	mt_i2c_write(0x0990, 0x0000, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2322, 4); 	// MCU_ADDRESS [AWB_CCM_RL_3]                             
	mt_i2c_write(0x0990, 0x004D, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2324, 4); 	// MCU_ADDRESS [AWB_CCM_RL_4]                             
	mt_i2c_write(0x0990, 0xFFCD, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2326, 4); 	// MCU_ADDRESS [AWB_CCM_RL_5]                             
	mt_i2c_write(0x0990, 0xFFB8, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2328, 4); 	// MCU_ADDRESS [AWB_CCM_RL_6]                             
	mt_i2c_write(0x0990, 0x004D, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x232A, 4); 	// MCU_ADDRESS [AWB_CCM_RL_7]                             
	mt_i2c_write(0x0990, 0x0080, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x232C, 4); 	// MCU_ADDRESS [AWB_CCM_RL_8]                             
	mt_i2c_write(0x0990, 0xFF66, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x232E, 4); 	// MCU_ADDRESS [AWB_CCM_RL_9]                             
	mt_i2c_write(0x0990, 0x0008, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0x2330, 4); 	// MCU_ADDRESS [AWB_CCM_RL_10]                            
	mt_i2c_write(0x0990, 0xFFF7, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xA363, 4); 	// MCU_ADDRESS [AWB_TG_MIN0]                              
	mt_i2c_write(0x0990, 0x00D2, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xA364, 4); 	// MCU_ADDRESS [AWB_TG_MAX0]                              
	mt_i2c_write(0x0990, 0x00EE, 4); 	// MCU_DATA_0  
	
	//[Patch]                                           
	mt_i2c_write(0x3244, 0x0328, 4); 	// RESERVED_SOC1_3244                                     
	mt_i2c_write(0x323E, 0xC22C, 4); 	// RESERVED_SOC1_323E                                     
	mt_i2c_write(0x098C, 0x0415, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xF601, 4);                                                             
	mt_i2c_write(0x0992, 0x42C1, 4);                                                             
	mt_i2c_write(0x0994, 0x0326, 4);                                                             
	mt_i2c_write(0x0996, 0x11F6, 4);                                                             
	mt_i2c_write(0x0998, 0x0143, 4);                                                             
	mt_i2c_write(0x099A, 0xC104, 4);                                                             
	mt_i2c_write(0x099C, 0x260A, 4);                                                             
	mt_i2c_write(0x099E, 0xCC04, 4);                                                             
	mt_i2c_write(0x098C, 0x0425, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x33BD, 4);                                                             
	mt_i2c_write(0x0992, 0xA362, 4);                                                             
	mt_i2c_write(0x0994, 0xBD04, 4);                                                             
	mt_i2c_write(0x0996, 0x3339, 4);                                                             
	mt_i2c_write(0x0998, 0xC6FF, 4);                                                             
	mt_i2c_write(0x099A, 0xF701, 4);                                                             
	mt_i2c_write(0x099C, 0x6439, 4);                                                             
	mt_i2c_write(0x099E, 0xDE5D, 4);                                                             
	mt_i2c_write(0x098C, 0x0435, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x18CE, 4);                                                             
	mt_i2c_write(0x0992, 0x0325, 4);                                                             
	mt_i2c_write(0x0994, 0xCC00, 4);                                                             
	mt_i2c_write(0x0996, 0x27BD, 4);                                                             
	mt_i2c_write(0x0998, 0xC2B8, 4);                                                             
	mt_i2c_write(0x099A, 0xCC04, 4);                                                             
	mt_i2c_write(0x099C, 0xBDFD, 4);                                                             
	mt_i2c_write(0x099E, 0x033B, 4);                                                             
	mt_i2c_write(0x098C, 0x0445, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xCC06, 4);                                                             
	mt_i2c_write(0x0992, 0x6BFD, 4);                                                             
	mt_i2c_write(0x0994, 0x032F, 4);                                                             
	mt_i2c_write(0x0996, 0xCC03, 4);                                                             
	mt_i2c_write(0x0998, 0x25DD, 4);                                                             
	mt_i2c_write(0x099A, 0x5DC6, 4);                                                             
	mt_i2c_write(0x099C, 0x1ED7, 4);                                                             
	mt_i2c_write(0x099E, 0x6CD7, 4);                                                             
	mt_i2c_write(0x098C, 0x0455, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x6D5F, 4);                                                             
	mt_i2c_write(0x0992, 0xD76E, 4);                                                             
	mt_i2c_write(0x0994, 0xD78D, 4);                                                             
	mt_i2c_write(0x0996, 0x8620, 4);                                                             
	mt_i2c_write(0x0998, 0x977A, 4);                                                             
	mt_i2c_write(0x099A, 0xD77B, 4);                                                             
	mt_i2c_write(0x099C, 0x979A, 4);                                                             
	mt_i2c_write(0x099E, 0xC621, 4);                                                             
	mt_i2c_write(0x098C, 0x0465, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xD79B, 4);                                                             
	mt_i2c_write(0x0992, 0xFE01, 4);                                                             
	mt_i2c_write(0x0994, 0x6918, 4);                                                             
	mt_i2c_write(0x0996, 0xCE03, 4);                                                             
	mt_i2c_write(0x0998, 0x4DCC, 4);                                                             
	mt_i2c_write(0x099A, 0x0013, 4);                                                             
	mt_i2c_write(0x099C, 0xBDC2, 4);                                                             
	mt_i2c_write(0x099E, 0xB8CC, 4);                                                             
	mt_i2c_write(0x098C, 0x0475, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x05E9, 4);                                                             
	mt_i2c_write(0x0992, 0xFD03, 4);                                                             
	mt_i2c_write(0x0994, 0x4FCC, 4);                                                             
	mt_i2c_write(0x0996, 0x034D, 4);                                                             
	mt_i2c_write(0x0998, 0xFD01, 4);                                                             
	mt_i2c_write(0x099A, 0x69FE, 4);                                                             
	mt_i2c_write(0x099C, 0x02BD, 4);                                                             
	mt_i2c_write(0x099E, 0x18CE, 4);                                                             
	mt_i2c_write(0x098C, 0x0485, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x0361, 4);                                                             
	mt_i2c_write(0x0992, 0xCC00, 4);                                                             
	mt_i2c_write(0x0994, 0x11BD, 4);                                                             
	mt_i2c_write(0x0996, 0xC2B8, 4);                                                             
	mt_i2c_write(0x0998, 0xCC06, 4);                                                             
	mt_i2c_write(0x099A, 0x28FD, 4);                                                             
	mt_i2c_write(0x099C, 0x036F, 4);                                                             
	mt_i2c_write(0x099E, 0xCC03, 4);                                                             
	mt_i2c_write(0x098C, 0x0495, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x61FD, 4);                                                             
	mt_i2c_write(0x0992, 0x02BD, 4);                                                             
	mt_i2c_write(0x0994, 0xDE00, 4);                                                             
	mt_i2c_write(0x0996, 0x18CE, 4);                                                             
	mt_i2c_write(0x0998, 0x00C2, 4);                                                             
	mt_i2c_write(0x099A, 0xCC00, 4);                                                             
	mt_i2c_write(0x099C, 0x37BD, 4);                                                             
	mt_i2c_write(0x099E, 0xC2B8, 4);                                                             
	mt_i2c_write(0x098C, 0x04A5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xCC06, 4);                                                             
	mt_i2c_write(0x0992, 0x4FDD, 4);                                                             
	mt_i2c_write(0x0994, 0xE6CC, 4);                                                             
	mt_i2c_write(0x0996, 0x00C2, 4);                                                             
	mt_i2c_write(0x0998, 0xDD00, 4);                                                             
	mt_i2c_write(0x099A, 0xC601, 4);                                                             
	mt_i2c_write(0x099C, 0xF701, 4);                                                             
	mt_i2c_write(0x099E, 0x64C6, 4);                                                             
	mt_i2c_write(0x098C, 0x04B5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x05F7, 4);                                                             
	mt_i2c_write(0x0992, 0x0165, 4);                                                             
	mt_i2c_write(0x0994, 0x7F01, 4);                                                             
	mt_i2c_write(0x0996, 0x6639, 4);                                                             
	mt_i2c_write(0x0998, 0x373C, 4);                                                             
	mt_i2c_write(0x099A, 0x3C3C, 4);                                                             
	mt_i2c_write(0x099C, 0x3C3C, 4);                                                             
	mt_i2c_write(0x099E, 0x30EC, 4);                                                             
	mt_i2c_write(0x098C, 0x04C5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x11ED, 4);                                                             
	mt_i2c_write(0x0992, 0x02EC, 4);                                                             
	mt_i2c_write(0x0994, 0x0FED, 4);                                                             
	mt_i2c_write(0x0996, 0x008F, 4);                                                             
	mt_i2c_write(0x0998, 0x30ED, 4);                                                             
	mt_i2c_write(0x099A, 0x04EC, 4);                                                             
	mt_i2c_write(0x099C, 0x0DEE, 4);                                                             
	mt_i2c_write(0x099E, 0x04BD, 4);                                                             
	mt_i2c_write(0x098C, 0x04D5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xA406, 4);                                                             
	mt_i2c_write(0x0992, 0x30EC, 4);                                                             
	mt_i2c_write(0x0994, 0x02ED, 4);                                                             
	mt_i2c_write(0x0996, 0x06FC, 4);                                                             
	mt_i2c_write(0x0998, 0x10C0, 4);                                                             
	mt_i2c_write(0x099A, 0x2705, 4);                                                             
	mt_i2c_write(0x099C, 0xCCFF, 4);                                                             
	mt_i2c_write(0x099E, 0xFFED, 4);                                                             
	mt_i2c_write(0x098C, 0x04E5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x06F6, 4);                                                             
	mt_i2c_write(0x0992, 0x0256, 4);                                                             
	mt_i2c_write(0x0994, 0x8616, 4);                                                             
	mt_i2c_write(0x0996, 0x3DC3, 4);                                                             
	mt_i2c_write(0x0998, 0x0261, 4);                                                             
	mt_i2c_write(0x099A, 0x8FE6, 4);                                                             
	mt_i2c_write(0x099C, 0x09C4, 4);                                                             
	mt_i2c_write(0x099E, 0x07C1, 4);                                                             
	mt_i2c_write(0x098C, 0x04F5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x0226, 4);                                                             
	mt_i2c_write(0x0992, 0x1DFC, 4);                                                             
	mt_i2c_write(0x0994, 0x10C2, 4);                                                             
	mt_i2c_write(0x0996, 0x30ED, 4);                                                             
	mt_i2c_write(0x0998, 0x02FC, 4);                                                             
	mt_i2c_write(0x099A, 0x10C0, 4);                                                             
	mt_i2c_write(0x099C, 0xED00, 4);                                                             
	mt_i2c_write(0x099E, 0xC602, 4);                                                             
	mt_i2c_write(0x098C, 0x0505, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xBDC2, 4);                                                             
	mt_i2c_write(0x0992, 0x5330, 4);                                                             
	mt_i2c_write(0x0994, 0xEC00, 4);                                                             
	mt_i2c_write(0x0996, 0xFD10, 4);                                                             
	mt_i2c_write(0x0998, 0xC0EC, 4);                                                             
	mt_i2c_write(0x099A, 0x02FD, 4);                                                             
	mt_i2c_write(0x099C, 0x10C2, 4);                                                             
	mt_i2c_write(0x099E, 0x201B, 4);                                                             
	mt_i2c_write(0x098C, 0x0515, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xFC10, 4);                                                             
	mt_i2c_write(0x0992, 0xC230, 4);                                                             
	mt_i2c_write(0x0994, 0xED02, 4);                                                             
	mt_i2c_write(0x0996, 0xFC10, 4);                                                             
	mt_i2c_write(0x0998, 0xC0ED, 4);                                                             
	mt_i2c_write(0x099A, 0x00C6, 4);                                                             
	mt_i2c_write(0x099C, 0x01BD, 4);                                                             
	mt_i2c_write(0x099E, 0xC253, 4);                                                             
	mt_i2c_write(0x098C, 0x0525, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x30EC, 4);                                                             
	mt_i2c_write(0x0992, 0x00FD, 4);                                                             
	mt_i2c_write(0x0994, 0x10C0, 4);                                                             
	mt_i2c_write(0x0996, 0xEC02, 4);                                                             
	mt_i2c_write(0x0998, 0xFD10, 4);                                                             
	mt_i2c_write(0x099A, 0xC2C6, 4);                                                             
	mt_i2c_write(0x099C, 0x80D7, 4);                                                             
	mt_i2c_write(0x099E, 0x85C6, 4);                                                             
	mt_i2c_write(0x098C, 0x0535, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x40F7, 4);                                                             
	mt_i2c_write(0x0992, 0x10C4, 4);                                                             
	mt_i2c_write(0x0994, 0xF602, 4);                                                             
	mt_i2c_write(0x0996, 0x5686, 4);                                                             
	mt_i2c_write(0x0998, 0x163D, 4);                                                             
	mt_i2c_write(0x099A, 0xC302, 4);                                                             
	mt_i2c_write(0x099C, 0x618F, 4);                                                             
	mt_i2c_write(0x099E, 0xEC14, 4);                                                             
	mt_i2c_write(0x098C, 0x0545, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xFD10, 4);                                                             
	mt_i2c_write(0x0992, 0xC501, 4);                                                             
	mt_i2c_write(0x0994, 0x0101, 4);                                                             
	mt_i2c_write(0x0996, 0x0101, 4);                                                             
	mt_i2c_write(0x0998, 0xFC10, 4);                                                             
	mt_i2c_write(0x099A, 0xC2DD, 4);                                                             
	mt_i2c_write(0x099C, 0x7FFC, 4);                                                             
	mt_i2c_write(0x099E, 0x10C7, 4);                                                             
	mt_i2c_write(0x098C, 0x0555, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xDD76, 4);                                                             
	mt_i2c_write(0x0992, 0xF602, 4);                                                             
	mt_i2c_write(0x0994, 0x5686, 4);                                                             
	mt_i2c_write(0x0996, 0x163D, 4);                                                             
	mt_i2c_write(0x0998, 0xC302, 4);                                                             
	mt_i2c_write(0x099A, 0x618F, 4);                                                             
	mt_i2c_write(0x099C, 0xEC14, 4);                                                             
	mt_i2c_write(0x099E, 0x939F, 4);                                                             
	mt_i2c_write(0x098C, 0x0565, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x30ED, 4);                                                             
	mt_i2c_write(0x0992, 0x08DC, 4);                                                             
	mt_i2c_write(0x0994, 0x7693, 4);                                                             
	mt_i2c_write(0x0996, 0x9D25, 4);                                                             
	mt_i2c_write(0x0998, 0x08F6, 4);                                                             
	mt_i2c_write(0x099A, 0x02BC, 4);                                                             
	mt_i2c_write(0x099C, 0x4F93, 4);                                                             
	mt_i2c_write(0x099E, 0x7F23, 4);                                                             
	mt_i2c_write(0x098C, 0x0575, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x3DF6, 4);                                                             
	mt_i2c_write(0x0992, 0x02BC, 4);                                                             
	mt_i2c_write(0x0994, 0x4F93, 4);                                                             
	mt_i2c_write(0x0996, 0x7F23, 4);                                                             
	mt_i2c_write(0x0998, 0x06F6, 4);                                                             
	mt_i2c_write(0x099A, 0x02BC, 4);                                                             
	mt_i2c_write(0x099C, 0x4FDD, 4);                                                             
	mt_i2c_write(0x099E, 0x7FDC, 4);                                                             
	mt_i2c_write(0x098C, 0x0585, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x9DDD, 4);                                                             
	mt_i2c_write(0x0992, 0x76F6, 4);                                                             
	mt_i2c_write(0x0994, 0x02BC, 4);                                                             
	mt_i2c_write(0x0996, 0x4F93, 4);                                                             
	mt_i2c_write(0x0998, 0x7F26, 4);                                                             
	mt_i2c_write(0x099A, 0x0FE6, 4);                                                             
	mt_i2c_write(0x099C, 0x0AC1, 4);                                                             
	mt_i2c_write(0x099E, 0x0226, 4);                                                             
	mt_i2c_write(0x098C, 0x0595, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x09D6, 4);                                                             
	mt_i2c_write(0x0992, 0x85C1, 4);                                                             
	mt_i2c_write(0x0994, 0x8026, 4);                                                             
	mt_i2c_write(0x0996, 0x0314, 4);                                                             
	mt_i2c_write(0x0998, 0x7401, 4);                                                             
	mt_i2c_write(0x099A, 0xF602, 4);                                                             
	mt_i2c_write(0x099C, 0xBC4F, 4);                                                             
	mt_i2c_write(0x099E, 0x937F, 4);                                                             
	mt_i2c_write(0x098C, 0x05A5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x2416, 4);                                                             
	mt_i2c_write(0x0992, 0xDE7F, 4);                                                             
	mt_i2c_write(0x0994, 0x09DF, 4);                                                             
	mt_i2c_write(0x0996, 0x7F30, 4);                                                             
	mt_i2c_write(0x0998, 0xEC08, 4);                                                             
	mt_i2c_write(0x099A, 0xDD76, 4);                                                             
	mt_i2c_write(0x099C, 0x200A, 4);                                                             
	mt_i2c_write(0x099E, 0xDC76, 4);                                                             
	mt_i2c_write(0x098C, 0x05B5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xA308, 4);                                                             
	mt_i2c_write(0x0992, 0x2304, 4);                                                             
	mt_i2c_write(0x0994, 0xEC08, 4);                                                             
	mt_i2c_write(0x0996, 0xDD76, 4);                                                             
	mt_i2c_write(0x0998, 0x1274, 4);                                                             
	mt_i2c_write(0x099A, 0x0122, 4);                                                             
	mt_i2c_write(0x099C, 0xDE5D, 4);                                                             
	mt_i2c_write(0x099E, 0xEE14, 4);                                                             
	mt_i2c_write(0x098C, 0x05C5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xAD00, 4);                                                             
	mt_i2c_write(0x0992, 0x30ED, 4);                                                             
	mt_i2c_write(0x0994, 0x11EC, 4);                                                             
	mt_i2c_write(0x0996, 0x06ED, 4);                                                             
	mt_i2c_write(0x0998, 0x02CC, 4);                                                             
	mt_i2c_write(0x099A, 0x0080, 4);                                                             
	mt_i2c_write(0x099C, 0xED00, 4);                                                             
	mt_i2c_write(0x099E, 0x8F30, 4);                                                             
	mt_i2c_write(0x098C, 0x05D5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xED04, 4);                                                             
	mt_i2c_write(0x0992, 0xEC11, 4);                                                             
	mt_i2c_write(0x0994, 0xEE04, 4);                                                             
	mt_i2c_write(0x0996, 0xBDA4, 4);                                                             
	mt_i2c_write(0x0998, 0x0630, 4);                                                             
	mt_i2c_write(0x099A, 0xE603, 4);                                                             
	mt_i2c_write(0x099C, 0xD785, 4);                                                             
	mt_i2c_write(0x099E, 0x30C6, 4);                                                             
	mt_i2c_write(0x098C, 0x05E5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x0B3A, 4);                                                             
	mt_i2c_write(0x0992, 0x3539, 4);                                                             
	mt_i2c_write(0x0994, 0x3C3C, 4);                                                             
	mt_i2c_write(0x0996, 0x3C34, 4);                                                             
	mt_i2c_write(0x0998, 0xCC32, 4);                                                             
	mt_i2c_write(0x099A, 0x3EBD, 4);                                                             
	mt_i2c_write(0x099C, 0xA558, 4);                                                             
	mt_i2c_write(0x099E, 0x30ED, 4);                                                             
	mt_i2c_write(0x098C, 0x05F5, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x04BD, 4);                                                             
	mt_i2c_write(0x0992, 0xB2D7, 4);                                                             
	mt_i2c_write(0x0994, 0x30E7, 4);                                                             
	mt_i2c_write(0x0996, 0x06CC, 4);                                                             
	mt_i2c_write(0x0998, 0x323E, 4);                                                             
	mt_i2c_write(0x099A, 0xED00, 4);                                                             
	mt_i2c_write(0x099C, 0xEC04, 4);                                                             
	mt_i2c_write(0x099E, 0xBDA5, 4);                                                             
	mt_i2c_write(0x098C, 0x0605, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x44CC, 4);                                                             
	mt_i2c_write(0x0992, 0x3244, 4);                                                             
	mt_i2c_write(0x0994, 0xBDA5, 4);                                                             
	mt_i2c_write(0x0996, 0x585F, 4);                                                             
	mt_i2c_write(0x0998, 0x30ED, 4);                                                             
	mt_i2c_write(0x099A, 0x02CC, 4);                                                             
	mt_i2c_write(0x099C, 0x3244, 4);                                                             
	mt_i2c_write(0x099E, 0xED00, 4);                                                             
	mt_i2c_write(0x098C, 0x0615, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xF601, 4);                                                             
	mt_i2c_write(0x0992, 0xD54F, 4);                                                             
	mt_i2c_write(0x0994, 0xEA03, 4);                                                             
	mt_i2c_write(0x0996, 0xAA02, 4);                                                             
	mt_i2c_write(0x0998, 0xBDA5, 4);                                                             
	mt_i2c_write(0x099A, 0x4430, 4);                                                             
	mt_i2c_write(0x099C, 0xE606, 4);                                                             
	mt_i2c_write(0x099E, 0x3838, 4);                                                             
	mt_i2c_write(0x098C, 0x0625, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x3831, 4);                                                             
	mt_i2c_write(0x0992, 0x39BD, 4);                                                             
	mt_i2c_write(0x0994, 0xD661, 4);                                                             
	mt_i2c_write(0x0996, 0xF602, 4);                                                             
	mt_i2c_write(0x0998, 0xF4C1, 4);                                                             
	mt_i2c_write(0x099A, 0x0126, 4);                                                             
	mt_i2c_write(0x099C, 0x0BFE, 4);                                                             
	mt_i2c_write(0x099E, 0x02BD, 4);                                                             
	mt_i2c_write(0x098C, 0x0635, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0xEE10, 4);                                                             
	mt_i2c_write(0x0992, 0xFC02, 4);                                                             
	mt_i2c_write(0x0994, 0xF5AD, 4);                                                             
	mt_i2c_write(0x0996, 0x0039, 4);                                                             
	mt_i2c_write(0x0998, 0xF602, 4);                                                             
	mt_i2c_write(0x099A, 0xF4C1, 4);                                                             
	mt_i2c_write(0x099C, 0x0226, 4);                                                             
	mt_i2c_write(0x099E, 0x0AFE, 4);                                                             
	mt_i2c_write(0x098C, 0x0645, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x02BD, 4);                                                             
	mt_i2c_write(0x0992, 0xEE10, 4);                                                             
	mt_i2c_write(0x0994, 0xFC02, 4);                                                             
	mt_i2c_write(0x0996, 0xF7AD, 4);                                                             
	mt_i2c_write(0x0998, 0x0039, 4);                                                             
	mt_i2c_write(0x099A, 0x3CBD, 4);                                                             
	mt_i2c_write(0x099C, 0xB059, 4);                                                             
	mt_i2c_write(0x099E, 0xCC00, 4);                                                             
	mt_i2c_write(0x098C, 0x0655, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x28BD, 4);                                                             
	mt_i2c_write(0x0992, 0xA558, 4);                                                             
	mt_i2c_write(0x0994, 0x8300, 4);                                                             
	mt_i2c_write(0x0996, 0x0027, 4);                                                             
	mt_i2c_write(0x0998, 0x0BCC, 4);                                                             
	mt_i2c_write(0x099A, 0x0026, 4);                                                             
	mt_i2c_write(0x099C, 0x30ED, 4);                                                             
	mt_i2c_write(0x099E, 0x00C6, 4);                                                             
	mt_i2c_write(0x098C, 0x0665, 4); 	// MCU_ADDRESS                                            
	mt_i2c_write(0x0990, 0x03BD, 4);                                                             
	mt_i2c_write(0x0992, 0xA544, 4);                                                             
	mt_i2c_write(0x0994, 0x3839, 4);                                                             
	mt_i2c_write(0x0996, 0xBDD9, 4);                                                             
	mt_i2c_write(0x0998, 0x42D6, 4);                                                             
	mt_i2c_write(0x099A, 0x9ACB, 4);                                                             
	mt_i2c_write(0x099C, 0x01D7, 4);                                                             
	mt_i2c_write(0x099E, 0x9B39, 4);                                                             
	mt_i2c_write(0x098C, 0x2006, 4); 	// MCU_ADDRESS [MON_ARG1]                                 
	mt_i2c_write(0x0990, 0x0415, 4); 	// MCU_DATA_0                                             
	mt_i2c_write(0x098C, 0xA005, 4); 	// MCU_ADDRESS [MON_CMD]                                  
	mt_i2c_write(0x0990, 0x0001, 4); 	// MCU_DATA_0  
	msleep(100);            
	
	//-----------------------------------------------------
	mt_i2c_write(0x001E, 0x0477, 4);		// pad slew
	//for mode a/b output format
	mt_i2c_write(0x098C, 0x2755, 4);		// mode_output_format_a
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence		
	mt_i2c_write(0x098C, 0x2757, 4);		// mode_output_format_b 
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence
	//----------------------------------------------------
	                        
	//  POLL  MON_PATCH_ID_0 =>  0x01                                               
	mt_i2c_write(0x0018, 0x0028, 4); 	// STANDBY_CONTROL    
	msleep(100);                                    
	//  POLL  SEQ_STATE =>  0x03                                                    
	mt_i2c_write(0x098C, 0xA103, 4); 	// MCU_ADDRESS [SEQ_CMD]                                  
	mt_i2c_write(0x0990, 0x0006, 4); 	// MCU_DATA_0          
	
	#endif 
	
	}
	else
	{	
		mt_i2c_write(0x098C, 0xA115, 4); 
		mt_i2c_write(0x0990, 0x0000, 4); 
		mt_i2c_write(0x098C, 0xA103, 4); 
		mt_i2c_write(0x0990, 0x0001, 4); 
		msleep(100);
	}
	return 0; 
}

IM_INT32 mt9d115_demo_switch_vga(void){ 
	return 0; 
}


IM_INT32 mt9d115_demo_switch_720p(void)
{
	In720pMode = IM_TRUE;

	//Reset 
  	mt_i2c_write(0x001A, 0x0051, 4);
  	msleep(50);  //10
  	mt_i2c_write(0x001A, 0x0050, 4);
  	msleep(50);  //10

  //[PLL]
	mt_i2c_write(0x0014, 0x2545 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0010, 0x0010 , 4);		// PLL_DIVIDERS
	mt_i2c_write(0x0012, 0x1FF7 , 4);		// PLL_P_DIVIDERS
	mt_i2c_write(0x0014, 0x2547 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0014, 0x2447 , 4);		// PLL_CONTROL
	msleep(10);
	mt_i2c_write(0x0014, 0x2047 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0014, 0x2046 , 4);		// PLL_CONTROL
	mt_i2c_write(0x0018, 0x4028 , 4);		// STANDBY_CONTROL
  	msleep(30);
	
	//[Timing]
	mt_i2c_write(0x321C, 0x0003 , 4);		// OFIFO_CONTROL_STATUS
	mt_i2c_write(0x098C, 0x2703 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0500 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2705 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02D0 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2707 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0640 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2709 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04B0 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x270D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x270F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2711 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02DB , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2713 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x050B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2715 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0111 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2717 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0024 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2719 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x003A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x00F6 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x008B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x271F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x032D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2721 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x074C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2723 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2725 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2727 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04BB , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2729 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x064B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0111 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0024 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x272F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x003A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2731 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x00F6 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2733 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x008B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2735 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x050D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2737 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x080E , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2739 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04FF , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x273F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02CF , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2747 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2749 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x063F , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x274B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x274D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04AF , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x222D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x006B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA408 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0019 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA409 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001F , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40B , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0021 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2411 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x006B , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2413 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0080 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2415 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0061 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2417 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0074 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA404 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0010 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA40E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0003 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA410 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000A , 4);		// MCU_DATA_0

//[AE]
	
	mt_i2c_write(0x098C, 0xA117 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA11D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA129 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA24F , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0014 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA216 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0091 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0091 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2212 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x007F , 4);		// MCU_DATA_0

	mt_i2c_write(0x3210, 0x01B0 , 4);		// COLOR_PIPELINE_CONTROL
	mt_i2c_write(0x364E, 0x0410 , 4);		// P_GR_P0Q0
	mt_i2c_write(0x3650, 0x65EB , 4);		// P_GR_P0Q1
	mt_i2c_write(0x3652, 0x01F2 , 4);		// P_GR_P0Q2
	mt_i2c_write(0x3654, 0x982F , 4);		// P_GR_P0Q3
	mt_i2c_write(0x3656, 0xB773 , 4);		// P_GR_P0Q4
	mt_i2c_write(0x3658, 0x7F4F , 4);		// P_RD_P0Q0
	mt_i2c_write(0x365A, 0x994B , 4);		// P_RD_P0Q1
	mt_i2c_write(0x365C, 0x0D12 , 4);		// P_RD_P0Q2
	mt_i2c_write(0x365E, 0x7BEA , 4);		// P_RD_P0Q3
	mt_i2c_write(0x3660, 0x9193 , 4);		// P_RD_P0Q4
	mt_i2c_write(0x3662, 0x0030 , 4);		// P_BL_P0Q0
	mt_i2c_write(0x3664, 0xB62B , 4);		// P_BL_P0Q1
	mt_i2c_write(0x3666, 0x4A31 , 4);		// P_BL_P0Q2
	mt_i2c_write(0x3668, 0x6AAE , 4);		// P_BL_P0Q3
	mt_i2c_write(0x366A, 0x83D3 , 4);		// P_BL_P0Q4
	mt_i2c_write(0x366C, 0x7F6F , 4);		// P_GB_P0Q0
	mt_i2c_write(0x366E, 0x282B , 4);		// P_GB_P0Q1
	mt_i2c_write(0x3670, 0x0332 , 4);		// P_GB_P0Q2
	mt_i2c_write(0x3672, 0x998F , 4);		// P_GB_P0Q3
	mt_i2c_write(0x3674, 0xBB93 , 4);		// P_GB_P0Q4
	mt_i2c_write(0x3676, 0x738C , 4);		// P_GR_P1Q0
	mt_i2c_write(0x3678, 0xF96F , 4);		// P_GR_P1Q1
	mt_i2c_write(0x367A, 0x84B0 , 4);		// P_GR_P1Q2
	mt_i2c_write(0x367C, 0x0952 , 4);		// P_GR_P1Q3
	mt_i2c_write(0x367E, 0x39B1 , 4);		// P_GR_P1Q4
	mt_i2c_write(0x3680, 0x094D , 4);		// P_RD_P1Q0
	mt_i2c_write(0x3682, 0x218F , 4);		// P_RD_P1Q1
	mt_i2c_write(0x3684, 0x6A8F , 4);		// P_RD_P1Q2
	mt_i2c_write(0x3686, 0xBAB1 , 4);		// P_RD_P1Q3
	mt_i2c_write(0x3688, 0xE951 , 4);		// P_RD_P1Q4
	mt_i2c_write(0x368A, 0x8DCC , 4);		// P_BL_P1Q0
	mt_i2c_write(0x368C, 0x862F , 4);		// P_BL_P1Q1
	mt_i2c_write(0x368E, 0x4E0E , 4);		// P_BL_P1Q2
	mt_i2c_write(0x3690, 0x3D51 , 4);		// P_BL_P1Q3
	mt_i2c_write(0x3692, 0x8F4F , 4);		// P_BL_P1Q4
	mt_i2c_write(0x3694, 0x038B , 4);		// P_GB_P1Q0
	mt_i2c_write(0x3696, 0x442F , 4);		// P_GB_P1Q1
	mt_i2c_write(0x3698, 0x8270 , 4);		// P_GB_P1Q2
	mt_i2c_write(0x369A, 0x9911 , 4);		// P_GB_P1Q3
	mt_i2c_write(0x369C, 0x21AF , 4);		// P_GB_P1Q4
	mt_i2c_write(0x369E, 0x5BD2 , 4);		// P_GR_P2Q0
	mt_i2c_write(0x36A0, 0x9C70 , 4);		// P_GR_P2Q1
	mt_i2c_write(0x36A2, 0x9D34 , 4);		// P_GR_P2Q2
	mt_i2c_write(0x36A4, 0xDB8A , 4);		// P_GR_P2Q3
	mt_i2c_write(0x36A6, 0xC075 , 4);		// P_GR_P2Q4
	mt_i2c_write(0x36A8, 0x5652 , 4);		// P_RD_P2Q0
	mt_i2c_write(0x36AA, 0x44B0 , 4);		// P_RD_P2Q1
	mt_i2c_write(0x36AC, 0xF272 , 4);		// P_RD_P2Q2
	mt_i2c_write(0x36AE, 0xC373 , 4);		// P_RD_P2Q3
	mt_i2c_write(0x36B0, 0xD276 , 4);		// P_RD_P2Q4
	mt_i2c_write(0x36B2, 0x2692 , 4);		// P_BL_P2Q0
	mt_i2c_write(0x36B4, 0x25D0 , 4);		// P_BL_P2Q1
	mt_i2c_write(0x36B6, 0xA533 , 4);		// P_BL_P2Q2
	mt_i2c_write(0x36B8, 0x8793 , 4);		// P_BL_P2Q3
	mt_i2c_write(0x36BA, 0x8676 , 4);		// P_BL_P2Q4
	mt_i2c_write(0x36BC, 0x5F12 , 4);		// P_GB_P2Q0
	mt_i2c_write(0x36BE, 0x9310 , 4);		// P_GB_P2Q1
	mt_i2c_write(0x36C0, 0xF253 , 4);		// P_GB_P2Q2
	mt_i2c_write(0x36C2, 0x2910 , 4);		// P_GB_P2Q3
	mt_i2c_write(0x36C4, 0xA316 , 4);		// P_GB_P2Q4
	mt_i2c_write(0x36C6, 0xFDAE , 4);		// P_GR_P3Q0
	mt_i2c_write(0x36C8, 0x4F51 , 4);		// P_GR_P3Q1
	mt_i2c_write(0x36CA, 0x6FD3 , 4);		// P_GR_P3Q2
	mt_i2c_write(0x36CC, 0x6910 , 4);		// P_GR_P3Q3
	mt_i2c_write(0x36CE, 0xA076 , 4);		// P_GR_P3Q4
	mt_i2c_write(0x36D0, 0x90ED , 4);		// P_RD_P3Q0
	mt_i2c_write(0x36D2, 0xB9EF , 4);		// P_RD_P3Q1
	mt_i2c_write(0x36D4, 0xC573 , 4);		// P_RD_P3Q2
	mt_i2c_write(0x36D6, 0x9933 , 4);		// P_RD_P3Q3
	mt_i2c_write(0x36D8, 0xE912 , 4);		// P_RD_P3Q4
	mt_i2c_write(0x36DA, 0x9470 , 4);		// P_BL_P3Q0
	mt_i2c_write(0x36DC, 0x4CAF , 4);		// P_BL_P3Q1
	mt_i2c_write(0x36DE, 0x31F0 , 4);		// P_BL_P3Q2
	mt_i2c_write(0x36E0, 0xB951 , 4);		// P_BL_P3Q3
	mt_i2c_write(0x36E2, 0x8E96 , 4);		// P_BL_P3Q4
	mt_i2c_write(0x36E4, 0x140D , 4);		// P_GB_P3Q0
	mt_i2c_write(0x36E6, 0x9ACF , 4);		// P_GB_P3Q1
	mt_i2c_write(0x36E8, 0x0DB2 , 4);		// P_GB_P3Q2
	mt_i2c_write(0x36EA, 0x9B33 , 4);		// P_GB_P3Q3
	mt_i2c_write(0x36EC, 0xD7F3 , 4);		// P_GB_P3Q4
	mt_i2c_write(0x36EE, 0xC034 , 4);		// P_GR_P4Q0
	mt_i2c_write(0x36F0, 0xB710 , 4);		// P_GR_P4Q1
	mt_i2c_write(0x36F2, 0xE0F7 , 4);		// P_GR_P4Q2
	mt_i2c_write(0x36F4, 0x64B4 , 4);		// P_GR_P4Q3
	mt_i2c_write(0x36F6, 0x143B , 4);		// P_GR_P4Q4
	mt_i2c_write(0x36F8, 0xDE93 , 4);		// P_RD_P4Q0
	mt_i2c_write(0x36FA, 0xA9B2 , 4);		// P_RD_P4Q1
	mt_i2c_write(0x36FC, 0xAC58 , 4);		// P_RD_P4Q2
	mt_i2c_write(0x36FE, 0x7A75 , 4);		// P_RD_P4Q3
	mt_i2c_write(0x3700, 0x19FB , 4);		// P_RD_P4Q4
	mt_i2c_write(0x3702, 0xB513 , 4);		// P_BL_P4Q0
	mt_i2c_write(0x3704, 0x5E51 , 4);		// P_BL_P4Q1
	mt_i2c_write(0x3706, 0x9738 , 4);		// P_BL_P4Q2
	mt_i2c_write(0x3708, 0xBA14 , 4);		// P_BL_P4Q3
	mt_i2c_write(0x370A, 0x1DBB , 4);		// P_BL_P4Q4
	mt_i2c_write(0x370C, 0xBF74 , 4);		// P_GB_P4Q0
	mt_i2c_write(0x370E, 0x31B1 , 4);		// P_GB_P4Q1
	mt_i2c_write(0x3710, 0x95B8 , 4);		// P_GB_P4Q2
	mt_i2c_write(0x3712, 0xA9D4 , 4);		// P_GB_P4Q3
	mt_i2c_write(0x3714, 0x31DB , 4);		// P_GB_P4Q4
	mt_i2c_write(0x3644, 0x0320 , 4);		// POLY_ORIGIN_C
	mt_i2c_write(0x3642, 0x0258 , 4);		// POLY_ORIGIN_R
	mt_i2c_write(0x3210, 0x01B8 , 4);		// COLOR_PIPELINE_CONTROL
	mt_i2c_write(0x098C, 0x2306 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x01D6 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2308 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF89 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFA1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF73 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x230E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x019C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2310 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2312 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFB0 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2314 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF2D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2316 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0223 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x001E , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0022 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x002C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2318 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0024 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0038 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFCD , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x231E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0023 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2320 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0010 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2322 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0026 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2324 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFE9 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2326 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF1 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2328 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x003A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x005D , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFF69 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFE4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000C , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x000A , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0006 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFFC , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x232E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0004 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x2330 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xFFF4 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA36D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA36E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0000 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0020 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA206 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0x0415 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xF601 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x42C1 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x0326 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x11F6 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0143 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xC104 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x260A , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC04 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0425 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x33BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA362 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xBD04 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x3339 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xC6FF , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xF701 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x6439 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xFE01 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0435 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x6918 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xCE03 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x25CC , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0013 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xBDC2 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xB8CC , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0489 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xFD03 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0445 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x27CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x0325 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xFD01 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x69FE , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x02BD , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x18CE , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0339 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0455 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x11BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xC2B8 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xCC04 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xC8FD , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0347 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xCC03 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x39FD , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x02BD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0465 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xDE00 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x18CE , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x00C2 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xCC00 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x37BD , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xC2B8 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xCC04 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xEFDD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0475 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xE6CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x00C2 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xDD00 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xC601 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF701 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x64C6 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x03F7 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x0165 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0485 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x7F01 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x6639 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x3C3C , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x3C34 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xCC32 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x3EBD , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xA558 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x30ED , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0495 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x04BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xB2D7 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x30E7 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x06CC , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x323E , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xED00 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xEC04 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xBDA5 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04A5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x44CC , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x3244 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xBDA5 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x585F , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x30ED , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x02CC , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x3244 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xED00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04B5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xF601 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xD54F , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xEA03 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xAA02 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xBDA5 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x4430 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xE606 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x3838 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04C5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x3831 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0x39BD , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xD661 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xF602 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF4C1 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x0126 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0BFE , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x02BD , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04D5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0xEE10 , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xFC02 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xF5AD , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0039 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0xF602 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0xF4C1 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x0226 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x0AFE , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04E5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x02BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xEE10 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0xFC02 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0xF7AD , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0039 , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x3CBD , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0xB059 , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0xCC00 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x04F5 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x28BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA558 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x8300 , 4);		// MCU_DATA_2
	mt_i2c_write(0x0996, 0x0027 , 4);		// MCU_DATA_3
	mt_i2c_write(0x0998, 0x0BCC , 4);		// MCU_DATA_4
	mt_i2c_write(0x099A, 0x0026 , 4);		// MCU_DATA_5
	mt_i2c_write(0x099C, 0x30ED , 4);		// MCU_DATA_6
	mt_i2c_write(0x099E, 0x00C6 , 4);		// MCU_DATA_7
	mt_i2c_write(0x098C, 0x0505 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x03BD , 4);		// MCU_DATA_0
	mt_i2c_write(0x0992, 0xA544 , 4);		// MCU_DATA_1
	mt_i2c_write(0x0994, 0x3839 , 4);		// MCU_DATA_2
	mt_i2c_write(0x098C, 0x2006 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0415 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA005 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xAB20 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0060 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA20D , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0020 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA206 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0048 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA11E , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0002 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA108 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA10A , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	mt_i2c_write(0x098C, 0xA10C , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0001 , 4);		// MCU_DATA_0
	//-----------------------------------------------------
	mt_i2c_write(0x001E, 0x0477, 4);		// slew
	
	//for mode a/b output format
	mt_i2c_write(0x098C, 0x2755, 4);		// mode_output_format_a
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence		
	mt_i2c_write(0x098C, 0x2757, 4);		// mode_output_format_b 
	mt_i2c_write(0x0990, 0x0002, 4);		// for ycbcr sequence
	//----------------------------------------------------

	mt_i2c_write(0x0018, 0x0028 , 4);		// STANDBY_CONTROL
  	msleep(200);
	mt_i2c_write(0x098C, 0xA103 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0006 , 4);		// MCU_DATA_0
  	msleep(100);
	mt_i2c_write(0x098C, 0xA103 , 4);		// MCU_ADDRESS
	mt_i2c_write(0x0990, 0x0005 , 4);		// MCU_DATA_0
  	msleep(100);	
	return 0;
}


static IM_INT32 cam_mt9d115_write_init_parameter(void)
{				
	IM_INT32 ret;                    
	In720pMode = IM_FALSE;
	SetInitParameter = IM_TRUE;

	//Reset 
  	//mt_i2c_write(0x001A, 0x0051, 4);
  	//msleep(50);  //10
  	//mt_i2c_write(0x001A, 0x0050, 4);
  	//msleep(50);  //10

	mt9d115_demo_switch_svga();

	SetInitParameter = IM_FALSE;
	return ret;
}


//--------------------------------------



/*
 * Write a list of register settings;
 */
static IM_INT32 cam_mt9d115_write_array(struct mt9d115_regval_list *vals , IM_UINT32 size)
{
	IM_INT32 i,ret;
	if (size == 0)
		return -1;

	for(i = 0; i < size ; i++)
	{
		if(vals->reg_num[0] == 0xff && vals->reg_num[1] == 0xff) {
			msleep((vals->value[0]<<8) | (vals->value[1]&0xff));
		}	
		else
	       	{	
            		ret = cam_mt9d115_write(vals->reg_num, vals->value);
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






IM_INT32 mt9d115_demo_set_sepia(void){ 
	return 0; 
}

IM_INT32 mt9d115_demo_set_bluish(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_greenish(void){

	return 0; 
}

IM_INT32 mt9d115_demo_set_reddish(void){

	return 0; 
}

IM_INT32 mt9d115_demo_set_yellowish(void){

	return 0; 
}

IM_INT32 mt9d115_demo_set_bandw(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_negative(void){

	return 0; 
}

IM_INT32 mt9d115_demo_set_normal(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_auto(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_sunny(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_cloudy(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_office(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_home(void){ 

	return 0; 
}

IM_INT32 mt9d115_demo_set_effect(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gMt9d115.efType)
	{
		return 0;
	}

	switch(value)                            
	{                                        
		case CAM_SPECIAL_EFFECT_NONE:    
			mt9d115_demo_set_normal();   
			break;                   
		case CAM_SPECIAL_EFFECT_MONO:   
			mt9d115_demo_set_bandw();    
			break;                   
		case CAM_SPECIAL_EFFECT_NEGATIVE:  
			mt9d115_demo_set_negative(); 
			break;                   
		case CAM_SPECIAL_EFFECT_SOLARIZE:     
			mt9d115_demo_set_yellowish();
			break;                   
		case CAM_SPECIAL_EFFECT_PASTEL: 
			mt9d115_demo_set_reddish();  
			break;                   
		case CAM_SPECIAL_EFFECT_SEPIA:     
			mt9d115_demo_set_sepia();    
			break;                   
		case CAM_SPECIAL_EFFECT_POSTERIZE:   
			mt9d115_demo_set_bluish();   
			break;                   
		case CAM_SPECIAL_EFFECT_AQUA:         
			mt9d115_demo_set_greenish(); 
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

	gMt9d115.efType = value;

	return 0;
}

IM_INT32 mt9d115_demo_set_wb(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	
	if(value == gMt9d115.wbMode)
	{
		return 0;
	}

	switch(value)
	{
		case CAM_WB_MODE_AUTO:
			mt9d115_demo_set_auto();
			break;
		case CAM_WB_MODE_INCANDESCENT:
			mt9d115_demo_set_home();
			break;
		case CAM_WB_MODE_FLUORESCENT:
			mt9d115_demo_set_office();
			break;
		case CAM_WB_MODE_DAYLIGHT:
			mt9d115_demo_set_sunny();
			break;
		case CAM_WB_MODE_CLOUDY_DAYLIGHT:
			mt9d115_demo_set_cloudy();
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

	gMt9d115.wbMode = value;

	msleep(10);

	return 0;
}

IM_INT32 mt9d115_demo_set_night_mode(IM_BOOL enable)
{
	#if 0	
	IM_INT32 i, ret;
	IM_INFOMSG((IM_STR("%s(enable = %d)"), IM_STR(_IM_FUNC_), enable));
	if (enable) 		/* Night Mode */
	{
		for(i = 0; i < (sizeof(mt9d115_night_regs) / 4); i++)
		{
			ret = cam_mt9d115_write((IM_UINT8 *)(&mt9d115_night_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}
	else  				/* Normal Mode */
	{
		for(i = 0; i < (sizeof(mt9d115_sunset_regs) / 4); i++)
		{
			ret = cam_mt9d115_write((IM_UINT8 *)(&mt9d115_sunset_regs[i]));
			if(ret != 0)
			{
				IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
				return ret;
			}
		}
	}
	#endif 
	return 0;
}

IM_INT32 mt9d115_demo_set_scene_mode(IM_INT32 value)
{
	if(value == gMt9d115.sceneMode)
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

	gMt9d115.sceneMode = value;

	return mt9d115_demo_set_night_mode(nightModeEn);
}

IM_INT32 mt9d115_demo_set_exposure(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gMt9d115.expVal)
	{
		return 0;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case -3:							/* EV -1.5 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma ;
			break;
		case -2:							/* EV -1 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case -1:							/* EV -0.5 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case 0:								/* EV 0 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case 1:							/* EV +0.5 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case 2:							/* EV +1 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case 3:							/* EV +1.5 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		case 4:							/* EV +2 */
			//mt_i2c_write(0x098E, 0xC87A, 4); 	// LOGICAL_ADDRESS_ACCESS
			//mt_i2c_write(0xC87A, 0x1E, 3); 	// cam AET target average luma 
			break;
		default:
			return -1;
	}

	gMt9d115.expVal = value;

	msleep(20);

	return 0;
}

IM_INT32 mt9d115_demo_set_antibanding(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	if(value == gMt9d115.bandMode)
	{
		return 0;
	}

	switch (value)
	{
		case CAM_ANTIBANDING_MODE_OFF:
		case CAM_ANTIBANDING_MODE_AUTO:
		case CAM_ANTIBANDING_MODE_50HZ:
			//cam_mt9d115_write((IM_UINT8 *)(&mt9d115_50hz_regs));
			break;

		case CAM_ANTIBANDING_MODE_60HZ:
			//cam_mt9d115_write((IM_UINT8 *)(&mt9d115_60hz_regs));
			break;

		default:
			return -1;
	}

	gMt9d115.bandMode = value;

	return 0;
}

//------- for flashlight
IM_INT32 mt9d115_demo_flash_on(IM_INT32 on)
{
	IM_INFOMSG((IM_STR("%s(mt9d115 FLASH ON= %d)"), IM_STR(_IM_FUNC_), on));
    if(0 == gMt9d115.flashLight)
    {
        return 0;
    }
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gMt9d115.flashLight, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gMt9d115.flashLight, 0);

	camsenpwl_io_set_outdat(gMt9d115.flashLight, on);

	return 0;
}

IM_INT32 mt9d115_demo_set_flash_mode(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gMt9d115.flashMode)
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
	
	gMt9d115.flashMode = value;
	return 0;
}

IM_INT32 mt9d115_demo_set_light_on(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));
	if(value == gMt9d115.light)
	{
		return 0;
	}
	
	switch (value) {
	case 0:
        mt9d115_demo_flash_on(0);
		break;
	case 1:
        mt9d115_demo_flash_on(1);
		break;   
	default:
		return -1;
	}
	
	gMt9d115.light = value;
	return 0;
}


//======================================================
IM_RET mt9d115_demo_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(MT9D115_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

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

IM_RET mt9d115_demo_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET mt9d115_demo_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
	IM_INT32 i, ret;
	//IM_INT16 buf = 0;
	struct mt9d115_regval_list regs;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	gPwl = pwl;

	camsenpwl_memset((void*)&gMt9d115, 0x0, sizeof(gMt9d115));
	//init value
	gMt9d115.wbMode = CAM_WB_MODE_AUTO;
	gMt9d115.efType = CAM_SPECIAL_EFFECT_NONE;
	gMt9d115.expVal = 0;
	gMt9d115.bandMode = CAM_ANTIBANDING_MODE_OFF;
	gMt9d115.sceneMode = CAM_SCENE_MODE_AUTO;
   	gMt9d115.flashMode = CAM_FLASH_MODE_OFF;
	gMt9d115.light = 0;

	gMt9d115.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(MT9D115_PWDN= %d)"), IM_STR(_IM_FUNC_), gMt9d115.pwdn));
	gMt9d115.reset = camsenpwl_get_reset_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(MT9D115_reset= %d)"), IM_STR(_IM_FUNC_), gMt9d115.reset));

	gMt9d115.flashLight = camsenpwl_get_flash_light_padnum(gPwl);
	IM_INFOMSG((IM_STR("%s(GC2015_FLASHLIGHT= %d)"), IM_STR(_IM_FUNC_), gMt9d115.flashLight));	

	if (gMt9d115.flashLight != 0)
	{
		mt9d115_demo_flash_on(0);
	}

	//config io
	/*io_index138(XCAMD12)<-->mt9d115.MCLK, io_indexgMt9d115.reset(XCAMD16)<-->mt9d115.RESET,
	 * io_index136(XCAMD10)<-->mt9d115.PWDN, io_index<-->mt9d115.POWER*/

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
	
	//set RESET(io index=gMt9d115.reset) state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gMt9d115.reset, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gMt9d115.reset, 0);

	//set pwdn state(mode and dir)
	//set io mode =1(use as gpio mode)
	camsenpwl_io_set_mode(gMt9d115.pwdn, 1);
	//set io dir =0(output dir)
	camsenpwl_io_set_dir(gMt9d115.pwdn, 0);

	/******************************************************
	*POWER enable: supply power, has enable all the time
	******************************************************/
	//set io(index) outdata =1(power enable)
	//camsenpwl_io_set_outdat(, 1);
	//delay some time
	msleep(5);

	/******************************
	*reset sensor
	******************************/
	//set io(index=gMt9d115.reset) outdata = 1->0->1(reset need rising edge)
	//camsenpwl_io_set_outdat(gMt9d115.reset, 1);
	camsenpwl_io_set_outdat(gMt9d115.reset, 0);
	msleep(10);
	camsenpwl_io_set_outdat(gMt9d115.reset, 1);
	msleep(10);
	
	/******************************
	*power down disable
	******************************/
	//set io outdata = 1(pwdn also  need rising edge, high active to disable pwdn, set 0x21e09110 bit4 to 1)
	camsenpwl_io_set_outdat(gMt9d115.pwdn, 1);
	camsenpwl_io_set_outdat(gMt9d115.pwdn, 0);
	msleep(10);
	/******************************
	*provided mclk
	******************************/
	//set MCLK(io index=138) state(mode)
	//set io mode = 0(use as function mode)
	//camsenpwl_io_set_mode(138, 0);
	camsenpwl_clock_enable(gPwl, 12000000);//irequest MCLK 24MHz
	
	//check real clock freq if need
	//camsenpwl_clock_get_freq(gPwl);
	msleep(30);

//---------- reset 114	
	camsenpwl_io_set_mode(137, 1);
	camsenpwl_io_set_dir(137, 0);
	camsenpwl_io_set_outdat(137, 0);
	msleep(10);
//----------------------


	//read sensor id
	if(checkOnly == IM_TRUE)
	{

		regs.reg_num[0] = 0x00;
        	regs.reg_num[1] = 0x00;
        	regs.value[0] = 0xFF;
        	regs.value[1] = 0xFF;
        	ret = cam_mt9d115_read(regs.reg_num, regs.value);
		
		IM_INFOMSG((IM_STR("%s(ID0 = 0x%x, ID1=0x %x)"), IM_STR(_IM_FUNC_), regs.value[0],regs.value[1]));
        	if( (regs.value[0] != 0x25) || (regs.value[1] != 0x80) )
		{
			IM_ERRMSG((IM_STR("mt9d115 id error!")));
            		goto Fail;
		}
		return IM_RET_OK;
	}

//--------------------------
	ret = cam_mt9d115_write_init_parameter();
	if(ret != 0)
    	{
		IM_ERRMSG((IM_STR("Failed to write default registers value!")));
       		goto Fail;
    	}

//--------------------------
	/*
	ret = cam_mt9d115_write_array(mt9d115_softreset_regs,ARRAY_SIZE(mt9d115_softreset_regs));
	if(ret != 0)
    	{
        	IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        	goto Fail;
    	}

	msleep(50);

	ret = cam_mt9d115_write_array(mt9d115_init_regs,ARRAY_SIZE(mt9d115_init_regs));
	if(ret != 0)
    	{
        	IM_ERRMSG((IM_STR("Failed to write default registers value!")));
        	goto Fail;
    	}
	*/

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	//power down enable
	camsenpwl_io_set_outdat(gMt9d115.pwdn, 1);
	//reset sensor
	camsenpwl_io_set_outdat(gMt9d115.reset, 0);
	//close mclk
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET mt9d115_demo_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/******************************
	*power down enable
	******************************/
	camsenpwl_io_set_outdat(gMt9d115.pwdn, 1);

	/******************************
	*reset sensor
	******************************/
	camsenpwl_io_set_outdat(gMt9d115.reset, 0);

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

IM_RET mt9d115_demo_start(void)
{
	IM_INT32 ret;
	//struct mt9d115_regval_list  mt9d115_streaming = {0x01, 0x04, 0x03};
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	//ret = cam_mt9d115_write((IM_UINT8 *)(&mt9d115_streaming));
	//if(ret != 0)
	//{
	//	IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
	//	return IM_RET_FAILED;
	//}
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET mt9d115_demo_stop(void)
{
	IM_INT32 ret;
	//struct mt9d115_regval_list  mt9d115_streaming = {0x01, 0x04, 0x00};
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	//ret = cam_mt9d115_write((IM_UINT8 *)(&mt9d115_streaming));
	//if(ret != 0)
	//{
	//	IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
	//	return IM_RET_FAILED;
	//}
	return ret;
}

IM_RET mt9d115_demo_get_caps(camsen_caps_t *caps)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	caps->supportRes = CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_720P |CAM_RES_UXGA;
	caps->maxRes = CAM_RES_UXGA; 
	caps->initRes = CAM_RES_SVGA;

	return ret;
}

IM_RET mt9d115_demo_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET mt9d115_demo_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INT32 ret;
	IM_UINT32 res, fps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	res = outMode->res;
	fps = outMode->fps;

	if(res == CAM_RES_UXGA)
	{
		ret = mt9d115_demo_switch_uxga();

		//printk("115 set out mode, use uxga tttttTTTT \n\n\n\n");

		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9d115_demo_switch_uxga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_720P)
	{
		ret = mt9d115_demo_switch_720p();
		
		//printk("115 set out mode, use 720p tttttTTTT \n\n\n\n");
		
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9d115_demo_switch_720p() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_SVGA)
	{
		ret = mt9d115_demo_switch_svga();
		
		//printk("115 set out mode, use svga tttttTTTT \n\n\n\n");
		
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9d115_demo_switch_svga() failed!")));
			return IM_RET_FAILED;
		}
	}
	else if(res == CAM_RES_VGA)
	{
		ret = mt9d115_demo_switch_vga();
		
		//printk("115 set out mode, use vga tttttTTTT \n\n\n\n");
		
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9d115_demo_switch_vga() failed!")));
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

IM_RET mt9d115_demo_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

	if(property == CAM_KEY_RW_WB_MODE)
	{
		mt9d115_demo_set_wb(value);
	}
	else if(property == CAM_KEY_RW_SPECIAL_EFFECT)
	{
		mt9d115_demo_set_effect(value);
	}
	else if(property == CAM_KEY_RW_SCENE_MODE)
	{
		mt9d115_demo_set_scene_mode(value);
	}
	//---- for flashlight
	else if(property == CAM_KEY_RW_FLASH_MODE)
	{
		mt9d115_demo_set_flash_mode(value);
	}
	else if(property == CAM_KEY_RW_LIGHT_TURN_ON)
	{
		mt9d115_demo_set_light_on(value);
	}
	//-------------
	else if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
	{
		mt9d115_demo_set_exposure(value);
	}
	else if(property == CAM_KEY_RW_ANTIBANDING_MODE)
	{
		mt9d115_demo_set_antibanding(value);
	}

	return ret;
}

IM_RET mt9d115_demo_get_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	switch(property)
	{
		case CAM_KEY_R_CAPS:
			value = /*CAM_CAP_WB_MODE_SUPPORT |
				CAM_CAP_SPECIAL_EFFECT_SUPPORT |
				CAM_CAP_SCENE_MODE_SUPPORT |
				CAM_CAP_FLASH_MODE_SUPPORT |
				CAM_CAP_ANTIBANDING |
				CAM_CAP_EXPOSURE |*/
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
			memcpy(p, (void*)&gMt9d115.wbMode, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SPECIAL_EFFECT:
			value = CAM_SPECIAL_EFFECT_NONE |
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
			memcpy(p, (void*)&gMt9d115.efType, sizeof(IM_INT32));
			break;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			value = CAM_SCENE_MODE_AUTO |
				CAM_SCENE_MODE_NIGHT |
				CAM_SCENE_MODE_NIGHT_PORTRAIT |
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(p, (void*)&gMt9d115.sceneMode, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gMt9d115.flashMode, sizeof(IM_INT32));
			break; 
		case CAM_KEY_RW_LIGHT_TURN_ON: 
			memcpy(p, (void*)&gMt9d115.light, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gMt9d115.expVal, sizeof(IM_INT32));
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
			memcpy(p, (void*)&gMt9d115.bandMode, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops mt9d115_demo_ops = {
	.name                   = "mt9d115_demo",
	.i2c_dev_addr           = MT9D115_I2C_ADDR,

	.sen_pwdn				= mt9d115_demo_pwdn,
	.sen_get_pmu_info		= mt9d115_demo_get_pmu_info,
	.sen_init				= mt9d115_demo_init,
	.sen_deinit				= mt9d115_demo_deinit,
	.sen_start				= mt9d115_demo_start,
	.sen_stop				= mt9d115_demo_stop,
	.sen_get_caps			= mt9d115_demo_get_caps,
	.sen_set_out_mode		= mt9d115_demo_set_out_mode,
	.sen_get_out_mode		= mt9d115_demo_get_out_mode,
	.sen_set_property		= mt9d115_demo_set_property,
	.sen_get_property		= mt9d115_demo_get_property,
};
