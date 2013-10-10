/*------------------------------------------------------------------------------
-- 	Copyright {c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gc0329_demo.h
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

#define GC0329_I2C_ADDR                (0x62>>1)

static	struct gc0329_regval_list {
	IM_UINT8  reg;
	IM_UINT8  value;
};

static	struct gc0329_regval_list  gc0329_qvga_regs[] = {
		
};

static	struct gc0329_regval_list  gc0329_vga_regs[] = {
	//set in init 
};

static	struct gc0329_regval_list  gc0329_init_regs[] = {
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfc, 0x16},
	{0xfc, 0x16},
	{0xfc, 0x16},
	{0xfc, 0x16},
	{0xfe, 0x00},

	{0x73, 0x90},
	{0x74, 0x80}, 
	{0x75, 0x80}, 
	{0x76, 0x94}, 
	{0x42, 0x00}, 
	{0x77, 0x57}, 
	{0x78, 0x4d}, 
	{0x79, 0x45}, 
	//{0x42, 0xfc},
	//banding////
	{0x05,0x01},
	{0x06,0x32},  // hb
	{0x07,0x00},
	{0x08,0x70}, // vb
	{0xfe,0x01},
	{0x29,0x00},
	{0x2a,0x78},// step
	
	{0x2b,0x02},
	{0x2c,0x58},//lev 1
	{0x2d,0x03},
	{0x2e,0x48},
	{0x2f,0x05}, //fps
	{0x30,0xa0}, //fps
	{0x31,0x07},
	{0x32,0x08},
	{0x33,0x20},	
	{0xfe,0x00},
	////////////////////analog////////////////////
	{0x0a, 0x02}, 
	{0x0c, 0x02}, 
	{0x17, 0x14},
	{0x19, 0x05}, 
	{0x1b, 0x24}, 
	{0x1c, 0x04}, 
	{0x1e, 0x08}, 
	{0x1f, 0xc8},// c0 20120720 
	{0x20, 0x00}, 
	{0x21, 0x48}, 
	{0x22, 0xba}, 
	{0x23, 0x22},
	{0x24, 0x16},	   
	////////////////////blk////////////////////
	{0x26, 0xf7}, 
	{0x29, 0x80}, 
	{0x32, 0x04},
	{0x33, 0x20},
	{0x34, 0x20},
	{0x35, 0x20},
	{0x36, 0x20},
	////////////////////ISP BLOCK ENABL////////////////////
	{0x40, 0xff},
	{0x41, 0x44}, 
	{0x42, 0x7e}, 
	{0x44, 0xa2}, 
	{0x46, 0x03},
	{0x4b, 0xca},
	{0x4d, 0x01}, 
	{0x4f, 0x01},
	{0x70, 0x48}, 
	//{0xb0, 0x00},
	//{0xbc, 0x00},
	//{0xbd, 0x00},
	//{0xbe, 0x00},
	////////////////////DNDD////////////////////
	{0x80, 0xe7},
	{0x82, 0x55},
	{0x83, 0x03}, // 20120423
	{0x87, 0x4a},
	////////////////////INTPEE////////////////////
	{0x95, 0x34},//45	//edge effect
	////////////////////ASDE////////////////////
	//{0xfe, 0x01},
	//{0x18, 0x22}, 
	//{0xfe, 0x00},
	//{0x9c, 0x0a}, 
	//{0xa0, 0xaf}, 
	//{0xa2, 0xff},
	//{0xa4, 0x50},
	//{0xa5, 0x21},
	//{0xa7, 0x35},
	////////////////////RGB gamma////////////////////
	//RGB gamma m4'
	{0xbf, 0x06},
	{0xc0, 0x14},
	{0xc1, 0x27},
	{0xc2, 0x3b},
	{0xc3, 0x4f},
	{0xc4, 0x62},
	{0xc5, 0x72},
	{0xc6, 0x8d},
	{0xc7, 0xa4},
	{0xc8, 0xb8},
	{0xc9, 0xc9},
	{0xcA, 0xd6},
	{0xcB, 0xe0},
	{0xcC, 0xe8},
	{0xcD, 0xf4},
	{0xcE, 0xFc},
	{0xcF, 0xFF},
#if 0				 
//smallest gamma curve
	{0xfe, 0x00}, //case RGB_Gamma_m1
	{0xbf, 0x06},
	{0xc0, 0x12},
	{0xc1, 0x22},
	{0xc2, 0x35},
	{0xc3, 0x4b},
	{0xc4, 0x5f},
	{0xc5, 0x72},
	{0xc6, 0x8d},
	{0xc7, 0xa4},
	{0xc8, 0xb8},
	{0xc9, 0xc8},
	{0xca, 0xd4},
	{0xcb, 0xde},
	{0xcc, 0xe6},
	{0xcd, 0xf1},
	{0xce, 0xf8},
	{0xcf, 0xfd},
	

	{0xBF, 0x08},//case RGB_Gamma_m2
	{0xc0, 0x0F},
	{0xc1, 0x21},
	{0xc2, 0x32},
	{0xc3, 0x43},
	{0xc4, 0x50},
	{0xc5, 0x5E},
	{0xc6, 0x78},
	{0xc7, 0x90},
	{0xc8, 0xA6},
	{0xc9, 0xB9},
	{0xcA, 0xC9},
	{0xcB, 0xD6},
	{0xcC, 0xE0},
	{0xcD, 0xEE},
	{0xcE, 0xF8},
	{0xcF, 0xFF},		
		
	{0xBF, 0x0B},//case RGB_Gamma_m3
	{0xc0, 0x16},
	{0xc1, 0x29},
	{0xc2, 0x3C},
	{0xc3, 0x4F},
	{0xc4, 0x5F},
	{0xc5, 0x6F},
	{0xc6, 0x8A},
	{0xc7, 0x9F},
	{0xc8, 0xB4},
	{0xc9, 0xC6},
	{0xcA, 0xD3},
	{0xcB, 0xDD},
	{0xcC, 0xE5},
	{0xcD, 0xF1},
	{0xcE, 0xFA},
	{0xcF, 0xFF},			

	{0xBF, 0x0E},//case RGB_Gamma_m4
	{0xc0, 0x1C},
	{0xc1, 0x34},
	{0xc2, 0x48},
	{0xc3, 0x5A},
	{0xc4, 0x6B},
	{0xc5, 0x7B},
	{0xc6, 0x95},
	{0xc7, 0xAB},
	{0xc8, 0xBF},
	{0xc9, 0xCE},
	{0xcA, 0xD9},
	{0xcB, 0xE4},
	{0xcC, 0xEC},
	{0xcD, 0xF7},
	{0xcE, 0xFD},
	{0xcF, 0xFF},
	

	{0xBF, 0x10}, //case RGB_Gamma_m5
	{0xc0, 0x20},
	{0xc1, 0x38},
	{0xc2, 0x4E},
	{0xc3, 0x63},
	{0xc4, 0x76},
	{0xc5, 0x87},
	{0xc6, 0xA2},
	{0xc7, 0xB8},
	{0xc8, 0xCA},
	{0xc9, 0xD8},
	{0xcA, 0xE3},
	{0xcB, 0xEB},
	{0xcC, 0xF0},
	{0xcD, 0xF8},
	{0xcE, 0xFD},
	{0xcF, 0xFF},
	

	{0xBF, 0x14},	// largest gamma curve	 case RGB_Gamma_m6:		
	{0xc0, 0x28},
	{0xc1, 0x44},
	{0xc2, 0x5D},
	{0xc3, 0x72},
	{0xc4, 0x86},
	{0xc5, 0x95},
	{0xc6, 0xB1},
	{0xc7, 0xC6},
	{0xc8, 0xD5},
	{0xc9, 0xE1},
	{0xcA, 0xEA},
	{0xcB, 0xF1},
	{0xcC, 0xF5},
	{0xcD, 0xFB},
	{0xcE, 0xFE},
	{0xcF, 0xFF},	

	//Gamma for night mode
	{0xBF, 0x0B},//	case RGB_Gamma_night:	
	{0xc0, 0x16},
	{0xc1, 0x29},
	{0xc2, 0x3C},
	{0xc3, 0x4F},
	{0xc4, 0x5F},
	{0xc5, 0x6F},
	{0xc6, 0x8A},
	{0xc7, 0x9F},
	{0xc8, 0xB4},
	{0xc9, 0xC6},
	{0xcA, 0xD3},
	{0xcB, 0xDD},
	{0xcC, 0xE5},
	{0xcD, 0xF1},
	{0xcE, 0xFA},
	{0xcF, 0xFF},	


	//RGB_Gamma_m1
	{0xfe, 0x00},//default
	{0xbf, 0x06},
	{0xc0, 0x12},
	{0xc1, 0x22},
	{0xc2, 0x35},
	{0xc3, 0x4b},
	{0xc4, 0x5f},
	{0xc5, 0x72},
	{0xc6, 0x8d},
	{0xc7, 0xa4},
	{0xc8, 0xb8},
	{0xc9, 0xc8},
	{0xca, 0xd4},
	{0xcb, 0xde},
	{0xcc, 0xe6},
	{0xcd, 0xf1},
	{0xce, 0xf8},
	{0xcf, 0xfd},
#endif
	//////////////////CC///////////////////
	{0xfe, 0x00},
	{0xb3, 0x44},
	{0xb4, 0xfd},
	{0xb5, 0x02},
	{0xb6, 0xfa},
	{0xb7, 0x48},
	{0xb8, 0xf0},
	// crop 						   
	{0x50, 0x01},
	////////////////////YCP////////////////////
	{0xfe, 0x00},
	{0xd1, 0x38},
	{0xd2, 0x38},
	{0xdd, 0x54},
	////////////////////AEC////////////////////
	{0xfe, 0x01},
	{0x10, 0x40},
	{0x11, 0x21}, 
	{0x12, 0x07}, 
	{0x13, 0x50},
	{0x17, 0x88}, 
	{0x21, 0xb0},
	{0x22, 0x48},
	{0x3c, 0x95},
	{0x3d, 0x50},
	{0x3e, 0x48}, 
	////////////////////AWB////////////////////
	{0xfe, 0x01},
	{0x06, 0x16},
	{0x07, 0x06},
	{0x08, 0x98},
	{0x09, 0xee},
	{0x50, 0xfc}, 
	{0x51, 0x20},
	{0x52, 0x0b},
	{0x53, 0x20}, 
	{0x54, 0x40}, 
	{0x55, 0x10}, 
	{0x56, 0x20}, 
	//{0x57, 0x40},
	{0x58, 0xa0}, 
	{0x59, 0x28}, 
	{0x5a, 0x02}, 
	{0x5b, 0x63}, 
	{0x5c, 0x34},
	{0x5d, 0x73}, 
	{0x5e, 0x11}, 
	{0x5f, 0x40}, 
	{0x60, 0x40}, 
	{0x61, 0xc8}, 
	{0x62, 0xa0}, 
	{0x63, 0x40}, 
	{0x64, 0x50}, 
	{0x65, 0x98}, 
	{0x66, 0xfa}, 
	{0x67, 0x70}, 
	{0x68, 0x58}, 
	{0x69, 0x85}, 
	{0x6a, 0x40},
	{0x6b, 0x39},
	{0x6c, 0x18},
	{0x6d, 0x28},
	{0x6e, 0x41}, 
	{0x70, 0x02}, 
	{0x71, 0x00}, 
	{0x72, 0x10},
	{0x73, 0x40}, 
	{0x74, 0x40},
	{0x75, 0x58},
	{0x76, 0x24},
	{0x77, 0x40},
	{0x78, 0x20},
	{0x79, 0x60},
	{0x7a, 0x58},
	{0x7b, 0x20},
	{0x7c, 0x30},
	{0x7d, 0x35},
	{0x7e, 0x10},
	{0x7f, 0x08},
	{0x81, 0x50}, 
	{0x82, 0x42}, 
	{0x83, 0x40}, 
	{0x84, 0x40}, 
	{0x85, 0x40}, 
	
	////////////////////ABS////////////////////
	{0x9c, 0x02}, 
	{0x9d, 0x20},
	//{0x9f, 0x40},	
	////////////////////CC-AWB////////////////////
	{0xd0, 0x00},
	{0xd2, 0x2c},
	{0xd3, 0x80}, 
	////////////////////LSC///////////////////
	{0xfe, 0x01},
	{0xa0, 0x00},
	{0xa1, 0x3c},
	{0xa2, 0x50},
	{0xa3, 0x00},
	{0xa8, 0x0f},
	{0xa9, 0x08},
	{0xaa, 0x00},
	{0xab, 0x04},
	{0xac, 0x00},
	{0xad, 0x07},
	{0xae, 0x0e},
	{0xaf, 0x00},
	{0xb0, 0x00},
	{0xb1, 0x09},
	{0xb2, 0x00},
	{0xb3, 0x00},
	{0xb4, 0x31},
	{0xb5, 0x19},
	{0xb6, 0x24},
	{0xba, 0x3a},
	{0xbb, 0x24},
	{0xbc, 0x2a},
	{0xc0, 0x17},
	{0xc1, 0x13},
	{0xc2, 0x17},
	{0xc6, 0x21},
	{0xc7, 0x1c},
	{0xc8, 0x1c},
	{0xb7, 0x00},
	{0xb8, 0x00},
	{0xb9, 0x00},
	{0xbd, 0x00},
	{0xbe, 0x00},
	{0xbf, 0x00},
	{0xc3, 0x00},
	{0xc4, 0x00},
	{0xc5, 0x00},
	{0xc9, 0x00},
	{0xca, 0x00},
	{0xcb, 0x00},
	{0xa4, 0x00},
	{0xa5, 0x00},
	{0xa6, 0x00},
	{0xa7, 0x00},
	/////20120502////
	{0xfe,0x01},
	{0x18,0x22},
	{0x21,0xc0},
	{0x06,0x12},
	{0x08,0x9c},
	{0x51,0x28},
	{0x52,0x10},
	{0x53,0x20},
	{0x54,0x40},
	{0x55,0x16},
	{0x56,0x30},
	{0x58,0x60},
	{0x59,0x08},
	{0x5c,0x35},
	{0x5d,0x72},
	{0x67,0x80},
	{0x68,0x60},
	{0x69,0x90},
	{0x6c,0x30},
	{0x6d,0x60},
	{0x70,0x10},
	{0xfe,0x00},
	{0x9c,0x0a},
	{0xa0,0xaf},
	{0xa2,0xff},
	{0xa4,0x60},
	{0xa5,0x31},
	{0xa7,0x35},
	{0x42,0xfe},
	{0xd1,0x34},
	{0xd2,0x34},
	{0xfe,0x00},

	//-----ob
	{0xd3,0x3c},		//luma contrast
	//------------
	////////////////////asde ///////////////////
	//{0xa0, 0xaf},
	//{0xa2, 0xff},
	{0x44, 0xa2},
	{0xf0, 0x07},
	{0xf1, 0x01},
	{0xfe, 0x01},
	{0x9c, 0x00},		//ABS slope adjustment
        
	{0xfe, 0x00}, 
};

static	struct gc0329_regval_list  gc0329_sepia_regs[] = {
	{0x43 , 0x02},
	{0xda , 0xd0},
	{0xdb , 0x28},
};

static	struct gc0329_regval_list  gc0329_bluish_regs[] = {
	{0x43 , 0x02},
	{0xda , 0x50},
	{0xdb , 0xe0},
};

static	struct gc0329_regval_list  gc0329_greenish_regs[] = {
	{0x43 , 0x02},
	{0xda , 0xc0},
	{0xdb , 0xc0},
};

static	struct gc0329_regval_list  gc0329_reddish_regs[] = {
	
};

static	struct gc0329_regval_list  gc0329_yellowish_regs[] = {
	
};

static	struct gc0329_regval_list  gc0329_bandw_regs[] = {
	{0x43 , 0x02},
	{0xda , 0x00},
	{0xdb , 0x00},
};

static	struct gc0329_regval_list  gc0329_negative_regs[] = {
	{0x43 , 0x01},
};

static	struct gc0329_regval_list  gc0329_normal_regs[] = {
	{0x43 , 0x00},
};





static	struct gc0329_regval_list  gc0329_auto_regs[] = {
{0x77, 0x57},
{0x78, 0x4d},
{0x79, 0x45},
{0x42, 0xfe}, //AWB on
	
};

static	struct gc0329_regval_list  gc0329_sunny_regs[] = {
{0x42, 0xfc}, //AWB off
{0x77, 0x74}, 
{0x78, 0x52},
{0x79, 0x40},	
};

static	struct gc0329_regval_list  gc0329_cloudy_regs[] = {
{0x42, 0xfc}, //AWB off
{0x77, 0x8c}, //WB_manual_gain 
{0x78, 0x50},
{0x79, 0x40},	
};

static	struct gc0329_regval_list  gc0329_office_regs[] = {   //flourescent
{0x42, 0xfc}, //AWB off
{0x77, 0x40},
{0x78, 0x42},
{0x79, 0x50},
};

static	struct gc0329_regval_list  gc0329_home_regs[] = {	//incandescent
{0x42, 0xfc}, //AWB off
{0x77, 0x48},
{0x78, 0x40},
{0x79, 0x5c},
};

static	struct gc0329_regval_list  gc0329_night_50hz_regs[] = {
	
	//banding////
	{0x05,0x01},
	{0x06,0x32},  // hb
	{0x07,0x00},
	{0x08,0x70}, // vb
	{0xfe,0x01},
	{0x29,0x00},
	{0x2a,0x78},// step
	
	{0x2b,0x02},
	{0x2c,0x58},//lev 1
	{0x2d,0x03},
	{0x2e,0x48},
	{0x2f,0x05}, //fps
	{0x30,0xa0}, //fps
	{0x31,0x07},
	{0x32,0x08},
	{0x33,0x30},	
	{0xfe,0x00},

};

static	struct gc0329_regval_list  gc0329_night_60hz_regs[] = {
		//banding////
	{0x05,0x01},
	{0x06,0x32},  // hb
	{0x07,0x00},
	{0x08,0x70}, // vb
	{0xfe,0x01},
	{0x29,0x00},
	{0x2a,0x64},// step
	
	{0x2b,0x02},
	{0x2c,0x58},//lev 1
	{0x2d,0x03},
	{0x2e,0x48},
	{0x2f,0x04}, //fps
	{0x30,0xb0}, //fps
	{0x31,0x07},
	{0x32,0xd0},
	{0x33,0x30},	
	{0xfe,0x00},
};

static	struct gc0329_regval_list  gc0329_sunset_50hz_regs[] = {
		//banding////
	{0x05,0x01},
	{0x06,0x32},  // hb
	{0x07,0x00},
	{0x08,0x70}, // vb
	{0xfe,0x01},
	{0x29,0x00},
	{0x2a,0x78},// step
	
	{0x2b,0x02},
	{0x2c,0x58},//lev 1
	{0x2d,0x03},
	{0x2e,0x48},
	{0x2f,0x05}, //fps
	{0x30,0xa0}, //fps
	{0x31,0x07},
	{0x32,0x08},
	{0x33,0x20},	
	{0xfe,0x00},
};

static	struct gc0329_regval_list  gc0329_sunset_60hz_regs[] = {
			//banding////
	{0x05,0x01},
	{0x06,0x32},  // hb
	{0x07,0x00},
	{0x08,0x70}, // vb
	{0xfe,0x01},
	{0x29,0x00},
	{0x2a,0x64},// step
	
	{0x2b,0x02},
	{0x2c,0x58},//lev 1
	{0x2d,0x03},
	{0x2e,0x48},
	{0x2f,0x04}, //fps
	{0x30,0xb0}, //fps
	{0x31,0x07},
	{0x32,0xd0},
	{0x33,0x20},	
	{0xfe,0x00},
};


static	struct gc0329_regval_list  gc0329_50hz_regs[] = {
	
};


static	struct gc0329_regval_list  gc0329_60hz_regs[] = {
	
};

static	struct gc0329_regval_list  gc0329_ev_neg4_regs[] = {
{0xd5, 0xc0},//-4
{0xfe, 0x01},
{0x13, 0x30},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_neg3_regs[] = {
{0xd5, 0xd0},//-3
{0xfe, 0x01},
{0x13, 0x38},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_neg2_regs[] = {
{0xd5, 0xe0},//-2
{0xfe, 0x01},
{0x13, 0x40},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_neg1_regs[] = {
{0xd5, 0xf0},//-1
{0xfe, 0x01},
{0x13, 0x48},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_zero_regs[] = {
{0xd5, 0x00},  //0
{0xfe, 0x01},
{0x13, 0x50},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_pos1_regs[] = {
{0xd5, 0x10},//+1
{0xfe, 0x01},
{0x13, 0x60},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_pos2_regs[] = {
{0xd5, 0x20},//+2
{0xfe, 0x01},
{0x13, 0x70},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_pos3_regs[] = {
{0xd5, 0x30},//+3
{0xfe, 0x01},
{0x13, 0x80},
{0xfe, 0x00},
};

static	struct gc0329_regval_list  gc0329_ev_pos4_regs[] = {
{0xd5, 0x40},//+4
{0xfe, 0x01},
{0x13, 0x90},
{0xfe, 0x00},
};