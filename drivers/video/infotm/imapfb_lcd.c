/***************************************************************************** 
** drivers/video/infotm/imapfb_lcd.c
** 
** Copyright (c) 2012~2020 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of Display Controller.
**
** Author:
**    Sam Ye <weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1   sam@2012/03/20 	 : first commit 
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/wait.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <InfotmMedia.h>
#include <IM_buffallocapi.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include "imapfb.h"
#include "logo_new.h"
#include "logo.h"

#if defined (CONFIG_LCDTYPE_HSD070IDW1_800x480)
#define IMAPXFB_HRES		800		/* horizon pixel x resolition */
#define IMAPXFB_VRES		480		/* line cnt y resolution */
#define IMAPXFB_BPP 	   16
#define IMAPXFB_BYTES_PER_PIXEL 2
#define IMAPXFB_PIXFMT 		FBLAYER_PIXFMT_16BPP_RGB565
#define CONFIG_FB_IMAP_BPP16

#elif defined(CONFIG_LCDTYPE_DEVBOARD_1024x600)
#define IMAPXFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPXFB_VRES		600		/* line cnt y resolution */
#define IMAPXFB_BPP 	   16
#define IMAPXFB_BYTES_PER_PIXEL 	2
#define IMAPXFB_PIXFMT 		FBLAYER_PIXFMT_16BPP_RGB565
#define CONFIG_FB_IMAP_BPP16

#elif defined(CONFIG_LCDTYPE_STARRY_20810700150212)
#define IMAPXFB_HRES		800 /* horizon pixel x resolition */
#define IMAPXFB_VRES		480 /* line cnt y resolution */
#define IMAPXFB_BPP 	   16
#define IMAPXFB_BYTES_PER_PIXEL 	2
#define IMAPXFB_PIXFMT 		FBLAYER_PIXFMT_16BPP_RGB565
#define CONFIG_FB_IMAP_BPP16

#elif defined(CONFIG_LCDTYPE_BF097XN)
#define IMAPXFB_HRES		1024/* horizon pixel x resolition */
#define IMAPXFB_VRES		768/* line cnt y resolution */
#define IMAPXFB_BPP 	   16
#define IMAPXFB_BYTES_PER_PIXEL 	2
#define IMAPXFB_PIXFMT 		FBLAYER_PIXFMT_16BPP_RGB565
#define CONFIG_FB_IMAP_BPP16

#endif

// all the values are useless except for the width & height & bpp
imapfb_fimd_info_t imapfb_fimd = {

	.bpp = IMAPXFB_BPP,
	.bytes_per_pixel = IMAPXFB_BYTES_PER_PIXEL ,
	.pixfmt = IMAPXFB_PIXFMT,

	.sync = 0,
	.cmap_static = 1,

	.xres = IMAPXFB_HRES,
	.yres = IMAPXFB_VRES,

	.osd_xres = IMAPXFB_HRES,
	.osd_yres = IMAPXFB_VRES,

	.osd_xres_virtual = IMAPXFB_HRES,
	.osd_yres_virtual = IMAPXFB_VRES,

	.osd_xoffset = 0,
	.osd_yoffset = 0,

	.pixclock = 27000000,

	.hsync_len = 10,
	.vsync_len = 10,
	.left_margin = 10,
	.upper_margin = 10,
	.right_margin = 10,
	.lower_margin = 10,
};

#if defined (CONFIG_FB_IMAP_KERNEL_LOGO)
void imapfb_kernel_logo(void *buf)
{
	int i, nX, nY;
	UINT8 *pDB;
	UINT8 *pFB;
	pFB = (UINT8 *)buf;
	
#if defined (CONFIG_FB_IMAP_BPP16)
	pDB = (UINT8 *)logo_300x120;

	memset(pFB, 0xff, IMAPFB_HRES * IMAPFB_VRES * 2);
	for (i = 0; i < IMAPFB_HRES * IMAPFB_VRES; i++)
	{
		nX = i % IMAPFB_HRES;
		nY = i / IMAPFB_HRES;
		if((nX >= ((IMAPFB_HRES - 300) / 2)) && (nX < ((IMAPFB_HRES + 300) / 2)) && (nY >= ((IMAPFB_VRES - 120) / 2)) && (nY < ((IMAPFB_VRES + 120) / 2)))
		{
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
		}
		else
		{
			pFB++;
			pFB++;
		}
	}
#elif defined (CONFIG_FB_IMAP_BPP32)
	pDB = (UINT8 *)gImage_logo;

	memset(pFB, 0xff, IMAPFB_HRES * IMAPFB_VRES * 4);
	for (i = 0; i < IMAPFB_HRES * IMAPFB_VRES; i++)
	{
		nX = i % IMAPFB_HRES;
		nY = i / IMAPFB_HRES;
		if((nX >= ((IMAPFB_HRES - 320) / 2)) && (nX < ((IMAPFB_HRES + 320) / 2)) && (nY >= ((IMAPFB_VRES - 240) / 2)) && (nY < ((IMAPFB_VRES + 240) / 2)))
		{
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
		}
		else
		{
			pFB++;
			pFB++;
			pFB++;
			pFB++;
		}
	}
#endif
}
#endif
