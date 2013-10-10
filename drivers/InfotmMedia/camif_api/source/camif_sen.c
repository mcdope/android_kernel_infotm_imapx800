/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camif_sen.c
--
--  Description :
--		
--
--	Author:
--  	Jimmy Shu   <jimm.shu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	Jimmy@2012/10/18: first commit.
-- v1.0.2	Jimmy@2013/01/29: remove three kinds of effects(solarize,posterize,aqua) 
			because x15 not support.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <camif_api.h>
#include <camif_pwl.h>
#include <camif_lib.h>
#include <camsen_lib.h>
#include <camif_sen.h>


#define DBGINFO	    0	
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMIFSEN_I:"
#define WARNHEAD	"CAMIFSEN_W:"
#define ERRHEAD		"CAMIFSEN_E:"
#define TIPHEAD		"CAMIFSEN_T:"

#define TIME_STATS 0

//
// parameter check.
//
#define PCHECK_NULL(p)	\
	if(p == IM_NULL){	\
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

#define PCHECK_GT(a, b)	\
	if((a) > (b)){	\
		IM_ERRMSG((IM_STR("Invalid parameter")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

#define PCHECK_GE(a, b)	\
	if((a) >= (b)){	\
		IM_ERRMSG((IM_STR("Invalid parameter")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

#define PCHECK_LT(a, b)		\
	if((a) < (b)){	\
		IM_ERRMSG((IM_STR("Invalid parameter")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

#define PCHECK_LE(a, b)		\
	if((a) <= (b)){	\
		IM_ERRMSG((IM_STR("Invalid parameter")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

#define PCHECK_EQ(a, b)		\
	if((a) == (b)){	\
		IM_ERRMSG((IM_STR("Invalid parameter")));	\
		return IM_RET_INVALID_PARAMETER;	\
	}

static camif_sen_context_t *gCamifSen = IM_NULL;

typedef struct{
	camif_func_frame_done_t	frame_done;	//callbakc.
}global_struct_t;

static global_struct_t gl; /*= {
	.frame_done = IM_NULL,	//callback.
}global_struct_t;*/

IM_RET camif_save_ready_frame(void);

//=================================================================
void camif_calc_frame_size(IM_UINT32 res, IM_UINT32 fmt, IM_UINT32 *w, IM_UINT32 *h)
{
	IM_INT32 size = 0;
	IM_INFOMSG((IM_STR("%s(res=0x%x, fmt=0x%x)"), IM_STR(_IM_FUNC_), res, fmt));
	switch(res){
	case CAM_RES_QQCIF:
		size = 88*72;
		*w = 88;
		*h = 72;
		break;
	case CAM_RES_SUB_QCIF:
		size = 128*96;
		*w = 128;
		*h = 96;
		break;
	case CAM_RES_QQVGA:
		size = 160*120;
		*w = 160;
		*h = 120;
		break;
	case CAM_RES_QCIF:
		size = 176*144;
		*w = 176;
		*h = 144;
		break;
	case CAM_RES_QVGA:
		size = 320*240;
		*w = 320;
		*h = 240;
		break;
	case CAM_RES_CIF:
		size = 352*288;
		*w = 352;
		*h = 288;
		break;
	case CAM_RES_VGA:
		size = 640*480;
		*w = 640;
		*h = 480;
		break;
	case CAM_RES_480P:
		size = 720*480;
		*w = 720;
		*h = 480;
		break;
	case CAM_RES_PAL:
		size = 768*576;
		*w = 768;
		*h = 576;
		break;
	case CAM_RES_SVGA:
		size = 800*600;
		*w = 800;
		*h = 600;
		break;
	case CAM_RES_XVGA:
		size = 1024*768;
		*w = 1024;
		*h = 768;
		break;
	case CAM_RES_720P:
		size = 1280*720;
		*w = 1280;
		*h = 720;
		break;
	case CAM_RES_SXGA:
		size = 1280*1024;
		*w = 1280;
		*h = 1024;
		break;
	case CAM_RES_SXGAPlus:
		size = 1400*1050;
		*w = 1400;
		*h = 1050;
		break;
	case CAM_RES_UXGA:
		size = 1600*1200;
		*w = 1600;
		*h = 1200;
		break;
	case CAM_RES_1080P:
		size = 1920*1080;
		*w = 1920;
		*h = 1080;
		break;
	case CAM_RES_320W:
		size = 2048*1536;
		*w = 2048;
		*h = 1536;
		break;
	case CAM_RES_WQXGA:
		size = 2560*1600;
		*w = 2560;
		*h = 1600;
		break;
	case CAM_RES_500W:
		size = 2592*1936;
		*w = 2592;
		*h = 1936;
		break;
	case CAM_RES_QUXGA:
		size = 3200*2400;
		*w = 3200;
		*h = 2400;
		break;
	case CAM_RES_WQXGA_U:
		size = 3840*2400;
		*w = 3840;
		*h = 2400;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
		IM_ASSERT(IM_FALSE);
		break;
	}

	switch(fmt){
	case CAM_PIXFMT_YUV420P:
		gCamifSen->imageSize = (size*3)>>1;
		gCamifSen->ySize = size;
		gCamifSen->uSize = size>>2;
		gCamifSen->vSize = gCamifSen->uSize;
		break;
	case CAM_PIXFMT_YUV420SP:
		gCamifSen->imageSize = (size*3)>>1;
		gCamifSen->ySize = size;
		gCamifSen->uSize = size>>1;
		gCamifSen->vSize = 0;
		break;
	case CAM_PIXFMT_YUV422P:
		gCamifSen->imageSize = size<<1;
		gCamifSen->ySize = size;
		gCamifSen->uSize = size>>1;
		gCamifSen->vSize = gCamifSen->uSize;
		break;
	case CAM_PIXFMT_YUV422SP:
		gCamifSen->imageSize = size<<1;
		gCamifSen->ySize = size;
		gCamifSen->uSize = size;
		gCamifSen->vSize = 0;
		break;
	case CAM_PIXFMT_YUV444P:
		gCamifSen->imageSize = size*3;
		gCamifSen->ySize = size;
		gCamifSen->uSize = size;
		gCamifSen->vSize = size;
		break;
	case CAM_PIXFMT_YUV422I:
		gCamifSen->imageSize = size<<1;
		gCamifSen->ySize = size<<1;
		gCamifSen->uSize = 0;
		gCamifSen->vSize = 0;
		break;
	case CAM_PIXFMT_16BPP_RGB565:
		gCamifSen->imageSize = size<<1;
		gCamifSen->ySize = size<<1;
		gCamifSen->uSize = 0;
		gCamifSen->vSize = 0;
		break;
	case CAM_PIXFMT_32BPP_RGB0888:
	case CAM_PIXFMT_32BPP_RGB8880:
	case CAM_PIXFMT_32BPP_BGR0888:
	case CAM_PIXFMT_32BPP_BGR8880:
		gCamifSen->imageSize = size<<2;
		gCamifSen->ySize = size<<2;
		gCamifSen->uSize = 0;
		gCamifSen->vSize = 0;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(fmt=0x%x)"), IM_STR(_IM_FUNC_), fmt));
		IM_ASSERT(IM_FALSE);
		break;
	}

	return;
}

void camif_get_frame_size_from_res(IM_UINT32 res, IM_UINT32 *w, IM_UINT32 *h)
{
	IM_INFOMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
	switch(res){
	case CAM_RES_QQCIF:
		*w = 88;
		*h = 72;
		break;
	case CAM_RES_SUB_QCIF:
		*w = 128;
		*h = 96;
		break;
	case CAM_RES_QQVGA:
		*w = 160;
		*h = 120;
		break;
	case CAM_RES_QCIF:
		*w = 176;
		*h = 144;
		break;
	case CAM_RES_QVGA:
		*w = 320;
		*h = 240;
		break;
	case CAM_RES_CIF:
		*w = 352;
		*h = 288;
		break;
	case CAM_RES_VGA:
		*w = 640;
		*h = 480;
		break;
	case CAM_RES_480P:
		*w = 720;
		*h = 480;
		break;
	case CAM_RES_PAL:
		*w = 768;
		*h = 576;
		break;
	case CAM_RES_SVGA:
		*w = 800;
		*h = 600;
		break;
	case CAM_RES_XVGA:
		*w = 1024;
		*h = 768;
		break;
	case CAM_RES_720P:
		*w = 1280;
		*h = 720;
		break;
	case CAM_RES_SXGA:
		*w = 1280;
		*h = 1024;
		break;
	case CAM_RES_SXGAPlus:
		*w = 1400;
		*h = 1050;
		break;
	case CAM_RES_UXGA:
		*w = 1600;
		*h = 1200;
		break;
	case CAM_RES_1080P:
		*w = 1920;
		*h = 1080;
		break;
	case CAM_RES_320W:
		*w = 2048;
		*h = 1536;
		break;
	case CAM_RES_WQXGA:
		*w = 2560;
		*h = 1600;
		break;
	case CAM_RES_500W:
		*w = 2592;
		*h = 1936;
		break;
	case CAM_RES_QUXGA:
		*w = 3200;
		*h = 2400;
		break;
	case CAM_RES_WQXGA_U:
		*w = 3840;
		*h = 2400;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
		IM_ASSERT(IM_FALSE);
		break;
	}

	return;
}

IM_IMAGE_TYPE camifFmt2ImType(IM_UINT32 fmt)
{
	IM_IMAGE_TYPE type = IM_IMAGE_RGB0888;
	switch(fmt){
	case CAM_PIXFMT_YUV420P:
		type = IM_IMAGE_YUV420P;
		break;
	case CAM_PIXFMT_YUV420SP:
		type = IM_IMAGE_YUV420SP;
		break;
	case CAM_PIXFMT_YUV422P:
		type = IM_IMAGE_YUV422P;
		break;
	case CAM_PIXFMT_YUV422SP:
		type = IM_IMAGE_YUV422SP;
		break;
	case CAM_PIXFMT_YUV444P:
		type = IM_IMAGE_YUV444P;
		break;
	case CAM_PIXFMT_YUV422I:
		type = IM_IMAGE_YUV422I;
		break;
	case CAM_PIXFMT_16BPP_RGB565:
		type = IM_IMAGE_RGB565;
		break;
	case CAM_PIXFMT_32BPP_RGB0888:
		type = IM_IMAGE_RGB0888;
		break;
	case CAM_PIXFMT_32BPP_RGB8880:
		type = IM_IMAGE_RGB8880;
		break;
	case CAM_PIXFMT_32BPP_BGR0888:
		type = IM_IMAGE_BGR0888;
		break;
	case CAM_PIXFMT_32BPP_BGR8880:
		type = IM_IMAGE_BGR8880;
		break;
	default:
		IM_ASSERT(IM_FALSE);
		break;
	}

	return type;
}

IM_RET camif_sensor_set_property(IM_INT32 key, void *value)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = camsen_set_property(gCamifSen->camsenHandle, key, value);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camsen_set_property() failed!")));
        return IM_RET_FAILED;
    }

	return ret;
}

static inline IM_UINT32 index2proId(IM_UINT32 index)
{
	IM_UINT32 i = 0;
	while((index & (1<<i)) == 0x0)
	{
		i++;
	}

	return i;
}

static void camif_set_fpsBits(cam_preview_config_t *precfg,IM_UINT32 resBits)
{
	IM_UINT32 i = 0;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    for(i = 0;i<CAM_RES_ENUM_MAX;i++)
    {
        if((resBits>>i)&0x01){
            precfg->attri[i].fpsBits = CAM_FPS_30;
        }
        else{
            precfg->attri[i].fpsBits = 0;
        }  
    }    

}

IM_RET camifsen_set_precfg(IM_UINT32 width, IM_UINT32 height,IM_UINT32 fmt,IM_BOOL interlaced)
{
	IM_INT32 i = 0;
	IM_RET ret = IM_RET_OK;
	CAMIF_PATH_CONFIG precfg;
	CAMIF_SOURCE_CONFIG srccfg;
	PCHECK_NULL(gCamifSen);
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s(widht =%d, height=%d )"), IM_STR(_IM_FUNC_), width,height));

	camifpwl_memset((void *)&precfg, 0, sizeof(CAMIF_PATH_CONFIG));
	camifpwl_memset((void *)&srccfg, 0, sizeof(CAMIF_SOURCE_CONFIG));
	
    precfg.halfWordSwap = IM_FALSE;
	precfg.byteSwap = IM_FALSE;
	precfg.width = width;
	precfg.height = height;

    IM_INFOMSG((IM_STR("set_precfg:precfg.widht:%d precfg.height:%d\n"),precfg.width,precfg.height));
    //set source
    
	srccfg.xSize = width;
	srccfg.ySize = height;
	srccfg.xOffset = 0;
	srccfg.yOffset = 0;
	
    srccfg.ituMode = CAMIF_ITU_MODE_601;
    srccfg.scanMode = CAMIF_SCAN_MODE_PROGRESSIVE;
    srccfg.yuvOrder = CAMIF_INPUT_YUV_ORDER_YCbYCr;
    srccfg.invPinPclk = CAMIF_POLARITY_CAMPCLK_NORMAL;
    srccfg.invPinVsync = CAMIF_POLARITY_CAMVSYNC_NORMAL;
    srccfg.invPinHref = CAMIF_POLARITY_CAMHREF_NORMAL;
    srccfg.invPinField = CAMIF_POLARITY_CAMFIELD_NORMAL;    
	
    ret = camiflib_set_source(&srccfg);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camiflib_set_source() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}

    //set preview
	precfg.intrMode = CAMIF_INTR_SOURCE_FRAME_END;
	if((BUFFER_NUM-1) == 1){
		precfg.ppMode = CAMIF_PPMODE_DISABLE;
	}else if((BUFFER_NUM-1) == 2){
		precfg.ppMode = CAMIF_PPMODE_2_BUFFER;
	}else{
		IM_ASSERT((BUFFER_NUM-1) == 4);
		precfg.ppMode = CAMIF_PPMODE_4_BUFFER;
	}

	if(fmt == CAM_PIXFMT_16BPP_RGB565){
		precfg.sampleMode = CAMIF_SAMPLE_MODE_RGB16BPP_565;
	}else if(fmt == CAM_PIXFMT_32BPP_RGB0888){
		precfg.sampleMode = CAMIF_SAMPLE_MODE_RGB24BPP_0888;
	}else if(fmt == CAM_PIXFMT_32BPP_RGB8880){
		precfg.sampleMode = CAMIF_SAMPLE_MODE_RGB24BPP_8880;
	}else if(fmt == CAM_PIXFMT_YUV420SP){
		precfg.sampleMode = CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR;
	}else if(fmt == CAM_PIXFMT_YUV422SP){
		precfg.sampleMode = CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR;
	}

	if(gCamifSen->ySize != 0){
		for(i=0; i<BUFFER_NUM-1; i++){
			precfg.buffY[i].vir_addr = gCamifSen->buffers[i].vir_addr;
			precfg.buffY[i].phy_addr = gCamifSen->buffers[i].phy_addr;
			precfg.buffY[i].size = gCamifSen->ySize;
			precfg.buffY[i].flag = gCamifSen->buffers[i].flag;
		}
	}
	if(gCamifSen->uSize != 0){
		for(i=0; i<BUFFER_NUM-1; i++){
			precfg.buffCb[i].vir_addr = (void *)((IM_INT32)gCamifSen->buffers[i].vir_addr + gCamifSen->ySize);
			precfg.buffCb[i].phy_addr = gCamifSen->buffers[i].phy_addr + gCamifSen->ySize;
			precfg.buffCb[i].size = gCamifSen->uSize;
			precfg.buffCb[i].flag = gCamifSen->buffers[i].flag;
		}
	}

	precfg.matrix.mode = CAMIF_MATRIX_MODE_DEFAULT;
    //check if it is interlaced mode
    if(interlaced){
        precfg.interlaced = 1;
    }
    else{
        precfg.interlaced = 0;
    }
    IM_INFOMSG((IM_STR("interlaced:%d\n"),interlaced));

	ret = camiflib_set_preview(&precfg);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camiflib_set_preview() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
    return ret;
}    

IM_RET camifsen_set_config(IM_UINT32 res, IM_UINT32 fmt, IM_UINT32 fps,IM_BOOL interlaced)
{
	IM_RET ret = IM_RET_OK;
	camsen_out_mode_t outMode;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s(res=0x%x, fmt=0x%x, fps=0x%x)"), IM_STR(_IM_FUNC_), res, fmt, fps));
	
    outMode.res = res;
	outMode.fmt = fmt;
	outMode.fps = fps;
	ret = camsen_set_out_mode(gCamifSen->camsenHandle, &outMode);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camsen_set_out_mode() failed!")));
		return IM_RET_FAILED;
	}

	//update  current preview config setting.
	gCamifSen->preCfg.res = res;
	gCamifSen->preCfg.fmt = fmt;
	gCamifSen->preCfg.fps = fps;

	return ret;
}

IM_RET camifsen_get_config(cam_preview_config_t *preCfg)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
    PCHECK_NULL(gCamifSen);

	camifpwl_memcpy((void *)preCfg, (void *)&gCamifSen->preCfg, sizeof(cam_preview_config_t));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camifsen_set_property(IM_INT32 key, void *value, IM_INT32 size)
{
	IM_UINT32 proVal = 100;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s(key=0x%x)"), IM_STR(_IM_FUNC_), key));

	memcpy((void *)&proVal, value, sizeof(IM_UINT32));

	if(((key == CAM_KEY_RW_ZOOM) && (proVal == gCamifSen->zmVal)) ||
		((key == CAM_KEY_RW_WB_MODE) && (proVal == gCamifSen->wbMode)) ||
		((key == CAM_KEY_RW_SPECIAL_EFFECT) && (proVal == gCamifSen->spEffect)) ||
		((key == CAM_KEY_RW_SCENE_MODE) && (proVal == gCamifSen->scMode)))
	{
		IM_INFOMSG((IM_STR("This property is already setting value!")));
		return IM_RET_OK;
	}
	
	IM_INFOMSG((IM_STR("%s(key=0x%x, different property=0x%x)"), IM_STR(_IM_FUNC_), key, proVal));
	

	//set sensor property
	ret = camif_sensor_set_property(key, value);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camif_sensor_set_property() failed!")));
		return IM_RET_FAILED;
	}


	switch (key)
	{
		case CAM_KEY_RW_WB_MODE:
			memcpy((void*)&gCamifSen->wbMode, value, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy((void*)&gCamifSen->spEffect, value, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy((void*)&gCamifSen->scMode, value, sizeof(IM_UINT32));
			break;
		default:
			break;
	}
	
	return ret;
}

IM_RET camifsen_get_property(IM_INT32 key, void *value, IM_INT32 size)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 proVal = 0;

	IM_INFOMSG((IM_STR("%s(key=0x%x)"), IM_STR(_IM_FUNC_), key));

    ret = camsen_get_property(gCamifSen->camsenHandle, key, value);
	
/*	
	if(CAM_KEY_R_SUPPORT_SPECIAL_EFFECT == key){
		camifpwl_memcpy((void *)&proVal, (void *)value, sizeof(IM_INT32));
		proVal &=~(CAM_SPECIAL_EFFECT_POSTERIZE | CAM_SPECIAL_EFFECT_AQUA | CAM_SPECIAL_EFFECT_SOLARIZE);
		
		camifpwl_memcpy((void *)value, (void *)&proVal, sizeof(IM_INT32));
	}
*/	
	return ret;
}

IM_RET camifsen_assign_buffer(IM_Buffer *buffer)
{

	IM_INFOMSG((IM_STR("%s(buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x)"), 
		IM_STR(_IM_FUNC_), (IM_INT32)buffer->vir_addr, buffer->phy_addr, buffer->size, buffer->flag));
	PCHECK_NULL(gCamifSen);
	PCHECK_NULL(buffer);
	PCHECK_NULL(buffer->vir_addr);
	PCHECK_EQ(buffer->phy_addr, 0);

	if(gCamifSen->enabled == IM_TRUE){
		IM_ERRMSG((IM_STR("camif is started, can not assign buffer")));
		return IM_RET_FAILED;
	}

	if(gCamifSen->bufferNum >= BUFFER_NUM){
		IM_WARNMSG((IM_STR("camif buffer is enough, don't need anymore")));
		return IM_RET_FALSE;
	}

	camifpwl_memcpy((void *)&gCamifSen->buffers[gCamifSen->bufferNum], (void *)buffer, sizeof(IM_Buffer));
	gCamifSen->bufferNum++;

	return IM_RET_OK;
}

IM_RET camifsen_release_buffer(IM_Buffer *buffer)
{
	IM_INFOMSG((IM_STR("%s(buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x)"), 
		IM_STR(_IM_FUNC_), (IM_INT32)buffer->vir_addr, buffer->phy_addr, buffer->size, buffer->flag));
	PCHECK_NULL(gCamifSen);
	PCHECK_NULL(buffer);
	PCHECK_NULL(buffer->vir_addr);
	PCHECK_EQ(buffer->phy_addr, 0);
	
	camifpwl_spinlock(gCamifSen->spinLock);	
	if(gCamifSen->bufferNum >= BUFFER_NUM){
		camifpwl_spinunlock(gCamifSen->spinLock);	
		IM_WARNMSG((IM_STR("camif buffer is enough, don't need anymore")));
		return IM_RET_FALSE;
	}

	if(gCamifSen->enabled == IM_TRUE){
		PCHECK_LT(buffer->size, gCamifSen->imageSize);
	}

	camifpwl_memcpy((void *)&gCamifSen->buffers[BUFFER_NUM-1], (void *)buffer, sizeof(IM_Buffer));
	gCamifSen->bufferNum++;
	camifpwl_spinunlock(gCamifSen->spinLock);	

	return IM_RET_OK;

}

IM_RET camifsen_start(void)
{
	IM_INT32 i;
	IM_RET ret = IM_RET_OK;
	IM_UINT32 w = 0, h = 0;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gCamifSen);

	if(gCamifSen->enabled == IM_TRUE){
		IM_WARNMSG((IM_STR("camif has been started")));
		return IM_RET_OK;
	}

	if(gCamifSen->bufferNum < BUFFER_NUM){
		IM_ERRMSG((IM_STR("camif buffer is not enough")));
		return IM_RET_FAILED;
	}

    //cal the width and height and imageSize    
    camif_calc_frame_size(gCamifSen->preCfg.res,gCamifSen->preCfg.fmt, &w, &h);
	IM_INFOMSG((IM_STR("%s(outWidth=%d, outHeight=%d)"), IM_STR(_IM_FUNC_), w, h));

	for(i=0; i<BUFFER_NUM; i++){
		if(gCamifSen->buffers[i].size < gCamifSen->imageSize){
			IM_ERRMSG((IM_STR("camif buffer[%d] size(%d, imageSize=%d) is invalid"), 
				i, gCamifSen->buffers[i].size, gCamifSen->imageSize));
			IM_ERRMSG((IM_STR("camif res= 0x%x  fmt=0x%x"),gCamifSen->preCfg.res,gCamifSen->preCfg.fmt)); 
			return IM_RET_FAILED;
		}
	}

    //camif set preview config
    ret = camifsen_set_precfg(w,h,gCamifSen->preCfg.fmt,IM_FALSE);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_init, camifsen_set_precfg() failed!")));
		return IM_RET_FAILED;
	}
    
    //start camif
    ret = camiflib_start_preview(IM_FALSE);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camiflib_start() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
    
    //start sensor
	ret = camsen_start(gCamifSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camsen_start, camsen_start() failed!")));
		return IM_RET_FAILED;
	}

	gCamifSen->hasRdyFrame = IM_FALSE;
	gCamifSen->enabled = IM_TRUE;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET camifsen_stop(void)
{
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gCamifSen);

	if(gCamifSen->enabled == IM_FALSE){
		IM_WARNMSG((IM_STR("camif has not been started")));
		return IM_RET_OK;
	}

	ret = camiflib_stop_preview();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camiflib_stop() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
	ret = camsen_stop(gCamifSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_stop, camsen_stop() failed!")));
		return IM_RET_FAILED;
	}
	gCamifSen->bufferNum = 0;
	gCamifSen->hasRdyFrame = IM_FALSE;
	gCamifSen->enabled = IM_FALSE;

	return ret;
}

static IM_UINT32 frame_num = 0;

static IM_INT64 camifsen_get_system_time_us(){
	struct timeval tv;
	do_gettimeofday(&tv);
	return (IM_INT64)((IM_INT64)tv.tv_usec  + (IM_INT64)(((IM_INT64)tv.tv_sec)* 1000000LL));
}

//interrupt callback function
IM_RET camifsen_irq_handle(void)
{
	IM_RET ret = IM_RET_OK;
	camif_strm_frame_t frm;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gCamifSen);

	if(gCamifSen->enabled == IM_FALSE){
		IM_ERRMSG((IM_STR("camif not enabled")));
		return IM_RET_FAILED;
	}

	camifpwl_spinlock(gCamifSen->spinLock);	
	frame_num++;
	if((gCamifSen->bufferNum < BUFFER_NUM) && (gCamifSen->hasRdyFrame == IM_FALSE)){
		IM_ERRMSG((IM_STR("camif has no chance to save captured frame, frame_num=%d"), frame_num));
		camifpwl_spinunlock(gCamifSen->spinLock);	
		return IM_RET_FAILED;
	}else{
		ret =camif_save_ready_frame();
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("@camifsen_irq_handle, camif_save_ready_frame() failed!")));
			camifpwl_spinunlock(gCamifSen->spinLock);	
			return IM_RET_FAILED;
		}
	}	

	if(gCamifSen->hasRdyFrame == IM_FALSE){
		IM_ERRMSG((IM_STR("camif has no captured frame")));
		camifpwl_spinunlock(gCamifSen->spinLock);	
		return IM_RET_FAILED;
	}

	frm.buffer.vir_addr = gCamifSen->rdyBuffer.vir_addr;
	frm.buffer.phy_addr = gCamifSen->rdyBuffer.phy_addr;
	frm.buffer.size = gCamifSen->rdyBuffer.size;
	frm.buffer.flag = gCamifSen->rdyBuffer.flag;
	frm.ts = camifsen_get_system_time_us();
	frm.id = frame_num;
	IM_INFOMSG((IM_STR("camif saved frame num=%d"), frame_num));
	IM_INFOMSG((IM_STR("camif saved frame timestamp=%lld"), frm.ts));
	IM_INFOMSG((IM_STR("camif get buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x"), 
		(IM_INT32)frm.buffer.vir_addr, frm.buffer.phy_addr, frm.buffer.size, frm.buffer.flag));

	camifpwl_spinunlock(gCamifSen->spinLock);	
	//call frame_done
	gl.frame_done(&frm);

	gCamifSen->hasRdyFrame = IM_FALSE;
	return ret;
}

IM_RET camif_save_ready_frame(void)
{
	IM_RET ret = IM_RET_OK;
	IM_Buffer readyBuffer[3] ={{0},{0},{0}}, replaceBuffer[3] = {{0},{0},{0}};

	if(gCamifSen->bufferNum == BUFFER_NUM){	// there has idle buffer.
		IM_INFOMSG((IM_STR("camif use idle buffer to replace out ready frame")));
		if(gCamifSen->ySize != 0){
			replaceBuffer[0].vir_addr = gCamifSen->buffers[BUFFER_NUM-1].vir_addr;
			replaceBuffer[0].phy_addr = gCamifSen->buffers[BUFFER_NUM-1].phy_addr;
			replaceBuffer[0].size = gCamifSen->ySize;
			replaceBuffer[0].flag = gCamifSen->buffers[BUFFER_NUM-1].flag;
		}
		if(gCamifSen->uSize != 0){
			replaceBuffer[1].vir_addr = (void *)((IM_INT32)gCamifSen->buffers[BUFFER_NUM-1].vir_addr + gCamifSen->ySize);
			replaceBuffer[1].phy_addr = gCamifSen->buffers[BUFFER_NUM-1].phy_addr + gCamifSen->ySize;
			replaceBuffer[1].size = gCamifSen->uSize;
			replaceBuffer[1].flag = gCamifSen->buffers[BUFFER_NUM-1].flag;
		}
		if(gCamifSen->vSize != 0){
			replaceBuffer[2].vir_addr = (void *)((IM_INT32)gCamifSen->buffers[BUFFER_NUM-1].vir_addr + gCamifSen->ySize + gCamifSen->uSize);
			replaceBuffer[2].phy_addr = gCamifSen->buffers[BUFFER_NUM-1].phy_addr + gCamifSen->ySize + gCamifSen->uSize;
			replaceBuffer[2].size = gCamifSen->vSize;
			replaceBuffer[2].flag = gCamifSen->buffers[BUFFER_NUM-1].flag;
		}

		gCamifSen->bufferNum--;
	}else{	// there has ready buffer.
		IM_ASSERT(gCamifSen->hasRdyFrame == IM_TRUE);
		IM_INFOMSG((IM_STR("camif use ready buffer to replace out ready frame")));
		if(gCamifSen->ySize != 0){
			replaceBuffer[0].vir_addr = gCamifSen->rdyBuffer.vir_addr;
			replaceBuffer[0].phy_addr = gCamifSen->rdyBuffer.phy_addr;
			replaceBuffer[0].size = gCamifSen->ySize;
			replaceBuffer[0].flag = gCamifSen->rdyBuffer.flag;
		}
		if(gCamifSen->uSize != 0){
			replaceBuffer[1].vir_addr = (void *)((IM_INT32)gCamifSen->rdyBuffer.vir_addr + gCamifSen->ySize);
			replaceBuffer[1].phy_addr = gCamifSen->rdyBuffer.phy_addr + gCamifSen->ySize;
			replaceBuffer[1].size = gCamifSen->uSize;
			replaceBuffer[1].flag = gCamifSen->rdyBuffer.flag;
		}
		if(gCamifSen->vSize != 0){
			replaceBuffer[2].vir_addr = (void *)((IM_INT32)gCamifSen->rdyBuffer.vir_addr + gCamifSen->ySize + gCamifSen->uSize);
			replaceBuffer[2].phy_addr = gCamifSen->rdyBuffer.phy_addr + gCamifSen->ySize + gCamifSen->uSize;
			replaceBuffer[2].size = gCamifSen->vSize;
			replaceBuffer[2].flag = gCamifSen->rdyBuffer.flag;
		}

		gCamifSen->hasRdyFrame = IM_FALSE;
	}

	ret = camiflib_get_ready_buffer(CAMIF_PATH_PREVIEW,readyBuffer, replaceBuffer);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camiflib_get_ready_buffer() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
	gCamifSen->rdyBuffer.vir_addr = readyBuffer[0].vir_addr;
	gCamifSen->rdyBuffer.phy_addr = readyBuffer[0].phy_addr;
	gCamifSen->rdyBuffer.size = readyBuffer[0].size + readyBuffer[1].size + readyBuffer[2].size;
	gCamifSen->rdyBuffer.flag = readyBuffer[0].flag;
	IM_INFOMSG((IM_STR("save frame%d: vir=0x%x, phy=0x%x, size=%d, flag=0x%x"), 
				frame_num, (IM_INT32)gCamifSen->rdyBuffer.vir_addr, gCamifSen->rdyBuffer.phy_addr, 
				gCamifSen->rdyBuffer.size, gCamifSen->rdyBuffer.flag));

	gCamifSen->hasRdyFrame = IM_TRUE;
	return ret;

}
//=================================================================


//=====================camera control exec=========================
IM_RET camifsen_auto_focus(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}
//=================================================================



//=====================camapi interface=============================
IM_RET camifsen_init(camif_camdev_config_t *devCfg, camif_workmodel_config_t *wmCfg)
{
	IM_INT32 index, i ;
	IM_INT32 irq = 0;
	IM_RET ret = IM_RET_OK;
	camsen_caps_t caps;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gCamifSen != IM_NULL) return IM_RET_OK;

	gCamifSen = (camif_sen_context_t *)camifpwl_malloc(sizeof(camif_sen_context_t));
	if(gCamifSen == IM_NULL){
		IM_ERRMSG((IM_STR("camifpwl_malloc() failed")));
		goto Fail;
	}
	camifpwl_memset((void *)gCamifSen, 0, sizeof(camif_sen_context_t));

	//init digital zoom value
	gCamifSen->zmVal = 100;							//default zoom value(non zoom)	
	gCamifSen->wbMode = CAM_WB_MODE_AUTO;				//default white balance mode
	gCamifSen->spEffect = CAM_SPECIAL_EFFECT_NONE;	//default special effect
	gCamifSen->scMode = CAM_SCENE_MODE_AUTO;			//default scene mode

	if(camifpwl_spinlock_init(&gCamifSen->spinLock) != IM_RET_OK){
		IM_ERRMSG((IM_STR("camifpwl_spinlock_init() failed")));
		goto Fail;
	}
	
	gl.frame_done = wmCfg->smcfg.frame_done; 
	//init control mode config according to sensor module caps 
	{
		//init code box
		//wmCfg->cmcfg.ctCnt = caps.ctCnt;
		wmCfg->cmcfg.ctCnt = 0;
		for(index=0; index < wmCfg->cmcfg.ctCnt; index++){
			wmCfg->cmcfg.ctCodeBoxSize[index] = 0/*caps.xx*/;
			for(i=0; i < wmCfg->cmcfg.ctCodeBoxSize[index]; i++){
				wmCfg->cmcfg.ctCodeBox[index][i] = 0/*caps.xx*/;
			}

			//register control exec function such as auto foucs
			//wmCfg->cmcfg.control_exec[index] = camifsen_auto_focus;
		}
	}

	//camera sensor init
	ret = camsen_init(&gCamifSen->camsenHandle, devCfg->camsenName, devCfg->facing, IM_FALSE);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_init, camsen_init() failed!")));
		goto Fail;
	}

    ret = camsen_get_caps(gCamifSen->camsenHandle, &caps);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("@ispsen_init, camsen_get_caps() failed!")));
        goto Fail;
    }

    //set precfg
    camif_set_fpsBits(&gCamifSen->preCfg,caps.supportRes);

	gCamifSen->preCfg.resBits = caps.supportRes;
    
    gCamifSen->preCfg.fmtBits = CAM_PIXFMT_YUV420SP;
    
    gCamifSen->preCfg.res = caps.initRes; 
    gCamifSen->preCfg.fmt = CAM_PIXFMT_YUV420SP;
    gCamifSen->preCfg.fps = CAM_FPS_30;
	
    //camif lib init    
	ret = camiflib_init();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_init, camiflib_init() failed!")));
		goto Fail;
	}

	//register camif irq callback function, it must call after camifpwl_init(), that means call after camiflib_init()
	camifpwl_register_irq(irq, camifsen_irq_handle); 


	return ret;
Fail:
	//free resource
	if(gCamifSen != IM_NULL){
		if(gCamifSen->spinLock != IM_NULL){
			camifpwl_spinlock_deinit(gCamifSen->spinLock);	
			gCamifSen->spinLock = IM_NULL;
		}

		camifpwl_free((void *)gCamifSen);
		gCamifSen = IM_NULL;
	}
	return IM_RET_FAILED;		

}

IM_RET camifsen_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gCamifSen);

	//sensor deinit
	ret = camsen_deinit(gCamifSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_deinit, camsen_deinit() failed!")));
		return IM_RET_FAILED;		
	}
	//camiflib deinit
	ret = camiflib_deinit();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camifsen_deinit, camiflib_deinit() failed!")));
		return IM_RET_FAILED;		
	}

	if(gCamifSen->spinLock != IM_NULL){
		camifpwl_spinlock_deinit(gCamifSen->spinLock);	
		gCamifSen->spinLock = IM_NULL;
	}

	camifpwl_free((void *)gCamifSen);
	gCamifSen = IM_NULL;
	return ret;
}

IM_RET camifsen_ioctl(IM_INT32 cmd, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_Buffer *buff = IM_NULL;
	cam_preview_config_t *preCfg;
	camif_ioctl_ds_set_config *cfg;
	camif_ioctl_ds_key_property *pro;
	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));
	switch(cmd){
		case CAMIF_IOCTL_CMD_SET_CFG:
			cfg = (camif_ioctl_ds_set_config *)p;
			IM_INFOMSG((IM_STR("camifsen_api(res=0x%x, fps=0x%x)"), cfg->res, cfg->fps));
			ret = camifsen_set_config(cfg->res, cfg->fmt, cfg->fps,IM_FALSE);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_set_config() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_GET_CFG:
			preCfg = (cam_preview_config_t *)p;
			ret = camifsen_get_config(preCfg);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_get_config() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_SET_PRO:
			pro = (camif_ioctl_ds_key_property *)p;
			ret = camifsen_set_property(pro->key, (void*)pro->param, pro->size);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_set_property() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_GET_PRO:
			pro = (camif_ioctl_ds_key_property *)p;
			ret = camifsen_get_property(pro->key, (void*)pro->param, pro->size);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_get_property() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_ASSIGN_BUF:
			buff = (IM_Buffer *)p;
			ret = camifsen_assign_buffer(buff);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_assign_buffer() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_RELEASE_BUF:
			buff = (IM_Buffer *)p;
			ret = camifsen_release_buffer(buff);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_release_buffer() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_START:
			ret = camifsen_start();
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_start() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMIF_IOCTL_CMD_STOP:
			ret = camifsen_stop();
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@camifsen_ioctl, camifsen_stop() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		default:
			IM_ERRMSG((IM_STR("@camifsen_ioctl, ioctl cmd error: %d!"), cmd));
			return IM_RET_FAILED;		
	}
	return ret;
}

IM_RET camifsen_suspend(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET camifsen_resume(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

//register api to work model
IM_RET camif_workmodel_register_camapi(camif_camapi_interface_t *apis)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	if(apis == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameteri, apis is NULL")));
		return IM_RET_FAILED;		
	}
	apis->camapi_init = camifsen_init;
	apis->camapi_deinit = camifsen_deinit;
	apis->camapi_ioctl = camifsen_ioctl;
	apis->camapi_suspend = camifsen_suspend;
	apis->camapi_resume = camifsen_resume;

	return IM_RET_OK;
}

