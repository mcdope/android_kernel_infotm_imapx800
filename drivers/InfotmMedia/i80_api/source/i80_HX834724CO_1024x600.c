/*****************************************************************************
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of i80 HS834724CO controller 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/08/01 :  first commit
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <ids_pwl.h>
#include <i80_api.h>
#include <i80_lib.h>
#include <i80_def.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"HX834724CO_I:"
#define WARNHEAD	"HX834724CO_W:"
#define ERRHEAD		"HX834724CO_E:"
#define TIPHEAD		"HX834724CO_T:"

static void proc1(void);
static void proc2(void);
static void proc3(void);

i80c_config_t HX834724CO_I80C_CONF = {
	{
		.Signal_rd=I80IF_SIGNAL_ACTIVE_LOW,
		.Signal_wr=I80IF_SIGNAL_ACTIVE_LOW,
		.Signal_rs=I80IF_SIGNAL_ACTIVE_LOW,	// 不要设置，因为在具体配置cmd列表时会指定RS的电平，如果这里配置了，就表示是否再翻转一次
		.Signal_cs1=I80IF_SIGNAL_ACTIVE_LOW,
		.Signal_cs0=I80IF_SIGNAL_ACTIVE_LOW,
		.nCycles_cs_setup=0,
		.nCycles_wr_setup=0,
		.nCycles_wr_active=1,
		.nCycles_wr_hold=0,
	}
	,
		.INTMASK =3,								// i80 frame end 中断使能标志 00: No interrupt 01: Frame over 10: Normal CMD over 11: Frame over and Normal CMD over
		.Main_lcd = 0 ,
		.Data_format = 0x180,
		{
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, },
			{ 0,0,0, }
		}
	,
		{
			.DISAUTOCMD = 0,
			.AUTO_CMD_RATE = I80IF_AUTOCMD_RATE_PER_1_FRAME, 
		}
	,
		.screen_width = 240,
		.screen_height = 320,
};

// power-on process : poll the blocks as its order , and do the relative actions.
// power-off process : poll the opposite order.
I80_Priv_conf HX834724CO_PRIV_CONF = {
	.block[0] = {
		I80_MANU_POWER,  // power on the i80 screen
		10, 		// sleep time after power on, in ms
		IM_NULL,
		IM_NULL,
	},
	.block[1] = {
		I80_MANU_RGB, 	// firstly MCU send command and data, transform to  RGB data send, i80 controller open 
		10, 		// sleep time after controller open, in ms
		(void *)&HX834724CO_I80C_CONF,
		IM_NULL,
	},
	.block[2] = {
		I80_MANU_BL,  	// backlight on ,
		0,			// sleep time after backlight on , in ms.
		IM_NULL,
		IM_NULL,
	},
	.block[3] = {
		I80_MANU_PROC1,   // 
		0,
		IM_NULL,
		proc1,	// proc1 
	},
	.block[4] = {
		I80_MANU_PROC2,   // 
		0,
		IM_NULL,
		proc2,  // proc2,
	},
	.block[5] = {
		I80_MANU_PROC3, // 
		0,
		IM_NULL,
		proc3,  // proc3
	}
	//..
};

// i80 screen reserve function 1
static void proc1(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

// i80 screen reserve function 2 
static void proc2(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

// i80 screen reserve function 3 
static void proc3(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}
