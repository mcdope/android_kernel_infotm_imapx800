/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_lib.h
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
-- v1.0.2	arsor@2012/07/20: add ee setting detect threshold martix api.
--
------------------------------------------------------------------------------*/

#ifndef _ISP_LIB_H_
#define _ISP_LIB_H_

//isp intf mode
#define	ISP_INTFMODE_IO_RAWRGB			0	//RAW RGB

#define	ISP_INTFMODE_MIPI_RAWRGB		1	//mipi interface, CSI RAWRGB
#define	ISP_INTFMODE_MIPI_YUV			3	//mipi interface, CSI YUV
#define	ISP_INTFMODE_MIPI_RGB			3	//mipi interface, CSI RGB

#define	ISP_INTFMODE_ITU_RGB16BIT		4	//ITU(CAMIF), RGB 16 BIT
#define	ISP_INTFMODE_ITU_RGB888			5	//ITU(CAMIF), RGB888
#define	ISP_INTFMODE_ITU_YUV422			6	//ITU(CAMIF), YUV422
#define	ISP_INTFMODE_ITU_YUV444			7	//ITU(CAMIF), YUV444


//input bits 
#define	ISP_INPUT_LOW_BITS_8		0
#define	ISP_INPUT_LOW_BITS_10		1
#define	ISP_INPUT_MID_BITS_8		2
#define	ISP_INPUT_BITS_12			3


//rawdata mode
#define	ISP_RAWDATAMODE_RGRG	0
#define	ISP_RAWDATAMODE_BGBG	1
#define	ISP_RAWDATAMODE_GRGR	2
#define	ISP_RAWDATAMODE_GBGB	3

#define	ISP_RAWDATAMODE_RGGB	ISP_RAWDATAMODE_RGRG
#define	ISP_RAWDATAMODE_BGGR	ISP_RAWDATAMODE_BGBG
#define	ISP_RAWDATAMODE_GRBG	ISP_RAWDATAMODE_GRGR
#define	ISP_RAWDATAMODE_GBRG	ISP_RAWDATAMODE_GBGB

//
//ITU TYPE
//
//ITU scanmode
#define	ISP_ITU_SCAN_PROGRESSIVE	0
#define	ISP_ITU_SCAN_INTERLACED		1

//ITU foramt
#define	ISP_ITU_FORMAT_ITU656		0
#define	ISP_ITU_FORMAT_ITU601		1

//ITU order
//YUV422
#define	ISP_ITU_ORDER_YUV422_UYVY			0
#define	ISP_ITU_ORDER_YUV422_VYUY			1
#define	ISP_ITU_ORDER_YUV422_YUYV			2
#define	ISP_ITU_ORDER_YUV422_YVYU			3

//YUV444
#define	ISP_ITU_ORDER_YUV444_YUV			0
#define	ISP_ITU_ORDER_YUV444_YVU			1
#define	ISP_ITU_ORDER_YUV444_UYV			2
#define	ISP_ITU_ORDER_YUV444_UVY			3
#define	ISP_ITU_ORDER_YUV444_VYU			4
#define	ISP_ITU_ORDER_YUV444_VUY			5

//RGB888
#define	ISP_ITU_ORDER_RGB888_RGB			0
#define	ISP_ITU_ORDER_RGB888_RBG			1
#define	ISP_ITU_ORDER_RGB888_GRB			2
#define	ISP_ITU_ORDER_RGB888_GBR			3
#define	ISP_ITU_ORDER_RGB888_BRG			4
#define	ISP_ITU_ORDER_RGB888_BGR			5

//RGB 16BIT
#define	ISP_ITU_ORDER_RGB16BIT_RGB565		0
#define	ISP_ITU_ORDER_RGB16BIT_RGB555		1
#define	ISP_ITU_ORDER_RGB16BIT_RGB444x		2
#define	ISP_ITU_ORDER_RGB16BIT_RGBx444		3


//
// ping-pong buffer mode
//
#define ISP_PPMODE_DISABLE	0	// user provide buff[0], internal only use buff[0]
#define ISP_PPMODE_1_BUFFER	1	// user provide buff[0], internal buff[1--3] = buff[0]
#define ISP_PPMODE_2_BUFFER	2	// user provide buff[0] and buff[1], internal buff[2] = buff[0], buff[3] = buff[1]
#define ISP_PPMODE_4_BUFFER	4	// user provide buff[0], buff[1], buff[2], buff[3]

//round mode
#define	ISP_ROUND_MINUS		0
#define	ISP_ROUND_NEAREST	1

/*********************************                    
*    	define isp awb mode      *                    
*********************************/
#define	ISP_WB_MODE_NORMAL					0                      
#define	ISP_WB_MODE_CLRTEM_2500				1  					               
#define	ISP_WB_MODE_CLRTEM_2600				2               
#define	ISP_WB_MODE_CLRTEM_2700				3               
#define	ISP_WB_MODE_CLRTEM_2800				4               
#define	ISP_WB_MODE_CLRTEM_2900				5               
#define	ISP_WB_MODE_CLRTEM_3000				6               
#define	ISP_WB_MODE_CLRTEM_3100				7               
#define	ISP_WB_MODE_CLRTEM_3200				8               
#define	ISP_WB_MODE_CLRTEM_3300				9               
#define	ISP_WB_MODE_CLRTEM_3400				10              
#define	ISP_WB_MODE_CLRTEM_3500				11              
#define	ISP_WB_MODE_CLRTEM_3600				12              
#define	ISP_WB_MODE_CLRTEM_3700				13              
#define	ISP_WB_MODE_CLRTEM_3800				14              
#define	ISP_WB_MODE_CLRTEM_3900				15              
#define	ISP_WB_MODE_CLRTEM_4000				16              
#define	ISP_WB_MODE_CLRTEM_4100				17              
#define	ISP_WB_MODE_CLRTEM_4200				18              
#define	ISP_WB_MODE_CLRTEM_4300				19              
#define	ISP_WB_MODE_CLRTEM_4400				20              
#define	ISP_WB_MODE_CLRTEM_4500				21              
#define	ISP_WB_MODE_CLRTEM_4600				22              
#define	ISP_WB_MODE_CLRTEM_4700				23              
#define	ISP_WB_MODE_CLRTEM_4800				24              
#define	ISP_WB_MODE_CLRTEM_4900				25              
#define	ISP_WB_MODE_CLRTEM_5000				26              
#define	ISP_WB_MODE_CLRTEM_5100				27              
#define	ISP_WB_MODE_CLRTEM_5200				28              
#define	ISP_WB_MODE_CLRTEM_5300				29              
#define	ISP_WB_MODE_CLRTEM_5400				30              
#define	ISP_WB_MODE_CLRTEM_5500				31              
#define	ISP_WB_MODE_CLRTEM_5600				32              
#define	ISP_WB_MODE_CLRTEM_5700				33              
#define	ISP_WB_MODE_CLRTEM_5800				34              
#define	ISP_WB_MODE_CLRTEM_5900				35              
#define	ISP_WB_MODE_CLRTEM_6000				36              
#define	ISP_WB_MODE_CLRTEM_6100				37              
#define	ISP_WB_MODE_CLRTEM_6200				38              
#define	ISP_WB_MODE_CLRTEM_6300				39              
#define	ISP_WB_MODE_CLRTEM_6400				40              
#define	ISP_WB_MODE_CLRTEM_6500				41              
#define	ISP_WB_MODE_CLRTEM_6600				42              
#define	ISP_WB_MODE_CLRTEM_6700				43              
#define	ISP_WB_MODE_CLRTEM_6800				44              
#define	ISP_WB_MODE_CLRTEM_6900				45              
#define	ISP_WB_MODE_CLRTEM_7000				46              
#define	ISP_WB_MODE_CLRTEM_7100				47              
#define	ISP_WB_MODE_CLRTEM_7200				48              
#define	ISP_WB_MODE_CLRTEM_7300				49              
#define	ISP_WB_MODE_CLRTEM_7400				50              
#define	ISP_WB_MODE_CLRTEM_7500				51              
#define	ISP_WB_MODE_CLRTEM_7600				52              
#define	ISP_WB_MODE_CLRTEM_7700				53              
#define	ISP_WB_MODE_CLRTEM_7800				54              
#define	ISP_WB_MODE_CLRTEM_7900				55              
#define	ISP_WB_MODE_CLRTEM_8000				56              
#define	ISP_WB_MODE_CLRTEM_8100				57              
#define	ISP_WB_MODE_CLRTEM_8200				58              
#define	ISP_WB_MODE_CLRTEM_8300				59              
#define	ISP_WB_MODE_CLRTEM_8400				60              
#define	ISP_WB_MODE_CLRTEM_8500				61              
#define	ISP_WB_MODE_CLRTEM_8600				62              
#define	ISP_WB_MODE_CLRTEM_8700				63              
#define	ISP_WB_MODE_CLRTEM_8800				64              
#define	ISP_WB_MODE_CLRTEM_8900				65              
#define	ISP_WB_MODE_CLRTEM_9000				66              
#define	ISP_WB_MODE_CLRTEM_9100				67              
#define	ISP_WB_MODE_CLRTEM_9200				68              
#define	ISP_WB_MODE_CLRTEM_9300				69              
#define	ISP_WB_MODE_CLRTEM_9400				70              
#define	ISP_WB_MODE_CLRTEM_9500				71              
#define	ISP_WB_MODE_CLRTEM_9600				72              
#define	ISP_WB_MODE_CLRTEM_9700				73              
#define	ISP_WB_MODE_CLRTEM_9800				74              
#define	ISP_WB_MODE_CLRTEM_9900				75              
#define	ISP_WB_MODE_DAYLIGHT_0				76            
#define	ISP_WB_MODE_DAYLIGHT_POS1			77              
#define	ISP_WB_MODE_DAYLIGHT_POS2			78              
#define	ISP_WB_MODE_DAYLIGHT_POS3			79              
#define	ISP_WB_MODE_DAYLIGHT_NEG1			80              
#define	ISP_WB_MODE_DAYLIGHT_NEG2			81              
#define	ISP_WB_MODE_DAYLIGHT_NEG3			82              
#define	ISP_WB_MODE_COOLWHITEFLU_0			83            
#define	ISP_WB_MODE_COOLWHITEFLU_POS1		84              
#define	ISP_WB_MODE_COOLWHITEFLU_POS2		85              
#define	ISP_WB_MODE_COOLWHITEFLU_NEG1		86              
#define	ISP_WB_MODE_INCANDESCENT_0			87            
#define	ISP_WB_MODE_INCANDESCENT_POS1		88              
#define	ISP_WB_MODE_INCANDESCENT_POS2		89              
#define	ISP_WB_MODE_INCANDESCENT_POS3		90              
#define	ISP_WB_MODE_INCANDESCENT_NEG1		91              
#define	ISP_WB_MODE_INCANDESCENT_NEG2		92              
#define	ISP_WB_MODE_INCANDESCENT_NEG3		93              
#define	ISP_WB_MODE_FLASH_0					94              
#define	ISP_WB_MODE_FLASH_POS1				95            
#define	ISP_WB_MODE_FLASH_POS2				96            
#define	ISP_WB_MODE_FLASH_POS3				97            
#define	ISP_WB_MODE_FLASH_NEG1				98            
#define	ISP_WB_MODE_FLASH_NEG2				99            
#define	ISP_WB_MODE_FLASH_NEG3				100           
#define	ISP_WB_MODE_CLOUDY_0				101           
#define	ISP_WB_MODE_CLOUDY_POS1				102             
#define	ISP_WB_MODE_CLOUDY_POS2				103             
#define	ISP_WB_MODE_CLOUDY_POS3				104             
#define	ISP_WB_MODE_CLOUDY_NEG1				105             
#define	ISP_WB_MODE_CLOUDY_NEG2				106             
#define	ISP_WB_MODE_CLOUDY_NEG3				107             
#define	ISP_WB_MODE_SHADE_0					108             
#define	ISP_WB_MODE_SHADE_POS1				109           
#define	ISP_WB_MODE_SHADE_POS2				110           
#define	ISP_WB_MODE_SHADE_POS3				111           
#define	ISP_WB_MODE_SHADE_NEG1				112           
#define	ISP_WB_MODE_SHADE_NEG2				113           
#define	ISP_WB_MODE_SHADE_NEG3				114					               
                                                      
#define	ISP_LAST_WB_MODE					ISP_WB_MODE_SHADE_NEG3


/*****************************************************/
typedef struct{
	IM_UINT32		scanMode;	//ISP_ITU_SCAN_XXX, ISP only support progressive scanMode now
	IM_UINT32		format;		//ISP_ITU_FORMAT_XXX
	IM_UINT32		order;		//ISP_ITU_ORDER_XXX
}isp_itu_type_t;

typedef struct{
	IM_UINT32	hsync;
	IM_UINT32	href;
	IM_UINT32	vsync;
	IM_UINT32	pclk;
}isp_signal_polarity_t;

typedef struct{
	IM_UINT32				inWidth;
	IM_UINT32				inHeight;
	IM_UINT32				intfMode;		//ISP_INTFMODE_XXX
	IM_UINT32				inputBitsNum;
	IM_UINT32				rawMode;
	isp_itu_type_t			ituType;
	isp_signal_polarity_t	sigPol;
}isp_input_mode_t;

typedef struct{
	IM_UINT32				outWidth;
	IM_UINT32				outHeight;
	IM_IMAGE_TYPE			outImgType;		//IM_IMAGE_XXX
}isp_output_mode_t;

typedef struct{
	IM_UINT32	ppMode;
	IM_Buffer	buffY[4];	// y, rgb, ycbycr
	IM_Buffer	buffCb[4];	// cb, cbcr
	IM_Buffer	buffCr[4];	// cr
}isp_dma_config_t;

/*************************isp sub module*********************/
//
// bccb module
//

//bc cb mode
#define ISP_BCCB_MODE_BCCB	0
#define ISP_BCCB_MODE_CBBC	1

typedef struct
{
	IM_BOOL		bcEnable;
	IM_BOOL		cbEnable;
	IM_UINT32 	bccbMode;		//ISP_BCCB_MODE_XXX
	IM_UINT32	blkTh;			//0-255
	//IM_UINT32	wbGainMode;		//ISP_WB_MODE_XXX
	IM_UINT32	rGain;			//(0.0-4.0)*256
	IM_UINT32	gGain;			//(0.0-4.0)*256
	IM_UINT32	bGain;			//(0.0-4.0)*256
}isp_bccb_config_t;

//
// bdc module
//

/****************************************************************
*bdc correct type											    *	
* enable break point correct:	crtType |= ISP_BDC_BPCRT_ENABLE *
* enable saltpeper denoise:		crtType |= ISP_BDC_SLTPEP_ENABLE*
* enable gauss denoise:			crtType |= ISP_BDC_GAUSS_ENABLE *
*****************************************************************/
#define ISP_BDC_BPCRT_ENABLE		(1<<0)
#define ISP_BDC_SLTPEP_ENABLE		(1<<1)
#define	ISP_BDC_GAUSS_ENABLE		(1<<2)

// bdc mode
#define ISP_BDC_MODE_DETECT		0
#define ISP_BDC_MODE_CORRECT	1

// detect type
#define ISP_BDC_DETECT_TYPE_0	0
#define ISP_BDC_DETECT_TYPE_1	1

// noise level
#define ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS		0
#define ISP_BDC_CORRECT_NOISE_LEVEL_EDGE		1
#define ISP_BDC_CORRECT_NOISE_LEVEL_CONTENT	2

typedef struct
{
	IM_BOOL				enable;
	IM_UINT32 			bdcMode;	//ISP_BDC_MODE_XXX
	IM_UINT32 			detType;	//ISP_BDC_DETECT_TYPE_X
	IM_UINT32			crtType;
	IM_UINT32 			hiTh;
	IM_UINT32			loTh;
	IM_UINT32			sltPepTh;
	IM_UINT32			nosLvl;		//ISP_BDC_CORRECT__NOISE_LEVELX
	IM_Buffer			dmaBuf;
}isp_bdc_config_t;

//
// lens module
//

//lens mode
#define ISP_LENS_LUTB_MODE_0	0	//(for 320*240)
#define ISP_LENS_LUTB_MODE_1	1	//(for 640*480)
#define ISP_LENS_LUTB_MODE_2	2	//(for 800*480)
#define ISP_LENS_LUTB_MODE_3	3	//(for 1920*1080)
#define ISP_LENS_LUTB_MODE_4	4	//(for 4096*3072)

typedef struct
{
	IM_BOOL			enable;
	IM_UINT32		lutbMode;	//ISP_LENS_LUTB_MODE_X
}isp_lens_config_t;

//
// gma module
//
/*gma mode*/
#define ISP_GMA_MODE_0	0	//gamma = 1.0(default mode)
#define ISP_GMA_MODE_1	1	//gamma = 1.8
#define ISP_GMA_MODE_2	2	//gamma = 2.0
#define ISP_GMA_MODE_3	3	//gamma = 2.2
#define ISP_GMA_MODE_4	4	//gamma = 2.4
#define ISP_GMA_MODE_5	5	//gamma = 2.6
#define ISP_GMA_MODE_6	6	//gamma = 0.56
#define ISP_GMA_MODE_7	7	//gamma = 0.5
#define ISP_GMA_MODE_8	8	//gamma = 0.45
#define ISP_GMA_MODE_9	9	//gamma = 0.42
#define ISP_GMA_MODE_10	10	//gamma = 0.38

typedef struct{
	IM_BOOL		enable;
	IM_UINT32	mode;	//ISP_GMA_MODE_X
	IM_UINT32	rdMode;	//ISP_ROUND_XXX
}isp_gma_config_t;

//
//ee module
//
#define	ISP_EE_DIRECTION_HM		0	//0    degree
#define	ISP_EE_DIRECTION_VM		1	//90   degree
#define	ISP_EE_DIRECTION_D0M	2	//135  degree
#define	ISP_EE_DIRECTION_D1M	3	//45   degree

#define	ISP_EE_GAUSS_MODE_0		0	//default mode
#define	ISP_EE_GAUSS_MODE_1		1

//use default value at initial for each direction, but you can set other value
typedef struct{
	IM_UINT32	coef00;
	IM_UINT32	coef01;
	IM_UINT32	coef02;
	IM_UINT32	coef10;
	IM_UINT32	coef11;
	IM_UINT32	coef12;
	IM_UINT32	coef20;
	IM_UINT32	coef21;
	IM_UINT32	coef22;
}isp_ee_op_matrix;

typedef struct{	
	IM_UINT32	hTh;
	IM_UINT32	vTh;
	IM_UINT32	d0Th;
	IM_UINT32	d1Th;
}isp_ee_thr_matrix;

typedef struct
{
	IM_BOOL				enable;
	IM_UINT32			coefw;
	IM_UINT32			coefa;
	IM_UINT32			rdMode;	//ISP_ROUND_XXX
	IM_BOOL				gasEn;
	IM_UINT32			gasMode;//ISP_EE_GAUSS_MODE_X
	IM_UINT32			errTh;
	isp_ee_thr_matrix	thrMat;
}isp_ee_config_t;

//
// fcc module
//
typedef struct
{
	IM_BOOL		enable;
	IM_UINT32	threshold;
}isp_fcc_config_t;

//
//af module
//
#define ISP_AF_TYPE_CENTRE			0
#define ISP_AF_TYPE_THREE_WINS		1
#define ISP_AF_TYPE_FIVE_WINS		2
#define ISP_AF_TYPE_CENTRE_WEIGHT	3
#define ISP_AF_TYPE_NINE_WINS		4

typedef struct{
	IM_UINT32	x0;	//x-Begin
	IM_UINT32	y0;	//y-Begin;
	IM_UINT32	x1;	//x-End;
	IM_UINT32	y1;	//y-End;
}isp_af_block_position;

typedef struct{
	isp_af_block_position	blokPos[9];
}isp_af_coordinate_t;

typedef struct{
	IM_UINT32	value;
	IM_UINT32	cnt[9];
}isp_af_result_t;

typedef struct{
	IM_BOOL					enable;
	IM_UINT32				type;	//ISP_AF_TYPE_XXX
	isp_af_coordinate_t 	coordinate;
}isp_af_config_t;

//
//awb module
//
#define ISP_AWB_ANALYZE_MODE_GWD	0	//grayworld
#define ISP_AWB_ANALYZE_MODE_ROI	1	//roi
#define ISP_AWB_ANALYZE_MODE_IMP	2	//improved grayworld

typedef struct{
	IM_UINT32		thy1;
	IM_UINT32		thy2;
	IM_UINT32		costh;
	IM_UINT32		errth;
	IM_UINT32		coef1;
	IM_UINT32		cosbth;
	IM_UINT32		cbAmp;
	IM_UINT32		crAmp;	
	IM_UINT32		meanth;
}isp_awb_thr_matrix;

typedef struct{
	IM_UINT32		rpDown;
	IM_UINT32		rpUp;
	IM_UINT32		bpDown;
	IM_UINT32		bpUp;
}isp_awb_par_matrix;

typedef struct{
	IM_UINT32		x00;
	IM_UINT32		x01;
	IM_UINT32		y00;
	IM_UINT32		y01;
}isp_awb_roi_position;

typedef struct{
	IM_UINT32	rGain;
	IM_UINT32	gGain;
	IM_UINT32	bGain;
	IM_UINT32	cbSum;
	IM_UINT32	crSum;
}isp_awb_result_t;

/*if anaMode = ISP_AWB_ANALYZE_MODE_GWD, roiPos, thrMat and parMat all shonot set value,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos need to set value, but thrMat and parMat should not ,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos should not set value, but thrMat and parMat need set value.*/
typedef struct{
	IM_BOOL					enable;
	IM_BOOL					anaEnable;
	IM_UINT32				anaMode;	//ISP_AWB_ANALYZE_MODE_XXX
	isp_awb_roi_position	roiPos;
	isp_awb_thr_matrix		thrMat;
	isp_awb_par_matrix		parMat;
}isp_awb_config_t;

//
//cmncsc module
//
#define	ISP_CMNCSC_RGB2YUV	0	//default mode
#define	ISP_CMNCSC_YUV2RGB	1

typedef struct{
	IM_BOOL				enable;
	IM_UINT32			mode;
}isp_cmncsc_config_t;

//
//ae module
//
/****************************************************************
*ae type, 0:CENTER,1:CENTER_WEIGHT,2:AVERAGE,3:ROI,4:MAIN_REGION*
*****************************************************************/
#define ISP_AE_TYPE_CENTER			0
#define ISP_AE_TYPE_CENTER_WEIGHT	1
#define ISP_AE_TYPE_AVERAGE			2
#define ISP_AE_TYPE_ROI				3
#define ISP_AE_TYPE_MAIN_REGION		4
typedef struct{
	IM_UINT32	x0;	//x-Begin
	IM_UINT32	y0;	//y-Begin;
	IM_UINT32	x1;	//x-End;
	IM_UINT32	y1;	//y-End;
}isp_ae_block_position;

typedef struct{
	IM_UINT32			  log2_blokPixNum; //log2(blokPixNum) need to set to register
	isp_ae_block_position blokPos[25];
}isp_ae_coordinate_t;

typedef struct{
	IM_UINT32			  blokNum;//(reciprocal of block num) *2^15 need to set to register
	IM_UINT32			  blokEn;
}isp_ae_block_select;

typedef struct{
	IM_UINT32	avg;
	IM_UINT32	cnt[25];
}isp_ae_result_t;

typedef struct{
	IM_BOOL					enable;
	isp_ae_block_select		blokSelect;
	isp_ae_coordinate_t		coordinate;
}isp_ae_config_t;

//
//hist module
//
typedef struct{
	IM_UINT32	th1;
	IM_UINT32	th2;
}isp_hist_thr_matrix;

typedef struct{
	IM_UINT32	bkLit;
	IM_UINT32	hist[16];	
}isp_hist_result_t;

typedef struct{
	IM_BOOL						enable;
	IM_UINT32					blitTh1Num;//numerator value such as 20
	IM_UINT32					blitTh1Den;//denominator value such as 30
	isp_hist_thr_matrix			thrMat;
}isp_hist_config_t;

//
// acc module 
//
#define	ISP_ACC_LUTB_CONTROL_MODE_NORMAL	0	//ACC LUTB controlled by CPU
#define	ISP_ACC_LUTB_CONTROL_MODE_CPU		1	//ACC LUTB in Normal usage

// acc lutb params mode.
#define ISP_ACC_LUTB_MODE_0	0	//default mode
//#define ISP_ACC_LUTB_MODE_1	1	//

typedef struct{
	IM_UINT32		lutbCtrMode;	//ISP_ACC_LUTB_CONTROL_MODE_XXX
	IM_UINT32		lutbMode;		//ISP_ACC_LUTB_MODE_X
}isp_acc_lutb_type;

typedef struct{
	IM_UINT32			histFrames;
	IM_UINT32 			histRdMode;
}isp_acc_hist_t;

typedef struct{
	IM_UINT32		coefa;
	IM_UINT32		coefb;
	IM_UINT32		coefc;
	IM_UINT32		coefd;
}isp_acc_co_matrix;

typedef struct{
	IM_UINT32		roaHi;
	IM_UINT32		roaLo;
	IM_UINT32		robHi;
	IM_UINT32		robLo;
	IM_UINT32		rocHi;
	IM_UINT32		rocLo;
}isp_acc_ro_matrix;

typedef struct{
	IM_BOOL				enable;
	IM_UINT32 			rdMode;	//rdMode = ISP_ROUND_XXX
	IM_INT32			coefe;
	isp_acc_lutb_type	lutbType;
	isp_acc_hist_t		hist;
	isp_acc_co_matrix 	coMat;
	isp_acc_ro_matrix	roMat;
}isp_acc_config_t;

//
//ief module
//
#define	ISP_IEF_CSC_YUV2RGB	0	//default csc mode
#define	ISP_IEF_CSC_CUSTOM	1

#define ISP_IEF_TYPE_NORMAL			0	
#define ISP_IEF_TYPE_GRAY			1
#define ISP_IEF_TYPE_NEGATIVE		2
#define ISP_IEF_TYPE_SOLARIZATION	3
#define ISP_IEF_TYPE_SEPIA			4
#define ISP_IEF_TYPE_EMBOSS			5
#define ISP_IEF_TYPE_SKETCH_SOBEL	6
#define ISP_IEF_TYPE_SKETCH_LP		7
#define ISP_IEF_TYPE_COLOR_SELECT	8

#define ISP_IEF_COLOR_SELECT_MODE_R	0
#define ISP_IEF_COLOR_SELECT_MODE_G	1
#define ISP_IEF_COLOR_SELECT_MODE_B	2

typedef struct{
	IM_UINT32		coefCr;
	IM_UINT32		coefCb;
}isp_ief_rgcf_matrix;

/*if cscMode = ISP_IEF_CSC_YUV2RGB(default mode), all csc coef should not set 
  else cscMode = ISP_IEF_CSC_CUSTOM, all csc coef value must be set*/
typedef struct{
	IM_UINT32	cscMode;	//ISP_IEF_CSC_XXX
	IM_INT32	coef11;
	IM_INT32	coef12;
	IM_INT32	coef13;
	IM_INT32	coef21;
	IM_INT32	coef22;
	IM_INT32	coef23;
	IM_INT32	coef31;
	IM_INT32	coef32;
	IM_INT32	coef33;
	IM_INT32	oft_a;
	IM_INT32	oft_b;
}isp_ief_csc_matrix;

typedef struct{
	IM_UINT32	mode;	//ISP_IEF_COLOR_SELECT_MODE_X
	IM_UINT32	tha;
	IM_UINT32	thb;
}isp_ief_select_matrix;

typedef struct{
	IM_BOOL					enable;
	IM_BOOL					cscEn;
	IM_UINT32				type;	//ISP_IEF_TYPE_XXX
	isp_ief_rgcf_matrix		rgcfMat;
	isp_ief_select_matrix	selMat;
	isp_ief_csc_matrix		cscMat;
}isp_ief_config_t;

//
//acm module
//
typedef struct{
	IM_UINT32		tha;
	IM_UINT32		thb;
	IM_UINT32		thc;
}isp_acm_thr_matrix;

typedef struct{
	IM_UINT32		coefp;
	IM_UINT32		coefr;
	IM_UINT32		coefg;
	IM_UINT32		coefb;
	IM_UINT32		m0r;
	IM_UINT32		m1r;
	IM_UINT32		m2r;
	IM_UINT32		m3r;
	IM_UINT32		m4r;
	IM_UINT32		m0g;
	IM_UINT32		m1g;
	IM_UINT32		m2g;
	IM_UINT32		m3g;
	IM_UINT32		m4g;
	IM_UINT32		m0b;
	IM_UINT32		m1b;
	IM_UINT32		m2b;
	IM_UINT32		m3b;
	IM_UINT32		m4b;
}isp_acm_coef_matrix;

typedef struct{
	IM_BOOL					enable;
	IM_UINT32				rdMode;	//ISP_ROUND_XXX
	IM_INT32				ths;
	isp_acm_thr_matrix		thrMat;
	isp_acm_coef_matrix		coMat;
}isp_acm_config_t;
//
//osd module
//

// palette type that osd supportted.
#define ISP_OSD_PALETTE_FORMAT_A888		0
#define ISP_OSD_PALETTE_FORMAT_888		1
#define ISP_OSD_PALETTE_FORMAT_A666		2
#define ISP_OSD_PALETTE_FORMAT_A665		3
#define ISP_OSD_PALETTE_FORMAT_666		4
#define ISP_OSD_PALETTE_FORMAT_A555		5
#define ISP_OSD_PALETTE_FORMAT_565		6

// image formats that osd supports.
#define ISP_OSD_IMAGE_PAL_BPP1			0
#define ISP_OSD_IMAGE_PAL_BPP2			1
#define ISP_OSD_IMAGE_PAL_BPP4			2
#define ISP_OSD_IMAGE_PAL_BPP8			3
#define ISP_OSD_IMAGE_RGB_BPP8_1A232	4
#define ISP_OSD_IMAGE_RGB_BPP16_565		5
#define ISP_OSD_IMAGE_RGB_BPP16_1A555	6
#define ISP_OSD_IMAGE_RGB_BPP16_I555	7
#define ISP_OSD_IMAGE_RGB_BPP18_666		8
#define ISP_OSD_IMAGE_RGB_BPP18_1A665	9
#define ISP_OSD_IMAGE_RGB_BPP19_1A666	10
#define ISP_OSD_IMAGE_RGB_BPP24_888		11
#define ISP_OSD_IMAGE_RGB_BPP24_1A887	12
#define ISP_OSD_IMAGE_RGB_BPP25_1A888	13
#define ISP_OSD_IMAGE_RGB_BPP28_4A888	14
#define ISP_OSD_IMAGE_RGB_BPP16_4A444	15
#define ISP_OSD_IMAGE_RGB_BPP32_8A888	16

//format not support
//#define ISP_OSD_IMAGE_YUV_420SP		17 // yuv420 semi-planar, 
//#define ISP_OSD_IMAGE_RGB_BPP32_888A	18
//#define ISP_OSD_IMAGE_RGB_BPP16_555A	19
//#define ISP_OSD_IMAGE_RGB_BPP16_555I	20

// alpha path flag, xx_0 indicate select alpha 0 path, else alpha 1 path.
#define ISP_OSD_ALPHA_PATH_0	0
#define ISP_OSD_ALPHA_PATH_1	1

// alpha blend type, alpha value is rely on per plane or per pixel.
#define ISP_OSD_BLEND_PER_PLANE	0
#define ISP_OSD_BLEND_PER_PIXEL	1

// color key match mode, match background and display(or blending with)forground, or opposite.
#define ISP_OSD_COLORKEY_DIR_MATCH_FORGROUND	0
#define ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND	1

// buffer mode.
#define ISP_OSD_BUFFER_SEL_MANUAL	0
#define ISP_OSD_BUFFER_SEL_AUTO		1

// buffer select type
#define ISP_OSD_BUFSEL_BUF0			0
#define ISP_OSD_BUFSEL_BUF1			1
#define ISP_OSD_BUFSEL_BUF2			2
#define ISP_OSD_BUFSEL_BUF3			3

// swap for input wnd1 source.
typedef struct{
	IM_BOOL 	bitSwap;
	IM_BOOL 	bits2Swap;
	IM_BOOL 	bits4Swap;
	IM_BOOL 	byteSwap;
	IM_BOOL 	halfwordSwap;
}isp_osd_swap_t;

// alpha configuration structure.
typedef struct{
	IM_UINT32	path;		// ISP_OSD_ALPHA_PATH_x.
	IM_UINT32	blendMode;	// ISP_OSD_BLEND_PER_xxx.
	IM_UINT32	alpha0_r;	// range 0--15 
	IM_UINT32	alpha0_g;	// range 0--15 
	IM_UINT32	alpha0_b;	// range 0--15 
	IM_UINT32	alpha1_r;	// range 0--15 
	IM_UINT32	alpha1_g;	// range 0--15 
	IM_UINT32	alpha1_b;	// range 0--15 
}isp_osd_alpha_t;

// color key configuration.
typedef struct{
	IM_BOOL		enable;
	IM_BOOL 	enableBlend;
	IM_UINT32	matchMode;	// ISP_OSD_COLORKEY_DIR_MATCH_xxx.
	IM_UINT32	mask;		// format is RGB888.
	IM_UINT32	color;		// format is RGB888.
}isp_osd_colorkey_t;

// map color.
typedef struct{
	IM_BOOL		enable;
	IM_UINT32	color;		// format is RGB888.
}isp_osd_mapcolor_t;

// palette.
typedef struct{
	IM_UINT32	palFormat;			// ISP_OSD_PALETTE_FORMAT_xxx.
	IM_UINT32	tableLength;	// it's not bytes, bug depth, in 4bytes.
	IM_UINT32	*table;
}isp_osd_palette_t;


typedef struct{
	IM_UINT32	mode;	 // ISP_OSD_BUFFER_SEL_xxx. 
	IM_UINT32	number;	 // in auto mode, it's ping-pong buffer number(2,3,4), 
	IM_UINT32	selType; // ISP_OSD_BUFSEL_XXX
}isp_osd_buffer_mode;

// osd window buffer.
typedef struct{	
	IM_BOOL		mask[4];	// which buffer is noticeable(IM_FALSE).
	IM_Buffer	buff[4];
	//IM_UINT32	bitOffset[4]; //no demand
}isp_osd_buffer_t;

// osd vitual window.
typedef struct{
	IM_UINT32	width;		// pixel unit, range 1--(2^16-1).
	IM_UINT32	height;		// pixel unit.
	IM_UINT32	xOffset;	// pixel unit.
	IM_UINT32	yOffset;	// pixel unit.
}isp_osd_vm_t;

// osd window coordinate.
typedef struct{
	IM_INT32	x0;		// pixel unit, it may be negative.
	IM_INT32	y0;		// pixel unit, it may be negative.
	IM_UINT32	w;
	IM_UINT32	h;
}isp_osd_coordinate_t;

//window 0
typedef struct {
	IM_BOOL					enable;	
	isp_osd_mapcolor_t		mapclr;		//window0 not support mapcolor, so it is useless
	isp_osd_coordinate_t	coordinate;
}osd_wnd0_config_t;

//window1
typedef struct {
	IM_BOOL					enable;
	IM_UINT32				imgFormat;	// ISP_OSD_IMAGE_xxx.
	isp_osd_palette_t		palette;
	isp_osd_swap_t			swap;
	isp_osd_alpha_t			alpha;
	isp_osd_mapcolor_t		mapclr;
	isp_osd_colorkey_t		clrkey;
	isp_osd_vm_t			vm;
	isp_osd_coordinate_t	coordinate;
	//isp_osd_buffer_mode 	bm;
	IM_Buffer				buf;
}osd_wnd1_config_t;

typedef struct {
	IM_UINT32				enable;
	IM_UINT32 				bgColor;	// background color of osd, format is RGB888. 
	IM_UINT32				outWidth;	// range 1--4096
	IM_UINT32				outHeight;	// range 1--4096
	osd_wnd0_config_t		wnd0;
	osd_wnd1_config_t		wnd1;
}isp_osd_config_t;

//
//crop module 
//
/*crop window coordinate*/
typedef struct{
	IM_UINT32	x0;
	IM_UINT32	y0;
	IM_UINT32	x1;
	IM_UINT32	y1;
}isp_crop_coordinate_t;

typedef struct{
	IM_BOOL					enable;
	isp_crop_coordinate_t	coordinate;
}isp_crop_config_t;

//
//scl module 
//
#define ISP_SCL_PARAM_TYPE_0		0//default type
#define ISP_SCL_PARAM_TYPE_1		1

#define ISP_SCL_CSC_MODE_0			0//default mode
#define ISP_SCL_CSC_MODE_1			1


#if 0//use IM_IMAGE_XXX(defined in IM_format.h substitute
//SCL output formats supported
#define ISP_SCL_OFORMAT_RGB_0888			0	//only one DMA channel
#define ISP_SCL_OFORMAT_RGB_8880			1	//only one DMA channel
#define ISP_SCL_OFORMAT_BGR_0888			2	//only one DMA channel
#define ISP_SCL_OFORMAT_BGR_8880			3	//only one DMA channel
#define ISP_SCL_OFORMAT_YUV444				4	//only one DMA
#define ISP_SCL_OFORMAT_YUV420SP			5	//two 	DMA
#define ISP_SCL_OFORMAT_YUV420P				6	//three DMA
#define ISP_SCL_OFORMAT_YUV422SP			7	//two 	DMA
#define ISP_SCL_OFORMAT_YUV422P				8	//three DMA
#endif

//parameters type and csc mode use default value at initinal, but you can set other value 
typedef struct
{
	IM_BOOL			verEnable;
	IM_BOOL			horEnable;
	IM_UINT32		sclInWidth;		// range 1--4096, it is ISP last(real) outWidth if scl enable
	IM_UINT32		sclInHeight;	// range 1--4096,it is ISP last(real) outHeight if scl enable
	IM_UINT32		sclOutWidth;		// range 1--4096, it is ISP last(real) outWidth if scl enable
	IM_UINT32		sclOutHeight;	// range 1--4096,it is ISP last(real) outHeight if scl enable
	IM_UINT32		vrdMode;	//ISP_ROUND_XXX
	IM_UINT32		hrdMode;	//ISP_ROUND_XXX
	//IM_UINT32		paramType;	//ISP_SCL_PARAM_TYPE_XXX
	//IM_UINT32		cscMode;	//ISP_SCL_CSC_MODE_X
}isp_scl_config_t;

typedef struct
{	
	//input mode
	IM_UINT32				inWidth;
	IM_UINT32				inHeight;
	IM_UINT32				intfMode;		//ISP_INTFMODE_XXX
	IM_UINT32				inputBitsNum;
	IM_UINT32				rawMode;
	isp_itu_type_t			ituType;
	isp_signal_polarity_t	sigPol;
	//output mode
	IM_UINT32				outWidth;
	IM_UINT32				outHeight;
	IM_IMAGE_TYPE			outImgType;		//IM_IMAGE_XXX
	//sub module	
	IM_BOOL					demEnable;		//demosiac enable
	isp_bccb_config_t		bccbCfg;
	isp_bdc_config_t		bdcCfg;
	isp_lens_config_t		lensCfg;
	isp_gma_config_t		gmaCfg;
	isp_ee_config_t			eeCfg;
	isp_fcc_config_t		fccCfg;
	isp_af_config_t			afCfg;
	isp_awb_config_t		awbCfg;
	isp_cmncsc_config_t		cmncscCfg;
	isp_ae_config_t			aeCfg;
	isp_hist_config_t		histCfg;
	isp_acc_config_t		accCfg;
	isp_ief_config_t		iefCfg;
	isp_acm_config_t		acmCfg;
	isp_osd_config_t 		osdCfg;
	isp_crop_config_t		cropCfg;
	isp_scl_config_t		sclCfg;
	//isp_dma_config_t		dmaCfg;
}isp_config_t;
                                                                                
typedef struct
{	
	//sub module whether need to reconfigure
	IM_BOOL					demNeedConfig;	
	IM_BOOL					bccbNeedConfig;	
	IM_BOOL					bdcNeedConfig;	
	IM_BOOL					lensNeedConfig;	
	IM_BOOL					gmaNeedConfig;	
	IM_BOOL					eeNeedConfig;	
	IM_BOOL					fccNeedConfig;	
	IM_BOOL					afNeedConfig;	
	IM_BOOL					awbNeedConfig;	
	IM_BOOL					cmncscNeedConfig;	
	IM_BOOL					aeNeedConfig;	
	IM_BOOL					histNeedConfig;	
	IM_BOOL					accNeedConfig;	
	IM_BOOL					iefNeedConfig;	
	IM_BOOL					acmNeedConfig;	
	IM_BOOL					osdNeedConfig;	
	IM_BOOL					cropNeedConfig;	
	IM_BOOL					sclNeedConfig;	
	//sub module detail configs
	IM_BOOL					demEnable;		//demosiac enable
	isp_bccb_config_t		bccbCfg;
	isp_bdc_config_t		bdcCfg;
	isp_lens_config_t		lensCfg;
	isp_gma_config_t		gmaCfg;
	isp_ee_config_t			eeCfg;
	isp_fcc_config_t		fccCfg;
	isp_af_config_t			afCfg;
	isp_awb_config_t		awbCfg;
	isp_cmncsc_config_t		cmncscCfg;
	isp_ae_config_t			aeCfg;
	isp_hist_config_t		histCfg;
	isp_acc_config_t		accCfg;
	isp_ief_config_t		iefCfg;
	isp_acm_config_t		acmCfg;
	isp_osd_config_t 		osdCfg;
	isp_crop_config_t		cropCfg;
	isp_scl_config_t		sclCfg;
}isp_sub_config_t;
/*------------------------------------------------------------------------------
    Function prototypes                                                         
------------------------------------------------------------------------------*/
IM_RET isplib_init(isp_config_t *ispCfg);
IM_RET isplib_deinit(void);
IM_RET isplib_set_buffers(isp_dma_config_t *dmaCfg);
IM_RET isplib_set_input_mode(isp_input_mode_t *inputMode);
IM_RET isplib_set_input_resolution(IM_UINT32 width, IM_UINT32 height);
IM_RET isplib_set_output_mode(isp_output_mode_t *outputMode);
IM_RET isplib_start(void);
IM_RET isplib_stop(void);
IM_RET isplib_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout);
IM_RET isplib_get_ready_buffer(IM_Buffer *readyBuffs, IM_Buffer *replaceBuffs);


IM_RET isplib_config_update(IM_BOOL updateEn);


/*===============================================
set sub module property
=================================================*/
//bccb module
IM_RET isplib_bccb_set_mode(IM_UINT32 mode);
IM_RET isplib_bccb_set_bc(IM_UINT32 blkTh);
IM_RET isplib_bccb_set_cb(IM_UINT32 rGain, IM_UINT32 gGain, IM_UINT32 bGain);
IM_RET isplib_bccb_set_bc_enable(void);
IM_RET isplib_bccb_set_bc_disable(void);
IM_RET isplib_bccb_set_cb_enable(void);
IM_RET isplib_bccb_set_cb_disable(void);

//bdc module
IM_RET isplib_bdc_set_detect_mode(IM_Buffer dmaBuf, IM_UINT32 detType, IM_UINT32 hiTh, IM_UINT32 loTh);
IM_RET isplib_bdc_set_correct_mode(IM_Buffer dmaBuf, IM_UINT32 crtType, IM_UINT32 sltPepTh, IM_UINT32 nosLvl);
IM_RET isplib_bdc_get_bp_info(void);
IM_RET isplib_bdc_set_enable(void);
IM_RET isplib_bdc_set_disable(void);

//lens module
IM_RET isplib_lens_set_lutb(IM_UINT32 lutbMode);
IM_RET isplib_lens_set_enable(void);
IM_RET isplib_lens_set_disable(void);

//gma module
IM_RET isplib_gma_set_mode(IM_UINT32 mode);		//mode = ISP_GMA_MODE_X
IM_RET isplib_gma_set_round_mode(IM_UINT32 rdMode);	//round = ISP_ROUND_XXX
IM_RET isplib_gma_set_enable(void);
IM_RET isplib_gma_set_disable(void);

//ee module
IM_RET isplib_ee_set_coefw(IM_UINT32 coefw);
IM_RET isplib_ee_set_coefa(IM_UINT32 coefa);
IM_RET isplib_ee_set_round_mode(IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET isplib_ee_set_gauss_filter_enable(IM_BOOL gasEn);
IM_RET isplib_ee_set_gauss_mode(IM_UINT32 gasMode);
IM_RET isplib_ee_set_error_threshold(IM_UINT32 errTh);
IM_RET isplib_ee_set_detect_threshold(IM_UINT32 detTh);
IM_RET isplib_ee_set_detect_threshold_matrix(isp_ee_thr_matrix *thrMat);
IM_RET isplib_ee_set_edge_operator_matrix(IM_UINT32 direction, isp_ee_op_matrix *opMat);//direction = ISP_EE_DIRECTION_XXX
IM_RET isplib_ee_set_enable(void);
IM_RET isplib_ee_set_disable(void);

//fcc module
IM_RET isplib_fcc_set_threshold(IM_UINT32 threshold);
IM_RET isplib_fcc_set_enable(void);
IM_RET isplib_fcc_set_disable(void);

//af module
IM_RET isplib_af_set_type(IM_UINT32 type);
IM_RET isplib_af_set_block_coordinate(isp_af_coordinate_t *coordinate);
IM_RET isplib_af_get_result(isp_af_result_t *rsut);
IM_RET isplib_af_set_enable(void);
IM_RET isplib_af_set_disable(void);

//awb module
/*if anaMode = ISP_AWB_ANALYZE_MODE_GWD, roiPos, thrMat and parMat all shonot set value,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos need to set value, but thrMat and parMat should not ,
else if anaMode = ISP_AWB_ANALYZE_MODE_ROI, roiPos should not set value, but thrMat and parMat need set value.*/
IM_RET isplib_awb_set_analyze_mode(IM_UINT32 anaMode);
IM_RET isplib_awb_set_roi_position(isp_awb_roi_position *roiPos);
IM_RET isplib_awb_set_threshold_matrix(isp_awb_thr_matrix *thrMat);
IM_RET isplib_awb_set_par_matrix(isp_awb_par_matrix *parMat);
IM_RET isplib_awb_get_result(isp_awb_result_t *rsut);
IM_RET isplib_awb_set_enable(void);
IM_RET isplib_awb_set_disable(void);

//cmncsc module
IM_RET isplib_cmncsc_set_mode(IM_UINT32 mode);	//mode = ISP_CMNCSC_XXX
IM_RET isplib_cmncsc_set_enable(void);
IM_RET isplib_cmncsc_set_disable(void);

//ae module
IM_RET isplib_ae_set_block_select(isp_ae_block_select *blokSelect);
IM_RET isplib_ae_set_block_coordinate(isp_ae_coordinate_t *coordinate);
IM_RET isplib_ae_set_type(IM_UINT32 type);
IM_RET isplib_ae_get_result(isp_ae_result_t *rsut);
IM_RET isplib_ae_set_enable(void);
IM_RET isplib_ae_set_disable(void);


//hist module
IM_RET isplib_hist_set_backlit_threshold(IM_UINT32 blitTh1Num, IM_UINT32 blitTh1Den);
IM_RET isplib_hist_set_hist_threshold_matrix(isp_hist_thr_matrix *thrMat);
IM_RET isplib_hist_get_result(isp_hist_result_t *rsut);
IM_RET isplib_hist_set_enable(void);
IM_RET isplib_hist_set_disable(void);

//acc module
IM_RET isplib_acc_set_round_mode(IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET isplib_acc_set_lutb(isp_acc_lutb_type *lutbType);
IM_RET isplib_acc_set_hist(isp_acc_hist_t *hist);
IM_RET isplib_acc_set_contrast_coef(IM_INT32 coefe);
IM_RET isplib_acc_set_coef_matrix(isp_acc_co_matrix *coMat);
IM_RET isplib_acc_set_ro_matrix(isp_acc_ro_matrix *roMat);
IM_RET isplib_acc_set_enable(void);
IM_RET isplib_acc_set_disable(void);

//ief module
IM_RET isplib_ief_set_type(IM_UINT32 type);	//type = ISP_IEF_TYPE_XXX
IM_RET isplib_ief_set_rgcf_matrix(isp_ief_rgcf_matrix *rgcfMat);
IM_RET isplib_ief_set_color_select_mode_matrix(isp_ief_select_matrix *selMat);
/*if cscMode = ISP_IEF_CSC_YUV2RGB(default mode), all csc coef should not set 
  else cscMode = ISP_IEF_CSC_CUSTOM, all csc coef value must be set*/
IM_RET isplib_ief_set_csc_matrix(isp_ief_csc_matrix *cscMat);
IM_RET isplib_ief_set_csc_enable(void);
IM_RET isplib_ief_set_csc_disable(void);
IM_RET isplib_ief_set_enable(void);
IM_RET isplib_ief_set_disable(void);

//acm module
IM_RET isplib_acm_set_round_mode(IM_UINT32 rdMode);	//rdMode = ISP_ROUND_XXX
IM_RET isplib_acm_set_saturation_threshold(IM_INT32 ths);
IM_RET isplib_acm_set_threshold_matrix(isp_acm_thr_matrix *thrMat);
IM_RET isplib_acm_set_coef_matrix(isp_acm_coef_matrix *coMat);
IM_RET isplib_acm_set_enable(void);
IM_RET isplib_acm_set_disable(void);

//osd module

/*osd wndow0*/
IM_RET isplib_osd_wnd0_init(osd_wnd0_config_t *wnd0);
IM_RET isplib_osd_wnd0_deinit(void);
IM_RET isplib_osd_wnd0_set_mapcolor(isp_osd_mapcolor_t *map);
IM_RET isplib_osd_wnd0_set_coordinate(isp_osd_coordinate_t *coordinate);
IM_RET isplib_osd_wnd0_set_enable(void);
IM_RET isplib_osd_wnd0_set_disable(void);

/*osd wndow1*/
IM_RET isplib_osd_wnd1_init(osd_wnd1_config_t *wnd1);
IM_RET isplib_osd_wnd1_deinit(void);
IM_RET isplib_osd_wnd1_set_swap(isp_osd_swap_t *swap);
IM_RET isplib_osd_wnd1_set_alpha(isp_osd_alpha_t *alpha);
IM_RET isplib_osd_wnd1_set_format( IM_UINT32 imgFormat);
IM_RET isplib_osd_wnd1_set_palette_format(IM_UINT32 imgFormat, isp_osd_palette_t *palette);
IM_RET isplib_osd_wnd1_set_mapcolor(isp_osd_mapcolor_t *map);
IM_RET isplib_osd_wnd1_set_colorkey(isp_osd_colorkey_t *clrkey);
IM_RET isplib_osd_wnd1_set_buffer_mode(isp_osd_buffer_mode *bm);
IM_RET isplib_osd_wnd1_set_buffers(isp_osd_buffer_t *buffer);
IM_RET isplib_osd_wnd1_set_virtual_window(isp_osd_vm_t *vm);
IM_RET isplib_osd_wnd1_set_coordinate(isp_osd_coordinate_t *coordinate);
IM_RET isplib_osd_wnd1_set_enable(void);
IM_RET isplib_osd_wnd1_set_disable(void);

/*osd global*/
IM_RET isplib_osd_set_background_color(IM_UINT32 bgColor);
IM_RET isplib_osd_set_out_size(IM_UINT32 width, IM_UINT32 height);
IM_RET isplib_osd_set_enable(void);
IM_RET isplib_osd_set_disable(void);

//crop module
IM_RET isplib_crop_set_coordinate(isp_crop_coordinate_t *coordinate);
IM_RET isplib_crop_set_enable(void);
IM_RET isplib_crop_set_disable(void);

//scl module
IM_RET isplib_scl_set_input(IM_UINT32 inWidth, IM_UINT32 inHeight);
IM_RET isplib_scl_set_output(IM_UINT32 outWidth, IM_UINT32 outHeight);
IM_RET isplib_scl_set_round_mode(IM_UINT32 vrdMode, IM_UINT32 hrdMode);
IM_RET isplib_scl_set_param_type(IM_UINT32 paramType);
IM_RET isplib_scl_set_csc_mode(IM_UINT32 cscMode);
IM_RET isplib_scl_set_enable(IM_BOOL verEnable, IM_BOOL horEnable);

//digital zoom, zoom values multiply by 100, so real zoom = valuse/100
IM_RET isplib_set_digital_zoom(IM_UINT32 value);

#endif /* #ifndef _ISP_LIB_H_ */
