/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_cmncsc.h
--
--  Description :
--		common colorspace conversion module.
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

#ifndef _ISP_CMNCSC_H_
#define	_ISP_CMNCSC_H_

typedef struct{
	IM_BOOL				enable;
	IM_BOOL				modeNeedChange;
	IM_UINT32			mode;	
	IM_UINT32			*regVal;
	IM_UINT32			*regOfst;
}isp_cmncsc_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET cmncsc_init(isp_cmncsc_context_t *cmncsc, isp_cmncsc_config_t *cfg);

IM_RET cmncsc_set_mode(isp_cmncsc_context_t *cmncsc, IM_UINT32 mode);	//mode = ISP_CMNCSC_XXX
IM_RET cmncsc_set_enable(isp_cmncsc_context_t *cmncsc);
IM_RET cmncsc_set_disable(isp_cmncsc_context_t *cmncsc);


#endif	//_ISP_CMNCSC_H_
