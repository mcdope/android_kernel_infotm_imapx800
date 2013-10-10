/***************************************************************
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:  Make sure that this file coudl only be included in lcd_api.c
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
**     2012/09/11 :  sam@ fist commit
**                   Functions : 
**                       i)  Define LCD type IDs.
**                       ii) Register new lcd screen added.
**                   
****************************************************************/

#ifndef _LCD_CFG_H_
#define _LCD_CFG_H_

#define LCDTYPE_CLAP070NJ02CW_1024x600      (1)
#if 0
#define LCDTYPE_HSD070IDW1_800x480          (2)
#define LCDTYPE_BF097XN_1024x768            (3)
#define LCDTYPE_BF097XN01_1024x768          (4)
#define LCDTYPE_LXJC070NDM350_18C2_800x480  (5)
#define LCDTYPE_CRD080TN01_1024x768         (6)
#define LCDTYPE_LY7060BD_800x480            (7)
#define LCDTYPE_M710C_1024x600              (8)
#define LCDTYPE_RS3WSN70040A_1024x600       (9)
#define LCDTYPE_T070SWS057H_1024x600        (10)
#define LCDTYPE_T70P03_1024x600             (11)
#define LCDTYPE_TM097TDH01_1024x768         (12)
#define LCDTYPE_W2_1024x600                 (13)
#define LCDTYPE_RS3WSN70040A_BiNai_1024x600 (14)
#define LCDTYPE_CLAA070WP03XG_1024x600		(15)
#define LCDTYPE_KR080PC1S_800x600           (16)
#define LCDTYPE_SHUOYING_N1066_1024X600	    (17)
#define LCDTYPE_KR070PA6S_800x480	        (18)
#define LCDTYPE_HSD101_1280x800             (19)
#define LCDTYPE_HSD101PWW1_1280x800         (20)
#define LCDTYPE_TM097TDH01V2_1024x768         (21)
#define LCDTYPE_SL008DC21B01_800x600         (22)
#define LCDTYPE_SL008DH24B01_1024x768         (23)
#define LCDTYPE_LP133WH2_1366x768	    	(24)
#endif 
#define LCDTYPE_AT070TN93V2_800x480         (2)
#define LCDTYPE_KX0705001A0_800x480         (3)

extern struct LCD_Priv_conf CLAP070NJ02CW_PRIV_CONF;
extern struct LCD_Priv_conf AT070TN93V2_PRIV_CONF;
extern struct LCD_Priv_conf KX0705001A0_PRIV_CONF;
#if 0
extern struct LCD_Priv_conf HSD070IDW1_PRIV_CONF;
extern struct LCD_Priv_conf BF097XN_PRIV_CONF;
extern struct LCD_Priv_conf BF097XN01_PRIV_CONF;
extern struct LCD_Priv_conf LXJC070NDM350_PRIV_CONF;
extern struct LCD_Priv_conf CRD080TN01_PRIV_CONF;
extern struct LCD_Priv_conf LY7060BD_PRIV_CONF;
extern struct LCD_Priv_conf M710C_PRIV_CONF;
extern struct LCD_Priv_conf RS3WSN70040A_PRIV_CONF;
extern struct LCD_Priv_conf T070SWS057H_PRIV_CONF;
extern struct LCD_Priv_conf T70P03_PRIV_CONF;
extern struct LCD_Priv_conf TM097TDH01_PRIV_CONF;
extern struct LCD_Priv_conf W2_PRIV_CONF;
extern struct LCD_Priv_conf RS3WSN70040A_BiNai_PRIV_CONF;
extern struct LCD_Priv_conf CLAA070WP03XG_PRIV_CONF;
extern struct LCD_Priv_conf KR080PC1S_PRIV_CONF;
extern struct LCD_Priv_conf SHUOYING_N1066_PRIV_CONF;
extern struct LCD_Priv_conf KR070PA6S_PRIV_CONF;
extern struct LCD_Priv_conf HSD101_PRIV_CONF;
extern struct LCD_Priv_conf HSD101PWW1_PRIV_CONF;
extern struct LCD_Priv_conf TM097TDH01V2_PRIV_CONF;
extern struct LCD_Priv_conf SL008DC21B01_PRIV_CONF;
extern struct LCD_Priv_conf SL008DH24B01_PRIV_CONF;
extern struct LCD_Priv_conf LP133WH2_PRIV_CONF;
#endif


dispdev_cfg_t gLCDConf[] = {
     {
        "CLAP070NJ02CW_1024x600",
        LCDTYPE_CLAP070NJ02CW_1024x600,
        (void*)&CLAP070NJ02CW_PRIV_CONF,
    }, 
#if 0
   {
        "HSD070IDW1_800x480",
        LCDTYPE_HSD070IDW1_800x480,
        (void*)&HSD070IDW1_PRIV_CONF,
    },

    {
        "BF097XN_1024x768",
        LCDTYPE_BF097XN_1024x768,
        (void*)&BF097XN_PRIV_CONF,
    },
    {
        "BF097XN01_1024x768",
        LCDTYPE_BF097XN01_1024x768,
        (void*)&BF097XN01_PRIV_CONF,
    },
    {
        "LXJC070NDM350_18C2_800x480",
        LCDTYPE_LXJC070NDM350_18C2_800x480,
        (void*)&LXJC070NDM350_PRIV_CONF,
    },
    {
        "CRD080TN01_1024x768",
        LCDTYPE_CRD080TN01_1024x768,
        (void*)&CRD080TN01_PRIV_CONF,
    },
    {
        "LY7060BD_800x480",
        LCDTYPE_LY7060BD_800x480,
        (void*)&LY7060BD_PRIV_CONF,
    },
    {
        "M710C_1024x600",
        LCDTYPE_M710C_1024x600,
        (void*)&M710C_PRIV_CONF,
    },
    {
        "RS3WSN70040A_1024x600",
        LCDTYPE_RS3WSN70040A_1024x600,
        (void*)&RS3WSN70040A_PRIV_CONF,
    },
    {
        "T070SWS057H_1024x600",
        LCDTYPE_T070SWS057H_1024x600,
        (void*)&T070SWS057H_PRIV_CONF,
    },
    {
        "T70P03_1024x600",
        LCDTYPE_T70P03_1024x600,
        (void*)&T70P03_PRIV_CONF,
    },
    {
        "TM097TDH01_1024x768",
        LCDTYPE_TM097TDH01_1024x768,
        (void*)&TM097TDH01_PRIV_CONF,
    },
    {
        "W2_1024x600",
        LCDTYPE_W2_1024x600,
        (void*)&W2_PRIV_CONF,
    },
    {
        "RS3WSN70040A_BiNai_1024x600",
        LCDTYPE_RS3WSN70040A_BiNai_1024x600,
        (void*)&RS3WSN70040A_BiNai_PRIV_CONF,
    },
    {
        "CLAA070WP03XG_1024x600",
        LCDTYPE_CLAA070WP03XG_1024x600,
        (void*)&CLAA070WP03XG_PRIV_CONF,
    },
    {
        "KR080PC1S_800x600",
        LCDTYPE_KR080PC1S_800x600,
        (void*)&KR080PC1S_PRIV_CONF,
    },
    {
        "SHUOYING_N1066_1024X600",
        LCDTYPE_SHUOYING_N1066_1024X600,
        (void*)&SHUOYING_N1066_PRIV_CONF,
    },
    {
        "KR070PA6S_800x480",
        LCDTYPE_KR070PA6S_800x480,
        (void*)&KR070PA6S_PRIV_CONF,
    },
    {
        "HSD101_1280x800",
        LCDTYPE_HSD101_1280x800,
        (void*)&HSD101_PRIV_CONF,
    },
    {
        "HSD101PWW1_1280x800",
        LCDTYPE_HSD101PWW1_1280x800,
        (void*)&HSD101PWW1_PRIV_CONF,
    },
    {
        "TM097TDH01V2_1024x768",
        LCDTYPE_TM097TDH01V2_1024x768,
        (void*)&TM097TDH01V2_PRIV_CONF,
    },
    {
        "SL008DC21B01_800x600",
        LCDTYPE_SL008DC21B01_800x600,
        (void*)&SL008DC21B01_PRIV_CONF,
    },
    {
        "SL008DH24B01_1024x768",
        LCDTYPE_SL008DH24B01_1024x768,
        (void*)&SL008DH24B01_PRIV_CONF,
    },
	{
		"LP133WH2_1366x768",
		LCDTYPE_LP133WH2_1366x768,
		(void*)&LP133WH2_PRIV_CONF,
	},	
#endif
    {
        "KX0705001A0_800x480",
        LCDTYPE_KX0705001A0_800x480,
        (void*)&KX0705001A0_PRIV_CONF,
    },

    {
        "AT070TN93V2_800x480",
        LCDTYPE_AT070TN93V2_800x480,
        (void*)&AT070TN93V2_PRIV_CONF,
    },
    {
        "none", // end of LCDConf
        0,
        IM_NULL,
    }
};


#endif // _LCD_CFG_H_
