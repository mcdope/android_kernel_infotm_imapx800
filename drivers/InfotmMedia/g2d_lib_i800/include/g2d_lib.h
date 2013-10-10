/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_lib.h
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
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8.
-- v1.0.3	arsor@2012/04/18: mmu test ok and fixed poll timeout bug.
--
------------------------------------------------------------------------------*/

#ifndef __G2D_LIB_H__
#define __G2D_LIB_H__


//#############################################################################
//
//g2d property
//

#define G2D_PRO_MEM			0
#define G2D_PRO_DRAW		1
#define G2D_PRO_ROTATE		2
#define G2D_PRO_ALPHAROP	3




//
//dith
//
#define G2D_DITH_COEF_TYPE_0		0
#define G2D_DITH_COEF_TYPE_1		1


#define G2D_DITH_COEF_TYPE_DEFAULT	G2D_DITH_COEF_TYPE_0
#define G2D_DITH_COEF_TYPE_LAST		G2D_DITH_COEF_TYPE_1

//
//sclee
//
#define	G2D_ROUND_MINUS				0
#define	G2D_ROUND_NEAREST			1

#define G2D_ORDER_SCLEE				0
#define G2D_ORDER_EESCL			 	1

//SCL PARAM TYPE
#define G2D_SCL_PARAM_TYPE_0		0
#define G2D_SCL_PARAM_TYPE_1	 	1

#define G2D_SCL_PARAM_TYPE_DEFAULT	G2D_SCL_PARAM_TYPE_0
#define G2D_SCL_PARAM_TYPE_LAST 	G2D_SCL_PARAM_TYPE_1

//EE OPERATOR MATRIX TYPE
#define G2D_EE_OPMAT_TYPE_0			0
#define G2D_EE_OPMAT_TYPE_1			1

#define G2D_EE_OPMAT_TYPE_DEFAULT	G2D_EE_OPMAT_TYPE_0
#define G2D_EE_OPMAT_TYPE_LAST		G2D_EE_OPMAT_TYPE_1

//EE GAUSS MODE 
#define	G2D_EE_GAUSS_MODE_0			0
#define	G2D_EE_GAUSS_MODE_1			1	
#define	G2D_EE_GAUSS_MODE_2			2

#define G2D_EE_GAUSS_MODE_DEFAULT	G2D_EE_GAUSS_MODE_0
#define G2D_EE_GAUSS_MODE_LAST		G2D_EE_GAUSS_MODE_2

//
// pallete formats that g2d supportted.
//
#define G2D_PALATTE_FORMAT_A888		0
#define G2D_PALATTE_FORMAT_888		1
#define G2D_PALATTE_FORMAT_A666		2
#define G2D_PALATTE_FORMAT_A665		3
#define G2D_PALATTE_FORMAT_666		4
#define G2D_PALATTE_FORMAT_A555		5
#define G2D_PALATTE_FORMAT_565		6

//
// RGB BPP formats that g2d supports.
//
#define G2D_IMAGE_PAL_BPP1        0
#define G2D_IMAGE_PAL_BPP2        1
#define G2D_IMAGE_PAL_BPP4        2
#define G2D_IMAGE_PAL_BPP8        3
#define G2D_IMAGE_RGB_BPP8_1A232  4
#define G2D_IMAGE_RGB_BPP16_565   5
#define G2D_IMAGE_RGB_BPP16_1A555 6
#define G2D_IMAGE_RGB_BPP16_I555  7
#define G2D_IMAGE_RGB_BPP18_666   8
#define G2D_IMAGE_RGB_BPP18_1A665 9	
#define G2D_IMAGE_RGB_BPP19_1A666 10
#define G2D_IMAGE_RGB_BPP24_888   11
#define G2D_IMAGE_RGB_BPP24_1A887 12
#define G2D_IMAGE_RGB_BPP25_1A888 13
#define G2D_IMAGE_RGB_BPP28_4A888 14
#define G2D_IMAGE_RGB_BPP32_888A  18
#define G2D_IMAGE_RGB_BPP16_555A  19
#define G2D_IMAGE_RGB_BPP16_555I  20
#define G2D_IMAGE_RGB_BPP32_A888  21
#define G2D_IMAGE_RGB_BPP12_444   22
#define G2D_IMAGE_RGB_BPP16_4A444 23
#define G2D_IMAGE_RGB_BPP17_1A565 24
#define G2D_IMAGE_RGB_BPP24_8A565 25
#define G2D_IMAGE_YUV_422I_YUYV   26
#define G2D_IMAGE_YUV_422I_UYVY   27
#define G2D_IMAGE_YUV_422I_YVYU   28
#define G2D_IMAGE_YUV_422I_VYUY   29

#define G2D_IMAGE_YUV_444I        50
#define G2D_IMAGE_YUV_422SP       51
#define G2D_IMAGE_YUV_420SP       52


//
// RGB BR reverse
//
#define	G2D_BR_REVERSE_RGB			0
#define	G2D_BR_REVERSE_BGR			1

//
// draw type
//
#define	G2D_DRAW_TYPE_LINE			0
#define	G2D_DRAW_TYPE_RECT			1

//
// rotate type
//
#define	G2D_ROT_TYPE_C90			0
#define	G2D_ROT_TYPE_C180			1
#define	G2D_ROT_TYPE_C270			2
#define	G2D_ROT_TYPE_MIRROR			3
#define	G2D_ROT_TYPE_FLIP			4

//
// mem operator type
//
#define	G2D_MEM_TYPE_CPY			0
#define	G2D_MEM_TYPE_SET			1
#define	G2D_MEM_TYPE_CLR			2

//
// mem set data bitwidth type
//
#define	G2D_SETDATA_TYPE_BITS8			0
#define	G2D_SETDATA_TYPE_BITS16			1
#define	G2D_SETDATA_TYPE_BITS32			2
#define	G2D_SETDATA_TYPE_BITS64			3


//
// color key match mode, match background and display(or blending with)forground, or opposite.
//
#define G2D_COLORKEY_DIR_MATCH_FORGROUND	0
#define G2D_COLORKEY_DIR_MATCH_BACKGROUND	1

//
// Alpha-rop work type
//
#define G2D_AR_TYPE_SCLSRC						0	//scale(src)
#define G2D_AR_TYPE_AFA_SCLSRC_DEST				1	//alpha(scale(src),dst)
#define G2D_AR_TYPE_AFA_SCLSRC_PAT				2	//alpha(scale(src),pat)
#define G2D_AR_TYPE_ROP_SCLSRC_PAT				3	//rop3(scale(src),pat)
#define G2D_AR_TYPE_ROP_SCLSRC_DEST_PAT			4	//rop3(scale(src),dst,pat)
#define G2D_AR_TYPE_ROP_AFA_SCLSRC_DEST_PAT		5	//rop3(alpha(scale(src),dst),pat), has bug, not support
#define G2D_AR_TYPE_ROP_AFA_SCLSRC_PAT_DEST		6	//rop3(alpha(scale(src),pat),dst), has bug, not support
#define G2D_AR_TYPE_AFA_ROP_SCLSRC_PAT_DEST		7	//alpha(rop3(scale(src),pat),dst), has bug, not support
#define G2D_AR_TYPE_AFA_ROP_SCLSRC_DEST_PAT		8	//alpha(rop3(scale(src),dst),pat), has bug, not support
#define G2D_AR_TYPE_ROP_SCLSRC_DEST				9	//rop3(scale(src),dst)

//#############################################################################

//
//g2d dither
//
typedef struct{		
	IM_BOOL			dithEn;	
	IM_BOOL			tempoEn;	
	IM_UINT32		width;
	IM_UINT32		height;
	IM_UINT32		rChannel;
	IM_UINT32		gChannel;
	IM_UINT32		bChannel;
	IM_UINT32   	coefType;	//G2D_DITH_COEF_TYPE_XXX
}G2D_DITHER;

//
//g2d sclee
//
typedef struct{	
	IM_UINT32	hTh;
	IM_UINT32	vTh;
	IM_UINT32	d0Th;
	IM_UINT32	d1Th;
}G2D_EE_TH_MATRIX;

typedef struct{		
	//SCL
	IM_BOOL			verEnable;
	IM_BOOL			horEnable;
	IM_UINT32		vrdMode;
	IM_UINT32		hrdMode;
	IM_UINT32		paramType;
	IM_UINT32		inWidth;
	IM_UINT32		inHeight;
	IM_UINT32		outWidth;
	IM_UINT32		outHeight;
	
	//EE
	IM_BOOL			eeEnable;
	IM_UINT32		order;
	IM_UINT32		coefw;
	IM_UINT32		coefa;
	IM_UINT32		rdMode;	
	IM_BOOL			denosEnable;
	IM_UINT32		gasMode;
	IM_UINT32		errTh;
	IM_UINT32		opMatType;
	G2D_EE_TH_MATRIX	thrMat;
}G2D_SCLEE;

//
// g2d bit swap.
//
typedef struct{
	IM_UINT32	bitAddr;
	IM_BOOL 	bitSwap;
	IM_BOOL 	bits2Swap;
	IM_BOOL 	bits4Swap;
	IM_BOOL 	byteSwap;
	IM_BOOL 	halfwordSwap;
}G2D_BIT_SWAP;

//
// g2d .
//
typedef struct{
	IM_UINT32	fulWidth;	// pixel unit.
	IM_UINT32	fulHeight;	// pixel unit.
	IM_UINT32	xOffset;	// pixel unit.
	IM_UINT32	yOffset;	// pixel unit.
}G2D_VM;

//
// g2d csc matrix configuration structure.
//
typedef struct{
	IM_UINT32 	oft_b;		// range 0--63
	IM_UINT32	oft_a;		// range 0--63
	IM_INT32	coef11;		// range -2048--2047
	IM_INT32	coef12;		// range -2048--2047
	IM_INT32	coef13;		// range -2048--2047
	IM_INT32	coef21;		// range -2048--2047
	IM_INT32	coef22;		// range -2048--2047
	IM_INT32	coef23;		// range -2048--2047
	IM_INT32	coef31;		// range -2048--2047
	IM_INT32	coef32;		// range -2048--2047
	IM_INT32	coef33;		// range -2048--2047
}G2D_CSC_MATRIX;

//
// palette.
//
typedef struct{
	IM_UINT32	palFormat;			//G2D_PALATTE_FORMAT_XXX
	IM_UINT32	tableLength;		// it's not bytes, bug depth, in 4bytes.
	IM_UINT32	*table;
}G2D_PALETTE;


//
// g2d src channel info.
//
typedef struct{
	IM_UINT32		imgFormat;	//G2D_IMAGE_XXX
	G2D_PALETTE		palette;
	IM_UINT32	 	brRevs;		//G2D_BR_REVERSE_RGB
	IM_UINT32		width;
	IM_UINT32		height;
	G2D_VM			vm;
	G2D_CSC_MATRIX 	cscMat;
	G2D_BIT_SWAP 	ySwap;
	G2D_BIT_SWAP 	uvSwap;
	IM_Buffer		buffY;
	IM_Buffer		buffUV;
}G2D_SRC_CHANNEL;

//
// g2d dst channel info.
//
typedef struct{
	IM_UINT32		imgFormat;	//G2D_IMAGE_XXX
	G2D_PALETTE		palette;
	IM_UINT32	 	brRevs;		//G2D_BR_REVERSE_RGB
	IM_UINT32		width;
	IM_UINT32		height;
	G2D_VM			vm;
	G2D_BIT_SWAP 	swap;
	IM_Buffer		buff;
}G2D_DST_CHANNEL;

//
// g2d pattern channel info.
//
typedef struct{
	IM_UINT32		width;			//width<=8
	IM_UINT32	 	height;			//height<=8
	IM_UINT32		tableLength;	// it's not bytes, bug depth, in 4bytes.
	IM_UINT32		*table;
}G2D_PAT_CHANNEL;

//
// g2d out channel info.
//
typedef struct{
	IM_UINT32		imgFormat;	//G2D_IMAGE_XXX
	IM_UINT32		width;
	IM_UINT32		height;
	G2D_VM			vm;
	G2D_BIT_SWAP 	swap;
	IM_Buffer		buff;
}G2D_OUT_CHANNEL;

//
// g2d draw struct.
//
typedef struct{
	IM_BOOL			enable;
	IM_BOOL			footEn;
	IM_UINT32		lineWidth;
	IM_UINT32		type;		//G2D_DRAW_TYPE_XXX
	IM_UINT32		xStart;
	IM_UINT32		yStart;
	IM_UINT32		xEnd;
	IM_UINT32		yEnd;
	IM_UINT32		mask;	//mask point
	IM_UINT32		data;	//line color
}G2D_DRAW;

//
// g2d rotate struct.
//
typedef struct{
	IM_BOOL			enable;
	IM_UINT32		type;	//G2D_ROT_TYPE_XXX
}G2D_ROTATE;

//
// g2d mem copy/set/clear operator struct.
//
typedef struct{
	IM_BOOL			enable;
	IM_UINT32		type;	//G2D_MEM_TYPE_XXX
	IM_UINT32		length;
	IM_UINT32		setDataL32;
	IM_UINT32		setDataH32;
	IM_UINT32		setDataType; //G2D_SETDATA_TYPE_XXX
	IM_Buffer		buff;
}G2D_MEM_OP;

//
// g2d ROP3/ALPHA/colorkey operator struct.
//
typedef struct{
	IM_BOOL			ckEn;
	IM_BOOL			blendEn;
	IM_UINT32		matchMode;	// G2D_COLORKEY_DIR_MATCH_xxx.
	IM_UINT32		mask;		// format is RGB888.
	IM_UINT32		color;		// format is RGB888.
	IM_BOOL			gAlphaEn;
	IM_UINT32		gAlphaA;	//(set value)0--255<==>(real value)0--1.0
	IM_UINT32		gAlphaB;
	IM_UINT32		arType;	//G2D_AR_TYPE_XXX
	IM_UINT32		ropCode;

	G2D_SCLEE		sclee;
}G2D_ALPHA_ROP;


//==============================interface=====================================

IM_RET g2dlib_init(void);
IM_RET g2dlib_deinit(void);
IM_RET g2dlib_reset(void);

IM_RET g2dlib_set_property(IM_UINT32 property, void *ctx, G2D_SRC_CHANNEL *srcCh,
							G2D_DST_CHANNEL *dstCh, G2D_PAT_CHANNEL *patCh, G2D_OUT_CHANNEL *outCh);
IM_RET g2dlib_set_dith(G2D_DITHER *dith);
IM_RET g2dlib_set_global_alpha(IM_UINT32 gAlphaA, IM_UINT32 gAlphaB);
IM_RET g2dlib_exe(void);

#endif//__G2D_LIB_H__	
