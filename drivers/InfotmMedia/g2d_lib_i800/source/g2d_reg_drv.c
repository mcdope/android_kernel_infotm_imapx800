/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_reg_drv.c
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

#include <InfotmMedia.h>
#include <g2d_lib.h>
#include "g2d.h"
#include "g2d_reg_drv.h"

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
*  g2d register bits field info  *
*   { REGNUM, BITS, POSITION }   *
*********************************/
static const IM_UINT32 g2dRegSpec[G2D_LAST_REG_ID + 1][3] = {
#include "g2d_reg_bitfield_table.h"
};

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: SetG2DRegister

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
void SetG2DRegister(IM_UINT32 * regBase, IM_UINT32 id, IM_INT32 value)
{

    IM_UINT32 tmp;

    IM_ASSERT(id <= G2D_LAST_REG_ID);

    tmp = regBase[g2dRegSpec[id][0]];
    tmp &= ~(regMask[g2dRegSpec[id][1]] << g2dRegSpec[id][2]);
    tmp |= (value & regMask[g2dRegSpec[id][1]]) << g2dRegSpec[id][2];
    regBase[g2dRegSpec[id][0]] = tmp;

}

/*------------------------------------------------------------------------------

    Function name: GetG2DRegister

        Functional description: get uint32 value

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_UINT32 GetG2DRegister(const IM_UINT32 * regBase, IM_UINT32 id)
{

    IM_UINT32 tmp;

    IM_ASSERT(id <= G2D_LAST_REG_ID);

    tmp = regBase[g2dRegSpec[id][0]];
    tmp = tmp >> g2dRegSpec[id][2];
    tmp &= regMask[g2dRegSpec[id][1]];
    return (tmp);

}


