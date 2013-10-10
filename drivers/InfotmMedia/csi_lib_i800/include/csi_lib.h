/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: csi_lib.h
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

#ifndef _CSI_LIB_H_
#define _CSI_LIB_H_


/*------------------------------------------------------------------------------
    Function prototypes                                                         
------------------------------------------------------------------------------*/
IM_RET csilib_init(IM_INT32 lanes, IM_INT32 freq);
IM_RET csilib_deinit(void);
IM_RET csilib_open(void);
IM_RET csilib_close(void);
IM_RET csilib_set_frequency(IM_INT32 freq);
IM_UINT32 csilib_get_state();


#endif /* _CSI_LIB_H_ */
