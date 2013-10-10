/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_reg_drv.h
--
--  Description :
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


#ifndef _ISP_REG_DRV_H
#define _ISP_REG_DRV_H

#include <InfotmMedia.h>
#include "isp_common.h"
/*------------------------------------------------------------------------------ 
    Function prototypes                                                          
------------------------------------------------------------------------------*/ 
                                                                                 
/*id:ISP_XXX*/                                                                   
void SetIspRegister(IM_UINT32 * regBase, IM_UINT32 id, IM_INT32 value);         
IM_UINT32 GetIspRegister(const IM_UINT32 * regBase, IM_UINT32 id);
                                                                                 
#endif /* #ifndef _ISP_REG_DRV_H */
                                                 
