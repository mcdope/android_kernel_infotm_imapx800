/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_osd.h
--
--  Description :
--		osd module.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __ISP_OSD_H__
#define __ISP_OSD_H__

//#############################################################################
//osd palette table offset
#define ISP_OSD_PAL_TABLE	0x8400


// window 0.
typedef struct {
	IM_BOOL					enable;	
	isp_osd_mapcolor_t		mapclr;		//window0 not support mapcolor, so it is useless
	isp_osd_coordinate_t	coordinate;
}osd_wnd0_context_t;


// window 1.
typedef struct {
	IM_BOOL					enable;
	IM_UINT32				imgFormat;	// ISP_OSD_IMAGE_xxx.
	isp_osd_palette_t		palette;
	isp_osd_swap_t			swap;
	isp_osd_alpha_t			alpha;
	isp_osd_mapcolor_t		mapclr;
	isp_osd_colorkey_t		clrkey;
	isp_osd_vm_t			vm;
	isp_osd_coordinate_t	coordinate;
	isp_osd_buffer_mode 	bm;
	isp_osd_buffer_t		buffer;
	IM_UINT32				imageSize;
	IM_UINT32				buffOffset;
	IM_UINT32				bitBuffOffset;
}osd_wnd1_context_t;


typedef struct {
	IM_UINT32				enable;
	IM_UINT32 				bgColor;	// background color of osd, format is RGB888. 
	IM_UINT32				outWidth;	// range 1--4096
	IM_UINT32				outHeight;	// range 1--4096
	osd_wnd0_context_t		wnd0;
	osd_wnd1_context_t		wnd1;
}osd_config_t;


typedef struct {
	IM_UINT32				enable;
	IM_UINT32 				bgColor;	// background color of osd, format is RGB888. 
	IM_UINT32				outWidth;	// range 1--4096
	IM_UINT32				outHeight;	// range 1--4096
	osd_wnd0_context_t		wnd0;
	osd_wnd1_context_t		wnd1;

	IM_BOOL 				enableRefetch;
	IM_BOOL 				enableRelax;
	IM_UINT32 				refetchWaitTime;	// range 0--15
	IM_BOOL 				enableSafeBand;
	IM_BOOL 				enableAllFetch;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_osd_context_t;

//==============================interface=====================================
//osd wndow0
IM_RET osd_wnd0_init(isp_osd_context_t *osd, osd_wnd0_context_t *wnd0);
IM_RET osd_wnd0_deinit(isp_osd_context_t *osd);
IM_RET osd_wnd0_set_mapcolor(isp_osd_context_t *osd, isp_osd_mapcolor_t *map);
IM_RET osd_wnd0_set_coordinate(isp_osd_context_t *osd, isp_osd_coordinate_t *coordinate);
IM_RET osd_wnd0_set_enable(isp_osd_context_t *osd);
IM_RET osd_wnd0_set_disable(isp_osd_context_t *osd);

//osd wndow1
IM_RET osd_wnd1_init(isp_osd_context_t *osd, osd_wnd1_context_t *wnd1);
IM_RET osd_wnd1_deinit(isp_osd_context_t *osd);
IM_RET osd_wnd1_set_swap(isp_osd_context_t *osd, isp_osd_swap_t *swap);
IM_RET osd_wnd1_set_alpha(isp_osd_context_t *osd, isp_osd_alpha_t *alpha);
IM_RET osd_wnd1_set_format(isp_osd_context_t *osd,  IM_UINT32 imgFormat);
IM_RET osd_wnd1_set_palette_format(isp_osd_context_t *osd, IM_UINT32 imgFormat, isp_osd_palette_t *palette);
IM_RET osd_wnd1_set_mapcolor(isp_osd_context_t *osd, isp_osd_mapcolor_t *map);
IM_RET osd_wnd1_set_colorkey(isp_osd_context_t *osd, isp_osd_colorkey_t *clrkey);
IM_RET osd_wnd1_set_buffer_mode(isp_osd_context_t *osd,  isp_osd_buffer_mode *bm);
IM_RET osd_wnd1_set_buffers(isp_osd_context_t *osd, isp_osd_buffer_t *buffer);
IM_RET osd_wnd1_set_virtual_window(isp_osd_context_t *osd, isp_osd_vm_t *vm);
IM_RET osd_wnd1_set_coordinate(isp_osd_context_t *osd, isp_osd_coordinate_t *coordinate);
IM_RET osd_wnd1_set_enable(isp_osd_context_t *osd);
IM_RET osd_wnd1_set_disable(isp_osd_context_t *osd);

//osd global
IM_RET osd_init(isp_osd_context_t *osd, osd_config_t *cfg);
IM_RET osd_deinit(isp_osd_context_t *osd);
IM_RET osd_set_background_color(isp_osd_context_t *osd, IM_UINT32 bgColor);
IM_RET osd_set_out_size(isp_osd_context_t *osd, IM_UINT32 width, IM_UINT32 height);
IM_RET osd_set_enable(isp_osd_context_t *osd);
IM_RET osd_set_disable(isp_osd_context_t *osd);


//#############################################################################
#endif	// __ISP_OSD_H__

