/***************************************************************
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:  Make sure that this file coudl only be included in dsi_api.c
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
**     2012/10/10 :  sam@ fist commit
**                   Functions : 
**                       i)  Define DSI type IDs.
**                       ii) Register new dsi screen added.
**                   
****************************************************************/

#ifndef _DSI_CFG_H_
#define _DSI_CFG_H_

#define DSITYPE_VVX10F004B00_1920x1200				(1)

extern struct DSI_Priv_conf VVX10F004B00_PRIV_CONF;


dispdev_cfg_t gDSIConf[] = {
    {
        "VVX10F004B00_1920x1200",
        DSITYPE_VVX10F004B00_1920x1200,
        (void*)&VVX10F004B00_PRIV_CONF,
    },
    {
        "none", // end of gDSIConf
        0,
        IM_NULL,
    }
};


#endif // _DSI_CFG_H_

