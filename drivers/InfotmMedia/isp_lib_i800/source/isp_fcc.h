/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_fcc.h
--
--  Description :
--		fcc module.
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

#ifndef _ISP_FCC_H_
#define	_ISP_FCC_H_

typedef struct{
	IM_BOOL		enable;
	IM_UINT32	threshold;
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_fcc_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET fcc_init(isp_fcc_context_t *fcc, isp_fcc_config_t *cfg);

IM_RET fcc_set_threshold(isp_fcc_context_t *fcc, IM_UINT32 threshold);
IM_RET fcc_set_enable(isp_fcc_context_t *fcc);
IM_RET fcc_set_disable(isp_fcc_context_t *fcc);


#endif	//_ISP_FCC_H_
