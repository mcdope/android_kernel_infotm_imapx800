/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_ae.h
--
--  Description :
--		ae module.
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

#ifndef _ISP_AE_H_
#define	_ISP_AE_H_


typedef struct{
	IM_BOOL					enable;
	isp_ae_block_select		blokSelect;
	isp_ae_coordinate_t		coordinate;
	isp_ae_result_t			rsut;
	IM_UINT32				*regVal;
	IM_UINT32				*regOfst;
}isp_ae_context_t;


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/
IM_RET ae_init(isp_ae_context_t *ae, isp_ae_config_t *cfg);

IM_RET ae_set_block_select(isp_ae_context_t *ae, isp_ae_block_select *blokSelect);
IM_RET ae_set_block_coordinate(isp_ae_context_t *ae, isp_ae_coordinate_t *coordinate);
IM_RET ae_get_result(isp_ae_context_t *ae, isp_ae_result_t *rsut);
IM_RET ae_set_enable(isp_ae_context_t *ae);
IM_RET ae_set_disable(isp_ae_context_t *ae);


#endif	//_ISP_AE_H_
