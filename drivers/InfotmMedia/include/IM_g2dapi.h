/*--------------------------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
----------------------------------------------------------------------------------------------------
--	@file IM_g2dapi.h
--
--  Description :
--		This file defines g2d application interface.
--	  	Four operator property supported in the G2D module:
--		1.memory operator include three types:
--	 	 	1) memcpy
-- 			2) memset(per bytes/2bytes/4bytes/8bytes)
-- 			3) memclr
-- 		2.draw operator include three draw types and some additional functions:
-- 			1) draw point
-- 			2) draw line
-- 			3) draw rectangle
--			4) image size crop
--			5) image format convert
--			6) dither, can only reduce channel bites such as rgb888->rgb565, reduce bits(3 2 3)
-- 		3.rotate operator include five rotate types and some additional functions:
-- 			1) 90 degree rotation
-- 			2) 180 degree rotation
-- 			3) 270 degree rotation
-- 			4) mirror rotation
-- 			5) flip rotation
--			6) image size crop
--			7) image format convert
--			8) dither, can only reduce channel bites such as rgb888->rgb565, reduce bits(3 2 3)
--		4.alpha-rop operator include several function and their combination
--			1) scaler with image enhancement(ie is enable all the time)
--			2) alpha blending
--			3) colorkey
--			4) raster operator
--			5) image size crop
--			6) image format convert
--			7) dither, can only reduce channel bites such as rgb888->rgb565, reduce bits(3 2 3)
-
--	Note: 
--		1. All valid buffer set to g2d must be physical linear or dev-mmu address buffer, it means 
--			that (buffer.flag&IM_BUFFER_FLAG_PHY) != 0 or (buffer.flag&IM_BUFFER_FLAG_DEVADDR) != 0.
--		
--		2. Src1 and src2 format support RB reverse and bitswap, while dst format only support bitswap,
--			so some other input/output format which not list here maybe also support by RB reverse & bitswap.
--
--		3. all input/output buffer aligment is aimed at real(cropped) buffer.
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
----------------------------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8 & android-4.0.
-- v1.0.3	arsor@2012/04/18: mmu test ok and fixed poll timeout bug.
--
------------------------------------------------------------------------------*/

#ifndef __IM_G2DAPI_H__
#define __IM_G2DAPI_H__


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef G2DAPI_EXPORTS
	#define G2D_API		__declspec(dllexport)	/* For dll lib */
#else
	#define G2D_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define G2D_API
#endif	
//#############################################################################

typedef void *	G2DINST;	// g2d instance.

//g2d operator property
#define G2D_OPP_MEM			0
#define G2D_OPP_DRAW		1
#define G2D_OPP_ROTATE		2
#define G2D_OPP_ALPHAROP	3

//mem operator type
#define G2D_MEM_CPY			0
#define G2D_MEM_SET			1
#define G2D_MEM_CLR			2
//memset bytes unit
#define G2D_MEMSET_1BYTE	0
#define G2D_MEMSET_2BYTES	1
#define G2D_MEMSET_4BYTES	2
#define G2D_MEMSET_8BYTES	3

//draw type
#define G2D_DRAW_POINT		0
#define G2D_DRAW_LINE		1
#define G2D_DRAW_RECT		2
//line style
#define G2D_LINE_REAL		0
#define G2D_LINE_BROKEN		1

//rotate type
#define G2D_ROTATE_C90		0
#define G2D_ROTATE_C180		1
#define G2D_ROTATE_C270		2
#define G2D_ROTATE_MIRROR	3
#define G2D_ROTATE_FLIP		4


//alpharop function type 
//G2D_AROP_ALPHA and G2D_AROP_RASTER set at same time is not allowed
#define G2D_AROP_SCL	(1<<0)
#define G2D_AROP_EE		(1<<1)
#define G2D_AROP_ALPHA	(1<<2)
#define G2D_AROP_RASTER	(1<<3)

//alpha type
#define G2D_ALPHA_SRC_SRC		0
#define G2D_ALPHA_SRC_PAT		1

//
// alpha blending mode.
//
#define G2D_BLEND_MODE_PLANE	0x0
#define G2D_BLEND_MODE_PIXEL	0x1

//color key match mode, match background and display(or blending with)forground, or opposite.
#define G2D_COLORKEY_MATCH_FORGROUND	0
#define G2D_COLORKEY_MATCH_BACKGROUND	1

//rop type
#define G2D_ROP_SRC_SRC			0
#define G2D_ROP_SRC_PAT			1
#define G2D_ROP_SRC_SRC_PAT		2

//flag type
#define G2D_FLAG_USE_DEVMMU		(1<<0) //whether use devmmu, if set this flag, the IM_Buffer's phy_addr is the devaddr, else is physical address.

//
// g2d mem config struct.
//
/**
 * restrained condition:
 * 1. address of srcBuf must be 8bytes alignment when size>(2048-2)*8bytes for memcpy operator
 */
typedef struct{
	IM_UINT32	type;			//G2D_MEM_XXX
	IM_UINT32	size;			//if memset 4 bytes, the real set bytes = 4*size, so with memset 2bytes/8bytes
	IM_INT32	setValueL;		//value of memset low 4 bytes, only used for memset
	IM_INT32	setValueH;		//value of memset high 4 bytes, only used for memset 8bytes
	IM_UINT32	setUnitByte;	//G2D_MEMSET_XXX, only used for memset
	IM_Buffer	srcBuf;			//not used for memset and memclr
	IM_Buffer	dstBuf;		
}g2d_mem_t;


/* Clip Rectangle */
typedef struct{
	IM_UINT32	left;
	IM_UINT32	top;
	IM_UINT32	width;
	IM_UINT32	height; 
}g2d_rect_t;

/*palette*/
/**
* only support palette format as follow:
* 	IM_PIC_FMT_32BITS_ARGB_1888
* 	IM_PIC_FMT_32BITS_0RGB_8888
* 	IM_PIC_FMT_32BITS_ARGB_1666
* 	IM_PIC_FMT_32BITS_ARGB_1665
* 	IM_PIC_FMT_32BITS_0RGB_D666
* 	IM_PIC_FMT_16BITS_ARGB_1555
* 	IM_PIC_FMT_16BITS_RGB_565
*/
typedef struct{
	IM_UINT32	format;	//IM_PIC_FMT_XXX
	IM_UINT32	tableLength;
	IM_UINT32	*table;
}g2d_palette_t;

/* Image info structure */
typedef struct{
	IM_UINT32		format;//IM_PIC_FMT_XXX	
	g2d_palette_t	palette;
	IM_UINT32		width;
	IM_UINT32		height;
	g2d_rect_t		clipRect;
	IM_Buffer		buffer[3];//buffer[0] for rgb or y, buffer[1] for uv of semiplaner, not need buffer[2] because planer not support
}g2d_image_t;

//dither
typedef struct{
	IM_BOOL			dithEn;	
	IM_UINT32		rChReduceBits;
	IM_UINT32		gChReduceBits;
	IM_UINT32		bChReduceBits;
}g2d_dither_t;

//
// g2d draw config struct.
//
/**
 * restrained condition:
 * 1. draw input image format supports as follow:
 * 		IM_PIC_FMT_16BITS_RGB_565
 * 		IM_PIC_FMT_16BITS_ARGB_1555
 * 		IM_PIC_FMT_16BITS_IRGB_1555
 * 		IM_PIC_FMT_32BITS_0RGB_D666
 * 		IM_PIC_FMT_32BITS_ARGB_1665
 * 		IM_PIC_FMT_32BITS_ARGB_1666
 * 		IM_PIC_FMT_32BITS_0RGB_8888
 * 		IM_PIC_FMT_32BITS_0BGR_8888 //need RB reverse to 0RGB8888
 * 		IM_PIC_FMT_32BITS_ARGB_1887
 * 		IM_PIC_FMT_32BITS_ARGB_1888
 * 		IM_PIC_FMT_32BITS_ARGB_4888
 * 		IM_PIC_FMT_32BITS_RGBA_8888
 * 		IM_PIC_FMT_32BITS_ABGR_8888	//need bitswap to RGBA8888
 * 		IM_PIC_FMT_16BITS_RGBA_5551
 * 		IM_PIC_FMT_16BITS_RGBI_5551
 * 		IM_PIC_FMT_32BITS_ARGB_8888
 * 		IM_PIC_FMT_16BITS_0RGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_16BITS_RGBA_4444 //need bitswap & RB reverse to ARGB4444
 * 		IM_PIC_FMT_32BITS_ARGB_1565
 * 		IM_PIC_FMT_32BITS_ARGB_8565
 *
 * 2. draw output image format supports as follow:
 * 		IM_PIC_FMT_16BITS_RGB_565
 * 		IM_PIC_FMT_16BITS_ARGB_1555
 * 		IM_PIC_FMT_16BITS_IRGB_1555
 * 		IM_PIC_FMT_32BITS_0RGB_D666
 * 		IM_PIC_FMT_32BITS_ARGB_1665
 * 		IM_PIC_FMT_32BITS_ARGB_1666
 * 		IM_PIC_FMT_32BITS_0RGB_8888
 * 		IM_PIC_FMT_32BITS_ARGB_1887
 * 		IM_PIC_FMT_32BITS_ARGB_1888
 * 		IM_PIC_FMT_32BITS_ARGB_4888
 * 		IM_PIC_FMT_32BITS_RGBA_8888
 * 		IM_PIC_FMT_32BITS_ABGR_8888	//need bitswap from RGBA8888
 * 		IM_PIC_FMT_16BITS_RGBA_5551
 * 		IM_PIC_FMT_16BITS_RGBI_5551
 * 		IM_PIC_FMT_32BITS_ARGB_8888
 * 		IM_PIC_FMT_16BITS_0RGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_32BITS_ARGB_1565
 * 		IM_PIC_FMT_32BITS_ARGB_8565
 *
 * 3. input buffer(cropped imgSrc) alignment:
 * 		2bytes alignment for 16bpp-format of input image
 * 		4bytes alignment for 32bpp-format of input image
 * 		
 * 4. output buffer(cropped imgSrc or imgDst) alignment:
 * 		8bytes alignment for all-format of output image
 *	
 * 5. output(cropped imgSrc or imgDst) resolution:
 * 		out image width must be multiple of 4 for 16bpp-format of output image
 * 		out image width must be multiple of 2 for 32bpp-format of output image
 * 		
 * 6. input/output(orginal img) resolution:
 * 		in/out orginal image width must be multiple of 2.
 * 		
 * 7. draw line:
 * 		line-width must be 1 for oblique line and slope not support 0<k<1(0<angle<45) 
 * 		
 * 8. draw rectangle:
 * 		broken line not support 
 * 		
 */
typedef struct{
	IM_UINT32		type;		//G2D_DRAW_XXX
	IM_UINT32		lineWidth;
	IM_UINT32		lineStyle;	//G2D_LINE_XXX
	IM_UINT32		mask;		//used to defined type of broken line 
	IM_UINT32		colorData;	//line color, such as red is 0xff0000
	IM_UINT32		xStart;
	IM_UINT32		yStart;
	IM_UINT32		xEnd;
	IM_UINT32		yEnd;
	g2d_dither_t	dith;
	g2d_image_t		imgSrc;
	g2d_image_t		imgDst;		//imgDst.buffer[0].size must set to 0 if draw direct in the source image
}g2d_draw_t;

//
// g2d rotate config struct.
//
/**
 * restrained condition:
 * 1. rotate input image format supports:
 *		 IM_PIC_FMT_16BITS_RGB_565
 *		 IM_PIC_FMT_16BITS_ARGB_1555
 *		 IM_PIC_FMT_16BITS_IRGB_1555
 *		 IM_PIC_FMT_32BITS_0RGB_D666
 *		 IM_PIC_FMT_32BITS_ARGB_1665
 *		 IM_PIC_FMT_32BITS_ARGB_1666
 *		 IM_PIC_FMT_32BITS_0RGB_8888
 * 		 IM_PIC_FMT_32BITS_0BGR_8888 //need RB reverse to 0RGB8888
 *		 IM_PIC_FMT_32BITS_ARGB_1887
 *		 IM_PIC_FMT_32BITS_ARGB_1888
 *		 IM_PIC_FMT_32BITS_ARGB_4888
 *		 IM_PIC_FMT_32BITS_RGBA_8888
 * 		 IM_PIC_FMT_32BITS_ABGR_8888 //need bitswap to RGBA8888
 *		 IM_PIC_FMT_16BITS_RGBA_5551
 *		 IM_PIC_FMT_16BITS_RGBI_5551
 *		 IM_PIC_FMT_32BITS_ARGB_8888
 *		 IM_PIC_FMT_16BITS_0RGB_4444
 *		 IM_PIC_FMT_16BITS_ARGB_4444
 * 		 IM_PIC_FMT_16BITS_RGBA_4444 //need bitswap & RB reverse to ARGB4444
 *		 IM_PIC_FMT_32BITS_ARGB_1565
 *		 IM_PIC_FMT_32BITS_ARGB_8565
 *		 IM_PIC_FMT_16BITS_YUV422I_YUYV
 *		 IM_PIC_FMT_16BITS_YUV422I_UYVY
 *		 IM_PIC_FMT_16BITS_YUV422I_YVYU
 *		 IM_PIC_FMT_16BITS_YUV422I_VYUY
 *		 IM_PIC_FMT_32BITS_YUV444I
 *
 *2. rotate output image format supports:
 *		 IM_PIC_FMT_16BITS_RGB_565
 *		 IM_PIC_FMT_16BITS_ARGB_1555
 *		 IM_PIC_FMT_16BITS_IRGB_1555
 *		 IM_PIC_FMT_32BITS_0RGB_D666
 *		 IM_PIC_FMT_32BITS_ARGB_1665
 *		 IM_PIC_FMT_32BITS_ARGB_1666
 *		 IM_PIC_FMT_32BITS_0RGB_8888
 *		 IM_PIC_FMT_32BITS_ARGB_1887
 *		 IM_PIC_FMT_32BITS_ARGB_1888
 *		 IM_PIC_FMT_32BITS_ARGB_4888
 *		 IM_PIC_FMT_32BITS_RGBA_8888
 * 		 IM_PIC_FMT_32BITS_ABGR_8888 //need bitswap from RGBA8888
 *		 IM_PIC_FMT_16BITS_RGBA_5551
 *		 IM_PIC_FMT_16BITS_RGBI_5551
 *		 IM_PIC_FMT_32BITS_ARGB_8888
 *		 IM_PIC_FMT_16BITS_0RGB_4444
 *		 IM_PIC_FMT_16BITS_ARGB_4444
 *		 IM_PIC_FMT_32BITS_ARGB_1565
 *		 IM_PIC_FMT_32BITS_ARGB_8565
 *
 * 3. input buffer(cropped imgSrc) alignment:
 * 		2bytes alignment for 16bpp-format of input image
 * 		4bytes alignment for 32bpp-format of input image
 * 		4bytes alignment for YUV422I or YUV444I input image
 * 		
 * 4. output buffer(cropped imgDst) alignment:
 * 		8bytes alignment for all-format of output image
 * 		
 * 5. output(cropped imgDst) resolution:
 * 		out image width must be multiple of 4 for 16bpp-format of output image
 * 		out image width must be multiple of 2 for 32bpp-format of output image
 * 		
 * 6. input/output(orginal img) resolution:
 * 		in/out orginal image width must be multiple of 2.
 *
 */

typedef struct{
	IM_UINT32		type;		//G2D_ROTATE_XXX
	g2d_dither_t	dith;
	g2d_image_t		imgSrc;
	g2d_image_t		imgDst;
}g2d_rotate_t;


// color key configuration.
typedef struct{
	IM_BOOL		enable;
	IM_BOOL 	enableBlend;
	IM_UINT32	matchMode;
	IM_UINT32	mask;
	IM_UINT32	color;
}g2d_colorkey_t;

typedef struct{
	IM_UINT32		type;			//G2D_ALPHA_XXX
	IM_BOOL			blendMode;		//G2D_BLEND_XXX
	IM_UINT32		srcAlpha;		//(0.0--1.0)*256, and set to 255 if this value > 255, only used for G2D_BLEND_MODE_PLANE
	IM_UINT32		dstAlpha;		//(0.0--1.0)*256, and set to 255 if this value > 255, only used for G2D_BLEND_MODE_PLANE
	g2d_colorkey_t  colorKey;
}g2d_alpha_t;

typedef struct{
	IM_UINT32		type;			//G2D_ROP_XXX
	IM_UINT32		ropCode;
}g2d_rop_t;

typedef struct{
	IM_UINT32		width;			//width<=8
	IM_UINT32	 	height;			//height<=8
	IM_UINT32		tableLength;	//tableLength >= width*height(usually equal) 
	IM_UINT32		*table;
}g2d_pattern_t;

//
// g2d alpharop config struct, alpha and rop can not enable at same time.
//
/**
 * restrained condition:
 * 1. alpharop input image1 format supports:
 * 		IM_PIC_FMT_1BITS_RGB_PAL
 * 		IM_PIC_FMT_2BITS_RGB_PAL
 * 		IM_PIC_FMT_4BITS_RGB_PAL
 * 		IM_PIC_FMT_8BITS_RGB_PAL
 * 		IM_PIC_FMT_8BITS_ARGB_1232
 * 		IM_PIC_FMT_16BITS_RGB_565
 * 		IM_PIC_FMT_16BITS_ARGB_1555
 * 		IM_PIC_FMT_16BITS_IRGB_1555
 * 		IM_PIC_FMT_32BITS_0RGB_D666
 * 		IM_PIC_FMT_32BITS_ARGB_1665
 * 		IM_PIC_FMT_32BITS_ARGB_1666
 * 		IM_PIC_FMT_32BITS_0RGB_8888
 * 		IM_PIC_FMT_32BITS_0BGR_8888 //need RB reverse to 0RGB8888
 * 		IM_PIC_FMT_32BITS_ARGB_1887
 * 		IM_PIC_FMT_32BITS_ARGB_1888
 * 		IM_PIC_FMT_32BITS_ARGB_4888
 * 		IM_PIC_FMT_32BITS_RGBA_8888
 * 		IM_PIC_FMT_32BITS_ABGR_8888	//need swap to RGBA8888
 * 		IM_PIC_FMT_16BITS_RGBA_5551
 * 		IM_PIC_FMT_16BITS_RGBI_5551
 * 		IM_PIC_FMT_32BITS_ARGB_8888
 * 		IM_PIC_FMT_16BITS_0RGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_16BITS_RGBA_4444 //need bitswap & RB reverse to ARGB4444
 * 		IM_PIC_FMT_32BITS_ARGB_1565
 * 		IM_PIC_FMT_32BITS_ARGB_8565
 * 		IM_PIC_FMT_16BITS_YUV422I_YUYV
 * 		IM_PIC_FMT_16BITS_YUV422I_UYVY
 * 		IM_PIC_FMT_16BITS_YUV422I_YVYU
 * 		IM_PIC_FMT_16BITS_YUV422I_VYUY
 * 		IM_PIC_FMT_32BITS_YUV444I
 * 		IM_PIC_FMT_16BITS_YUV422SP
 * 		IM_PIC_FMT_12BITS_YUV420SP
 *
 * 2. alpharop input image2 format supports:
 * 		IM_PIC_FMT_1BITS_RGB_PAL
 * 		IM_PIC_FMT_2BITS_RGB_PAL
 * 		IM_PIC_FMT_4BITS_RGB_PAL
 * 		IM_PIC_FMT_8BITS_RGB_PAL
 * 		IM_PIC_FMT_8BITS_ARGB_1232
 * 		IM_PIC_FMT_16BITS_RGB_565
 * 		IM_PIC_FMT_16BITS_ARGB_1555
 * 		IM_PIC_FMT_16BITS_IRGB_1555
 * 		IM_PIC_FMT_32BITS_0RGB_D666
 * 		IM_PIC_FMT_32BITS_ARGB_1665
 * 		IM_PIC_FMT_32BITS_ARGB_1666
 * 		IM_PIC_FMT_32BITS_0RGB_8888
 * 		IM_PIC_FMT_32BITS_0BGR_8888 //need RB reverse to 0RGB8888
 * 		IM_PIC_FMT_32BITS_ARGB_1887
 * 		IM_PIC_FMT_32BITS_ARGB_1888
 * 		IM_PIC_FMT_32BITS_ARGB_4888
 * 		IM_PIC_FMT_32BITS_RGBA_8888
 * 		IM_PIC_FMT_32BITS_ABGR_8888	//need swap to RGBA8888
 * 		IM_PIC_FMT_16BITS_RGBA_5551
 * 		IM_PIC_FMT_16BITS_RGBI_5551
 * 		IM_PIC_FMT_32BITS_ARGB_8888
 * 		IM_PIC_FMT_16BITS_0RGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_16BITS_RGBA_4444 //need bitswap & RB reverse to ARGB4444
 * 		IM_PIC_FMT_32BITS_ARGB_1565
 * 		IM_PIC_FMT_32BITS_ARGB_8565
 *
 * 3. alpharop output image format supports:
 * 		IM_PIC_FMT_16BITS_RGB_565
 * 		IM_PIC_FMT_16BITS_ARGB_1555
 * 		IM_PIC_FMT_16BITS_IRGB_1555
 * 		IM_PIC_FMT_32BITS_0RGB_D666
 * 		IM_PIC_FMT_32BITS_ARGB_1665
 * 		IM_PIC_FMT_32BITS_ARGB_1666
 * 		IM_PIC_FMT_32BITS_0RGB_8888
 * 		IM_PIC_FMT_32BITS_ARGB_1887
 *		IM_PIC_FMT_32BITS_ARGB_1888
 * 		IM_PIC_FMT_32BITS_ARGB_4888
 * 		IM_PIC_FMT_32BITS_RGBA_8888
 * 		IM_PIC_FMT_32BITS_ABGR_8888	//need bitswap from RGBA8888
 * 		IM_PIC_FMT_16BITS_RGBA_5551
 * 		IM_PIC_FMT_16BITS_RGBI_5551
 * 		IM_PIC_FMT_32BITS_ARGB_8888
 * 		IM_PIC_FMT_16BITS_0RGB_4444
 * 		IM_PIC_FMT_16BITS_ARGB_4444
 * 		IM_PIC_FMT_32BITS_ARGB_1565
 * 		IM_PIC_FMT_32BITS_ARGB_8565
 *
 * 4. input buffer(cropped imgSrc1 and imgSrc2) alignment:
 * 		2bytes alignment for 16bpp-format of input image
 * 		4bytes alignment for 32bpp-format of input image
 * 		4bytes alignment for YUV422I or YUV444I input image
 * 		2bytes alignment for YUV422SP or YUV420SP input image
 * 		
 * 5. output buffer(cropped imgDst) alignment:
 * 		8bytes alignment for all-format of output image
 * 		
 * 6. output(cropped imgDst) resolution:
 * 		out image width must be multiple of 4 for 16bpp-format of output image
 * 		out image width must be multiple of 2 for 32bpp-format of output image
 * 		
 * 7. output(orginal imgDst) resolution:
 * 		in/out orginal image width must be multiple of 2.
 *
 */

typedef struct{
	IM_UINT32		type;	//G2D_AROP_XXX
	g2d_alpha_t		alpha;
	g2d_rop_t		rop;
	g2d_image_t		imgSrc1;
	g2d_image_t		imgSrc2;
	g2d_pattern_t	imgPat;
	g2d_image_t		imgDst;
	g2d_dither_t	dith;
	IM_INT32		edgeSharpValue;
}g2d_alpharop_t;


/*============================Interface API==================================*/
G2D_API IM_UINT32 g2d_version(IM_TCHAR *ver_string);

G2D_API IM_RET g2d_init(OUT G2DINST *inst);
G2D_API IM_RET g2d_deinit(IN G2DINST inst);

/**
 *  property = G2D_OPP_XXX, relevant config should set for each property setting:
 *  flag = G2D_FLAG_XXX | G2D_FLAG_XXX...
 *  1. G2D_OPP_MEM: config = (void *)(g2d_mem_t * memCfg)
 *  2. G2D_OPP_DRAW: config = (void *)(g2d_draw_t * drawCfg)
 *  3. G2D_OPP_ROTATE: config = (void *)(g2d_rotate_t * rotateCfg)
 *  4. G2D_OPP_ALPHAROP: config = (void *)(g2d_alpharop_t * alpharopCfg)
 */
G2D_API IM_RET g2d_set_config(IN G2DINST inst, IN IM_INT32 property, IN IM_UINT32 flag, IN void *config);

/**
 * do sync call when taskId = -1, 
 * do async call when taskId >= 0, and taskId is task number. 
 */
G2D_API IM_RET g2d_do(IN G2DINST inst, IN IM_INT32 taskId);

//only used for asyn call, taskId is the completed task number
G2D_API IM_RET g2d_wait_complete(IN G2DINST inst, OUT IM_INT32 *taskId);



//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_G2DAPI_H__


