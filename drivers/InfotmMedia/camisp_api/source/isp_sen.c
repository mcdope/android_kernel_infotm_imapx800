/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file ispsen_sen.c
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/04/18: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/20: support some special effect & scene mode with isp.
-- v1.0.4	arsor@2012/07/25: support general sensor which not need to config 
--			isp detatil parameters at file isp_sen_cfg.h.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <camisp_api.h>
#include <isp_pwl.h>
#include <isp_lib.h>
#include <camsen_lib.h>
#include <isp_sen_cfg.h>
#include <isp_sen.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"ISPSEN_I:"
#define WARNHEAD	"ISPSEN_W:"
#define ERRHEAD		"ISPSEN_E:"
#define TIPHEAD		"ISPSEN_T:"

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

static isp_sen_context_t *gIspSen = IM_NULL;

typedef struct{
	func_frame_done_t	frame_done;	//callbakc.
}global_struct_t;

static global_struct_t gl; /*= {
	.frame_done = IM_NULL,	//callback.
}global_struct_t;*/

IM_RET save_ready_frame(void);

//=================================================================
void calc_frame_size(IM_UINT32 res, IM_UINT32 fmt, IM_UINT32 *w, IM_UINT32 *h)
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
	case CAM_RES_130W:
		size = 1280*960;
		*w = 1280;
		*h = 960;
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
	case CAM_RES_12M:
		size = 4000*3000;
		*w = 4000;
		*h = 3000;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
		IM_ASSERT(IM_FALSE);
		break;
	}

	switch(fmt){
	case CAM_PIXFMT_YUV420P:
		gIspSen->imageSize = (size*3)>>1;
		gIspSen->ySize = size;
		gIspSen->uSize = size>>2;
		gIspSen->vSize = gIspSen->uSize;
		break;
	case CAM_PIXFMT_YUV420SP:
		gIspSen->imageSize = (size*3)>>1;
		gIspSen->ySize = size;
		gIspSen->uSize = size>>1;
		gIspSen->vSize = 0;
		break;
	case CAM_PIXFMT_YUV422P:
		gIspSen->imageSize = size<<1;
		gIspSen->ySize = size;
		gIspSen->uSize = size>>1;
		gIspSen->vSize = gIspSen->uSize;
		break;
	case CAM_PIXFMT_YUV422SP:
		gIspSen->imageSize = size<<1;
		gIspSen->ySize = size;
		gIspSen->uSize = size;
		gIspSen->vSize = 0;
		break;
	case CAM_PIXFMT_YUV444P:
		gIspSen->imageSize = size*3;
		gIspSen->ySize = size;
		gIspSen->uSize = size;
		gIspSen->vSize = size;
		break;
	case CAM_PIXFMT_YUV422I:
		gIspSen->imageSize = size<<1;
		gIspSen->ySize = size<<1;
		gIspSen->uSize = 0;
		gIspSen->vSize = 0;
		break;
	case CAM_PIXFMT_16BPP_RGB565:
		gIspSen->imageSize = size<<1;
		gIspSen->ySize = size<<1;
		gIspSen->uSize = 0;
		gIspSen->vSize = 0;
		break;
	case CAM_PIXFMT_32BPP_RGB0888:
	case CAM_PIXFMT_32BPP_RGB8880:
	case CAM_PIXFMT_32BPP_BGR0888:
	case CAM_PIXFMT_32BPP_BGR8880:
		gIspSen->imageSize = size<<2;
		gIspSen->ySize = size<<2;
		gIspSen->uSize = 0;
		gIspSen->vSize = 0;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(fmt=0x%x)"), IM_STR(_IM_FUNC_), fmt));
		IM_ASSERT(IM_FALSE);
		break;
	}

	return;
}

void get_frame_size_from_res(IM_UINT32 res, IM_UINT32 *w, IM_UINT32 *h)
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
	case CAM_RES_130W:
		*w = 1280;
		*h = 960;
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
	case CAM_RES_12M:
		*w = 4000;
		*h = 3000;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
		IM_ASSERT(IM_FALSE);
		break;
	}

	return;
}

IM_IMAGE_TYPE camFmt2ImType(IM_UINT32 fmt)
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

IM_RET sensor_set_property(IM_INT32 key, void *value)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(key != CAM_KEY_RW_SPECIAL_EFFECT)	//all mode use camera config now except special effect
	{
		IM_INFOMSG((IM_STR("%s(set wb mode)"), IM_STR(_IM_FUNC_)));
		ret = camsen_set_property(gIspSen->camsenHandle, key, value);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("camsen_set_property() failed!")));
			return IM_RET_FAILED;
		}
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

static IM_UINT32 get_property_index(IM_INT32 key, void *value)
{
	IM_UINT32 proId = 0;
	IM_UINT32 index = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&index, value, sizeof(IM_UINT32));
	proId = index2proId(index); 
	IM_INFOMSG((IM_STR("%s(pro index=0x%x, proId=%d)"), IM_STR(_IM_FUNC_), index, proId));
	
	switch (key)
	{
		case CAM_KEY_RW_SPECIAL_EFFECT:
			IM_INFOMSG((IM_STR("%s(set property: special effect)"), IM_STR(_IM_FUNC_)));
			proId += PRO_SPE_EFT_OFFSET;
			break;
		case CAM_KEY_RW_SCENE_MODE:
			IM_INFOMSG((IM_STR("%s(set property: scene mode)"), IM_STR(_IM_FUNC_)));
			proId += PRO_SCE_MODE_OFFSET;
			break;
		default:
			IM_WARNMSG((IM_STR("Isp not support this key(= 0x%d)!"), key));
			break;
	}

	return proId;
}

IM_RET isp_set_property(IM_INT32 key, void *value)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 proId = 0;
	isp_sub_config_t subCfg;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	memset((void*)&subCfg, 0x0, sizeof(isp_sub_config_t));

	if((key == CAM_KEY_RW_SPECIAL_EFFECT) || (key == CAM_KEY_RW_SCENE_MODE))	//only scene mode & special effect property use isp config now
	{
		//get isp sub config index
		proId = get_property_index(key, value);
		IM_INFOMSG((IM_STR("%s(proId = %d)"), IM_STR(_IM_FUNC_), proId));
		memcpy((void*)&subCfg, (void*)&gIspSubCfg[gIspSen->sensorId][proId], sizeof(isp_sub_config_t));

		/***********************
		*  set isp sub config  *
		***********************/

		//set bccb module if enable
		//...

		if(subCfg.eeNeedConfig == IM_TRUE)	//this module need reconfigure
		{
			//set ee module if enable
			if(subCfg.eeCfg.enable == IM_TRUE)
			{
				IM_INFOMSG((IM_STR("%s(ee module is enable)"), IM_STR(_IM_FUNC_)));
				isplib_ee_set_coefw(subCfg.eeCfg.coefw);

				isplib_ee_set_coefa(subCfg.eeCfg.coefa);

				isplib_ee_set_round_mode(subCfg.eeCfg.rdMode);

				isplib_ee_set_gauss_filter_enable(subCfg.eeCfg.gasEn);

				isplib_ee_set_gauss_mode(subCfg.eeCfg.gasMode);

				isplib_ee_set_error_threshold(subCfg.eeCfg.errTh);

				isplib_ee_set_detect_threshold_matrix(&subCfg.eeCfg.thrMat);

				//isplib_ee_set_edge_operator_matrix(direction, opMat);

				isplib_ee_set_enable();
			}
			else
			{
				isplib_ee_set_disable();
			}
		}

		if(subCfg.accNeedConfig == IM_TRUE)	//this module need reconfigure
		{
			//set acc module if enable
			if(subCfg.accCfg.enable == IM_TRUE)
			{
				IM_INFOMSG((IM_STR("%s(acc module is enable)"), IM_STR(_IM_FUNC_)));
				isplib_acc_set_round_mode(subCfg.accCfg.rdMode);

				isplib_acc_set_lutb(&subCfg.accCfg.lutbType);

				isplib_acc_set_hist(&subCfg.accCfg.hist);

				isplib_acc_set_contrast_coef(subCfg.accCfg.coefe);

				isplib_acc_set_coef_matrix(&subCfg.accCfg.coMat);

				isplib_acc_set_ro_matrix(&subCfg.accCfg.roMat);

				isplib_acc_set_enable();
			}
			else
			{
				isplib_acc_set_disable();
			}
		}		

		if(subCfg.iefNeedConfig == IM_TRUE)	//this module need reconfigure
		{
			//set ief module if enable
			if(subCfg.iefCfg.enable == IM_TRUE)
			{
				IM_INFOMSG((IM_STR("%s(ief module is enable)"), IM_STR(_IM_FUNC_)));
				isplib_ief_set_type(subCfg.iefCfg.type);

				isplib_ief_set_rgcf_matrix(&subCfg.iefCfg.rgcfMat);

				isplib_ief_set_color_select_mode_matrix(&subCfg.iefCfg.selMat);

				isplib_ief_set_enable();
			}
			else
			{
				isplib_ief_set_disable();
			}
		}

		if(subCfg.acmNeedConfig == IM_TRUE)	//this module need reconfigure
		{
			//set acm module if enable
			if(subCfg.acmCfg.enable == IM_TRUE)
			{
				IM_INFOMSG((IM_STR("%s(acm module is enable)"), IM_STR(_IM_FUNC_)));
				isplib_acm_set_round_mode(subCfg.acmCfg.rdMode);

				isplib_acm_set_saturation_threshold(subCfg.acmCfg.ths);

				isplib_acm_set_threshold_matrix(&subCfg.acmCfg.thrMat);
				IM_INFOMSG((IM_STR("%s(acm param: coefr=%d, coefg=%d, coefb=%d)"), IM_STR(_IM_FUNC_), subCfg.acmCfg.coMat.coefr, subCfg.acmCfg.coMat.coefg,subCfg.acmCfg.coMat.coefb));

				isplib_acm_set_coef_matrix(&subCfg.acmCfg.coMat);

				isplib_acm_set_enable();
			}
			else
			{
				isplib_acm_set_disable();
			}
		}
	}

	return ret;
}


IM_RET ispsen_set_config(IM_UINT32 res, IM_UINT32 fmt, IM_UINT32 fps)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("%s(res=0x%x, fmt=0x%x, fps=0x%x)"), IM_STR(_IM_FUNC_), res, fmt, fps));
	IM_UINT32 w = 0, h = 0;
	IM_INT32 i = 0;
	IM_RET ret = IM_RET_OK;
	isp_input_mode_t inputMode;
	isp_output_mode_t outputMode;
	camsen_out_mode_t outMode;
	PCHECK_NULL(gIspSen);

	/*set sensor output mode, find the sensor supply resolution for request resolution,
	 *then convert to request resolution by isp scale
	 */
	outMode.res = res;
	while(i<CAM_RES_ENUM_MAX)
	{
		if(gIspSenInit[gIspSen->sensorId].resCaps[i].request == res)
		{
			outMode.res = gIspSenInit[gIspSen->sensorId].resCaps[i].supply;
			break;
		}
		i++;
	}
	outMode.fmt = fmt;
	outMode.fps = fps;
	ret = camsen_set_out_mode(gIspSen->camsenHandle, &outMode);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camsen_set_out_mode() failed!")));
		return IM_RET_FAILED;
	}

	get_frame_size_from_res(outMode.res, &inputMode.inWidth, &inputMode.inHeight);
	IM_INFOMSG((IM_STR("%s(inWidth=%d, inHeight=%d)"), IM_STR(_IM_FUNC_), inputMode.inWidth, inputMode.inHeight));
	//set input mode
	ret = isplib_set_input_resolution(inputMode.inWidth, inputMode.inHeight);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_set_input_resolution() failed!")));
		return IM_RET_FAILED;
	}
	
	calc_frame_size(res, fmt, &w, &h);
	IM_INFOMSG((IM_STR("%s(outWidth=%d, outHeight=%d)"), IM_STR(_IM_FUNC_), w, h));
	//set isp output mode
	outputMode.outWidth = w;
	outputMode.outHeight = h;
	outputMode.outImgType = camFmt2ImType(fmt);
	ret = isplib_set_output_mode(&outputMode);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_set_output_mode() failed!")));
		return IM_RET_FAILED;
	}
	ret = isplib_set_digital_zoom(100 + gIspSen->zmVal*10);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_set_digital_zoom() failed!")));
		return IM_RET_FAILED;
	}


	//update  current preview config setting.
	gIspSen->preCfg.res = res;
	gIspSen->preCfg.fmt = fmt;
	gIspSen->preCfg.fps = fps;

	return ret;
}

IM_RET ispsen_get_config(cam_preview_config_t *preCfg)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_RET ret = IM_RET_OK;
	IM_INT32 i;
	PCHECK_NULL(gIspSen);

	isppwl_memcpy((void *)preCfg, (void *)&gIspSen->preCfg, sizeof(cam_preview_config_t));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET ispsen_set_property(IM_INT32 key, void *value, IM_INT32 size)
{
	IM_INFOMSG((IM_STR("%s(key=0x%x)"), IM_STR(_IM_FUNC_), key));
	IM_UINT32 proVal = 0;
	IM_RET ret = IM_RET_OK;
	int i, step;

	memcpy((void *)&proVal, value, sizeof(IM_UINT32));

	if(((key == CAM_KEY_RW_ZOOM) && (proVal == gIspSen->zmVal)) ||
		((key == CAM_KEY_RW_WB_MODE) && (proVal == gIspSen->wbMode)) ||
		((key == CAM_KEY_RW_SPECIAL_EFFECT) && (proVal == gIspSen->spEffect)) ||
		((key == CAM_KEY_RW_SCENE_MODE) && (proVal == gIspSen->scMode)))
	{
		IM_INFOMSG((IM_STR("This property is already setting value!")));
		return IM_RET_OK;
	}
	
	IM_INFOMSG((IM_STR("%s(key=0x%x, different property=0x%x)"), IM_STR(_IM_FUNC_), key, proVal));
	
	if(key == CAM_KEY_RW_ZOOM) //for zoom only need to change isp
	{
		gIspSen->zmVal = proVal;
		return isplib_set_digital_zoom(100 + proVal*10);
	}

	//set sensor property
	ret = sensor_set_property(key, value);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("sensor_set_property() failed!")));
		return IM_RET_FAILED;
	}

	//set isp property
	ret = isp_set_property(key, value);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("camsen_set_property() failed!")));
		return IM_RET_FAILED;
	}

	switch (key)
	{
		case CAM_KEY_RW_WB_MODE:
			memcpy((void*)&gIspSen->wbMode, value, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy((void*)&gIspSen->spEffect, value, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy((void*)&gIspSen->scMode, value, sizeof(IM_UINT32));
			break;
		default:
			break;
	}
	
	return ret;
}

IM_RET ispsen_get_property(IM_INT32 key, void *value, IM_INT32 size)
{
	IM_INFOMSG((IM_STR("%s(key=0x%x)"), IM_STR(_IM_FUNC_), key));
	IM_INT32 caps;
	IM_RET ret = IM_RET_OK;

	if((key != CAM_KEY_R_SUPPORT_SPECIAL_EFFECT) &&
	  (key != CAM_KEY_RW_SPECIAL_EFFECT)	&&
	  0x1)
	{
		ret = camsen_get_property(gIspSen->camsenHandle, key, value);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("camsen_get_property() failed!")));
			return IM_RET_FAILED;
		}
	}

	switch(key)
	{
		case CAM_KEY_R_CAPS:
			memcpy((void*)&caps, value, sizeof(IM_INT32));
			caps |= (CAM_CAP_SPECIAL_EFFECT_SUPPORT |
				CAM_CAP_SCENE_MODE_SUPPORT |
				CAM_CAP_ZOOM |
				0x0);
			memcpy(value, (void*)&caps, sizeof(IM_INT32));
			return IM_RET_OK;
		case CAM_KEY_R_SUPPORT_SPECIAL_EFFECT:
			caps = CAM_SPECIAL_EFFECT_NONE |
				CAM_SPECIAL_EFFECT_MONO |
				CAM_SPECIAL_EFFECT_NEGATIVE |
				CAM_SPECIAL_EFFECT_SOLARIZE |
				//CAM_SPECIAL_EFFECT_PASTEL |
				//CAM_SPECIAL_EFFECT_MOSAIC |
				//CAM_SPECIAL_EFFECT_RESIZE |
				CAM_SPECIAL_EFFECT_SEPIA |
				CAM_SPECIAL_EFFECT_POSTERIZE |
				CAM_SPECIAL_EFFECT_WHITEBOARD |
				CAM_SPECIAL_EFFECT_BLACKBOARD |
				0x0;
			memcpy(value, (void*)&caps, sizeof(IM_INT32));
			return IM_RET_OK;
		case CAM_KEY_R_SUPPORT_SCENE_MODE:
			caps = CAM_SCENE_MODE_AUTO |
				//CAM_SCENE_MODE_ACTION |
				CAM_SCENE_MODE_PORTRAIT |
				CAM_SCENE_MODE_LANDSCAPE |
				CAM_SCENE_MODE_NIGHT |
				//CAM_SCENE_MODE_NIGHT_PORTRAIT |
				//CAM_SCENE_MODE_THEATRE |
				//CAM_SCENE_MODE_BEACH |
				//CAM_SCENE_MODE_SNOW |
				CAM_SCENE_MODE_SUNSET |
				//CAM_SCENE_MODE_STEADYPHOTO |
				//CAM_SCENE_MODE_FIREWORKS |
				//CAM_SCENE_MODE_SPORTS |
				//CAM_SCENE_MODE_PARTY |
				//CAM_SCENE_MODE_CANDLELIGHT |
				//CAM_SCENE_MODE_BARCODE |
				0x0;
			memcpy(value, (void*)&caps, sizeof(IM_INT32));
			return IM_RET_OK;
		case CAM_KEY_R_MAX_ZOOM:
			*((IM_INT32 *)value) = 30;//100, 110, 120,...400(30 values except 1000 in all)
			return IM_RET_OK;
		case CAM_KEY_RW_ZOOM:
			memcpy(value, (void*)&gIspSen->zmVal, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SPECIAL_EFFECT:
			memcpy(value, (void*)&gIspSen->spEffect, sizeof(IM_UINT32));
			break;
		case CAM_KEY_RW_SCENE_MODE:
			memcpy(value, (void*)&gIspSen->scMode, sizeof(IM_UINT32));
			break;
		default:
			break;
	}

	IM_INFOMSG((IM_STR("%s(key=0x%x, property=%d)"), IM_STR(_IM_FUNC_), key, *((IM_INT32 *)value)));

	return ret;
}

IM_RET ispsen_assign_buffer(IM_Buffer *buffer)
{

	IM_INFOMSG((IM_STR("%s(buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x)"), 
		IM_STR(_IM_FUNC_), (IM_INT32)buffer->vir_addr, buffer->phy_addr, buffer->size, buffer->flag));
	PCHECK_NULL(gIspSen);
	PCHECK_NULL(buffer);
	PCHECK_NULL(buffer->vir_addr);
	PCHECK_EQ(buffer->phy_addr, 0);

	if(gIspSen->enabled == IM_TRUE){
		IM_ERRMSG((IM_STR("isp is started, can not assign buffer")));
		return IM_RET_FAILED;
	}

	if(gIspSen->bufferNum >= BUFFER_NUM){
		IM_WARNMSG((IM_STR("isp buffer is enough, don't need anymore")));
		return IM_RET_FALSE;
	}

	isppwl_memcpy((void *)&gIspSen->buffers[gIspSen->bufferNum], (void *)buffer, sizeof(IM_Buffer));
	gIspSen->bufferNum++;

	return IM_RET_OK;
}

IM_RET ispsen_release_buffer(IM_Buffer *buffer)
{
	IM_INFOMSG((IM_STR("%s(buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x)"), 
		IM_STR(_IM_FUNC_), (IM_INT32)buffer->vir_addr, buffer->phy_addr, buffer->size, buffer->flag));
	PCHECK_NULL(gIspSen);
	PCHECK_NULL(buffer);
	PCHECK_NULL(buffer->vir_addr);
	PCHECK_EQ(buffer->phy_addr, 0);
	
	isppwl_spinlock(gIspSen->spinLock);	
	if(gIspSen->bufferNum >= BUFFER_NUM){
		isppwl_spinunlock(gIspSen->spinLock);	
		IM_WARNMSG((IM_STR("isp buffer is enough, don't need anymore")));
		return IM_RET_FALSE;
	}

	if(gIspSen->enabled == IM_TRUE){
		PCHECK_LT(buffer->size, gIspSen->imageSize);
	}

	isppwl_memcpy((void *)&gIspSen->buffers[BUFFER_NUM-1], (void *)buffer, sizeof(IM_Buffer));
	gIspSen->bufferNum++;
	isppwl_spinunlock(gIspSen->spinLock);	

	return IM_RET_OK;

}

IM_RET ispsen_start(void)
{
	IM_INT32 i;
	IM_RET ret = IM_RET_OK;
	isp_dma_config_t dmaCfg;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gIspSen);

	if(gIspSen->enabled == IM_TRUE){
		IM_WARNMSG((IM_STR("isp has been started")));
		return IM_RET_OK;
	}

	if(gIspSen->bufferNum < BUFFER_NUM){
		IM_ERRMSG((IM_STR("isp buffer is not enough")));
		return IM_RET_FAILED;
	}

	for(i=0; i<BUFFER_NUM; i++){
		if(gIspSen->buffers[i].size < gIspSen->imageSize){
			IM_ERRMSG((IM_STR("isp buffer[%d] size(%d, imageSize=%d) is invalid"), 
				i, gIspSen->buffers[i].size, gIspSen->imageSize));
			return IM_RET_FAILED;
		}
	}

	if((BUFFER_NUM-1) == 1){
		dmaCfg.ppMode = ISP_PPMODE_DISABLE;
	}else if((BUFFER_NUM-1) == 2){
		dmaCfg.ppMode = ISP_PPMODE_2_BUFFER;
	}else{
		IM_ASSERT((BUFFER_NUM-1) == 4);
		dmaCfg.ppMode = ISP_PPMODE_4_BUFFER;
	}
	IM_INFOMSG((IM_STR("ySize=%d, uSize=%d, vSize=%d, imgSize=%d"), gIspSen->ySize, gIspSen->uSize, gIspSen->vSize, gIspSen->imageSize));
	if(gIspSen->ySize != 0){
		for(i=0; i<BUFFER_NUM-1; i++){
			dmaCfg.buffY[i].vir_addr = gIspSen->buffers[i].vir_addr;
			dmaCfg.buffY[i].phy_addr = gIspSen->buffers[i].phy_addr;
			dmaCfg.buffY[i].size = gIspSen->ySize;
			dmaCfg.buffY[i].flag = gIspSen->buffers[i].flag;
		}
	}
	if(gIspSen->uSize != 0){
		for(i=0; i<BUFFER_NUM-1; i++){
			dmaCfg.buffCb[i].vir_addr = (void *)((IM_INT32)gIspSen->buffers[i].vir_addr + gIspSen->ySize);
			dmaCfg.buffCb[i].phy_addr = gIspSen->buffers[i].phy_addr + gIspSen->ySize;
			dmaCfg.buffCb[i].size = gIspSen->uSize;
			dmaCfg.buffCb[i].flag = gIspSen->buffers[i].flag;
		}
	}
	if(gIspSen->vSize != 0){
		for(i=0; i<BUFFER_NUM-1; i++){
			dmaCfg.buffCr[i].vir_addr = (void *)((IM_INT32)gIspSen->buffers[i].vir_addr + gIspSen->ySize + gIspSen->uSize);
			dmaCfg.buffCr[i].phy_addr = gIspSen->buffers[i].phy_addr + gIspSen->ySize + gIspSen->uSize;
			dmaCfg.buffCr[i].size = gIspSen->vSize;
			dmaCfg.buffCr[i].flag = gIspSen->buffers[i].flag;
		}
	}

	ret = isplib_set_buffers(&dmaCfg);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_set_buffers() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}

	ret = camsen_start(gIspSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@camsen_start, camsen_start() failed!")));
		return IM_RET_FAILED;
	}

	ret = isplib_start();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_start() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}

	gIspSen->hasRdyFrame = IM_FALSE;
	gIspSen->enabled = IM_TRUE;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET ispsen_stop(void)
{
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gIspSen);

	if(gIspSen->enabled == IM_FALSE){
		IM_WARNMSG((IM_STR("isp has not been started")));
		return IM_RET_OK;
	}

	ret = isplib_stop();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_stop() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
	ret = camsen_stop(gIspSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@ispsen_stop, camsen_stop() failed!")));
		return IM_RET_FAILED;
	}
	gIspSen->bufferNum = 0;
	gIspSen->hasRdyFrame = IM_FALSE;
	gIspSen->enabled = IM_FALSE;

	return ret;
}

static IM_UINT32 frame_num = 0;

static IM_INT64 ispsen_get_system_time_us(){
	struct timeval tv;
	do_gettimeofday(&tv);
	return (IM_INT64)((IM_INT64)tv.tv_usec  + (IM_INT64)(((IM_INT64)tv.tv_sec)* 1000000LL));
}

//interrupt callback function
IM_RET ispsen_irq_handle(void)
{
	IM_RET ret = IM_RET_OK;
	strm_frame_t frm;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gIspSen);

	if(gIspSen->enabled == IM_FALSE){
		IM_ERRMSG((IM_STR("isp not enabled")));
		return IM_RET_FAILED;
	}

	isppwl_spinlock(gIspSen->spinLock);	
	frame_num++;
	if((gIspSen->bufferNum < BUFFER_NUM) && (gIspSen->hasRdyFrame == IM_FALSE)){
		IM_ERRMSG((IM_STR("isp has no chance to save captured frame, frame_num=%d"), frame_num));
		isppwl_spinunlock(gIspSen->spinLock);	
		return IM_RET_FAILED;
	}else{
		ret = save_ready_frame();
		if(ret != IM_RET_OK)
		{
			IM_ERRMSG((IM_STR("@ispsen_irq_handle, save_ready_frame() failed!")));
			isppwl_spinunlock(gIspSen->spinLock);	
			return IM_RET_FAILED;
		}
	}	

	if(gIspSen->hasRdyFrame == IM_FALSE){
		IM_ERRMSG((IM_STR("isp has no captured frame")));
		isppwl_spinunlock(gIspSen->spinLock);	
		return IM_RET_FAILED;
	}

	frm.buffer.vir_addr = gIspSen->rdyBuffer.vir_addr;
	frm.buffer.phy_addr = gIspSen->rdyBuffer.phy_addr;
	frm.buffer.size = gIspSen->rdyBuffer.size;
	frm.buffer.flag = gIspSen->rdyBuffer.flag;
	frm.ts = ispsen_get_system_time_us();
	frm.id = frame_num;
	IM_INFOMSG((IM_STR("isp saved frame num=%d"), frame_num));
	IM_INFOMSG((IM_STR("isp saved frame timestamp=%lld"), frm.ts));
	IM_INFOMSG((IM_STR("isp get buffer: vir=0x%x, phy=0x%x, size=%d, flag=0x%x"), 
		(IM_INT32)frm.buffer.vir_addr, frm.buffer.phy_addr, frm.buffer.size, frm.buffer.flag));

	isppwl_spinunlock(gIspSen->spinLock);	
	//call frame_done
	gl.frame_done(&frm);

	gIspSen->hasRdyFrame = IM_FALSE;
	return ret;
}

IM_RET save_ready_frame(void)
{
	IM_RET ret = IM_RET_OK;
	IM_Buffer readyBuffer[3] = {0}, replaceBuffer[3] = {0};

	if(gIspSen->bufferNum == BUFFER_NUM){	// there has idle buffer.
		IM_INFOMSG((IM_STR("isp use idle buffer to replace out ready frame")));
		if(gIspSen->ySize != 0){
			replaceBuffer[0].vir_addr = gIspSen->buffers[BUFFER_NUM-1].vir_addr;
			replaceBuffer[0].phy_addr = gIspSen->buffers[BUFFER_NUM-1].phy_addr;
			replaceBuffer[0].size = gIspSen->ySize;
			replaceBuffer[0].flag = gIspSen->buffers[BUFFER_NUM-1].flag;
		}
		if(gIspSen->uSize != 0){
			replaceBuffer[1].vir_addr = (void *)((IM_INT32)gIspSen->buffers[BUFFER_NUM-1].vir_addr + gIspSen->ySize);
			replaceBuffer[1].phy_addr = gIspSen->buffers[BUFFER_NUM-1].phy_addr + gIspSen->ySize;
			replaceBuffer[1].size = gIspSen->uSize;
			replaceBuffer[1].flag = gIspSen->buffers[BUFFER_NUM-1].flag;
		}
		if(gIspSen->vSize != 0){
			replaceBuffer[2].vir_addr = (void *)((IM_INT32)gIspSen->buffers[BUFFER_NUM-1].vir_addr + gIspSen->ySize + gIspSen->uSize);
			replaceBuffer[2].phy_addr = gIspSen->buffers[BUFFER_NUM-1].phy_addr + gIspSen->ySize + gIspSen->uSize;
			replaceBuffer[2].size = gIspSen->vSize;
			replaceBuffer[2].flag = gIspSen->buffers[BUFFER_NUM-1].flag;
		}

		gIspSen->bufferNum--;
	}else{	// there has ready buffer.
		IM_ASSERT(gIspSen->hasRdyFrame == IM_TRUE);
		IM_INFOMSG((IM_STR("isp use ready buffer to replace out ready frame")));
		if(gIspSen->ySize != 0){
			replaceBuffer[0].vir_addr = gIspSen->rdyBuffer.vir_addr;
			replaceBuffer[0].phy_addr = gIspSen->rdyBuffer.phy_addr;
			replaceBuffer[0].size = gIspSen->ySize;
			replaceBuffer[0].flag = gIspSen->rdyBuffer.flag;
		}
		if(gIspSen->uSize != 0){
			replaceBuffer[1].vir_addr = (void *)((IM_INT32)gIspSen->rdyBuffer.vir_addr + gIspSen->ySize);
			replaceBuffer[1].phy_addr = gIspSen->rdyBuffer.phy_addr + gIspSen->ySize;
			replaceBuffer[1].size = gIspSen->uSize;
			replaceBuffer[1].flag = gIspSen->rdyBuffer.flag;
		}
		if(gIspSen->vSize != 0){
			replaceBuffer[2].vir_addr = (void *)((IM_INT32)gIspSen->rdyBuffer.vir_addr + gIspSen->ySize + gIspSen->uSize);
			replaceBuffer[2].phy_addr = gIspSen->rdyBuffer.phy_addr + gIspSen->ySize + gIspSen->uSize;
			replaceBuffer[2].size = gIspSen->vSize;
			replaceBuffer[2].flag = gIspSen->rdyBuffer.flag;
		}

		gIspSen->hasRdyFrame = IM_FALSE;
	}

	ret = isplib_get_ready_buffer(readyBuffer, replaceBuffer);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("isplib_get_ready_buffer() failed, ret=%d"), ret));
		return IM_RET_FAILED;
	}
	gIspSen->rdyBuffer.vir_addr = readyBuffer[0].vir_addr;
	gIspSen->rdyBuffer.phy_addr = readyBuffer[0].phy_addr;
	gIspSen->rdyBuffer.size = readyBuffer[0].size + readyBuffer[1].size + readyBuffer[2].size;
	gIspSen->rdyBuffer.flag = readyBuffer[0].flag;
	IM_INFOMSG((IM_STR("save frame%d: vir=0x%x, phy=0x%x, size=%d, flag=0x%x"), 
				frame_num, (IM_INT32)gIspSen->rdyBuffer.vir_addr, gIspSen->rdyBuffer.phy_addr, 
				gIspSen->rdyBuffer.size, gIspSen->rdyBuffer.flag));

	gIspSen->hasRdyFrame = IM_TRUE;

	return ret;

}
//=================================================================


//=====================camera control exec=========================
IM_RET ispsen_auto_focus(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}
//=================================================================



//=====================camapi interface=============================
IM_RET ispsen_init(camdev_config_t *devCfg, workmodel_config_t *wmCfg)
{
	IM_INT32 index, i, j, k;
	IM_RET ret = IM_RET_OK;
	IM_BOOL vgaSupport = IM_TRUE;
	IM_UINT32 vgaId = 6;
	IM_UINT32 w, h;
	IM_INT32 irq;
	camsen_caps_t caps;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gIspSen != IM_NULL) return IM_RET_OK;

	gIspSen = (isp_sen_context_t *)isppwl_malloc(sizeof(isp_sen_context_t));
	if(gIspSen == IM_NULL){
		IM_ERRMSG((IM_STR("isppwl_malloc() failed")));
		goto Fail;
	}
	isppwl_memset((void *)gIspSen, 0, sizeof(isp_sen_context_t));

	//init digital zoom value
	gIspSen->zmVal = 0;							//default zoom value(non zoom)	
	gIspSen->wbMode = CAM_WB_MODE_AUTO;				//default white balance mode
	gIspSen->spEffect = CAM_SPECIAL_EFFECT_NONE;	//default special effect
	gIspSen->scMode = CAM_SCENE_MODE_AUTO;			//default scene mode

	if(isppwl_spinlock_init(&gIspSen->spinLock) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_spinlock_init() failed")));
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
			//wmCfg->cmcfg.control_exec[index] = ispsen_auto_focus;
		}
	}	

	//find sensor id
	for(i=0; i<sizeof(gIspSenInit)/sizeof(isp_sensor_init_t); i++)
	{
		if(strcmp(gIspSenInit[i].senName, devCfg->camsenName) == 0)
		{
			break;
		}
	}
	gIspSen->sensorId = i;

	IM_INFOMSG((IM_STR("%s(gIspSen->sensorId=%d)"), IM_STR(_IM_FUNC_), gIspSen->sensorId));
	//camera sensor init
	ret = camsen_init(&gIspSen->camsenHandle, devCfg->camsenName, devCfg->facing, IM_FALSE);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@ispsen_init, camsen_init() failed!")));
		goto Fail;
	}
	
	if(i == (sizeof(gIspSenInit)/sizeof(isp_sensor_init_t)))//general sensorId
	{
		if(devCfg->itfType == CAMISP_INTERFACE_MIPI)
		{
			gIspSen->sensorId = i - 2; //general mipi sensor
		}
		else
		{
			gIspSen->sensorId = i - 1; //general dvp sensor
		}
		//get some info from camsen driver such as resolution
		strcpy(gIspSenInit[gIspSen->sensorId].senName, devCfg->camsenName);

		ret = camsen_get_caps(gIspSen->camsenHandle, &caps);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("@ispsen_init, camsen_init() failed!")));
			goto Fail;
		}

		gIspSenInit[gIspSen->sensorId].preCfg.resBits = caps.supportRes;
		IM_INFOMSG((IM_STR("@ispsen_init, sensor support resolution=0x%x, init resolution=0x%x!"), caps.supportRes, caps.initRes));
		k = 0;
		for(j=0; j<CAM_RES_ENUM_MAX; j++)
		{
			if(caps.supportRes&(1<<j))
			{
				//support VGA if sensor can support size larger than VGA but not VGA 
				if((vgaSupport == IM_FALSE) && ((1<<j) > CAM_RES_VGA))
				{
                    gIspSenInit[gIspSen->sensorId].preCfg.resBits |= CAM_RES_VGA;
					gIspSenInit[gIspSen->sensorId].preCfg.attri[vgaId].fpsBits = CAM_FPS_30;
					gIspSenInit[gIspSen->sensorId].resCaps[k].request = CAM_RES_VGA;
					gIspSenInit[gIspSen->sensorId].resCaps[k].supply = (1<<j);
					k++;
				}

				gIspSenInit[gIspSen->sensorId].preCfg.attri[j].fpsBits = CAM_FPS_30;
				gIspSenInit[gIspSen->sensorId].resCaps[k].request = gIspSenInit[gIspSen->sensorId].resCaps[k].supply = (1<<j);
				k++;
			}
			else if((1<<j) == CAM_RES_VGA)//sensor not support VGA
			{
				vgaId = j;
				vgaSupport = IM_FALSE;
			}
		}

		for(j=0; j<CAM_RES_ENUM_MAX; j++)
		{
			if(gIspSenInit[gIspSen->sensorId].resCaps[j].supply == caps.initRes)
			{
				gIspSenInit[gIspSen->sensorId].preCfg.res = gIspSenInit[gIspSen->sensorId].resCaps[j].request;
				break;

			}
		}

		IM_INFOMSG((IM_STR("@ispsen_init, isp support resolution=0x%x, init resolution=0x%x!"), gIspSenInit[gIspSen->sensorId].preCfg.resBits, gIspSenInit[gIspSen->sensorId].preCfg.res));

		//isp input size
		get_frame_size_from_res(gIspSenInit[gIspSen->sensorId].resCaps[j].supply, &w, &h);

		gIspSenInit[gIspSen->sensorId].ispCfg.inWidth = w;
		gIspSenInit[gIspSen->sensorId].ispCfg.inHeight = h;
		gIspSenInit[gIspSen->sensorId].ispCfg.cropCfg.coordinate.x1 = w - 1;
		gIspSenInit[gIspSen->sensorId].ispCfg.cropCfg.coordinate.y1 = h - 1;
		gIspSenInit[gIspSen->sensorId].ispCfg.sclCfg.sclInWidth = w;
		gIspSenInit[gIspSen->sensorId].ispCfg.sclCfg.sclInHeight = h;
		
		IM_INFOMSG((IM_STR("@@@@@@@@@@@, isp input size(w=%d, h=%d)"), w, h));

		//isp output size
		get_frame_size_from_res(gIspSenInit[gIspSen->sensorId].resCaps[j].request, &w, &h);
		gIspSenInit[gIspSen->sensorId].ispCfg.sclCfg.sclOutWidth = w;
		gIspSenInit[gIspSen->sensorId].ispCfg.sclCfg.sclOutHeight = h;

		gIspSenInit[gIspSen->sensorId].ispCfg.outWidth = w;
		gIspSenInit[gIspSen->sensorId].ispCfg.outHeight = h;

		IM_INFOMSG((IM_STR("###########, isp output size(w=%d, h=%d)"), w, h));
		
	}

	//init isp_sen perview config
	isppwl_memcpy((void *)&gIspSen->preCfg, (void *)&gIspSenInit[gIspSen->sensorId].preCfg, sizeof(cam_preview_config_t));


	if(devCfg->dataType == CAM_DATATYPE_LOW8_ALIGN)
	{
		gIspSenInit[gIspSen->sensorId].ispCfg.inputBitsNum = ISP_INPUT_LOW_BITS_8;
	}
	else if(devCfg->dataType == CAM_DATATYPE_MID8_ALIGN)
	{
		gIspSenInit[gIspSen->sensorId].ispCfg.inputBitsNum = ISP_INPUT_MID_BITS_8;
	}
	else if(devCfg->dataType == CAM_DATATYPE_LOW10_ALIGN)
	{
		gIspSenInit[gIspSen->sensorId].ispCfg.inputBitsNum = ISP_INPUT_LOW_BITS_10;
	}
	else
	{
		gIspSenInit[gIspSen->sensorId].ispCfg.inputBitsNum = ISP_INPUT_BITS_12;
	}

	ret = isplib_init(&gIspSenInit[gIspSen->sensorId].ispCfg);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@ispsen_init, isplib_init() failed!")));
		goto Fail;
	}

	//register isp irq callback function, it must call after isppwl_init(), that means call after isplib_init()
	isppwl_register_irq(irq, ispsen_irq_handle); 

	//Need compute according to out image format and out image width and height
	calc_frame_size(gIspSenInit[gIspSen->sensorId].preCfg.res, gIspSenInit[gIspSen->sensorId].preCfg.fmt, &w, &h);

	return ret;
Fail:
	//free resource
	if(gIspSen != IM_NULL){
		if(gIspSen->spinLock != IM_NULL){
			isppwl_spinlock_deinit(gIspSen->spinLock);	
			gIspSen->spinLock = IM_NULL;
		}

		isppwl_free((void *)gIspSen);
		gIspSen = IM_NULL;
	}
	return IM_RET_FAILED;		

}

IM_RET ispsen_deinit(void)
{
	IM_RET ret = IM_RET_OK;
	IM_UINT32 sensorId = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	PCHECK_NULL(gIspSen);

	//sensor deinit
	ret = camsen_deinit(gIspSen->camsenHandle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@ispsen_deinit, camsen_deinit() failed!")));
		return IM_RET_FAILED;		
	}
	//isplib deinit
	ret = isplib_deinit();
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@ispsen_deinit, isplib_deinit() failed!")));
		return IM_RET_FAILED;		
	}

	if(gIspSen->spinLock != IM_NULL){
		isppwl_spinlock_deinit(gIspSen->spinLock);	
		gIspSen->spinLock = IM_NULL;
	}

	isppwl_free((void *)gIspSen);
	gIspSen = IM_NULL;
	return ret;
}

IM_RET ispsen_ioctl(IM_INT32 cmd, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_Buffer *buff = IM_NULL;
	cam_preview_config_t *preCfg;
	ioctl_ds_set_config *cfg;
	ioctl_ds_key_property *pro;
	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));
	switch(cmd){
		case CAMISP_IOCTL_CMD_SET_CFG:
			cfg = (ioctl_ds_set_config *)p;
			IM_INFOMSG((IM_STR("ispsen_api(res=0x%x, fps=0x%x)"), cfg->res, cfg->fps));
			ret = ispsen_set_config(cfg->res, cfg->fmt, cfg->fps);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_set_config() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_GET_CFG:
			preCfg = (cam_preview_config_t *)p;
			ret = ispsen_get_config(preCfg);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_get_config() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_SET_PRO:
			pro = (ioctl_ds_key_property *)p;
			ret = ispsen_set_property(pro->key, (void*)pro->param, pro->size);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_set_property() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_GET_PRO:
			pro = (ioctl_ds_key_property *)p;
			ret = ispsen_get_property(pro->key, (void*)pro->param, pro->size);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_get_property() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_ASSIGN_BUF:
			buff = (IM_Buffer *)p;
			ret = ispsen_assign_buffer(buff);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_assign_buffer() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_RELEASE_BUF:
			buff = (IM_Buffer *)p;
			ret = ispsen_release_buffer(buff);
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_release_buffer() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_START:
			ret = ispsen_start();
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_start() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		case CAMISP_IOCTL_CMD_STOP:
			ret = ispsen_stop();
			if(ret != IM_RET_OK)
			{
				IM_ERRMSG((IM_STR("@ispsen_ioctl, ispsen_stop() failed!")));
				return IM_RET_FAILED;		
			}
			break;
		default:
			IM_ERRMSG((IM_STR("@ispsen_ioctl, ioctl cmd error: %d!"), cmd));
			return IM_RET_FAILED;		
	}
	return ret;
}

IM_RET ispsen_suspend(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET ispsen_resume(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

//register api to work model
IM_RET workmodel_register_camapi(camapi_interface_t *apis)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	if(apis == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameteri, apis is NULL")));
		return IM_RET_FAILED;		
	}
	apis->camapi_init = ispsen_init;
	apis->camapi_deinit = ispsen_deinit;
	apis->camapi_ioctl = ispsen_ioctl;
	apis->camapi_suspend = ispsen_suspend;
	apis->camapi_resume = ispsen_resume;

	return IM_RET_OK;
}

