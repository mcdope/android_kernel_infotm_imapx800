/*****************************************************************************
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of dsi VVX10F004B00 controller 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/10/10 :  first commit
** 1.1.0	 Sam@2012/10/22 :  DSI first stable version, support the first product.
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <ids_pwl.h>
#include <dsi_api.h>
#include <dsi_lib.h>
#include <dsi_def.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"VVX10F004B00_I:"
#define WARNHEAD	"VVX10F004B00_W:"
#define ERRHEAD		"VVX10F004B00_E:"
#define TIPHEAD		"VVX10F004B00_T:"

static void proc1(void);
static void proc2(void);

mipidsi_dpi_t VVX10F004B00_DPI_CONF = {
	.width  =  1920,
	.height = 1200,
#if 0
	.vspw   = 2,
	.vbpd 	= 16,
	.vfpd 	= 17,
	.hspw 	= 16,
	.hbpd 	= 32,
	.hfpd 	= 112,
	.fpsx1000 = 50000,
#else
	.vspw   = 2,
	.vbpd 	= 10,
	.vfpd 	= 23,
	.hspw 	= 2,
	.hbpd 	= 2,
	.hfpd 	= 156,
	.fpsx1000 = 45000,
#endif
	.hsync_pol 	= 1,
	.vsync_pol 	= 1,
	.vden_pol = 1,
    .data_en_pol = 1,
	.virtual_channel = 0,
};

// power-on process : poll the blocks as its order , and do the relative actions.
// power-off process : poll the opposite order.
DSI_Priv_conf VVX10F004B00_PRIV_CONF = {
	.block[0] = {
		DSI_MANU_POWER,  // power on the dsi screen
		100, 		// sleep time after power on, in ms
		IM_NULL,
		IM_NULL,
	},
	.block[1] = {
		DSI_MANU_CMD_ON, 	// send command to power on the mipi device.
		10, 			// sleep time after power on the mipi device
		IM_NULL,
		IM_NULL,
	},
	.block[2] = {
		DSI_MANU_CMD_OFF, 	// send command to power off the mipi device.
		10, 			// sleep time after power off the mipi device
		IM_NULL,
		IM_NULL,
	},
	.block[3] = {
		DSI_MANU_RGB, 	//  RGB data send, dsi controller open 
		10, 		// sleep time after controller open, in ms
		(void *)&VVX10F004B00_DPI_CONF,
		IM_NULL,
	},
	.block[4] = {
		DSI_MANU_BL,  	// backlight on ,
		0,			// sleep time after backlight on , in ms.
		IM_NULL,
		IM_NULL,
	},
	.block[5] = {
		DSI_MANU_PROC1,   // 
		0,
		IM_NULL,
		proc1,	// proc1 
	},
	.block[6] = {
		DSI_MANU_PROC2,   // 
		0,
		IM_NULL,
		proc2,  // proc2,
	},
	//..
};

// dsi screen reserve function 1
static void proc1(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

// dsi screen reserve function 2 
static void proc2(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return;
}

