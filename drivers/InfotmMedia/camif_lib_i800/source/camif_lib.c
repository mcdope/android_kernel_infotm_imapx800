/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.0.1	leo@2012/05/11: first commit.
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camif_pwl.h"
#include "camif_lib.h"

#define DBGINFO	    0	
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMLIB_I:"
#define WARNHEAD	"CAMLIB_W:"
#define ERRHEAD		"CAMLIB_E:"
#define TIPHEAD		"CAMLIB_T:"

//
//
//
#define P	CAMIF_PATH_PREVIEW
#define C	CAMIF_PATH_CODEC

#define Y	0
#define U	1
#define V	2

#define BIT(val)	(((val) == IM_FALSE) ? 0 : 1)
#define nBIT(val)	(((val) == IM_FALSE) ? 1 : 0)

//
//
//
static CAMIF *gCamif = IM_NULL;
static IM_BOOL gInterlaced = 0;
//
//
//
IM_INLINE static IM_RET setMatrix(CAMIF_CSC_MATRIX *matrix);
IM_INLINE static IM_RET setDmaBuffer(IM_UINT32 path, IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer);
IM_INLINE static IM_RET getDmaBuffer(IM_UINT32 path, IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer);

//
//
//
IM_RET camiflib_init(void)
{
	IM_INT32 i;
	IM_UINT32 regoffset[] = {
		#include "reg_table.h"
	};

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gCamif == IM_NULL);

	//
	if(camifpwl_init() != IM_RET_OK){
		IM_ERRMSG((IM_STR("camifpwl_init() failed")));
		return IM_RET_FAILED;
	}

	//
	gCamif = camifpwl_malloc(sizeof(CAMIF));
	if(gCamif == IM_NULL){
		IM_ERRMSG((IM_STR("camifpwl_malloc(gCamif) failed")));
		goto Fail;
	}
	camifpwl_memset((void *)gCamif, 0, sizeof(CAMIF));

	//
	for(i=0; i<CIREGNUM; i++){
		gCamif->regs[i].offset = regoffset[i];
		gCamif->regs[i].value = 0;
	}
	
	return IM_RET_OK;
Fail:
	if(gCamif != IM_NULL){
		camifpwl_free((void *)gCamif);
		gCamif = IM_NULL;
	}
	camifpwl_deinit();
	return IM_RET_FAILED;
}

IM_RET camiflib_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if(gCamif == IM_NULL){
		IM_WARNMSG((IM_STR("camiflib has not been inited")));
		return IM_RET_OK;
	}

	//
	if(gCamif->pstat[P].enabled == IM_TRUE){
		camiflib_stop_preview();
	}
	if(gCamif->pstat[C].enabled == IM_TRUE){
		camiflib_stop_codec();
	}

	//
	camifpwl_free((void *)gCamif);
	gCamif = IM_NULL;

	//
	camifpwl_deinit();

	return IM_RET_OK;
}

IM_RET camiflib_check_register(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET camiflib_set_source(CAMIF_SOURCE_CONFIG *srccfg)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(srccfg != IM_NULL);
	//IM_ASSERT((gCamif->pstat[P].configured == IM_FALSE) && (gCamif->pstat[C].configured == IM_FALSE));
	IM_ASSERT((gCamif->pstat[P].enabled == IM_FALSE) && (gCamif->pstat[C].enabled == IM_FALSE));

	IM_INFOMSG((IM_STR("srccfg->xSize=%d"), srccfg->xSize));
	IM_INFOMSG((IM_STR("srccfg->ySize=%d"), srccfg->ySize));
	IM_INFOMSG((IM_STR("srccfg->xOffset=%d"), srccfg->xOffset));
	IM_INFOMSG((IM_STR("srccfg->yOffset=%d"), srccfg->yOffset));
	IM_INFOMSG((IM_STR("srccfg->ituMode=%d"), srccfg->ituMode));
	IM_INFOMSG((IM_STR("srccfg->scanMode=%d"), srccfg->scanMode));
	IM_INFOMSG((IM_STR("srccfg->yuvOrder=%d"), srccfg->yuvOrder));
	IM_INFOMSG((IM_STR("srccfg->uvOffset=%d"), srccfg->uvOffset));
	IM_INFOMSG((IM_STR("srccfg->invPinPclk=%d"), srccfg->invPinPclk));
	IM_INFOMSG((IM_STR("srccfg->invPinVsync=%d"), srccfg->invPinVsync));
	IM_INFOMSG((IM_STR("srccfg->invPinHref=%d"), srccfg->invPinHref));
	IM_INFOMSG((IM_STR("srccfg->invPinField=%d"), srccfg->invPinField));

	// check.
	if((srccfg->xOffset > 0x7FF) || (srccfg->yOffset > 0x7FF) ||
		(srccfg->xOffset >= srccfg->xSize) || (srccfg->yOffset >= srccfg->ySize)){
		IM_ERRMSG((IM_STR("invalid offset, xSize=%d, ySize=%d, xOffset=%d, yOffset=%d"), 
			srccfg->xSize, srccfg->ySize, srccfg->xOffset, srccfg->yOffset));
		return IM_RET_INVALID_PARAMETER;
	}
	IM_ASSERT((srccfg->ituMode == CAMIF_ITU_MODE_601) || (srccfg->ituMode == CAMIF_ITU_MODE_656));
	IM_ASSERT((srccfg->scanMode == CAMIF_SCAN_MODE_PROGRESSIVE) || (srccfg->scanMode == CAMIF_SCAN_MODE_INTERLACED));
	IM_ASSERT((srccfg->yuvOrder >= CAMIF_INPUT_YUV_ORDER_YCbYCr) && (srccfg->yuvOrder <= CAMIF_INPUT_YUV_ORDER_CrYCbY));

	//
	gCamif->regs[CIWDOFST].value = (srccfg->xOffset << CIWDOFST_WinHorOfst) | (srccfg->yOffset << CIWDOFST_WinVerOfst);
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIWDOFST].offset, gCamif->regs[CIWDOFST].value));

	gCamif->regs[CISRCFMT].value = (srccfg->ituMode << CISRCFMT_ITU601or656) | (srccfg->scanMode << CISRCFMT_ScanMode) |
		(BIT(srccfg->uvOffset) << CISRCFMT_UVOffset) | (srccfg->yuvOrder << CISRCFMT_Order422);
	IM_JIF(camifpwl_write_reg(gCamif->regs[CISRCFMT].offset, gCamif->regs[CISRCFMT].value));

	gCamif->regs[CIGCTRL].value &= ~((1<<CIGCTRL_InvPolCAMPCLK) | (1<<CIGCTRL_InvPolCAMVSYNC) | 
		(1<<CIGCTRL_InvPolCAMHREF) | (1<<CIGCTRL_InvPolCAMFIELD));
	gCamif->regs[CIGCTRL].value |= (BIT(srccfg->invPinPclk) << CIGCTRL_InvPolCAMPCLK) | 
		(BIT(srccfg->invPinVsync) << CIGCTRL_InvPolCAMVSYNC) | 
		(BIT(srccfg->invPinHref) << CIGCTRL_InvPolCAMHREF) | 
		(BIT(srccfg->invPinField) << CIGCTRL_InvPolCAMFIELD);

	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, gCamif->regs[CIGCTRL].value));
	
    camifpwl_memcpy((void *)&gCamif->source, (void *)srccfg, sizeof(CAMIF_SOURCE_CONFIG));
	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_set_err_intr(CAMIF_ERR_INTR_CONFIG *intrcfg)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(intrcfg != IM_NULL);
	IM_ASSERT((gCamif->pstat[P].enabled == IM_FALSE) && (gCamif->pstat[C].enabled == IM_FALSE));

	IM_INFOMSG((IM_STR("intrcfg->ch0OverFlow=%d"), intrcfg->ch0OverFlow));
	IM_INFOMSG((IM_STR("intrcfg->ch1OverFlow=%d"), intrcfg->ch1OverFlow));
	IM_INFOMSG((IM_STR("intrcfg->ch2OverFlow=%d"), intrcfg->ch2OverFlow));
	IM_INFOMSG((IM_STR("intrcfg->ch3OverFlow=%d"), intrcfg->ch3OverFlow));
	IM_INFOMSG((IM_STR("intrcfg->ch4OverFlow=%d"), intrcfg->ch4OverFlow));
	IM_INFOMSG((IM_STR("intrcfg->badSync=%d"), intrcfg->badSync));

	//
	gCamif->regs[CIGCTRL].value &= ~((1<<CIGCTRL_IRQ_OvFiCH0_en) | (1<<CIGCTRL_IRQ_OvFiCH1_en) |
					(1<<CIGCTRL_IRQ_OvFiCH2_en) | (1<<CIGCTRL_IRQ_OvFiCH3_en) |
					(1<<CIGCTRL_IRQ_OvFiCH4_en) | (1<<CIGCTRL_IRQ_Bad_SYN_en));
	gCamif->regs[CIGCTRL].value |= (BIT(intrcfg->ch0OverFlow) << CIGCTRL_IRQ_OvFiCH0_en) |
		(BIT(intrcfg->ch1OverFlow) << CIGCTRL_IRQ_OvFiCH1_en) |
		(BIT(intrcfg->ch2OverFlow) << CIGCTRL_IRQ_OvFiCH2_en) |
		(BIT(intrcfg->ch3OverFlow) << CIGCTRL_IRQ_OvFiCH3_en) |
		(BIT(intrcfg->ch4OverFlow) << CIGCTRL_IRQ_OvFiCH4_en) |
		(BIT(intrcfg->badSync) << CIGCTRL_IRQ_Bad_SYN_en);

	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, gCamif->regs[CIGCTRL].value));

	camifpwl_memcpy((void *)&gCamif->errintr, (void *)intrcfg, sizeof(CAMIF_ERR_INTR_CONFIG));
	return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_set_preview(CAMIF_PATH_CONFIG *precfg)
{
	IM_INT32 i;
	IM_BOOL useMatrix = IM_FALSE;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(precfg != IM_NULL);
	IM_ASSERT(gCamif->pstat[P].enabled == IM_FALSE);

	IM_INFOMSG((IM_STR("precfg->sampleMode=%d"), precfg->sampleMode));
	IM_INFOMSG((IM_STR("precfg->halfWordSwap=%d"), precfg->halfWordSwap));
	IM_INFOMSG((IM_STR("precfg->byteSwap=%d"), precfg->byteSwap));
	IM_INFOMSG((IM_STR("precfg->width=%d"), precfg->width));
	IM_INFOMSG((IM_STR("precfg->height=%d"), precfg->height));
	IM_INFOMSG((IM_STR("precfg->intrMode=%d"), precfg->intrMode));
	IM_INFOMSG((IM_STR("precfg->ppMode=%d"), precfg->ppMode));
	for(i=0; i<((precfg->ppMode==CAMIF_PPMODE_DISABLE)?1:precfg->ppMode); i++){
		IM_INFOMSG((IM_STR("precfg->buffY[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)precfg->buffY[i].vir_addr, precfg->buffY[i].phy_addr, precfg->buffY[i].size));
		IM_INFOMSG((IM_STR("precfg->buffCb[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)precfg->buffCb[i].vir_addr, precfg->buffCb[i].phy_addr, precfg->buffCb[i].size));
		IM_INFOMSG((IM_STR("precfg->buffCr[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)precfg->buffCr[i].vir_addr, precfg->buffCr[i].phy_addr, precfg->buffCr[i].size));
	}
	IM_INFOMSG((IM_STR("precfg->matrix.mode=%d"), precfg->matrix.mode));
	if(precfg->matrix.mode == CAMIF_MATRIX_MODE_CUSTOM){
		IM_INFOMSG((IM_STR("precfg->matrix.yuv2rgb=%d"), precfg->matrix.yuv2rgb));
		IM_INFOMSG((IM_STR("precfg->matrix.oft_a=%d"), precfg->matrix.oft_a));
		IM_INFOMSG((IM_STR("precfg->matrix.oft_b=%d"), precfg->matrix.oft_b));
		IM_INFOMSG((IM_STR("precfg->matrix.coef11=%d"), precfg->matrix.coef11));
		IM_INFOMSG((IM_STR("precfg->matrix.coef12=%d"), precfg->matrix.coef12));
		IM_INFOMSG((IM_STR("precfg->matrix.coef13=%d"), precfg->matrix.coef13));
		IM_INFOMSG((IM_STR("precfg->matrix.coef21=%d"), precfg->matrix.coef21));
		IM_INFOMSG((IM_STR("precfg->matrix.coef22=%d"), precfg->matrix.coef22));
		IM_INFOMSG((IM_STR("precfg->matrix.coef23=%d"), precfg->matrix.coef23));
		IM_INFOMSG((IM_STR("precfg->matrix.coef31=%d"), precfg->matrix.coef31));
		IM_INFOMSG((IM_STR("precfg->matrix.coef32=%d"), precfg->matrix.coef32));
		IM_INFOMSG((IM_STR("precfg->matrix.coef33=%d"), precfg->matrix.coef33));
	}
    //interlaced mode
    if(gInterlaced){
	    precfg->height = (precfg->height>>1);
	    IM_INFOMSG((IM_STR("camlib_set_preview:precfg->height %d\n"),precfg->height));
    }

	// check.
	if((precfg->sampleMode != CAMIF_SAMPLE_MODE_RGB16BPP_565) && 
		(precfg->sampleMode != CAMIF_SAMPLE_MODE_RGB16BPP_555I) &&
		(precfg->sampleMode != CAMIF_SAMPLE_MODE_RGB24BPP_0888) && 
		(precfg->sampleMode != CAMIF_SAMPLE_MODE_RGB24BPP_8880) &&
		(precfg->sampleMode != CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR) && 
		(precfg->sampleMode != CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR)){
		IM_ERRMSG((IM_STR("Invalid sampleMode(%d)"), precfg->sampleMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((precfg->intrMode != CAMIF_INTR_SOURCE_FRAME_END) &&
		(precfg->intrMode != CAMIF_INTR_SOURCE_DMA_DONE) &&
		(precfg->intrMode != CAMIF_INTR_SOURCE_SMART)){
		IM_ERRMSG((IM_STR("Invalid intrMode(%d)"), precfg->intrMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((precfg->width > CAMIF_PR_XSIZE_MAX) || (precfg->width < CAMIF_PR_XSIZE_MIN) ||
		(precfg->width & 0x3)){
		IM_ERRMSG((IM_STR("Invalid width(%d)"), precfg->width));
		return IM_RET_INVALID_PARAMETER;
	}

	if((precfg->height > CAMIF_PR_YSIZE_MAX) || (precfg->height < CAMIF_PR_YSIZE_MIN) ||
		(precfg->height & 0x1)){
		IM_ERRMSG((IM_STR("Invalid height(%d)"), precfg->height));
		return IM_RET_INVALID_PARAMETER;
	}

	if((precfg->sampleMode == CAMIF_SAMPLE_MODE_RGB16BPP_565) || 
		(precfg->sampleMode == CAMIF_SAMPLE_MODE_RGB16BPP_555I)){
		gCamif->pstat[P].lengthY = precfg->width * precfg->height << 1;
		gCamif->pstat[P].lengthCb = 0;
		gCamif->pstat[P].lengthCr = 0;
		useMatrix = IM_TRUE;
	}else if((precfg->sampleMode == CAMIF_SAMPLE_MODE_RGB24BPP_0888) ||
		(precfg->sampleMode == CAMIF_SAMPLE_MODE_RGB24BPP_8880)){
		gCamif->pstat[P].lengthY = precfg->width * precfg->height << 2;
		gCamif->pstat[P].lengthCb = 0;
		gCamif->pstat[P].lengthCr = 0;
		useMatrix = IM_TRUE;
	}else if(precfg->sampleMode == CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR){
		gCamif->pstat[P].lengthY = precfg->width * precfg->height;
		gCamif->pstat[P].lengthCb = gCamif->pstat[P].lengthY;
		gCamif->pstat[P].lengthCr = 0;
		useMatrix = IM_FALSE;
	}else if(precfg->sampleMode == CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR){
		gCamif->pstat[P].lengthY = precfg->width * precfg->height;
		gCamif->pstat[P].lengthCb = gCamif->pstat[P].lengthY >> 1;
		gCamif->pstat[P].lengthCr = 0;
		useMatrix = IM_FALSE;
	}

	for(i=0; i<((precfg->ppMode==CAMIF_PPMODE_DISABLE)?1:precfg->ppMode); i++){
		if(gCamif->pstat[P].lengthY != 0){
			if((precfg->buffY[i].phy_addr & 0x7) || (precfg->buffY[i].size < gCamif->pstat[P].lengthY)){
				IM_ERRMSG((IM_STR("Invalid buffY, vir=0x%x, phy=0x%x, size=%d, lengthY=%d"), 
					(IM_INT32)precfg->buffY[i].vir_addr, precfg->buffY[i].phy_addr, precfg->buffY[i].size, 
					gCamif->pstat[P].lengthY));
				return IM_RET_INVALID_PARAMETER;
			}
		}

		if(gCamif->pstat[P].lengthCb != 0){
			if((precfg->buffCb[i].phy_addr & 0x7) || (precfg->buffCb[i].size < gCamif->pstat[P].lengthCb)){
				IM_ERRMSG((IM_STR("Invalid buffCb, vir=0x%x, phy=0x%x, size=%d, lengthCb=%d"), 
					(IM_INT32)precfg->buffCb[i].vir_addr, precfg->buffCb[i].phy_addr, precfg->buffCb[i].size, 
					gCamif->pstat[P].lengthCb));
				return IM_RET_INVALID_PARAMETER;
			}
		}
	}

	if(useMatrix == IM_TRUE)
	{
		if((precfg->matrix.mode != CAMIF_MATRIX_MODE_DEFAULT) && (precfg->matrix.mode != CAMIF_MATRIX_MODE_CUSTOM)){
			IM_ERRMSG((IM_STR("Invalid matrix mode (%d)"), precfg->matrix.mode));
			return IM_RET_INVALID_PARAMETER;
		}
		if(precfg->matrix.mode == CAMIF_MATRIX_MODE_CUSTOM){
			if(precfg->matrix.yuv2rgb != IM_TRUE){
				IM_ERRMSG((IM_STR("don't support rgb2yuv csc")));
				return IM_RET_INVALID_PARAMETER;
			}
			if(precfg->matrix.oft_a > 31){
				IM_ERRMSG((IM_STR("Invalid matrix oft_a (%d)"), precfg->matrix.oft_a));
				return IM_RET_INVALID_PARAMETER;
			}
			if(precfg->matrix.oft_b > 31){
				IM_ERRMSG((IM_STR("Invalid matrix oft_b (%d)"), precfg->matrix.oft_b));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef11 < -1024) || (precfg->matrix.coef11 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef11 (%d)"), precfg->matrix.coef11));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef12 < -1024) || (precfg->matrix.coef12 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef12 (%d)"), precfg->matrix.coef12));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef13 < -1024) || (precfg->matrix.coef13 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef13 (%d)"), precfg->matrix.coef13));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef21 < -1024) || (precfg->matrix.coef21 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef21 (%d)"), precfg->matrix.coef21));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef22 < -1024) || (precfg->matrix.coef22 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef22 (%d)"), precfg->matrix.coef22));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef23 < -1024) || (precfg->matrix.coef23 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef23 (%d)"), precfg->matrix.coef23));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef31 < -1024) || (precfg->matrix.coef31 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef31 (%d)"), precfg->matrix.coef31));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef32 < -1024) || (precfg->matrix.coef32 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef32 (%d)"), precfg->matrix.coef32));
				return IM_RET_INVALID_PARAMETER;
			}
			if((precfg->matrix.coef33 < -1024) || (precfg->matrix.coef33 > 1023)){
				IM_ERRMSG((IM_STR("Invalid matrix coef33 (%d)"), precfg->matrix.coef33));
				return IM_RET_INVALID_PARAMETER;
			}
		}
	}


	camifpwl_memcpy((void *)&gCamif->path[P], (void *)precfg, sizeof(gCamif->path[P]));
   //check if interalced mode
    if(precfg->interlaced){
        gInterlaced = 1;
		IM_INFOMSG((IM_STR("gInterlaced (%d)"), gInterlaced));

    }  

	if(gCamif->path[P].ppMode == CAMIF_PPMODE_1_BUFFER){
		for(i=1; i<4; i++){
			IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[i], gCamif->path[P].buffY[0]);
			IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[i], gCamif->path[P].buffCb[0]);
		}
	}else if(gCamif->path[P].ppMode == CAMIF_PPMODE_2_BUFFER){
	if(!gInterlaced)	
    {    
        IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[2], gCamif->path[P].buffY[0]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[2], gCamif->path[P].buffCb[0]);

		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[3], gCamif->path[P].buffY[1]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[3], gCamif->path[P].buffCb[1]);
    }
    else
    {
        IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[0], gCamif->path[P].buffY[0]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[0], gCamif->path[P].buffCb[0]);

		gCamif->path[P].buffY[1].vir_addr = 	precfg->buffY[0].vir_addr + precfg->width*precfg->height;
		gCamif->path[P].buffY[1].phy_addr = precfg->buffY[0].phy_addr + precfg->width*precfg->height;;
		gCamif->path[P].buffY[1].size = precfg->buffY[0].size;
		gCamif->path[P].buffY[1].flag = precfg->buffY[0].flag;
		gCamif->path[P].buffCb[1].vir_addr = precfg->buffCb[0].vir_addr + (precfg->width*precfg->height>>1);
		gCamif->path[P].buffCb[1].phy_addr = precfg->buffCb[0].phy_addr +  (precfg->width*precfg->height>>1);
		gCamif->path[P].buffCb[1].size = precfg->buffCb[0].size;
		gCamif->path[P].buffCb[1].flag = precfg->buffCb[0].flag;
		
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[2], gCamif->path[P].buffY[1]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[2], gCamif->path[P].buffCb[1]);

		gCamif->path[P].buffY[3].vir_addr = 	precfg->buffY[1].vir_addr + precfg->width*precfg->height;
		gCamif->path[P].buffY[3].phy_addr = precfg->buffY[1].phy_addr + precfg->width*precfg->height;;
		gCamif->path[P].buffY[3].size = precfg->buffY[1].size;
		gCamif->path[P].buffY[3].flag = precfg->buffY[1].flag;
		gCamif->path[P].buffCb[3].vir_addr = precfg->buffCb[1].vir_addr + (precfg->width*precfg->height>>1);
		gCamif->path[P].buffCb[3].phy_addr = precfg->buffCb[1].phy_addr +  (precfg->width*precfg->height>>1);
		gCamif->path[P].buffCb[3].size = precfg->buffCb[1].size;
		gCamif->path[P].buffCb[3].flag = precfg->buffCb[1].flag;
    }
    }

	gCamif->pstat[P].configured = IM_TRUE;

	return IM_RET_OK;
}

IM_RET camiflib_set_codec(CAMIF_PATH_CONFIG *cocfg)
{
	IM_INT32 i;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(cocfg != IM_NULL);
	IM_ASSERT(gCamif->pstat[C].enabled == IM_FALSE);

	IM_INFOMSG((IM_STR("cocfg->sampleMode=%d"), cocfg->sampleMode));
	IM_INFOMSG((IM_STR("cocfg->halfWordSwap=%d"), cocfg->halfWordSwap));
	IM_INFOMSG((IM_STR("cocfg->byteSwap=%d"), cocfg->byteSwap));
	IM_INFOMSG((IM_STR("cocfg->width=%d"), cocfg->width));
	IM_INFOMSG((IM_STR("cocfg->height=%d"), cocfg->height));
	IM_INFOMSG((IM_STR("cocfg->intrMode=%d"), cocfg->intrMode));
	IM_INFOMSG((IM_STR("cocfg->ppMode=%d"), cocfg->ppMode));
	for(i=0; i<((cocfg->ppMode==CAMIF_PPMODE_DISABLE)?1:cocfg->ppMode); i++){
		IM_INFOMSG((IM_STR("cocfg->buffY[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)cocfg->buffY[i].vir_addr, cocfg->buffY[i].phy_addr, cocfg->buffY[i].size));
		IM_INFOMSG((IM_STR("cocfg->buffCb[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)cocfg->buffCb[i].vir_addr, cocfg->buffCb[i].phy_addr, cocfg->buffCb[i].size));
		IM_INFOMSG((IM_STR("cocfg->buffCr[%d] vir=0x%x, phy=0x%x, size=%d"), 
			i, (IM_INT32)cocfg->buffCr[i].vir_addr, cocfg->buffCr[i].phy_addr, cocfg->buffCr[i].size));
	}

	// check.
	if((cocfg->sampleMode != CAMIF_SAMPLE_MODE_YUV420_PLANAR) &&
		(cocfg->sampleMode != CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR) && 
		(cocfg->sampleMode != CAMIF_SAMPLE_MODE_YUV422_PLANAR) &&
		(cocfg->sampleMode != CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR) &&
		(cocfg->sampleMode != CAMIF_SAMPLE_MODE_YUV422_INTERLEAVED)){
		IM_ERRMSG((IM_STR("Invalid sampleMode(%d)"), cocfg->sampleMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((cocfg->intrMode != CAMIF_INTR_SOURCE_FRAME_END) &&
		(cocfg->intrMode != CAMIF_INTR_SOURCE_DMA_DONE) &&
		(cocfg->intrMode != CAMIF_INTR_SOURCE_SMART)){
		IM_ERRMSG((IM_STR("Invalid intrMode(%d)"), cocfg->intrMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if((cocfg->width > CAMIF_CO_XSIZE_MAX) || (cocfg->width < CAMIF_CO_XSIZE_MIN) ||
		(cocfg->width & 0x3)){
		IM_ERRMSG((IM_STR("Invalid width(%d)"), cocfg->width));
		return IM_RET_INVALID_PARAMETER;
	}

	if((cocfg->height > CAMIF_CO_YSIZE_MAX) || (cocfg->height < CAMIF_CO_YSIZE_MIN) ||
		(cocfg->height & 0x1)){
		IM_ERRMSG((IM_STR("Invalid height(%d)"), cocfg->height));
		return IM_RET_INVALID_PARAMETER;
	}

	if(cocfg->sampleMode == CAMIF_SAMPLE_MODE_YUV422_PLANAR){
		gCamif->pstat[C].lengthY = cocfg->width * cocfg->height;
		gCamif->pstat[C].lengthCb = gCamif->pstat[C].lengthY >> 1;
		gCamif->pstat[C].lengthCr = gCamif->pstat[C].lengthY >> 1;
	}else if(cocfg->sampleMode == CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR){
		gCamif->pstat[C].lengthY = cocfg->width * cocfg->height;
		gCamif->pstat[C].lengthCb = gCamif->pstat[C].lengthY;
		gCamif->pstat[C].lengthCr = 0;
	}else if(cocfg->sampleMode == CAMIF_SAMPLE_MODE_YUV422_INTERLEAVED){
		gCamif->pstat[C].lengthY = cocfg->width * cocfg->height << 1;
		gCamif->pstat[C].lengthCb = 0;
		gCamif->pstat[C].lengthCr = 0;
	}else if(cocfg->sampleMode == CAMIF_SAMPLE_MODE_YUV420_PLANAR){
		gCamif->pstat[C].lengthY = cocfg->width * cocfg->height;
		gCamif->pstat[C].lengthCb = gCamif->pstat[C].lengthY >> 2;
		gCamif->pstat[C].lengthCr = gCamif->pstat[C].lengthY >> 2;
	}else if(cocfg->sampleMode == CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR){
		gCamif->pstat[C].lengthY = cocfg->width * cocfg->height;
		gCamif->pstat[C].lengthCb = gCamif->pstat[C].lengthY >> 1;
		gCamif->pstat[C].lengthCr = 0;
	}

	for(i=0; i<((cocfg->ppMode==CAMIF_PPMODE_DISABLE)?1:cocfg->ppMode); i++){
		if(gCamif->pstat[C].lengthY != 0){
			if((cocfg->buffY[i].phy_addr & 0x7) || (cocfg->buffY[i].size < gCamif->pstat[C].lengthY)){
				IM_ERRMSG((IM_STR("Invalid buffY, vir=0x%x, phy=0x%x, size=%d, lengthY=%d"), 
					(IM_INT32)cocfg->buffY[i].vir_addr, cocfg->buffY[i].phy_addr, cocfg->buffY[i].size, 
					gCamif->pstat[C].lengthY));
				return IM_RET_INVALID_PARAMETER;
			}
		}

		if(gCamif->pstat[C].lengthCb != 0){
			if((cocfg->buffCb[i].phy_addr & 0x7) || (cocfg->buffCb[i].size < gCamif->pstat[C].lengthCb)){
				IM_ERRMSG((IM_STR("Invalid buffCb, vir=0x%x, phy=0x%x, size=%d, lengthCb=%d"), 
					(IM_INT32)cocfg->buffCb[i].vir_addr, cocfg->buffCb[i].phy_addr, cocfg->buffCb[i].size, 
					gCamif->pstat[C].lengthCb));
				return IM_RET_INVALID_PARAMETER;
			}
		}

		if(gCamif->pstat[C].lengthCr != 0){
			if((cocfg->buffCr[i].phy_addr & 0x7) || (cocfg->buffCr[i].size < gCamif->pstat[C].lengthCr)){
				IM_ERRMSG((IM_STR("Invalid buffCr, vir=0x%x, phy=0x%x, size=%d, lengthCr=%d"), 
					(IM_INT32)cocfg->buffCr[i].vir_addr, cocfg->buffCr[i].phy_addr, cocfg->buffCr[i].size, 
					gCamif->pstat[C].lengthCr));
				return IM_RET_INVALID_PARAMETER;
			}
		}
	}

	//
	camifpwl_memcpy((void *)&gCamif->path[C], (void *)cocfg, sizeof(gCamif->path[C]));

	if(gCamif->path[C].ppMode == CAMIF_PPMODE_1_BUFFER){
		for(i=1; i<4; i++){
			IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffY[i], gCamif->path[C].buffY[0]);
			IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCb[i], gCamif->path[C].buffCb[0]);
			IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCr[i], gCamif->path[C].buffCr[0]);
		}
	}else if(gCamif->path[C].ppMode == CAMIF_PPMODE_2_BUFFER){
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffY[2], gCamif->path[C].buffY[0]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCb[2], gCamif->path[C].buffCb[0]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCr[2], gCamif->path[C].buffCr[0]);

		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffY[3], gCamif->path[C].buffY[1]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCb[3], gCamif->path[C].buffCb[1]);
		IM_BUFFER_COPYTO_BUFFER(gCamif->path[C].buffCr[3], gCamif->path[C].buffCr[1]);
	}	

	gCamif->pstat[C].configured = IM_TRUE;
	
	return IM_RET_OK;
}

IM_RET camiflib_start_preview(IM_BOOL oneshot)
{
	IM_BOOL useMatrix = IM_FALSE;

	IM_INFOMSG((IM_STR("%s(oneshot=%d)"), IM_STR(_IM_FUNC_), oneshot));
	
	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(gCamif->pstat[P].configured == IM_TRUE);

	if(gCamif->pstat[P].enabled == IM_TRUE){
		IM_WARNMSG((IM_STR("preview has been started")));
		return IM_RET_OK;
	}

	// check.
	if((gCamif->path[P].width > gCamif->source.xSize - gCamif->source.xOffset) ||
		(gCamif->path[P].height > gCamif->source.ySize - gCamif->source.yOffset)){
		IM_ERRMSG((IM_STR("preview window(%d*%d) overlap source window(%d*%d)"),
			gCamif->path[P].width, gCamif->path[P].height, 
			gCamif->source.xSize - gCamif->source.xOffset, gCamif->source.ySize - gCamif->source.yOffset));
		return IM_RET_FAILED;
	}

	if(gCamif->pstat[C].configured == IM_FALSE){// if codec has not configured, there give it same size with preview.
		gCamif->path[C].width = gCamif->path[P].width;
		gCamif->path[C].height = gCamif->path[P].height;	
	}
	if(gCamif->pstat[C].enabled == IM_TRUE){
		if((gCamif->path[P].width > gCamif->path[C].width) || (gCamif->path[P].height > gCamif->path[C].height)){
			IM_ERRMSG((IM_STR("preview window(%d*%d) little then codec window(%d*%d)"),
				gCamif->path[P].width, gCamif->path[P].height, gCamif->path[C].width, gCamif->path[C].height));
			return IM_RET_FAILED;
		}
	}

	//
	gCamif->regs[CIPRTRGFMT].value = (((gCamif->path[P].halfWordSwap==IM_FALSE)?0:1) << CIPRTRGFMT_HalfWordSwap) |
		(((gCamif->path[P].byteSwap==IM_FALSE)?0:1) << CIPRTRGFMT_ByteSwap);
	if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_RGB16BPP_565){
		useMatrix = IM_TRUE;
		gCamif->regs[CIPRTRGFMT].value |= (0<<CIPRTRGFMT_StoreFormat) | (0<<CIPRTRGFMT_BPP16Format);
	}else if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_RGB16BPP_555I){
		useMatrix = IM_TRUE;
		gCamif->regs[CIPRTRGFMT].value |= (0<<CIPRTRGFMT_StoreFormat) | (1<<CIPRTRGFMT_BPP16Format);
	}else if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_RGB24BPP_0888){
		useMatrix = IM_TRUE;
		gCamif->regs[CIPRTRGFMT].value |= (1<<CIPRTRGFMT_StoreFormat) | (0<<CIPRTRGFMT_BPP24BL);
	}else if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_RGB24BPP_8880){
		useMatrix = IM_TRUE;
		gCamif->regs[CIPRTRGFMT].value |= (1<<CIPRTRGFMT_StoreFormat) | (1<<CIPRTRGFMT_BPP24BL);
	}else if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR){
		gCamif->regs[CIPRTRGFMT].value |= (3<<CIPRTRGFMT_StoreFormat);
	}else if(gCamif->path[P].sampleMode == CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR){
		gCamif->regs[CIPRTRGFMT].value |= (2<<CIPRTRGFMT_StoreFormat);
	}
    IM_INFOMSG((IM_STR("gCamif->path[C].width:%d gCamif->path[C].height:%d\n"),gCamif->path[C].width,gCamif->path[C].height));
    IM_INFOMSG((("gCamif->path[P].width:%d gCamif->path[P].height:%d\n"),gCamif->path[P].width,gCamif->path[P].height));
	gCamif->regs[CIPRTRGSIZE].value = (gCamif->path[P].width << CIPRTRGSIZE_TargetHsize) | 
		(gCamif->path[P].height << CIPRTRGSIZE_TargetVsize);
	gCamif->pstat[P].ratioX = gCamif->path[C].width / gCamif->path[P].width;
	gCamif->pstat[P].ratioY = gCamif->path[C].height / gCamif->path[P].height;
	gCamif->regs[CIPRSCPRERATIO].value = ((gCamif->pstat[P].ratioX - 1) << CIPRSCPRERATIO_PreHorRatio) |
					((gCamif->pstat[P].ratioY - 1) << CIPRSCPRERATIO_PreVerRatio);
	gCamif->regs[CIGCTRL].value &= ~(0x3 << CIGCTRL_IRQ_Int_Mask_Pr);
	gCamif->regs[CIGCTRL].value |= (1<<CIGCTRL_IRQ_en) | (gCamif->path[P].intrMode << CIGCTRL_IRQ_Int_Mask_Pr);

	if(gCamif->path[P].ppMode == CAMIF_PPMODE_DISABLE){
		if(gCamif->pstat[P].lengthY != 0){
			gCamif->regs[CICH4_ST_ADR].value = gCamif->path[P].buffY[0].phy_addr;
			gCamif->regs[CICH4_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(0<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthY >> 3) << CICHxALTCTRL_DMALen);
		}	
		if(gCamif->pstat[P].lengthCb != 0){
			gCamif->regs[CICH3_ST_ADR].value = gCamif->path[P].buffCb[0].phy_addr;
			gCamif->regs[CICH3_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(0<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthCb >> 3) << CICHxALTCTRL_DMALen);
		}
	}else{
		if(gCamif->pstat[P].lengthY != 0){
			gCamif->regs[CICH4_ST_ADR].value = gCamif->path[P].buffY[0].phy_addr;
			gCamif->regs[CICH4_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH4_ALT_ST_ADR_1].value = gCamif->path[P].buffY[1].phy_addr;
			gCamif->regs[CICH4_ALT_CTRL_1].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH4_ALT_ST_ADR_2].value = gCamif->path[P].buffY[2].phy_addr;
			gCamif->regs[CICH4_ALT_CTRL_2].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH4_ALT_ST_ADR_3].value = gCamif->path[P].buffY[3].phy_addr;
			gCamif->regs[CICH4_ALT_CTRL_3].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthY >> 3) << CICHxALTCTRL_DMALen);
		}	
		if(gCamif->pstat[P].lengthCb != 0){
			gCamif->regs[CICH3_ST_ADR].value = gCamif->path[P].buffCb[0].phy_addr;
			gCamif->regs[CICH3_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthCb >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH3_ALT_ST_ADR_1].value = gCamif->path[P].buffCb[1].phy_addr;
			gCamif->regs[CICH3_ALT_CTRL_1].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthCb >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH3_ALT_ST_ADR_2].value = gCamif->path[P].buffCb[2].phy_addr;
			gCamif->regs[CICH3_ALT_CTRL_2].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthCb >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH3_ALT_ST_ADR_3].value = gCamif->path[P].buffCb[3].phy_addr;
			gCamif->regs[CICH3_ALT_CTRL_3].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[P].lengthCb >> 3) << CICHxALTCTRL_DMALen);
		}
	}

	if(useMatrix == IM_TRUE){
		IM_JIF(setMatrix(&gCamif->path[P].matrix));
	}else{
		gCamif->regs[CIMX_CFG].value = 1<<CIMATRIX_CFG_PassBy;
		IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_CFG].offset, gCamif->regs[CIMX_CFG].value));
	}

	// when C disable, must set it size.
	if(gCamif->pstat[C].enabled == IM_FALSE){
		gCamif->regs[CICOTRGSIZE].value = (gCamif->path[C].width << CICOTRGSIZE_TargetHsize) |
			(gCamif->path[C].height << CICOTRGSIZE_TargetVsize);
		IM_JIF(camifpwl_write_reg(gCamif->regs[CICOTRGSIZE].offset, gCamif->regs[CICOTRGSIZE].value));
	}

	// reset preview DMA.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_CTRL].offset, (1<<CICHxCTRL_RST)));

	// flush preview related registers.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, gCamif->regs[CIGCTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIPRTRGSIZE].offset, gCamif->regs[CIPRTRGSIZE].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIPRTRGFMT].offset, gCamif->regs[CIPRTRGFMT].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIPRSCPRERATIO].offset, gCamif->regs[CIPRSCPRERATIO].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ST_ADR].offset, gCamif->regs[CICH3_ST_ADR].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_CTRL].offset, gCamif->regs[CICH3_CTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_ST_ADR_1].offset, gCamif->regs[CICH3_ALT_ST_ADR_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_CTRL_1].offset, gCamif->regs[CICH3_ALT_CTRL_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_ST_ADR_2].offset, gCamif->regs[CICH3_ALT_ST_ADR_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_CTRL_2].offset, gCamif->regs[CICH3_ALT_CTRL_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_ST_ADR_3].offset, gCamif->regs[CICH3_ALT_ST_ADR_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_ALT_CTRL_3].offset, gCamif->regs[CICH3_ALT_CTRL_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ST_ADR].offset, gCamif->regs[CICH4_ST_ADR].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_CTRL].offset, gCamif->regs[CICH4_CTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_ST_ADR_1].offset, gCamif->regs[CICH4_ALT_ST_ADR_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_CTRL_1].offset, gCamif->regs[CICH4_ALT_CTRL_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_ST_ADR_2].offset, gCamif->regs[CICH4_ALT_ST_ADR_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_CTRL_2].offset, gCamif->regs[CICH4_ALT_CTRL_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_ST_ADR_3].offset, gCamif->regs[CICH4_ALT_ST_ADR_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_ALT_CTRL_3].offset, gCamif->regs[CICH4_ALT_CTRL_3].value));

	// start capture.
	gCamif->regs[CIIMGCPT].value &= ~((1<<CIIMGCPT_ImgCptEn_PrSc) | (1<<CIIMGCPT_OneShot_PrSc));
	gCamif->regs[CIIMGCPT].value |= (1<<CIIMGCPT_CAMIF_EN) | (1<<CIIMGCPT_ImgCptEn_PrSc) 
		| (BIT(oneshot) << CIIMGCPT_OneShot_PrSc);
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIIMGCPT].offset, gCamif->regs[CIIMGCPT].value));
	
	gCamif->pstat[P].enabled = IM_TRUE;
	gCamif->pstat[P].oneshot = oneshot;
#if 0
	IM_JIF(camifpwl_read_reg(gCamif->regs[CISRCFMT].offset, &gCamif->regs[CISRCFMT].value));
    IM_INFOMSG((("-----CISRCFMT---0x%x\n"),gCamif->regs[CISRCFMT].value));	
	IM_JIF(camifpwl_read_reg(gCamif->regs[CIGCTRL].offset, &gCamif->regs[CIGCTRL].value));
    IM_INFOMSG((("-----CIGCTRL---0x%x\n"),gCamif->regs[CIGCTRL].value));	
	IM_JIF(camifpwl_read_reg(gCamif->regs[CIPRTRGSIZE].offset, &gCamif->regs[CIPRTRGSIZE].value));
    IM_INFOMSG((("-----CIPRTRGSIZE---0x%x\n"),gCamif->regs[CIPRTRGSIZE].value));	
	IM_JIF(camifpwl_read_reg(gCamif->regs[CIPRTRGFMT].offset, &gCamif->regs[CIPRTRGFMT].value));
    IM_INFOMSG((("-----CIPRTRGFMT---0x%x\n"),gCamif->regs[CIPRTRGFMT].value));	
	IM_JIF(camifpwl_read_reg(gCamif->regs[CIIMGCPT].offset, &gCamif->regs[CIIMGCPT].value));
    IM_INFOMSG((("-----CIIMGCPT---0x%x\n"),gCamif->regs[CIIMGCPT].value));	
#endif
    return IM_RET_OK;

Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_stop_preview(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gCamif != IM_NULL);

	if(gCamif->pstat[P].enabled == IM_FALSE){
		IM_WARNMSG((IM_STR("preview has not been started")));
		return IM_RET_OK;
	}

	// stop capture.
	gCamif->regs[CIIMGCPT].value &= ~((1<<CIIMGCPT_CAMIF_EN) | (1<<CIIMGCPT_ImgCptEn_PrSc) | (1<<CIIMGCPT_OneShot_PrSc));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIIMGCPT].offset, gCamif->regs[CIIMGCPT].value));
	gCamif->pstat[P].enabled = IM_FALSE;
	gCamif->pstat[P].oneshot = IM_FALSE;

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_start_codec(IM_BOOL oneshot)
{
	IM_UINT32 ratioX, ratioY;

	IM_INFOMSG((IM_STR("%s(oneshot=%d)"), IM_STR(_IM_FUNC_), oneshot));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(gCamif->pstat[C].configured == IM_TRUE);

	if(gCamif->pstat[C].enabled == IM_TRUE){
		IM_WARNMSG((IM_STR("codec has been started")));
		return IM_RET_OK;
	}

	// check.
	if(gCamif->path[C].width > (gCamif->source.xSize - gCamif->source.xOffset)){
		IM_ERRMSG((IM_STR("codec window width invalid, width=%d, xSize=%d, xOffset=%d"),
			gCamif->path[C].width, gCamif->source.xSize, gCamif->source.xOffset));
		return IM_RET_FAILED;
	}
	if(gCamif->path[C].height > (gCamif->source.ySize - gCamif->source.yOffset)){
		IM_ERRMSG((IM_STR("codec window height invalid, height=%d, ySize=%d, yOffset=%d"),
			gCamif->path[C].height, gCamif->source.ySize, gCamif->source.yOffset));
		return IM_RET_FAILED;
	}

	if(gCamif->pstat[P].enabled == IM_TRUE){
		if((gCamif->path[C].width < gCamif->path[P].width) || (gCamif->path[C].height < gCamif->path[P].height)){
			IM_ERRMSG((IM_STR("Invalid codec window, codec=%d*%d, preview=%d*%d"),
				gCamif->path[C].width, gCamif->path[C].height, gCamif->path[P].width, gCamif->path[P].height));
			return IM_RET_FAILED;
		}

		ratioX = gCamif->path[C].width / gCamif->path[P].width;
		ratioY = gCamif->path[C].height / gCamif->path[P].height;
		if((ratioX != gCamif->pstat[P].ratioX) || (ratioY != gCamif->pstat[P].ratioY)){
			IM_ERRMSG((IM_STR("Because pr-ratio has changed, so you must stop-preview first")));
			return IM_RET_FAILED;
		}
	}

	gCamif->regs[CICOTRGFMT].value = (BIT(gCamif->path[C].halfWordSwap) << CICOTRGFMT_HalfWordSwap) |
					(BIT(gCamif->path[C].byteSwap) << CICOTRGFMT_ByteSwap);
	if(gCamif->path[C].sampleMode == CAMIF_SAMPLE_MODE_YUV422_PLANAR){
		gCamif->regs[CICOTRGFMT].value |= (1<<CICOTRGFMT_ycc422) | (0<<CICOTRGFMT_StoredFormat);
	}else if(gCamif->path[C].sampleMode == CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR){
		gCamif->regs[CICOTRGFMT].value |= (1<<CICOTRGFMT_ycc422) | (1<<CICOTRGFMT_StoredFormat);
	}else if(gCamif->path[C].sampleMode == CAMIF_SAMPLE_MODE_YUV422_INTERLEAVED){
		gCamif->regs[CICOTRGFMT].value |= (1<<CICOTRGFMT_ycc422) | (2<<CICOTRGFMT_StoredFormat);
	}else if(gCamif->path[C].sampleMode == CAMIF_SAMPLE_MODE_YUV420_PLANAR){
		gCamif->regs[CICOTRGFMT].value |= (0<<CICOTRGFMT_ycc422) | (0<<CICOTRGFMT_StoredFormat);
	}else if(gCamif->path[C].sampleMode == CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR){
		gCamif->regs[CICOTRGFMT].value |= (0<<CICOTRGFMT_ycc422) | (1<<CICOTRGFMT_StoredFormat);
	}

	gCamif->regs[CICOTRGSIZE].value = (gCamif->path[C].width << CICOTRGSIZE_TargetHsize) | 
				(gCamif->path[C].height << CICOTRGSIZE_TargetVsize);
	gCamif->regs[CIGCTRL].value &= ~(0x3 << CIGCTRL_IRQ_Int_Mask_Co);
	gCamif->regs[CIGCTRL].value |= (1<<CIGCTRL_IRQ_en) | (gCamif->path[C].intrMode << CIGCTRL_IRQ_Int_Mask_Co);

	if(gCamif->path[C].ppMode == CAMIF_PPMODE_DISABLE){
		if(gCamif->pstat[C].lengthY != 0){
			gCamif->regs[CICH2_ST_ADR].value = gCamif->path[C].buffY[0].phy_addr;
			gCamif->regs[CICH2_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(0<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
		}	
		if(gCamif->pstat[C].lengthCb != 0){
			gCamif->regs[CICH1_ST_ADR].value = gCamif->path[C].buffCb[0].phy_addr;
			gCamif->regs[CICH1_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(0<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCb >> 3) << CICHxALTCTRL_DMALen);
		}
		if(gCamif->pstat[C].lengthCr != 0){
			gCamif->regs[CICH0_ST_ADR].value = gCamif->path[C].buffCr[0].phy_addr;
			gCamif->regs[CICH0_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(0<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCr >> 3) << CICHxALTCTRL_DMALen);
		}
	}else{
		if(gCamif->pstat[C].lengthY != 0){
			gCamif->regs[CICH2_ST_ADR].value = gCamif->path[C].buffY[0].phy_addr;
			gCamif->regs[CICH2_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH2_ALT_ST_ADR_1].value = gCamif->path[C].buffY[1].phy_addr;
			gCamif->regs[CICH2_ALT_CTRL_1].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH2_ALT_ST_ADR_2].value = gCamif->path[C].buffY[2].phy_addr;
			gCamif->regs[CICH2_ALT_CTRL_2].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH2_ALT_ST_ADR_3].value = gCamif->path[C].buffY[3].phy_addr;
			gCamif->regs[CICH2_ALT_CTRL_3].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
		}	
		if(gCamif->pstat[C].lengthCb != 0){
			gCamif->regs[CICH1_ST_ADR].value = gCamif->path[C].buffCb[0].phy_addr;
			gCamif->regs[CICH1_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCb >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH1_ALT_ST_ADR_1].value = gCamif->path[C].buffCb[1].phy_addr;
			gCamif->regs[CICH1_ALT_CTRL_1].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCb >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH1_ALT_ST_ADR_2].value = gCamif->path[C].buffCb[2].phy_addr;
			gCamif->regs[CICH1_ALT_CTRL_2].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthY >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH1_ALT_ST_ADR_3].value = gCamif->path[C].buffCb[3].phy_addr;
			gCamif->regs[CICH1_ALT_CTRL_3].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCb >> 3) << CICHxALTCTRL_DMALen);
		}
		if(gCamif->pstat[C].lengthCr != 0){
			gCamif->regs[CICH0_ST_ADR].value = gCamif->path[C].buffCr[0].phy_addr;
			gCamif->regs[CICH0_CTRL].value = (1<<CICHxCTRL_DMAEn) | (0<<CICHxCTRL_RST) |
				(1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCr >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH0_ALT_ST_ADR_1].value = gCamif->path[C].buffCr[1].phy_addr;
			gCamif->regs[CICH0_ALT_CTRL_1].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCr >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH0_ALT_ST_ADR_2].value = gCamif->path[C].buffCr[2].phy_addr;
			gCamif->regs[CICH0_ALT_CTRL_2].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) | 
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCr >> 3) << CICHxALTCTRL_DMALen);
			gCamif->regs[CICH0_ALT_ST_ADR_3].value = gCamif->path[C].buffCr[3].phy_addr;
			gCamif->regs[CICH0_ALT_CTRL_3].value = (1<<CICHxCTRL_AltEn) | (1<<CICHxCTRL_Autoload) |	
				(1<<CICHxALTCTRL_Dir) | ((gCamif->pstat[C].lengthCr >> 3) << CICHxALTCTRL_DMALen);
		}
	}

	// reset codec DMA.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_CTRL].offset, (1<<CICHxCTRL_RST)));

	// flush codec related registers.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, gCamif->regs[CIGCTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICOTRGSIZE].offset, gCamif->regs[CICOTRGSIZE].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICOTRGFMT].offset, gCamif->regs[CICOTRGFMT].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ST_ADR].offset, gCamif->regs[CICH0_ST_ADR].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_CTRL].offset, gCamif->regs[CICH0_CTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_ST_ADR_1].offset, gCamif->regs[CICH0_ALT_ST_ADR_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_CTRL_1].offset, gCamif->regs[CICH0_ALT_CTRL_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_ST_ADR_2].offset, gCamif->regs[CICH0_ALT_ST_ADR_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_CTRL_2].offset, gCamif->regs[CICH0_ALT_CTRL_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_ST_ADR_3].offset, gCamif->regs[CICH0_ALT_ST_ADR_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_ALT_CTRL_3].offset, gCamif->regs[CICH0_ALT_CTRL_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_CTRL].offset, gCamif->regs[CICH1_CTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_ST_ADR_1].offset, gCamif->regs[CICH1_ALT_ST_ADR_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_CTRL_1].offset, gCamif->regs[CICH1_ALT_CTRL_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_ST_ADR_2].offset, gCamif->regs[CICH1_ALT_ST_ADR_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_CTRL_2].offset, gCamif->regs[CICH1_ALT_CTRL_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_ST_ADR_3].offset, gCamif->regs[CICH1_ALT_ST_ADR_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_ALT_CTRL_3].offset, gCamif->regs[CICH1_ALT_CTRL_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ST_ADR].offset, gCamif->regs[CICH2_ST_ADR].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_CTRL].offset, gCamif->regs[CICH2_CTRL].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_ST_ADR_1].offset, gCamif->regs[CICH2_ALT_ST_ADR_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_CTRL_1].offset, gCamif->regs[CICH2_ALT_CTRL_1].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_ST_ADR_2].offset, gCamif->regs[CICH2_ALT_ST_ADR_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_CTRL_2].offset, gCamif->regs[CICH2_ALT_CTRL_2].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_ST_ADR_3].offset, gCamif->regs[CICH2_ALT_ST_ADR_3].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_ALT_CTRL_3].offset, gCamif->regs[CICH2_ALT_CTRL_3].value));

	// start capture.
	gCamif->regs[CIIMGCPT].value &= ~((1<<CIIMGCPT_ImgCptEn_CoSc) | (1<<CIIMGCPT_OneShot_CoSc));
	gCamif->regs[CIIMGCPT].value |= (1<<CIIMGCPT_CAMIF_EN) | (1<<CIIMGCPT_ImgCptEn_CoSc) 
		| (BIT(oneshot == IM_FALSE) << CIIMGCPT_OneShot_CoSc);
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIIMGCPT].offset, gCamif->regs[CIIMGCPT].value));
	
	gCamif->pstat[C].enabled = IM_TRUE;
	gCamif->pstat[C].oneshot = oneshot;

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_stop_codec(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gCamif != IM_NULL);

	if(gCamif->pstat[C].enabled == IM_FALSE){
		IM_WARNMSG((IM_STR("codec has not been started")));
		return IM_RET_OK;
	}

	// stop capture.
	gCamif->regs[CIIMGCPT].value &= ~((1<<CIIMGCPT_ImgCptEn_CoSc) | (1<<CIIMGCPT_OneShot_CoSc));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIIMGCPT].offset, gCamif->regs[CIIMGCPT].value));
	
	gCamif->pstat[C].enabled = IM_FALSE;
	gCamif->pstat[C].oneshot = IM_FALSE;

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout)
{
	IM_RET ret;

	IM_INFOMSG((IM_STR("%s(timeout=%d)"), IM_STR(_IM_FUNC_), timeout));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(intr != IM_NULL);
	IM_ASSERT((gCamif->pstat[P].enabled == IM_TRUE) || (gCamif->pstat[C].enabled == IM_TRUE));

	ret = camifpwl_wait_hw_ready(intr, timeout);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("dwl_wait_hw_ready() failed, ret=%d"), ret));
		return ret;
	}
	IM_INFOMSG((IM_STR("wait ok, actual=0x%x"), *intr));

	if(*intr & CAMIF_INTR_ERROR){
		IM_ERRMSG((IM_STR("DWL_CAM_INTR_ERROR")));
		return IM_RET_FAILED;
	}
	
	return IM_RET_OK;
}

IM_RET camiflib_get_ready_buffer(IM_UINT32 path, IM_Buffer *readyBuffs, IM_Buffer *replaceBuffs)
{
	IM_INT32 index, i;

	IM_INFOMSG((IM_STR("%s(path=%d)"), IM_STR(_IM_FUNC_), path));

	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT((readyBuffs != IM_NULL) && (replaceBuffs != IM_NULL));
	IM_ASSERT((path==P) || (path==C));

	if(path == P)
	{
		if(gCamif->pstat[P].enabled == IM_FALSE){
			IM_ERRMSG((IM_STR("preview path is disabled")));
			return IM_RET_FAILED;
		}
	
		// get ready chx number.
		IM_JIF(camifpwl_read_reg(gCamif->regs[CICH4_CTRL].offset, &gCamif->regs[CICH4_CTRL].value));
		index =  (gCamif->regs[CICH4_CTRL].value >> CICHxCTRL_WorkRegInd) & 0x03;
		if(gCamif->path[P].ppMode != CAMIF_PPMODE_DISABLE){
            //interlaced mode
            if(gInterlaced)
            {    
			    index = (index+2)%4;
			
			    if(1 == index ||3 == index ){
				    index = index -1;
				}	
			}
            else
            {    
			    index = (index != 0)?(index - 1):3;
		    }
        }
		IM_INFOMSG((IM_STR("preview ready bufer index=%d"), index));

		if(gCamif->pstat[P].lengthY != 0){
			if((replaceBuffs[0].phy_addr & 0x07) || (replaceBuffs[0].size < gCamif->pstat[P].lengthY)){
				IM_ERRMSG((IM_STR("Invalid replaceBuff[0], vir=0x%x, phy=0x%x, size=%d"),
					(IM_INT32)replaceBuffs[0].vir_addr, replaceBuffs[0].phy_addr, replaceBuffs[0].size));
				return IM_RET_FAILED;
			}
			IM_JIF(getDmaBuffer(P, Y, index, &readyBuffs[0]));
			IM_JIF(setDmaBuffer(P, Y, index, &replaceBuffs[0]));

			if(gCamif->path[P].ppMode == CAMIF_PPMODE_1_BUFFER){
				for(i=0; i<4; i++){
					if(i != index){
						IM_JIF(setDmaBuffer(P, Y, i, &replaceBuffs[0]));
					}
				}
			}else if(gCamif->path[P].ppMode == CAMIF_PPMODE_2_BUFFER){
            if(gInterlaced){
			    	IM_JIF(setDmaBuffer(P, Y, (index+1)%4, &replaceBuffs[0]));
            }   
            else{
				    IM_JIF(setDmaBuffer(P, Y, (index+2)%4, &replaceBuffs[0]));
                 }
			}
		}

		if(gCamif->pstat[P].lengthCb != 0){
			if((replaceBuffs[1].phy_addr & 0x07) || (replaceBuffs[1].size < gCamif->pstat[P].lengthCb)){
				IM_ERRMSG((IM_STR("Invalid replaceBuff[1], vir=0x%x, phy=0x%x, size=%d"),
					(IM_INT32)replaceBuffs[1].vir_addr, replaceBuffs[1].phy_addr, replaceBuffs[1].size));
				return IM_RET_FAILED;
			}
			IM_JIF(getDmaBuffer(P, U, index, &readyBuffs[1]));
			IM_JIF(setDmaBuffer(P, U, index, &replaceBuffs[1]));

			if(gCamif->path[P].ppMode == CAMIF_PPMODE_1_BUFFER){
				for(i=0; i<4; i++){
					if(i != index){
						IM_JIF(setDmaBuffer(P, U, i, &replaceBuffs[1]));
					}
				}
			}else if(gCamif->path[P].ppMode == CAMIF_PPMODE_2_BUFFER){
				IM_JIF(setDmaBuffer(P, U, (index+2)%4, &replaceBuffs[1]));
			}
		}
		IM_ASSERT(gCamif->pstat[P].lengthCr == 0);

		if(gCamif->pstat[P].oneshot == IM_TRUE){
			IM_JIF(camifpwl_read_reg(gCamif->regs[CIIMGCPT].offset, &gCamif->regs[CIIMGCPT].value));
			IM_ASSERT((gCamif->regs[CIIMGCPT].value & (1<<CIIMGCPT_ImgCptEn_PrSc)) == 0);
			gCamif->pstat[P].oneshot = IM_FALSE;
			gCamif->pstat[P].enabled = IM_FALSE;
		}
	}
	else if(path == C)
	{
		if(gCamif->pstat[C].enabled == IM_FALSE){
			IM_ERRMSG((IM_STR("codec path is disabled")));
			return IM_RET_FAILED;
		}
	
		// get ready chx number.
		IM_JIF(camifpwl_read_reg(gCamif->regs[CICH2_CTRL].offset, &gCamif->regs[CICH2_CTRL].value));
		index =  (gCamif->regs[CICH2_CTRL].value >> CICHxCTRL_WorkRegInd) & 0x03;
		if(gCamif->path[C].ppMode != CAMIF_PPMODE_DISABLE){
			index = (index != 0)?(index - 1):3;
		}
		IM_INFOMSG((IM_STR("codec ready bufer index=%d"), index));

		if(gCamif->pstat[C].lengthY != 0){
			if((replaceBuffs[0].phy_addr & 0x07) || (replaceBuffs[0].size < gCamif->pstat[C].lengthY)){
				IM_ERRMSG((IM_STR("Invalid replaceBuff[0], vir=0x%x, phy=0x%x, size=%d"),
					(IM_INT32)replaceBuffs[0].vir_addr, replaceBuffs[0].phy_addr, replaceBuffs[0].size));
				return IM_RET_FAILED;
			}
			IM_JIF(getDmaBuffer(C, Y, index, &readyBuffs[0]));
			IM_JIF(setDmaBuffer(C, Y, index, &replaceBuffs[0]));

			if(gCamif->path[C].ppMode == CAMIF_PPMODE_1_BUFFER){
				for(i=0; i<4; i++){
					if(i != index){
						IM_JIF(setDmaBuffer(C, Y, i, &replaceBuffs[0]));
					}
				}
			}else if(gCamif->path[C].ppMode == CAMIF_PPMODE_2_BUFFER){
				IM_JIF(setDmaBuffer(C, Y, (index+2)%4, &replaceBuffs[0]));
			}
		}

		if(gCamif->pstat[C].lengthCb != 0){
			if((replaceBuffs[1].phy_addr & 0x07) || (replaceBuffs[1].size < gCamif->pstat[C].lengthCb)){
				IM_ERRMSG((IM_STR("Invalid replaceBuff[1], vir=0x%x, phy=0x%x, size=%d"),
					(IM_INT32)replaceBuffs[1].vir_addr, replaceBuffs[1].phy_addr, replaceBuffs[1].size));
				return IM_RET_FAILED;
			}
			IM_JIF(getDmaBuffer(C, U, index, &readyBuffs[1]));
			IM_JIF(setDmaBuffer(C, U, index, &replaceBuffs[1]));

			if(gCamif->path[C].ppMode == CAMIF_PPMODE_1_BUFFER){
				for(i=0; i<4; i++){
					if(i != index){
						IM_JIF(setDmaBuffer(C, U, i, &replaceBuffs[1]));
					}
				}
			}else if(gCamif->path[C].ppMode == CAMIF_PPMODE_2_BUFFER){
				IM_JIF(setDmaBuffer(C, U, (index+2)%4, &replaceBuffs[1]));
			}
		}

		if(gCamif->pstat[C].lengthCr != 0){
			if((replaceBuffs[2].phy_addr & 0x07) || (replaceBuffs[2].size < gCamif->pstat[C].lengthCr)){
				IM_ERRMSG((IM_STR("Invalid replaceBuff[2], vir=0x%x, phy=0x%x, size=%d"),
					(IM_INT32)replaceBuffs[2].vir_addr, replaceBuffs[2].phy_addr, replaceBuffs[2].size));
				return IM_RET_FAILED;
			}
			IM_JIF(getDmaBuffer(C, V, index, &readyBuffs[2]));
			IM_JIF(setDmaBuffer(C, V, index, &replaceBuffs[2]));

			if(gCamif->path[C].ppMode == CAMIF_PPMODE_1_BUFFER){
				for(i=0; i<4; i++){
					if(i != index){
						IM_JIF(setDmaBuffer(C, V, i, &replaceBuffs[2]));
					}
				}
			}else if(gCamif->path[C].ppMode == CAMIF_PPMODE_2_BUFFER){
				IM_JIF(setDmaBuffer(C, V, (index+2)%4, &replaceBuffs[2]));
			}
		}

		if(gCamif->pstat[C].oneshot == IM_TRUE){
			IM_JIF(camifpwl_read_reg(gCamif->regs[CIIMGCPT].offset, &gCamif->regs[CIIMGCPT].value));
			IM_ASSERT((gCamif->regs[CIIMGCPT].value & (1<<CIIMGCPT_ImgCptEn_CoSc)) == 0);
			gCamif->pstat[C].oneshot = IM_FALSE;
			gCamif->pstat[C].enabled = IM_FALSE;
		}

	}

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_set_source_offset(IM_UINT32 xOffset, IM_UINT32 yOffset)
{
	IM_INFOMSG((IM_STR("%s(xOffset=%d, yOffset=%d)"), IM_STR(_IM_FUNC_), xOffset, yOffset));
	
	IM_ASSERT(gCamif != IM_NULL);

	if((xOffset > 0x7FF) || (yOffset > 0x7FF) ||
		(xOffset >= gCamif->source.xSize) || (yOffset >= gCamif->source.ySize)){
		IM_ERRMSG((IM_STR("invalid offset, xSize=%d, ySize=%d, xOffset=%d, yOffset=%d"), 
			gCamif->source.xSize, gCamif->source.ySize, xOffset, yOffset));
		return IM_RET_INVALID_PARAMETER;
	}

	if((gCamif->pstat[P].enabled == IM_TRUE) || (gCamif->pstat[C].enabled == IM_TRUE))
	{
		if((gCamif->path[C].width < (gCamif->source.xSize - xOffset)) ||
			(gCamif->path[C].height < (gCamif->source.ySize - yOffset))){
			IM_ERRMSG((IM_STR("invalid offset, codec=%d*%d, source=%d*%d, xOffset=%d, yOffset=%d"), 
				gCamif->path[C].width, gCamif->path[C].height, gCamif->source.xSize, gCamif->source.ySize, xOffset, yOffset));
			return IM_RET_INVALID_PARAMETER;
		}
	}

	gCamif->regs[CIWDOFST].value = (xOffset << CIWDOFST_WinHorOfst) | (yOffset << CIWDOFST_WinVerOfst);
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIWDOFST].offset, gCamif->regs[CIWDOFST].value));

	gCamif->source.xOffset = xOffset;
	gCamif->source.yOffset = yOffset;

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_INLINE static IM_RET setMatrix(CAMIF_CSC_MATRIX *matrix)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(matrix->mode == CAMIF_MATRIX_MODE_DEFAULT){
		gCamif->regs[CIMX_CFG].value = (1<<CIMATRIX_CFG_toRGB) | (0<<CIMATRIX_CFG_PassBy) | 
			(0<<CIMATRIX_CFG_InvMsbIn) | (0<<CIMATRIX_CFG_InvMsbOut) |
			(16<<CIMATRIX_CFG_OftB) | (0<<CIMATRIX_CFG_OftA);
		gCamif->regs[CIMX_COEF11].value = (IM_UINT32)298;
		gCamif->regs[CIMX_COEF12].value = (IM_UINT32)0;
		gCamif->regs[CIMX_COEF13].value = (IM_UINT32)409;
		gCamif->regs[CIMX_COEF21].value = (IM_UINT32)298;
		gCamif->regs[CIMX_COEF22].value = (IM_UINT32)-100;
		gCamif->regs[CIMX_COEF23].value = (IM_UINT32)208;
		gCamif->regs[CIMX_COEF31].value = (IM_UINT32)298;
		gCamif->regs[CIMX_COEF32].value = (IM_UINT32)516;
		gCamif->regs[CIMX_COEF33].value = (IM_UINT32)0;
	}
	else if(matrix->mode == CAMIF_MATRIX_MODE_CUSTOM){
		gCamif->regs[CIMX_CFG].value = (1<<CIMATRIX_CFG_toRGB) | (0<<CIMATRIX_CFG_PassBy) | 
			(0<<CIMATRIX_CFG_InvMsbIn) | (0<<CIMATRIX_CFG_InvMsbOut) |
			(matrix->oft_b << CIMATRIX_CFG_OftB) | (matrix->oft_a << CIMATRIX_CFG_OftA);
		gCamif->regs[CIMX_COEF11].value = matrix->coef11;
		gCamif->regs[CIMX_COEF12].value = matrix->coef12;
		gCamif->regs[CIMX_COEF13].value = matrix->coef13;
		gCamif->regs[CIMX_COEF21].value = matrix->coef21;
		gCamif->regs[CIMX_COEF22].value = matrix->coef22;
		gCamif->regs[CIMX_COEF23].value = matrix->coef23;
		gCamif->regs[CIMX_COEF31].value = matrix->coef31;
		gCamif->regs[CIMX_COEF32].value = matrix->coef32;
		gCamif->regs[CIMX_COEF33].value = matrix->coef33;
	}	
	
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_CFG].offset, gCamif->regs[CIMX_CFG].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF11].offset, gCamif->regs[CIMX_COEF11].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF12].offset, gCamif->regs[CIMX_COEF12].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF13].offset, gCamif->regs[CIMX_COEF13].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF21].offset, gCamif->regs[CIMX_COEF21].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF22].offset, gCamif->regs[CIMX_COEF22].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF23].offset, gCamif->regs[CIMX_COEF23].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF31].offset, gCamif->regs[CIMX_COEF31].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF32].offset, gCamif->regs[CIMX_COEF32].value));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIMX_COEF33].offset, gCamif->regs[CIMX_COEF33].value));

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_set_matrix(CAMIF_CSC_MATRIX *matrix)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT(matrix != IM_NULL);

	IM_INFOMSG((IM_STR("matrix->mode=%d"), matrix->mode));
	IM_INFOMSG((IM_STR("matrix->yuv2rgb=%d"), matrix->yuv2rgb));
	IM_INFOMSG((IM_STR("matrix->oft_a=%d"), matrix->oft_a));
	IM_INFOMSG((IM_STR("matrix->oft_b=%d"), matrix->oft_b));
	IM_INFOMSG((IM_STR("matrix->coef11=%d"), matrix->coef11));
	IM_INFOMSG((IM_STR("matrix->coef12=%d"), matrix->coef12));
	IM_INFOMSG((IM_STR("matrix->coef13=%d"), matrix->coef13));
	IM_INFOMSG((IM_STR("matrix->coef21=%d"), matrix->coef21));
	IM_INFOMSG((IM_STR("matrix->coef22=%d"), matrix->coef22));
	IM_INFOMSG((IM_STR("matrix->coef23=%d"), matrix->coef23));
	IM_INFOMSG((IM_STR("matrix->coef31=%d"), matrix->coef31));
	IM_INFOMSG((IM_STR("matrix->coef32=%d"), matrix->coef32));
	IM_INFOMSG((IM_STR("matrix->coef33=%d"), matrix->coef33));

	if((matrix->mode != CAMIF_MATRIX_MODE_DEFAULT) && (matrix->mode != CAMIF_MATRIX_MODE_CUSTOM)){
		IM_ERRMSG((IM_STR("Invalid matrix mode (%d)"), matrix->mode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(matrix->mode == CAMIF_MATRIX_MODE_CUSTOM){
		if(matrix->yuv2rgb != IM_TRUE){
			IM_ERRMSG((IM_STR("don't support rgb2yuv csc")));
			return IM_RET_INVALID_PARAMETER;
		}
		if(matrix->oft_a > 31){
			IM_ERRMSG((IM_STR("Invalid matrix oft_a (%d)"), matrix->oft_a));
			return IM_RET_INVALID_PARAMETER;
		}
		if(matrix->oft_b > 31){
			IM_ERRMSG((IM_STR("Invalid matrix oft_b (%d)"), matrix->oft_b));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef11 < -1024) || (matrix->coef11 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef11 (%d)"), matrix->coef11));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef12 < -1024) || (matrix->coef12 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef12 (%d)"), matrix->coef12));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef13 < -1024) || (matrix->coef13 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef13 (%d)"), matrix->coef13));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef21 < -1024) || (matrix->coef21 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef21 (%d)"), matrix->coef21));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef22 < -1024) || (matrix->coef22 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef22 (%d)"), matrix->coef22));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef23 < -1024) || (matrix->coef23 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef23 (%d)"), matrix->coef23));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef31 < -1024) || (matrix->coef31 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef31 (%d)"), matrix->coef31));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef32 < -1024) || (matrix->coef32 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef32 (%d)"), matrix->coef32));
			return IM_RET_INVALID_PARAMETER;
		}
		if((matrix->coef33 < -1024) || (matrix->coef33 > 1023)){
			IM_ERRMSG((IM_STR("Invalid matrix coef33 (%d)"), matrix->coef33));
			return IM_RET_INVALID_PARAMETER;
		}
	}

	if(setMatrix(matrix) == IM_RET_OK){
		IM_ERRMSG((IM_STR("setMatrix() failed")));
		return IM_RET_FAILED;
	}
	camifpwl_memcpy((void *)&gCamif->path[P].matrix, (void *)matrix, sizeof(CAMIF_CSC_MATRIX));

	return IM_RET_OK;
}

IM_RET camiflib_hw_reset()
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(gCamif != IM_NULL);
	IM_ASSERT((gCamif->pstat[P].enabled == IM_FALSE) && (gCamif->pstat[C].enabled == IM_FALSE));

	// reset camif.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, (1<<CIGCTRL_SwRst)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, (0<<CIGCTRL_SwRst)));
	
	// reset DMAs.
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH0_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH1_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH2_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH3_CTRL].offset, (1<<CICHxCTRL_RST)));
	IM_JIF(camifpwl_write_reg(gCamif->regs[CICH4_CTRL].offset, (1<<CICHxCTRL_RST)));

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_RET camiflib_set_reset_pin(IM_BOOL elecLevel)
{
	IM_INFOMSG((IM_STR("%s(elecLevel=%d)"), IM_STR(_IM_FUNC_), elecLevel));

	IM_ASSERT(gCamif != IM_NULL);

	if(elecLevel == IM_TRUE){
		gCamif->regs[CIGCTRL].value |= 1<<CIGCTRL_CamRst;
	}else{
		gCamif->regs[CIGCTRL].value &= ~(1<<CIGCTRL_CamRst);
	}

	IM_JIF(camifpwl_write_reg(gCamif->regs[CIGCTRL].offset, gCamif->regs[CIGCTRL].value));
	
	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_INLINE static IM_RET setDmaBuffer(IM_UINT32 path, IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer)
{
	IM_INT32 regIndex;

	IM_INFOMSG((IM_STR("%s(path=%d, ch=%d, alt=%d)"), IM_STR(_IM_FUNC_), path, ch, alt));

	if(path == P){
		if(ch == Y){
            if(gInterlaced)//interlaced mode
            {    
			    if(0 == alt%2){
			        IM_pBUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[alt], buffer);
			    }
			else{
				    gCamif->path[P].buffY[alt].vir_addr = buffer->vir_addr + gCamif->path[P].width*gCamif->path[P].height/2;
				    gCamif->path[P].buffY[alt].phy_addr = buffer->phy_addr + gCamif->path[P].width*gCamif->path[P].height/2;
				    gCamif->path[P].buffY[alt].size = buffer->size;
				    gCamif->path[P].buffY[alt].flag = buffer->flag;
			    }
            }
            else{			
			    IM_pBUFFER_COPYTO_BUFFER(gCamif->path[P].buffY[alt], buffer);
            } 
			    regIndex = CICH4_ST_ADR + (alt << 1);
		}else if(ch == U){
            if(gInterlaced)//interlaced mode
            {    
			    if(0 == alt%2){
			        IM_pBUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[alt], buffer);
			    }
			    else{
				    gCamif->path[P].buffCb[alt].vir_addr = buffer->vir_addr + gCamif->path[P].width*gCamif->path[P].height/4;
				    gCamif->path[P].buffCb[alt].phy_addr = buffer->phy_addr + gCamif->path[P].width*gCamif->path[P].height/4;
			    	gCamif->path[P].buffCb[alt].size = buffer->size;
				    gCamif->path[P].buffCb[alt].flag = buffer->flag;
			    }
            }
            else{    
			    IM_pBUFFER_COPYTO_BUFFER(gCamif->path[P].buffCb[alt], buffer);
            } 
			    regIndex = CICH3_ST_ADR + (alt << 1);
		}
	}
	else if(path == C){
		if(ch == Y){
			IM_pBUFFER_COPYTO_BUFFER(gCamif->path[C].buffY[alt], buffer);
			regIndex = CICH2_ST_ADR + (alt << 1);
		}else if(ch == U){
			IM_pBUFFER_COPYTO_BUFFER(gCamif->path[C].buffCb[alt], buffer);
			regIndex = CICH1_ST_ADR + (alt << 1);
		}else if(ch == V){
			IM_pBUFFER_COPYTO_BUFFER(gCamif->path[C].buffCr[alt], buffer);
			regIndex = CICH0_ST_ADR + (alt << 1);
		}
	}
    if(gInterlaced)//interlaced mode
    {    
        if(path == P)
	    {
		    if(ch == Y){
			    gCamif->regs[regIndex].value = gCamif->path[P].buffY[alt].phy_addr;
		    }
		    else{
			    gCamif->regs[regIndex].value = gCamif->path[P].buffCb[alt].phy_addr;
		    }
		    IM_INFOMSG((IM_STR("WRITE_REG(0x%x, 0x%x)"), gCamif->regs[regIndex].offset, gCamif->regs[regIndex].value));
		    IM_JIF(camifpwl_write_reg(gCamif->regs[regIndex].offset, gCamif->regs[regIndex].value));
	    }
    }
    else
    {    
	    gCamif->regs[regIndex].value = buffer->phy_addr;
	    IM_JIF(camifpwl_write_reg(gCamif->regs[regIndex].offset, gCamif->regs[regIndex].value));
    }   

	return IM_RET_OK;
Fail:
	return IM_RET_FAILED;
}

IM_INLINE static IM_RET getDmaBuffer(IM_UINT32 path, IM_UINT32 ch, IM_UINT32 alt, IM_Buffer *buffer)
{
	IM_INFOMSG((IM_STR("%s(path=%d, ch=%d, alt=%d)"), IM_STR(_IM_FUNC_), path, ch, alt));

	if(path == P){
		if(ch == Y){
			IM_BUFFER_COPYTO_pBUFFER(buffer, gCamif->path[P].buffY[alt]);
		}else if(ch == U){
			IM_BUFFER_COPYTO_pBUFFER(buffer, gCamif->path[P].buffCb[alt]);
		}
	}
	else if(path == C){
		if(ch == Y){
			IM_BUFFER_COPYTO_pBUFFER(buffer, gCamif->path[C].buffY[alt]);
		}else if(ch == U){
			IM_BUFFER_COPYTO_pBUFFER(buffer, gCamif->path[C].buffCb[alt]);
		}else if(ch == V){
			IM_BUFFER_COPYTO_pBUFFER(buffer, gCamif->path[C].buffCr[alt]);
		}
	}

	return IM_RET_OK;
}



