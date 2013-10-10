/***************************************************************************** 
** drivers/video/infotm/imapfb.h
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Head file of Display Controller.
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef _IMAPFB_H_
#define _IMAPFB_H_

#include <linux/interrupt.h>

/* Debug macros */
#define DEBUG 0

/* Open/Close framebuffer window */
#define ON 	1
#define OFF	0

#define IMAPFB_MAX_NUM			1	/* max framebuffer device number */

#define IMAPFB_PALETTE_BUFF_CLEAR		0x80000000	/* palette entry is clear/invalid */

/* Macros */
#define FB_MIN_NUM(x, y)		((x) < (y) ? (x) : (y))
#define IMAPFB_NUM			FB_MIN_NUM(IMAPFB_MAX_NUM, CONFIG_FB_IMAP_NUM)

/* IO controls */
#define IMAPFB_SWITCH_ON 	_IO ('F', 207)
#define IMAPFB_SWITCH_OFF      _IO ('F', 208)

#define IMAPFB_GET_LCD_EDID_INFO		_IOR ('F', 327, void*)
#define IMAPFB_LCD_DETECT_CONNECT	_IO ('F', 328)

#define IMAPFB_GET_PHY_ADDR             0x121115
#define IMAPFB_GET_PMM_DEVADDR 			0x120514

/* Data type definition */
typedef unsigned long long	UINT64;
typedef unsigned int		UINT32;
typedef unsigned short		UINT16;
typedef unsigned char		UINT8;
typedef signed long long	SINT64;
typedef signed int			SINT32;
typedef signed short		SINT16;
typedef signed char		SINT8;

/* Color key function info. */
typedef struct {
	UINT32 direction;
	UINT32 colval;
} imapfb_color_key_info_t;

/* RGB and transparency bit field assignment */
typedef struct {
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
} imapfb_rgb_t;

const static imapfb_rgb_t imapfb_a1rgb232_8 = {
	.red		= {.offset = 5, .length = 2,},
	.green	= {.offset = 2, .length = 3,},
	.blue	= {.offset = 0, .length = 2,},
	.transp	= {.offset = 7, .length = 1,},
};

const static imapfb_rgb_t imapfb_rgb565_16 = {
	.red		= {.offset = 11, .length = 5,},
	.green	= {.offset = 5, .length = 6,},
	.blue	= {.offset = 0, .length = 5,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_rgb666_18 = {
	.red		= {.offset = 12, .length = 6,},
	.green	= {.offset = 6, .length = 6,},
	.blue	= {.offset = 0, .length = 6,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_a1rgb666_19 = {
	.red		= {.offset = 12, .length = 6,},
	.green	= {.offset = 6, .length = 6,},
	.blue	= {.offset = 0, .length = 6,},
	.transp	= {.offset = 18, .length = 1,},
};

const static imapfb_rgb_t imapfb_rgb888_24 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_a1rgb888_25 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 1,},
};

const static imapfb_rgb_t imapfb_a4rgb888_28 = {
	.red		= {.offset = 16,.length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 4,},
};

const static imapfb_rgb_t imapfb_a8rgb888_32 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 8,},
};

/* Imap framebuffer struct */
typedef struct {
	struct fb_info		fb;		/* linux framebuffer struct */
	struct device		*dev;	/* linux framebuffer device */

	struct clk		*clk1;	/* clock resource for imapx200 display controller subsystem (IDS) */
	struct clk		*clk2;	/* clock resource for imapx200 display controller subsystem (IDS) */

	//struct resource	*mem;	/* memory resource for IDS mmio */
	//void __iomem		*io;		/* mmio for IDS */

	UINT32			win_id;	/* framebuffer window number */

	/* buf0 raw memory addresses */
	dma_addr_t		map_dma_f1;	/* physical */
	u_char *			map_cpu_f1;	/* virtual */
	UINT32			map_size_f1;	/* size */

	/* buf1 raw memory addresses */
	dma_addr_t		map_dma_f2;	/* physical */
	u_char *			map_cpu_f2;	/* virtual */
	unsigned int		map_size_f2;	/* size */

	/* buf2 raw memory addresses */
	dma_addr_t		map_dma_f3;	/* physical */
	u_char *			map_cpu_f3;	/* virtual */
	UINT32			map_size_f3;	/* size */

	/* buf3 raw memory addresses */
	dma_addr_t		map_dma_f4;	/* physical */
	u_char *			map_cpu_f4;	/* virtual */
	UINT32			map_size_f4;	/* size */

	/* keep these registers in case we need to re-write palette */
	UINT32			palette_buffer[256];	/* real palette buffer */
	UINT32			pseudo_pal[256];		/* pseudo palette buffer */

	void *fbhandle;
} imapfb_info_t;

typedef struct {
	/* Screen physical resolution */
	UINT32 xres;
	UINT32 yres;

	/* OSD Screen info */
	UINT32 osd_xres;			/* Visual OSD x resolution */
	UINT32 osd_yres;			/* Visual OSD y resolution */
	UINT32 osd_xres_virtual;	/* Virtual OSD x resolution */
	UINT32 osd_yres_virtual;	/* Virtual OSD y resolution */
	UINT32 osd_xoffset;		/* Visual to Virtual OSD x offset */
	UINT32 osd_yoffset;		/* Visual to Virtual OSD y offset */

	UINT32 bpp;				/* Bits per pixel */
	UINT32 bytes_per_pixel;	/* Bytes per pixel for true color */
	UINT32 pixclock;			/* Pixel clock */

	UINT32 hsync_len;		/* Horizontal sync length */
	UINT32 left_margin;		/* Horizontal back porch */
	UINT32 right_margin;		/* Horizontal front porch */
	UINT32 vsync_len;		/* Vertical sync length */
	UINT32 upper_margin;	/* Vertical back porch */
	UINT32 lower_margin;	/* Vertical front porch */
	UINT32 sync;				/* whether sync signal is used or not */

	UINT32 cmap_grayscale:1;
	UINT32 cmap_inverse:1;
	UINT32 cmap_static:1;
	UINT32 unused:29;

	UINT32 pixfmt;
}imapfb_fimd_info_t;

#endif


