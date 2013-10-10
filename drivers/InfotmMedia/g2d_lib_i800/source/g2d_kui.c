/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_kui.c
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
#include <InfotmMedia.h>
#include <IM_g2dapi.h>
#include <g2d_lib.h>
#include <g2d_pwl.h>
#include <wq.h>
#include <g2d_kui.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"G2DKUI_I:"
#define WARNHEAD	"G2DKUI_W:"
#define ERRHEAD		"G2DKUI_E:"
#define TIPHEAD		"G2DKUI_T:"


typedef struct {
	IM_BOOL inited;
	IM_BOOL mmuInited;
	IM_BOOL mmuEnabled;
	wq_handle_t wqHandle;
	IM_INT32 refCnt;
}g2dkui_lable_t;

static g2dkui_lable_t gG2d = {
	.inited = IM_FALSE,
	.mmuInited = IM_FALSE,
	.mmuEnabled = IM_FALSE,
	.wqHandle = IM_NULL,
	.refCnt = 0,
};


typedef struct {
	wq_user_t usr;
	wq_package_t *pkg;
}g2dkui_context_t;


IM_UINT32 FMT_IMTOG2D(IM_UINT32 imFmt, G2D_BIT_SWAP *swap, IM_UINT32 *brRev)
{
	IM_UINT32 ret = 0xffffffff;

	swap->bitAddr = 0x0;
	swap->bitSwap = IM_FALSE;
	swap->bits2Swap = IM_FALSE;
	swap->bits4Swap = IM_FALSE;
	swap->byteSwap = IM_FALSE;
	swap->halfwordSwap = IM_FALSE;

	*brRev = G2D_BR_REVERSE_RGB;

	switch(imFmt)
	{
		case IM_PIC_FMT_1BITS_RGB_PAL:
			ret = G2D_IMAGE_PAL_BPP1;
			break;
		case IM_PIC_FMT_2BITS_RGB_PAL:
			ret = G2D_IMAGE_PAL_BPP2;
			break;
		case IM_PIC_FMT_4BITS_RGB_PAL:
			ret = G2D_IMAGE_PAL_BPP4;
			break;
		case IM_PIC_FMT_8BITS_RGB_PAL:
			ret = G2D_IMAGE_PAL_BPP8;
			break;
		case IM_PIC_FMT_8BITS_ARGB_1232:
			ret = G2D_IMAGE_RGB_BPP8_1A232;
			break;
		case IM_PIC_FMT_16BITS_RGB_565:
			ret = G2D_IMAGE_RGB_BPP16_565;
			break;
		case IM_PIC_FMT_16BITS_ARGB_1555:
			ret = G2D_IMAGE_RGB_BPP16_1A555;
			break;
		case IM_PIC_FMT_16BITS_IRGB_1555:
			ret = G2D_IMAGE_RGB_BPP16_I555;
			break;
		case IM_PIC_FMT_32BITS_0RGB_D666:
			ret = G2D_IMAGE_RGB_BPP18_666;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1665:
			ret = G2D_IMAGE_RGB_BPP18_1A665;	
			break;
		case IM_PIC_FMT_32BITS_ARGB_1666:
			ret = G2D_IMAGE_RGB_BPP19_1A666;
			break;
		case IM_PIC_FMT_32BITS_0RGB_8888:
			ret = G2D_IMAGE_RGB_BPP24_888;
			break;
		case IM_PIC_FMT_32BITS_0BGR_8888:
			//convert to 0RGB0888 by RB reverse
			ret = G2D_IMAGE_RGB_BPP24_888;

			*brRev = G2D_BR_REVERSE_BGR;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1887:
			ret = G2D_IMAGE_RGB_BPP24_1A887;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1888:
			ret = G2D_IMAGE_RGB_BPP25_1A888;
			break;
		case IM_PIC_FMT_32BITS_ARGB_4888:
			ret = G2D_IMAGE_RGB_BPP28_4A888;
			break;
		case IM_PIC_FMT_32BITS_RGBA_8888:
			ret = G2D_IMAGE_RGB_BPP32_888A;
			break;
		case IM_PIC_FMT_32BITS_ABGR_8888:
			//convert to RGBA8888 by bitswap
			ret = G2D_IMAGE_RGB_BPP32_888A;

			swap->bitAddr = 0x0;
			swap->bitSwap = IM_FALSE;
			swap->bits2Swap = IM_FALSE;
			swap->bits4Swap = IM_FALSE;
			swap->byteSwap = IM_TRUE;
			swap->halfwordSwap = IM_TRUE;
			break;
		case IM_PIC_FMT_16BITS_RGBA_5551:
			ret = G2D_IMAGE_RGB_BPP16_555A;
			break;
		case IM_PIC_FMT_16BITS_RGBI_5551:
			ret = G2D_IMAGE_RGB_BPP16_555I;
			break;
		case IM_PIC_FMT_32BITS_ARGB_8888:
			ret = G2D_IMAGE_RGB_BPP32_A888;
			break;
		case IM_PIC_FMT_16BITS_0RGB_4444:
			ret = G2D_IMAGE_RGB_BPP12_444;
			break;
		case IM_PIC_FMT_16BITS_ARGB_4444:
			ret = G2D_IMAGE_RGB_BPP16_4A444;
			break;
		case IM_PIC_FMT_16BITS_RGBA_4444:
			//convert to ARGB4444 by bitswap & RB reverse
			ret = G2D_IMAGE_RGB_BPP16_4A444;
			swap->bitAddr = 0x0;
			swap->bitSwap = IM_FALSE;
			swap->bits2Swap = IM_FALSE;
			swap->bits4Swap = IM_TRUE;
			swap->byteSwap = IM_TRUE;
			swap->halfwordSwap = IM_FALSE;
			*brRev = G2D_BR_REVERSE_BGR;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1565:
			ret = G2D_IMAGE_RGB_BPP17_1A565;
			break;
		case IM_PIC_FMT_32BITS_ARGB_8565:
			ret = G2D_IMAGE_RGB_BPP24_8A565;
			break;
		case IM_PIC_FMT_16BITS_YUV422I_YUYV:
			ret = G2D_IMAGE_YUV_422I_YUYV;
			break;
		case IM_PIC_FMT_16BITS_YUV422I_UYVY:
			ret = G2D_IMAGE_YUV_422I_UYVY;
			break;
		case IM_PIC_FMT_16BITS_YUV422I_YVYU:
			ret = G2D_IMAGE_YUV_422I_YVYU;
			break;
		case IM_PIC_FMT_16BITS_YUV422I_VYUY:
			ret = G2D_IMAGE_YUV_422I_VYUY;
			break;
		case IM_PIC_FMT_32BITS_YUV444I:
			ret = G2D_IMAGE_YUV_444I;
			break;
		case IM_PIC_FMT_16BITS_YUV422SP:
			ret = G2D_IMAGE_YUV_422SP;
			break;
		case IM_PIC_FMT_12BITS_YUV420SP:
			ret = G2D_IMAGE_YUV_420SP;
			break;
		default:
			IM_ERRMSG((IM_STR("@FMT_IMTOG2D(imFmt=%d), This image format is not supported for G2D!"), imFmt));
			IM_ASSERT(IM_FALSE);
			break;
	}

	return ret;
}

IM_UINT32 PALFMT_IMTOG2D(IM_UINT32 imPalFmt)
{
	IM_UINT32 ret = 0xffffffff;

	switch(imPalFmt)
	{
		case IM_PIC_FMT_32BITS_ARGB_1888:
			ret = G2D_PALATTE_FORMAT_A888;
			break;
		case IM_PIC_FMT_32BITS_0RGB_8888:
			ret = G2D_PALATTE_FORMAT_888;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1666:
			ret = G2D_PALATTE_FORMAT_A666;
			break;
		case IM_PIC_FMT_32BITS_ARGB_1665:
			ret = G2D_PALATTE_FORMAT_A665;
			break;
		case IM_PIC_FMT_32BITS_0RGB_D666:
			ret = G2D_PALATTE_FORMAT_666;
			break;
		case IM_PIC_FMT_16BITS_ARGB_1555:
			ret = G2D_PALATTE_FORMAT_A555;
			break;
		case IM_PIC_FMT_16BITS_RGB_565:
			ret = G2D_PALATTE_FORMAT_565;
			break;
		default:
			IM_ERRMSG((IM_STR("@PALFMT_IMTOG2D(), This image palette format is not supported for G2D!")));
			IM_ASSERT(IM_FALSE);
			break;
	}

	return ret;
}

static int taskBgNum = 0;
static int taskEdNum = 0;

IM_RET g2dkui_exec(void *userData)
{
	IM_RET ret = IM_RET_OK;
	g2d_config_t *g2dCfg = (g2d_config_t *)userData;
	void *ctx = IM_NULL;
	G2D_MEM_OP memOp;
	G2D_DRAW draw;
	G2D_ROTATE rotate;
	G2D_ALPHA_ROP alphaRop;
	G2D_DITHER dith;

	G2D_SRC_CHANNEL sch;
	G2D_SRC_CHANNEL *srcCh = IM_NULL;
	G2D_DST_CHANNEL dch;
	G2D_DST_CHANNEL *dstCh = IM_NULL;
	G2D_PAT_CHANNEL pch;
	G2D_PAT_CHANNEL *patCh = IM_NULL;
	G2D_OUT_CHANNEL outCh;
	IM_UINT32 brRev = 0x0;

	IM_BOOL dstNeed = IM_FALSE;
	IM_BOOL patNeed = IM_FALSE;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	taskBgNum++;
	IM_INFOMSG((IM_STR("%s(taskBgNum=%d)++"), IM_STR(_IM_FUNC_), taskBgNum));


	//mmu enable or disable
	if((g2dCfg->useDevMMU == IM_TRUE) && (gG2d.mmuEnabled == IM_FALSE)){
		//IM_INFOMSG((IM_STR("%s(enable MMU!!!!)"), IM_STR(_IM_FUNC_)));
		g2dpwl_mmu_enable();
		gG2d.mmuEnabled = IM_TRUE;
	}else if((g2dCfg->useDevMMU == IM_FALSE) && (gG2d.mmuEnabled == IM_TRUE)){
		//IM_INFOMSG((IM_STR("%s(disable MMU!!!!)"), IM_STR(_IM_FUNC_)));
		g2dpwl_mmu_disable();
		gG2d.mmuEnabled = IM_FALSE;
	}
	//init some struct
	//memOp.enable = IM_FALSE;
	//draw.enable = IM_FALSE;
	//rotate.enable = IM_FALSE;
	
	//
	g2dpwl_memset((void*)&outCh, 0, sizeof(G2D_OUT_CHANNEL));
	
	//get ctx, srcCh, destCh, patCh, outCh info from g2dCfg 
	switch(g2dCfg->property)
	{
	case G2D_OPP_MEM:
		g2dpwl_memset((void*)&memOp, 0, sizeof(G2D_MEM_OP));
		memOp.enable = IM_TRUE;
		memOp.type = g2dCfg->memCfg.type;
		memOp.length = g2dCfg->memCfg.size;
		if(memOp.type == G2D_MEM_TYPE_CPY){
			IM_BUFFER_COPYTO_BUFFER(memOp.buff, g2dCfg->memCfg.srcBuf);
		}else if(memOp.type == G2D_MEM_TYPE_SET){
			memOp.setDataType = g2dCfg->memCfg.setUnitByte;
			if(memOp.setDataType == G2D_SETDATA_TYPE_BITS64){
				memOp.setDataL32 = g2dCfg->memCfg.setValueL&0xffffffff;
				memOp.setDataH32 = g2dCfg->memCfg.setValueH&0xffffffff;
			}else if(memOp.setDataType == G2D_SETDATA_TYPE_BITS32){
				memOp.setDataL32 = g2dCfg->memCfg.setValueL&0xffffffff;
			}else if(memOp.setDataType == G2D_SETDATA_TYPE_BITS16){
				memOp.setDataL32 = g2dCfg->memCfg.setValueL&0xffff;
			}else{
				memOp.setDataL32 = g2dCfg->memCfg.setValueL&0xff;
			}
		}
		ctx = (void *)&memOp;

		IM_BUFFER_COPYTO_BUFFER(outCh.buff, g2dCfg->memCfg.dstBuf);
		break;
	case G2D_OPP_DRAW:
		g2dpwl_memset((void*)&draw, 0, sizeof(G2D_DRAW));
		draw.enable = IM_TRUE;
		draw.footEn = IM_TRUE;
		draw.lineWidth = g2dCfg->drawCfg.lineWidth;

		if(g2dCfg->drawCfg.lineStyle == G2D_LINE_REAL){
			draw.mask = 0xffffffff;
		}else{
			draw.mask = g2dCfg->drawCfg.mask;
		}

		draw.data = g2dCfg->drawCfg.colorData;

		draw.xStart = g2dCfg->drawCfg.xStart; 
		draw.yStart = g2dCfg->drawCfg.yStart; 
		draw.xEnd = g2dCfg->drawCfg.xEnd; 
		draw.yEnd = g2dCfg->drawCfg.yEnd; 

		if(g2dCfg->drawCfg.type == G2D_DRAW_POINT){
			draw.type = G2D_DRAW_TYPE_LINE;
			draw.footEn = IM_TRUE;
			draw.xEnd = draw.xStart; 
			draw.yEnd = draw.yStart; 
		}else if(g2dCfg->drawCfg.type == G2D_DRAW_LINE){
			draw.type = G2D_DRAW_TYPE_LINE;
		}else{
			draw.type = G2D_DRAW_TYPE_RECT;
			draw.mask = 0xffffffff;//rect only support real line
		}

		ctx = (void *)&draw;

		//whether dither enable
		dith.dithEn = g2dCfg->drawCfg.dith.dithEn;
		dith.rChannel = g2dCfg->drawCfg.dith.rChReduceBits;
		dith.gChannel = g2dCfg->drawCfg.dith.gChReduceBits;
		dith.bChannel = g2dCfg->drawCfg.dith.bChReduceBits;

		//dst channel
		g2dpwl_memset((void*)&dch, 0, sizeof(G2D_DST_CHANNEL));
		dch.imgFormat = FMT_IMTOG2D(g2dCfg->drawCfg.imgSrc.format, &dch.swap, &dch.brRevs);
		//not support palette format for draw path
		/*if(dch.imgFormat < G2D_IMAGE_RGB_BPP8_1A232){
			IM_ERRMSG((IM_STR("@g2dkui_exec(), err draw input format failed!")));
			return IM_RET_FAILED;
		}*/
		//dch.palette = ;
		
		dch.width = g2dCfg->drawCfg.imgSrc.clipRect.width;
		dch.height = g2dCfg->drawCfg.imgSrc.clipRect.height;
		dch.vm.xOffset = g2dCfg->drawCfg.imgSrc.clipRect.left;
		dch.vm.yOffset = g2dCfg->drawCfg.imgSrc.clipRect.top;
		dch.vm.fulWidth = g2dCfg->drawCfg.imgSrc.width;
		dch.vm.fulHeight = g2dCfg->drawCfg.imgSrc.height;
		IM_BUFFER_COPYTO_BUFFER(dch.buff, g2dCfg->drawCfg.imgSrc.buffer[0]);
		dstCh = &dch;

		//outCh
		if(g2dCfg->drawCfg.imgDst.buffer[0].size != 0){//draw to other buffer
			outCh.imgFormat = FMT_IMTOG2D(g2dCfg->drawCfg.imgDst.format, &outCh.swap, &brRev);
			outCh.width = g2dCfg->drawCfg.imgDst.clipRect.width;
			outCh.height = g2dCfg->drawCfg.imgDst.clipRect.height;
			outCh.vm.xOffset = g2dCfg->drawCfg.imgDst.clipRect.left;
			outCh.vm.yOffset = g2dCfg->drawCfg.imgDst.clipRect.top;
			outCh.vm.fulWidth = g2dCfg->drawCfg.imgDst.width;
			outCh.vm.fulHeight = g2dCfg->drawCfg.imgDst.height;
			IM_BUFFER_COPYTO_BUFFER(outCh.buff, g2dCfg->drawCfg.imgDst.buffer[0]);
		}else{//draw direct to source buffer
			outCh.imgFormat = FMT_IMTOG2D(g2dCfg->drawCfg.imgSrc.format, &outCh.swap, &brRev);
			outCh.width = g2dCfg->drawCfg.imgSrc.clipRect.width;
			outCh.height = g2dCfg->drawCfg.imgSrc.clipRect.height;
			outCh.vm.xOffset = g2dCfg->drawCfg.imgSrc.clipRect.left;
			outCh.vm.yOffset = g2dCfg->drawCfg.imgSrc.clipRect.top;
			outCh.vm.fulWidth = g2dCfg->drawCfg.imgSrc.width;
			outCh.vm.fulHeight = g2dCfg->drawCfg.imgSrc.height;
			IM_BUFFER_COPYTO_BUFFER(outCh.buff, g2dCfg->drawCfg.imgSrc.buffer[0]);
		}
		break;
	case G2D_OPP_ROTATE:
		g2dpwl_memset((void*)&rotate, 0, sizeof(G2D_ROTATE));
		rotate.enable = IM_TRUE;
		rotate.type = g2dCfg->rotateCfg.type;

		ctx = (void *)&rotate;

		//whether dither enable
		dith.dithEn = g2dCfg->rotateCfg.dith.dithEn;
		dith.rChannel = g2dCfg->rotateCfg.dith.rChReduceBits;
		dith.gChannel = g2dCfg->rotateCfg.dith.gChReduceBits;
		dith.bChannel = g2dCfg->rotateCfg.dith.bChReduceBits;

		//src channel
		g2dpwl_memset((void*)&sch, 0, sizeof(G2D_SRC_CHANNEL));
		
		sch.imgFormat = FMT_IMTOG2D(g2dCfg->rotateCfg.imgSrc.format, &sch.ySwap, &sch.brRevs);
		//sch.uvSwap.bitAddr = 0x0;//use default value
		//sch.uvSwap.bitSwap = IM_FALSE;
		//sch.uvSwap.bits2Swap = IM_FALSE;
		//sch.uvSwap.bits4Swap = IM_FALSE;
		//sch.uvSwap.byteSwap = IM_FALSE;
		//sch.uvSwap.halfwordSwap = IM_FALSE;
		
		//not support palette format for rotate path
		/*if(sch.imgFormat < G2D_IMAGE_RGB_BPP8_1A232){
			IM_ERRMSG((IM_STR("@g2dkui_exec(), err rotate input format failed!")));
			return IM_RET_FAILED;
		}*/
		//sch.palette = ;
		
		sch.width = g2dCfg->rotateCfg.imgSrc.clipRect.width;
		sch.height = g2dCfg->rotateCfg.imgSrc.clipRect.height;
		sch.vm.xOffset = g2dCfg->rotateCfg.imgSrc.clipRect.left;
		sch.vm.yOffset = g2dCfg->rotateCfg.imgSrc.clipRect.top;
		sch.vm.fulWidth = g2dCfg->rotateCfg.imgSrc.width;
		sch.vm.fulHeight = g2dCfg->rotateCfg.imgSrc.height;
		sch.cscMat.oft_b = 16;//use default value
		sch.cscMat.oft_a = 0;
		sch.cscMat.coef11 = 1192;
		sch.cscMat.coef12 = 0;
		sch.cscMat.coef13 = 1634;
		sch.cscMat.coef21 = 1192;
		sch.cscMat.coef22 = -402;
		sch.cscMat.coef23 = -833;
		sch.cscMat.coef31 = 1192;
		sch.cscMat.coef32 = 2065;
		sch.cscMat.coef33 = 0;
		IM_BUFFER_COPYTO_BUFFER(sch.buffY, g2dCfg->rotateCfg.imgSrc.buffer[0]);
		//not support yuv semiplaner or planer format for rotate path
		//IM_BUFFER_COPYTO_BUFFER(sch.buffUV, g2dCfg->rotateCfg.imgSrc.buffer[1]);
		srcCh = &sch;

		//outCh
		outCh.imgFormat = FMT_IMTOG2D(g2dCfg->rotateCfg.imgDst.format, &outCh.swap, &brRev);
		outCh.width = g2dCfg->rotateCfg.imgDst.clipRect.width;
		outCh.height = g2dCfg->rotateCfg.imgDst.clipRect.height;
		outCh.vm.xOffset = g2dCfg->rotateCfg.imgDst.clipRect.left;
		outCh.vm.yOffset = g2dCfg->rotateCfg.imgDst.clipRect.top;
		outCh.vm.fulWidth = g2dCfg->rotateCfg.imgDst.width;
		outCh.vm.fulHeight = g2dCfg->rotateCfg.imgDst.height;
		IM_BUFFER_COPYTO_BUFFER(outCh.buff, g2dCfg->rotateCfg.imgDst.buffer[0]);
		break;
	case G2D_OPP_ALPHAROP:
		g2dpwl_memset((void*)&alphaRop, 0, sizeof(G2D_ALPHA_ROP));
		if(g2dCfg->alpharopCfg.type & G2D_AROP_ALPHA)
		{
			alphaRop.ckEn = g2dCfg->alpharopCfg.alpha.colorKey.enable;
			alphaRop.blendEn = g2dCfg->alpharopCfg.alpha.colorKey.enableBlend;
			alphaRop.matchMode = g2dCfg->alpharopCfg.alpha.colorKey.matchMode;
			alphaRop.mask = g2dCfg->alpharopCfg.alpha.colorKey.mask;
			alphaRop.color = g2dCfg->alpharopCfg.alpha.colorKey.color;
			if(g2dCfg->alpharopCfg.alpha.blendMode == G2D_BLEND_MODE_PLANE)
			{
				alphaRop.gAlphaEn = IM_TRUE;
				alphaRop.gAlphaA = g2dCfg->alpharopCfg.alpha.srcAlpha;
				alphaRop.gAlphaB = g2dCfg->alpharopCfg.alpha.dstAlpha;
			}
			else
			{
				alphaRop.gAlphaEn = IM_FALSE;
				alphaRop.gAlphaA = 128;
				alphaRop.gAlphaB = 128;
			}	
			if(g2dCfg->alpharopCfg.alpha.type == G2D_ALPHA_SRC_PAT)
			{
				alphaRop.arType = G2D_AR_TYPE_AFA_SCLSRC_PAT;
				patNeed = IM_TRUE;
			}
			else
			{
				alphaRop.arType = G2D_AR_TYPE_AFA_SCLSRC_DEST;
				dstNeed = IM_TRUE;
			}
		}
		else if(g2dCfg->alpharopCfg.type & G2D_AROP_RASTER)
		{
			alphaRop.ropCode = g2dCfg->alpharopCfg.rop.ropCode;
			if(g2dCfg->alpharopCfg.rop.type == G2D_ROP_SRC_PAT)
			{
				alphaRop.arType = G2D_AR_TYPE_ROP_SCLSRC_PAT;
				patNeed = IM_TRUE;
			}
			else if(g2dCfg->alpharopCfg.rop.type == G2D_ROP_SRC_SRC)
			{
				alphaRop.arType = G2D_AR_TYPE_ROP_SCLSRC_DEST;
				dstNeed = IM_TRUE;
			}
			else
			{
				alphaRop.arType = G2D_AR_TYPE_ROP_SCLSRC_DEST_PAT;
				patNeed = IM_TRUE;
				dstNeed = IM_TRUE;
			}
		}
		else
		{
			alphaRop.arType = G2D_AR_TYPE_SCLSRC;
		}

		if(g2dCfg->alpharopCfg.type & (G2D_AROP_SCL | G2D_AROP_EE))
		{	
			//sclee
			if (g2dCfg->alpharopCfg.type & G2D_AROP_SCL) {
				alphaRop.sclee.verEnable = IM_TRUE;
				alphaRop.sclee.horEnable = IM_TRUE;
				IM_INFOMSG((IM_STR("sclEnable")));
			}
			else {
				alphaRop.sclee.verEnable = IM_FALSE;
				alphaRop.sclee.horEnable = IM_FALSE;
			}
			//alphaRop.sclee.vrdMode = G2D_ROUND_MINUS;//use default value
			//alphaRop.sclee.hrdMode = G2D_ROUND_MINUS;
			//alphaRop.sclee.paramType = G2D_SCL_PARAM_TYPE_0;
			alphaRop.sclee.inWidth = g2dCfg->alpharopCfg.imgSrc1.clipRect.width;
			alphaRop.sclee.inHeight = g2dCfg->alpharopCfg.imgSrc1.clipRect.height;
			alphaRop.sclee.outWidth = g2dCfg->alpharopCfg.imgDst.clipRect.width;
			alphaRop.sclee.outHeight = g2dCfg->alpharopCfg.imgDst.clipRect.height;
			
			alphaRop.sclee.coefw = 0x33;
			alphaRop.sclee.coefa = 0x40;
			alphaRop.sclee.denosEnable = IM_TRUE;
			//alphaRop.sclee.errTh = 0x5a;
			alphaRop.sclee.thrMat.hTh = 0x28;
			alphaRop.sclee.thrMat.vTh = 0x28;
			alphaRop.sclee.thrMat.d0Th = 0x28;
			alphaRop.sclee.thrMat.d1Th = 0x28;

			if (g2dCfg->alpharopCfg.type & G2D_AROP_EE) {
				IM_INFOMSG((IM_STR("eeEnable")));
				alphaRop.sclee.eeEnable = IM_TRUE;
				alphaRop.sclee.order = G2D_ORDER_SCLEE;//use default valuea
				alphaRop.sclee.coefw = g2dCfg->alpharopCfg.edgeSharpValue;
				IM_INFOMSG((IM_STR("coefw=%d"), alphaRop.sclee.coefw));
				alphaRop.sclee.rdMode = G2D_ROUND_MINUS;
				alphaRop.sclee.denosEnable = IM_FALSE;
				alphaRop.sclee.gasMode = G2D_EE_GAUSS_MODE_0;
				alphaRop.sclee.errTh = 0x6e;
				alphaRop.sclee.opMatType = G2D_EE_OPMAT_TYPE_0;
			}
			else {
				IM_INFOMSG((IM_STR("eeDisable")));
				alphaRop.sclee.eeEnable = IM_FALSE;
			}
		}
		else
		{
			//sclee
			alphaRop.sclee.verEnable = IM_FALSE;
			alphaRop.sclee.horEnable = IM_FALSE;
			alphaRop.sclee.inWidth = g2dCfg->alpharopCfg.imgSrc1.clipRect.width;
			alphaRop.sclee.inHeight = g2dCfg->alpharopCfg.imgSrc1.clipRect.height;
			alphaRop.sclee.outWidth = g2dCfg->alpharopCfg.imgDst.clipRect.width;
			alphaRop.sclee.outHeight = g2dCfg->alpharopCfg.imgDst.clipRect.height;
		}
		ctx = (void *)&alphaRop;

		//whether dither enable
		if(g2dCfg->alpharopCfg.imgDst.format == IM_PIC_FMT_16BITS_RGB_565)
		{
			dith.dithEn = IM_TRUE;
			dith.rChannel = 3;
			dith.gChannel = 2;
			dith.bChannel = 3;
		}
		else
		{
			dith.dithEn = g2dCfg->alpharopCfg.dith.dithEn;
			dith.rChannel = g2dCfg->alpharopCfg.dith.rChReduceBits;
			dith.gChannel = g2dCfg->alpharopCfg.dith.gChReduceBits;
			dith.bChannel = g2dCfg->alpharopCfg.dith.bChReduceBits;
		}

		//src channel
		g2dpwl_memset((void*)&sch, 0, sizeof(G2D_SRC_CHANNEL));
		sch.imgFormat = FMT_IMTOG2D(g2dCfg->alpharopCfg.imgSrc1.format, &sch.ySwap, &sch.brRevs);
		if(sch.imgFormat < G2D_IMAGE_RGB_BPP8_1A232)//palette format
		{
			sch.palette.palFormat = g2dCfg->alpharopCfg.imgSrc1.palette.format;
			sch.palette.tableLength = g2dCfg->alpharopCfg.imgSrc1.palette.tableLength;
			sch.palette.table = g2dCfg->alpharopCfg.imgSrc1.palette.table;
		}
		//sch.uvSwap.bitAddr = 0x0;//use default value
		//sch.uvSwap.bitSwap = IM_FALSE;
		//sch.uvSwap.bits2Swap = IM_FALSE;
		//sch.uvSwap.bits4Swap = IM_FALSE;
		//sch.uvSwap.byteSwap = IM_FALSE;
		//sch.uvSwap.halfwordSwap = IM_FALSE;
		
		sch.width = g2dCfg->alpharopCfg.imgSrc1.clipRect.width;
		sch.height = g2dCfg->alpharopCfg.imgSrc1.clipRect.height;
		sch.vm.xOffset = g2dCfg->alpharopCfg.imgSrc1.clipRect.left;
		sch.vm.yOffset = g2dCfg->alpharopCfg.imgSrc1.clipRect.top;
		sch.vm.fulWidth = g2dCfg->alpharopCfg.imgSrc1.width;
		sch.vm.fulHeight = g2dCfg->alpharopCfg.imgSrc1.height;
		sch.cscMat.oft_b = 16;//use default value
		sch.cscMat.oft_a = 0;
		sch.cscMat.coef11 = 1192;
		sch.cscMat.coef12 = 0;
		sch.cscMat.coef13 = 1634;
		sch.cscMat.coef21 = 1192;
		sch.cscMat.coef22 = -402;
		sch.cscMat.coef23 = -833;
		sch.cscMat.coef31 = 1192;
		sch.cscMat.coef32 = 2065;
		sch.cscMat.coef33 = 0;
		IM_BUFFER_COPYTO_BUFFER(sch.buffY, g2dCfg->alpharopCfg.imgSrc1.buffer[0]);
		IM_BUFFER_COPYTO_BUFFER(sch.buffUV, g2dCfg->alpharopCfg.imgSrc1.buffer[1]);
		srcCh = &sch;

		if(dstNeed == IM_TRUE)
		{
			//dst channel
			g2dpwl_memset((void*)&dch, 0, sizeof(G2D_DST_CHANNEL));
			dch.imgFormat = FMT_IMTOG2D(g2dCfg->alpharopCfg.imgSrc2.format, &dch.swap, &dch.brRevs);
			if(dch.imgFormat < G2D_IMAGE_RGB_BPP8_1A232)//palette format
			{
				dch.palette.palFormat = g2dCfg->alpharopCfg.imgSrc2.palette.format;
				dch.palette.tableLength = g2dCfg->alpharopCfg.imgSrc2.palette.tableLength;
				dch.palette.table = g2dCfg->alpharopCfg.imgSrc2.palette.table;
			}
			dch.width = g2dCfg->alpharopCfg.imgSrc2.clipRect.width;
			dch.height = g2dCfg->alpharopCfg.imgSrc2.clipRect.height;
			dch.vm.xOffset = g2dCfg->alpharopCfg.imgSrc2.clipRect.left;
			dch.vm.yOffset = g2dCfg->alpharopCfg.imgSrc2.clipRect.top;
			dch.vm.fulWidth = g2dCfg->alpharopCfg.imgSrc2.width;
			dch.vm.fulHeight = g2dCfg->alpharopCfg.imgSrc2.height;
			IM_BUFFER_COPYTO_BUFFER(dch.buff, g2dCfg->alpharopCfg.imgSrc2.buffer[0]);
			dstCh = &dch;
		}

		if(patNeed == IM_TRUE)
		{
			//pat channel
			g2dpwl_memset((void*)&pch, 0, sizeof(G2D_PAT_CHANNEL));
			pch.width = g2dCfg->alpharopCfg.imgPat.width;
			pch.height = g2dCfg->alpharopCfg.imgPat.height;
			pch.tableLength = g2dCfg->alpharopCfg.imgPat.tableLength;
			pch.table = g2dCfg->alpharopCfg.imgPat.table;
			patCh = &pch;
		}

		//outCh
		outCh.imgFormat = FMT_IMTOG2D(g2dCfg->alpharopCfg.imgDst.format, &outCh.swap, &brRev);
		outCh.width = g2dCfg->alpharopCfg.imgDst.clipRect.width;
		outCh.height = g2dCfg->alpharopCfg.imgDst.clipRect.height;
		outCh.vm.xOffset = g2dCfg->alpharopCfg.imgDst.clipRect.left;
		outCh.vm.yOffset = g2dCfg->alpharopCfg.imgDst.clipRect.top;
		outCh.vm.fulWidth = g2dCfg->alpharopCfg.imgDst.width;
		outCh.vm.fulHeight = g2dCfg->alpharopCfg.imgDst.height;
		outCh.buff = g2dCfg->alpharopCfg.imgDst.buffer[0];
		IM_BUFFER_COPYTO_BUFFER(outCh.buff, g2dCfg->alpharopCfg.imgDst.buffer[0]);
		break;
	default:
		IM_ERRMSG((IM_STR("@g2dkui_exec(),g2d operator type is error or not supported!")));
		break;
	}

	ret = g2dlib_reset();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_exec(), g2dlib_reset() failed!")));
		return IM_RET_FAILED;
	}

	//value of global alpha must set to 128 before some g2d operator
	ret = g2dlib_set_global_alpha(128, 128);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_exec(), g2dlib_set_global_alpha() failed!")));
		return IM_RET_FAILED;
	}

	ret = g2dlib_set_property(g2dCfg->property, ctx, srcCh, dstCh, patCh, &outCh);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_exec(), g2dlib_set_property() failed!")));
		return IM_RET_FAILED;
	}
	
	if(dith.dithEn == IM_TRUE)
	{
		dith.tempoEn = IM_TRUE;
		dith.width = outCh.width;
		dith.height = outCh.height;
		dith.coefType = G2D_DITH_COEF_TYPE_0;//use default type
		ret = g2dlib_set_dith(&dith);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dkui_exec(), g2dlib_set_dith() failed!")));
			return IM_RET_FAILED;
		}
	}

	ret = g2dlib_exe();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_exec(), g2dlib_exe() failed!")));
		return IM_RET_FAILED;
	}

	taskEdNum++;
	IM_INFOMSG((IM_STR("%s(taskEdNum=%d)--"), IM_STR(_IM_FUNC_), taskBgNum));
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

void g2dkui_free_package(wq_user_t usr, wq_package_t *pkg)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	g2dpwl_free(pkg->userData);
	g2dpwl_free(pkg);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return;
}

//=============================================
//g2d internal api interface
//=============================================
const void *g2dkui_init(void)
{
	g2dkui_context_t *g2dCtx = IM_NULL;
	
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	g2dCtx = (g2dkui_context_t *)g2dpwl_malloc(sizeof(g2dkui_context_t));
	if(g2dCtx == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dkui_init(), g2dpwl_malloc(g2dkui_context_t)!")));
		goto Fail;
	}
	g2dpwl_memset(g2dCtx, 0, sizeof(g2dkui_context_t));

	if(gG2d.inited == IM_FALSE){
		if(wq_init(&gG2d.wqHandle, g2dkui_exec) != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dkui_init(), wq_init() failed!")));
			goto Fail;
		}

		if(g2dlib_init() != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dkui_init(), g2dlib_init() failed!")));
			goto Fail;
		}
		gG2d.inited = IM_TRUE;

		if(g2dpwl_mmu_init() != IM_RET_OK){
			IM_ERRMSG((IM_STR("@g2dkui_init(), g2dpwl_mmu_init() failed!")));
			goto Fail;
		}
		gG2d.mmuInited = IM_TRUE;
	}

	if(wq_init_user(gG2d.wqHandle, &g2dCtx->usr, g2dkui_free_package) != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_init(), wq_init() failed!")));
		goto Fail;
	}

	gG2d.refCnt++;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return (void*)g2dCtx;

Fail:
	if(g2dCtx != IM_NULL)
		g2dpwl_free(g2dCtx);

	if(gG2d.refCnt == 0){
		if(gG2d.mmuInited == IM_TRUE){
			g2dpwl_mmu_deinit();
			gG2d.mmuInited = IM_FALSE;
		}
		if(gG2d.inited == IM_TRUE){
			g2dlib_deinit();
			gG2d.inited = IM_TRUE;
		}

		if(gG2d.wqHandle != IM_NULL)
			wq_deinit(gG2d.wqHandle);
	}

	return IM_NULL;
}

IM_RET g2dkui_deinit(void *instance)
{
	g2dkui_context_t *g2dCtx = (g2dkui_context_t *)instance;
	IM_ASSERT(g2dCtx != IM_NULL);
	IM_ASSERT(g2dCtx->pkg == IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	wq_deinit_user(gG2d.wqHandle, g2dCtx->usr);

	g2dpwl_free(g2dCtx);

	gG2d.refCnt--;

	if(gG2d.mmuEnabled == IM_TRUE){
		g2dpwl_mmu_disable();
		gG2d.mmuEnabled = IM_FALSE;
	}
	if(gG2d.refCnt == 0){
		g2dpwl_mmu_deinit();
		gG2d.mmuInited = IM_FALSE;
		g2dlib_deinit();
		gG2d.inited = IM_FALSE;
		wq_deinit(gG2d.wqHandle);
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dkui_set_config(void *instance, g2d_config_t *config)
{
	g2d_config_t *g2dCfg = IM_NULL;
	g2dkui_context_t *g2dCtx = (g2dkui_context_t *)instance;
	IM_ASSERT(g2dCtx != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	g2dCtx->pkg = (wq_package_t *)g2dpwl_malloc(sizeof(wq_package_t));
	if(g2dCtx->pkg == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dkui_set_config(), g2dpwl_malloc(g2dkui_context_t) failed!")));
		return IM_RET_FAILED;
	}
	g2dpwl_memset(g2dCtx->pkg, 0, sizeof(wq_package_t));
	
	g2dCfg = (g2d_config_t *)g2dpwl_malloc(sizeof(g2d_config_t));
	if(g2dCfg == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dkui_set_config(), g2dpwl_malloc(g2d_config_t) failed!")));
		g2dpwl_free(g2dCtx->pkg);
		return IM_RET_FAILED;
	}
	g2dpwl_memcpy(g2dCfg, config, sizeof(g2d_config_t));

	g2dCtx->pkg->userData = (void *)g2dCfg;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dkui_do(void *instance, IM_INT32 taskId)
{
	IM_RET ret = IM_RET_OK;
	g2dkui_context_t *g2dCtx = (g2dkui_context_t *)instance;
	IM_ASSERT(g2dCtx != IM_NULL);
	IM_ASSERT(g2dCtx->pkg != IM_NULL);
	IM_INFOMSG((IM_STR("%s(taskId=%d)++"), IM_STR(_IM_FUNC_), taskId));

	g2dCtx->pkg->pkgid = taskId;
	if(-1 == taskId)//it is sync call when taskId = -1
		g2dCtx->pkg->prio = WQ_PKG_PRIO_SYNC;
	else
		g2dCtx->pkg->prio = WQ_PKG_PRIO_CRITICAL;

	ret = wq_put_package(gG2d.wqHandle, g2dCtx->usr, g2dCtx->pkg);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkui_do(), wq_put_package() failed!")));
		return IM_RET_FAILED;
	}
	g2dCtx->pkg = IM_NULL;

	//it is sync call when taskId = -1
	if(-1 == taskId){
		ret = wq_wait_complete(gG2d.wqHandle, g2dCtx->usr, WQ_WAIT_SYNC, &g2dCtx->pkg);
		if((ret != IM_RET_OK) || (g2dCtx->pkg == IM_NULL) || (g2dCtx->pkg->userData == IM_NULL)){
			IM_ERRMSG((IM_STR("@g2dkui_do(), wq_wait_complete() failed!")));
			return IM_RET_FAILED;
		}
		g2dpwl_free(g2dCtx->pkg->userData);
		g2dpwl_free(g2dCtx->pkg);
		g2dCtx->pkg = IM_NULL;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dkui_wait(void *instance, IM_INT32 *taskId)
{
	IM_RET ret = IM_RET_OK;
	wq_package_t *pkg = IM_NULL;
	g2dkui_context_t *g2dCtx = (g2dkui_context_t *)instance;
	IM_ASSERT(g2dCtx != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	ret = wq_wait_complete(gG2d.wqHandle, g2dCtx->usr, WQ_WAIT_ASYNC, &pkg);
	if((ret != IM_RET_OK) || (pkg == IM_NULL) || (pkg->userData == IM_NULL)){
		IM_ERRMSG((IM_STR("@g2dkui_wait(), wq_wait_complete() failed or timeout!")));
		return ret;
	}

	*taskId = pkg->pkgid;
	g2dpwl_free(pkg->userData);
	g2dpwl_free(pkg);
	pkg = IM_NULL;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}


IM_RET g2dkui_suspend(void)
{
	IM_INFOMSG((IM_STR("%s(mmuInited=%d, mmuEnabled=%d)"), IM_STR(_IM_FUNC_), gG2d.mmuInited, gG2d.mmuEnabled));
	if(gG2d.mmuInited == IM_TRUE){
		if(gG2d.mmuEnabled == IM_TRUE){
			g2dpwl_mmu_disable();
		}
		g2dpwl_mmu_deinit();
		gG2d.mmuInited = IM_FALSE;
	}

	return IM_RET_OK;
}


IM_RET g2dkui_resume(void)
{
	IM_INFOMSG((IM_STR("%s(mmuInited=%d, mmuEnabled=%d)"), IM_STR(_IM_FUNC_), gG2d.mmuInited, gG2d.mmuEnabled));
	if(gG2d.mmuInited == IM_FALSE){
		g2dpwl_mmu_init();
		gG2d.mmuInited = IM_TRUE;
	}
	return IM_RET_OK;
}

