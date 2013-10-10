/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --

-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: g2d_drv.h
--
--  Description :
--		Head file IMAPX800 G2D driver process.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8.
--
------------------------------------------------------------------------------*/

#ifndef __IMAPX800_G2D_DRV_H__
#define __IMAPX800_G2D_DRV_H__


typedef struct  {
	void	*g2dInst;
	int		taskId;
}g2ddrv_context_t;

//for kernel app call g2d draw operator
/*typedef struct  {
	IM_UINT32 type;
	...
}g2d_draw_config_t;*/ 

#endif  /* __IMAPX800_G2D_DRV_H_ */
