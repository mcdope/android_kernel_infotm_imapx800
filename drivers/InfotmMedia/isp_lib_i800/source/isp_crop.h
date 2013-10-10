/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_crop.h
--
--  Description :
--		crop module.
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

#ifndef _ISP_CROP_H_
#define	_ISP_CROP_H_


typedef struct{
	IM_BOOL					enable;
	isp_crop_coordinate_t	coordinate;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_crop_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET crop_init(isp_crop_context_t *crop, isp_crop_config_t *cfg);

IM_RET crop_set_coordinate(isp_crop_context_t *crop, isp_crop_coordinate_t *coordinate);
IM_RET crop_set_enable(isp_crop_context_t *crop);
IM_RET crop_set_disable(isp_crop_context_t *crop);


#endif	//_ISP_CROP_H_
