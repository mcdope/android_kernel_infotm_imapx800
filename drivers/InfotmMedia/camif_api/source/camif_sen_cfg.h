/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camif_sensor_config.h
--
--  Description :
--		config file to isp for each sensor	
--
--	Author:
--  	Jimmy Shu   <jimmy.shu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	jimmy@2012/10/16: first commit.
--
------------------------------------------------------------------------------*/

#define	PRO_SPE_EFT_NONE				0
#define	PRO_SPE_EFT_MONO				1
#define	PRO_SPE_EFT_NEGATIVE			2
#define	PRO_SPE_EFT_SOLARIZE			3
#define	PRO_SPE_EFT_PASTEL				4
#define	PRO_SPE_EFT_MOSAIC				5
#define	PRO_SPE_EFT_RESIZE				6
#define	PRO_SPE_EFT_SEPIA				7
#define	PRO_SPE_EFT_POSTERIZE			8
#define	PRO_SPE_EFT_WHITEBOARD			9
#define	PRO_SPE_EFT_BLACKBOARD			10
#define	PRO_SPE_EFT_AQUA				11
//#define	PRO_SPE_EFT_XXX				12
//...
//#define	PRO_SPE_EFT_LAST			19	//8 type reserve for other special effect maybe support in the future
#define	PRO_SPE_EFT_OFFSET				PRO_SPE_EFT_NONE

#define PRO_SCE_MODE_AUTO				20
#define PRO_SCE_MODE_ACTION				21
#define PRO_SCE_MODE_PORTRAIT			22
#define PRO_SCE_MODE_LANDSCAPE			23
#define PRO_SCE_MODE_NIGHT				24
#define PRO_SCE_MODE_NIGHT_PORTRAIT		25
#define PRO_SCE_MODE_THEATRE			26
#define PRO_SCE_MODE_BEACH				27
#define PRO_SCE_MODE_SNOW				28
#define PRO_SCE_MODE_SUNSET				29
#define PRO_SCE_MODE_STEADYPHOTO		30
#define PRO_SCE_MODE_FIREWORKS			31
#define PRO_SCE_MODE_SPORTS				32
#define PRO_SCE_MODE_PARTY				33
#define PRO_SCE_MODE_CANDLELIGHT		34
#define PRO_SCE_MODE_BARCODE 			35
//#define	PRO_SCE_MODE_XXX			36
//...
//#define	PRO_SCE_MODE_LAST			39	//4 type reserve for other scene mode maybe support in the future
#define PRO_SCE_MODE_OFFSET				PRO_SCE_MODE_AUTO

#define MAX_PRO (50)


typedef struct{
	IM_UINT32 request;
	IM_UINT32 supply;
}camif_resolution_caps_t;

typedef struct{
	IM_CHAR					senName[256];
	IM_INT32				flag;
	camif_resolution_caps_t	resCaps[CAM_RES_ENUM_MAX];
	cam_preview_config_t 	preCfg;
}camif_sensor_init_t;


