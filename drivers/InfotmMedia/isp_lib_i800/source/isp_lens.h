/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_lens.h
--
--  Description :
--		lens module.
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

#ifndef _ISP_LENS_H_
#define	_ISP_LENS_H_


//lens shading LOOKUP TABLE offset
#define ISP_LENS_LUTB0	0x4000  //R even(17*9)
#define ISP_LENS_LUTB1	0x4400	//R odd	(17*8)
#define ISP_LENS_LUTB2	0x4800	//G even(17*9)
#define ISP_LENS_LUTB3	0x4c00	//G odd	(17*8)
#define ISP_LENS_LUTB4	0x5000	//B even(17*9)
#define ISP_LENS_LUTB5	0x5400	//B odd	(17*8)

#define	ISP_LENS_LUTB_LENGTH	153	//(17*9)

typedef struct{
	IM_BOOL		enable;
	IM_BOOL		lutbModeNeedChange;
	IM_UINT32	lutbMode;	//ISP_LENS_LUTB_MODE_X
	IM_UINT32	height;
	IM_UINT32	width;
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_lens_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET lens_init(isp_lens_context_t *lens, isp_lens_config_t *cfg);

IM_RET lens_set_lutb(isp_lens_context_t *lens, IM_UINT32 lutbMode);
IM_RET lens_set_enable(isp_lens_context_t *lens);
IM_RET lens_set_disable(isp_lens_context_t *lens);


#endif	//_ISP_LENS_H_
