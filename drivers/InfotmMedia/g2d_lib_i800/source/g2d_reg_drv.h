/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_reg_drv.h
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

#ifndef _G2D_REG_DRV_H
#define _G2D_REG_DRV_H

/*------------------------------------------------------------------------------ 
    Function prototypes                                                          
------------------------------------------------------------------------------*/ 
                                                                                 
/*id:G2D_XXX*/                                                                   
void SetG2DRegister(IM_UINT32 * regBase, IM_UINT32 id, IM_INT32 value);         
IM_UINT32 GetG2DRegister(const IM_UINT32 * regBase, IM_UINT32 id);
                                                                                 
#endif /* #ifndef _G2D_REG_DRV_H */
                                                 
