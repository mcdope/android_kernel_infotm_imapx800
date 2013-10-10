/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_gma.h
--
--  Description :
--		gma module.
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

#ifndef _ISP_GMA_H_
#define	_ISP_GMA_H_


typedef struct{
	IM_BOOL		enable;
	IM_BOOL		modeNeedChange;
	IM_UINT32	mode;	//ISP_GMA_MODE_X
	IM_UINT32	rdMode;	//ISP_ROUND_XXX
	IM_UINT32	*regVal;
	IM_UINT32	*regOfst;
}isp_gma_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET gma_init(isp_gma_context_t *gma, isp_gma_config_t *cfg);

IM_RET gma_set_mode(isp_gma_context_t *gma, IM_UINT32 mode);		//mode = ISP_GMA_MODE_X
IM_RET gma_set_round_mode(isp_gma_context_t *gma, IM_UINT32 rdMode);	//round = ISP_ROUND_XXX
IM_RET gma_set_enable(isp_gma_context_t *gma);
IM_RET gma_set_disable(isp_gma_context_t *gma);


#endif	//_ISP_GMA_H_
