/***************************************************************
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:  Make sure that this file could only be included in i80_api.c
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
**     2012/09/11 :  sam@ fist commit
**                   Functions : 
**                       i)  Define I80 type IDs.
**                       ii) Register new i80 screen added.
**                   
****************************************************************/

#ifndef _I80_CFG_H_
#define _I80_CFG_H_

#define I80TYPE_ILI9320_800x480          (1)
#define I80TYPE_HX834724CO_1024x600      (2)


extern struct I80_Priv_conf ILI9320_PRIV_CONF;
extern struct I80_Priv_conf HX834724CO_PRIV_CONF;


dispdev_cfg_t gI80Conf[] = {
    {
        "ILI9320_800x480",
        I80TYPE_ILI9320_800x480,
        (void*)&ILI9320_PRIV_CONF,
    },
    {
        "HX834724CO_1024x600",
        I80TYPE_HX834724CO_1024x600,
        (void*)&HX834724CO_PRIV_CONF,
    }, 
	{
		"none", // end of I80Conf
		0,
		IM_NULL,
	}
};


#endif // _I80_CFG_H_

