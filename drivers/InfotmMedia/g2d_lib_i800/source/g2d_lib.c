/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_lib.c
--
--  Description :
--		This file implements g2d library for infotm g2d hardware.
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

#include <InfotmMedia.h>
#include <g2d_pwl.h>
#include <g2d_lib.h>
#include "g2d.h"
#include "g2d_reg_drv.h"
#include "g2d_sclee.h"
#include "g2d_dith.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"G2DLIB_I:"
#define WARNHEAD	"G2DLIB_W:"
#define ERRHEAD		"G2DLIB_E:"
#define TIPHEAD		"G2DLIB_T:"

//
//g2d palette/pattern look-up table offset
//
#define G2D_SRC_MONO_LUT_BASE		0x5000	//table length <=256
#define G2D_DEST_MONO_LUT_BASE		0x6000  //table length <=256
#define G2D_PAT_LUT_BASE		  	0x7000	//table length <=64

//global g2d context
G2D *g2d = IM_NULL;


/*------------------------------------------------------------------------------
    Function name   , g2d_refresh_regs
    Description     , update shadow registers from real register
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2d_refresh_regs(void)
{
    IM_INT32 i;
    IM_UINT32 *pRegVal = g2d->regVal;
	IM_UINT32 *pRegOft = g2d->regOft;
	IM_ASSERT(g2d != IM_NULL);
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
    for(i = 0; i < G2DREGNUM; i++){
		IM_JIF(g2dpwl_read_reg(pRegOft[i], pRegVal+i));
    }

	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;

}

/*------------------------------------------------------------------------------
    Function name   , g2d_flush_regs
    Description     , Flush shadow register to real register
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2d_flush_regs(void)
{
    IM_INT32 i;
    IM_UINT32 *pRegVal = g2d->regVal;
	IM_UINT32 *pRegOft = g2d->regOft;
	IM_ASSERT(g2d != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set source palette table to memory
	if(g2d->setSrcLut == IM_TRUE){
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_SRC_MONO, 0);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));

		for(i=0;i < g2d->srcChannel.palette.tableLength;i++){
			IM_JIF(g2dpwl_write_reg((G2D_SRC_MONO_LUT_BASE+4*i), g2d->srcChannel.palette.table[i]));
		}

		SetG2DRegister(g2d->regVal, G2D_LUT_READY_SRC_MONO, 1);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		g2d->setSrcLut = IM_FALSE;
	}

	//set dst palette table to memory
	if(g2d->setDstLut == IM_TRUE){
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_DEST_MONO, 0);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		for(i=0;i < g2d->dstChannel.palette.tableLength;i++){
			IM_JIF(g2dpwl_write_reg((G2D_DEST_MONO_LUT_BASE+4*i), g2d->dstChannel.palette.table[i]));
		}
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_DEST_MONO, 1);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		g2d->setDstLut = IM_FALSE;
	}

	//set pattern table to memory
	if(g2d->setPatLut == IM_TRUE){
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_PAT, 0);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		for(i=0;i < g2d->patChannel.tableLength;i++){
			IM_JIF(g2dpwl_write_reg((G2D_PAT_LUT_BASE+4*i), g2d->patChannel.table[i]));
		}
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_PAT, 1);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		g2d->setPatLut = IM_FALSE;
	}

	//only need to flush mem regs for mem operator
    for(i = 0; i < MEMREGNUM; i++){
    	IM_JIF(g2dpwl_write_reg(pRegOft[i], pRegVal[i]));
        //pRegVal[i] = 0;
    }
	/*need to flush other key regs for other operator
	 *note, write soft reset register at last*/  
	if(g2d->needKeyFlush == IM_TRUE){
    	for(i = MEMREGNUM + 1; i < KEYREGNUM; i++){
    		IM_JIF(g2dpwl_write_reg(pRegOft[i], pRegVal[i]));
        	//pRegVal[i] = 0;
    	}
	}

	//need to flush all regs as request
	if(g2d->needAllFlush == IM_TRUE){
    	for(i = KEYREGNUM; i < G2DREGNUM; i++){
    		IM_JIF(g2dpwl_write_reg(pRegOft[i], pRegVal[i]));
        	//pRegVal[i] = 0;
    	}
	}

	//enable/reset g2d module (falling edge enable)
	SetG2DRegister(g2d->regVal, G2D_SRC_XYST_SOFTRST, 0);
	IM_JIF(g2dpwl_write_reg(pRegOft[rG2D_SRC_XYST], pRegVal[rG2D_SRC_XYST]));

	SetG2DRegister(g2d->regVal, G2D_SRC_XYST_SOFTRST, 1);
	IM_JIF(g2dpwl_write_reg(pRegOft[rG2D_SRC_XYST], pRegVal[rG2D_SRC_XYST]));

	SetG2DRegister(g2d->regVal, G2D_SRC_XYST_SOFTRST, 0);
	IM_JIF(g2dpwl_write_reg(pRegOft[rG2D_SRC_XYST], pRegVal[rG2D_SRC_XYST]));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}


/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_src_channel
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_src_channel(G2D_SRC_CHANNEL *srcChannel)
{
	IM_UINT32 lutLen;
	IM_UINT32 lpixelOffset, l64bitPixOfst, lBuffOffset;
	IM_UINT32 y64bitPixOfst = 0, yBuffOffset = 0, uv64bitPixOfst = 0, uvBuffOffset = 0;
	IM_ASSERT(g2d != IM_NULL);
	IM_ASSERT(srcChannel != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	//set image format
	if(srcChannel->imgFormat <= G2D_IMAGE_PAL_BPP8){	//palette image format
		if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP1)
			lutLen = 2;
		else if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP2)
			lutLen = 4;
		else if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP4)
			lutLen = 16;
		else if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP8)
			lutLen = 256;
		else{
			IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), this is not palette imgFormat")));
			return IM_RET_INVALID_PARAMETER;
		}

		if(srcChannel->palette.table == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid palette table, NULL pointer")));
			return IM_RET_INVALID_PARAMETER;
		}

		if((srcChannel->palette.palFormat < G2D_PALATTE_FORMAT_A888)
			|| (srcChannel->palette.palFormat > G2D_PALATTE_FORMAT_565)){
			IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid palette imgFormat %d"), srcChannel->palette.palFormat));
			return IM_RET_INVALID_PARAMETER;
		}

		if((srcChannel->palette.tableLength > 256) || (srcChannel->palette.tableLength < lutLen)){
			IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid palette tablelength %d"), srcChannel->palette.tableLength));
			return IM_RET_INVALID_PARAMETER;
		}

		//set palette table to memory
		//move to flush
		g2d->setSrcLut = IM_TRUE;
		/*SetG2DRegister(g2d->regVal, G2D_LUT_READY_SRC_MONO, 0);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));

		for(i=0;i < srcChannel->palette.tableLength;i++){
			IM_JIF(g2dpwl_write_reg((G2D_SRC_MONO_LUT_BASE+4*i), srcChannel->palette.table[i]));
		}

		SetG2DRegister(g2d->regVal, G2D_LUT_READY_SRC_MONO, 1);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));*/

		//set imgFormat and palFormat
		SetG2DRegister(g2d->regVal, G2D_SRC_FORMAT_BPP, srcChannel->imgFormat);
		SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x0);//000, rgb , 001, yuv444 , 010, yuv422 , 100, yuv420
		SetG2DRegister(g2d->regVal, G2D_SRC_FORMAT_PAL, srcChannel->palette.palFormat);

		if(g2d->srcChannel.palette.table == IM_NULL){
			g2d->srcChannel.palette.tableLength = srcChannel->palette.tableLength;
			g2d->srcChannel.palette.table = (IM_UINT32 *)g2dpwl_malloc(g2d->srcChannel.palette.tableLength * 4);
		}else /*if(g2d->srcChannel.palette.tableLength < srcChannel->palette.tableLength)*/{
			g2dpwl_free(g2d->srcChannel.palette.table);
			g2d->srcChannel.palette.tableLength = srcChannel->palette.tableLength;
			g2d->srcChannel.palette.table = (IM_UINT32 *)g2dpwl_malloc(g2d->srcChannel.palette.tableLength * 4);
		}

		if(g2d->srcChannel.palette.table == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), failed to alloc src palette table")));
			return IM_RET_NOMEMORY;
		}

		g2d->srcChannel.palette.palFormat = srcChannel->palette.palFormat;
		g2dpwl_memcpy((void *)g2d->srcChannel.palette.table, (void *)srcChannel->palette.table, srcChannel->palette.tableLength * 4);

		g2d->srcChannel.imgFormat = srcChannel->imgFormat;


		// set buffer start address, 64-bit pixoffset and position
		lpixelOffset = srcChannel->vm.yOffset * srcChannel->vm.fulWidth + srcChannel->vm.xOffset;
		if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP1){//1bits-per-pixel
			lBuffOffset = lpixelOffset >> 3;
			l64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)<<3) + lpixelOffset) & 0x3f/*64 pixel in 64-bit*/;
		}else if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP2){//2bits-per-pixel
			lBuffOffset = lpixelOffset >> 2;
			l64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)<<2) + lpixelOffset) & 0x1f/*32 pixel in 64-bit*/;
		}else if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP4){//4bits-per-pixel
			lBuffOffset = lpixelOffset >> 1;
			l64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)<<1) + lpixelOffset) & 0xf/*16 pixel in 64-bit*/;
		}else /*if(srcChannel->imgFormat == G2D_IMAGE_PAL_BPP8)*/{//8bits-per-pixel
			lBuffOffset = lpixelOffset;
			l64bitPixOfst = ((srcChannel->buffY.phy_addr & 0x7) + lpixelOffset) & 0x7/*8 pixel in 64-bit*/;
		}

		SetG2DRegister(g2d->regVal, G2D_Y_ADDR_ST, lBuffOffset + srcChannel->buffY.phy_addr);
		SetG2DRegister(g2d->regVal, G2D_SRC_WIDOFFSET, l64bitPixOfst);

		g2d->srcChannel.buffY.phy_addr = srcChannel->buffY.phy_addr;

		//set BR REVERSE and bitswap
		SetG2DRegister(g2d->regVal, G2D_SRC_RGB_BRREV, srcChannel->brRevs);

		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BITADR, srcChannel->ySwap.bitAddr);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BITSWP, srcChannel->ySwap.bitSwap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BIT2SWP, srcChannel->ySwap.bits2Swap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_HFBSWP, srcChannel->ySwap.bits4Swap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BYTSWP, srcChannel->ySwap.byteSwap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_HFWSWP, srcChannel->ySwap.halfwordSwap);

		g2d->srcChannel.brRevs = srcChannel->brRevs;
		g2dpwl_memcpy((void *)(&g2d->srcChannel.ySwap), (void*)&srcChannel->ySwap, sizeof(G2D_BIT_SWAP));
	}else if(srcChannel->imgFormat <= G2D_IMAGE_RGB_BPP24_8A565){//other rgb image format
		//set imgFormat only
		SetG2DRegister(g2d->regVal, G2D_SRC_FORMAT_BPP, srcChannel->imgFormat);
		SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x0);//000, rgb , 001, yuv444 , 010, yuv422 , 100, yuv420
		g2d->srcChannel.imgFormat = srcChannel->imgFormat;

		// set buffer start address, 64-bit pixoffset and position
		lpixelOffset = srcChannel->vm.yOffset * srcChannel->vm.fulWidth + srcChannel->vm.xOffset;

		if(srcChannel->imgFormat == G2D_IMAGE_RGB_BPP8_1A232){//8bits-per-pixel
			lBuffOffset = lpixelOffset;
			l64bitPixOfst = ((srcChannel->buffY.phy_addr & 0x7) + lpixelOffset) & 0x7/*8 pixel in 64-bit*/;
		}else if((srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_565)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_1A555)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_I555)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555A)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555I)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP12_444)
					|| (srcChannel->imgFormat == G2D_IMAGE_RGB_BPP16_4A444)){//16bits-per-pixel
			if((srcChannel->buffY.phy_addr & 0x1) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid buf address =0x%x"), srcChannel->buffY.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			lBuffOffset = lpixelOffset << 1;
			l64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)>>1) + lpixelOffset) & 0x3/*4 pixel in 64-bit*/;
		}else{//32bits-per-pixel
			if((srcChannel->buffY.phy_addr & 0x3) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid buf address =0x%x"), srcChannel->buffY.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			lBuffOffset = lpixelOffset << 2;
			l64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)>>2) + lpixelOffset) & 0x1/*2 ixel in 64-bit*/;

		}

		SetG2DRegister(g2d->regVal, G2D_Y_ADDR_ST, lBuffOffset + srcChannel->buffY.phy_addr);
		SetG2DRegister(g2d->regVal, G2D_SRC_WIDOFFSET, l64bitPixOfst);

		g2d->srcChannel.buffY.phy_addr = srcChannel->buffY.phy_addr;

		//set BR REVERSE and bitswap
		SetG2DRegister(g2d->regVal, G2D_SRC_RGB_BRREV, srcChannel->brRevs);

		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BITADR, srcChannel->ySwap.bitAddr);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BITSWP, srcChannel->ySwap.bitSwap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BIT2SWP, srcChannel->ySwap.bits2Swap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_HFBSWP, srcChannel->ySwap.bits4Swap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_BYTSWP, srcChannel->ySwap.byteSwap);
		SetG2DRegister(g2d->regVal, G2D_SRC_BITSWP_HFWSWP, srcChannel->ySwap.halfwordSwap);

		g2d->srcChannel.brRevs = srcChannel->brRevs;
		g2dpwl_memcpy((void *)(&g2d->srcChannel.ySwap), (void*)&srcChannel->ySwap, sizeof(G2D_BIT_SWAP));
	}else if(srcChannel->imgFormat <= G2D_IMAGE_YUV_420SP){//yuv image format
		//enable yuv2rgb csc 
		SetG2DRegister(g2d->regVal, G2D_SRC_RGB_CSC_BYPASSS, 0x0);
		//000, rgb , 001, yuv444 , 010, yuv422 , 100, yuv420
		if(srcChannel->imgFormat == G2D_IMAGE_YUV_444I){
			SetG2DRegister(g2d->regVal, G2D_SRC_FORMAT_BPP, G2D_IMAGE_RGB_BPP32_A888);//must set a 32bpp format for G2D_IMAGE_YUV_444I
			SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x1);
		}else if((srcChannel->imgFormat == G2D_IMAGE_YUV_422I_YUYV)
				||(srcChannel->imgFormat == G2D_IMAGE_YUV_422I_UYVY)
				||(srcChannel->imgFormat == G2D_IMAGE_YUV_422I_YVYU)
				||(srcChannel->imgFormat == G2D_IMAGE_YUV_422I_VYUY)){
			SetG2DRegister(g2d->regVal, G2D_SRC_FORMAT_BPP, srcChannel->imgFormat);
			SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x0);
		}else if(srcChannel->imgFormat == G2D_IMAGE_YUV_422SP){
			SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x2);
		}else{//yuv420
			SetG2DRegister(g2d->regVal, G2D_SRC_RGB_YUVFMT, 0x4);
		}

		g2d->srcChannel.imgFormat = srcChannel->imgFormat;

		// set buffer start address, 64-bit pixoffset
		lpixelOffset = srcChannel->vm.yOffset * srcChannel->vm.fulWidth + srcChannel->vm.xOffset;

		if(srcChannel->imgFormat == G2D_IMAGE_YUV_444I){//32bits-per-pixel for 0yuv
			if((srcChannel->buffY.phy_addr & 0x3) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid buf address =0x%x"), srcChannel->buffY.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			yBuffOffset = lpixelOffset << 2;
			y64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)>>2) + lpixelOffset) & 0x1/*2 ixel in 64-bit*/;
		}else if((srcChannel->imgFormat == G2D_IMAGE_YUV_422I_YUYV) 
				|| (srcChannel->imgFormat == G2D_IMAGE_YUV_422I_UYVY)
				|| (srcChannel->imgFormat == G2D_IMAGE_YUV_422I_YVYU)
				|| (srcChannel->imgFormat == G2D_IMAGE_YUV_422I_VYUY)){//32bits-per-two pixel
			if((srcChannel->buffY.phy_addr & 0x1) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid buf address =0x%x"), srcChannel->buffY.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			yBuffOffset = lpixelOffset << 1;
			y64bitPixOfst = (((srcChannel->buffY.phy_addr & 0x7)>>1) + lpixelOffset) & 0x3/*4 ixel in 64-bit*/;
		}else if(srcChannel->imgFormat == G2D_IMAGE_YUV_422SP){//8bits-per-pixel for y, 16 bits-per-2pixel for uv
			if((lpixelOffset & 0x1) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid size, pixel offset must 2 pixel alignment for yuv422, vm.xOffset=%d, vm.yOffset=%d, vm.fulWidth=%d"),
					srcChannel->vm.xOffset, srcChannel->vm.yOffset, srcChannel->vm.fulWidth));
				return IM_RET_INVALID_PARAMETER;
			}
			yBuffOffset = lpixelOffset;
			y64bitPixOfst = ((srcChannel->buffY.phy_addr & 0x7) + lpixelOffset) & 0x7;/*8 pixel in 64-bit for y*/
			uvBuffOffset = lpixelOffset;
			uv64bitPixOfst = (((srcChannel->buffUV.phy_addr & 0x7) + lpixelOffset)>>1) & 0x3;/*4 pixel in 64-bit for uv*/
		}else{//yuv420, 8bits-per-pixel for y, 16 bits-per-4pixel for uv
			if(((lpixelOffset&0x1) != 0) || ((srcChannel->vm.yOffset&0x1) != 0)){
				IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid size, pixel offset must 2 pixel alignment and yoffset must 2 line alignment for yuv420, vm.xOffset=%d, vm.yOffset=%d, vm.fulWidth=%d"),
					srcChannel->vm.xOffset, srcChannel->vm.yOffset, srcChannel->vm.fulWidth));
				return IM_RET_INVALID_PARAMETER;
			}
			yBuffOffset = lpixelOffset;
			y64bitPixOfst = ((srcChannel->buffY.phy_addr & 0x7) + lpixelOffset) & 0x7/*8 pixel in 64-bit for y*/;
			uvBuffOffset = (srcChannel->vm.yOffset>>1) * srcChannel->vm.fulWidth + srcChannel->vm.xOffset;
			uv64bitPixOfst = (((srcChannel->buffUV.phy_addr & 0x7) + (srcChannel->vm.yOffset>>1) * srcChannel->vm.fulWidth + srcChannel->vm.xOffset)>>1) & 0x3;/*4 pixel in 64-bit for uv*/
		}
		SetG2DRegister(g2d->regVal, G2D_Y_ADDR_ST, yBuffOffset + srcChannel->buffY.phy_addr);
		if((srcChannel->imgFormat == G2D_IMAGE_YUV_422SP) || (srcChannel->imgFormat == G2D_IMAGE_YUV_420SP)){
			SetG2DRegister(g2d->regVal, G2D_YUV_OFFSET_Y, y64bitPixOfst);
		}else{
			SetG2DRegister(g2d->regVal, G2D_SRC_WIDOFFSET, y64bitPixOfst);
		}
		
		SetG2DRegister(g2d->regVal, G2D_UV_ADDR_ST, uvBuffOffset + srcChannel->buffUV.phy_addr);
		SetG2DRegister(g2d->regVal, G2D_YUV_OFFSET_UV, uv64bitPixOfst);

		g2d->srcChannel.buffY.phy_addr = srcChannel->buffY.phy_addr;
		g2d->srcChannel.buffUV.phy_addr = srcChannel->buffUV.phy_addr;

		//set y and uv bitswap
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_BITADR, srcChannel->ySwap.bitAddr);
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_BITSWP, srcChannel->ySwap.bitSwap);
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_BIT2SWP, srcChannel->ySwap.bits2Swap);
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_HFBSWP, srcChannel->ySwap.bits4Swap);
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_BYTSWP, srcChannel->ySwap.byteSwap);
		SetG2DRegister(g2d->regVal, G2D_Y_BITSWP_HFWSWP, srcChannel->ySwap.halfwordSwap);

		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_BITADR, srcChannel->uvSwap.bitAddr);
		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_BITSWP, srcChannel->uvSwap.bitSwap);
		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_BIT2SWP, srcChannel->uvSwap.bits2Swap);
		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_HFBSWP, srcChannel->uvSwap.bits4Swap);
		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_BYTSWP, srcChannel->uvSwap.byteSwap);
		SetG2DRegister(g2d->regVal, G2D_UV_BITSWP_HFWSWP, srcChannel->uvSwap.halfwordSwap);

		g2dpwl_memcpy((void *)(&g2d->srcChannel.ySwap), (void*)&srcChannel->ySwap, sizeof(G2D_BIT_SWAP));
		g2dpwl_memcpy((void *)(&g2d->srcChannel.uvSwap), (void*)&srcChannel->uvSwap, sizeof(G2D_BIT_SWAP));

		//set cscMat
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF1112_COEF11, srcChannel->cscMat.coef11);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF1112_COEF12, srcChannel->cscMat.coef12);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF1321_COEF13, srcChannel->cscMat.coef13);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF1321_COEF21, srcChannel->cscMat.coef21);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF2322_COEF22, srcChannel->cscMat.coef22);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF2322_COEF23, srcChannel->cscMat.coef23);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF3132_COEF31, srcChannel->cscMat.coef31);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF3132_COEF32, srcChannel->cscMat.coef32);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF33_OFT_COEF33, srcChannel->cscMat.coef33);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF33_OFT_OFTA, srcChannel->cscMat.oft_a);
		SetG2DRegister(g2d->regVal, G2D_SRC_COEF33_OFT_OFTB, srcChannel->cscMat.oft_b);
		g2dpwl_memcpy((void *)(&g2d->srcChannel.cscMat), (void*)&srcChannel->cscMat, sizeof(G2D_CSC_MATRIX));
	}else{
		IM_ERRMSG((IM_STR("@g2dlib_set_src_channel(), Invalid imgFormat %d"), srcChannel->imgFormat));
		return IM_RET_INVALID_PARAMETER;
	}

	//set width and height
	SetG2DRegister(g2d->regVal, G2D_SRC_SIZE_WIDTH, srcChannel->width);
	SetG2DRegister(g2d->regVal, G2D_SRC_SIZE_HEIGHT, srcChannel->height);
	SetG2DRegister(g2d->regVal, G2D_SRC_SIZE_1_WIDTH, srcChannel->width - 1);
	SetG2DRegister(g2d->regVal, G2D_SRC_SIZE_1_HEIGHT, srcChannel->height - 1);

	g2d->srcChannel.width = srcChannel->width;
	g2d->srcChannel.height = srcChannel->height;

	//set vm(position and size)
	SetG2DRegister(g2d->regVal, G2D_SRC_FULL_WIDTH, srcChannel->vm.fulWidth);
	SetG2DRegister(g2d->regVal, G2D_SRC_FULL_WIDTH_1, srcChannel->vm.fulWidth - 1);
	SetG2DRegister(g2d->regVal, G2D_SRC_XYST_XSTART, srcChannel->vm.xOffset);
	SetG2DRegister(g2d->regVal, G2D_SRC_XYST_YSTART, srcChannel->vm.yOffset);


	g2dpwl_memcpy((void *)(&g2d->srcChannel.vm), (void*)&srcChannel->vm, sizeof(G2D_VM));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;

}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_dst_channel
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_dst_channel(G2D_DST_CHANNEL *dstChannel)
{
	IM_UINT32 lutLen;
	IM_UINT32 lpixelOffset, l64bitPixOfst, lBuffOffset;
	IM_ASSERT(g2d != IM_NULL);
	IM_ASSERT(dstChannel != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set image format
	if(dstChannel->imgFormat <= G2D_IMAGE_PAL_BPP8){//palette image format
		if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP1)
			lutLen = 2;
		else if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP2)
			lutLen = 4;
		else if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP4)
			lutLen = 16;
		else if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP8)
			lutLen = 256;
		else{
			IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), this is not palette imgFormat")));
			return IM_RET_INVALID_PARAMETER;
		}

		if(dstChannel->palette.table == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Invalid palette table, NULL pointer")));
			return IM_RET_INVALID_PARAMETER;
		}

		if((dstChannel->palette.palFormat < G2D_PALATTE_FORMAT_A888)
			|| (dstChannel->palette.palFormat > G2D_PALATTE_FORMAT_565)){
			IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Invalid palette imgFormat %d"), dstChannel->palette.palFormat));
			return IM_RET_INVALID_PARAMETER;
		}

		if((dstChannel->palette.tableLength > 256) || (dstChannel->palette.tableLength < lutLen)){
			IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Invalid palette tablelength %d"), dstChannel->palette.tableLength));
			return IM_RET_INVALID_PARAMETER;
		}

		//set palette table to memory
		//move to flush
		g2d->setDstLut = IM_TRUE;
		/*SetG2DRegister(g2d->regVal, G2D_LUT_READY_DEST_MONO, 0);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
		for(i=0;i < dstChannel->palette.tableLength;i++){
			IM_JIF(g2dpwl_write_reg((G2D_DEST_MONO_LUT_BASE+4*i), dstChannel->palette.table[i]));
		}
		SetG2DRegister(g2d->regVal, G2D_LUT_READY_DEST_MONO, 1);
		IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));*/

		//set imgFormat and palFormat
		SetG2DRegister(g2d->regVal, G2D_DEST_FORMAT_BPP, dstChannel->imgFormat);
		SetG2DRegister(g2d->regVal, G2D_DEST_FORMAT_PAL, dstChannel->palette.palFormat);

		if(g2d->dstChannel.palette.table == IM_NULL){
			g2d->dstChannel.palette.tableLength = dstChannel->palette.tableLength;
			g2d->dstChannel.palette.table = (IM_UINT32 *)g2dpwl_malloc(g2d->dstChannel.palette.tableLength * 4);
		}else /*if(g2d->dstChannel.palette.tableLength < dstChannel->palette.tableLength)*/{
			g2dpwl_free(g2d->dstChannel.palette.table);
			g2d->dstChannel.palette.tableLength = dstChannel->palette.tableLength;
			g2d->dstChannel.palette.table = (IM_UINT32 *)g2dpwl_malloc(g2d->dstChannel.palette.tableLength * 4);
		}

		if(g2d->dstChannel.palette.table == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), failed to alloc dst palette table")));
			return IM_RET_NOMEMORY;
		}

		g2d->dstChannel.palette.palFormat = dstChannel->palette.palFormat;

		g2dpwl_memcpy((void *)g2d->dstChannel.palette.table, (void *)dstChannel->palette.table, dstChannel->palette.tableLength * 4);

		g2d->dstChannel.imgFormat = dstChannel->imgFormat;


		// set buffer start address, 64-bit pixoffset and position
		lpixelOffset = dstChannel->vm.yOffset * dstChannel->vm.fulWidth + dstChannel->vm.xOffset;
		if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP1){//1bits-per-pixel
			lBuffOffset = lpixelOffset >> 3;
			l64bitPixOfst = (((dstChannel->buff.phy_addr & 0x7)<<3) + lpixelOffset) & 0x3f/*64 pixel in 64-bit*/;
		}else if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP2){//2bits-per-pixel
			lBuffOffset = lpixelOffset >> 2;
			l64bitPixOfst = (((dstChannel->buff.phy_addr & 0x7)<<2) + lpixelOffset) & 0x1f/*32 pixel in 64-bit*/;
		}else if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP4){//4bits-per-pixel
			lBuffOffset = lpixelOffset >> 1;
			l64bitPixOfst = (((dstChannel->buff.phy_addr & 0x7)<<1) + lpixelOffset) & 0xf/*16 pixel in 64-bit*/;
		}else /*if(dstChannel->imgFormat == G2D_IMAGE_PAL_BPP8)*/{//8bits-per-pixel
			lBuffOffset = lpixelOffset;
			l64bitPixOfst = ((dstChannel->buff.phy_addr & 0x7) + lpixelOffset) & 0x7/*8 pixel in 64-bit*/;
		}

		SetG2DRegister(g2d->regVal, G2D_DEST_ADDR_ST, lBuffOffset + dstChannel->buff.phy_addr);
		SetG2DRegister(g2d->regVal, G2D_DEST_WIDOFFSET_OFFSET, l64bitPixOfst);

		g2d->dstChannel.buff.phy_addr = dstChannel->buff.phy_addr;
	}else if(dstChannel->imgFormat <= G2D_IMAGE_RGB_BPP24_8A565){
		//set imgFormat only
		SetG2DRegister(g2d->regVal, G2D_DEST_FORMAT_BPP, dstChannel->imgFormat);
		g2d->dstChannel.imgFormat = dstChannel->imgFormat;

		//set buffer start address, 64-bit pixoffset and position
		lpixelOffset = dstChannel->vm.yOffset * dstChannel->vm.fulWidth + dstChannel->vm.xOffset;

		if(dstChannel->imgFormat == G2D_IMAGE_RGB_BPP8_1A232){//8bits-per-pixel
			lBuffOffset = lpixelOffset;
			l64bitPixOfst = ((dstChannel->buff.phy_addr & 0x7) + lpixelOffset) & 0x7/*8 pixel in 64-bit*/;
		}else if((dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_565)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_1A555)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_I555)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555A)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555I)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP12_444)
					|| (dstChannel->imgFormat == G2D_IMAGE_RGB_BPP16_4A444)){//16bits-per-pixel
			if((dstChannel->buff.phy_addr & 0x1) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Invalid buf address =0x%x"), dstChannel->buff.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			lBuffOffset = lpixelOffset << 1;
			l64bitPixOfst = (((dstChannel->buff.phy_addr & 0x7)>>1) + lpixelOffset) & 0x3/*4 pixel in 64-bit*/;
		}else{//32bits-per-pixel
			if((dstChannel->buff.phy_addr & 0x3) != 0){
				IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Invalid buf address =0x%x"), dstChannel->buff.phy_addr));
				return IM_RET_INVALID_PARAMETER;
			}
			lBuffOffset = lpixelOffset << 2;
			l64bitPixOfst = (((dstChannel->buff.phy_addr & 0x7)>>2) + lpixelOffset) & 0x1/*2 pixel in 64-bit*/;
		}

		SetG2DRegister(g2d->regVal, G2D_DEST_ADDR_ST, lBuffOffset + dstChannel->buff.phy_addr);
		SetG2DRegister(g2d->regVal, G2D_DEST_WIDOFFSET_OFFSET, l64bitPixOfst);

		g2d->dstChannel.buff.phy_addr = dstChannel->buff.phy_addr;
	}else{
		IM_ERRMSG((IM_STR("@g2dlib_set_dst_channel(), Dest channel not support yuv format,Invalid imgFormat %d"), dstChannel->imgFormat));
		return IM_RET_INVALID_PARAMETER;
	}

	//set width and height
	SetG2DRegister(g2d->regVal, G2D_DEST_SIZE_WIDTH, dstChannel->width);
	SetG2DRegister(g2d->regVal, G2D_DEST_SIZE_HEIGHT, dstChannel->height);
	SetG2DRegister(g2d->regVal, G2D_DEST_SIZE_1_WIDTH, dstChannel->width - 1);
	SetG2DRegister(g2d->regVal, G2D_DEST_SIZE_1_HEIGHT, dstChannel->height - 1);

	g2d->dstChannel.width = dstChannel->width;
	g2d->dstChannel.height = dstChannel->height;

	//set vm(position and size)
	SetG2DRegister(g2d->regVal, G2D_DEST_FULL_WIDTH, dstChannel->vm.fulWidth);
	SetG2DRegister(g2d->regVal, G2D_DEST_FULL_WIDTH_1, dstChannel->vm.fulWidth - 1);
	SetG2DRegister(g2d->regVal, G2D_DEST_XYST_XSTART, dstChannel->vm.xOffset);
	SetG2DRegister(g2d->regVal, G2D_DEST_XYST_YSTART, dstChannel->vm.yOffset);

	g2dpwl_memcpy((void *)(&g2d->dstChannel.vm), (void*)&dstChannel->vm, sizeof(G2D_VM));

	//set BR REVERSE and bitswap
	SetG2DRegister(g2d->regVal, G2D_DEST_WIDOFFSET_BRREV, dstChannel->brRevs);

	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_BITADR, dstChannel->swap.bitAddr);
	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_BITSWP, dstChannel->swap.bitSwap);
	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_BIT2SWP, dstChannel->swap.bits2Swap);
	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_HFBSWP, dstChannel->swap.bits4Swap);
	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_BYTSWP, dstChannel->swap.byteSwap);
	SetG2DRegister(g2d->regVal, G2D_DEST_BITSWP_HFWSWP, dstChannel->swap.halfwordSwap);

	g2d->dstChannel.brRevs = dstChannel->brRevs;
	g2dpwl_memcpy((void *)(&g2d->dstChannel.swap), (void*)&dstChannel->swap, sizeof(G2D_BIT_SWAP));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;

}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_pat_channel
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_pat_channel(G2D_PAT_CHANNEL *patChannel)
{
	IM_ASSERT(g2d != IM_NULL);
	IM_ASSERT(patChannel != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set pattern size
	if((patChannel->width > 8) || (patChannel->width < 1)
			|| (patChannel->height > 8) || (patChannel->height < 1)){
		IM_ERRMSG((IM_STR("@g2dlib_set_pat_channel(), Invalid pattern size, width=%d, height=%d"), patChannel->width, patChannel->height));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_PAT_SIZE_WIDTH, patChannel->width);
	SetG2DRegister(g2d->regVal, G2D_PAT_SIZE_HEIGHT, patChannel->height);

	g2d->patChannel.width = patChannel->width;
	g2d->patChannel.height = patChannel->height;

	//set pattern look-up table
	if(patChannel->table == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dlib_set_pat_channel(), Invalid table, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	if((patChannel->tableLength > 64) || (patChannel->tableLength < (patChannel->width*patChannel->height))){
		IM_ERRMSG((IM_STR("@g2dlib_set_pat_channel(), Invalid pattern tablelength %d"), patChannel->tableLength));
		return IM_RET_INVALID_PARAMETER;
	}

	//set pattern table to memory
	//move to flush
	g2d->setPatLut = IM_TRUE;
	/*SetG2DRegister(g2d->regVal, G2D_LUT_READY_PAT, 0);
	IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));
	for(i=0;i < patChannel->tableLength;i++){
		IM_JIF(g2dpwl_write_reg((G2D_PAT_LUT_BASE+4*i), patChannel->table[i]));
	}
	SetG2DRegister(g2d->regVal, G2D_LUT_READY_PAT, 1);
	IM_JIF(g2dpwl_write_reg(g2d->regOft[rG2D_LUT_READY], g2d->regVal[rG2D_LUT_READY]));*/

	if(g2d->patChannel.table == IM_NULL){
		g2d->patChannel.tableLength = patChannel->tableLength;
		g2d->patChannel.table = (IM_UINT32 *)g2dpwl_malloc(g2d->patChannel.tableLength * 4);
	}else/*if(g2d->patChannel.tableLength < patChannel->tableLength)*/{
		g2dpwl_free(g2d->patChannel.table);
		g2d->patChannel.tableLength = patChannel->tableLength;
		g2d->patChannel.table = (IM_UINT32 *)g2dpwl_malloc(g2d->patChannel.tableLength * 4);
	}

	if(g2d->patChannel.table == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dlib_set_pat_channel(), failed to alloc pattern table")));
		return IM_RET_NOMEMORY;
	}

	g2dpwl_memcpy((void *)g2d->patChannel.table, (void *)patChannel->table, patChannel->tableLength * 4);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;

}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_out_channel
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_out_channel(G2D_OUT_CHANNEL *outChannel)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 lpixelOffset, lBuffOffset;
	IM_ASSERT(g2d != IM_NULL);
	IM_ASSERT(outChannel != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set imgFormat
 	if((outChannel->imgFormat >= G2D_IMAGE_RGB_BPP16_565)
		&& (outChannel->imgFormat <= G2D_IMAGE_RGB_BPP24_8A565)){
		SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_FORMAT_BPP, outChannel->imgFormat);
 	}else{
		IM_ERRMSG((IM_STR("@g2dlib_set_out_channel(), Invalid out imgFormat %d"), outChannel->imgFormat));
		return IM_RET_INVALID_PARAMETER;
	}

	//set bitswap
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_BITADR, outChannel->swap.bitAddr);
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_BITSWP, outChannel->swap.bitSwap);
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_BIT2SWP, outChannel->swap.bits2Swap);
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_HFBSWP, outChannel->swap.bits4Swap);
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_BYTSWP, outChannel->swap.byteSwap);
	SetG2DRegister(g2d->regVal, G2D_OUT_BITSWP_HFWSWP, outChannel->swap.halfwordSwap);

	g2dpwl_memcpy((void *)(&g2d->outChannel.swap), (void*)&outChannel->swap, sizeof(G2D_BIT_SWAP));

	//set width and height
	SetG2DRegister(g2d->regVal, G2D_IF_WIDTH, outChannel->width);
	SetG2DRegister(g2d->regVal, G2D_IF_WIDTH_1, outChannel->width - 1);
	SetG2DRegister(g2d->regVal, G2D_IF_HEIGHT, outChannel->height);
	SetG2DRegister(g2d->regVal, G2D_IF_HEIGHT_1, outChannel->height - 1);

	g2d->outChannel.width = outChannel->width;
	g2d->outChannel.height = outChannel->height;

	//set vm(position and size)
	SetG2DRegister(g2d->regVal, G2D_IF_FULL_WIDTH, outChannel->vm.fulWidth);
	SetG2DRegister(g2d->regVal, G2D_IF_FULL_WIDTH_1, outChannel->vm.fulWidth - 1);
	SetG2DRegister(g2d->regVal, G2D_IF_XYST_XSTART, outChannel->vm.xOffset);
	SetG2DRegister(g2d->regVal, G2D_IF_XYST_YSTART, outChannel->vm.yOffset);

	g2dpwl_memcpy((void *)(&g2d->outChannel.vm), (void*)&outChannel->vm, sizeof(G2D_VM));

	//set buffer start address
	lpixelOffset = outChannel->vm.yOffset * outChannel->vm.fulWidth + outChannel->vm.xOffset;
	if((outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_565)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_1A555)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_I555)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555A)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_555I)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP12_444)
		|| (outChannel->imgFormat == G2D_IMAGE_RGB_BPP16_4A444))// 16bpp(2bytes per pixel)
	{
		lBuffOffset = lpixelOffset << 1;
	}
	else// 32bpp(4bytes per pixel)
		lBuffOffset = lpixelOffset << 2;

	SetG2DRegister(g2d->regVal, G2D_IF_ADDR_ST, lBuffOffset + outChannel->buff.phy_addr);

	g2d->outChannel.buff.phy_addr = outChannel->buff.phy_addr;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_draw
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_draw(G2D_DRAW *draw)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set draw enable
	SetG2DRegister(g2d->regVal, G2D_ALU_CNTL_DRAW_EN, (draw->enable==IM_TRUE)?1:0);
	g2d->draw.enable = draw->enable;

	//set draw end enable
	SetG2DRegister(g2d->regVal, G2D_DRAW_CNTL_FOOT, (draw->footEn==IM_TRUE)?1:0);
	g2d->draw.footEn = draw->footEn;

	//set draw width
	if(draw->lineWidth > 0xf){
		IM_ERRMSG((IM_STR("@g2dlib_set_darw(), Invalid DRAW lineWidth=%d"), draw->lineWidth));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_DRAW_CNTL_WIDTH, draw->lineWidth);
	g2d->draw.lineWidth = draw->lineWidth;

	//set draw type
	if((draw->type != G2D_DRAW_TYPE_LINE)
		&&(draw->type != G2D_DRAW_TYPE_RECT)){
		IM_ERRMSG((IM_STR("@g2dlib_set_draw(), Invalid DRAW type=%d"), draw->type));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_DRAW_CNT_CODE, draw->type);
	g2d->draw.type = draw->type;

	//set draw pos
	if((draw->xStart > 4095) ||(draw->yStart > 4095)
		||(draw->xEnd > 4095) ||(draw->yEnd > 4095)){
		IM_ERRMSG((IM_STR("@g2dlib_set_draw(), Invalid pos,  xStart=%d,yStart=%d,xEnd=%d,yEnd=%d"),
							draw->xStart, draw->yStart, draw->xEnd, draw->yEnd));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_DRAW_START_X, draw->xStart);
	SetG2DRegister(g2d->regVal, G2D_DRAW_START_Y, draw->yStart);
	SetG2DRegister(g2d->regVal, G2D_DRAW_END_X, draw->xEnd);
	SetG2DRegister(g2d->regVal, G2D_DRAW_END_Y, draw->yEnd);
	g2d->draw.xStart = draw->xStart;
	g2d->draw.yStart = draw->yStart;
	g2d->draw.xEnd = draw->xEnd;
	g2d->draw.yEnd = draw->yEnd;

	//set draw mask and maskdata
	SetG2DRegister(g2d->regVal, G2D_DRAW_MASK, draw->mask);
	SetG2DRegister(g2d->regVal, G2D_DRAW_MASK_DATA, draw->data);
	g2d->draw.mask = draw->mask;
	g2d->draw.data = draw->data;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_rotate
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_rotate(G2D_ROTATE *rotate)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set rotate enable
	SetG2DRegister(g2d->regVal, G2D_ALU_CNTL_ROT_SEL, (rotate->enable==IM_TRUE)?1:0);
	g2d->rotate.enable = rotate->enable;

	//set rotate type
	if((rotate->type < G2D_ROT_TYPE_C90) || (rotate->type > G2D_ROT_TYPE_FLIP)){
		IM_ERRMSG((IM_STR("@g2dlib_set_rotate(), Invalid rotate type=%d"), rotate->type));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_ALU_CNTL_ROT_CODE, rotate->type);
	g2d->rotate.type = rotate->type;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_mem_operator
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_mem_operator(G2D_MEM_OP *memOp)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set MEM operator enable
	SetG2DRegister(g2d->regVal, G2D_ALU_CNTL_MEM_SEL, (memOp->enable==IM_TRUE)?1:0);
	g2d->memOp.enable = memOp->enable;

	//set memOp type
	if((memOp->type < G2D_MEM_TYPE_CPY)
		|| (memOp->type > G2D_MEM_TYPE_CLR)){
		IM_ERRMSG((IM_STR("@g2dlib_set_mem_operator(), Invalid memOp type=%d"), memOp->type));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_MEM_CODE_LEN_CODE, memOp->type);
	g2d->memOp.type = memOp->type;

	//set memOp length
	SetG2DRegister(g2d->regVal, G2D_MEM_LENGTH, memOp->length);
	g2d->memOp.length = memOp->length;

	//set memOp setDataType
	if((memOp->setDataType < G2D_SETDATA_TYPE_BITS8)
		|| (memOp->setDataType > G2D_SETDATA_TYPE_BITS64)){
		IM_ERRMSG((IM_STR("@g2dlib_set_mem_operator(), Invalid memOp setDataType=%d"), memOp->setDataType));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_MEM_CODE_LEN_SET_LEN, memOp->setDataType);
	g2d->memOp.setDataType = memOp->setDataType;

	//set memOp setDataL32 and setDataH32
	SetG2DRegister(g2d->regVal, G2D_MEM_SETDATA32, memOp->setDataL32);
	SetG2DRegister(g2d->regVal, G2D_MEM_SETDATA64, memOp->setDataH32);
	g2d->memOp.setDataL32 = memOp->setDataL32;
	g2d->memOp.setDataH32 = memOp->setDataH32;

	//set memOp buffer start address
	SetG2DRegister(g2d->regVal, G2D_MEM_ADDR_ST, memOp->buff.phy_addr);
	g2d->memOp.buff.phy_addr = memOp->buff.phy_addr;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_alpha_rop
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_alpha_rop(G2D_ALPHA_ROP *alphaRop)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set colorkey enable
	SetG2DRegister(g2d->regVal, G2D_CK_CNTL_CK_EN, (alphaRop->ckEn==IM_TRUE)?1:0);
	g2d->alphaRop.ckEn = alphaRop->ckEn;

	//set blend enable
	SetG2DRegister(g2d->regVal, G2D_CK_CNTL_CK_ABLD_EN, (alphaRop->blendEn==IM_TRUE)?1:0);
	g2d->alphaRop.blendEn = alphaRop->blendEn;

	//set global alpha enable
	SetG2DRegister(g2d->regVal, G2D_ALPHAG_GSEL, (alphaRop->gAlphaEn==IM_TRUE)?1:0);
	g2d->alphaRop.gAlphaEn = alphaRop->gAlphaEn;

	//set global alpha param
	if((alphaRop->gAlphaA > 255) || (alphaRop->gAlphaA < 0)
		|| (alphaRop->gAlphaB > 255) || (alphaRop->gAlphaB < 0) ){
		IM_ERRMSG((IM_STR("@g2dlib_set_alpha_rop(), Invalid alpha params, alphaA=%d, alphaB=%d"), alphaRop->gAlphaA, alphaRop->gAlphaB));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_ALPHAG_GA, alphaRop->gAlphaA);
	SetG2DRegister(g2d->regVal, G2D_ALPHAG_GB, alphaRop->gAlphaB);
	g2d->alphaRop.gAlphaA = alphaRop->gAlphaA;
	g2d->alphaRop.gAlphaB = alphaRop->gAlphaB;

	//set colorkey match mode
	if((alphaRop->matchMode != G2D_COLORKEY_DIR_MATCH_FORGROUND)
		&& (alphaRop->matchMode != G2D_COLORKEY_DIR_MATCH_BACKGROUND)){
		IM_ERRMSG((IM_STR("@g2d_set_alpha_rop(), Invalid colorkey match mode, matchMode=%d"), alphaRop->matchMode));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_CK_CNTL_CK_DIR, alphaRop->matchMode);
	g2d->alphaRop.matchMode = alphaRop->matchMode;

	//set colorkey mask and maskcolor
	SetG2DRegister(g2d->regVal, G2D_CK_MASK, alphaRop->mask);
	SetG2DRegister(g2d->regVal, G2D_CK_KEY, alphaRop->color);
	g2d->alphaRop.mask = alphaRop->mask;
	g2d->alphaRop.color = alphaRop->color;

	//set alpha-rop3 type
	if((alphaRop->arType < G2D_AR_TYPE_SCLSRC)
		|| (alphaRop->arType > G2D_AR_TYPE_ROP_SCLSRC_DEST)){
		IM_ERRMSG((IM_STR("@g2d_set_alpha_rop(), Invalid alpha-rop3 type, arType=%d"), alphaRop->arType));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_CK_CNTL_AR_CODE, alphaRop->arType);
	g2d->alphaRop.arType = alphaRop->arType;

	//set rop3 code
	SetG2DRegister(g2d->regVal, G2D_CK_CNTL_ROP_CODE, alphaRop->ropCode);
	g2d->alphaRop.ropCode = alphaRop->ropCode;

	//set sclee
	sclee_set_config(g2d->sclee, &alphaRop->sclee);
	g2dpwl_memcpy((void *)(&g2d->alphaRop.sclee), (void*)&alphaRop->sclee, sizeof(G2D_SCLEE));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}


//=====================================================================================
// g2d lib interface
//=====================================================================================

/*------------------------------------------------------------------------------
    Function name   , g2dlib_init
    Description     , Initialize G2D
    Return type     :
    Argument        , None
------------------------------------------------------------------------------*/
IM_RET g2dlib_init(void)
{
	IM_INT32 i;
	IM_RET ret = IM_RET_OK;
	IM_UINT32 regOffsetTable[] = {
		#include "g2d_reg_offset_table.h"
	};
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//IM_ASSERT(g2d == IM_NULL);
	if(g2d != IM_NULL)
	{
		IM_WARNMSG((IM_STR("@g2dlib_init(), g2d lib has been init!")));
		return IM_RET_OK;
	}

	//init pwl
	ret = g2dpwl_init();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_init(), g2dpwl_init() failed!")));
		return IM_RET_FAILED;
	}
	
	g2d = (G2D *)g2dpwl_malloc(sizeof(G2D));
	if(g2d == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dlib_init(), g2dpwl_malloc(G2D) failed!")));
		return IM_RET_FAILED;
	}
	g2dpwl_memset(g2d, 0, sizeof(G2D));
	//g2d->needKeyFlush = IM_FALSE;
	//g2d->needAllFlush = IM_FALSE;

	//init shadow registers from real register
	for(i=0; i<G2DREGNUM; i++){
		g2d->regOft[i] = regOffsetTable[i];
		g2d->regVal[i] = 0;
	}
	ret = g2d_refresh_regs();
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("@g2dlib_init(), g2d_refresh_regs() failed!")));
		goto Fail;
	}

	//malloc sclee context
	g2d->sclee = (SCLEE*)g2dpwl_malloc(sizeof(SCLEE));
	if(g2d->sclee == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dlib_init(), g2dpwl_malloc(SCLEE) failed!")));
		goto Fail;
	}

	/* set everything initially zero */
	g2dpwl_memset(g2d->sclee, 0, sizeof(SCLEE));
	g2d->sclee->regVal = g2d->regVal;

	//malloc dither context
	g2d->dith = (DITHER*)g2dpwl_malloc(sizeof(DITHER));
	if(g2d->dith == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dlib_init(), g2dpwl_malloc(DITHER) failed!")));
		goto Fail;
	}

	/* set everything initially zero */
	g2dpwl_memset(g2d->dith, 0, sizeof(DITHER));
	g2d->dith->regVal = g2d->regVal;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;

Fail:
	if(g2d != IM_NULL){
		if(g2d->dith != IM_NULL){
			g2dpwl_free(g2d->dith);
			g2d->dith = IM_NULL;
		}

		if(g2d->sclee != IM_NULL){
			g2dpwl_free(g2d->sclee);
			g2d->sclee = IM_NULL;
		}

		g2dpwl_free(g2d);
		g2d = IM_NULL;
	}
	g2dpwl_deinit();
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_deinit
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	if(g2d->srcChannel.palette.table != IM_NULL){
		g2dpwl_free(g2d->srcChannel.palette.table);
		g2d->srcChannel.palette.table = IM_NULL;
	}

	if(g2d->dstChannel.palette.table != IM_NULL){
		g2dpwl_free(g2d->dstChannel.palette.table);
		g2d->dstChannel.palette.table = IM_NULL;
	}

	if(g2d->patChannel.table != IM_NULL){
		g2dpwl_free(g2d->patChannel.table);
		g2d->patChannel.table = IM_NULL;
	}

	if(g2d->dith != IM_NULL){
		g2dpwl_free(g2d->dith);
		g2d->dith = IM_NULL;
	}

	if(g2d->sclee != IM_NULL){
		g2dpwl_free(g2d->sclee);
		g2d->sclee = IM_NULL;
	}

	g2dpwl_free(g2d);
	g2d = IM_NULL;

	ret = g2dpwl_deinit();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_deinit(), g2dpwl_deinit() failed!")));
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_property
    Description     :
    Return type     :
    Argument        :
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_property(IM_UINT32 property, void *ctx, G2D_SRC_CHANNEL *srcCh,
							G2D_DST_CHANNEL *dstCh, G2D_PAT_CHANNEL *patCh, G2D_OUT_CHANNEL *outCh)
{
	IM_RET ret = IM_RET_OK;
	G2D_DRAW *draw = IM_NULL;
	G2D_MEM_OP *memOp = IM_NULL;
	G2D_ROTATE *rotate = IM_NULL;
	G2D_ALPHA_ROP *alphaRop = IM_NULL;

	IM_UINT32 temp_xoffset = 0;
	IM_UINT32 temp_yoffset = 0;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	if((ctx == IM_NULL) || (outCh == IM_NULL)){
		IM_ERRMSG((IM_STR("@g2dlib_set_property(), ctx or outCh is NULL!")));
		return IM_RET_FAILED;
	}

	if(property == G2D_PRO_MEM){
		memOp = (G2D_MEM_OP*)ctx;
		ret = g2dlib_set_mem_operator(memOp);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_mem_operator() failed!")));
			return ret;
		}
		//outCh->vm->offset must set 0 for mem operator
		temp_xoffset = outCh->vm.xOffset;
		temp_yoffset = outCh->vm.yOffset;
		outCh->vm.xOffset = 0;
		outCh->vm.yOffset = 0;
		outCh->imgFormat = G2D_IMAGE_RGB_BPP24_888;
	}else if(property == G2D_PRO_DRAW){
		g2d->needKeyFlush = IM_TRUE;
		if(dstCh == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), dstCh is NULL!")));
			return IM_RET_FAILED;
		}
		ret = g2dlib_set_dst_channel(dstCh);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_dst_channel() failed!")));
			return ret;
		}

		draw = (G2D_DRAW *)ctx;
		ret = g2dlib_set_draw(draw);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_draw() failed!")));
			return ret;
		}
	}else if(property == G2D_PRO_ROTATE){
		g2d->needKeyFlush = IM_TRUE;
		if(srcCh == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), srcCh is NULL!")));
			return IM_RET_FAILED;
		}
		ret = g2dlib_set_src_channel(srcCh);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_src_channel() failed!")));
			return ret;
		}
		rotate = (G2D_ROTATE*)ctx;
		ret = g2dlib_set_rotate(rotate);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_rotate() failed!")));
			return ret;
		}
	}else if(property == G2D_PRO_ALPHAROP){
		g2d->needKeyFlush = IM_TRUE;
		if(srcCh == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), srcCh is NULL!")));
			return IM_RET_FAILED;
		}
		ret = g2dlib_set_src_channel(srcCh);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_src_channel() failed!")));
			return ret;
		}

		if(dstCh != IM_NULL){
			ret = g2dlib_set_dst_channel(dstCh);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_dst_channel() failed!")));
				return ret;
			}
		}

		if((patCh != IM_NULL) && (patCh->table != IM_NULL)){
			ret = g2dlib_set_pat_channel(patCh);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_pat_channel() failed!")));
				return ret;
			}
		}

		alphaRop = (G2D_ALPHA_ROP*)ctx;
		ret = g2dlib_set_alpha_rop(alphaRop);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_alpha_rop() failed!")));
			return ret;
		}
	}else{
		IM_ERRMSG((IM_STR("@g2dlib_set_property(), property is error!")));
		return IM_RET_FAILED;
	}


	ret = g2dlib_set_out_channel(outCh);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_set_property(), g2dlib_set_out_channel() failed!")));
		return ret;
	}
	//set orginal offset value back to outCh vm when do mem operator
	if(property == G2D_PRO_MEM){
		outCh->vm.xOffset = temp_xoffset;
		outCh->vm.yOffset = temp_yoffset;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_reset
    Description     , reset all register
    Return type     :
    Argument        :
------------------------------------------------------------------------------*/
IM_RET g2dlib_reset(void)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	//reset all g2d register use system reset
	ret = g2dpwl_reset();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_deinit(), g2dpwl_reset() failed!")));
	}

	ret = g2d_refresh_regs();

	return ret;


}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_dith
    Description     :
    Return type     :
    Argument        , 
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_dith(G2D_DITHER *dith)
{
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return dith_set_config(g2d->dith, dith);

}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_set_global_alpha
    Description     :
    Return type     :
    Argument        :
------------------------------------------------------------------------------*/
IM_RET g2dlib_set_global_alpha(IM_UINT32 gAlphaA, IM_UINT32 gAlphaB)
{
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//set global alpha param
	if((gAlphaA > 255) || (gAlphaA < 0)
		|| (gAlphaB > 255) || (gAlphaB < 0) ){
		IM_ERRMSG((IM_STR("@g2dlib_set_global_alpha(), Invalid alpha params, alphaA=%d, alphaB=%d"), gAlphaA, gAlphaB));
		return IM_RET_INVALID_PARAMETER;
	}
	SetG2DRegister(g2d->regVal, G2D_ALPHAG_GA, gAlphaA);
	SetG2DRegister(g2d->regVal, G2D_ALPHAG_GB, gAlphaB);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

/*------------------------------------------------------------------------------
    Function name   , g2dlib_exe
    Description     :
    Return type     :
    Argument        :
------------------------------------------------------------------------------*/
IM_RET g2dlib_exe(void)
{
	IM_UINT32 intr;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(g2d != IM_NULL);

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	ret = g2d_flush_regs();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_exe(), g2d_flush_regs() failed!")));
		return IM_RET_FAILED;
	}

	ret = g2dpwl_wait_hw_ready(&intr, -1);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dlib_exe(), g2dpwl_wait_hw_ready() failed!")));
		return IM_RET_FAILED;
	}
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}


