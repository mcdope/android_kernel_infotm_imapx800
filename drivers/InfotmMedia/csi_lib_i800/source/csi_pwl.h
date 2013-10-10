/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: csi_pwl.h
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/09/05: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __CSI_PWL_H__
#define __CSI_PWL_H__

//==============================interface======================================
IM_RET csipwl_init(void);
IM_RET csipwl_deinit(void);

IM_RET csipwl_write_reg(IM_UINT32 offset, IM_UINT32 value);

IM_RET csipwl_read_reg(IM_UINT32 offset, IM_UINT32 *value);

#endif	// __CSI_PWL_H__

