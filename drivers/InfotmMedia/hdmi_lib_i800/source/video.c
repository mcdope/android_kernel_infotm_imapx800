/*
 * video.c
 *
 *  Created on: Jul 5, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */

#include "video.h"
#include "hdmi_log.h"
#include "halColorSpaceConverter.h"
#include "halFrameComposerVideo.h"
#include "halIdentification.h"
#include "halMainController.h"
#include "halVideoPacketizer.h"
#include "halVideoSampler.h"
#include "halVideoGenerator.h"
#include "halFrameComposerDebug.h"
#include "dtd.h"
#include "error.h"

const IM_UINT16 TX_BASE_ADDR = 0x0200;
const IM_UINT16 VP_BASE_ADDR = 0x0800;
static const IM_UINT16 FC_BASE_ADDR = 0x1000;
static const IM_UINT16 MC_BASE_ADDR = 0x4000;
const IM_UINT16 CSC_BASE_ADDR = 0x4100;
const IM_UINT16 VG_BASE_ADDR = 0x7000;

IM_INT32 video_Initialize(IM_UINT16 baseAddr, videoParams_t *params, IM_UINT8 dataEnablePolarity)
{
	LOG_TRACE1(dataEnablePolarity);
	return TRUE;
}
IM_INT32 video_Configure(IM_UINT16 baseAddr, videoParams_t *params, IM_UINT8 dataEnablePolarity, IM_UINT8 hdcp)
{
	LOG_TRACE();

	/* DVI mode does not support pixel repetition */
	
 	if (!videoParams_GetHdmi(params) && videoParams_IsPixelRepetition(params))
	{
		error_Set(ERR_DVI_MODE_WITH_PIXEL_REPETITION);
		LOG_ERROR("DVI mode with pixel repetition: video not transmitted");
		return FALSE;
	}
	if (video_ForceOutput(baseAddr, 1) != TRUE)
		return FALSE;
	if (video_FrameComposer(baseAddr, params, dataEnablePolarity, hdcp) != TRUE)
		return FALSE;
	if (video_VideoPacketizer(baseAddr, params) != TRUE)
		return FALSE;
	if (video_ColorSpaceConverter(baseAddr, params) != TRUE)
		return FALSE;
	if (video_VideoSampler(baseAddr, params) != TRUE)
		return FALSE;
	return TRUE;
	//return video_VideoGenerator(baseAddr, params, dataEnablePolarity); //sam : 0x7000+ controller register not used any more.
}

IM_INT32 video_ForceOutput(IM_UINT16 baseAddr, IM_UINT8 force)
{
	halFrameComposerDebug_ForceAudio(baseAddr + FC_BASE_ADDR + 0x0200,
			0);
	halFrameComposerDebug_ForceVideo(baseAddr + FC_BASE_ADDR + 0x0200,
			force);
	return TRUE;
}

IM_INT32 video_ColorSpaceConverter(IM_UINT16 baseAddr, videoParams_t *params)
{
	IM_UINT32 interpolation = 0;
	IM_UINT32 decimation = 0;
	IM_UINT32 color_depth = 0;

	LOG_TRACE();

	if (videoParams_IsColorSpaceInterpolation(params))
	{
		if (videoParams_GetCscFilter(params) > 1)
		{
			error_Set(ERR_CHROMA_INTERPOLATION_FILTER_INVALID);
			LOG_ERROR2("invalid chroma interpolation filter: ", videoParams_GetCscFilter(params));
			return FALSE;
		}
		interpolation = 1 + videoParams_GetCscFilter(params);
	}
	else if (videoParams_IsColorSpaceDecimation(params))
	{
		if (videoParams_GetCscFilter(params) > 2)
		{
			error_Set(ERR_CHROMA_DECIMATION_FILTER_INVALID);
			LOG_ERROR2("invalid chroma decimation filter: ", videoParams_GetCscFilter(params));
			return FALSE;
		}
		decimation = 1 + videoParams_GetCscFilter(params);
	}

	if ((videoParams_GetColorResolution(params) == 8)
			|| (videoParams_GetColorResolution(params) == 0))
		color_depth = 4;
	else if (videoParams_GetColorResolution(params) == 10)
		color_depth = 5;
	else if (videoParams_GetColorResolution(params) == 12)
		color_depth = 6;
	else if (videoParams_GetColorResolution(params) == 16)
		color_depth = 7;
	else
	{
		error_Set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
		LOG_ERROR2("invalid color depth: ", videoParams_GetColorResolution(params));
		return FALSE;
	}

	halColorSpaceConverter_Interpolation(baseAddr + CSC_BASE_ADDR,
			interpolation);
	halColorSpaceConverter_Decimation(baseAddr + CSC_BASE_ADDR,
			decimation);
	halColorSpaceConverter_CoefficientA1(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscA(params)[0]);
	halColorSpaceConverter_CoefficientA2(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscA(params)[1]);
	halColorSpaceConverter_CoefficientA3(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscA(params)[2]);
	halColorSpaceConverter_CoefficientA4(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscA(params)[3]);
	halColorSpaceConverter_CoefficientB1(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscB(params)[0]);
	halColorSpaceConverter_CoefficientB2(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscB(params)[1]);
	halColorSpaceConverter_CoefficientB3(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscB(params)[2]);
	halColorSpaceConverter_CoefficientB4(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscB(params)[3]);
	halColorSpaceConverter_CoefficientC1(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscC(params)[0]);
	halColorSpaceConverter_CoefficientC2(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscC(params)[1]);
	halColorSpaceConverter_CoefficientC3(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscC(params)[2]);
	halColorSpaceConverter_CoefficientC4(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscC(params)[3]);
	halColorSpaceConverter_ScaleFactor(baseAddr + CSC_BASE_ADDR,
			videoParams_GetCscScale(params));
	halColorSpaceConverter_ColorDepth(baseAddr + CSC_BASE_ADDR,
			color_depth);
	return TRUE;
}

IM_INT32 video_FrameComposer(IM_UINT16 baseAddr, videoParams_t *params, IM_UINT8 dataEnablePolarity, IM_UINT8 hdcp)
{
	const dtd_t *dtd = videoParams_GetDtd(params);
	IM_UINT16 i = 0;

	LOG_TRACE();
	/* HDCP support was checked previously */
	halFrameComposerVideo_HdcpKeepout(baseAddr + FC_BASE_ADDR, hdcp);
	halFrameComposerVideo_VSyncPolarity(baseAddr + FC_BASE_ADDR,
			dtd_GetVSyncPolarity(dtd));
	halFrameComposerVideo_HSyncPolarity(baseAddr + FC_BASE_ADDR,
			dtd_GetHSyncPolarity(dtd));
	halFrameComposerVideo_DataEnablePolarity(baseAddr + FC_BASE_ADDR,
			dataEnablePolarity);
	halFrameComposerVideo_DviOrHdmi(baseAddr + FC_BASE_ADDR,
			videoParams_GetHdmi(params));
	halFrameComposerVideo_VBlankOsc(baseAddr + FC_BASE_ADDR,
			(dtd_GetCode(dtd) == 39) ? 0 : dtd_GetInterlaced(dtd));
	halFrameComposerVideo_Interlaced(baseAddr + FC_BASE_ADDR,
			dtd_GetInterlaced(dtd));
	halFrameComposerVideo_HActive(baseAddr + FC_BASE_ADDR,
			dtd_GetHActive(dtd));
	halFrameComposerVideo_HBlank(baseAddr + FC_BASE_ADDR,
			dtd_GetHBlanking(dtd));
	halFrameComposerVideo_VActive(baseAddr + FC_BASE_ADDR,
			dtd_GetVActive(dtd));
	halFrameComposerVideo_VBlank(baseAddr + FC_BASE_ADDR,
			dtd_GetVBlanking(dtd));
	halFrameComposerVideo_HSyncEdgeDelay(baseAddr + FC_BASE_ADDR,
			dtd_GetHSyncOffset(dtd));
	halFrameComposerVideo_HSyncPulseWidth(baseAddr + FC_BASE_ADDR,
			dtd_GetHSyncPulseWidth(dtd));
	halFrameComposerVideo_VSyncEdgeDelay(baseAddr + FC_BASE_ADDR,
			dtd_GetVSyncOffset(dtd));
	halFrameComposerVideo_VSyncPulseWidth(baseAddr + FC_BASE_ADDR,
			dtd_GetVSyncPulseWidth(dtd));
	halFrameComposerVideo_ControlPeriodMinDuration(baseAddr
			+ FC_BASE_ADDR, 12);
	halFrameComposerVideo_ExtendedControlPeriodMinDuration(baseAddr
			+ FC_BASE_ADDR, 32);
	/* spacing < 256^2 * config / tmdsClock, spacing <= 50ms
	 * worst case: tmdsClock == 25MHz => config <= 19
	 */
	halFrameComposerVideo_ExtendedControlPeriodMaxSpacing(baseAddr
			+ FC_BASE_ADDR, 1);
	for (i = 0; i < 3; i++)
		halFrameComposerVideo_PreambleFilter(baseAddr + FC_BASE_ADDR,
				(i + 1) * 11, i);
	halFrameComposerVideo_PixelRepetitionInput(
			baseAddr + FC_BASE_ADDR, dtd_GetPixelRepetitionInput(dtd)
					+ 1);
	return TRUE;
}

IM_INT32 video_VideoPacketizer(IM_UINT16 baseAddr, videoParams_t *params)
{
	IM_UINT32 color_depth = 0;
	IM_UINT32 remap_size = 0;
	IM_UINT32 output_select = 0;

	LOG_TRACE();
	if (videoParams_GetEncodingOut(params) == RGB
			|| videoParams_GetEncodingOut(params) == YCC444)
	{
		if (videoParams_GetColorResolution(params) == 0)
			output_select = 3;
		else if (videoParams_GetColorResolution(params) == 8)
		{
			color_depth = 4;
			output_select = 3;
		}
		else if (videoParams_GetColorResolution(params) == 10)
			color_depth = 5;
		else if (videoParams_GetColorResolution(params) == 12)
			color_depth = 6;
		else if (videoParams_GetColorResolution(params) == 16)
			color_depth = 7;
		else
		{
			error_Set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
			LOG_ERROR2("invalid color depth: ", videoParams_GetColorResolution(params));
			return FALSE;
		}
	}
	else if (videoParams_GetEncodingOut(params) == YCC422)
	{
		if ((videoParams_GetColorResolution(params) == 8)
				|| (videoParams_GetColorResolution(params) == 0))
			remap_size = 0;
		else if (videoParams_GetColorResolution(params) == 10)
			remap_size = 1;
		else if (videoParams_GetColorResolution(params) == 12)
			remap_size = 2;
		else
		{
			error_Set(ERR_COLOR_REMAP_SIZE_INVALID);
			LOG_ERROR2("invalid color remap size: ", videoParams_GetColorResolution(params));
			return FALSE;
		}
		output_select = 1;
	}
	else
	{
		error_Set(ERR_OUTPUT_ENCODING_TYPE_INVALID);
		LOG_ERROR2("invalid output encoding type: ", videoParams_GetEncodingOut(params));
		return FALSE;
	}

	halVideoPacketizer_PixelRepetitionFactor(baseAddr + VP_BASE_ADDR,
			videoParams_GetPixelRepetitionFactor(params));
	halVideoPacketizer_ColorDepth(baseAddr + VP_BASE_ADDR, color_depth);
	halVideoPacketizer_PixelPackingDefaultPhase(baseAddr
			+ VP_BASE_ADDR, videoParams_GetPixelPackingDefaultPhase(params));
	halVideoPacketizer_Ycc422RemapSize(baseAddr + VP_BASE_ADDR,
			remap_size);
	halVideoPacketizer_OutputSelector(baseAddr + VP_BASE_ADDR,
			output_select);
	return TRUE;
}

IM_INT32 video_VideoSampler(IM_UINT16 baseAddr, videoParams_t *params)
{
	IM_UINT32 map_code = 0;

	LOG_TRACE();
	if (videoParams_GetEncodingIn(params) == RGB || videoParams_GetEncodingIn(
			params) == YCC444)
	{
		if ((videoParams_GetColorResolution(params) == 8)
				|| (videoParams_GetColorResolution(params) == 0))
			map_code = 1;
		else if (videoParams_GetColorResolution(params) == 10)
			map_code = 3;
		else if (videoParams_GetColorResolution(params) == 12)
			map_code = 5;
		else if (videoParams_GetColorResolution(params) == 16)
			map_code = 7;
		else
		{
			error_Set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
			LOG_ERROR2("invalid color depth: ", videoParams_GetColorResolution(params));
			return FALSE;
		}
		map_code += (videoParams_GetEncodingIn(params) == YCC444) ? 8 : 0;
	}
	else if (videoParams_GetEncodingIn(params) == YCC422)
	{
		/* YCC422 mapping is discontinued - only map 1 is supported */
		if (videoParams_GetColorResolution(params) == 12)
			map_code = 18;
		else if (videoParams_GetColorResolution(params) == 10)
			map_code = 20;
		else if ((videoParams_GetColorResolution(params) == 8)
				|| (videoParams_GetColorResolution(params) == 0))
			map_code = 22;
		else
		{
			error_Set(ERR_COLOR_REMAP_SIZE_INVALID);
			LOG_ERROR2("invalid color remap size: ", videoParams_GetColorResolution(params));
			return FALSE;
		}
	}
	else
	{
		error_Set(ERR_INPUT_ENCODING_TYPE_INVALID);
		LOG_ERROR2("invalid input encoding type: ", videoParams_GetEncodingIn(params));
		return FALSE;
	}

	halVideoSampler_InternalDataEnableGenerator(baseAddr
			+ TX_BASE_ADDR, 0);
	halVideoSampler_VideoMapping(baseAddr + TX_BASE_ADDR, map_code);
	halVideoSampler_StuffingGy(baseAddr + TX_BASE_ADDR, 0);
	halVideoSampler_StuffingRcr(baseAddr + TX_BASE_ADDR, 0);
	halVideoSampler_StuffingBcb(baseAddr + TX_BASE_ADDR, 0);
	return TRUE;
}

IM_INT32 video_VideoGenerator(IM_UINT16 baseAddr, videoParams_t *params, IM_UINT8 dataEnablePolarity)
{
	/*
	 * video generator block is not included in real application hardware,
	 * then the external video sources are used and this code has no effect
	 */
	IM_UINT32 resolution = 0;
	const dtd_t *dtd;
	LOG_TRACE();
	dtd = videoParams_GetDtd(params);

	if ((videoParams_GetColorResolution(params) == 8)
			|| (videoParams_GetColorResolution(params) == 0))
		resolution = 0;
	else if (videoParams_GetColorResolution(params) == 10)
		resolution = 1;
	else if (videoParams_GetColorResolution(params) == 12)
		resolution = 2;
	else if (videoParams_GetColorResolution(params) == 16
			&& videoParams_GetEncodingIn(params) != YCC422)
		resolution = 3;
	else
	{
		error_Set(ERR_COLOR_DEPTH_NOT_SUPPORTED);		
		LOG_ERROR2("invalid color depth: ", videoParams_GetColorResolution(params));
		return FALSE;
	}

	halVideoGenerator_Ycc(baseAddr + VG_BASE_ADDR,
			videoParams_GetEncodingIn(params) != RGB);
	halVideoGenerator_Ycc422(baseAddr + VG_BASE_ADDR,
			videoParams_GetEncodingIn(params) == YCC422);
	halVideoGenerator_VBlankOsc(baseAddr + VG_BASE_ADDR, (dtd_GetCode(
			dtd) == 39) ? 0 : (dtd_GetInterlaced(dtd) ? 1 : 0));
	halVideoGenerator_ColorIncrement(baseAddr + VG_BASE_ADDR, 0);
	halVideoGenerator_Interlaced(baseAddr + VG_BASE_ADDR,
			dtd_GetInterlaced(dtd));
	halVideoGenerator_VSyncPolarity(baseAddr + VG_BASE_ADDR,
			dtd_GetVSyncPolarity(dtd));
	halVideoGenerator_HSyncPolarity(baseAddr + VG_BASE_ADDR,
			dtd_GetHSyncPolarity(dtd));
	halVideoGenerator_DataEnablePolarity(baseAddr + VG_BASE_ADDR,
			dataEnablePolarity);
	halVideoGenerator_ColorResolution(baseAddr + VG_BASE_ADDR,
			resolution);
	halVideoGenerator_PixelRepetitionInput(baseAddr + VG_BASE_ADDR,
			dtd_GetPixelRepetitionInput(dtd));
	halVideoGenerator_HActive(baseAddr + VG_BASE_ADDR, dtd_GetHActive(
			dtd));
	halVideoGenerator_HBlank(baseAddr + VG_BASE_ADDR,
			dtd_GetHBlanking(dtd));
	halVideoGenerator_HSyncEdgeDelay(baseAddr + VG_BASE_ADDR,
			dtd_GetHSyncOffset(dtd));
	halVideoGenerator_HSyncPulseWidth(baseAddr + VG_BASE_ADDR,
			dtd_GetHSyncPulseWidth(dtd));
	halVideoGenerator_VActive(baseAddr + VG_BASE_ADDR, dtd_GetVActive(
			dtd));
	halVideoGenerator_VBlank(baseAddr + VG_BASE_ADDR,
			dtd_GetVBlanking(dtd));
	halVideoGenerator_VSyncEdgeDelay(baseAddr + VG_BASE_ADDR,
			dtd_GetVSyncOffset(dtd));
	halVideoGenerator_VSyncPulseWidth(baseAddr + VG_BASE_ADDR,
			dtd_GetVSyncPulseWidth(dtd));
	halVideoGenerator_Enable3D(baseAddr + VG_BASE_ADDR,
			(videoParams_GetHdmiVideoFormat(params) == 2) ? 1 : 0);
	halVideoGenerator_Structure3D(baseAddr + VG_BASE_ADDR,
			videoParams_Get3dStructure(params));
	halVideoGenerator_SwReset(baseAddr + VG_BASE_ADDR, 1);
	return TRUE;
}
