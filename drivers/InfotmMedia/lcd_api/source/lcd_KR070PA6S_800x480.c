/*****************************************************************************
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of lcd HSD070IDW1 controller 
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
#include <lcd_api.h>
#include <lcd_lib.h>
#include <lcd_def.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"KR070PA6S_I:"
#define WARNHEAD	"KR070PA6S_W:"
#define ERRHEAD		"KR070PA6S_E:"
#define TIPHEAD	    "KR070PA6S_T:"

static void proc1(void);
static void proc2(void);
static void proc3(void);

lcdc_config_t KR070PA6S_LCDC_CONF = {
	.width =  800,
	.height = 480,
	.VSPW = 13,
	.VBPD = 10,
	.VFPD = 22,
	.HSPW = 30,
	.HBPD = 16,
	.HFPD = 210,
	.signalPwrenEnable = 1,
	.signalPwrenInverse = 0,
	.signalVclkInverse = 0,
	.signalHsyncInverse = 1,
	.signalVsyncInverse = 1,
	.signalDataInverse = 0,
	.signalVdenInverse = 0,
	.SignalDataMapping = DATASIGMAPPING_BGR,
	.fpsx1000 =  60000,
};

// power-on process : poll the blocks as its order , and do the relative actions.
// power-off process : poll the opposite order.
LCD_Priv_conf KR070PA6S_PRIV_CONF = {
	.block[0] = {
		LCD_MANU_POWER,  // power on the lcd screen,
		10, 		// sleep time after power on, in ms
		IM_NULL,
		IM_NULL,
	},
	.block[1] = {
		LCD_MANU_RGB, 	//  RGB data send, lcd controller open 
		10, 		// sleep time after controller open, in ms
		(void *)&KR070PA6S_LCDC_CONF ,
		IM_NULL,
	},
	.block[2] = {
		LCD_MANU_LVDS,  	// PWM for lvds open ,
		250, 		// sleep time after lvds open , in ms,
		IM_NULL,
		IM_NULL,
	},
	.block[3] = {
		LCD_MANU_BL,  	// backlight on ,
		0,			// sleep time after backlight on , in ms.
		IM_NULL,
		IM_NULL,
	},
	.block[4] = {
		LCD_MANU_PROC1,   // 
		0,
		IM_NULL,
		proc1,	// proc1 
	},
	.block[5] = {
		LCD_MANU_PROC2,   // 
		0,
		IM_NULL,
		proc2,  // proc2,
	},
	.block[6] = {
		LCD_MANU_PROC3, // 
		0,
		IM_NULL,
		proc3,  // proc3
	}
	//..
};

// lcd screen reserve function 1
static void proc1(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

// lcd screen reserve function 2 
static void proc2(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

// lcd screen reserve function 3 
static void proc3(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}
