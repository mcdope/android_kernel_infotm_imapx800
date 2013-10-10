/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_reg_drv.c
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

#include "isp_common.h"
#include "isp_reg_drv.h"

static const IM_UINT32 regMask[33] = { 0x00000000,
    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
    0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
    0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
    0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
    0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
    0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
    0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
    0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

/*********************************
*    isp register bits info      *
*   { REGNUM, BITS, POSITION }   *
*********************************/
static const IM_UINT32 ispRegSpec[ISP_LAST_REG_ID + 1][3] = {
#include "ispreg_id_table.h"
};

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: SetIspRegister

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
void SetIspRegister(IM_UINT32 * regBase, IM_UINT32 id, IM_INT32 value)
{

    IM_UINT32 tmp;

    IM_ASSERT(id <= ISP_LAST_REG_ID);

    tmp = regBase[ispRegSpec[id][0]];
    tmp &= ~(regMask[ispRegSpec[id][1]] << ispRegSpec[id][2]);
    tmp |= (value & regMask[ispRegSpec[id][1]]) << ispRegSpec[id][2];
    regBase[ispRegSpec[id][0]] = tmp;

}

/*------------------------------------------------------------------------------

    Function name: GetIspRegister

        Functional description: get uint32 value

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_UINT32 GetIspRegister(const IM_UINT32 * regBase, IM_UINT32 id)
{

    IM_UINT32 tmp;

    IM_ASSERT(id <= ISP_LAST_REG_ID);

    tmp = regBase[ispRegSpec[id][0]];
    tmp = tmp >> ispRegSpec[id][2];
    tmp &= regMask[ispRegSpec[id][1]];
    return (tmp);

}

