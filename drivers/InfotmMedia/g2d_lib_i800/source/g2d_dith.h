/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_dith.h
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __G2D_DITH_H__
#define __G2D_DITH_H__

IM_RET dith_set_config(DITHER *dith, G2D_DITHER *dithCfg);
IM_RET dith_set_size(DITHER *dith, IM_UINT32 width, IM_UINT32 height);
IM_RET dith_set_channel_bits_reduce(DITHER *dith, IM_UINT32 rChannel, IM_UINT32 gChannel, IM_UINT32 bChannel);
IM_RET dith_set_coef_type(DITHER *dith, IM_UINT32 coefType);	//coefType = G2D_DITH_COEF_TYPE_XXX
IM_RET dith_set_tempo_enable(DITHER *dith, IM_BOOL tempoEn);
IM_RET dith_set_enable(DITHER *dith, IM_BOOL dithEn);


#endif	// __G2D_DITH_H__

