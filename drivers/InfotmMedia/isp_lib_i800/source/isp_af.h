/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_af.h
--
--  Description :
--		af module.
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

#ifndef _ISP_AF_H_
#define	_ISP_AF_H_


typedef struct{
	IM_BOOL					enable;
	IM_UINT32				type;	//ISP_AF_TYPE_XXX
	isp_af_coordinate_t		coordinate;
	isp_af_result_t			rsut;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_af_context_t;



/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET af_init(isp_af_context_t *af, isp_af_config_t *cfg);

IM_RET af_set_type(isp_af_context_t *af, IM_UINT32 type);
IM_RET af_set_block_coordinate(isp_af_context_t *af, isp_af_coordinate_t *coordinate);
IM_RET af_get_result(isp_af_context_t *af, isp_af_result_t *rsut);
IM_RET af_set_enable(isp_af_context_t *af);
IM_RET af_set_disable(isp_af_context_t *af);


#endif	//_ISP_AF_H_
