/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: gc2035_demo_500w.h
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

#define gc2035_I2C_ADDR                (0x78 >> 1) 
//#define CHT_809_LENS_LSC
//#define CHT_809_LENS_AWB
//#define A_LIGHT_CORRECTION
//#define GC2035YUV_FastFrame  // 96M PCLK
//#define GC2035YUV_CAP    // 1/4 clk for capture
static struct gc2035_regval_list {
	IM_UINT8  reg;
	IM_UINT8  value;
};

static struct gc2035_regval_list  gc2035_init_regs[] = {
#if 1
	{0xfe,0x80},
	{0xfe,0x80},
	{0xfe,0x80},
	{0xfe,0x80},
	{0xfc,0x06},
	{0xf9,0xfe}, //[0] pll enable
	{0xfa,0x00},
	{0xf6,0x00},
	{0xf7,0x17}, //pll enable
	{0xf8,0x00},
	{0xfe,0x00},
	{0x82,0x00},
	{0xb3,0x60},
	{0xb4,0x40},
	{0xb5,0x60},
	{0x03,0x05},
	{0x04,0x2e},

      //measure window
	{0xfe,0x00},
	{0xec,0x04},
	{0xed,0x04},
	{0xee,0x60},
	{0xef,0x90},
	
	{0x0a,0x00}, //row start
	{0x0c,0x00}, //col start

	{0x0d,0x04},
	{0x0e,0xc0},
	{0x0f,0x06}, //Window setting
	{0x10,0x58},// 

	{0x17,0x14}, //[0]mirror [1]flip
	{0x18,0x0a}, //sdark 4 row in even frame??
	{0x19,0x0a}, //AD pipe number

	{0x1a,0x01}, //CISCTL mode4
	{0x1b,0x48},
	{0x1e,0x88}, //analog mode1 [7] tx-high en
	{0x1f,0x0f}, //analog mode2

	{0x20,0x05}, //[0]adclk mode  [1]rowclk_MODE [2]rsthigh_en
	{0x21,0x0f}, //[3]txlow_en
	{0x22,0xf0}, //[3:0]vref
	{0x23,0xc3}, //f3//ADC_r
	{0x24,0x1f}, //pad drive

	{0xad,0x7e},  // r  ratio 0x80
	{0xae,0x80}, // g ratio  0x80
	{0xaf,0x84},// b ratio  0x80

	//==============================aec
	//AEC
	{0xfe,0x01},
	{0x09,0x00},

	//{0x0c,0x01}, //0x01  aec measure point  james added
	
	{0x11,0x10},
	{0x47,0x30},
	{0x0b,0x90},
	{0x13,0x78}, //0x80  aec target
	{0x1f,0xc0},//addd
	{0x20,0x50},//add  0x60  pre gain
	{0xfe,0x00},
	{0xf7,0x17}, //pll enable
	{0xf8,0x00},
	
	{0x05,0x01},
	{0x06,0x18},
	{0x07,0x00},
	{0x08,0x48},
	{0xfe,0x01},
	{0x27,0x00},
	{0x28,0x6a},
	{0x29,0x03},
	{0x2a,0x50},//8fps
	{0x2b,0x04},
	{0x2c,0xf8},
	{0x2d,0x06},
	{0x2e,0xa0},//6fps
	{0x3e,0x40},//0x40                   
	{0xfe,0x00},           
	{0xb6,0x03}, //AEC enable
	{0xfe,0x00},

	///////BLK

	{0x3f,0x00}, //prc close???
	{0x40,0x77}, // a7 77
	{0x42,0x7f},
	{0x43,0x2b},//0x30 

	{0x5c,0x08},
	//{0x6c  3a //manual_offset ,real B channel
	//{0x6d  3a//manual_offset ,real B channel
	//{0x6e  36//manual_offset ,real R channel
	//{0x6f  36//manual_offset ,real R channel
	{0x5e,0x20},
	{0x5f,0x20},
	{0x60,0x20},
	{0x61,0x20},
	{0x62,0x20},
	{0x63,0x20},
	{0x64,0x20},
	{0x65,0x20},
	{0x66,0x20},
	{0x67,0x20},
	{0x68,0x20},
	{0x69,0x20},

	/////crop// 
	{0x90,0x01},  //crop enable
	{0x95,0x04},  //1600x1200
	{0x96,0xb0},
	{0x97,0x06},
	{0x98,0x40},

	{0xfe,0x03},
	{0x42,0x80}, 
	{0x43,0x06}, //output buf width //buf width这一块的配置还需要搞清楚
	{0x41,0x02}, // delay
	{0x40,0x00}, //fifo half full trig
	{0x17,0x01}, //wid mipi部分的分频是为什么v？
	{0xfe,0x00},

	{0x80,0xff},//block enable 0xff
	{0x81,0x26},//38  //skin_Y 8c_debug

	{0x03,0x05},
	{0x04,0x2e}, 
	{0x84,0x02}, //output put foramat
	{0x86,0x07}, //sync plority
	{0x87,0x80}, //middle gamma on
	{0x8b,0xbc},//debug mode需要搞清楚一下
	{0xa7,0x80},//B_channel_gain
	{0xa8,0x80},//B_channel_gain
	{0xb0,0x80}, //globle gain
	{0xc0,0x40},
	
#if 1	
 ////ba-wang -default_init///// 
	{0xfe,0x01},
	{0xc2,0x10},//0x1f
	{0xc3,0x04},//0x07
	{0xc4,0x01},//0x03
	{0xc8,0x08},//10
	{0xc9,0x04},//0x0a
	{0xca,0x02},//0x08
	{0xbc,0x16},//0x4a
	{0xbd,0x10},//0x1c
	{0xbe,0x10},//0x1a
	{0xb6,0x10},//0x30
	{0xb7,0x08},//0x1c
	{0xb8,0x06},//0x15
	{0xc5,0x00},
	{0xc6,0x00},
	{0xc7,0x00},
	{0xcb,0x00},
	{0xcc,0x00},
	{0xcd,0x00},
	{0xbf,0x04},//0x0c
	{0xc0,0x00},//0x04
	{0xc1,0x00},
	{0xb9,0x00},
	{0xba,0x00},
	{0xbb,0x00},
	{0xaa,0x00},
	{0xab,0x00},
	{0xac,0x00},
	{0xad,0x00},
	{0xae,0x00},
	{0xaf,0x00},
	{0xb0,0x00},
	{0xb1,0x00},
	{0xb2,0x00},
	{0xb3,0x00},
	{0xb4,0x00},
	{0xb5,0x00},
	{0xd0,0x01},
	{0xd2,0x00},
	{0xd3,0x00},
	{0xd8,0x00},
	{0xda,0x00},
	{0xdb,0x00},
	{0xdc,0x00},
	{0xde,0x00},//0x07
	{0xdf,0x00},
	{0xd4,0x00},
	{0xd6,0x00},
	{0xd7,0x00},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0x04},
	{0xa7,0x00},
	{0xa8,0x00},
	{0xa9,0x00},
	{0xa1,0x80},
	{0xa2,0x80},	
#elif 0
	{0xfe,0x01},  //CHT_809_LENS_LSC
	{0xc2,0x29},
	{0xc3,0x1d},
	{0xc4,0x14},
	{0xc8,0x0d},
	{0xc9,0x0c},
	{0xca,0x06},
	{0xbc,0x39},
	{0xbd,0x1f},
	{0xbe,0x1d},
	{0xb6,0x2d},
	{0xb7,0x23},
	{0xb8,0x16},
	{0xc5,0x13},
	{0xc6,0x0e},
	{0xc7,0x0b},
	{0xcb,0x00},
	{0xcc,0x00},
	{0xcd,0x00},
	{0xbf,0x06},
	{0xc0,0x00},
	{0xc1,0x00},
	{0xb9,0x05},
	{0xba,0x05},
	{0xbb,0x00},
	{0xaa,0x06},
	{0xab,0x02},
	{0xac,0x02},
	{0xad,0x00},
	{0xae,0x00},
	{0xaf,0x00},
	{0xb0,0x03},
	{0xb1,0x00},
	{0xb2,0x00},
	{0xb3,0x04},
	{0xb4,0x00},
	{0xb5,0x04},
	{0xd0,0x00},
	{0xd2,0x00},
	{0xd3,0x00},
	{0xd8,0x00},
	{0xda,0x00},
	{0xdb,0x00},
	{0xdc,0x00},
	{0xde,0x00},
	{0xdf,0x00},
	{0xd4,0x00},
	{0xd6,0x00},
	{0xd7,0x00},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0x77},
	{0xa7,0x77},
	{0xa8,0x00},
	{0xa9,0x00},
	{0xa1,0x80},
	{0xa2,0x80},
#elif 0 
	{0xfe,0x01}, //CHT_810_lens
	{0xc2,0x1f},
	{0xc3,0x10},
	{0xc4,0x10},
	{0xc8,0x1c},
	{0xc9,0x14},
	{0xca,0x0e},
	{0xbc,0x3c},
	{0xbd,0x25},
	{0xbe,0x22},
	{0xb6,0x35},
	{0xb7,0x24},
	{0xb8,0x17},
	{0xc5,0x00},
	{0xc6,0x0b},
	{0xc7,0x00},
	{0xcb,0x00},
	{0xcc,0x02},
	{0xcd,0x00},
	{0xbf,0x05},
	{0xc0,0x06},
	{0xc1,0x08},
	{0xb9,0x00},
	{0xba,0x00},
	{0xbb,0x07},
	{0xaa,0x00},
	{0xab,0x00},
	{0xac,0x03},
	{0xad,0x06},
	{0xae,0x0a},
	{0xaf,0x0a},
	{0xb0,0x08},
	{0xb1,0x00},
	{0xb2,0x00},
	{0xb3,0x02},
	{0xb4,0x00},
	{0xb5,0x03},
	{0xd0,0x00},
	{0xd2,0x00},
	{0xd3,0x00},
	{0xd8,0x0f},
	{0xda,0x00},
	{0xdb,0x00},
	{0xdc,0x00},
	{0xde,0x12},
	{0xdf,0x0f},
	{0xd4,0x00},
	{0xd6,0x05},
	{0xd7,0x0c},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0x00},
	{0xa7,0x00},
	{0xa8,0x00},
	{0xa9,0x00},
	{0xa1,0x80},
	{0xa2,0x80},
	{0xfe,0x00},
#elif 0
       {0xfe,0x01},/////BW-M2002C////
	{0xc2,0x27},
	{0xc3,0x14},
	{0xc4,0x10},
	{0xc8,0x1a},
	{0xc9,0x15},
	{0xca,0x10},
	{0xbc,0x38},
	{0xbd,0x1e},
	{0xbe,0x1a},
	{0xb6,0x42},
	{0xb7,0x24},
	{0xb8,0x1a},
	{0xc5,0x0c},
	{0xc6,0x00},
	{0xc7,0x00},
	{0xcb,0x00},
	{0xcc,0x00},
	{0xcd,0x00},
	{0xbf,0x0c},
	{0xc0,0x03},
	{0xc1,0x00},
	{0xb9,0x16},
	{0xba,0x00},
	{0xbb,0x00},
	{0xaa,0x09},
	{0xab,0x0b},
	{0xac,0x0f},
	{0xad,0x03},
	{0xae,0x0b},
	{0xaf,0x0f},
	{0xb0,0x00},
	{0xb1,0x05},
	{0xb2,0x05},
	{0xb3,0x03},
	{0xb4,0x04},
	{0xb5,0x07},
	{0xd0,0x09},
	{0xd2,0x00},
	{0xd3,0x08},
	{0xd8,0x00},
	{0xda,0x00},
	{0xdb,0x00},
	{0xdc,0x00},
	{0xde,0x00},
	{0xdf,0x00},
	{0xd4,0x00},
	{0xd6,0x00},
	{0xd7,0x07},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0x77},
	{0xa7,0x77},
	{0xa8,0x00},
	{0xa9,0x00},
	{0xa1,0x80},
	{0xa2,0x80},

#endif
	{0xfe,0x02},
	{0xa4,0x00},
	{0xfe,0x00},

	{0xfe,0x02},
	{0xc0,0x00},   //0x01
	{0xc1,0x3c},  //0x40 Green_cc
	{0xc2,0xfc},
	{0xc3,0xfd},//0x05
	{0xc4,0x00},//0xec
	{0xc5,0x42},//0x42
	{0xc6,0x00},//0xf8

	{0xc7,0x3c},
	{0xc8,0xfc},
	{0xc9,0xfd},
	{0xca,0x00},
	{0xcb,0x42},
	{0xcc,0x00},

	{0xcd,0x3c},
	{0xce,0xfc},
	{0xcf,0xfd},
	{0xe3,0x00},
	{0xe4,0x42},
	{0xe5,0x00},

	{0xfe,0x00},

#if 1
	//awb  BW  default
	{0xfe,0x01},
	{0x4f,0x00}, 
	{0x4d,0x10}, ////////////////10
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x20},  ///////////////20
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x30}, //////////////////30
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x04},  // office
	{0x4e,0x00},   
	{0x4e,0x02},  // d65
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x40},  //////////////////////40
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},  // cwf    
	{0x4e,0x08},  // cwf 
	{0x4e,0x04},  // d50
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x50}, //////////////////50
	{0x4e,0x00},  
	{0x4e,0x00},
	{0x4e,0x00},  
	{0x4e,0x10},  // tl84 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x60}, /////////////////60
	{0x4e,0x00},    
	{0x4e,0x00},      
	{0x4e,0x00},  
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x70}, ///////////////////70
	{0x4e,0x00},  
	{0x4e,0x00},  
	{0x4e,0x20},  // a 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x80}, /////////////////////80
	{0x4e,0x00}, 
	{0x4e,0x40},  // h
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x90}, //////////////////////90
	{0x4e,0x00},  // h
	{0x4e,0x40},  // h
	{0x4e,0x40},  // h 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xa0}, /////////////////a0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xb0}, //////////////////////////////////b0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xc0}, //////////////////////////////////c0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xd0}, ////////////////////////////d0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xe0}, /////////////////////////////////e0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xf0}, /////////////////////////////////f0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4f,0x01},
#elif 0
	{0xfe,0x01}, //CHT_810_AWB
	{0x4f,0x00}, 
	{0x4d,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x10}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4d,0x20},  ///////////////20
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x30}, //////////////////30
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x02}, //SPL D65
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x40},  //////////////////////40
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x04},//OFFICE &M60 D65
	{0x4e,0x02}, ///SPL D65&judge II D65
	{0x4e,0x02}, //spl d65
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x50}, //////////////////50
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x08}, //M60 CWF
	{0x4e,0x08}, //SPL cwf&outdoor green&judge II CWF
	{0x4e,0x04}, //SPL D50
	{0x4e,0x04}, //outdoor d50 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x60}, /////////////////60
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x20}, ///SPL TL84&skin&M60 U30
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x70}, ///////////////////70
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x20},  //SPL A light&judge II U30/A&M60 A
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x80}, /////////////////////80
	{0x4e,0x00},
	{0x4e,0x20},//SPL H&M60 A
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},           
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0x90}, //////////////////////90
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xa0}, /////////////////a0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xb0}, //////////////////////////////////b0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xc0}, //////////////////////////////////c0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xd0}, ////////////////////////////d0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xe0}, /////////////////////////////////e0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xf0}, /////////////////////////////////f0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4f,0x01},   
	/********************
	{0x56,0x06},
	{0x52,0x40},
	{0x54,0x60},
	{0x57,0x20}, //pre adjust
	{0x58,0x01}, 
	{0x5b,0x04}, //AWB_gain_delta
	{0x61,0xaa}, //R/G stand
	{0x62,0xaa}, //R/G stand
	{0x71,0x00},
	{0x72,0x25}, //AWB Y2C diff
	{0x74,0x10},  //AWB_C_max
	{0x77,0x08},  //AWB_p2_x
	{0x78,0xfd}, //AWB_p2_y
	{0x86,0x30},
	{0x87,0x00},
	{0x88,0x04}, //06  //[1]dark mode
	{0x8a,0xc0}, //[7] move mode en [6:0]move mode thd
	{0x89,0x73},
	{0x84,0x08},  //auto_window
	{0xfe,0x00},
	{0xec,0x06},//measure win
	{0xed,0x06},
	{0xee,0x62},
	{0xef,0x92},
	{0x8b,0x00},  //awb compare luma
	{0x8d,0x70}, //awb gain limit R 
	{0x8e,0x70}, //G
	{0x8f,0xf4}, //B
	*****************/
#elif 0		
	{0xfe,0x01}, //CHT_809_LENS_AWB
	{0x4f,0x00}, 
	{0x4d,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x10}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4d,0x20},  ///////////////20
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x30}, //////////////////30
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x04}, //OFFICE&opt
	{0x4e,0x02}, //SPL D65&blue
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x40},  //////////////////////40
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},  
	{0x4e,0x08}, //SPL CWF &outdoor green
	{0x4e,0x04}, //OUTDOOR D50 PM 2:50 &M60 D65
	{0x4e,0x04}, //SPL D65&OUTDOOR D50 PM &judge II D65
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x50}, //////////////////50
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x08}, //judge II CWF & M60 CWF
	{0x4e,0x08},//SPL CWF 
	{0x4e,0x04}, //SPL D50 &skin
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x60}, /////////////////60
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x10}, ///SPL TL84
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x70}, ///////////////////70
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x20},  //SPL A light&judge II U30/A
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x80}, /////////////////////80
	{0x4e,0x00},
	{0x4e,0x20}, //SPL H
	{0x4e,0x20}, //judge II A &M60 A
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},           
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0x90}, //////////////////////90
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xa0}, /////////////////a0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xb0}, //////////////////////////////////b0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xc0}, //////////////////////////////////c0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xd0}, ////////////////////////////d0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xe0}, /////////////////////////////////e0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0xf0}, /////////////////////////////////f0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4f,0x01},
#elif 0
	{0xfe,0x01},//////////BW-M2002C
	{0x4f,0x00}, 
	{0x4d,0x10}, ///////////10
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4d,0x20}, ////////////20
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x30}, /////////////30
	{0x4e,0x00},
	{0x4e,0x80},//green
	{0x4e,0x80},//green
	{0x4e,0x02},//OPT  
	{0x4e,0x04},//office&M60 D65
	{0x4e,0x02},//SPL d65&7000K
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x40}, ////////////40
	{0x4e,0x00},
	{0x4e,0x80},//green
	{0x4e,0x80},//M60 CWF&green
	{0x4e,0x02},//SPL cwf&OFFICE
	{0x4e,0x02},//SPL D50 &M60 D65
	{0x4e,0x02},//outdoor
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x50}, /////////////50
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x08}, //SPL cwf&TL84
	{0x4e,0x04},//D50 
	{0x4e,0x00}, 
	{0x4e,0x00},  
	{0x4e,0x00},  
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x60}, /////////////60
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x20}, //SPL A&M60 U30
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x70}, /////////////70
	{0x4e,0x00}, 
	{0x4e,0x20},//SPL H
	{0x4e,0x20},//M60 A
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4d,0x80}, //////////////80
	{0x4e,0x00},
	{0x4e,0x20},//M60 A
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0x90}, ////////////////90
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xa0}, /////////////////a0
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4d,0xd0},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00}, 
	{0x4e,0x00}, 
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4e,0x00},
	{0x4f,0x01}, 
#endif
	{0xfe,0x01},
	{0x50,0xc8}, //0xc8
	{0x52,0x40},
	{0x54,0x60},
	{0x56,0x06},
	{0x57,0x20}, //pre adjust
	{0x58,0x01}, 
	{0x5c,0xf0},
	{0x5d,0x40},
	{0x5b,0x02}, //AWB_gain_delta
	{0x61,0xaa},//R/G stand
	{0x62,0xaa},//R/G stand
	{0x71,0x00},
	{0x74,0x10},  //AWB_C_max
	{0x77,0x08},  //AWB_p2_x
	{0x78,0xfd}, //AWB_p2_y
	{0x86,0x30},
	{0x87,0x00},
	{0x88,0x00},//04  0x06  dark_mode
	{0x8a,0xc0},//awb move mode 
	{0x89,0x75},  //0x75
	{0x84,0x08},  //auto_window
	{0x8b,0x00},  //awb compare luma
	{0x8d,0x70}, //awb gain limit R 
	{0x8e,0x70},//G
	{0x8f,0xf4},//B


	{0x5e,0xa4},
	{0x5f,0x60},
	{0x92,0x58},
	{0xfe,0x00},
	{0x82,0x02},//awb_en

	//fe ,0xec}, luma_value

	{0xfe,0x01},
	{0x9c,0x02},// abs
	
	{0x21,0xbf},
	{0xfe,0x02},
	{0xa5,0x60}, //lsc_th //40
	{0xa2,0xa0}, //lsc_dec_slope
	{0xa3,0x30}, //when total gain is bigger than the value, enter dark light mode  0x20 added
	{0xa3,0x30},//add 
	{0xa6,0x50}, //dd_th
	{0xa7,0x50}, //ot_th
	{0xab,0x31}, //[0]b_dn_effect_dark_inc_or_dec
	{0x88,0x15}, //[5:0]b_base
	{0xa9,0x6c}, //[7:4] ASDE_DN_b_slope_high

	{0xb0,0x66},  //6edge effect slope 0x66

	{0xb3,0x40}, //saturation dec slope  //0x70   0x40
	{0xb4,0x31},//0x32 0x42
	{0x8c,0xf6}, //[2]b_in_dark_inc
	{0x89,0x03}, //dn_c_weight

	{0xde,0xb9},  //b6[7]asde_autogray [3:0]auto_gray_value  0xb9
	{0x38,0x08},  //0aasde_autogray_slope 0x08  0x05
	{0x39,0x50},  //50asde_autogray_threshold  0x50     0x30

	{0xfe,0x00},
	{0x81,0x26},
	{0x87,0x90}, //[5:4]first_dn_en first_dd_en  0x80
	{0xfe,0x02},
	{0x83,0x00},//[6]green_bks_auto [5]gobal_green_bks
	{0x84,0x45},//RB offset
	{0xd1,0x3c},  //saturation_cb  0x38
	{0xd2,0x3c},  //saturation_Cr  0x38
	{0xd3,0x38},	// contrast 0x40
	{0xdc,0x30},
	{0xdd,0xb8},  //edge_sa_g,b
	{0xfe,0x00}, 

	{0xfe,0x02},
	{0x15,0x0e},
	{0x16,0x1b},
	{0x17,0x21},
	{0x18,0x27},
	{0x19,0x32},
	{0x1a,0x3d},
	{0x1b,0x46},
	{0x1c,0x4f},
	{0x1d,0x60},
	{0x1e,0x6c},
	{0x1f,0x77},
	{0x20,0x7e},
	{0x21,0x88},
	{0x22,0x97},
	{0x23,0xa2},
	{0x24,0xb0},
	{0x25,0xbb},
	{0x26,0xcc},
	{0x27,0xd8},
	{0x28,0xe1},
	{0x29,0xe9},


#if 0
//rgb-gamma
	{0xfe,0x02},   //default gamma
	{0x15,0x0a},
	{0x16,0x0c},
	{0x17,0x0b},
	{0x18,0x10},
	{0x19,0x13},
	{0x1a,0x21},
	{0x1b,0x2b},
	{0x1c,0x33},
	{0x1d,0x43},
	{0x1e,0x50},
	{0x1f,0x5f},
	{0x20,0x6b},
	{0x21,0x7a},
	{0x22,0x8e},
	{0x23,0xa4},
	{0x24,0xb8},
	{0x25,0xc8},
	{0x26,0xdf},
	{0x27,0xed},
	{0x28,0xf8},
	{0x29,0xff},
#endif

	//y-gamma
	{0x2b,0x00},
	{0x2c,0x04},
	{0x2d,0x09},
	{0x2e,0x18},
	{0x2f,0x27},
	{0x30,0x37},
	{0x31,0x49},
	{0x32,0x5c},
	{0x33,0x7e},
	{0x34,0xa0},
	{0x35,0xc0},
	{0x36,0xe0},
	{0x37,0xff},
	{0xfe,0x00},

	// a-lsc
	{0xfe,0x01},
	{0xa0,0x03},//0f
	{0xe8,0x51},
	{0xea,0x42},
	{0xe6,0x71},
	{0xe4,0x80},
	{0xe9,0x1f},
	{0x4c,0x13},
	{0xeb,0x1e},
	{0x4d,0x13},
	{0xe7,0x20},
	{0x4b,0x15},
	{0xe5,0x1c},
	{0x4a,0x15},
	{0xe0,0x20},
	{0xe1,0x20},
	{0xe2,0x20},
	{0xe3,0x20},
	{0xd1,0x74},
	{0x4e,0x0e},
	{0xd9,0x91},
	{0x4f,0x27},
	{0xdd,0x78},
	{0xce,0x31},
	{0xd5,0x71},
	{0xcf,0x39},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0x00},
	{0xa7,0x00},
	{0xa8,0x66},
	{0xa9,0x66},
	{0xa1,0x80},
	{0xa2,0x80},
	{0xfe,0x00},

	{0x82,0xfe},
	//sleep  400
	{0xf2,0x70},
	{0xf3,0xff},
	{0xf4,0x00},
	{0xf5,0x30},
	{0xfe,0x01},
	{0x0b,0x90},
	{0x87,0x10},
	{0xfe,0x00},

	/////,0xup},date
	//热?0x  
	{0xfe,0x02},
	{0xa6,0x80}, //dd_th
	{0xa7,0x50}, //ot_th //0x80 0x30
	{0xa9,0x66}, //6f[7:4] ASDE_DN_b_slope_high 0x68
	{0xb0,0x66},  //edge effect slope 0x99 0x66
	{0x38,0x08},  //asde_autogray_slope 0x08   0x0f
	{0x39,0x50},  //asde_autogray_threshold  0x60
	{0xfe,0x00},
	{0x87,0x90}, //[5:4]first_dn_en first_dd_en
#if 1
//// //subsample for svga///////
	{0xfe,0x00},  
	{0x0c,0x00}, //col  start
	
	{0x0d,0x04},
	{0x0e,0xc0},
	{0x0f,0x06},
	{0x10,0x56},  //50
	{0x99,0x22},
	{0x9b,0x00},
	{0x9f,0x00},
	{0x90,0x01},
        {0x94,0x04},  //x start
	{0x95,0x02},
	{0x96,0x58},//600
	{0x97,0x03},
	{0x98,0x20},//800
	{0xc8,0x00},

	{0xf7,0x0D},
	{0xf8,0x82},  //0x83
	{0xfa,0x00},//pll=4


	{0xfe,0x03},
	{0x42,0x40}, 
	{0x43,0x06}, //output buf width
	//{0x41,0x00}, // delay
	{0x40,0x40}, //fifo half full trig
	{0x17,0x00}, //widv is 0

	{0xfe,0x00},
	{0xc8,0x00}, 
	{0xb6,0x03}, 
//// //subsample for svga///////	
#elif 0
/*******scalar for svga**********/
        {0xfe , 0x00},
	{0x0a , 0x00}, //row start
	{0x0c , 0x00}, //col start

	{0x0d , 0x04},
	{0x0e , 0xc0},
	{0x0f , 0x06}, //Window setting
	{0x10 , 0x58},// 
	
	{0x90 , 0x01},
	//{0x94 , 0x00},  // x_start	
	{0x95 , 0x02},
	{0x96 , 0x58},// 600
	{0x97 , 0x03},
	{0x98 , 0x20},// 800
	{0xc8 , 0x14},

	{0xf7,0x0D},
	{0xf8,0x83},  //0x83
	{0xfa,0x00},//pll=4

	{0xfe , 0x03},
	{0x42 , 0x40}, 
	{0x43 , 0x06}, //output buf width  280*2=500
	//{0x41 , 0x02}, // delay
	{0x40 , 0x40}, //fifo half full trig
	{0x17 , 0x00}, //widv is 0

	{0xfe , 0x00},
	{0xc8 , 0x54}, 
	{0xb6 , 0x03},//aec on
	{0xff , 0xff}, 
	{0xff , 0xff}, 
/*******scalar for svga**********/

#elif 0
//////////scalar for vga////////////
	{0xfe,0x00},
	{0x90,0x01},
	{0x94,0x00},	  // x_start
	{0x95,0x01},
	{0x96,0xe0}, //480
	{0x97,0x02},
	{0x98,0x80},  // 640
	{0xc8,0x15},
	{0xf7,0x0D},
	{0xf8,0x83},  //0x83
	{0xfa,0x00},//pll=4
	
	{0xfe,0x03},
	{0x42,0x00}, 
	{0x43,0x05}, //output buf width   280*2=500
	{0x41,0x02}, // delay
	{0x40,0x40}, //fifo half full trig
	{0x17,0x00}, //widv is 0
	{0xfe,0x00},
	{0xc8,0x55}, 
//////////scalar for vga////////////

#elif 0
////////////subsample for vga/////////
	{0xfe,0x00},
	{0x0d,0x04},
	{0x0e,0xc0},
	{0x0f,0x06},
	{0x10,0x50},
	{0x99,0x55},
	{0x9b,0x01},
	{0x9f,0x01},
	{0x90,0x01},
	{0x95,0x01},
	{0x96,0xe0},
	{0x97,0x02},
	{0x98,0x80},
	{0xc8,0x00},

	//{0xf7,0x11}, //pll enable,  daemon , for 12M MCLK
	{0xf7,0x0D},
	{0xf8,0x83},
	{0xfa,0x00},//pll=4 added

	{0xfe,0x03},
	{0x42,0x40}, 
	{0x43,0x06}, //output buf width
	{0x41,0x00}, // delay  0x02
	{0x40,0x40}, //fifo half full trig
	{0x17,0x00}, //widv is 0

	{0xfe,0x00},
	{0xc8,0x00}, 
////////////subsample for vga/////////
#endif
#if 1  //	/////banding////////////
	{0xfe,0x00},  // banding for f7->oxod,0xf8->0x82
	{0x05,0x01},
	{0x06,0x0a},
	{0x07,0x00},
	{0x08,0x58},  
	{0xfe,0x01},
	{0x27,0x00},
	{0x28,0xa1},
	{0x29,0x05},
	{0x2a,0x08},//18fps
	{0x2b,0x06},
	{0x2c,0x4a},//12.5fps
	{0x2d,0x07},
	{0x2e,0x8c},//8fps
	{0x2f,0x08},
	{0x30,0xce},//8fps
	{0x3e,0x40},//0x40 0x00
#elif 0

	{0xfe,0x00},
	{0x05,0x00},
	{0x06,0xc4},
	{0x07,0x00},
	{0x08,0xae},  
	{0xfe,0x01},
	{0x27,0x00},
	{0x28,0xe5},
	{0x29,0x05},
	{0x2a,0x5e},//lev 1        55e-> 16fps;  643->14fps;728->12fps;8f2->10fps;abc-8fps
	{0x2b,0x06},
	{0x2c,0x43},//lev 2
	{0x2d,0x08},
	{0x2e,0xf2},//lev 3
	{0x2f,0x0a},
	{0x30,0xbc},//8fps
	{0x3e,0x40},// max->lev 3
	{0xfe,0x00},
#endif

#endif
 
};
static struct gc2035_regval_list  gc2035_vga_regs[] = {

#if 0
/*******scalar for svga**********/
        {0xfe , 0x00},
	{0x0a , 0x00}, //row start
	{0x0c , 0x00}, //col start

	{0x0d , 0x04},
	{0x0e , 0xc0},
	{0x0f , 0x06}, //Window setting
	{0x10 , 0x58},// 
	
	{0x90 , 0x01},
	//{0x94 , 0x00},  // x_start	
	{0x95 , 0x02},
	{0x96 , 0x58},// 600
	{0x97 , 0x03},
	{0x98 , 0x20},// 800
	{0xc8 , 0x14},

	{0xfa , 0x00}, 

	{0xfe , 0x03},
	{0x42 , 0x40}, 
	{0x43 , 0x06}, //output buf width  280*2=500
	//{0x41 , 0x02}, // delay
	{0x40 , 0x40}, //fifo half full trig
	{0x17 , 0x00}, //widv is 0

	{0xfe , 0x00},
	{0xc8 , 0x54}, 
	{0xb6 , 0x03},//aec on
/*******scalar for svga**********/
#elif 0
       {0xfe , 0x00},
	{0x0a , 0x00}, //row start
	{0x0c , 0x00}, //col start

	{0x0d , 0x04},
	{0x0e , 0xc0},
	{0x0f , 0x06}, //Window setting
	{0x10 , 0x58},// 
	
	{0x90 , 0x01},
	//{0x94 , 0x00},  // x_start	
	{0x95 , 0x01},
	{0x96 , 0xe0},
	{0x97 , 0x02},
	{0x98 , 0x80},
	{0xc8 , 0x15},

	{0xfa , 0x00}, 

	{0xfe , 0x03},
	{0x42 , 0x00}, 
	{0x43 , 0x05}, //output buf width  280*2=500
	//{0x41 , 0x02}, // delay
	{0x40 , 0x40}, //fifo half full trig
	{0x17 , 0x00}, //widv is 0

	{0xfe , 0x00},
	{0xc8 , 0x55}, 
	{0xb6 , 0x03},//aec on	
#elif 0
// ///////////subsample for vga///////////////////
	{0xfe,0x00}, // subsample for vga
	{0x0d,0x04}, 
	{0x0e,0xc0}, 
	{0x0f,0x06}, 
	{0x10,0x50}, 
	{0x99,0x55}, 
	{0x9b,0x01}, 
	{0x9f,0x01}, 
	{0x90,0x01}, 
	{0x95,0x01}, 
	{0x96,0xe0}, 
	{0x97,0x02}, 
	{0x98,0x80}, 
	{0xc8,0x00}, 
	             
	//{0xf7,0x11}, 
	             
	             
	{0xfe,0x03}, 
	{0x42,0x00}, 
	{0x43,0x05}, 
	{0x41,0x00}, //0x00   james
	{0x40,0x40}, 
	{0x17,0x00}, 
	             
	{0xfe,0x00}, 
	{0xc8,0x00}, 
// ///////////subsample for vga///////////////////	
#endif

};

static struct gc2035_regval_list  gc2035_svga_regs[] = {
#if 0
/*******scalar for svga**********/
        {0xfe , 0x00},
	{0x0a , 0x00}, //row start
	{0x0c , 0x00}, //col start

	{0x0d , 0x04},
	{0x0e , 0xc0},
	{0x0f , 0x06}, //Window setting
	{0x10 , 0x58},// 
	
	{0x90 , 0x01},
	//{0x94 , 0x00},  // x_start	
	{0x95 , 0x02},
	{0x96 , 0x58},// 600
	{0x97 , 0x03},
	{0x98 , 0x20},// 800
	{0xc8 , 0x14},

	{0xfa , 0x00}, 

	{0xfe , 0x03},
	{0x42 , 0x40}, 
	{0x43 , 0x06}, //output buf width  280*2=500
	//{0x41 , 0x02}, // delay
	{0x40 , 0x40}, //fifo half full trig
	{0x17 , 0x00}, //widv is 0

	{0xfe , 0x00},
	{0xc8 , 0x54}, 
	{0xb6 , 0x03},//aec on
/*******scalar for svga**********/
#else
//// //subsample for svga///////
	{0xfe,0x00},  
	//{0x0c,0x04}, //col  start
	
	{0x0d,0x04},
	{0x0e,0xc0},
	{0x0f,0x06},
	{0x10,0x56},  //50
	{0x99,0x22},
	{0x9b,0x00},
	{0x9f,0x00},
	{0x90,0x01},
	{0x94,0x04},  //x start
	{0x95,0x02},
	{0x96,0x58},//600
	{0x97,0x03},
	{0x98,0x20},//800
	{0xc8,0x00},

	{0xfa,0x00}, //pll enable,  daemon , for 12M MCLK

	{0xfe,0x03},
	{0x42,0x40}, 
	{0x43,0x06}, //output buf width
	//{0x41,0x00}, // delay
	{0x40,0x40}, //fifo half full trig
	{0x17,0x00}, //widv is 0

	{0xfe,0x00},
	{0xc8,0x00}, 
	{0xb6,0x03},
  
//// //subsample for svga///////
#endif
};
static struct gc2035_regval_list  gc2035_uxga_regs[] = { 	
	{0xfe , 0x00},
	{0x0a , 0x00}, //row start
	{0x0c , 0x00}, //col start

	{0x0d , 0x04},
	{0x0e , 0xc0},
	{0x0f , 0x06}, //Window setting
	{0x10 , 0x58},// 

	{0x90 , 0x01},  //crop enable
	//{0x94 , 0x04},  // x_start	
	
	{0x95 , 0x04},
	{0x96 , 0xb0},
	{0x97 , 0x06},
	{0x98 , 0x40},
	{0x99 , 0x11},
	{0xc8 , 0x00},
 
	{0xfa , 0x11},
	
	{0xfe , 0x03},
	{0x42 , 0x80}, 
	{0x43 , 0x06}, //output buf width
	//{0x41 , 0x00}, // delay
	{0x40 , 0x00}, //fifo half full trig
	{0x17 , 0x01}, //widv 
	{0xfe , 0x00},
	{0xc8 , 0x00},

	{0xff , 0xff}, 
	{0xff , 0xff}, 

};


static struct gc2035_regval_list  gc2035_qvga_regs[] = {

};

static struct gc2035_regval_list  gc2035_sepia_regs[] = {
	        {0xfe , 0x00},
               {0x83 , 0x82},
	        {0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_bluish_regs[] = {

	        {0xfe , 0x00},
               {0x83 , 0x62},
	        {0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_greenish_regs[] = {
	        {0xfe , 0x00},
               {0x83 , 0x52},
	        {0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_reddish_regs[] = {
	        {0xfe , 0x00},
               {0x83 , 0x12},
	        {0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_yellowish_regs[] = {
	        {0xfe , 0x00},
               {0x83 , 0x12},
	        {0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_bandw_regs[] = {

};

static struct gc2035_regval_list  gc2035_negative_regs[] = {
	        {0xfe , 0x00},
               {0x83 , 0x01},
	        {0xff , 0xff},

};

static struct gc2035_regval_list  gc2035_normal_regs[] = {

	        {0xfe , 0x00},
               {0x83 , 0xe0},
	        {0xff , 0xff},
};
	
static struct gc2035_regval_list  gc2035_auto_regs[] = {
		{0xfe , 0x00},			
		{0xb3 , 0x60},
		{0xb4 , 0x40},
		{0xb5 , 0x60},
		{0x82 , 0xfe},	
		{0xff , 0xff},
	
};

static struct gc2035_regval_list  gc2035_sunny_regs[] = {
		{0xfe , 0x00},				
		{0x82 , 0xfc},		
		{0xb3 , 0x78},	
		{0xb4 , 0x40},	
		{0xb5 , 0x50},	
		{0xff , 0xff}
};

static struct gc2035_regval_list  gc2035_cloudy_regs[] = {
//bai re guang
		{0xfe , 0x00},			
		{0x82 , 0xfc},		
		{0xb3 , 0x58},	
		{0xb4 , 0x40},	
		{0xb5 , 0x50},	 
		{0xff , 0xff},
};

static struct gc2035_regval_list  gc2035_office_regs[] = {
//ri guang deng
		{0xfe , 0x00},			
		{0x82 , 0xfc},		
		{0xb3 , 0x50},	
		{0xb4 , 0x40},	
		{0xb5 , 0xa8},	
		{0xff , 0xff},
	
};

static struct gc2035_regval_list  gc2035_home_regs[] = {
		{0xfe , 0x00},			
		{0x82 , 0xfc},		
		{0xb3 , 0xa0},	
		{0xb4 , 0x45},	
		{0xb5 , 0x40},	
		{0xff , 0xff},

};

static struct gc2035_regval_list  gc2035_sunset_regs[] = {
		{0xfe,0x01}, // set page 1
		{0x3e,0x00},  
	       {0xfe,0x00}, // set page 0
};

static struct gc2035_regval_list  gc2035_night_regs[] = {
		{0xfe,0x01}, // set page 1
		{0x3e,0x60},  
	       {0xfe,0x00}, // set page 0
};

static	struct gc2035_regval_list  gc2035_50hz_regs[] = {
/////banding////////////
	{0x05,0x00},
	{0x06,0xc4},
	{0x07,0x00},
	{0x08,0xae},  
	{0xfe,0x01}, // set page 1
	{0x27,0x00},
	{0x28,0xe5},
	{0x29,0x05},
	{0x2a,0x5e},//lev 1        55e-> 16fps;  643->14fps;728->12fps;8f2->10fps;abc-8fps
	{0x2b,0x06},
	{0x2c,0x43},//lev 2
	{0x2d,0x08},
	{0x2e,0xf2},//lev 3
	{0x2f,0x08},
	{0x30,0xf2},//lev 3	
	{0x3e,0x40},// max->lev 3};
	{0xfe,0x00}, // set page 0
	{0xff,0xff},
};
static	struct gc2035_regval_list  gc2035_60hz_regs[] = {
/////banding////////////
#if 0
	{0x05,0x00},
	{0x06,0xc4},
	{0x07,0x00},
	{0x08,0xae},  
	{0xfe,0x01}, // set page 1
	{0x27,0x00},
	{0x28,0xe5},
	{0x29,0x05},
	{0x2a,0x5e},//lev 1        55e-> 16fps;  643->14fps;728->12fps;8f2->10fps;abc-8fps
	{0x2b,0x06},
	{0x2c,0x43},//lev 2
	{0x2d,0x08},
	{0x2e,0xf2},//lev 3
	{0x2f,0x08},
	{0x30,0xf2},//lev 3	
	{0x3e,0x40},// max->lev 3};
	{0xfe,0x00}, // set page 0
	{0xff,0xff},
	#endif
};


static	struct gc2035_regval_list  gc2035_ev_neg4_regs[] = {
       {0xfe, 0x01},    
	{0x13, 0x40},    
	{0xfe, 0x02},    
	{0xd5, 0xc0},    
	{0xfe, 0x00},    
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_neg3_regs[] = {
       {0xfe, 0x01},
	{0x13, 0x50}, 
	{0xfe, 0x02},
	{0xd5, 0xd0}, 
	{0xfe, 0x00},
	{0xff, 0xff},			
};

static	struct gc2035_regval_list  gc2035_ev_neg2_regs[] = {
       {0xfe, 0x01},
	{0x13, 0x60}, 
	{0xfe, 0x02},
	{0xd5, 0xe0}, 
	{0xfe, 0x00},
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_neg1_regs[] = {
       {0xfe, 0x01},
	{0x13, 0x70}, 
	{0xfe, 0x02},
	{0xd5, 0xf0}, 
	{0xfe, 0x00},
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_zero_regs[] = {
       {0xfe, 0x01},
	{0x13, 0x78}, //0x80
       {0xfe, 0x02},
	{0xd5, 0x00}, 
	{0xfe, 0x00},
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_pos1_regs[] = {
       {0xfe, 0x01},
	{0x13, 0x98}, 
	{0xfe, 0x02},
	{0xd5, 0x10}, 
	{0xfe, 0x00},
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_pos2_regs[] = {
	{0xfe, 0x01},
	{0x13, 0xb0}, 
	{0xfe, 0x02},
	{0xd5, 0x20}, 
	{0xfe, 0x00},
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_pos3_regs[] = {
       {0xfe, 0x01},
	{0x13, 0xc0}, 
	{0xfe, 0x02},
	{0xd5, 0x30}, 
	{0xfe, 0x00},	
	{0xff, 0xff},		
};

static	struct gc2035_regval_list  gc2035_ev_pos4_regs[] = {
       {0xfe, 0x01},
	{0x13, 0xd0}, 
	{0xfe, 0x02},
	{0xd5, 0x50}, 
	{0xfe, 0x00},	
	{0xff, 0xff},	
};

